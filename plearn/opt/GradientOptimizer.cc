// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: GradientOptimizer.cc,v 1.13 2003/05/03 05:02:18 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "GradientOptimizer.h"
#include "TMat_maths.h"
#include "DisplayUtils.h"

namespace PLearn <%
using namespace std;

GradientOptimizer::GradientOptimizer(real the_start_learning_rate, 
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
   decrease_constant(the_decrease_constant) {}

GradientOptimizer::GradientOptimizer(VarArray the_params, Var the_cost,
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

GradientOptimizer::GradientOptimizer(VarArray the_params, Var the_cost, 
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


void GradientOptimizer::declareOptions(OptionList& ol)
{
    declareOption(ol, "start_learning_rate", &GradientOptimizer::start_learning_rate, OptionBase::buildoption, 
                  "    the initial learning rate\n");

    declareOption(ol, "min_learning_rate", &GradientOptimizer::min_learning_rate, OptionBase::buildoption, 
                  "    the minimum value for the learning rate, when there is learning rate adaptation\n");

    declareOption(ol, "max_learning_rate", &GradientOptimizer::max_learning_rate, OptionBase::buildoption, 
                  "    the maximum value for the learning rate, when there is learning rate adaptation\n");

    declareOption(ol, "adapt_coeff1", &GradientOptimizer::adapt_coeff1, OptionBase::buildoption, 
                  "    a coefficient for learning rate adaptation, use may depend on the kind of adaptation\n");

    declareOption(ol, "adapt_coeff2", &GradientOptimizer::adapt_coeff2, OptionBase::buildoption, 
                  "    a coefficient for learning rate adaptation, use may depend on the kind of adaptation\n");

    declareOption(ol, "decrease_constant", &GradientOptimizer::decrease_constant, OptionBase::buildoption, 
                  "    the learning rate decrease constant \n");

    declareOption(ol, "learning_rate_adaptation", &GradientOptimizer::learning_rate_adaptation, OptionBase::buildoption, 
                  "    the way the learning rates evolve\n");

    inherited::declareOptions(ol);
}

  /*
string GradientOptimizer::help() const
{
  return 
    "GradientOptimizer is the simple usual gradient descent algorithm \n"
    " (the number of samples on which to estimate gradients before an \n"
    "  update, which determines whether we are performing 'batch' \n"
    "  'stochastic' or even 'minibatch', is currently specified outside \n"
    "  this class, typically in the numer of s/amples of the meanOf function \n"
    "  to be optimized, as its 'nsamples' parameter). \n"
    "Options for GradientOptimizer are [ option_name: <type> (default) ]: \n"
    "  - start_learning_rate: <real> (0.01) \n"
    "    the initial learning rate \n"
    "  - decrease_constant: <real> (0) \n"
    "    the learning rate decrease constant \n"
    "\n"
    "GradientOptimizer derives form Optimizer. \n"
    + Optimizer::help();
}

void GradientOptimizer::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="start_learning_rate") 
    ::read(in, start_learning_rate);
  else if (optionname=="decrease_constant")
    ::read(in, decrease_constant);
  else 
    inherited::readOptionVal(in, optionname);
}



void GradientOptimizer::writeOptionVal(ostream& out, const string& optionname) const
{
  if (optionname=="start_learning_rate") 
    ::write(out, start_learning_rate);
  else if (optionname=="decrease_constant")
    ::write(out, decrease_constant);
  else 
    inherited::writeOptionVal(out, optionname);  
}
*/

//void GradientOptimizer::write(ostream& out) const
void GradientOptimizer::oldwrite(ostream& out) const
{
  writeHeader(out, "GradientOptimizer", 0);
  inherited::write(out);  
  writeField(out, "start_learning_rate", start_learning_rate);
  writeField(out, "decrease_constant", decrease_constant);
  writeFooter(out, "GradientOptimizer");
}

void GradientOptimizer::oldread(istream& in)
{
  int ver = readHeader(in, "GradientOptimizer");
  if(ver!=0)
    PLERROR("In GradientOptimizer::read version number %d not supported",ver);
  inherited::oldread(in);
  readField(in, "start_learning_rate", start_learning_rate);
  readField(in, "decrease_constant", decrease_constant);
  readFooter(in, "GradientOptimizer");
}


IMPLEMENT_NAME_AND_DEEPCOPY(GradientOptimizer);

///////////////
// optimizeN //
///////////////
bool GradientOptimizer::optimizeN(VecStatsCollector& stats_coll) {

  bool adapt = (learning_rate_adaptation != 0);
  stochastic_hack = stochastic_hack && !adapt;

  // Big hack for the special case of stochastic gradient, to avoid doing an explicit update
  // (temporarily change the gradient fields of the parameters to point to the parameters themselves,
  // so that gradients are "accumulated" directly in the parameters, thus updating them!
  if(stochastic_hack) {
    int n = params.size();
    for(int i=0; i<n; i++)
      oldgradientlocations[i] = params[i]->defineGradientLocation(params[i]->matValue);
  }

  meancost.clear();
  int niter = nstages - stage; // the number of iterations to perform

  for (; !early_stop && stage<nstages; stage++) {

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
    
    // Move along the chosen direction
    // And learning rate adaptation after each step
    switch (learning_rate_adaptation) {
      case 0:
        if (!stochastic_hack)
          params.updateAndClear();
        break;
      case 1:
        params.copyGradientTo(gradient);
        params.update(learning_rates, gradient); 
        params.clearGradient();
        break;
      case 2:
        params.copyGradientTo(gradient);
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

  meancost /= real(niter);
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

  return early_stop;
}

//////////////
// optimize //
//////////////
real GradientOptimizer::optimize()
{
  ofstream out;
  if (!filename.empty())
    {
     out.open(filename.c_str());
     out << " Stochastic! " << endl;
    }
  Vec lastmeancost(cost->size());
  early_stop = false;

  switch (learning_rate_adaptation) {
    case 0:
      break;
    case 1:
      params.copyTo(tmp_storage);
      old_evol.fill(0);
      break;
    case 2:
      Optimizer::computeGradient(this, tmp_storage);
      break;
  }
  bool adapt = (learning_rate_adaptation != 0);

  // Big hack for the special case of stochastic gradient, to avoid doing an explicit update
  // (temporarily change the gradient fields of the parameters to point to the parameters themselves,
  // so that gradients are "accumulated" directly in the parameters, thus updating them!)
  SumOfVariable* sumofvar = dynamic_cast<SumOfVariable*>((Variable*)cost);
  Array<Mat> oldgradientlocations;
  bool stochastic_hack = sumofvar!=0 && sumofvar->nsamples==1;
  // We can't use the hack with learning_rate_adaptation
  stochastic_hack = stochastic_hack && !adapt;
  // stochastic_hack=false;
  if(stochastic_hack)
  {
    int n = params.size();
    oldgradientlocations.resize(n);
    for(int i=0; i<n; i++)
      oldgradientlocations[i] = params[i]->defineGradientLocation(params[i]->matValue);
  }
  else
    params.clearGradient();

  // normally loop over number of epochs x training set size
  for (int t=0; !early_stop && t<nupdates; t++)
  {
    learning_rate = start_learning_rate/(1.0+decrease_constant*t);

    proppath.clearGradient();

    if (adapt)
      cost->gradient[0] = -1.;
    else
      cost->gradient[0] = -learning_rate;

    proppath.fbprop();//displayVarGraph(proppath, true, 333);

    meancost += cost->value;
    if ((every!=0) && ((t+1)%every==0))
      // normally this is done every epoch
    { 
      switch (learning_rate_adaptation) {
        case 0:
          break;
        case 1:
          adaptLearningRateBasic(learning_rates, tmp_storage, old_evol);
          params.copyTo(tmp_storage);
          break;
        case 2:
          break;
      }
      //cerr << ">>>>>> nupdates= " << nupdates << "  every=" << every << "  sumofvar->nsamples=" << sumofvar->nsamples << endl;
      meancost /= real(every);
      //if (decrease_constant != 0)
      //  cout << "at t=" << t << ", learning rate = " << learning_rate << endl;
      cout << t+1 << ' ' << meancost << ' ' << learning_rate << endl;
      // cout << "Learning rate : " << learning_rates[0] << "  " << learning_rates[1] << "  " << learning_rates[2] << endl;
      if (out)
        out << t+1 << ' ' << meancost << ' ' << learning_rate << endl;
      early_stop = measure(t+1,meancost);
      early_stop_i = (t+1)/every;
      lastmeancost << meancost;
      meancost.clear();
    }
    // set params += -learning_rate * params.gradient
    switch (learning_rate_adaptation) {
      case 0:
        if (!stochastic_hack)
          params.updateAndClear();
        break;
      case 1:
        params.copyGradientTo(gradient);
        params.update(learning_rates, gradient); 
        params.clearGradient();
        break;
      case 2:
        params.copyGradientTo(gradient);
        adaptLearningRateALAP1(tmp_storage, gradient);
        params.update(learning_rate, gradient);
        tmp_storage << gradient;
        params.clearGradient();
        break;
    }
  }

  if(stochastic_hack) // restore the gradients as they previously were...
    {
      int n = params.size();
      for(int i=0; i<n; i++)
        params[i]->defineGradientLocation(oldgradientlocations[i]);
    }

  return lastmeancost[0];
}

////////////////////////////
// adaptLearningRateALAP1 //
////////////////////////////
void GradientOptimizer::adaptLearningRateALAP1(
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

////////////////////////////
// adaptLearningRateBasic //
////////////////////////////
void GradientOptimizer::adaptLearningRateBasic(
    Vec learning_rates,
    Vec old_params,
    Vec old_evol) {
  Var* array = params->data();
  int j = 0;
  int k;
  int nb_min = 0;
  int nb_max = 0;
  int nb_moy = 0;
  real u;
  for (int i=0; i<params.size(); i++) {
    k = j;
    for (; j<k+array[i]->nelems(); j++) {
      u = old_evol[j];
      old_evol[j] = array[i]->valuedata[j-k] - old_params[j];
      if (u * old_evol[j] > 0)
        learning_rates[j] += learning_rates[j] * adapt_coeff1;
      else if (u * old_evol[j] < 0)
        learning_rates[j] -= learning_rates[j] * adapt_coeff2;
      if (learning_rates[j] < min_learning_rate) {
        learning_rates[j] = min_learning_rate;
        nb_min++;
      } else if (learning_rates[j] > max_learning_rate) {
        learning_rates[j] = max_learning_rate;
        nb_max++;
      } else {
        nb_moy++;
      }
      // cout << learning_rates[j] << "  ";
    }
  }
  cout << "nb_min = " << nb_min << "  --  nb_max = " << nb_max << "  --  nb_moy = " << nb_moy << endl;
  // cout << endl;
}





































/************* ScaledGradientOptimizer Stuff ************/


real ScaledGradientOptimizer::optimize()
{
  ofstream out;
  if (!filename.empty())
    out.open(filename.c_str());

  eps_scale.fill(1.0);
  Vec first_long_time_mv; 
  real best_cost = 1e30;
  Vec prev_params(gradient.length());
  Vec prev_gradient(gradient.length());
  Vec best_params(gradient.length());
  Vec best_gradient(gradient.length());
  params >> prev_params;
  params >> best_params;
  params.copyGradientTo(prev_gradient);
  params.copyGradientTo(best_gradient);
  int n_long = (int)(1.0/(short_time_mac*long_time_mac));
  cout << "start learning rate = " << start_learning_rate << endl;
  learning_rate = 0;
  Vec meancost(cost->size());
  Vec lastmeancost(cost->size());
  early_stop = false;
  for (int t=0; !early_stop && t<nupdates; t++)
  {
    params.clearGradient();
    proppath.clearGradient();
    cost->gradient[0] = 1.0;
    proppath.fbprop();
    if (every!=0) 
    {
      if ((t%every==0) && (t>0)) 
      {
        meancost /= real(every);      
        if (meancost[0] > best_cost)
        {
          start_learning_rate *= 0.5;
          params << best_params;
          params.copyGradientFrom(best_gradient);
        }
        else
        {
          best_cost = meancost[0];
          best_params << prev_params;
          best_gradient << prev_gradient;
          params >> prev_params;
          params.copyGradientTo(prev_gradient);
          start_learning_rate *= 1.1;
        }
        learning_rate = start_learning_rate/(1.0+decrease_constant*t);
        cout << t << ' ' << meancost << ' ' << learning_rate << endl;
        if (out)
          out << t << ' ' << meancost << ' ' << learning_rate << endl;
        early_stop = measure(t,meancost);
        lastmeancost << meancost;
        meancost.clear();
      }
      else
      {
        learning_rate = start_learning_rate/(1.0+decrease_constant*t);
      }
    } 
    params.copyGradientTo(gradient);
    if (t<n_long-1)
      // prepare to initialize the moving average
      // (by doing initially a batch average)
    {
      long_time_ma += gradient;
      squareAcc(long_time_mv, gradient);
    }
    else if (t==n_long-1) 
      // prepare to initialize the moving averages
    {
      long_time_ma *= real(1.0)/ (real)n_long;
      long_time_mv *= real(1.0)/ (real)n_long;
      squareMultiplyAcc(long_time_mv, long_time_ma,(real)-1);
      first_long_time_mv << long_time_mv;
      short_time_ma << long_time_ma;
    }
    else 
      // steady-state mode
    {
      exponentialMovingAverageUpdate(short_time_ma, gradient,short_time_mac);
      exponentialMovingAverageUpdate(long_time_ma, short_time_ma,long_time_mac);
      exponentialMovingSquareUpdate(long_time_mv, gradient,long_time_mac);
      if (t%n_long==0)
      {
        real prev_eps = 0.5*(max(eps_scale)+mean(eps_scale));
        //apply(long_time_mv,long_time_md,sqrt);
        cout << "******* AT T= " << t << " *******" << endl;
        cout << "average gradient norm = " 
             << norm(long_time_ma) << endl;
        cout << "average gradient = " << long_time_ma << endl;
        //cout << "short time average gradient = " << short_time_ma << endl;
        Vec long_time_md = sqrt(long_time_mv);
        cout << "sdev(gradient) = " << long_time_md << endl;
        cout << "mean(sdev(gradient)) = " << mean(long_time_md) << endl;
        add(long_time_mv,regularizer,eps_scale);
        //divide(1.0,long_time_mv,eps_scale);
        //divide(first_long_time_mv,long_time_mv,eps_scale);
        cout << "eps_scale = " << eps_scale << endl;
        real new_eps = 0.5*(max(eps_scale)+mean(eps_scale));
        start_learning_rate *= prev_eps / new_eps;
        learning_rate = start_learning_rate / (1 + decrease_constant*t);
        cout << "scale learning rate by " << prev_eps / new_eps << " to " << learning_rate << endl;

        //real *e=eps_scale.data();
        //for (int i=0;i<eps_scale.length();i++)
        //  if (e[i]>regularizer) e[i]=regularizer;
        //cout << "regularized  eps_scale = " << eps_scale << endl;
        //cout << "avg/sdev) = " << long_time_md  << endl;
        //eps_scale *= learning_rate;
        //cout << "regularized eps_scale * learning_rate = " << eps_scale << endl;
      }
    }
    // set params += -learning_rate * params.gradient
    meancost += cost->value;
    gradient *= eps_scale;
    params.update(-learning_rate,gradient);
  }
  return meancost[0];
}

bool ScaledGradientOptimizer::optimizeN(VecStatsCollector& stat_coll) {
  PLERROR("In ScaledGradientOptimizer::optimizeN this function is not implemented, use optimize instead");
  return true;
}

%> // end of namespace PLearn
