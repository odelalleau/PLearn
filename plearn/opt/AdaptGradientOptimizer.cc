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
   * $Id: AdaptGradientOptimizer.cc,v 1.4 2003/05/22 16:30:17 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "AdaptGradientOptimizer.h"
#include "AsciiVMatrix.h" // TODO Remove later
#include "TMat_maths.h"
#include "DisplayUtils.h"

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
   mini_batch(0)
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

    inherited::declareOptions(ol);
}

IMPLEMENT_NAME_AND_DEEPCOPY(AdaptGradientOptimizer);

////////////////////////////
// adaptLearningRateALAP1 //
////////////////////////////
void AdaptGradientOptimizer::adaptLearningRateALAP1(
    Vec old_gradient,
    Vec new_gradient) {
  int j = 0; // the current index in learning_rates
  int k;
  real prod = 0;
  Var* array = params->data();
  for (int i=0; i<params.size(); i++) {
    k = j;
    for (; j<k+array[i]->nelems(); j++) {
      prod += old_gradient[j] * new_gradient[j];
    }
  }
  // The division by j=params.nelems() is a scaling coeff
  learning_rate = learning_rate + adapt_coeff1 * prod / j;
  if (learning_rate < min_learning_rate) {
    learning_rate = min_learning_rate;
  } else if (learning_rate > max_learning_rate) {
    learning_rate = max_learning_rate;
  }
}

static bool first_time = true;
static Vec old_mean_gradient;
static VecStatsCollector store_gradient;
// TODO Remove later (?)

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
    old_mean_gradient.resize(params.nelems());
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
  Vec tmp_grad_stuff(params.nelems());
  tmp_grad_stuff << old_mean_gradient;
//  old_mean_gradient << store_gradient.getMean();
//  store_gradient.forget();
  for (int i=0; i<params.size(); i++) {
    k = j;
    for (; j<k+array[i]->nelems(); j++) {
      u = old_evol[j];
 /*     real x2 = array[i]->valuedata[j-k]; // the new parameter
      real x1 = old_params[j];            // the old parameter
      real g2 = old_mean_gradient[j];     // the new gradient
      real g1 = tmp_grad_stuff[j];        // the old gradient
      if (g2 == g1)
        cout << "Warning ! g2 == g1 !" << endl; */
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
      real coeff = min(10.0, abs(old_evol[j]));
//      old_evol[j] = array[i]->valuedata[j-k] - old_params[j];
      if (u * old_evol[j] > 0)
        learning_rates[j] += learning_rates[j] * adapt_coeff1; // * coeff;
      else if (u * old_evol[j] < 0) {
//        learning_rates[j] = (x2 - x1) / (g2 - g1) / 18000;
//        cout << learning_rates[j] << endl;
//        if (g2 * g1 >0)
//          cout << "Warning ! g2 and g1 have same sign !" << endl;
        learning_rates[j] -= learning_rates[j] * adapt_coeff2;
//        array[i]->valuedata[j-k] = old_params[j];
//        old_evol[j] = 0;
      }
      real min_lr = min_learning_rate / (1 + stage * decrease_constant);
      real max_lr = max_learning_rate / (1 + stage * decrease_constant);
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
  if (nb_min > 0) lr_min /= real(nb_min);
  if (nb_moy > 0) lr_moy /= real(nb_moy);
  if (nb_max > 0) lr_max /= real(nb_max);
  cout << "nb_min = " << nb_min << "  --  nb_max = " << nb_max << "  --  nb_moy = " << nb_moy << endl;
  cout << "w_lr_min = " << lr_min << "  --  w_lr_max = " << lr_max << " --  w_lr_moy = " << lr_moy << "  --  w_lr_mean = " << lr_mean/real(nb_min+nb_moy+nb_max) << endl; 
  // cout << endl;
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
  stochastic_hack = false; // TODO Remove later

  // Big hack for the special case of stochastic gradient, to avoid doing an explicit update
  // (temporarily change the gradient fields of the parameters to point to the parameters themselves,
  // so that gradients are "accumulated" directly in the parameters, thus updating them!
  if(stochastic_hack) {
    int n = params.size();
    for(int i=0; i<n; i++)
      oldgradientlocations[i] = params[i]->defineGradientLocation(params[i]->matValue);
  }

  meancost.clear();
  
  int stage_max = stage + nstages; // the stage to reach

  for (; !early_stop && stage<stage_max; stage++) {

    // Take into account the learning rate decrease
    // TODO Incorporate it into other learning rate adaptation kinds ?
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

    meancost += cost->value;

    // TODO Put back in the switch below when no more gradient stats
    // are collected ?
    params.copyGradientTo(gradient);
//    store_gradient.update(gradient);
    //collectGradientStats(gradient);
    
    // Move along the chosen direction
    // And learning rate adaptation after each step
    switch (learning_rate_adaptation) {
      case 0:
        if (!stochastic_hack)
          params.updateAndClear();
        break;
      case 1:
        // params.copyGradientTo(gradient);
        // TODO Not efficient, write a faster update ?
        params.update(learning_rates, gradient); 
        params.clearGradient();
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

    stats_coll.update(cost->value);
  }

  meancost /= real(nstages);

  cout << stage << " : " << meancost << endl;
  early_stop = measure(stage+1,meancost);

  // Learning rate adaptation after a full epoch
  switch (learning_rate_adaptation) {
    case 1:
      adaptLearningRateBasic(learning_rates, tmp_storage, old_evol);
      params.copyTo(tmp_storage);
      break;
    default:
      break;
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
