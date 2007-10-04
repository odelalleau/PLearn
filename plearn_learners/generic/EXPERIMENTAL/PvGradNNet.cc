// -*- C++ -*-

// PvGradNNet.cc
//
// Copyright (C) 2007 PA M, Pascal V
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

// Authors: P AM, Pascal V

/*! \file PvGradNNet.cc */

#include "PvGradNNet.h"
#include <plearn/math/pl_erf.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PvGradNNet,
    "Multi-layer neural network for experimenting with Pascal V's gradient idea.",
    "See the twiki's SRPropGradient entry.\n"
    );

PvGradNNet::PvGradNNet()
    : mNNet(),
      pv_initial_stepsize(1e-1),
      pv_min_stepsize(1e-6),
      pv_max_stepsize(50.0),
      pv_acceleration(1.2),
      pv_deceleration(0.5),
      pv_min_samples(2),
      pv_required_confidence(0.80),
      pv_strategy(1),
      pv_random_sample_step(false),
      pv_self_discount(0.5),
      pv_other_discount(0.95),
      n_updates(0)
{
    random_gen = new PRandom();
}

void PvGradNNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "pv_initial_stepsize",
                  &PvGradNNet::pv_initial_stepsize,
                  OptionBase::buildoption,
                  "Initial size of steps in parameter space");

    declareOption(ol, "pv_min_stepsize",
                  &PvGradNNet::pv_min_stepsize,
                  OptionBase::buildoption,
                  "Minimal size of steps in parameter space");

    declareOption(ol, "pv_max_stepsize",
                  &PvGradNNet::pv_max_stepsize,
                  OptionBase::buildoption,
                  "Maximal size of steps in parameter space");

    declareOption(ol, "pv_acceleration",
                  &PvGradNNet::pv_acceleration,
                  OptionBase::buildoption,
                  "Coefficient by which to multiply the step sizes.");

    declareOption(ol, "pv_deceleration",
                  &PvGradNNet::pv_deceleration,
                  OptionBase::buildoption,
                  "Coefficient by which to multiply the step sizes.");

    declareOption(ol, "pv_min_samples",
                  &PvGradNNet::pv_min_samples,
                  OptionBase::buildoption,
                  "PV's minimum number of samples to estimate gradient sign.\n"
                  "This should at least be 2.");

    declareOption(ol, "pv_required_confidence",
                  &PvGradNNet::pv_required_confidence,
                  OptionBase::buildoption,
                  "Minimum required confidence (probability of being positive or negative) for taking a step.");

    declareOption(ol, "pv_strategy",
                  &PvGradNNet::pv_strategy,
                  OptionBase::buildoption,
                  "Strategy to use for the weight updates (number from 1 to 4).");

    declareOption(ol, "pv_random_sample_step",
                  &PvGradNNet::pv_random_sample_step,
                  OptionBase::buildoption,
                  "If this is set to true, then we will randomly choose the step sign\n"
                  "for each parameter based on the estimated probability of it being\n"
                  "positive or negative.");

    declareOption(ol, "pv_self_discount",
                  &PvGradNNet::pv_self_discount,
                  OptionBase::buildoption,
                  "Discount used to perform soft invalidation of a weight's statistics\n"
                  "after its update.");

    declareOption(ol, "pv_other_discount",
                  &PvGradNNet::pv_other_discount,
                  OptionBase::buildoption,
                  "Discount used to perform soft invalidation of other weights'\n" 
                  "statistics after a weight update.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

// TODO - reloading an object will not work! layer_params will juste get lost.
void PvGradNNet::build_()
{
    int n = all_params.length();
    pv_all_nsamples.resize(n);
    pv_all_sum.resize(n);
    pv_all_sumsquare.resize(n);
    pv_all_stepsigns.resize(n);
    pv_all_stepsizes.resize(n);

    // Get some structure on the previous Vecs
    pv_layer_nsamples.resize(n_layers-1);
    pv_layer_sum.resize(n_layers-1);
    pv_layer_sumsquare.resize(n_layers-1);
    pv_layer_stepsigns.resize(n_layers-1);
    pv_layer_stepsizes.resize(n_layers-1);
    int np;
    for (int i=0,p=0;i<n_layers-1;i++)  {
        np=layer_sizes[i+1]*(1+layer_sizes[i]);
        pv_layer_nsamples.subVec(p,np).toMat(layer_sizes[i+1],layer_sizes[i]+1);
        pv_layer_sum.subVec(p,np).toMat(layer_sizes[i+1],layer_sizes[i]+1);
        pv_layer_sumsquare.subVec(p,np).toMat(layer_sizes[i+1],layer_sizes[i]+1);
        pv_layer_stepsigns[i]=pv_all_stepsigns.subVec(p,np).toMat(layer_sizes[i+1],layer_sizes[i]+1);
        pv_layer_stepsizes[i]=pv_all_stepsizes.subVec(p,np).toMat(layer_sizes[i+1],layer_sizes[i]+1);
        p+=np;
    }

//    pv_gradstats = new VecStatsCollector();

}

// ### Nothing to add here, simply calls build_
void PvGradNNet::build()
{
    inherited::build();
    build_();
}


void PvGradNNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(pv_all_nsamples, copies); 
    deepCopyField(pv_layer_nsamples, copies);
    deepCopyField(pv_all_sum, copies); 
    deepCopyField(pv_layer_sum, copies);
    deepCopyField(pv_all_sumsquare, copies);
    deepCopyField(pv_layer_sumsquare, copies);
    deepCopyField(pv_all_stepsigns, copies);
    deepCopyField(pv_layer_stepsigns, copies);
    deepCopyField(pv_all_stepsizes, copies);
    deepCopyField(pv_layer_stepsizes, copies);

//    deepCopyField(pv_gradstats, copies);
}

void PvGradNNet::forget()
{
    //! (Re-)initialize the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!)
    inherited::forget();

    pv_all_nsamples.fill(0);
    pv_all_sum.fill(0.0);
    pv_all_sumsquare.fill(0.0);
    pv_all_stepsigns.fill(0);
    pv_all_stepsizes.fill(pv_initial_stepsize);

    // used in the discountGrad() strategy
    n_updates = 0; 
    
//    pv_gradstats->forget();
}

//! Performs the backprop update. Must be called after the fbpropNet.
void PvGradNNet::bpropUpdateNet(int t)
{
    bpropNet(t);

    switch( pv_strategy )   {
        case 1 :
            pvGrad();
            break;
        case 2 :
            discountGrad();
            break;
        case 3 :
            globalSyncGrad();
            break;
        case 4 :
            neuronSyncGrad();
            break;
        default :
            PLERROR("PvGradNNet::bpropUpdateNet() - No such pv_strategy.");
    }
}

void PvGradNNet::pvGrad()   
{
    int np = all_params.length();
    real m, e, prob_pos, prob_neg;

    for(int k=0; k<np; k++) {
        // update stats
        pv_all_nsamples[k]++;
        pv_all_sum[k] += all_params_gradient[k];
        pv_all_sumsquare[k] += all_params_gradient[k] * all_params_gradient[k];

        if(pv_all_nsamples[k]>pv_min_samples)   {
            m = pv_all_sum[k] / pv_all_nsamples[k];
            // e is the standard error
            //e = sqrt( (pv_all_sumsquare[k] - (pv_all_sum[k]*pv_all_sum[k])/pv_all_nsamples[k]) / (real)(pv_all_nsamples[k]*(pv_all_nsamples[k]-1)) );
            // variance
            e = real((pv_all_sumsquare[k] - square(pv_all_sum[k])/pv_all_nsamples[k])/(pv_all_nsamples[k]-1));
            // standard error 
            e = sqrt(e/pv_all_nsamples[k]);

            // test to see if numerical problems
            if( fabs(m) < 1e-15 || e < 1e-15 )  {
                cout << "PvGradNNet::bpropUpdateNet() - small mean-error ratio." << endl;
                continue;
            }

            // TODO - for current treatment, not necessary to compute actual prob.
            // Comparing the ratio would be sufficient.
            prob_pos = gauss_01_cum(m/e);
            prob_neg = 1.-prob_pos;

            if(!pv_random_sample_step)  {
    
                // We adapt the stepsize before taking the step
                // gradient is positive
                if(prob_pos>=pv_required_confidence)    {
                    //pv_all_stepsizes[k] *= (pv_all_stepsigns[k]?pv_acceleration:pv_deceleration);
                    if(pv_all_stepsigns[k]>0)   {
                        pv_all_stepsizes[k]*=pv_acceleration;
                        if( pv_all_stepsizes[k] > pv_max_stepsize )
                            pv_all_stepsizes[k] = pv_max_stepsize;
                    }
                    else if(pv_all_stepsigns[k]<0)  {
                        pv_all_stepsizes[k]*=pv_deceleration;
                        if( pv_all_stepsizes[k] < pv_min_stepsize )
                            pv_all_stepsizes[k] = pv_min_stepsize;
                    }
                    all_params[k] -= pv_all_stepsizes[k];
                    pv_all_stepsigns[k] = 1;
                    pv_all_nsamples[k]=0;
                    pv_all_sum[k]=0.0;
                    pv_all_sumsquare[k]=0.0;
                }
                // gradient is negative
                else if(prob_neg>=pv_required_confidence)   {
                    //pv_all_stepsizes[k] *= ((!pv_all_stepsigns[k])?pv_acceleration:pv_deceleration);
                    if(pv_all_stepsigns[k]<0)   {
                        pv_all_stepsizes[k]*=pv_acceleration;
                        if( pv_all_stepsizes[k] > pv_max_stepsize )
                            pv_all_stepsizes[k] = pv_max_stepsize;
                    }
                    else if(pv_all_stepsigns[k]>0)  {
                        pv_all_stepsizes[k]*=pv_deceleration;
                        if( pv_all_stepsizes[k] < pv_min_stepsize )
                            pv_all_stepsizes[k] = pv_min_stepsize;
                    }
                    all_params[k] += pv_all_stepsizes[k];
                    pv_all_stepsigns[k] = -1;
                    pv_all_nsamples[k]=0;
                    pv_all_sum[k]=0.0;
                    pv_all_sumsquare[k]=0.0;
                }
            }
            /*else  // random sample update direction (sign)
            {
                bool ispos = (random_gen->binomial_sample(prob_pos)>0);
                if(ispos) // picked positive
                    all_params[k] += pv_all_stepsizes[k];
                else  // picked negative
                    all_params[k] -= pv_all_stepsizes[k];
                pv_all_stepsizes[k] *= (pv_all_stepsigns[k]==ispos)?pv_acceleration :pv_deceleration;
                pv_all_stepsigns[k] = ispos;
                st.forget();
            }*/
        }
        //pv_all_nsamples[k] = ns; // *stat*
    }

}

//! This gradient strategy is much like the one from PvGrad, however:
//!     - there is no hard statistic invalidation, it's soft with discounts.
//!     - a weight update will also affect other weights' statistics.
//! Note that discounting is applied after the update, never during.
//! With pv_self_discount=0.0 and pv_other_discount=1.0, should be same as
//! PvGrad().
void PvGradNNet::discountGrad()
{
    int np = all_params.length();
    real m, e, prob_pos, prob_neg;
    int stepsign;

    // 
    real discount = pow(pv_other_discount,n_updates);
    n_updates = 0;
    if( discount < 0.001 )
        PLWARNING("PvGradNNet::discountGrad() - discount < 0.001 - that seems small...");
    real sd = pv_self_discount / pv_other_discount; // trick: apply this self discount
                                                    // and then discount
                                                    // everyone the same
    for(int k=0; k<np; k++) {
        // Perform soft invalidation
        pv_all_nsamples[k] *= discount;
        pv_all_sum[k] *= discount;
        pv_all_sumsquare[k] *= discount;

        // update stats
        pv_all_nsamples[k]++;
        pv_all_sum[k] += all_params_gradient[k];
        pv_all_sumsquare[k] += all_params_gradient[k] * all_params_gradient[k];

        if(pv_all_nsamples[k]>pv_min_samples)   {
            m = pv_all_sum[k] / pv_all_nsamples[k];
            e = real((pv_all_sumsquare[k] - square(pv_all_sum[k])/pv_all_nsamples[k])/(pv_all_nsamples[k]-1));
            e = sqrt(e/pv_all_nsamples[k]);

            // test to see if numerical problems
            if( fabs(m) < 1e-15 || e < 1e-15 )  {
                cout << "PvGradNNet::bpropUpdateNet() - small mean-error ratio." << endl;
                continue;
            }

            // TODO - for current treatment, not necessary to compute actual
            // prob. Comparing the ratio would be sufficient.
            prob_pos = gauss_01_cum(m/e);
            prob_neg = 1.-prob_pos;

            if(prob_pos>=pv_required_confidence)
                stepsign = 1;
            else if(prob_neg>=pv_required_confidence)
                stepsign = -1;
            else
                continue;

            // consecutive steps of same sign, accelerate
            if( stepsign*pv_all_stepsigns[k]>0 )  {
                pv_all_stepsizes[k]*=pv_acceleration;
                if( pv_all_stepsizes[k] > pv_max_stepsize )
                    pv_all_stepsizes[k] = pv_max_stepsize;            
            // else if different signs decelerate
            }   else if( stepsign*pv_all_stepsigns[k]<0 )   {
                pv_all_stepsizes[k]*=pv_deceleration;
                if( pv_all_stepsizes[k] < pv_min_stepsize )
                    pv_all_stepsizes[k] = pv_min_stepsize;
            // else (previous sign was undetermined
            }//   else    {
            //}
            // step
            if( stepsign > 0 )
                all_params[k] -= pv_all_stepsizes[k];
            else
                all_params[k] += pv_all_stepsizes[k];
            pv_all_stepsigns[k] = stepsign;
            // soft invalidation of self
            pv_all_nsamples[k]*=sd;
            pv_all_sum[k]*=sd;
            pv_all_sumsquare[k]*=sd;
        }
    }
}

void PvGradNNet::globalSyncGrad()
{
}

void PvGradNNet::neuronSyncGrad()
{
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
