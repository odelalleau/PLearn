// -*- C++ -*-

// EntropyContrastLearner.cc
//
// Copyright (C) 2004 Marius Muja 
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
 * $Id: EntropyContrastLearner.cc,v 1.1 2004/09/16 19:50:47 mariusmuja Exp $ 
 ******************************************************* */

// Authors: Marius Muja

/*! \file EntropyContrastLearner.cc */


#include "EntropyContrastLearner.h"
#include "plearn/var/NoBpropVariable.h"
#include "plearn/var/DiagonalizedFactorsProductVariable.h"
#include "plearn/display/DisplayUtils.h"
#include <plearn/var/PDistributionVariable.h>
#include <plearn_learners/distributions/GaussianDistribution.h>
#include <plearn/math/random.h>


#define INDEX(i,j) (((i)*((i)+1))/2+(j))

namespace PLearn {
using namespace std;

void displayVarGr(const Var& v, bool display_values)
{
    displayVarGraph(v,display_values,200);
}

void displayVarFn(const Func& f,bool display_values)
{
    displayFunction(f,display_values,200);
}

EntropyContrastLearner::EntropyContrastLearner() 
    : distribution("normal"),
    weight_real(1),
    weight_generated(1),
    weight_extra(1),
    weight_decay_hidden(0),
    weight_decay_output(0),
    normalize_constraints(true)
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(EntropyContrastLearner, "ONE LINE DESCRIPTION", "MULTI-LINE \nHELP");

void EntropyContrastLearner::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave
    declareOption(ol, "nconstraints", &EntropyContrastLearner::nconstraints, OptionBase::buildoption,
            "The number of constraints to create (that's also the outputsize)");
    declareOption(ol, "nhidden", &EntropyContrastLearner::nhidden, OptionBase::buildoption,
            "the number of hidden units");
    declareOption(ol, "optimizer", &EntropyContrastLearner::optimizer, OptionBase::buildoption, 
            "specify the optimizer to use\n");
    declareOption(ol, "distribution", &EntropyContrastLearner::distribution, OptionBase::buildoption, 
            "the distribution to use\n");
    declareOption(ol, "weight_real", &EntropyContrastLearner::weight_real, OptionBase::buildoption, 
            "the relative weight for the cost of the real data, for default is 1\n");
    declareOption(ol, "weight_generated", &EntropyContrastLearner::weight_generated, OptionBase::buildoption, 
            "the relative weight for the cost of the generated data, for default is 1\n");
    declareOption(ol, "weight_extra", &EntropyContrastLearner::weight_extra, OptionBase::buildoption, 
            "the relative weight for the extra cost, for default is 1\n");
    declareOption(ol, "weight_decay_hidden", &EntropyContrastLearner::weight_decay_hidden, OptionBase::buildoption, 
            "decay factor for the hidden units\n");
    declareOption(ol, "weight_decay_output", &EntropyContrastLearner::weight_decay_output, OptionBase::buildoption, 
            "decay factor for the output units\n");
    declareOption(ol, "normalize_constraints", &EntropyContrastLearner::normalize_constraints, OptionBase::buildoption, 
            "normalize the output constraints\n");
    // ### ex:
    // declareOption(ol, "myoption", &EntropyContrastLearner::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void EntropyContrastLearner::build_()
{
//    manual_seed(time(NULL));
    
    if (train_set) {

        // input data
        int n = inputsize();
        x = Var(n, "input");

        // generated data
        PP<GaussianDistribution> dist = new GaussianDistribution();
        Vec eig_values(n); 
        Mat eig_vectors(n,n); eig_vectors.clear();
        for(int i=0; i<n; i++)
        {
            eig_values[i] = 0.1; 
            eig_vectors(i,i) = 1.0;
        }
        dist->mu = Vec(n);
        dist->eigenvalues = eig_values;
        dist->eigenvectors = eig_vectors;

        PP<PDistribution> temp;
        temp = dist;

        x_hat = new PDistributionVariable(x,temp);
        


        V.resize(nconstraints);
        for(int k=0 ; k<nconstraints ; ++k) { 
            V[k] = Var(nhidden,inputsize(),("V_"+tostring(k)).c_str());
            params.push_back(V[k]);
        }


        W.resize((nconstraints*(nconstraints+1))/2);
        
        for(int i=0 ; i<nconstraints ; ++i) { 
            for(int j=0 ; j<=i ; ++j) { 
                W[INDEX(i,j)] = Var(1,nhidden,("W_"+tostring(i)+tostring(j)).c_str());
                params.push_back(W[INDEX(i,j)]);
            }
        }


        // hidden layer
        VarArray hf(nconstraints);
        VarArray hf_hat(nconstraints);

        for(int k=0 ; k<nconstraints ; ++k) { 
            hf[k] = tanh(product(V[k],x));
        }
        
        for(int k=0 ; k<nconstraints ; ++k) { 
            hf_hat[k] = tanh(product(V[k],x_hat));
        }

        
        // network output
        VarArray f(nconstraints);
        VarArray f_hat(nconstraints);

        for(int i=0 ; i<nconstraints ; ++i) { 
            for(int j=i ; j>=0 ; --j) { 
               if (j==i) {
                   f[i] = product(W[INDEX(i,j)],hf[j]);
               } else {
                   f[i] = f[i] + product(W[INDEX(i,j)],no_bprop(hf[j]));
               }
            }
        }
        
        for(int i=0 ; i<nconstraints ; ++i) { 
            for(int j=i ; j>=0 ; --j) { 
               if (j==i) {
                   f_hat[i] = product(W[INDEX(i,j)],hf_hat[j]);
               } else {
                   f_hat[i] = f_hat[i] + product(W[INDEX(i,j)],no_bprop(hf_hat[j]));
               }
            }
        }


        VarArray hg(nconstraints);

        for(int k=0 ; k<nconstraints ; ++k) { 
            hg[k] = (1-square(tanh(product(V[k],x))))*V[k];
        }

        g.resize(nconstraints);

        for(int i=0 ; i<nconstraints ; ++i) { 
            for(int j=i ; j>=0 ; --j) { 
               if (j==i) {
                   g[i] = product(W[INDEX(i,j)],hg[j]);
               } else {
                   g[i] = g[i] + product(W[INDEX(i,j)],no_bprop(hg[j]));
               }
            }
        }


        // extra cost - to keep constrains perpendicular
        Var extra_cost;
        for(int i=0 ; i<nconstraints ; ++i) { 
          for(int j=i+1 ; j<nconstraints ; ++j) { 
            Var tmp = no_bprop(g[i]);
            if (extra_cost.isNull()) {
              extra_cost = square(dot(tmp,g[j])/product(norm(tmp),norm(g[j])));
            } else {
              extra_cost = extra_cost + square(dot(tmp,g[j])/product(norm(tmp),norm(g[j])));
            }
          }
        }

        Var f_var = hconcat(f);
        Var f_hat_var = hconcat(f_hat);

        Var c_entropy;

        VarArray costs;
        
        if (distribution=="normal") {
          
            mu = Var(1,nconstraints,"mu");
            params.push_back(mu);
            sigma = Var(1,nconstraints,"sigma");
            params.push_back(sigma);

            mu_hat = Var(1,nconstraints,"mu_hat");
            params.push_back(mu_hat);
            sigma_hat = Var(1,nconstraints,"sigma_hat");
            params.push_back(sigma_hat);

            Var c_mu = square(no_bprop(f_var)-mu);
            Var c_sigma = square(sigma-square(no_bprop(c_mu)));

            Var c_mu_hat = square(no_bprop(f_hat_var)-mu_hat);
            Var c_sigma_hat = square(sigma_hat-square(no_bprop(c_mu_hat)));

            c_entropy = weight_real*square(f_var-no_bprop(mu))/no_bprop(sigma) - weight_generated*square(f_hat_var-no_bprop(mu_hat))/no_bprop(sigma_hat);

            costs = c_entropy & c_mu & c_sigma & c_mu_hat & c_sigma_hat;
            
            if (nconstraints>1) {
                costs &= weight_extra*extra_cost;
            }        
            if (weight_decay_hidden>0) {
                costs &= weight_decay_hidden*sumsquare(hconcat(V));
            }
            if (weight_decay_output>0) {
                costs &= weight_decay_output*sumsquare(hconcat(W));
            }
        } 
        else if (distribution=="student") {
            c_entropy = weight_real*log(real(1)+square(f_var)) - weight_generated*log(real(1)+square(f_hat_var));
            
            costs.push_back(c_entropy);
            
            if (nconstraints>1) {
                costs &= weight_extra*extra_cost;
            }
            if (weight_decay_hidden>0) {
                costs &= weight_decay_hidden*sumsquare(hconcat(V));
            }
            if (weight_decay_output>0) {
                costs &= weight_decay_output*sumsquare(hconcat(W));
            }
        }


        training_cost = sum(hconcat(costs));
        training_cost->setName("cost");


        f_output = Func(x, hconcat(g)); 
    }
}

// ### Nothing to add here, simply calls build_
void EntropyContrastLearner::build()
{
    inherited::build();
    build_();
}


void EntropyContrastLearner::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("EntropyContrastLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int EntropyContrastLearner::outputsize() const
{
    return nconstraints;
}

void EntropyContrastLearner::forget()
{

  if (train_set) initializeParams();
  stage = 0;
}

void EntropyContrastLearner::train()
{
    if(!train_stats)  // make a default stats collector, in case there's none
        train_stats = new VecStatsCollector();

    int l = train_set->length();
    int nsamples = 1;
    Func paramf = Func(x, training_cost); // parameterized function to optimize
    //displayFunction(paramf);

    Var totalcost = meanOf(train_set, paramf, nsamples);
    if(optimizer)
    {
        optimizer->setToOptimize(params, totalcost);  
        optimizer->build();
        optimizer->reset();
    }
    else PLERROR("EntropyContrastLearner::train can't train without setting an optimizer first!");

    int optstage_per_lstage = l/nsamples;
    while(stage<nstages)
    {
        optimizer->nstages = optstage_per_lstage;

        // clear statistics of previous epoch
        train_stats->forget();

        optimizer->optimizeN(*train_stats);

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch

        if (verbosity>0) {
            cout << "Stage: " << stage << ", training cost: " << training_cost->matValue;
        }
    }


    Vec x_(inputsize());
    Vec g_(inputsize()*nconstraints);
    Func f(x, hconcat(g)); 
    
    ofstream file1("gen.dat");
    for(int t=0 ; t<200 ; ++t) { 
      train_set->getRow(t,x_);

      f->fprop(x_,g_);

      file1 << x_ << " ";

      for(int k=0 ; k<nconstraints ; ++k) { 
          int is = inputsize();
          Vec tmp(is);

          tmp = g_.subVec(k*is,is);
          normalize(tmp,2);
          tmp /= 15;
          
          file1 << tmp << " ";
      }
      file1 << "\n";

    }
    file1.close();
    
}


void EntropyContrastLearner::computeOutput(const Vec& input, Vec& output) const
{
    int nout = inputsize()*nconstraints;
    output.resize(nout);
    

    f_output->fprop(input,output);

    if (normalize_constraints) {
        int is = inputsize();
        for(int k=0 ; k<nconstraints ; ++k) { 
            Vec tmp(is);

            tmp = output.subVec(k*is,is);
            normalize(tmp,2);
            tmp /= 15;
        }
    }
}    

void EntropyContrastLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
        const Vec& target, Vec& costs) const
{
    // Compute the costs from *already* computed output. 
    // ...
}                                

TVec<string> EntropyContrastLearner::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutpus
    // (these may or may not be exactly the same as what's returned by getTrainCostNames).
    TVec<string> ret;
    return ret;
}

TVec<string> EntropyContrastLearner::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes and 
    // for which it updates the VecStatsCollector train_stats
    // (these may or may not be exactly the same as what's returned by getTestCostNames).
    TVec<string> ret;
    return ret;
}

void EntropyContrastLearner::initializeParams()
{
    real delta = 1; //1.0 / sqrt(real(inputsize()));
    for(int k=0 ; k<nconstraints ; ++k) { 
      fill_random_uniform(V[k]->matValue, -delta, delta);
    }
    delta = 1;//1.0 / real(nhidden);
    for(int k=0 ; k<((nconstraints*(nconstraints+1))/2) ; ++k) { 
      fill_random_uniform(W[k]->matValue, -delta, delta);
    }

    if (distribution=="normal") {
        mu->matValue.fill(0);
        sigma->matValue.fill(1);
        mu_hat->matValue.fill(0);
        sigma_hat->matValue.fill(1);
    }
}

} // end of namespace PLearn
