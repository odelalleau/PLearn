// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Olivier Delalleau
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
   * $Id: ConjGradientOptimizer.cc,v 1.3 2003/04/14 20:21:56 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ConjGradientOptimizer.h"
#include "TMat_maths_impl.h"

namespace PLearn <%
using namespace std;

//
// Constructors
//
ConjGradientOptimizer::ConjGradientOptimizer(
    real the_starting_step_size, 
    real the_epsilon,
    int n_updates, const string& filename, 
    int every_iterations)
  :inherited(n_updates, filename, every_iterations),
  starting_step_size(the_starting_step_size),
  epsilon(the_epsilon) {}

ConjGradientOptimizer::ConjGradientOptimizer(
    VarArray the_params, 
    Var the_cost,
    real the_starting_step_size, 
    real the_epsilon,
    int n_updates, const string& filename, 
    int every_iterations)
  :inherited(the_params, the_cost, n_updates, filename, every_iterations),
  starting_step_size(the_starting_step_size),
  epsilon(the_epsilon) {}

ConjGradientOptimizer::ConjGradientOptimizer(
    VarArray the_params, 
    Var the_cost, 
    VarArray the_update_for_measure,
    real the_starting_step_size, 
    real the_epsilon,
    int n_updates, const string& filename, 
    int every_iterations)
  :inherited(the_params, the_cost, the_update_for_measure,
             n_updates, filename, every_iterations),
  starting_step_size(the_starting_step_size),
  epsilon(the_epsilon) {}
  
// 
// declareOptions
// 
void ConjGradientOptimizer::declareOptions(OptionList& ol)
{
    declareOption(ol, "starting_step_size", &ConjGradientOptimizer::starting_step_size, OptionBase::buildoption, 
                  "    the initial step size for the line search algorithm\n");

    declareOption(ol, "epsilon", &ConjGradientOptimizer::epsilon, OptionBase::buildoption, 
                  "    the gradient resolution\n");

    inherited::declareOptions(ol);
}

//
// oldwrite
//
void ConjGradientOptimizer::oldwrite(ostream& out) const
{
  writeHeader(out, "ConjGradientOptimizer", 0);
  inherited::write(out);  
  writeField(out, "starting_step_size", starting_step_size);
  writeField(out, "epsilon", epsilon);
  writeFooter(out, "ConjGradientOptimizer");
}

//
// oldread
//
void ConjGradientOptimizer::oldread(istream& in)
{
  int ver = readHeader(in, "ConjGradientOptimizer");
  if(ver!=0)
    PLERROR("In ConjGradientOptimizer::read version number %d not supported",ver);
  inherited::oldread(in);
  readField(in, "starting_step_size", starting_step_size);
  readField(in, "epsilon", epsilon);
  readFooter(in, "ConjGradientOptimizer");
}

//
// Implement name and deepCopy
//
IMPLEMENT_NAME_AND_DEEPCOPY(ConjGradientOptimizer);

/*************************
 * MAIN SPECIFIC METHODS *
 *************************/

/////////////////////////////
// computeOppositeGradient //
/////////////////////////////
void ConjGradientOptimizer::computeOppositeGradient(
    VarArray params,
    Var cost,
    VarArray proppath,
    const Vec& gradient) {
  // Clear all what's left from previous computations
  proppath.clearGradient();
  params.clearGradient();
  // We want the opposite of the gradient, thus the -1
  cost->gradient[0] = -1;
  proppath.fbprop();
  params.copyGradientTo(gradient);
}

///////////////
// conjpomdp //
///////////////
bool ConjGradientOptimizer::conjpomdp (
    void (*grad)(VarArray, Var, VarArray, const Vec&),
    VarArray params,
    Var costs,
    VarArray proppath,
    real starting_step_size,
    real epsilon,
    Vec g,
    Vec h,
    Vec delta,
    Vec tmp_storage) {

  int i;
  // First perform a line search in the current search direction (h)
  gSearch(grad, params, costs, proppath, h, 
          starting_step_size, epsilon, tmp_storage);

  // delta = Gradient
  (*grad)(params, costs, proppath, delta);
  real norm_g = pownorm(g);
  // g <- delta - g
  for (i=0; i<g.length(); i++) {
    g[i] = delta[i] - g[i];
  }
  real gamma = dot(g, delta) / norm_g;
  // h <- delta + gamma * h
  for (i=0; i<h.length(); i++) {
    h[i] = delta[i] + gamma * h[i];
  }
  if (dot(h, delta) < 0) {
    // h <- delta
    h << delta;
  }
  // g <- delta
  g << delta;
  // We want to stop when the norm of the gradient is small enough
  return (pownorm(g) < epsilon);
};

/////////////
// gSearch //
/////////////
void ConjGradientOptimizer::gSearch (
    void (*grad)(VarArray, Var, VarArray, const Vec&),
    VarArray params,
    Var costs,
    VarArray proppath,
    Vec search_direction,
    real starting_step_size,
    real epsilon,
    Vec tmp_storage) {

  real step = starting_step_size;
  real sp, sm, pp, pm;

  // Backup the initial paremeters values
  params.copyTo(tmp_storage);

  params.update(step, search_direction);
  Vec delta(params.nelems());
  (*grad)(params, costs, proppath, delta);
  real prod = dot(delta, search_direction);

  if (prod < 0) {
    // Step back to bracket the maximum
    while (prod < -epsilon) {
      sp = step;
      pp = prod;
      step = step / 2;
      params.update(-step, search_direction);
      (*grad)(params, costs, proppath, delta);
      prod = dot(delta, search_direction);
    }
    sm = step;
    pm = prod;
  }
  else {
    // Step forward to bracket the maximum
    while (prod > epsilon) {
      sm = step;
      pm = prod;
      params.update(step, search_direction);
      (*grad)(params, costs, proppath, delta);
      prod = dot(delta, search_direction);
      step = step * 2;
    }
    sp = step;
    pp = prod;
  }

  if (pm > 0 && pp < 0)
    step = (sm*pp - sp*pm) / (pp - pm);
  else
    step = (sm+sp) / 2;

  params.copyFrom(tmp_storage);
  params.update(step, search_direction);
}

//////////////
// optimize //
//////////////
real ConjGradientOptimizer::optimize()
{
  ofstream out;
  if (!filename.empty()) {
     out.open(filename.c_str());
//     out << " Stochastic! " << endl;  // TODO what's the purpose ?
  }
  Vec meancost(cost->size());
  Vec lastmeancost(cost->size());
  early_stop = false;

  // Initiliazation of the structures for the CONJPOMDP algorithm
  Vec g(params.nelems());
  Vec h(params.nelems());
  Vec delta(params.nelems());
  Vec tmp_storage(params.nelems());
  computeOppositeGradient(params, cost, proppath, h); 
  g << h;
  cout << "nupdates =" << nupdates << endl;

  // Loop through the epochs
  for (int t=0; !early_stop && t<nupdates; t++) {

    // Make one iteration through the CONJPOMDP algorithm
    cost->fprop();
    cout << "cost = " << cost->value[0] << " " << cost->value[1] << " " << cost->value[2] << endl;
    early_stop = conjpomdp(
        computeOppositeGradient, 
        params, 
        cost,
        proppath, 
        starting_step_size, 
        epsilon,
        g,
        h,
        delta,
        tmp_storage);
//    early_stop = false; // TODO hack for test purpose

    // Display results TODO ugly copy/paste : to be cleaned ?
    meancost += cost->value;
    every = 2000; // TODO hack for test purpose
    if ((every!=0) && ((t+1)%every==0)) 
      // normally this is done every epoch
    { 
      //cerr << ">>>>>> nupdates= " << nupdates << "  every=" << every << "  sumofvar->nsamples=" << sumofvar->nsamples << endl;
      meancost /= real(every);
      //if (decrease_constant != 0)
      //  cout << "at t=" << t << ", learning rate = " << learning_rate << endl;
      cout << t+1 << ' ' << meancost << endl;
      if (out)
        out << t+1 << ' ' << meancost << endl;
      early_stop = early_stop || measure(t+1,meancost);
     // early_stop = measure(t+1,meancost); // TODO which is the best between this and the one above ?
      early_stop_i = (t+1)/every;
      lastmeancost << meancost;
      meancost.clear();
    }
  }
  return lastmeancost[0];
}

%> // end of namespace PLearn
