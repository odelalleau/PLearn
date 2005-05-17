// -*- C++ -*-

// NonLocalManifoldParzen.cc
//
// Copyright (C) 2004 Yoshua Bengio & Hugo Larochelle 
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
   * $Id: NonLocalManifoldParzen.cc,v 1.5 2005/05/17 18:26:22 tihocan Exp $
   ******************************************************* */

// Authors: Yoshua Bengio & Martin Monperrus

/*! \file NonLocalManifoldParzen.cc */


#include "NonLocalManifoldParzen.h"
#include <plearn/vmat/LocalNeighborsDifferencesVMatrix.h>
#include <plearn/var/ProductVariable.h>
#include <plearn/var/PlusVariable.h>
#include <plearn/var/SoftplusVariable.h>
#include <plearn/var/VarRowsVariable.h>
#include <plearn/var/VarRowVariable.h>
#include <plearn/var/SourceVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/math/random.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/NllGeneralGaussianVariable.h>
#include <plearn/var/DiagonalizedFactorsProductVariable.h>
#include <plearn/math/random.h>
#include <plearn/math/plapack.h>
#include <plearn/var/ColumnSumVariable.h>
#include <plearn/vmat/VMat_basic_stats.h>
#include <plearn/vmat/ConcatRowsVMatrix.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/var/PDistributionVariable.h>
#include <plearn_learners/distributions/UniformDistribution.h>
#include <plearn_learners/distributions/GaussianDistribution.h>
#include <plearn/display/DisplayUtils.h>
#include <plearn/opt/GradientOptimizer.h>
#include <plearn/var/TransposeVariable.h>
#include <plearn/var/Var_utils.h>
#include <plearn/var/ConcatRowsVariable.h>
#include <plearn/var/RowSumVariable.h>
#include <plearn/var/ThresholdBpropVariable.h>
#include <plearn/var/ReshapeVariable.h>
#include <plearn/var/SquareVariable.h>
#include <plearn/var/ExpVariable.h>
#include <plearn/io/load_and_save.h>
#include <plearn/vmat/VMat_computeNearestNeighbors.h>
#include <plearn/vmat/FractionSplitter.h>
#include <plearn/vmat/RepeatSplitter.h>
#include <plearn/var/FNetLayerVariable.h>

namespace PLearn {
using namespace std;


NonLocalManifoldParzen::NonLocalManifoldParzen() 
/* ### Initialize all fields to their default value here */
  :  reference_set(0), sigma_init(0.1), sigma_min(0.00001), nneighbors(5), nneighbors_density(-1), mu_nneighbors(2), ncomponents(1), sigma_threshold_factor(1), variances_transfer_function("softplus"), architecture_type("single_neural_network"),
    n_hidden_units(-1), batch_size(1), svd_threshold(1e-8)
{
}

PLEARN_IMPLEMENT_OBJECT(NonLocalManifoldParzen, "to do",
                        "I SAID TO DO!\n"
                        );


void NonLocalManifoldParzen::declareOptions(OptionList& ol)
{

  declareOption(ol, "nneighbors", &NonLocalManifoldParzen::nneighbors, OptionBase::buildoption,
		"Number of nearest neighbors to consider for gradient descent.\n"
		);

  declareOption(ol, "nneighbors_density", &NonLocalManifoldParzen::nneighbors_density, OptionBase::buildoption,
		"Number of nearest neighbors to consider for p(x) density estimation.\n"
		);

  declareOption(ol, "mu_nneighbors", &NonLocalManifoldParzen::mu_nneighbors, OptionBase::buildoption,
		"Number of nearest neighbors to learn the mus (if < 0, mu_nneighbors = nneighbors).\n"
		);

  declareOption(ol, "ncomponents", &NonLocalManifoldParzen::ncomponents, OptionBase::buildoption,
		"Number of tangent vectors to predict.\n"
		);

  declareOption(ol, "sigma_threshold_factor", &NonLocalManifoldParzen::sigma_threshold_factor, OptionBase::buildoption,
		"Threshold factor of the gradient on the sigma noise. \n"
		);

  declareOption(ol, "optimizer", &NonLocalManifoldParzen::optimizer, OptionBase::buildoption,
		"Optimizer that optimizes the cost function.\n"
		);
		  
  declareOption(ol, "variances_transfer_function", &NonLocalManifoldParzen::variances_transfer_function, 
                OptionBase::buildoption,
                "Type of output transfer function for predicted variances, to force them to be >0:\n"
                "  square : take the square\n"
                "  exp : apply the exponential\n"
                "  softplus : apply the function log(1+exp(.))\n"
                );
		  
  declareOption(ol, "architecture_type", &NonLocalManifoldParzen::architecture_type, OptionBase::buildoption,
		"For pre-defined tangent_predictor types: \n"
		"   single_neural_network : prediction = b + W*tanh(c + V*x), where W has n_hidden_units columns\n"
		"                          where the resulting vector is viewed as a ncomponents by n matrix\n"
    "   embedding_neural_network: prediction[k,i] = d(e[k])/d(x[i), where e(x) is an ordinary neural\n"
    "                             network representing the embedding function (see output_type option)\n"
		"where (b,W,c,V) are parameters to be optimized.\n"
		);

  declareOption(ol, "n_hidden_units", &NonLocalManifoldParzen::n_hidden_units, OptionBase::buildoption,
		"Number of hidden units (if architecture_type is some kind of neural network)\n"
		);

  declareOption(ol, "batch_size", &NonLocalManifoldParzen::batch_size, OptionBase::buildoption, 
                "    how many samples to use to estimate the average gradient before updating the weights\n"
                "    0 is equivalent to specifying training_set->length() \n");

  declareOption(ol, "svd_threshold", &NonLocalManifoldParzen::svd_threshold, OptionBase::buildoption,
		"Threshold to accept singular values of F in solving for linear combination weights on tangent subspace.\n"
		);

  declareOption(ol, "parameters", &NonLocalManifoldParzen::parameters, OptionBase::learntoption,
		"Parameters of the tangent_predictor function.\n"
		);

  declareOption(ol, "L", &NonLocalManifoldParzen::L, OptionBase::learntoption,
		"Number of gaussians.\n"
		);

  declareOption(ol, "Us", &NonLocalManifoldParzen::Us, OptionBase::learntoption,
		"The U matrices for the reference set.\n"
		);

  declareOption(ol, "mus", &NonLocalManifoldParzen::mus, OptionBase::learntoption,
		"The mu vectors for the reference set.\n"
                );

  declareOption(ol, "sms", &NonLocalManifoldParzen::sms, OptionBase::learntoption,
		"The sm values for the reference set.\n"
                );
  
  declareOption(ol, "sns", &NonLocalManifoldParzen::sns, OptionBase::learntoption,
		"The sn values for the reference set.\n"
                );

  declareOption(ol, "sigma_min", &NonLocalManifoldParzen::sigma_min, OptionBase::buildoption,
		"The minimum value for sigma noise.\n"
                );

  declareOption(ol, "sigma_init", &NonLocalManifoldParzen::sigma_min, OptionBase::buildoption,
		"Initial minimum value for sigma noise.\n"
                );

  declareOption(ol, "reference_set", &NonLocalManifoldParzen::reference_set, OptionBase::learntoption,
		"Reference points for density computation.\n"
                );

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void NonLocalManifoldParzen::build_()
{

  n = PLearner::inputsize_;

  if (n>0)
  {

    VarArray params;

    Var log_n_examples(1,1,"log(n_examples)");
    if(train_set)
    {
      L = train_set->length();
      reference_set = train_set; // Maybe things could be changed here to make access faster!
    }

    log_L= log((real) L);

    {
      if (n_hidden_units <= 0)
        PLERROR("NonLocalManifoldParzen::Number of hidden units should be positive, now %d\n",n_hidden_units);

      
      x = Var(n);
      Var a; // outputs of hidden layer

      if (hidden_layer) // user-specified hidden layer Var
      {
         NaryVariable* layer_var = dynamic_cast<NaryVariable*>((Variable*)hidden_layer);
         if (!layer_var)
           PLERROR("In NonLocalManifoldParzen::build - 'hidden_layer' should be "
                   "from a subclass of NaryVariable");
         if (layer_var->varray.size() < 1)
           layer_var->varray.resize(1);
         layer_var->varray[0] = transpose(x);
         layer_var->build(); // make sure everything is consistent and finish the build
         if (layer_var->varray.size()<2)
           PLERROR("In NonLocalManifoldParzen::build - 'hidden_layer' should have parameters");
         for (int i=1;i<layer_var->varray.size();i++)
           params.append(layer_var->varray[i]);
         a = layer_var;
      }
      else // standard hidden layer
      {
        Var c = Var(n_hidden_units,1,"c ");
        Var V = Var(n_hidden_units,n,"V ");               
        params.append(c);
        params.append(V);
        a = tanh(c + product(V,x));
      }

      muV = Var(n,n_hidden_units,"muV "); 
      snV = Var(1,n_hidden_units,"snV ");  
      snb = Var(1,1,"snB ");      
        

      if(architecture_type == "embedding_neural_network")
      {
        W = Var(ncomponents,n_hidden_units,"W ");       
        tangent_plane = diagonalized_factors_product(W,1-a*a,V); 
        embedding = product(W,a);
        output_embedding = Func(x,embedding);
        params.append(W);
      } 
      else if(architecture_type == "single_neural_network")
      {
        b = Var(ncomponents*n,1,"b");
        W = Var(ncomponents*n,n_hidden_units,"W ");
        tangent_plane = reshape(b + product(W,a),ncomponents,n);
        params.append(b);
        params.append(W);
      }
      else
        PLERROR("NonLocalManifoldParzen::build_, unknown architecture_type option %s",
                architecture_type.c_str());
     
      mu = product(muV,a); 
      params.append(muV);
      min_sig = new SourceVariable(1,1);
      min_sig->value[0] = sigma_min;
      min_sig->setName("min_sig");
      init_sig = Var(1,1);
      init_sig->setName("init_sig");

      if(variances_transfer_function == "softplus") sn = softplus(snb + product(snV,a))  + min_sig + softplus(init_sig);
      else if(variances_transfer_function == "square") sn = square(snb + product(snV,a)) + min_sig + square(init_sig);
      else if(variances_transfer_function == "exp") sn = exp(snb + product(snV,a)) + min_sig + exp(init_sig);
      else PLERROR("In NonLocalManifoldParzen::build_ : unknown variances_transfer_function option %s ", variances_transfer_function.c_str());
      
      params.append(snV);
      params.append(snb);
      params.append(init_sig);

      if(sigma_threshold_factor > 0)
      {        
        sn = threshold_bprop(sn,sigma_threshold_factor);
      }

      tangent_plane->setName("tangent_plane ");
      mu->setName("mu ");
      sn->setName("sn ");
      a->setName("a ");
      if(architecture_type == "embedding_neural_network")
        embedding->setName("embedding ");
      x->setName("x ");

      predictor = Func(x, params , tangent_plane & mu & sn );

      output_f_all = Func(x,tangent_plane & mu & sn);
    }

    if (parameters.size()>0 && parameters.nelems() == predictor->parameters.nelems())
      predictor->parameters.copyValuesFrom(parameters);
    parameters.resize(predictor->parameters.size());
    for (int i=0;i<parameters.size();i++)
      parameters[i] = predictor->parameters[i];

    Var target_index = Var(1,1);
    target_index->setName("target_index");
    Var neighbor_indexes = Var(nneighbors,1);
    neighbor_indexes->setName("neighbor_indexes");

    /*
    // The following variables are discarded to
    // make the gradient computation faster
    // and more stable in nlmp_general_gaussian
    log_p_x = Var(L,1);
    log_p_x->setName("log_p_x");

    // Initialisation hack for nlmp_general_gaussian hack
    for(int i=0; i<log_p_x.length(); i++)
      log_p_x->value[i] = MISSING_VALUE;

    log_p_target = new VarRowsVariable(log_p_x,target_index);
    log_p_target->value[0] = log(1.0/L);
    log_p_target->setName("log_p_target");
    log_p_neighbors =new VarRowsVariable(log_p_x,neighbor_indexes);
    log_p_neighbors->setName("log_p_neighbors");
    */

    tangent_targets = Var(nneighbors,n);
    
    if(mu_nneighbors < 0 ) mu_nneighbors = nneighbors;

    // compute - sum_{neighbors of x} log ( P(neighbor|x) ) according to semi-spherical model
    Var nll = nll_general_gaussian(tangent_plane, mu, sn, tangent_targets, log_L, mu_nneighbors); // + log_n_examples;

    Var knn = new SourceVariable(1,1);
    knn->setName("knn");
    knn->value[0] = nneighbors;
    sum_nll = new ColumnSumVariable(nll) / knn;

    cost_of_one_example = Func(x & tangent_targets & target_index & neighbor_indexes, predictor->parameters, sum_nll);

    if(nneighbors_density >= L || nneighbors_density < 0) nneighbors_density = L;

    t_row.resize(n);
    Ut_svd.resize(n,n);
    V_svd.resize(ncomponents,ncomponents);
    F.resize(tangent_plane->length(),tangent_plane->width());
    z.resize(n);
    x_minus_neighbor.resize(n);
    neighbor_row.resize(n);

    Us.resize(L);
    mus.resize(L, n);
    sms.resize(L,ncomponents);
    sns.resize(L);
    
    // Kernel methods
    mu_temp.resize(n);
    diff.resize(n);
    sm_temp.resize(ncomponents);
    sn_temp.resize(1);
  }

}

void NonLocalManifoldParzen::update_reference_set_parameters()
{
  // Compute Us, mus, sms, sns
  Us.resize(L);
  mus.resize(L, n);
  sms.resize(L,ncomponents);
  sns.resize(L);
  
  for(int t=0; t<L; t++)
  {
    Us[t].resize(ncomponents,n);
    reference_set->getRow(t,t_row);
    predictor->fprop(t_row, F.toVec() & mus(t) & sns.subVec(t,1));
    
    // N.B. this is the SVD of F'
    lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
    for (int k=0;k<ncomponents;k++)
    {
      sms(t,k) = S_svd[k];
      Us[t](k) << Ut_svd(k);
    }    
  }

}

void NonLocalManifoldParzen::knn(const VMat& vm, const Vec& x, const int& k, TVec<int>& neighbors, bool sortk) const
{
  int n = vm->length();
  distances.resize(n,2);
  distances.column(1) << Vec(0, n-1, 1); 
  dk.setDataForKernelMatrix(vm);
  t_dist.resize(n);
  dk.evaluate_all_i_x(x, t_dist);
  distances.column(0) << t_dist;
  partialSortRows(distances, k, sortk);
  neighbors.resize(k);

  for (int i=0; i < k  && i<n; i++)
  {
    neighbors[i] = int(distances(i,1));
  }


  /*
  for (int i = 0, j=0; i < k  && j<n; j++)
  {
    real d = distances(j,0);
    if (include_current_point || d>0)  //Ouach, caca!!!
    {
      neighbors[i] = int(distances(j,1));
      i++;
    }
  }
  */
}

// ### Nothing to add here, simply calls build_
void NonLocalManifoldParzen::build()
{
  inherited::build();
  build_();
}

#ifdef __INTEL_COMPILER
#pragma warning(disable:1419)  // Get rid of compiler warning.
#endif
extern void varDeepCopyField(Var& field, CopiesMap& copies);
#ifdef __INTEL_COMPILER
#pragma warning(default:1419)
#endif

void NonLocalManifoldParzen::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(cost_of_one_example, copies);
  deepCopyField(reference_set,copies);
  varDeepCopyField(x, copies);
  varDeepCopyField(b, copies);
  varDeepCopyField(W, copies);
  varDeepCopyField(c, copies);
  varDeepCopyField(V, copies);
  varDeepCopyField(tangent_targets, copies);
  varDeepCopyField(muV, copies);
  varDeepCopyField(snV, copies);
  varDeepCopyField(snb, copies);
  varDeepCopyField(mu, copies);
  varDeepCopyField(sn, copies);
  varDeepCopyField(tangent_plane, copies);
  varDeepCopyField(sum_nll, copies);
  varDeepCopyField(min_sig, copies);
  varDeepCopyField(init_sig, copies);
  varDeepCopyField(embedding, copies);

  deepCopyField(Us, copies);
  deepCopyField(mus, copies);
  deepCopyField(sms, copies);
  deepCopyField(sns, copies);
  deepCopyField(Ut_svd, copies);
  deepCopyField(V_svd, copies);
  deepCopyField(S_svd, copies);
  deepCopyField(F, copies);

  deepCopyField(parameters, copies);
  varDeepCopyField(hidden_layer, copies);
  deepCopyField(optimizer, copies);
  deepCopyField(predictor, copies);
  deepCopyField(output_f_all, copies);
  deepCopyField(output_embedding, copies);

  // TODO : verify WTF with DistanceKernel
  deepCopyField(z,copies);
  deepCopyField(x_minus_neighbor,copies);
  deepCopyField(t_row,copies);
  deepCopyField(neighbor_row,copies);
  deepCopyField(log_gauss,copies);
  deepCopyField(t_nn,copies);
  deepCopyField(t_dist,copies);
  deepCopyField(distances,copies);
}


void NonLocalManifoldParzen::forget()
{
  if (train_set) initializeParams();
  stage = 0;
}
    
void NonLocalManifoldParzen::train()
{

  // Set train_stats if not already done.
  if (!train_stats)
    train_stats = new VecStatsCollector();

  VMat train_set_with_targets;
  VMat targets_vmat;
  if (!cost_of_one_example)
    PLERROR("NonLocalManifoldParzen::train: build has not been run after setTrainingSet!");

  targets_vmat = local_neighbors_differences(train_set, nneighbors, false, true);

  train_set_with_targets = hconcat(train_set, targets_vmat);
  train_set_with_targets->defineSizes(inputsize()+inputsize()*nneighbors+1+nneighbors,0);
  int nsamples = batch_size>0 ? batch_size : train_set->length();

  Var totalcost = meanOf(train_set_with_targets, cost_of_one_example, nsamples);

  if(optimizer)
    {
      optimizer->setToOptimize(parameters, totalcost);  
      optimizer->build();
    }
  else PLERROR("NonLocalManifoldParzen::train can't train without setting an optimizer first!");
  
  // number of optimizer stages corresponding to one learner stage (one epoch)
  int optstage_per_lstage = train_set->length()/nsamples;

  ProgressBar* pb = 0;
  if(report_progress>0)
    pb = new ProgressBar("Training NonLocalManifoldParzen from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

  t_row.resize(train_set.width());

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
  
  update_reference_set_parameters();
}

//////////////////////
// initializeParams //
//////////////////////
void NonLocalManifoldParzen::initializeParams()
{
  if (seed_>=0)
    manual_seed(seed_);
  else
    PLearn::seed();

  if (architecture_type=="embedding_neural_network")
  {
    real delta = 1.0 / sqrt(real(inputsize()));
    fill_random_uniform(V->value, -delta, delta);
    delta = 1.0 / real(n_hidden_units);
    fill_random_uniform(W->matValue, -delta, delta);
    c->value.clear();
    snb->value.clear();
    fill_random_uniform(snV->matValue, -delta, delta);
    fill_random_uniform(muV->matValue, -delta, delta);
    //min_sig->value[0] = sigma_init;
    //min_d->value.fill(diff_init);
    if(variances_transfer_function == "softplus") { min_sig->value[0] = log(exp(sigma_init)-1); }
    else if(variances_transfer_function == "square") { min_sig->value[0] = sqrt(sigma_init);}
    else if(variances_transfer_function == "exp") { min_sig->value[0] = log(sigma_init); }
  }
  else if (architecture_type=="single_neural_network")
  {
    real delta = 1.0 / sqrt(real(inputsize()));
    fill_random_uniform(V->value, -delta, delta);
    delta = 1.0 / real(n_hidden_units);
    fill_random_uniform(W->matValue, -delta, delta);
    c->value.clear();
    snb->value.clear();
    fill_random_uniform(snV->matValue, -delta, delta);
    fill_random_uniform(muV->matValue, -delta, delta);
    b->value.clear();
    //min_sig->value[0] = sigma_init;
    //min_d->value.fill(diff_init);
    if(variances_transfer_function == "softplus") { init_sig->value[0] = log(exp(sigma_init)-1); }
    else if(variances_transfer_function == "square") { init_sig->value[0] = sqrt(sigma_init);}
    else if(variances_transfer_function == "exp") { init_sig->value[0] = log(sigma_init);}
  }
  else PLERROR("other types not handled yet!");
  
  /*
  for(int i=0; i<log_p_x.length(); i++)
    //p_x->value[i] = log(1.0/p_x.length());
    log_p_x->value[i] = MISSING_VALUE;
  */
  if(optimizer)
    optimizer->reset();
}

/////////////////
// log_density //
/////////////////
real NonLocalManifoldParzen::log_density(const Vec& x) const {
  // Compute log-density.

  t_row << x;
  real mahal = 0;
  real norm_term = 0;
  
  if(nneighbors_density != L)
  {
    // Fetching nearest neighbors for density estimation.
    knn(reference_set,x,nneighbors_density,t_nn,bool(0));
    log_gauss.resize(t_nn.length());
    for(int neighbor=0; neighbor<t_nn.length(); neighbor++)
    {
      reference_set->getRow(t_nn[neighbor],neighbor_row);
      substract(t_row,neighbor_row,x_minus_neighbor);
      substract(x_minus_neighbor,mus(t_nn[neighbor]),z);

      mahal = -0.5*pownorm(z)/sns[t_nn[neighbor]];      
      norm_term = - n/2.0 * Log2Pi - log_L - 0.5*(n-ncomponents)*log(sns[t_nn[neighbor]]);
      for(int k=0; k<ncomponents; k++)
      {       
        mahal -= square(dot(z,Us[t_nn[neighbor]](k)))*(0.5/(sms(t_nn[neighbor],k)+sns[t_nn[neighbor]]) - 0.5/sns[t_nn[neighbor]]); // Pourrait être accéléré!
        norm_term -= 0.5*log(sms(t_nn[neighbor],k)+sns[t_nn[neighbor]]);
      }
      
      log_gauss[neighbor] = mahal + norm_term;
    }
  }
  else
  {
    // Fetching nearest neighbors for density estimation.
    log_gauss.resize(L);
    for(int t=0; t<L;t++)
    {
      reference_set->getRow(t,neighbor_row);
      substract(t_row,neighbor_row,x_minus_neighbor);
      substract(x_minus_neighbor,mus(t),z);

      mahal = -0.5*pownorm(z)/sns[t];      
      norm_term = - n/2.0 * Log2Pi - log_L - 0.5*(n-ncomponents)*log(sns[t]);
      for(int k=0; k<ncomponents; k++)
      {
        mahal -= square(dot(z,Us[t](k)))*(0.5/(sms(t,k)+sns[t]) - 0.5/sns[t]); // Pourrait être accéléré!
        norm_term -= 0.5*log(sms(t,k)+sns[t]);
      }

      log_gauss[t] = mahal + norm_term;
    }
  }
  return logadd(log_gauss);
}

/////////////////////
// getEigenvectors //
/////////////////////
Mat NonLocalManifoldParzen::getEigenvectors(int j) const {
  return Us[j];
}

Vec NonLocalManifoldParzen::getTrainPoint(int j) const {
  Vec ret(reference_set->width());
  reference_set->getRow(j,ret);
  return ret;
}

///////////////////
// computeOutput //
///////////////////
void NonLocalManifoldParzen::computeOutput(const Vec& input, Vec& output) const
{
  switch(outputs_def[0])
  {
  case 'm':
    output_embedding(input);
    output << embedding->value;
    break;
  default:
    inherited::computeOutput(input,output);
  }
}

////////////////
// outputsize //
////////////////
int NonLocalManifoldParzen::outputsize() const
{
  switch(outputs_def[0])
  {
  case 'm':
    return ncomponents;
    break;
  default:
    return inherited::outputsize();
  }
}

real NonLocalManifoldParzen::evaluate(Vec x1,Vec x2,real scale)
{
  real ret;

  predictor->fprop(x2, F.toVec() & mu_temp & sn_temp);
    
  // N.B. this is the SVD of F'
  lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
  for (int k=0;k<ncomponents;k++)
  {
    sm_temp[k] = S_svd[k];
    F(k) << Ut_svd(k);
  }    
  
  diff = x1 - x2;
  diff -= mu_temp;
  ret = scale * pownorm(diff)/sn_temp[0];
  for (int k = 0; k < ncomponents ; k++) {
    ret += scale * (1.0 /( sm_temp[k] + sn_temp[0]) - 1.0/sn_temp[0]) * square(dot(F(k), diff));
  }
  return ret;
  
}

} // end of namespace PLearn
