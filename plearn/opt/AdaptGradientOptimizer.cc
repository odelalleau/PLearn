// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2003 Pascal Vincent, Yoshua Bengio,
//                         Olivier Delalleau and University of Montreal
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
   * $Id: AdaptGradientOptimizer.cc,v 1.15 2003/10/14 14:52:48 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "AdaptGradientOptimizer.h"
#include "SumOfVariable.h"

namespace PLearn <%
using namespace std;

AdaptGradientOptimizer::AdaptGradientOptimizer(real the_start_learning_rate, 
                                     real the_decrease_constant,
                                     real the_min_learning_rate,
                                     real the_max_learning_rate,
                                     int the_learning_rate_adaptation,
                                     real the_adapt_coeff1,
                                     real the_adapt_coeff2,
                                     int n_updates, const string& filename, 
                                     int every_iterations)
  :inherited(n_updates, filename, every_iterations),
   start_learning_rate(the_start_learning_rate),
   min_learning_rate(the_min_learning_rate),
   max_learning_rate(the_max_learning_rate),
   learning_rate_adaptation(the_learning_rate_adaptation),
   adapt_coeff1(the_adapt_coeff1),
   adapt_coeff2(the_adapt_coeff2),
   decrease_constant(the_decrease_constant),
   adapt_every(0)
{}

AdaptGradientOptimizer::AdaptGradientOptimizer(VarArray the_params, Var the_cost,
                                     real the_start_learning_rate, 
                                     real the_decrease_constant,
                                     real the_min_learning_rate,
                                     real the_max_learning_rate,
                                     int the_learning_rate_adaptation,
                                     real the_adapt_coeff1,
                                     real the_adapt_coeff2,
                                     int n_updates, const string& filename, 
                                     int every_iterations)
  :inherited(the_params,the_cost, n_updates, filename, every_iterations),
   start_learning_rate(the_start_learning_rate),
   min_learning_rate(the_min_learning_rate),
   max_learning_rate(the_max_learning_rate),
   learning_rate_adaptation(the_learning_rate_adaptation),
   adapt_coeff1(the_adapt_coeff1),
   adapt_coeff2(the_adapt_coeff2),
   decrease_constant(the_decrease_constant) {}

AdaptGradientOptimizer::AdaptGradientOptimizer(VarArray the_params, Var the_cost, 
                                     VarArray update_for_measure,
                                     real the_start_learning_rate, 
                                     real the_decrease_constant,
                                     real the_min_learning_rate,
                                     real the_max_learning_rate,
                                     int the_learning_rate_adaptation,
                                     real the_adapt_coeff1,
                                     real the_adapt_coeff2,
                                     int n_updates, const string& filename, 
                                     int every_iterations)
  :inherited(the_params,the_cost, update_for_measure,
             n_updates, filename, every_iterations),
   start_learning_rate(the_start_learning_rate),
   min_learning_rate(the_min_learning_rate),
   max_learning_rate(the_max_learning_rate),
   learning_rate_adaptation(the_learning_rate_adaptation),
   adapt_coeff1(the_adapt_coeff1),
   adapt_coeff2(the_adapt_coeff2),
   decrease_constant(the_decrease_constant) {}


void AdaptGradientOptimizer::declareOptions(OptionList& ol)
{
    declareOption(ol, "start_learning_rate", &AdaptGradientOptimizer::start_learning_rate, OptionBase::buildoption, 
                  "    the initial learning rate\n");

    declareOption(ol, "min_learning_rate", &AdaptGradientOptimizer::min_learning_rate, OptionBase::buildoption, 
                  "    the minimum value for the learning rate, when there is learning rate adaptation\n");

    declareOption(ol, "max_learning_rate", &AdaptGradientOptimizer::max_learning_rate, OptionBase::buildoption, 
                  "    the maximum value for the learning rate, when there is learning rate adaptation\n");

    declareOption(ol, "adapt_coeff1", &AdaptGradientOptimizer::adapt_coeff1, OptionBase::buildoption, 
                  "    a coefficient for learning rate adaptation, use may depend on the kind of adaptation\n");

    declareOption(ol, "adapt_coeff2", &AdaptGradientOptimizer::adapt_coeff2, OptionBase::buildoption, 
                  "    a coefficient for learning rate adaptation, use may depend on the kind of adaptation\n");

    declareOption(ol, "decrease_constant", &AdaptGradientOptimizer::decrease_constant, OptionBase::buildoption, 
                  "    the learning rate decrease constant : each update of the weights is scaled by the\n\
         coefficient 1/(1 + stage * decrease_constant\n");

    declareOption(ol, "learning_rate_adaptation", &AdaptGradientOptimizer::learning_rate_adaptation, OptionBase::buildoption, 
                  "    the way the learning rates evolve :\n\
          - 0  : no adaptation\n\
          - 1  : basic adaptation :\n\
                   if the gradient of the weight i has the same sign for two consecutive epochs\n\
                     then lr(i) = lr(i) + lr(i) * adapt_coeff1\n\
                     else lr(i) = lr(i) - lr(i) * adapt_coeff2\n\
          - 2  : ALAP1 formula. See code (not really tested)\n\
          - 3  : variance-dependent learning rate :\n\
                   let avg(i) be the exponential average of the variance of the gradient of the weight i\n\
                   over the past epochs, where the coefficient for the exponential average is adapt_coeff1\n\
                   (adapt_coeff1 = 0 means no average)\n\
                   if avg(i) is low (ie < average of all avg(j))\n\
                     then lr(i) = max_learning_rate\n\
                     else lr(i) = min_learning_rate\n");

    declareOption(ol, "adapt_every", &AdaptGradientOptimizer::adapt_every, OptionBase::buildoption, 
                  "    the learning rate adaptation will occur after adapt_every updates of the weights (0 means after each epoch)\n");

    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(AdaptGradientOptimizer, "ONE LINE DESCR", "NO HELP");

////////////
// build_ //
////////////
void AdaptGradientOptimizer::build_(){
  early_stop = false;
  count_updates = 0;
  learning_rate = start_learning_rate;
  SumOfVariable* sumofvar = dynamic_cast<SumOfVariable*>((Variable*)cost);
  stochastic_hack = sumofvar!=0 && sumofvar->nsamples==1;
  params.clearGradient();
  int n = params.nelems();
  if (n > 0) {
    store_var_grad.resize(n);
    store_var_grad.clear();
    store_grad.resize(n);
    store_quad_grad.resize(n);
    store_grad.clear();
    store_quad_grad.clear();
    learning_rates.resize(n);
    gradient.resize(n);
    tmp_storage.resize(n);
    old_evol.resize(n);
    oldgradientlocations.resize(params.size());
    learning_rates.fill(start_learning_rate);
    switch (learning_rate_adaptation) {
      case 0:
        break;
      case 1:
        // tmp_storage is used to store the old parameters
        params.copyTo(tmp_storage);
        old_evol.fill(0);
        break;
      case 2:
        // tmp_storage is used to store the initial opposite gradient
        Optimizer::computeOppositeGradient(this, tmp_storage);
        break;
      case 3:
        break;
      default:
        break;
    }
  }
}

////////////////////////////
// adaptLearningRateALAP1 //
////////////////////////////
void AdaptGradientOptimizer::adaptLearningRateALAP1(
    Vec old_gradient,
    Vec new_gradient) {
  int j = 0; // the current index in learning_rates
  real prod = 0;
  for (j = 0; j<params.nelems(); j++) {
    prod += old_gradient[j] * new_gradient[j];
  }
  // The division by j=params.nelems() is a scaling coeff
  learning_rate = learning_rate + adapt_coeff1 * prod / real(j);
  if (learning_rate < min_learning_rate) {
    learning_rate = min_learning_rate;
  } else if (learning_rate > max_learning_rate) {
    learning_rate = max_learning_rate;
  }
}

////////////////////////////
// adaptLearningRateBasic //
////////////////////////////
void AdaptGradientOptimizer::adaptLearningRateBasic(
    Vec old_params,
    Vec old_evol) {
  Var* array = params->data();
  int j = 0;
  int k;
  real u; // used to store old_evol[j]
  for (int i=0; i<params.size(); i++) {
    k = j;
    for (; j<k+array[i]->nelems(); j++) {
      u = old_evol[j];
      real diff = array[i]->valuedata[j-k] - old_params[j];
      if (diff > 0) {
        // the parameter has increased
        if (u > 0) {
          old_evol[j]++;
        } else {
          old_evol[j] = +1;
        }
      } else if (diff < 0) {
        // the parameter has decreased
        if (u < 0) {
          old_evol[j]--;
        } else {
          old_evol[j] = -1;
        }
      } else {
        // there has been no change
        old_evol[j] = 0;
      }
      if (u * old_evol[j] > 0) {
        // consecutive updates in the same direction
        learning_rates[j] += learning_rates[j] * adapt_coeff1;
      }
      else if (u * old_evol[j] < 0) {
        // oscillation
        learning_rates[j] -= learning_rates[j] * adapt_coeff2;
      }
     
      if (learning_rates[j] < min_learning_rate) {
          learning_rates[j] = min_learning_rate;
      } else if (learning_rates[j] > max_learning_rate) {
        learning_rates[j] = max_learning_rate;
      }
    }
  }
}

///////////////////////////////
// adaptLearningRateVariance //
///////////////////////////////
void AdaptGradientOptimizer::adaptLearningRateVariance() {
  real moy_var = 0;
  real exp_avg_coeff = 0;
  if (stage > 1) {
    exp_avg_coeff = adapt_coeff1;
  }
  for (int j=0; j<params.nelems(); j++) {
    // Compute variance
    store_var_grad[j] = 
      store_var_grad[j] * exp_avg_coeff +
      (store_quad_grad[j] - store_grad[j]*store_grad[j] / real(count_updates))
        * (1 - exp_avg_coeff);
    moy_var += store_var_grad[j];
  }
  count_updates = 0;
  store_quad_grad.clear();
  store_grad.clear();
  moy_var /= real(params.nelems());
  int nb_low_var = 0, nb_high_var = 0;
  real var_limit = 1.0;
  for (int j=0; j<params.nelems(); j++) {
    if (store_var_grad[j] <= moy_var * var_limit) {
      learning_rates[j] = max_learning_rate;
      nb_low_var++;
    } else {
      learning_rates[j] = min_learning_rate;
      nb_high_var++;
    }
  }
}

//////////////
// optimize //
//////////////
real AdaptGradientOptimizer::optimize()
{
  PLERROR("In AdaptGradientOptimizer::optimize Deprecated, use OptimizeN !");
  return 0;
}

///////////////
// optimizeN //
///////////////
bool AdaptGradientOptimizer::optimizeN(VecStatsCollector& stats_coll) {

  bool adapt = (learning_rate_adaptation != 0);
  stochastic_hack = stochastic_hack && !adapt;
  if (adapt_every == 0) {
    adapt_every = nstages;  // the number of steps to complete an epoch
  }

  // Big hack for the special case of stochastic gradient, to avoid doing an explicit update
  // (temporarily change the gradient fields of the parameters to point to the parameters themselves,
  // so that gradients are "accumulated" directly in the parameters, thus updating them!
  if(stochastic_hack) {
    int n = params.size();
    for(int i=0; i<n; i++)
      oldgradientlocations[i] = params[i]->defineGradientLocation(params[i]->matValue);
  }

  int stage_max = stage + nstages; // the stage to reach

  for (; !early_stop && stage<stage_max; stage++) {

    // Take into account the learning rate decrease
    // This is actually done during the update step, except when there is no
    // learning rate adaptation
    switch (learning_rate_adaptation) {
      case 0:
        learning_rate = start_learning_rate/(1.0+decrease_constant*stage);
        break;
      default:
        break;
    }

    proppath.clearGradient();
    if (adapt)
      cost->gradient[0] = -1.;
    else
      cost->gradient[0] = -learning_rate;

    proppath.fbprop();

    // Actions to take after each step, depending on the
    // adaptation method used :
    // - moving along the chosen direction
    // - adapting the learning rate
    // - storing some data
    real coeff = 1/(1.0 + stage * decrease_constant); // the scaling cofficient
    switch (learning_rate_adaptation) {
      case 0:
        if (!stochastic_hack) {
          params.updateAndClear();
        }
        break;
      case 1:
        params.copyGradientTo(gradient);
        // TODO Not really efficient, write a faster update ?
        params.update(learning_rates, gradient, coeff); 
        params.clearGradient();
        break;
      case 2:
        params.copyGradientTo(gradient);
        adaptLearningRateALAP1(tmp_storage, gradient);
        params.update(learning_rate, gradient);
        tmp_storage << gradient;
        params.clearGradient();
        break;
      case 3:
        // storing sum and sum-of-squares of the gradient in order to compute
        // the variance later
        params.copyGradientTo(gradient);
        for (int i=0; i<params.nelems(); i++) {
          store_grad[i] += gradient[i];
          store_quad_grad[i] += gradient[i] * gradient[i];
        }
        count_updates++;
        params.update(learning_rates, gradient, coeff);
        params.clearGradient();
        break;
      default:
        break;
    }

    if ((stage + 1) % adapt_every == 0) {
      // Time for learning rate adaptation
      switch (learning_rate_adaptation) {
        case 0:
          break;
        case 1:
          adaptLearningRateBasic(tmp_storage, old_evol);
          params.copyTo(tmp_storage);
          break;
        case 2:
          // Nothing, the adaptation is after each example
          break;
        case 3:
          adaptLearningRateVariance();
          break;
        default:
          break;
      }
    }

    stats_coll.update(cost->value);
  }

  if(stochastic_hack) // restore the gradients as they previously were...
    {
      int n = params.size();
      for(int i=0; i<n; i++)
        params[i]->defineGradientLocation(oldgradientlocations[i]);
    }

  if (early_stop)
    cout << "Early Stopping !" << endl;
  return early_stop;
}

%> // end of namespace PLearn
