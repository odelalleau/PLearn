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
 * $Id$
 ******************************************************* */

// Authors: Yoshua Bengio & Martin Monperrus

/*! \file NonLocalManifoldParzen.cc */


#include "NonLocalManifoldParzen.h"
#include <plearn/display/DisplayUtils.h>
#include <plearn/vmat/LocalNeighborsDifferencesVMatrix.h>
//#include <plearn/vmat/RandomNeighborsDifferencesVMatrix.h>
#include <plearn/var/ProductVariable.h>
#include <plearn/var/PlusVariable.h>
#include <plearn/var/SoftplusVariable.h>
#include <plearn/var/SumAbsVariable.h>
#include <plearn/var/SumSquareVariable.h>
#include <plearn/var/VarRowVariable.h>
#include <plearn/var/SourceVariable.h>
#include <plearn/var/Var_operators.h>
//#include <plearn/var/DiagonalGaussianVariable.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/RowOfVariable.h>
//#include <plearn/var/RowPowNormVariable.h>
#include <plearn/var/SumVariable.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/NllGeneralGaussianVariable.h>
#include <plearn/var/DiagonalizedFactorsProductVariable.h>
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
#include <plearn/var/NoBpropVariable.h>
#include <plearn/var/ReshapeVariable.h>
#include <plearn/var/SquareVariable.h>
#include <plearn/var/ExpVariable.h>
#include <plearn/io/load_and_save.h>
#include <plearn/vmat/VMat_computeNearestNeighbors.h>
#include <plearn/vmat/FractionSplitter.h>
#include <plearn/vmat/RepeatSplitter.h>
#include <plearn/var/FNetLayerVariable.h>
#include <plearn/vmat/MemoryVMatrix.h>

namespace PLearn {
using namespace std;


NonLocalManifoldParzen::NonLocalManifoldParzen() 
    :  //weight_embedding(1),
    curpos(0),
    weight_decay(0), penalty_type("L2_square"),
//noise_grad_factor(0.01),noise(0), noise_type("gaussian"), omit_last(0),
learn_mu(true), 
//magnified_version(false), 
reference_set(0), sigma_init(0.1), sigma_min(0.00001), nneighbors(5), nneighbors_density(-1), mu_nneighbors(2), ncomponents(1), sigma_threshold_factor(1), variances_transfer_function("softplus"), architecture_type("single_neural_network"),
    n_hidden_units(-1), batch_size(1), svd_threshold(1e-8), rw_n_step(1000), rw_size_step(0.01), rw_ith_component(0), rw_file_name("random_walk_"), rw_save_every(100), store_prediction(false), optstage_per_lstage(-1), save_every(-1)
{
}

PLEARN_IMPLEMENT_OBJECT(NonLocalManifoldParzen, "Non-Local version of Manifold Parzen Windows",
                        "Manifold Parzen Windows density model, where the parameters of\n"
                        "the gaussians in the mixture are predicted by a neural network."
    );


void NonLocalManifoldParzen::declareOptions(OptionList& ol)
{

//  declareOption(ol, "weight_embedding", &NonLocalManifoldParzen::weight_embedding, OptionBase::buildoption, 
//                "Embedding penalty weight\n");

    declareOption(ol, "weight_decay", &NonLocalManifoldParzen::weight_decay, OptionBase::buildoption, 
                  "Global weight decay for all layers\n");

    declareOption(ol, "penalty_type", &NonLocalManifoldParzen::penalty_type,
                  OptionBase::buildoption,
                  "Penalty to use on the weights (for weight and bias decay).\n"
                  "Can be any of:\n"
                  "  - \"L1\": L1 norm,\n"
                  "  - \"L1_square\": square of the L1 norm,\n"
                  "  - \"L2_square\" (default): square of the L2 norm.\n");

//  declareOption(ol, "omit_last", &NonLocalManifoldParzen::omit_last, OptionBase::buildoption,
//		"Number of training examples at the end of trainin set to ignore in the training.\n"
//		);

    declareOption(ol, "learn_mu", &NonLocalManifoldParzen::learn_mu, OptionBase::buildoption,
                  "Indication that mu should be learned.\n"
        );

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

    declareOption(ol, "hidden_layer", &NonLocalManifoldParzen::hidden_layer, OptionBase::buildoption,
                  "A user-specified NAry Var that computes the output of the first hidden layer\n"
                  "from the network input vector and a set of parameters. Its first argument should\n"
                  "be the network input and the remaining arguments the tunable parameters.\n");

    declareOption(ol, "batch_size", &NonLocalManifoldParzen::batch_size, OptionBase::buildoption, 
                  "    how many samples to use to estimate the average gradient before updating the weights\n"
                  "    0 is equivalent to specifying training_set->length() \n");

    declareOption(ol, "svd_threshold", &NonLocalManifoldParzen::svd_threshold, OptionBase::buildoption,
                  "Threshold to accept singular values of F in solving for linear combination weights on tangent subspace.\n"
        );

    declareOption(ol, "parameters", &NonLocalManifoldParzen::parameters, OptionBase::learntoption,
                  "Parameters of the tangent_predictor function.\n"
        );

    declareOption(ol, "shared_parameters", &NonLocalManifoldParzen::shared_parameters, OptionBase::buildoption,
                  "Parameters of another NonLocalManifoldParzen estimator to share with this current object.\n"
        );

    declareOption(ol, "L", &NonLocalManifoldParzen::L, OptionBase::learntoption,
                  "Number of gaussians.\n"
        );
    
//  declareOption(ol, "Us", &NonLocalManifoldParzen::Us, OptionBase::learntoption,
//		"The U matrices for the reference set.\n"
//		);
    
    declareOption(ol, "mus", &NonLocalManifoldParzen::mus, OptionBase::learntoption,
                  "The mu vectors for the reference set.\n"
        );
    
//  declareOption(ol, "sms", &NonLocalManifoldParzen::sms, OptionBase::learntoption,
//		"The sm values for the reference set.\n"
//                );
    
    declareOption(ol, "sns", &NonLocalManifoldParzen::sns, OptionBase::learntoption,
                  "The sn values for the reference set.\n"
        );
    
    declareOption(ol, "sms", &NonLocalManifoldParzen::sms, OptionBase::learntoption,
                  "The sm values for the reference set.\n"
        );
    
    declareOption(ol, "Fs", &NonLocalManifoldParzen::Fs, OptionBase::learntoption,
                  "The F values for the reference set.\n"
        );

    declareOption(ol, "sigma_min", &NonLocalManifoldParzen::sigma_min, OptionBase::buildoption,
                  "The minimum value for sigma noise.\n"
        );

    declareOption(ol, "sigma_init", &NonLocalManifoldParzen::sigma_init, OptionBase::buildoption,
                  "Initial minimum value for sigma noise.\n"
        );

    declareOption(ol, "rw_n_step", &NonLocalManifoldParzen::rw_n_step, OptionBase::buildoption,
                  "Number of steps in the random walk (for compute output).\n"
        );

    declareOption(ol, "rw_size_step", &NonLocalManifoldParzen::rw_size_step, OptionBase::buildoption,
                  "Size of the steps in the random walk (for compute output).\n"
        );

    declareOption(ol, "rw_ith_component", &NonLocalManifoldParzen::rw_ith_component, OptionBase::buildoption,
                  "Which principal component to follow.\n"
        );

    declareOption(ol, "rw_save_every", &NonLocalManifoldParzen::rw_save_every, OptionBase::buildoption,
                  "Number of iterations between savings of random walk results.\n"
        );
    declareOption(ol, "rw_file_name", &NonLocalManifoldParzen::rw_file_name, OptionBase::buildoption,
                  "File name for the random walk saves.\n"
        );

    declareOption(ol, "store_prediction", &NonLocalManifoldParzen::store_prediction, OptionBase::buildoption,
                  "Indication that the predicted parameters should be stored.\n"
                  "This may make testing faster. Note that the predictions are\n"
                  "stored after the last training stage, so if the predictor is\n"
                  "modified later on (e.g. if the parameters of the predictor are shared), then\n"
                  "this option might give different testing results.\n"
        );

    declareOption(ol, "optstage_per_lstage", &NonLocalManifoldParzen::optstage_per_lstage, OptionBase::buildoption,
                  "Number of optimizer stages. If < 0, then it is determined as a function of\n"
                  "the training set length and of the batch size.\n"
        );

    declareOption(ol, "save_every", &NonLocalManifoldParzen::save_every, OptionBase::buildoption,
                  "Number of iterations since the last save after which the parameters must be saved.\n"
                  "If < 0, then the parameters are saved at the end of train().\n"
        );


//  declareOption(ol, "noise", &NonLocalManifoldParzen::noise, OptionBase::buildoption,
//		"Noise parameter for the training data. For uniform noise, this gives the half the length \n" "of the uniform window (centered around the origin), and for gaussian noise, this gives the variance of the noise in all directions.\n"
//                );

//  declareOption(ol, "noise_type", &NonLocalManifoldParzen::noise_type, OptionBase::buildoption,
//		"Type of the noise (\"uniform\" or \"gaussian\").\n"
//                );
  
//  declareOption(ol, "noise_grad_factor", &NonLocalManifoldParzen::noise_grad_factor, OptionBase::buildoption,
//		"Gradient factor to apply to the noise signal error.\n"
//                );
  
//  declareOption(ol, "magnified_version", &NonLocalManifoldParzen::magnified_version, OptionBase::buildoption,
//		"Indication that, when computing the log density, the magnified estimation should be used.\n"
//                );

    declareOption(ol, "reference_set", &NonLocalManifoldParzen::reference_set, OptionBase::learntoption,
                  "Reference points for density computation.\n"
        );

    declareOption(ol, "curpos", &NonLocalManifoldParzen::curpos, OptionBase::learntoption,
                  "Position of the current example in the training set.\n"
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

        int sp_index = 0;

        Var log_n_examples(1,1,"log(n_examples)");
        if(train_set)
        {
            L = train_set->length();
            reference_set = train_set; // Maybe things could be changed here to make access faster!
        }

        log_L= pl_log((real) L);

        {
      
            x = Var(n);
            Var a; // outputs of hidden layer

            if (hidden_layer) // user-specified hidden layer Var
            {
                if(shared_parameters.size() != 0)
                    PLERROR("In NonLocalManifoldParzen:build_(): shared parameters is not implemented for user-specified hidden layer Var");
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
                a = transpose(layer_var);
                n_hidden_units = layer_var->width();
            }
            else // standard hidden layer
            {
                if (n_hidden_units <= 0)
                    PLERROR("NonLocalManifoldParzen::Number of hidden units should be positive, now %d\n",n_hidden_units);
                if(shared_parameters.size() != 0)
                {
                    c = shared_parameters[sp_index++];
                    V = shared_parameters[sp_index++];
                }
                else
                {
                    c = Var(n_hidden_units,1,"c ");
                    V = Var(n_hidden_units,n,"V ");               
                }
                params.append(c);
                params.append(V);
                a = tanh(c + product(V,x));
            }

            if(shared_parameters != 0)
            {
                muV = shared_parameters[sp_index++];
                snV = shared_parameters[sp_index++];
                snb = shared_parameters[sp_index++];
            }
            else
            {
                muV = Var(n,n_hidden_units,"muV "); 
                snV = Var(1,n_hidden_units,"snV ");  
                snb = Var(1,1,"snB ");      
            }
            params.append(muV);
            params.append(snV);
            params.append(snb);

            if(architecture_type == "embedding_neural_network")
            {
                if(shared_parameters.size() != 0)
                    W = shared_parameters[sp_index++];
                else
                    W = Var(ncomponents,n_hidden_units,"W ");       
                tangent_plane = diagonalized_factors_product(W,1-a*a,V); 
                embedding = product(W,a);
                output_embedding = Func(x,embedding);
                params.append(W);
            } 
            else if(architecture_type == "single_neural_network")
            {
                if(shared_parameters.size() != 0)
                {
                    b = shared_parameters[sp_index++];
                    W = shared_parameters[sp_index++];
                }
                else
                {
                    b = Var(ncomponents*n,1,"b");
                    W = Var(ncomponents*n,n_hidden_units,"W ");
                }
                tangent_plane = reshape(b + product(W,a),ncomponents,n);
                params.append(b);
                params.append(W);
            }
            else
                PLERROR("NonLocalManifoldParzen::build_, unknown architecture_type option %s",
                        architecture_type.c_str());
            if(learn_mu)
                mu = product(muV,a); 
            else
            {
                mu = new SourceVariable(n,1);
                mu->value.fill(0);
                mu_nneighbors = 0;
            }
            min_sig = new SourceVariable(1,1);
            min_sig->value[0] = sigma_min;
            min_sig->setName("min_sig");
            if(shared_parameters.size() != 0)
                init_sig = shared_parameters[sp_index++];
            else
            {
                init_sig = Var(1,1);
                init_sig->setName("init_sig");
            }
            params.append(init_sig);
            
            if(variances_transfer_function == "softplus") sn = softplus(snb + product(snV,a))  + min_sig + softplus(init_sig);
            else if(variances_transfer_function == "square") sn = square(snb + product(snV,a)) + min_sig + square(init_sig);
            else if(variances_transfer_function == "exp") sn = exp(snb + product(snV,a)) + min_sig + exp(init_sig);
            else PLERROR("In NonLocalManifoldParzen::build_ : unknown variances_transfer_function option %s ", variances_transfer_function.c_str());
      


            if(sigma_threshold_factor > 0)
            {        
                sn = threshold_bprop(sn,sigma_threshold_factor);
            }

            /*
              if(noise > 0)
              {
              if(noise_type == "uniform")
              {
              PP<UniformDistribution> temp = new UniformDistribution();
              Vec lower_noise(n);
              Vec upper_noise(n);
              for(int i=0; i<n; i++)
              {
              lower_noise[i] = -1*noise;
              upper_noise[i] = noise;
              }
              temp->min = lower_noise;
              temp->max = upper_noise;
              dist = temp;
              }
              else if(noise_type == "gaussian")
              {
              PP<GaussianDistribution> temp = new GaussianDistribution();
              Vec mu(n); mu.clear();
              Vec eig_values(n); 
              Mat eig_vectors(n,n); eig_vectors.clear();
              for(int i=0; i<n; i++)
              {
              eig_values[i] = noise; // maybe should be adjusted to the sigma noiseat the input
              eig_vectors(i,i) = 1.0;
              }
              temp->mu = mu;
              temp->eigenvalues = eig_values;
              temp->eigenvectors = eig_vectors;
              dist = temp;
              }
              else PLERROR("In GaussianContinuumDistribution::build_() : noise_type %c not defined",noise_type.c_str());
              noise_var = new PDistributionVariable(x,dist);
              for(int k=0; k<ncomponents; k++)
              {
              Var index_var = new SourceVariable(1,1);
              index_var->value[0] = k;
              Var f_k = new VarRowVariable(tangent_plane,index_var);
              noise_var = noise_var - product(f_k,noise_var)* transpose(f_k)/pownorm(f_k,2);
              }
        
              noise_var = no_bprop(noise_var);
              noise_var->setName(noise_type);
              }
              else
              {
      
              noise_var = new SourceVariable(n,1);
              noise_var->setName("no noise");
              for(int i=0; i<n; i++)
              noise_var->value[i] = 0;
              }
            */

            // Path for noisy mu
            //Var a_noisy = tanh(c + product(V,x+noise_var));
            //mu_noisy = no_bprop(product(muV,a_noisy),noise_grad_factor); 


            tangent_plane->setName("tangent_plane ");
            mu->setName("mu ");
            sn->setName("sn ");
            a->setName("a ");
            if(architecture_type == "embedding_neural_network")
                embedding->setName("embedding ");
            x->setName("x ");

            predictor = Func(x, params , tangent_plane & mu & sn );
        }

        if(shared_parameters.size() == 0)
        {
            if (parameters.size()>0 && parameters.nelems() == predictor->parameters.nelems())
                predictor->parameters.copyValuesFrom(parameters);
            parameters.resize(predictor->parameters.size());
            for (int i=0;i<parameters.size();i++)
                parameters[i] = predictor->parameters[i];
        }
        Var target_index = Var(1,1);
        target_index->setName("target_index");
        Var neighbor_indexes = Var(nneighbors,1);
        neighbor_indexes->setName("neighbor_indexes");
        Var random_index = Var(1,1);
        random_index->setName("neighbor_index");
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
        Var nll;
        //if(noise <= 0)
        nll = nll_general_gaussian(tangent_plane, mu, sn, tangent_targets, log_L, mu_nneighbors,0,0); // + log_n_examples;
        //else
        //nll = nll_general_gaussian(tangent_plane, mu, sn, tangent_targets, log_L, mu_nneighbors,noise_var,mu_noisy); // + log_n_examples;

        Var knn = new SourceVariable(1,1);
        knn->setName("knn");
        knn->value[0] = nneighbors;
        sum_nll = new ColumnSumVariable(nll) / knn;
        /*
          if(architecture_type == "embedding_neural_network")
          {
          // Notes: - seulement prendre le plus proche voisin d'un voisin random
          //        - il va peut-être falloir utiliser des fonctions de distances différentes
          //        - peut-être utiliser les directions principales apprises!
          //        - peut-être utiliser les distances dans l'espace initial pour pondérer!
          //        - il va peut-être falloir mettre un poids différent sur ce nouveau coût
          //        - utiliser ici une VarRowsVariable(...)
          //        - question: est-ce que je devrais faire une bprop partout, juste sur embedding
          //          juste sur neighbor et random, ... ?
      
          //Var nearest_emb = product(W, tanh(c + product(V,rowOf(reference_set,neighbor_indexes))));
          Var random_emb = product(W, tanh(c + product(V,rowOf(reference_set,random_index))));
      
          //Var nearest_emb_diff = nearest_emb - embedding;
          //Var random_emb_diff = random_emb - embedding;
          //sum_nll += weight_embedding * (sum(square(nearest_emb_diff)) - sum(square(random_emb_diff)));
          sum_nll += weight_embedding * diagonal_gaussian(random_emb,embedding,no_bprop(rowPowNorm(tangent_plane,2)));
          }
        */
        if(weight_decay > 0 )
        {
            if(penalty_type == "L1_square") sum_nll += (square(sumabs(W))+ square(sumabs(V)) + square(sumabs(muV)) + square(sumabs(snV)))*weight_decay;
            else if(penalty_type == "L1") sum_nll += (sumabs(W)+ sumabs(V) + sumabs(muV) + sumabs(snV))*weight_decay;
            else if(penalty_type == "L2_square") sum_nll += (sumsquare(W)+ sumsquare(V) + sumsquare(muV) + sumsquare(snV))*weight_decay;
            else PLERROR("In NonLocalManifoldParzen::build_(): penalty_type %s not recognized", penalty_type.c_str());
        }
        /*
          if(architecture_type == "embedding_neural_network")
          {
          Var random_diff = Var(n,1);
          cost_of_one_example = Func(x & tangent_targets & target_index & neighbor_indexes & random_diff & random_index, predictor->parameters, sum_nll);
          }
          else
        */
        cost_of_one_example = Func(x & tangent_targets & target_index & neighbor_indexes, predictor->parameters, sum_nll);

        if(nneighbors_density >= L || nneighbors_density < 0) nneighbors_density = L;

        t_row.resize(n);
        Ut_svd.resize(n,n);
        V_svd.resize(ncomponents,ncomponents);
        F.resize(tangent_plane->length(),tangent_plane->width());
        z.resize(n);
        x_minus_neighbor.resize(n);
        neighbor_row.resize(n);
        // log_density and Kernel methods
        U_temp.resize(ncomponents,n);
        mu_temp.resize(n);
        sm_temp.resize(ncomponents);
        sn_temp.resize(1);
        diff.resize(n);

        mus.resize(L, n);
        sns.resize(L);
        sms.resize(L,ncomponents);
        Fs.resize(L);
        for(int i=0; i<L; i++)
        {
            Fs[i].resize(ncomponents,n);
        }
    }

}

/*
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
  sms(t,k) = mypow(S_svd[k],2);
  Us[t](k) << Ut_svd(k);
  }    
  }

  }
*/

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
 varDeepCopyField(x, copies);
 varDeepCopyField(b, copies);
 varDeepCopyField(W, copies);
 varDeepCopyField(c, copies);
 varDeepCopyField(V, copies);
 varDeepCopyField(muV, copies);
 varDeepCopyField(snV, copies);
 varDeepCopyField(snb, copies);
 varDeepCopyField(tangent_targets, copies);
 varDeepCopyField(tangent_plane, copies);
 varDeepCopyField(mu, copies);
 varDeepCopyField(sn, copies);
 varDeepCopyField(sum_nll, copies);
 varDeepCopyField(min_sig, copies);
 varDeepCopyField(init_sig, copies);
 varDeepCopyField(embedding, copies);
 deepCopyField(output_embedding, copies);
 deepCopyField(predictor, copies);

 deepCopyField(U_temp,copies);
 deepCopyField(F, copies);
 deepCopyField(distances,copies);
 deepCopyField(mu_temp,copies);
 deepCopyField(sm_temp,copies);
 deepCopyField(sn_temp,copies);
 deepCopyField(diff,copies);
 deepCopyField(z,copies);
 deepCopyField(x_minus_neighbor,copies);
 deepCopyField(t_row,copies);
 deepCopyField(neighbor_row,copies);
 deepCopyField(log_gauss,copies);
 deepCopyField(t_dist,copies);
 deepCopyField(t_nn,copies);
 deepCopyField(Ut_svd, copies);
 deepCopyField(V_svd, copies);
 deepCopyField(S_svd, copies);

 deepCopyField(mus, copies);
 deepCopyField(sns, copies);
 deepCopyField(sms, copies);
 deepCopyField(Fs, copies);

 deepCopyField(parameters, copies);
 deepCopyField(shared_parameters, copies);

 deepCopyField(reference_set,copies);
 varDeepCopyField(hidden_layer, copies);
 deepCopyField(optimizer, copies);

}


void NonLocalManifoldParzen::forget()
{
    if (train_set) initializeParams();
    stage = 0;
}
    
void NonLocalManifoldParzen::train()
{
    // Check whether gradient descent is going to be done
    // If not, then we don't need to store the parameters,
    // except for sn...
    bool flag = (nstages == stage);

    if(store_prediction && flag) 
    {
        for(int i=0; i<L; i++)
        {
            sns[i] += sigma_min - min_sig->value[0];
        }
    }

    // Update sigma_min, in case it was changed,
    // e.g. using an HyperLearner
    min_sig->value[0] = sigma_min;

    // Set train_stats if not already done.
    if (!train_stats)
        train_stats = new VecStatsCollector();
    /*
    VMat train_set_with_targets;
    VMat targets_vmat;
    Var totalcost;
    int nsamples;
    */
    if (!cost_of_one_example)
        PLERROR("NonLocalManifoldParzen::train: build has not been run after setTrainingSet!");
    /*
      if(stage==0)
      {
      train_set = new SubVMatrix(train_set,0,0,train_set.length()-omit_last,train_set.width());
      }
    */
    /*
      if(architecture_type == "embedding_neural_network")
      targets_vmat = hconcat(local_neighbors_differences(train_set, nneighbors, false, true),random_neighbors_differences(train_set,1,false,true));
      else*/
  
    if(stage == 0)
    {
        targets_vmat = local_neighbors_differences(train_set, nneighbors, false, true);
        
        train_set_with_targets = hconcat(train_set, targets_vmat);
        train_set_with_targets->defineSizes(inputsize()+ inputsize()*nneighbors+1+nneighbors /*+ (architecture_type == "embedding_neural_network" ? inputsize()+1:0)*/,0);
        nsamples = batch_size>0 ? batch_size : train_set->length();
        
        totalcost = meanOf(train_set_with_targets, cost_of_one_example, nsamples);

        if(optimizer)
        {
            if(shared_parameters.size()!=0)
                optimizer->setToOptimize(shared_parameters, totalcost);  
            else
                optimizer->setToOptimize(parameters, totalcost);  
            optimizer->build();
        }
        else PLERROR("NonLocalManifoldParzen::train can't train without setting an optimizer first!");
    }

    dynamic_cast<SumOfVariable*>( (Variable*) totalcost)->curpos = curpos;
  
    // number of optimizer stages corresponding to one learner stage (one epoch)
    if(optstage_per_lstage < 0) optstage_per_lstage = train_set->length()/nsamples;

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
        if(stage%save_every == 0 && store_prediction && !flag)
        {
            for(int t=0; t<L;t++)
            {
                reference_set->getRow(t,neighbor_row);
                predictor->fprop(neighbor_row, F.toVec() & mus(t) & sns.subVec(t,1));                   
                // N.B. this is the SVD of F'
                lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
                for (int k=0;k<ncomponents;k++)
                {
                    sms(t,k) = mypow(S_svd[k],2);                
                    Fs[t](k) << Ut_svd(k);
                }    
                
            }
        }

        if(pb)
            pb->update(stage-initial_stage);
      
    }
    if(verbosity>1)
        cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;

    if(pb)
        delete pb;

    if(save_every < 0 && store_prediction && !flag)
    {
        for(int t=0; t<L;t++)
        {
            reference_set->getRow(t,neighbor_row);
            predictor->fprop(neighbor_row, F.toVec() & mus(t) & sns.subVec(t,1));                   
            // N.B. this is the SVD of F'
            lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
            for (int k=0;k<ncomponents;k++)
            {
                sms(t,k) = mypow(S_svd[k],2);                
                Fs[t](k) << Ut_svd(k);
            }    

        }
    }
    curpos = dynamic_cast<SumOfVariable*>( (Variable*) totalcost)->curpos;
}

//////////////////////
// initializeParams //
//////////////////////
void NonLocalManifoldParzen::initializeParams()
{
    resetGenerator(seed_);

    if (architecture_type=="embedding_neural_network")
    {
        real delta = 1.0 / sqrt(real(inputsize()));
        random_gen->fill_random_uniform(V->value, -delta, delta);
        delta = 1.0 / real(n_hidden_units);
        random_gen->fill_random_uniform(W->matValue, -delta, delta);
        c->value.clear();
        snb->value.clear();
        random_gen->fill_random_uniform(snV->matValue, -delta, delta);
        random_gen->fill_random_uniform(muV->matValue, -delta, delta);
        //min_sig->value[0] = sigma_init;
        //min_d->value.fill(diff_init);
        if(variances_transfer_function == "softplus") {
            init_sig->value[0] = pl_log(exp(sigma_init)-1); }
        else if(variances_transfer_function == "square") { init_sig->value[0] = sqrt(sigma_init);}
        else if(variances_transfer_function == "exp") {
            init_sig->value[0] = pl_log(sigma_init); }
    }
    else if (architecture_type=="single_neural_network")
    {
        real delta = 1.0 / sqrt(real(inputsize()));
        if (!hidden_layer)
           random_gen->fill_random_uniform(V->value, -delta, delta);
        delta = 1.0 / real(n_hidden_units);
        random_gen->fill_random_uniform(W->matValue, -delta, delta);
        if (!hidden_layer) c->value.clear();
        snb->value.clear();
        random_gen->fill_random_uniform(snV->matValue, -delta, delta);
        random_gen->fill_random_uniform(muV->matValue, -delta, delta);
        b->value.clear();
        //min_sig->value[0] = sigma_init;
        //min_d->value.fill(diff_init);
        if(variances_transfer_function == "softplus") {
            init_sig->value[0] = pl_log(exp(sigma_init)-1); }
        else if(variances_transfer_function == "square") { init_sig->value[0] = sqrt(sigma_init);}
        else if(variances_transfer_function == "exp") {
            init_sig->value[0] = pl_log(sigma_init);}
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
    real ret = 0;
    t_row << x;
    real mahal = 0;
    real norm_term = 0;

    // Update sigma_min, in case it was changed,
    // e.g. using an HyperLearner
    
    if(store_prediction && min_sig->value[0] != sigma_min) 
    {
        for(int i=0; i<L; i++)
        {
            sns[i] += sigma_min - min_sig->value[0];
        }
    }


    min_sig->value[0] = sigma_min;
  
/*
  if(magnified_version)
  {
  predictor->fprop(x, F.toVec() & mu_temp & sn_temp);
    
  // N.B. this is the SVD of F'
  lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
  for (int k=0;k<ncomponents;k++)
  {
  sm_temp[k] = mypow(S_svd[k],2);
  F(k) << Ut_svd(k);
  }    
    
  mahal = -0.5*pownorm(mu_temp)/sn_temp[0];      
  norm_term = - n/2.0 * Log2Pi - 0.5*(n-ncomponents)*log(sn_temp[0]);
  for(int k=0; k<ncomponents; k++)
  {       
  mahal -= square(dot(F(k), mu_temp))*(0.5/(sm_temp[k]+sn_temp[0]) - 0.5/sn_temp[0]); 
  norm_term -= 0.5*log(sm_temp[k]+sn_temp[0]);
  }
    
  ret = mahal + norm_term + log((real)nneighbors) - log((real)L);
  }
  else
  {*/
    if(nneighbors_density != L)
    {
        // Fetching nearest neighbors for density estimation.
        knn(reference_set,x,nneighbors_density,t_nn,bool(0));
        log_gauss.resize(t_nn.length());
        for(int neighbor=0; neighbor<t_nn.length(); neighbor++)
        {
            reference_set->getRow(t_nn[neighbor],neighbor_row);
            if(!store_prediction)
            {                
                predictor->fprop(neighbor_row, F.toVec() & mu_temp & sn_temp);        
                // N.B. this is the SVD of F'
                lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
                for (int k=0;k<ncomponents;k++)
                {
                    sm_temp[k] = mypow(S_svd[k],2);
                    U_temp(k) << Ut_svd(k);
                }    
            }
            else
            {
                mu_temp << mus(t_nn[neighbor]);
                sn_temp[0] = sns[t_nn[neighbor]];
                sm_temp << sms(t_nn[neighbor]);
                U_temp << Fs[t_nn[neighbor]];
            }
            substract(t_row,neighbor_row,x_minus_neighbor);
            //substract(x_minus_neighbor,mus(t_nn[neighbor]),z);
            substract(x_minus_neighbor,mu_temp,z);

            //mahal = -0.5*pownorm(z)/sns[t_nn[neighbor]];      
            //norm_term = - n/2.0 * Log2Pi - log_L - 0.5*(n-ncomponents)*log(sns[t_nn[neighbor]]);
        
            mahal = -0.5*pownorm(z)/sn_temp[0];      
            norm_term = - n/2.0 * Log2Pi - log_L - 0.5*(n-ncomponents)*pl_log(sn_temp[0]);
        

            for(int k=0; k<ncomponents; k++)
            {       
                //mahal -= square(dot(z,Us[t_nn[neighbor]](k)))*(0.5/(sms(t_nn[neighbor],k)+sns[t_nn[neighbor]]) - 0.5/sns[t_nn[neighbor]]); // Pourrait être accéléré!
                //norm_term -= 0.5*log(sms(t_nn[neighbor],k)+sns[t_nn[neighbor]]);
                mahal -= square(dot(z,U_temp(k)))*(0.5/(sm_temp[k]+sn_temp[0]) - 0.5/sn_temp[0]); 
                norm_term -= 0.5*pl_log(sm_temp[k]+sn_temp[0]);
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
            if(!store_prediction)
            {
                predictor->fprop(neighbor_row, F.toVec() & mu_temp & sn_temp);        
                
                // N.B. this is the SVD of F'
                lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
                for (int k=0;k<ncomponents;k++)
                {
                    sm_temp[k] = mypow(S_svd[k],2);
                    U_temp(k) << Ut_svd(k);
                }    
            }
            else
            {
                mu_temp << mus(t);
                sn_temp[0] = sns[t];
                sm_temp << sms(t);
                U_temp << Fs[t];
            }

            substract(t_row,neighbor_row,x_minus_neighbor);
            //substract(x_minus_neighbor,mus(t),z);
            substract(x_minus_neighbor,mu_temp,z);

            //mahal = -0.5*pownorm(z)/sns[t];      
            //norm_term = - n/2.0 * Log2Pi - log_L - 0.5*(n-ncomponents)*log(sns[t]);
        
            mahal = -0.5*pownorm(z)/sn_temp[0];      
            norm_term = - n/2.0 * Log2Pi - log_L - 0.5*(n-ncomponents)*pl_log(sn_temp[0]);
        
            for(int k=0; k<ncomponents; k++)
            {
                //mahal -= square(dot(z,Us[t](k)))*(0.5/(sms(t,k)+sns[t]) - 0.5/sns[t]); // Pourrait être accéléré!
                //norm_term -= 0.5*log(sms(t,k)+sns[t]);

                mahal -= square(dot(z,U_temp(k)))*(0.5/(sm_temp[k]+sn_temp[0]) - 0.5/sn_temp[0]); 
                norm_term -= 0.5*pl_log(sm_temp[k]+sn_temp[0]);
            }

            log_gauss[t] = mahal + norm_term;
        }
    }
    ret = logadd(log_gauss);
    //}

    return ret;
}

/*
  Mat NonLocalManifoldParzen::getEigenvectors(int j) const {
  {
  return Us[j];
  }
  
  Vec NonLocalManifoldParzen::getTrainPoint(int j) const {
  Vec ret(reference_set->width());
  reference_set->getRow(j,ret);
  return ret;
  }
*/

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
    case 'r':
    {
        string fsave = "";
        VMat temp;
        real step_size = rw_size_step;
        real dp;        
        t_row << input;
        Vec last_F(inputsize());
        for(int s=0; s<rw_n_step;s++)
        {
            if(s == 0) 
            {
                predictor->fprop(t_row, F.toVec() & mu_temp & sn_temp);
                last_F << F(rw_ith_component);
            }
            predictor->fprop(t_row, F.toVec() & mu_temp & sn_temp);
    
            // N.B. this is the SVD of F'
            lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
            F(rw_ith_component) << Ut_svd(rw_ith_component);
            
            if(s % rw_save_every == 0)
            {
                fsave = rw_file_name + tostring(s) + ".amat";
                temp = new MemoryVMatrix(t_row.toMat(1,t_row.length()));
                temp->saveAMAT(fsave,false,true);
                //PLearn::save(fsave,t_row);
            }
            dp = dot(last_F,F(rw_ith_component));
            if(dp>0) dp = 1;
            else dp = -1;
            t_row += step_size*F(rw_ith_component)*abs(S_svd[rw_ith_component])*dp;
            last_F << dp*F(rw_ith_component);
        }
        output << t_row;

        t_row << input;
        for(int s=0; s<rw_n_step;s++)
        {
            if(s == 0) 
            {
                predictor->fprop(t_row, F.toVec() & mu_temp & sn_temp);
                last_F << (-1.0)*F(rw_ith_component);
            }

            
            predictor->fprop(t_row, F.toVec() & mu_temp & sn_temp);
    
            // N.B. this is the SVD of F'
            lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
            F(rw_ith_component) << Ut_svd(rw_ith_component);
            
            if(s % rw_save_every == 0)
            {
                fsave = rw_file_name + tostring(-s) + ".amat";
                temp = new MemoryVMatrix(t_row.toMat(1,t_row.length()));
                temp->saveAMAT(fsave,false,true);
                //PLearn::save(fsave,t_row);
            }
            dp = dot(last_F,F(rw_ith_component));
            if(dp>0) dp = 1;
            else dp = -1;
            t_row += step_size*F(rw_ith_component)*abs(S_svd[rw_ith_component])*dp;
            last_F << dp*F(rw_ith_component);
        }
        break;
    }
    case 't':
    {
        predictor->fprop(input, F.toVec() & mu_temp & sn_temp);
        output << F.toVec();
        break;
    }
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
    case 'r':
        return n;
    case 't':
        return ncomponents*n;
    default:
        return inherited::outputsize();
    }
}

real NonLocalManifoldParzen::evaluate(Vec x1,Vec x2,real scale)
{
    real ret;

    // Update sigma_min, in case it was changed,
    // e.g. using an HyperLearner
    min_sig->value[0] = sigma_min;

    predictor->fprop(x2, F.toVec() & mu_temp & sn_temp);
    
    // N.B. this is the SVD of F'
    lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
    for (int k=0;k<ncomponents;k++)
    {
        sm_temp[k] = mypow(S_svd[k],2);
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
