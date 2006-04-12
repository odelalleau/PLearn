// -*- C++ -*-

// TangentLearner.cc
//
// Copyright (C) 2004 Martin Monperrus & Yoshua Bengio
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
 * $Id$ 
 ******************************************************* */

// Authors: Martin Monperrus & Yoshua Bengio

/*! \file TangentLearner.cc */


#include "TangentLearner.h"
#include <plearn/var/ProjectionErrorVariable.h>
//#include "LocalPCAVMatrix.h"
#include <plearn/vmat/LocalNeighborsDifferencesVMatrix.h>
#include <plearn/var/ProductVariable.h>
#include <plearn/var/PlusVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/var/NoBpropVariable.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/math/random.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/DiagonalizedFactorsProductVariable.h>
#include <plearn/math/random.h>
#include <plearn/math/plapack.h>
//#include "TMat_maths.h"
//#include "TVec_decl.h"

namespace PLearn {
using namespace std;

// les neurones de la couche cachée correspondent à des hyperplans
// la smartInitialization consiste a initialiser ces hyperplans passant
// des points du train_set pris aleatoirement
// comme ca, on est sur de bien quadriller l'espace des points.
// le c correspond a une sorte de contre weight decay
// plus c est grand plus on aura des poids grand et plus on a des neurones tranchés dans l'espace
Mat smartInitialization(VMat v, int n, real c, real regularization)
{
    int l = v->length();
    int w = v->width();
  
    Mat result(n,w);
    Mat temp(w,w);
    Vec b(w);
    b<<c;
  
    int i,j;

    for (i=0;i<n;++i)
    {
        temp.clear();
        for (j=0;j<w;++j)
        {
            v->getRow(uniform_multinomial_sample(l),temp(j));
        }
        // regularization pour eviter 1/ quand on a tire deux fois le meme indice  2/ quand les points sont trops proches
        regularizeMatrix(temp,regularization);
        result(i) << solveLinearSystem(temp, b);
    }
    return result;
}

TangentLearner::TangentLearner() 
/* ### Initialize all fields to their default value here */
    : training_targets("local_neighbors"), use_subspace_distance(false), normalize_by_neighbor_distance(true),
      ordered_vectors(false), smart_initialization(0),initialization_regularization(1e-3),
      n_neighbors(5), n_dim(1), architecture_type("single_neural_network"), output_type("tangent_plane"),
      n_hidden_units(-1), batch_size(1), norm_penalization(0), svd_threshold(1e-5), 
      projection_error_regularization(0), V_slack(0)
    
{
}

PLEARN_IMPLEMENT_OBJECT(TangentLearner, "Learns local tangent plane of the manifold near which the data lie.", 
			"This learner models a manifold near which the data are supposed to lie.\n"
			"The manifold is represented by a function which predicts a basis for the\n"
			"tangent planes at each point x, given x in R^n. Let f_i(x) be the predicted i-th tangent\n"
			"vector (in R^n). Then we will optimize the parameters that define the d functions f_i by\n"
                        "pushing the f_i so that they span the local tangent directions. Three criteria are\n"
                        "possible, according to the 'training_targets', 'normalize_by_neighbor_distance' and\n"
                        "'use_subspace_distance' option. The default criterion is the recommanded one, with\n"
                        " training_targets='local_neighbors', normalize_by_neighbor_distance=1,\n"
                        "and use_subspace_distance=0 (it really did not work well in our experiments with\n"
                        "use_subspace_distance=1). This corresponds to the following cost function:\n"
                        "    sum_x sum_j min_w ||t(x,j) - sum_i w_i f_i(x)||^2 / ||t(x,j)||^2\n"
                        "where x is an example, t(x,j) is the difference vector between x and its j-th neighbor,\n"
                        "and the w_i are chosen freely for each j and x and correspond to the weights given to\n"
                        "each basis vector f_i(x) to obtain the projection of t(x,j) on the tangent plane.\n"
			"More generally, if use_subspace_distance,\n"
                        "      criterion = min_{w,u}  || sum_i w_i f_i  -  sum_j u_j t(x,j) ||^2\n"
                        "      under the constraint that ||w||=1.\n"
                        "   else\n"
			"      criterion = sum_x sum_j min_w ||t(x,j) - sum_i w_i f_i(x)||^2 / ||t(x,j)||^2\n"
			"   where the first sum is over training examples and w is a free d-vector,\n"
			"   t(x,j) estimates local tangent directions based on near neighbors, and the denominator\n"
                        "   ||t(x,j)||^2 is optional (normalize_by_neighbor_distance). t(x,j)\n"
                        "   is defined according to the training_targets option:\n"
			"    'local_evectors' : local principal components (based on n_neighbors of x)\n"
			"    'local_neighbors': difference between x and its n_neighbors.\n"
                        "An additional criterion option that applies only to use_subspace_criterion=0 is\n"
                        "the orderered_vectors option, which applies a separate cost to each of the f_i:\n"
                        "the f_1 vector tries to make the projection of t(x,j) on f_1 close to t(x,j), while\n"
                        "the f_2 vector tries to make the projection of t(x,j) on the (f_1,f_2) basis close to t(x,j),\n"
                        "etc... i.e. the gradient on f_i is computed based on a cost that involves only\n"
                        "the projection on the first i vectors. This is analogous to principal component analysis:\n"
                        "the first vector tries to capture as much as possible of the variance, the second as much\n"
                        "as possible of the remaining variance, etc...\n"
                        "Different architectures are possible for the f_i(x) (architecture_type option):\n"
                        "   - multi_neural_network: one neural net per basis function\n"
                        "   - single_neural_network: single neural network with matrix output (one row per basis vector)\n"
                        "   - linear: F_{ij}(x) = sum_k A_{ijk} x_k\n"
                        "   - embedding_neural_network: the embedding function e_k(x) (for k-th dimension)\n"
                        "        is an ordinary neural network, and F_{ki}(x) = d(e_k(x))/d(x_i). This allows to\n"
                        "        output the embedding, instead of, or as well as, the tangent plane (output_type option).\n"
                        "   - embedding_quadratic: the embedding function e_k(x) (for k-th dimension)\n"
                        "        is a 2nd order polynomial of x, and F_{ki}(x) = d(e_k(x))/d(x_i). This allows to\n"
                        "        output the embedding, instead of, or as well as, the tangent plane (output_type option).\n"
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
    declareOption(ol, "smart_initialization",&TangentLearner::smart_initialization,OptionBase::buildoption,
                  "Use of Smart Initialization");
   
    declareOption(ol, "initialization_regularization",&TangentLearner::initialization_regularization,OptionBase::buildoption,
                  "initialization_regularization");
  
    declareOption(ol, "use_subspace_distance", &TangentLearner::use_subspace_distance, OptionBase::buildoption,
                  "Minimize distance between subspace spanned by f_i and by (x-neighbors), instead of between\n"
                  "the individual targets t_j and the subspace spanned by the f_i.\n");

    declareOption(ol, "normalize_by_neighbor_distance", &TangentLearner::normalize_by_neighbor_distance, 
                  OptionBase::buildoption, "Whether to normalize cost by distance of neighbor.\n");

    declareOption(ol, "ordered_vectors", &TangentLearner::ordered_vectors,
                  OptionBase::buildoption, "Whether to apply a differential cost to each f_i so as to\n"
                  "obtain an ordering similar to the one obtained with principal component analysis.\n");

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
                  "   embedding_neural_network: prediction[k,i] = d(e[k]/d(x[i), where e(x) is an ordinary neural\n"
                  "                             network representing the embedding function (see output_type option)\n"
                  "   slack_embedding_neural_network: like embedding_neural_network but outside V is replaced by\n"
                  "                                   a call to no_bprop(V,V_slack), i.e. the gradient to it can\n"
                  "                                   reduced (0<V_slack<1) or eliminated (V_slack=1).\n"
                  "   embedding_quadratic: prediction[k,i] = d(e_k/d(x_i) = A_k x + b_k, where e_k(x) is a quadratic\n"
                  "                        form in x, i.e. e_k = x' A_k x + b_k' x\n"
                  "   (empty string):  specify explicitly the function with tangent_predictor option\n"
                  "where (b,W,c,V) are parameters to be optimized.\n"
        );

    declareOption(ol, "V_slack", &TangentLearner::V_slack, OptionBase::buildoption,
                  "Coefficient that multiplies gradient on outside V when architecture_type=='slack_embedding_neural_network'\n"
        );

    declareOption(ol, "n_hidden_units", &TangentLearner::n_hidden_units, OptionBase::buildoption,
                  "Number of hidden units (if architecture_type is some kidn of neural network)\n"
        );

    declareOption(ol, "output_type", &TangentLearner::output_type, OptionBase::buildoption,
                  "Default value (the only one considered if architecture_type != embedding_*) is\n"
                  "   tangent_plane: output the predicted tangent plane.\n"
                  "   embedding: output the embedding vector (only if architecture_type == embedding_*).\n"
                  "   tangent_plane+embedding: output both (in this order).\n"
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
            output_f = tangent_predictor;
        }
        else if (architecture_type == "linear")
        {
            Var x(n);
            b = Var(n_dim*n,1,"b");
            W = Var(n_dim*n,n,"W");
            tangent_predictor = Func(x, b & W, b + product(W,x));
            output_f = tangent_predictor;
        }
        else if (architecture_type == "embedding_neural_network")
        {
            if (n_hidden_units <= 0)
                PLERROR("TangentLearner::Number of hidden units should be positive, now %d\n",n_hidden_units);
            Var x(n);
            W = Var(n_dim,n_hidden_units,"W");
            c = Var(n_hidden_units,1,"c");
            V = Var(n_hidden_units,n,"V");
            b = Var(n_dim,n,"b");
            Var a = tanh(c + product(V,x));
            Var tangent_plane = diagonalized_factors_product(W,1-a*a,V);
            tangent_predictor = Func(x, W & c & V, tangent_plane);
            embedding = product(W,a);
            if (output_type=="tangent_plane")
                output_f = tangent_predictor;
            else if (output_type=="embedding")
                output_f = Func(x, embedding);
            else if (output_type=="tangent_plane+embedding")
                output_f = Func(x, tangent_plane & embedding);
        }
        else if (architecture_type == "slack_embedding_neural_network")
        {
            if (n_hidden_units <= 0)
                PLERROR("TangentLearner::Number of hidden units should be positive, now %d\n",n_hidden_units);
            Var x(n);
            W = Var(n_dim,n_hidden_units,"W");
            c = Var(n_hidden_units,1,"c");
            V = Var(n_hidden_units,n,"V");
            b = Var(n_dim,n,"b");
            Var a = tanh(c + product(V,x));
            Var tangent_plane = diagonalized_factors_product(W,1-a*a,no_bprop(V,V_slack));
            tangent_predictor = Func(x, W & c & V, tangent_plane);
            embedding = product(W,a);
            if (output_type=="tangent_plane")
                output_f = tangent_predictor;
            else if (output_type=="embedding")
                output_f = Func(x, embedding);
            else if (output_type=="tangent_plane+embedding")
                output_f = Func(x, tangent_plane & embedding);
        }
        else if (architecture_type == "embedding_quadratic")
        {
            Var x(n);
            b = Var(n_dim,n,"b");
            W = Var(n_dim*n,n,"W");
            Var Wx = product(W,x);
            Var tangent_plane = Wx + b;
            tangent_predictor = Func(x, W & b, tangent_plane);
            embedding = product(new PlusVariable(b,Wx),x);
            if (output_type=="tangent_plane")
                output_f = tangent_predictor;
            else if (output_type=="embedding")
                output_f = Func(x, embedding);
            else if (output_type=="tangent_plane+embedding")
                output_f = Func(x, tangent_plane & embedding);
        }
        else if (architecture_type != "")
            PLERROR("TangentLearner::build, unknown architecture_type option %s (should be 'neural_network', 'linear', or empty string '')\n",
                    architecture_type.c_str());

        if (parameters.size()>0 && parameters.nelems() == tangent_predictor->parameters.nelems())
            tangent_predictor->parameters.copyValuesFrom(parameters);
        parameters.resize(tangent_predictor->parameters.size());
        for (int i=0;i<parameters.size();i++)
            parameters[i] = tangent_predictor->parameters[i];
    
        if (training_targets=="local_evectors")
            tangent_targets = Var(n_dim,n);
        else if (training_targets=="local_neighbors")
            tangent_targets = Var(n_neighbors,n);
        else PLERROR("TangentLearner::build, option training_targets is %s, should be 'local_evectors' or 'local_neighbors'.",
                     training_targets.c_str());

        Var proj_err = projection_error(tangent_predictor->outputs[0], tangent_targets, norm_penalization, n, 
                                        normalize_by_neighbor_distance, use_subspace_distance, svd_threshold, 
                                        projection_error_regularization, ordered_vectors);
        projection_error_f = Func(tangent_predictor->outputs[0] & tangent_targets, proj_err);
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

void TangentLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
    return output_f->outputsize;
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
        //cout << targets_vmat;
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
    if(report_progress>0)
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
        if (smart_initialization)
        {
            V->matValue<<smartInitialization(train_set,n_hidden_units,smart_initialization,initialization_regularization);
            W->value<<(1/real(n_hidden_units));
            b->matValue.clear();
            c->matValue.clear();
        }
        else
        {
            real delta = 1.0 / sqrt(real(inputsize()));
            fill_random_uniform(V->value, -delta, delta);
            delta = 1.0 / real(n_hidden_units);
            fill_random_uniform(W->matValue, -delta, delta);
            c->matValue.clear();
            //fill_random_uniform(c->matValue,-3,3);
            //b->matValue.clear();
        }
    }
    else if (architecture_type=="linear")
    {
        real delta = 1.0 / sqrt(real(inputsize()));
        b->matValue.clear();
        fill_random_uniform(W->matValue, -delta, delta);
    }
    else if (architecture_type=="embedding_neural_network")
    {
        real delta = 1.0 / sqrt(real(inputsize()));
        fill_random_uniform(V->value, -delta, delta);
        delta = 1.0 / real(n_hidden_units);
        fill_random_uniform(W->matValue, -delta, delta);
        c->value.clear();
        b->value.clear();
    }
    else if (architecture_type=="embedding_quadratic")
    {
        real delta = 1.0 / sqrt(real(inputsize()));
        fill_random_uniform(W->matValue, -delta, delta);
        b->value.clear();
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
    output << output_f(input);
}    

void TangentLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
					     const Vec& target, Vec& costs) const
{
    PLERROR("TangentLearner::computeCostsFromOutputs not defined for this learner");
}                                

TVec<string> TangentLearner::getTestCostNames() const
{
    return getTrainCostNames();
}

TVec<string> TangentLearner::getTrainCostNames() const
{
    TVec<string> cost(1); cost[0] = "projection_error";
    return cost;
}



} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
