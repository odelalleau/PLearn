// -*- C++ -*-

// TangentLearner.cc
//
// Copyright (C) 2004 Martin Monperrus 
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org

/* *******************************************************      
   * $Id: TangentLearner.cc,v 1.5 2004/06/02 02:06:13 yoshua Exp $ 
   ******************************************************* */

// Authors: Martin Monperrus & Yoshua Bengio

/*! \file TangentLearner.cc */


#include "TangentLearner.h"
#include "ProjectionErrorVariable.h"
//#include "LocalPCAVMatrix.h"
#include "LocalNeighborsDifferencesVMatrix.h"
#include "ProductVariable.h"
#include "Var_operators.h"
#include "ConcatColumnsVMatrix.h"
#include "random.h"
#include "SumOfVariable.h"
#include "TanhVariable.h"
//#include "TMat_maths_impl.h"
//#include "TVec_decl.h"

namespace PLearn {
using namespace std;

TangentLearner::TangentLearner() 
/* ### Initialize all fields to their default value here */
  : training_targets("local_neighbors"), use_subspace_distance(true), n_neighbors(5), n_dim(1),
    architecture_type("single_neural_network"), n_hidden_units(-1),
    batch_size(1), norm_penalization(0), svd_threshold(1e-3), projection_error_regularization(1e-4)
{
}

PLEARN_IMPLEMENT_OBJECT(TangentLearner, "Learns local tangent plane of the manifold near which the data lie.", 
			"This learner models a manifold near which the data are supposed to lie.\n"
			"The manifold is represented by a function which predicts a basis for the\n"
			"tangent planes at each point x, given x in R^n. Let f_i(x) be the predicted i-th tangent\n"
			"vector (in R^n). Then we will optimize the parameters that define the d functions f_i by\n"
      "to push the f_i so that they span the local tangent directions. Three criteria are\n"
      "possible, according to the 'training_targets' option:\n"
			" * If use_subspace_distance,\n"
      "      criterion = min_{w,u}  || sum_i w_i f_i  -  sum_j u_j t(x,j) ||^2\n"
      "      under the constraint that ||w||=1.\n"
      "   else\n"
			"      criterion = sum_x sum_j min_w ||t(x,j) - sum_i w_i f_i(x)||^2\n"
			"   where the first sum is over training examples and w is a free d-vector, and\n"
			"   t(x,j) estimates local tangent directions based on near neighbors. t(x,j)\n"
      "   is defined according to the training_targets option:\n"
			"    'local_evectors' : local principal components (based on n_neighbors of x)\n"
			"    'local_neighbors': difference between x and its n_neighbors.\n"
			);

void TangentLearner::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "training_targets", &TangentLearner::training_targets, OptionBase::buildoption,
		"Specifies a strategy for training the tangent plane predictor. Possible values are the strings\n"
		"   local_evectors   : local principal components (based on n_neighbors of x)\n"
		"   local_neighbors  : difference between x and its n_neighbors.\n"
		);

  declareOption(ol, "use_subspace_distance", &TangentLearner::use_subspace_distance, OptionBase::buildoption,
                "Minimize distance between subspace spanned by f_i and by (x-neighbors), instead of between\n"
                "the individual targets t_j and the subspace spanned by the f_i.\n");

  declareOption(ol, "n_neighbors", &TangentLearner::n_neighbors, OptionBase::buildoption,
		"Number of nearest neighbors to consider.\n"
		);

  declareOption(ol, "n_dim", &TangentLearner::n_dim, OptionBase::buildoption,
		"Number of tangent vectors to predict.\n"
		);

  declareOption(ol, "optimizer", &TangentLearner::optimizer, OptionBase::buildoption,
		"Optimizer that optimizes the cost function Number of tangent vectors to predict.\n"
		);
		  
//declareOption(ol, "tangent_predictor", &TangentLearner::tangent_predictor, OptionBase::buildoption,
//	"Func that specifies the parametrized mapping from inputs to predicted tangent planes\n"
//		);

  declareOption(ol, "architecture_type", &TangentLearner::architecture_type, OptionBase::buildoption,
		"For pre-defined tangent_predictor types: \n"
		"   multi_neural_network : prediction[j] = b[j] + W[j]*tanh(c[j] + V[j]*x), where W[j] has n_hidden_units columns\n"
		"                          where there is a separate set of parameters for each of n_dim tangent vectors to predict.\n"
		"   single_neural_network : prediction = b + W*tanh(c + V*x), where W has n_hidden_units columns\n"
		"                          where the resulting vector is viewed as a n_dim by n matrix\n"
		"   linear :         prediction = b + W*x\n"
		"   (empty string):  specify explicitly the function with tangent_predictor option\n"
		"where (b,W,c,V) are parameters to be optimized.\n"
		);

  declareOption(ol, "n_hidden_units", &TangentLearner::n_hidden_units, OptionBase::buildoption,
		"Number of hidden units (if architecture == 'neural_network')\n"
		);

  declareOption(ol, "batch_size", &TangentLearner::batch_size, OptionBase::buildoption, 
                "    how many samples to use to estimate the average gradient before updating the weights\n"
                "    0 is equivalent to specifying training_set->length() \n");

  declareOption(ol, "norm_penalization", &TangentLearner::norm_penalization, OptionBase::buildoption,
		"Factor that multiplies an extra penalization of the norm of f_i so that ||f_i|| be close to 1.\n"
    "The penalty is norm_penalization*sum_i (1 - ||f_i||^2)^2.\n"                
		);

  declareOption(ol, "svd_threshold", &TangentLearner::svd_threshold, OptionBase::buildoption,
		"Threshold to accept singular values of F in solving for linear combination weights on tangent subspace.\n"
		);

  declareOption(ol, "projection_error_regularization", &TangentLearner::projection_error_regularization, OptionBase::buildoption,
		"Term added to the linear system matrix involved in fitting subspaces in the projection error computation.\n"
		);

  declareOption(ol, "parameters", &TangentLearner::parameters, OptionBase::learntoption,
		"Parameters of the tangent_predictor function.\n"
		);

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void TangentLearner::build_()
{

  int n = PLearner::inputsize_;
  
  if (n>0)
  {
    if (architecture_type == "multi_neural_network")
      {
        if (n_hidden_units <= 0)
          PLERROR("TangentLearner::Number of hidden units should be positive, now %d\n",n_hidden_units);
      }
    if (architecture_type == "single_neural_network")
      {
        if (n_hidden_units <= 0)
          PLERROR("TangentLearner::Number of hidden units should be positive, now %d\n",n_hidden_units);
        Var x(n);
        b = Var(n_dim*n,1,"b");
        W = Var(n_dim*n,n_hidden_units,"W");
        c = Var(n_hidden_units,1,"c");
        V = Var(n_hidden_units,n,"V");
        tangent_predictor = Func(x, b & W & c & V, b + product(W,tanh(c + product(V,x))));
      }
    else if (architecture_type == "linear")
      {
        Var x(n);
        b = Var(n_dim*n,1,"b");
        W = Var(n_dim*n,n,"W");
        tangent_predictor = Func(x, b & W, b + product(W,x));
      }
    else if (architecture_type != "")
      PLERROR("TangentLearner::build, unknown architecture_type option %s (should be 'neural_network', 'linear', or empty string '')\n",
              architecture_type.c_str());

    if (parameters.size()>0 && parameters.nelems() == tangent_predictor->parameters.nelems())
      tangent_predictor->parameters.copyValuesFrom(parameters);
    else
      {
        parameters.resize(tangent_predictor->parameters.size());
        for (int i=0;i<parameters.size();i++)
          parameters[i] = tangent_predictor->parameters[i];
      }
    
    if (training_targets=="local_evectors")
      tangent_targets = Var(n_dim,n);
    else if (training_targets=="local_neighbors")
      tangent_targets = Var(n_neighbors,n);
    else PLERROR("TangentLearner::build, option training_targets is %s, should be 'local_evectors' or 'local_neighbors'.",
                 training_targets.c_str());

    Var proj_err = projection_error(tangent_predictor->outputs[0], tangent_targets, norm_penalization, n, 
                                    use_subspace_distance, svd_threshold, projection_error_regularization);
    cost_of_one_example = Func(tangent_predictor->inputs & tangent_targets, tangent_predictor->parameters, proj_err);

  }
}

// ### Nothing to add here, simply calls build_
void TangentLearner::build()
{
  inherited::build();
  build_();
}

extern void varDeepCopyField(Var& field, CopiesMap& copies);

void TangentLearner::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(cost_of_one_example, copies);
  varDeepCopyField(b, copies);
  varDeepCopyField(W, copies);
  varDeepCopyField(c, copies);
  varDeepCopyField(V, copies);
  varDeepCopyField(tangent_targets, copies);
  deepCopyField(parameters, copies);
  deepCopyField(optimizer, copies);
  deepCopyField(tangent_predictor, copies);
}


int TangentLearner::outputsize() const
{
  return n_dim*inputsize();
}

void TangentLearner::forget()
{
  if (train_set) initializeParams();
  stage = 0;
}
    
void TangentLearner::train()
{

  VMat train_set_with_targets;
  VMat targets_vmat;
  if (!cost_of_one_example)
    PLERROR("TangentLearner::train: build has not been run after setTrainingSet!");

  if (training_targets == "local_evectors")
  {
    //targets_vmat = new LocalPCAVMatrix(train_set, n_neighbors, n_dim);
    PLERROR("local_evectors not yet implemented");
  }
  else if (training_targets == "local_neighbors")
  {

    targets_vmat = local_neighbors_differences(train_set, n_neighbors);
  }
  else PLERROR("TangentLearner::train, unknown training_targets option %s (should be 'local_evectors' or 'local_neighbors')\n",
	       training_targets.c_str());
  
  train_set_with_targets = hconcat(train_set, targets_vmat);
  train_set_with_targets->defineSizes(inputsize(),inputsize()*n_neighbors,0);
  int l = train_set->length();  
  int nsamples = batch_size>0 ? batch_size : l;
  Var totalcost = meanOf(train_set_with_targets, cost_of_one_example, nsamples);
  if(optimizer)
    {
      optimizer->setToOptimize(parameters, totalcost);  
      optimizer->build();
    }
  else PLERROR("TangentLearner::train can't train without setting an optimizer first!");
  
  // number of optimizer stages corresponding to one learner stage (one epoch)
  int optstage_per_lstage = l/nsamples;

  ProgressBar* pb = 0;
  if(report_progress)
    pb = new ProgressBar("Training TangentLearner from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

  int initial_stage = stage;
  bool early_stop=false;
  while(stage<nstages && !early_stop)
    {
      optimizer->nstages = optstage_per_lstage;
      train_stats->forget();
      optimizer->early_stop = false;
      optimizer->optimizeN(*train_stats);
      train_stats->finalize();
      if(verbosity>2)
        cout << "Epoch " << stage << " train objective: " << train_stats->getMean() << endl;
      ++stage;
      if(pb)
        pb->update(stage-initial_stage);
    }
  if(verbosity>1)
    cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;

  if(pb)
    delete pb;
}

void TangentLearner::initializeParams()
{
  if (seed_>=0)
    manual_seed(seed_);
  else
    PLearn::seed();

  if (architecture_type=="single_neural_network")
  {
    real delta = 1.0 / sqrt(real(inputsize()));
    fill_random_uniform(V->value, -delta, delta);
    delta = 1.0 / real(n_hidden_units);
    fill_random_uniform(W->matValue, -delta, delta);
    b->matValue.clear();
    c->matValue.clear();
  }
  else if (architecture_type=="linear")
  {
    real delta = 1.0 / sqrt(real(inputsize()));
    b->matValue.clear();
    fill_random_uniform(W->matValue, -delta, delta);
  }
  else PLERROR("other types not handled yet!");
  // Reset optimizer
  if(optimizer)
    optimizer->reset();
}


void TangentLearner::computeOutput(const Vec& input, Vec& output) const
{
  int nout = outputsize();
  output.resize(nout);
  output << tangent_predictor(input);
}    

void TangentLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
					     const Vec& target, Vec& costs) const
{
}                                

TVec<string> TangentLearner::getTestCostNames() const
{
  TVec<string> no_costs;
  return no_costs;
}

TVec<string> TangentLearner::getTrainCostNames() const
{
  TVec<string> cost(1); cost[0] = "projection_error";
  return cost;
}


} // end of namespace PLearn
