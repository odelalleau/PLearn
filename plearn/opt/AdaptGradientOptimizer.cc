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
   * $Id: AdaptGradientOptimizer.cc,v 1.12 2003/10/07 16:38:00 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

// TODO: check all includes are needed
#include "AdaptGradientOptimizer.h"
#include "AsciiVMatrix.h" // TODO Remove later
#include "TMat_maths.h"
#include "DisplayUtils.h"

// TODO: remove the hacks
//#define HACK_NO_BASIC_ADAPT     // no basic learning rate adaptation
// #define HACK_LR_CLASSIC          // slightly different update call, but should be the same effect
//#define HACK_2_LR_VARIANCE 1.0      // two learning rates depending on low / high variance
// #define HACK_LINEAR_LR_VARIANCE  // learning rates with linear interpolation depending on variance
// #define HACK_USE_EXPONENTIAL_AVG_VARIANCE 0.9 // use an exponential average of the variance
//#define HACK_FORMULA_GRAD_VAR // use a formula of the kind lr = a * grad - b * var
//#define HACK_ALTERNATE_LR 0.1 // alternate 2 learning rates (initial and initial*(1+coeff))
//#define HACK_2_LR_LAYER (17*20)  // one learning rate for each layer
//#define HACK_SUDDEN_DEC       // decrease learning_rate to 0 in the last epochs
//#define HACK_CST_SUDDEN_DEC       // decrease learning_rate to 0 in the last epochs, constantly
//#define HACK_DECREASING_BOUNDS // decreasing bounds on the adaptive learning rate

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
                  "    the learning rate decrease constant \n");
    // TODO: be more explicit here

    declareOption(ol, "learning_rate_adaptation", &AdaptGradientOptimizer::learning_rate_adaptation, OptionBase::buildoption, 
                  "    the way the learning rates evolve\n");

    declareOption(ol, "adapt_every", &AdaptGradientOptimizer::adapt_every, OptionBase::buildoption, 
                  "    the learning rate adaptation will occur after adapt_every updates of the weights (0 means after each epoch)\n");

    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(AdaptGradientOptimizer, "ONE LINE DESCR", "NO HELP");

// TODO: remove them all
static Vec store_cost;
static Vec store_var_grad;
static Vec store_grad;
static Vec store_quad_grad;
static int count_updates = 0;

////////////
// build_ //
////////////
void AdaptGradientOptimizer::build_(){
  early_stop = false;
  cost_stage = 0;
  count_updates = 0; // TODO remove later
  learning_rate = start_learning_rate;
  SumOfVariable* sumofvar = dynamic_cast<SumOfVariable*>((Variable*)cost);
  stochastic_hack = sumofvar!=0 && sumofvar->nsamples==1;
  params.clearGradient();
  if (adapt_every == 0) {
    adapt_every = nstages;  // the number of steps to complete an epoch
    // TODO check this is true
  }
  int n = params.nelems();
  if (n > 0) {
    store_var_grad.resize(n);
    store_var_grad.clear();
    store_grad.resize(n);
    store_quad_grad.resize(n);
    store_grad.clear(); // TODO see if we keep those ones
    store_quad_grad.clear();
    learning_rates.resize(n);
    gradient.resize(n);
    tmp_storage.resize(n);
    old_evol.resize(n); // TODO I think we do keep this one
    oldgradientlocations.resize(params.size()); // TODO same here
    meancost.resize(cost->size());
    meancost.clear();
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
        // TODO See what we do here
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
    Vec learning_rates,
    Vec old_params,
    Vec old_evol) {
// TODO Remove later
  Var* array = params->data();
  int j = 0;
  int k;
  real u; // used to store old_evol[j]
  real min_lr = min_learning_rate; // / (1 + stage * decrease_constant);
  real max_lr = max_learning_rate; // / (1 + stage * decrease_constant);
  // TODO make sure this is the right thing to do
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
     
      if (learning_rates[j] < min_lr) {
          learning_rates[j] = min_lr;
      } else if (learning_rates[j] > max_lr) {
        learning_rates[j] = max_lr;
      }
    }
  }

#if defined(HACK_2_LR_VARIANCE) 
  real moy_var = 0;
  real exp_avg_coeff = 0;
#ifdef HACK_USE_EXPONENTIAL_AVG_VARIANCE
  if (stage > 1) {
    exp_avg_coeff = HACK_USE_EXPONENTIAL_AVG_VARIANCE;
  }
#endif
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
#if defined(HACK_2_LR_VARIANCE)
#if defined(HACK_2_LR_VARIANCE)
  real var_limit = HACK_2_LR_VARIANCE;
#endif // if defined(HACK_2_LR_VARIANCE)
  for (int j=0; j<params.nelems(); j++) {
    if (store_var_grad[j] <= moy_var * var_limit) {
#if defined(HACK_2_LR_VARIANCE)
      learning_rates[j] = max_lr;
#endif // if defined(HACK_2_LR_VARIANCE)
      nb_low_var++;
    } else {
#if defined(HACK_2_LR_VARIANCE)
      learning_rates[j] = min_lr;
#endif // if defined(HACK_2_LR_VARIANCE)
      nb_high_var++;
    }
  }
#endif // if defined(HACK_ADAPT_VARIANCE)
  cout << "Nb low var = " << nb_low_var << " -- Nb high var = " << nb_high_var << endl;
#endif // defined(HACK_2_LR_VARIANCE) || defined(HACK_LINEAR_LR_VARIANCE)

  cout << "Current min_LR = " << min_lr << "  and max_LR = " << max_lr << endl;
}

/////////////////
// computeCost //
/////////////////
void AdaptGradientOptimizer::computeCost() {
  meancost /= real(stage - cost_stage);
  cout << stage << " : " << meancost << endl;
  int count = 0;
  real moy_high = 0;
  real moy_low = 0;
/*  for (int i=0; i<store_cost.length(); i++) {
    if (store_cost[i] > meancost[1]) {
      if (store_cost[i] > 0.0001 * nstages) {
//        cout << i << ": " << store_cost[i] << "  -  ";
      }
      count++;
      moy_high += store_cost[i];
    } else {
      moy_low += store_cost[i];
    }
  }
  cout << endl << "Nb high = " << count << "  -- Moy_High = " << moy_high / real(count) << "  -- Moy_Low = " << moy_low / real(store_cost.length() - count) << endl; */
  early_stop = measure(stage,meancost);
  meancost.clear();
  cost_stage = stage;
  store_cost.clear();
}

//////////////
// optimize //
//////////////
real AdaptGradientOptimizer::optimize()
{
  PLERROR("In AdaptGradientOptimizer::optimize Deprecated, use OptimizeN !");
  return 0;
}

static bool altern_lr = false;
static int count_alt = 0;

///////////////
// optimizeN //
///////////////
bool AdaptGradientOptimizer::optimizeN(VecStatsCollector& stats_coll) {

  bool adapt = (learning_rate_adaptation != 0);
  stochastic_hack = stochastic_hack && !adapt;
  stochastic_hack = false; // TODO Remove later

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
    // TODO Incorporate it into other learning rate adaptation kinds ?
    double minlr = 0;
    switch (learning_rate_adaptation) {
      case 0:
#ifdef HACK_CST_SUDDEN_DEC
        if (stage > (int) (adapt_coeff1)) {
        } else {
          learning_rate = start_learning_rate/(1.0+decrease_constant*stage);
        }
#else
        learning_rate = start_learning_rate/(1.0+decrease_constant*stage);
#endif
        break;
      case 1:
        // TODO HackSource to remove
/*        minlr = min_learning_rate / (1.0 + stage * decrease_constant);
        for (int j=0; j<params.nelems(); j++) {
            learning_rates[j] = minlr;
        } */
        break;
      default:
        break;
    }

    proppath.clearGradient();
    if (adapt)
      cost->gradient[0] = -1.;
    else
#if defined(HACK_LR_CLASSIC) || defined(HACK_CST_SUDDEN_DEC)
      cost->gradient[0] = -1.;
#else
      cost->gradient[0] = -learning_rate;
#endif

    proppath.fbprop();

    meancost += cost->value;
    // TODO Remove later
    if (stage == 0) {
      store_cost.resize(1); //nstages;
    }
//    store_cost[nstages - (stage_max - stage)] += cost->value[1];

    // TODO Put back in the switch below when no more gradient stats
    // are collected ?
    params.copyGradientTo(gradient);

#if defined(HACK_2_LR_VARIANCE) || defined(HACK_LINEAR_LR_VARIANCE) || defined(HACK_FORMULA_GRAD_VAR)
    for (int i=0; i<params.nelems(); i++) {
      store_grad[i] += gradient[i];
      store_quad_grad[i] += gradient[i] * gradient[i];
      count_updates++;
    }
#endif
    
    real need_lower_lr = 0; // collectGradientStats(gradient);
//    collectGradientStats(gradient);

/*    if (need_lower_lr == 1) {
      cout << "Lower ! : ";
//      min_learning_rate *= 0.9;
    } else if (need_lower_lr == -1) {
      cout << "Higher ! : ";
//      min_learning_rate *= 1.1;
    }*/
    
    // Move along the chosen direction
    // And learning rate adaptation after each step
    real coeff = 1/(1.0 + stage * decrease_constant);
#ifdef HACK_CST_SUDDEN_DEC
#endif
//    if (stage % 10000 == 0) cout << "Coeff = " << coeff << endl;
    switch (learning_rate_adaptation) {
      case 0:
        if (!stochastic_hack) {
#if defined(HACK_LR_CLASSIC)
          params.update(learning_rate, gradient);
          params.clearGradient();
#elif defined(HACK_CST_SUDDEN_DEC)
          params.update(learning_rate, gradient, coeff, b);
          params.clearGradient();
#else
          params.updateAndClear();
#endif
        }
        break;
      case 1:
        // params.copyGradientTo(gradient);
        // TODO Not efficient, write a faster update ?
#if defined(HACK_FORMULA_GRAD_VAR) || defined(HACK_DECREASING_BOUNDS)
        params.update(learning_rates, gradient);
#else
        params.update(learning_rates, gradient, coeff); 
#endif
        params.clearGradient();
//        }
//        cout << "Gradient norm:" << sqrt(pownorm(gradient)) << endl;
        break;
      case 2:
        // params.copyGradientTo(gradient);
        adaptLearningRateALAP1(tmp_storage, gradient);
        params.update(learning_rate, gradient);
        tmp_storage << gradient;
        params.clearGradient();
        break;
      default:
        break;
    }

    if ((stage + 1) % adapt_every == 0) {
      // Time for learning rate adaptation
      switch (learning_rate_adaptation) {
        case 0:
#ifdef HACK_ALTERNATE_LR
#endif
          break;
        case 1:
          adaptLearningRateBasic(learning_rates, tmp_storage, old_evol);
          params.copyTo(tmp_storage);
          break;
        case 2:
          cout << "Current learning rate " << learning_rate << endl;
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
