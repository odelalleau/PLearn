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
   * $Id: AdaptGradientOptimizer.cc,v 1.9 2003/05/29 15:12:47 tihocan Exp $
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

static Vec old_meancost;
static Vec old_mean_gradient;
static Vec old_old_mean_gradient;
static Vec memory_coeff;  // to store the various exponential average coeffs
static Mat exp_average;   // to store the various exponential averages
static int nb_avg = 2;    // the number of exponential averages
// static int time_constant = 50000;
static int time_constant = 5800;
static Mat arithm_average;
static Mat arithm_sum;
// static int arithm_last = 0;
static Vec sum1;
static Vec sum2;
static Vec c1;
static Vec c2;
static Vec c3;
static Mat last_grad;
// static real alpha = 0.999986;
static real alpha = 0.99988;

////////////
// build_ //
////////////
void AdaptGradientOptimizer::build_(){
  early_stop = false;
  cost_stage = 0;
  learning_rate = start_learning_rate;
  SumOfVariable* sumofvar = dynamic_cast<SumOfVariable*>((Variable*)cost);
  stochastic_hack = sumofvar!=0 && sumofvar->nsamples==1;
  params.clearGradient();
  int n = params.nelems();
  if (n > 0) {
    learning_rates.resize(n);
    gradient.resize(n);
    tmp_storage.resize(n);
    old_evol.resize(n);
    oldgradientlocations.resize(params.size());
    meancost.resize(cost->size());
    meancost.clear();
    old_meancost.resize(cost->size());
    old_mean_gradient.resize(params.nelems());
    old_old_mean_gradient.resize(params.nelems());
    learning_rates.fill(start_learning_rate);
    switch (learning_rate_adaptation) {
      case 0:
        break;
      case 1:
        // tmp_storage is used to store the old parameters
        params.copyTo(tmp_storage);
        old_evol.fill(0);
        memory_coeff.resize(nb_avg);
        exp_average.resize(n, nb_avg);
        memory_coeff[0] = 0.99990;
        memory_coeff[1] = 0.99995;
        exp_average.clear();
        sum1.resize(n);
        sum2.resize(n);
        sum1.clear();
        sum2.clear();
        c1.resize(n);
        c1.clear();
        c2.resize(n);
        c2.clear();
        c3.resize(n);
        c3.clear();
        last_grad.resize(n,4);
        last_grad.clear();
//        arithm_average.resize(n, 2*time_constant);
 //       arithm_sum.resize(n, 2);
  //      arithm_average.clear();
   //     arithm_sum.clear();
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
//    lrates = new AsciiVMatrix("lrates.amat", params.nelems());
  }
//  lrates->appendRow(learning_rates);
  Var* array = params->data();
  int j = 0;
  int k;
  int nb_min = 0;
  int nb_max = 0;
  int nb_moy = 0;
  int nb_changes = 0;
  real u;
  real lr_min = 0;
  real lr_moy = 0;
  real lr_max = 0;
  real lr_mean = 0;
  real mean_lr = 0;
//  Vec diff_params(params.nelems());
  Vec tmp_grad_stuff(params.nelems());
//  tmp_grad_stuff << old_mean_gradient;
//  old_old_mean_gradient << old_mean_gradient;
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
 //     diff_params[j] = array[i]->valuedata[j-k] - old_params[j]- learning_rates[j] * old_mean_gradient[j] * 20000;
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
      if (u * old_evol[j] > 0)
        learning_rates[j] += learning_rates[j] * adapt_coeff1; // * coeff;
      else if (u * old_evol[j] < 0) {
//        learning_rates[j] = (x2 - x1) / (g2 - g1) / 18000;
//        cout << learning_rates[j] << endl;
//        if (g2 * g1 >0)
//          cout << "Warning ! g2 and g1 have same sign !" << endl;
        learning_rates[j] -= learning_rates[j] * adapt_coeff2;
        nb_changes++;
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
      mean_lr += learning_rates[j];
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
  cout << "mean_lr = " << mean_lr / int(params.nelems()) << "  -- nb_changes " << nb_changes << endl;
//  cout << "Norm(diff_params)^2 = " << pownorm(diff_params) << endl;
//  cout << "c(t1) = " << meancost[0]/real(stage-cost_stage) << " -- c(t0) + (t1-t0).grad(t0) = " << old_meancost[0] - dot(diff_params, old_old_mean_gradient) << " -- diff = " << abs(meancost[0]/real(stage-cost_stage) - old_meancost[0] + dot(diff_params, old_old_mean_gradient)) << endl;
//  cout << "c(t1) = " << meancost[0]/real(stage-cost_stage) << " -- c(t0) + (t1-t0).grad(t1) = " << old_meancost[0] - dot(diff_params, old_mean_gradient) << " -- diff = " << abs(meancost[0]/real(stage-cost_stage) - old_meancost[0] + dot(diff_params, old_mean_gradient)) << endl;
//  cout << "c(t1) = " << meancost[0]/real(stage-cost_stage) << " -- c(t0) + (t1-t0).grad(tx) = " << old_meancost[0] - dot(diff_params, (old_mean_gradient + old_old_mean_gradient) /2) << " -- diff = " << abs(meancost[0]/real(stage-cost_stage) - old_meancost[0] + dot(diff_params, (old_old_mean_gradient+old_mean_gradient) / 2)) << endl;
//  cout << "w_lr_min = " << lr_min << "  --  w_lr_max = " << lr_max << " --  w_lr_moy = " << lr_moy << "  --  w_lr_mean = " << lr_mean/real(nb_min+nb_moy+nb_max) << endl; 
  // cout << endl;
}

/////////////////
// computeCost //
/////////////////
void AdaptGradientOptimizer::computeCost() {
  meancost /= real(stage - cost_stage);
  cout << stage << " : " << meancost << endl;
  early_stop = measure(stage,meancost);
  old_meancost << meancost;
  meancost.clear();
  cost_stage = stage;
}

//////////////
// optimize //
//////////////
real AdaptGradientOptimizer::optimize()
{
  PLERROR("In AdaptGradientOptimizer::optimize Deprecated, use OptimizeN !");
  return 0;
}

static real d;
static int nb_min=0, nb_max=0, nb_moy=0, nb_changes=0, nb_non_changes=0;

///////////////
// optimizeN //
///////////////
bool AdaptGradientOptimizer::optimizeN(VecStatsCollector& stats_coll) {

  bool adapt = (learning_rate_adaptation != 0);
  stochastic_hack = stochastic_hack && !adapt;
//  stochastic_hack = false; // TODO Remove later

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
    real min_lr = min_learning_rate / (1 + stage * decrease_constant);
    real max_lr = max_learning_rate / (1 + stage * decrease_constant);
    bool adapt_t = (stage % time_constant == 0 && stage > 2 * time_constant);
//    int k = 0;
/*    if (adapt_t)
      k = (stage / time_constant) % 4; */
    // TODO Remove all the above stuff later
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

//        int k = (arithm_last + time_constant) % time_constant;
        for (int i=0; i<gradient.length(); i++) {
          d = gradient[i]; // * learning_rates[i];
/*          if (i >= 340)
            learning_rates[i] = min_learning_rate;
          else
            learning_rates[i] = max_learning_rate; */
/*          if (stage > 0 && stage % (100*18000) == 0) {
            if (learning_rates[i] != 0.005)
              learning_rates[i] = 0.005;
            else
              learning_rates[i] = 0.0005;
          } */
//          else if (stage > 100 * 18000)
//            learning_rates[i] = min_learning_rate / 3;
//          sum1[i] += d;
          sum1[i] = alpha*sum1[i] + (1-alpha)*d;
          /*          for (int j=0; j<nb_avg; j++) {
            exp_average(i,j) = 
            memory_coeff[j] * exp_average(i,j) + (1-memory_coeff[j]) * d;
            } */
          //          arithm_sum(i,0) = arithm_sum(i,0) - arithm_average(i,arithm_last) + d;
          //        arithm_sum(i,1) = arithm_sum(i,1) - arithm_average(i,k) + d;
          //      arithm_average(i, arithm_last) = d;
          if (adapt_t) {
/*            if (sum1[i] >= 0)
              last_grad(i,k) = 1;
            else
              last_grad(i,k) = -1;
            int nbp = 0;
            for (int j=0; j<4; j++) {
              if (last_grad(i,j) == 1)
                nbp++;
            }
              if (nbp == 2) */
            if (sum1[i] >=  0)
              if (sum2[i] >= 0)
                c1[i] = 0;
              else
                c1[i] = 1;
            else
              if (sum2[i] <= 0)
                c1[i] = 0;
              else
                c1[i] = 1;

            if (c1[i] == 1 && c2[i] == 1) {
//            if (sum1[i] * sum2[i] < 0)
              //          if (exp_average(i,0) * exp_average(i,1) < 0)
              //            if (arithm_sum(i,1) * arithm_sum(i,0) < 0) 
              // opposite average gradients
              learning_rates[i] -= learning_rates[i] * adapt_coeff2;
              //            learning_rates[i] = min_lr;
//              learning_rates[i] = start_learning_rate;
              nb_changes++;
            } else if (c1[i] == 0 && c2[i] == 0) {
//             } else if (nbp == 4 || nbp == 0) {
              learning_rates[i] += learning_rates[i] * adapt_coeff1;
              nb_non_changes++;
            }
            if (learning_rates[i] < min_lr) {
              learning_rates[i] = min_lr;
              nb_min++;
            } else if (learning_rates[i] > max_lr) {
              learning_rates[i] = max_lr;
              nb_max++;
            } else {
              nb_moy++;
            }
            sum2[i] = sum1[i];
            sum1[i] = 0;
            c3[i] = c2[i];
            c2[i] = c1[i];
          }
        }
/*        //        arithm_last++;
        if (arithm_last >= time_constant)
          arithm_last = 0; */
        if (adapt_t) {
          real lr_min = 1000;
          real lr_max = 0;
          bool alea = true;
          int n = learning_rates.length();
          Vec high_or_low(n);
          for (int i=0; i<n; i++) {
            if (learning_rates[i] < lr_min)
              lr_min = learning_rates[i];
            else if (learning_rates[i] > lr_max)
              lr_max = learning_rates[i];
          }
          real sum_l = 0, sum_h = 0;
          int nb_l = 0, nb_h = 0;
          real d = 0;
          for (int i=0; i<n; i++) {
            d = log(learning_rates[i]) - log(lr_min) - log(lr_max) + log(learning_rates[i]);
            if (d < 0) {
              sum_l += learning_rates[i];
              nb_l ++;
              high_or_low[i] = 0;
            } else if (d > 0) {
              sum_h += learning_rates[i];
              nb_h ++;
              high_or_low[i] = 1;
            } else {
              if (alea) {
                sum_l += learning_rates[i];
                nb_l ++;
                high_or_low[i] = 0;
                alea = false;
              }
              else {
                sum_h += learning_rates[i];
                nb_h ++;
                high_or_low[i] = 1;
                alea = true;
              }
            }
          }
          sum_l /= real(nb_l);
          sum_h /= real(nb_h);
          real m = exp((log(sum_l) + log(sum_h)) / 2) ;
          cout << m << endl;
          cout << "lr_min = " << lr_min << " -- lr_max = " << lr_max << " -- nb_l = " << nb_l << " -- nb_h = " << nb_h << " -- sum_l = " << sum_l << " -- sum_h = " << sum_h << endl;
          real c = nb_l / real(nb_h);
/*          if (c < 0.1) {
            // much more high learning rates than low ones
            adapt_coeff1 /= 2;
            cout << "Much more high learning rates than low ones" << endl;
            for (int i=0; i<n; i++) {
              if (high_or_low[i] == 1) {
                if (learning_rates[i] > sum_h)
                  learning_rates[i] = sum_h;
                else
                  learning_rates[i] = m;
              }
            }
          } else if (c > 10) {
            // much more low learning rates than high ones
            adapt_coeff2 /= 2;
            cout << " Much more low learning rates than high ones" << endl;
            for (int i=0; i<n; i++) {
              if (high_or_low[i] == 0) {
                if (learning_rates[i] < sum_l)
                  learning_rates[i] = sum_l;
                else
                  learning_rates[i] = m;
              }
            }
          } */
          c = sum_l / lr_max;
/*          if (c < 0.01) {
            adapt_coeff1 /= 2;
            adapt_coeff2 /= 2;
            cout << "Low and high are very different" << endl;
            for (int i=0; i<n; i++) {
              if (high_or_low[i] == 1) {
                if (learning_rates[i] > sum_h)
                  learning_rates[i] = sum_h;
                else
                  learning_rates[i] = m;
               } else {
                 if (learning_rates[i] < sum_l)
                  learning_rates[i] = sum_l;
                else
                  learning_rates[i] = m;
               }
            }
          } else if (c > 0.1) {
            adapt_coeff1 *= 2;
            adapt_coeff2 *= 2;
            cout << "Low and high are pretty close" << endl;
          }*/
        }
        if (adapt_t) {
          cout << "nb_min = " << nb_min << "  --  nb_max = " << nb_max << "  --  nb_moy = " << nb_moy << endl;
          cout << "nb_changes = " << nb_changes << " -- nb_non_changes = " << nb_non_changes << endl;
          nb_min =0;
          nb_max = 0;
          nb_moy = 0;
          nb_changes = 0;
          nb_non_changes = 0;
        }
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

//    stats_coll.update(cost->value); TODO Put back later
  }

  // Learning rate adaptation after a full epoch
  switch (learning_rate_adaptation) {
    case 1:
      // adaptLearningRateBasic(learning_rates, tmp_storage, old_evol);
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
