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
   * $Id: AdaptGradientOptimizer.cc,v 1.11 2003/10/07 15:19:44 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "AdaptGradientOptimizer.h"
#include "AsciiVMatrix.h" // TODO Remove later
#include "TMat_maths.h"
#include "DisplayUtils.h"

//#define HACK_NO_BASIC_ADAPT     // no basic learning rate adaptation
// #define HACK_LR_CLASSIC          // slightly different update call, but should be the same effect
//#define HACK_2_LR_VARIANCE 1.0      // two learning rates depending on low / high variance
// #define HACK_2_DC_VARIANCE 1.0      // two decrease constants depending on low / high variance
// #define HACK_LINEAR_LR_VARIANCE  // learning rates with linear interpolation depending on variance
//#define HACK_ADAPT_VARIANCE      // learning rate adaptation depending on higher / lower variance
// #define HACK_USE_EXPONENTIAL_AVG_VARIANCE 0.9 // use an exponential average of the variance
//#define HACK_FORMULA_GRAD_VAR // use a formula of the kind lr = a * grad - b * var
// #//define HACK_OPT_LR   // learning rate optimization with a genetic neural network
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
   mini_batch(0), adapt_every(0)
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

    declareOption(ol, "learning_rate_adaptation", &AdaptGradientOptimizer::learning_rate_adaptation, OptionBase::buildoption, 
                  "    the way the learning rates evolve\n");

    declareOption(ol, "mini_batch", &AdaptGradientOptimizer::mini_batch, OptionBase::buildoption, 
                  "    if set to x>=1, then it will consider that we present x times the same mini-batch\n");

    declareOption(ol, "adapt_every", &AdaptGradientOptimizer::adapt_every, OptionBase::buildoption, 
                  "    the learning rate adaptation will occur after adapt_every updates of the weights (0 means after each epoch)\n");

#ifdef HACK_OPT_LR
    declareOption(ol, "neural_lr", &AdaptGradientOptimizer::neural_lr, OptionBase::buildoption, 
                  "    the neural network used to compute the learning rate");
#endif

    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(AdaptGradientOptimizer, "ONE LINE DESCR", "NO HELP");

static Vec very_old_params;
static real original_min_lr = 0;
static real original_max_lr = 0;
static Vec store_cost;
static bool no_compute;
static Vec store_var_grad;
static Vec store_mean_grad;
static Vec store_grad;
static Vec store_quad_grad;
static int count_updates = 0;
#ifdef HACK_2_DC_VARIANCE
static Vec decrease_constants;
#endif
static real original_start_lr = 0;
static int count_epochs = 0;
static Vec lr_before_sd;
static int count_dec = 0;

////////////
// build_ //
////////////
void AdaptGradientOptimizer::build_(){
#ifdef HACK_2_DC_VARIANCE
  decrease_constant = 0;
#endif
  early_stop = false;
  no_compute = false;
  cost_stage = 0;
  count_updates = 0;
  count_epochs = 0;
  count_dec = 0;
  learning_rate = start_learning_rate;
  SumOfVariable* sumofvar = dynamic_cast<SumOfVariable*>((Variable*)cost);
  stochastic_hack = sumofvar!=0 && sumofvar->nsamples==1;
  params.clearGradient();
  int n = params.nelems();
  if (adapt_every == 0) {
    adapt_every = nstages;  // the number of steps to complete an epoch
  }
  cout << "adapt_every = " << adapt_every << endl;
  if (original_min_lr == 0) {
    original_min_lr = min_learning_rate;
  } else {
    min_learning_rate = original_min_lr;
  }
  if (original_max_lr == 0) {
    original_max_lr = max_learning_rate;
  } else {
    max_learning_rate = original_max_lr;
  }
  if (original_start_lr == 0) {
    original_start_lr = start_learning_rate;
  } else {
    start_learning_rate = original_start_lr;
  }
  if (n > 0) {
#ifdef HACK_2_DC_VARIANCE
    decrease_constants.resize(n);
    decrease_constants.fill(adapt_coeff1);
#endif
    lr_before_sd.resize(n);
    lr_before_sd.clear();
    store_var_grad.resize(n);
    store_var_grad.clear();
    store_mean_grad.resize(n);
    store_mean_grad.clear();
    store_grad.resize(n);
    store_quad_grad.resize(n);
    store_grad.clear();
    store_quad_grad.clear();
    learning_rates.resize(n);
    gradient.resize(n);
    tmp_storage.resize(n);
    very_old_params.resize(n);
    old_evol.resize(n);
    oldgradientlocations.resize(params.size());
    meancost.resize(cost->size());
    meancost.clear();
    learning_rates.fill(start_learning_rate);
    switch (learning_rate_adaptation) {
      case 0:
        break;
      case 1:
        // tmp_storage is used to store the old parameters
        params.copyTo(tmp_storage);
        params.copyTo(very_old_params);
        old_evol.fill(0);
        break;
      case 2:
        // tmp_storage is used to store the initial opposite gradient
        Optimizer::computeOppositeGradient(this, tmp_storage);
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
  int k;
  real prod = 0;
  for (j = 0; j<params.nelems(); j++) {
    prod += old_gradient[j] * new_gradient[j];
  }
/* Old way of computing
  Var* array = params->data();
  for (int i=0; i<params.size(); i++) {
    k = j;
    for (; j<k+array[i]->nelems(); j++) {
      prod += old_gradient[j] * new_gradient[j];
    }
  } */
  // The division by j=params.nelems() is a scaling coeff
  learning_rate = learning_rate + adapt_coeff1 * prod / real(j);
  if (learning_rate < min_learning_rate) {
    learning_rate = min_learning_rate;
  } else if (learning_rate > max_learning_rate) {
    learning_rate = max_learning_rate;
  }
}

static real coeff_sd = 0;
///////////
// bidlr //
///////////
real AdaptGradientOptimizer::bidlr(real minlr) {
  // TODO HackSource to remove
  real min_lr = minlr;
//  if (stage >= 18000 * 280) {
  if (stage >= 720183 * 12) {
//    min_lr = (stage - 18000 * 280) / real(18000 * 20) * (0 - min_lr) + min_lr;
//    min_lr = (stage - 720183 * 12) / real(720183 * 8) * (0 - min_lr) + min_lr;
/*    if (coeff_sd == 0) {
      coeff_sd = decrease_constant / (1 + stage * decrease_constant) / (1 + stage * decrease_constant);
      cout << "COEFF_SD = " << coeff_sd << endl;
    }
    min_lr = - coeff_sd * min_lr + min_lr; */
  }
//  }
  if (min_lr < 0)
    min_lr = 0.;

/*  if (stage > 18000 * 150)
    min_lr = max_lr / 10; */
  return min_lr;
}

static bool first_time = true;
static bool exchange = false;
static bool test_decrease = false;
static real consec_min = 1;
static real consec_max = 1;
static int clock = 0;
static Vec old_mean_gradient;
static VecStatsCollector store_gradient;
static real frac = 0, old_frac = 0;
// TODO Remove later (?)
#ifdef HACK_ADAPT_VARIANCE
static Vec old_var;
static Vec consec_low;
static Vec consec_high;
#endif
#ifdef HACK_OPT_LR
static Vec input_lr(2);
static Vec predic_lr(1);
#endif

////////////////////////////
// adaptLearningRateBasic //
////////////////////////////
void AdaptGradientOptimizer::adaptLearningRateBasic(
    Vec learning_rates,
    Vec old_params,
    Vec old_evol) {
  // TODO Remove later
//  static VMat lrates;
  if (first_time) {
    first_time = false;
#ifdef HACK_ADAPT_VARIANCE
    old_var.resize(params.nelems());
    old_var.clear();
    consec_low.resize(params.nelems());
    consec_low.clear();
    consec_high.resize(params.nelems());
    consec_high.clear();
#endif
//    old_mean_gradient.resize(params.nelems());
 //   lrates = new AsciiVMatrix("lrates.amat", params.nelems());
  }
//  lrates->appendRow(learning_rates);
  Var* array = params->data();
  int j = 0;
  int k;
  int nb_min = 0;
  int nb_max = 0;
  int nb_moy = 0;
  real u;
  real lr_min = 0;
  real lr_moy = 0;
  real lr_max = 0;
  real lr_mean = 0;
/*  Vec tmp_grad_stuff(params.nelems());
  tmp_grad_stuff << old_mean_gradient; */
  int n1=0, n2=0;
//  old_mean_gradient << store_gradient.getMean();
//  store_gradient.forget();
#if defined(HACK_DECREASING_BOUNDS)
  real min_lr = min_learning_rate / (1 + stage * decrease_constant);
  real max_lr = max_learning_rate / (1 + stage * decrease_constant);
#else
  real min_lr = min_learning_rate; // / (1 + stage * decrease_constant);
  real max_lr = max_learning_rate; // / (1 + stage * decrease_constant);
#endif
/*  if (clock % 13 == 0) {
  } else if ((clock - 1) % 13 == 0) {
  } else if ((clock - 2) % 13 == 0) {
  } else if ((clock - 3) % 13 == 0) {
    min_lr += min_lr * 0.05;
  } else if ((clock - 4) % 13 == 0) {
  } else if ((clock - 5) % 13 == 0) {
    min_lr += min_lr * 0.05;
  } else if ((clock - 6) % 13 == 0) {
  } else if ((clock - 7) % 13 == 0) {
    min_lr += min_lr * 0.05;
  } else if ((clock - 8) % 13 == 0) {
    min_lr += 2*min_lr * 0.05;
  } else if ((clock - 9) % 13 == 0) {
    min_lr += 3*min_lr * 0.05;
  } else if ((clock - 10) % 13 == 0) {
    min_lr += 2*min_lr * 0.05;
  } else if ((clock - 11) % 13 == 0) {
    min_lr += min_lr * 0.05;
  } */
  real norm_diff = 0;
  real old_norm_diff = 0;
  real angle = 0;
  real previous_norm_diff = 0;
  for (int i=0; i<params.size(); i++) {
    k = j;
    for (; j<k+array[i]->nelems(); j++) {
      u = old_evol[j];
      if (no_compute) {
//        array[i]->valuedata[j-k] = old_params[j] + old_params[j] - very_old_params[j];
        array[i]->valuedata[j-k] = array[i]->valuedata[j-k] + array[i]->valuedata[j-k] - old_params[j];
      }
 /*     real x2 = array[i]->valuedata[j-k]; // the new parameter
      real x1 = old_params[j];            // the old parameter
      real g2 = old_mean_gradient[j];     // the new gradient
      real g1 = tmp_grad_stuff[j];        // the old gradient
      if (g2 == g1)
        cout << "Warning ! g2 == g1 !" << endl; */
      real diff = array[i]->valuedata[j-k] - old_params[j];
      real old_diff = array[i]->valuedata[j-k] - very_old_params[j];
      norm_diff += diff*diff;
      old_norm_diff += old_diff*old_diff;
      previous_norm_diff += (old_params[j] - very_old_params[j])*(old_params[j] - very_old_params[j]);
      angle += diff * (old_params[j] - very_old_params[j]);
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
//      real coeff = min(10.0, abs(old_evol[j]));
//      old_evol[j] = array[i]->valuedata[j-k] - old_params[j];
      if (u * old_evol[j] > 0) {
#ifndef HACK_NO_BASIC_ADAPT
        learning_rates[j] += learning_rates[j] * adapt_coeff1; // * coeff;
#endif
        n1++;
      }
      else if (u * old_evol[j] < 0) {
//        learning_rates[j] = (x2 - x1) / (g2 - g1) / 18000;
//        cout << learning_rates[j] << endl;
//        if (g2 * g1 >0)
//          cout << "Warning ! g2 and g1 have same sign !" << endl;
#ifndef HACK_NO_BASIC_ADAPT
        learning_rates[j] -= learning_rates[j] * adapt_coeff2;
#endif
        n2++;
//        array[i]->valuedata[j-k] = old_params[j];
//        old_evol[j] = 0;
      }
/*      if (exchange) {
        real tmp = min_lr;
        min_lr = max_lr;
        max_lr = tmp;
      }  */
     
      // TODO HackSource to remove
/*      if (j < 340)
        learning_rates[j] = min_lr;
      else
        learning_rates[j] = min_lr; */
      if (learning_rates[j] < min_lr) {
//        if (abs(array[i]->valuedata[j-k]) < 0.7) {
          learning_rates[j] = min_lr;
//        } 
        if (learning_rates[j] == 0) {
          cout << "Null learning rate !" << endl;
        }
        nb_min++;
        lr_min += abs(array[i]->valuedata[j-k]);
      } else if (learning_rates[j] > max_lr) {
        learning_rates[j] = max_lr;
        nb_max++;
        lr_max += abs(array[i]->valuedata[j-k]);
      } else {
        nb_moy++;
        lr_moy += abs(array[i]->valuedata[j-k]);
      }
      lr_mean += abs(array[i]->valuedata[j-k]);
      // cout << learning_rates[j] << "  ";
      if (mini_batch > 0 && (stage / nstages_per_epoch) % mini_batch == 0) {
        // The next stage will examine a different mini-batch, so we need to
        // reset the evolution
        old_evol[j] = 0;
      }
    }
  }
/*  if (mini_batch > 0 && (stage / nstages_per_epoch) % mini_batch == 0) {
    cout << "Batch done !" << endl;
  } */
  angle = angle / (sqrt(norm_diff) * sqrt(previous_norm_diff));
  if (nb_min > 0) lr_min /= real(nb_min);
  if (nb_moy > 0) lr_moy /= real(nb_moy);
  if (nb_max > 0) lr_max /= real(nb_max);
  exchange = !exchange;
  clock++;
  cout << "Norm (old displacement) = " << sqrt(old_norm_diff) << "  --  Norm (displacement) = " << sqrt(norm_diff) << endl;
  cout << "Angle = " << angle << endl;
  cout << "Raised : " << n1 << " - Decreased : " << n2 << endl;
  cout << "nb_min = " << nb_min << "  --  nb_max = " << nb_max << "  --  nb_moy = " << nb_moy << endl;
  cout << "w_lr_min = " << lr_min << "  --  w_lr_max = " << lr_max << " --  w_lr_moy = " << lr_moy << "  --  w_lr_mean = " << lr_mean/real(nb_min+nb_moy+nb_max) << endl; 
  cout << "min_lr = " << min_lr << endl;
  old_frac = frac;
  if (n2 > 0) {
    frac = n2 / real(n1 + n2);
  } else {
    frac = 0;
  }
/*  if (angle > 0.999) {
    cout << "Decreasing LR !";
    min_learning_rate *= (1 - 0.01);
  } */

/*  if ((clock - 2) % 6 == 0) {
   if (frac > 0.02) {
     // decrease is necessary
     min_learning_rate *= (1 - (consec_min + 1) * 0.01);
     consec_min++;
     if (consec_min > 10)
       consec_min = 10;
     cout << "Decreasing min_LR !";
   } else {
     // decrease is useless
     // higher learning rates
     if (frac < 0.01) {
       cout << "Keeping increased min_LR ! : ";
     } else {
       min_learning_rate *= 1 / 1.01;
       cout << "Returning to previous min_LR ! : ";
     }
     consec_min = 1;
   }
  } else if ((clock - 1) % 6 == 0) {
    min_learning_rate *= 1.01;
    cout << "Increasing min_LR !";
  }
  if ((clock - 5) % 6 == 0) {
   if (frac > 0.02) {
     // decrease is necessary
     max_learning_rate *= (1 - (consec_max + 1) * 0.01);
     consec_max++;
     if (consec_max > 10)
       consec_max = 10;
     cout << "Decreasing max_LR !";
   } else {
     // decrease is useless
     // higher learning rates
     if (frac < 0.01) {
       cout << "Keeping increased max_LR ! : ";
     } else {
       max_learning_rate *= 1 / 1.01;
       cout << "Returning to previous max_LR ! : ";
     }
     consec_max = 1;
   }
  } else if ((clock - 4) % 6 == 0) {
    max_learning_rate *= 1.01;
    cout << "Increasing max_LR !";
  } */
/*  if ((clock - 2) % 3 == 0) {
   if (angle < 0.99) {
     // decrease is necessary
     min_learning_rate *= (1 - (consec_min + 1) * 0.01);
     consec_min++;
     if (consec_min > 10)
       consec_min = 10;
     cout << "Decreasing min_LR !";
   } else {
     // decrease is useless
     // higher learning rates
     if (angle > 0.999) {
       cout << "Keeping increased min_LR ! : ";
     } else {
       min_learning_rate *= 1 / 1.01;
       cout << "Returning to previous min_LR ! : ";
     }
     consec_min = 1;
   }
  } else if ((clock - 1) % 3 == 0) {
    min_learning_rate *= 1.01;
    cout << "Increasing min_LR !";
  } */
  
  min_lr = min_learning_rate; // / (1 + stage * decrease_constant);
  max_lr = max_learning_rate; // / (1 + stage * decrease_constant);

//  min_lr = bidlr(min_lr);

#if defined(HACK_2_LR_VARIANCE) || defined(HACK_LINEAR_LR_VARIANCE) || defined(HACK_ADAPT_VARIANCE) || defined(HACK_2_DC_VARIANCE) || defined(HACK_FORMULA_GRAD_VAR) || defined(HACK_OPT_LR)
  real moy_var = 0;
#if defined(HACK_FORMULA_GRAD_VAR) || defined(HACK_OPT_LR)
  real moy_mean = 0;
#endif
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
#if defined(HACK_FORMULA_GRAD_VAR) || defined(HACK_OPT_LR)
    store_mean_grad[j] = store_mean_grad[j] * exp_avg_coeff
      + (1 - exp_avg_coeff) * abs(store_grad[j]) / real(count_updates);
    moy_mean += store_mean_grad[j];
#endif
  }
#ifdef HACK_FORMULA_GRAD_VAR
  cout << "moy_mean = " << moy_mean / real(count_updates) << endl;
#endif
  count_updates = 0;
  store_quad_grad.clear();
  store_grad.clear();
  moy_var /= real(params.nelems());
  int nb_low_var = 0, nb_high_var = 0;
#if defined(HACK_ADAPT_VARIANCE)
  for (int j=0; j<params.nelems(); j++) {
    if (store_var_grad[j] >= old_var[j]) {
      // variance is higher : need to lower the learning rate
      consec_low[j] = 0;
      nb_high_var++;
      if (learning_rates[j] > min_lr) {
        consec_high[j]++;
        learning_rates[j] -= adapt_coeff2 * consec_high[j] * learning_rates[j];
      }
    } else {
      // variance is lower : need to raise the learning rate
      consec_high[j] = 0;
      nb_low_var++;
      if (learning_rates[j] < max_lr) {
        consec_low[j]++;
        learning_rates[j] += adapt_coeff1 * consec_low[j] * learning_rates[j];
      }
    }
  }
  old_var << store_var_grad;
#elif defined(HACK_OPT_LR)
  for (int j=0; j<params.nelems(); j++) {
    input_lr[0] = store_mean_grad[j];
    input_lr[1] = store_var_grad[j];
    neural_lr.use(input_lr, predic_lr);
    learning_rates[j] = predic_lr[0] * adapt_coeff1;
    if (learning_rates[j] < min_lr) {
      learning_rates[j] = min_lr;
      nb_low_var++;
    } else if (learning_rates[j] > max_lr) {
      learning_rates[j] = max_lr;
      nb_high_var++;
    }
  }
#elif defined(HACK_FORMULA_GRAD_VAR)
  for (int j=0; j<params.nelems(); j++) {
    learning_rates[j] = adapt_coeff1 * store_mean_grad[j] - adapt_coeff2 * store_var_grad[j];
    if (learning_rates[j] < min_lr) {
      nb_high_var++;
      learning_rates[j] = min_lr;
    } else if (learning_rates[j] > max_lr) {
      nb_low_var++;
      learning_rates[j] = max_lr;
    }
  }
#elif defined(HACK_2_LR_VARIANCE) || defined(HACK_2_DC_VARIANCE)
#if defined(HACK_2_LR_VARIANCE)
  real var_limit = HACK_2_LR_VARIANCE;
#elif defined(HACK_2_DC_VARIANCE)
  real var_limit = HACK_2_DC_VARIANCE;
#endif // if defined(HACK_2_LR_VARIANCE)
  for (int j=0; j<params.nelems(); j++) {
    if (store_var_grad[j] <= moy_var * var_limit) {
#if defined(HACK_2_LR_VARIANCE)
      learning_rates[j] = max_lr;
#elif defined(HACK_2_DC_VARIANCE)
      decrease_constants[j] = 1 / (1.0 + stage * adapt_coeff1);
#endif // if defined(HACK_2_LR_VARIANCE)
      nb_low_var++;
    } else {
#if defined(HACK_2_LR_VARIANCE)
      learning_rates[j] = min_lr;
#elif defined(HACK_2_DC_VARIANCE)
      decrease_constants[j] = 1 / (1.0 + stage * adapt_coeff2);
#endif // if defined(HACK_2_LR_VARIANCE)
      nb_high_var++;
    }
  }
#elif defined(HACK_LINEAR_LR_VARIANCE)
  real var_limit = 1.2;
  int nb_interpol_var = 0;
  real max_var = moy_var * var_limit;
  real min_var = moy_var / var_limit;
  for (int j=0; j<params.nelems(); j++) {
    if (store_var_grad[j] >= max_var) {
      learning_rates[j] = min_lr;
      nb_high_var++;
    } else if (store_var_grad[j] <= min_var) {
      learning_rates[j] = max_lr;
      nb_low_var++;
    } else {
      // linear interpolation
      learning_rates[j] = (store_var_grad[j] - min_var) / (max_var - min_var) * (max_lr - min_lr) + min_lr;
      nb_interpol_var++;
    }
  }
  cout << "Nb interpol = " << nb_interpol_var << " -- ";
#endif // if defined(HACK_ADAPT_VARIANCE)
  cout << "Nb low var = " << nb_low_var << " -- Nb high var = " << nb_high_var << endl;
#endif // defined(HACK_2_LR_VARIANCE) || defined(HACK_LINEAR_LR_VARIANCE)

  // TODO HackSource to remove
/*  for (int j=0; j<params.nelems(); j++) {
    if (j < 25*263)
      learning_rates[j] = min_lr;
    else
      learning_rates[j] = max_lr;
  } */

#ifdef HACK_2_LR_LAYER
  for (int j=0; j<HACK_2_LR_LAYER; j++) {
    learning_rates[j] = adapt_coeff1;
  }
  for (int j=HACK_2_LR_LAYER; j<params.nelems(); j++) {
    learning_rates[j] = adapt_coeff2;
  }
#endif

#ifdef HACK_SUDDEN_DEC
  count_epochs++;
  if (count_epochs >= adapt_coeff1) {
    if (count_epochs == (int) adapt_coeff1) {
      for (int j=0; j<params.nelems(); j++) {
        lr_before_sd[j] = learning_rates[j];
      }
    }
    for (int j=0; j<params.nelems(); j++) {
      learning_rates[j] = (count_epochs - adapt_coeff1) * (0 - lr_before_sd[j]) / adapt_coeff2 + lr_before_sd[j];
      if (learning_rates[j] < 0) {
        learning_rates[j] = 0;
      }
    }
  }
#endif

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

//    if (!no_compute && stage >= 18001 * 50) no_compute = true;

    // Take into account the learning rate decrease
    // TODO Incorporate it into other learning rate adaptation kinds ?
    double minlr = 0;
    switch (learning_rate_adaptation) {
      case 0:
#ifdef HACK_CST_SUDDEN_DEC
        if (stage > (int) (adapt_coeff1)) {
          learning_rate = lr_before_sd[0];
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
        minlr = bidlr(minlr);
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

#if defined(HACK_2_LR_VARIANCE) || defined(HACK_LINEAR_LR_VARIANCE) || defined(HACK_ADAPT_VARIANCE) || defined(HACK_FORMULA_GRAD_VAR) || defined(HACK_OPT_LR)
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
    real b = 0.0;
    coeff = 1.0;
    if (stage >= (int) adapt_coeff1) {
      if (stage == (int) adapt_coeff1) {
        lr_before_sd[0] = start_learning_rate / (1.0 + stage * decrease_constant);
      }
      coeff = - (real(stage) - adapt_coeff1) / (adapt_coeff2);
      b = lr_before_sd[0];
    }
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
//        if (!no_compute) {
#if defined(HACK_2_DC_VARIANCE)
        params.update(learning_rates, gradient, decrease_constants);
#elif defined(HACK_FORMULA_GRAD_VAR) || defined(HACK_OPT_LR) || defined(HACK_DECREASING_BOUNDS)
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
          if (altern_lr) {
            if (count_alt == 5) {
              altern_lr = false;
              count_alt = 0;
            }
            start_learning_rate = original_start_lr * (1.0 + HACK_ALTERNATE_LR);
            count_alt++;
          } else {
            if (count_alt == 5) {
              altern_lr = true;
              count_alt = 0;
            }
            start_learning_rate = original_start_lr;
            count_alt++;
          }
#endif
          break;
        case 1:
          adaptLearningRateBasic(learning_rates, tmp_storage, old_evol);
          very_old_params << tmp_storage;
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
