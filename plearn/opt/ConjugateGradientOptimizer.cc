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
   * $Id: ConjugateGradientOptimizer.cc,v 1.1 2003/04/11 22:04:05 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ConjugateGradient.h"
#include "ConjugateGradientOptimizer.h"

namespace PLearn <%
using namespace std;

//
// Constructors
//
ConjugateGradientOptimizer::ConjugateGradientOptimizer(
    real the_starting_step_size, 
    real the_epsilon,
    int n_updates, const string& filename, 
    int every_iterations)
  :inherited(n_updates, filename, every_iterations),
  starting_step_size(the_starting_step_size),
  epsilon(the_epsilon) {}

ConjugateGradientOptimizer::ConjugateGradientOptimizer(
    VarArray the_params, 
    Var the_cost,
    real the_starting_step_size, 
    real the_epsilon,
    int n_updates, const string& filename, 
    int every_iterations)
  :inherited(the_params, the_cost, n_updates, filename, every_iterations),
  starting_step_size(the_starting_step_size),
  epsilon(the_epsilon) {}

ConjugateGradientOptimizer::ConjugateGradientOptimizer(
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
void ConjugateGradientOptimizer::declareOptions(OptionList& ol)
{
    declareOption(ol, "starting_step_size", &ConjugateGradientOptimizer::starting_step_size, OptionBase::buildoption, 
                  "    the initial step size for the line search algorithm\n");

    declareOption(ol, "epsilon", &ConjugateGradientOptimizer::epsilon, OptionBase::buildoption, 
                  "    the gradient resolution\n");

    inherited::declareOptions(ol);
}

//
// oldwrite
//
void ConjugateGradientOptimizer::oldwrite(ostream& out) const
{
  writeHeader(out, "ConjugateGradientOptimizer", 0);
  inherited::write(out);  
  writeField(out, "starting_step_size", starting_step_size);
  writeField(out, "epsilon", epsilon);
  writeFooter(out, "ConjugateGradientOptimizer");
}

//
// oldread
//
void ConjugateGradientOptimizer::oldread(istream& in)
{
  int ver = readHeader(in, "ConjugateGradientOptimizer");
  if(ver!=0)
    PLERROR("In ConjugateGradientOptimizer::read version number %d not supported",ver);
  inherited::oldread(in);
  readField(in, "starting_step_size", starting_step_size);
  readField(in, "epsilon", epsilon);
  readFooter(in, "ConjugateGradientOptimizer");
}

//
// Name and DeepCopy
//
IMPLEMENT_NAME_AND_DEEPCOPY(ConjugateGradientOptimizer);

//
// optimize
//
real ConjugateGradientOptimizer::optimize()
{
  ofstream out;
  if (!filename.empty()) {
     out.open(filename.c_str());
//     out << " Stochastic! " << endl;  // TODO what's the purpose ?
  }
  Vec meancost(cost->size());
  Vec lastmeancost(cost->size());
  early_stop = false;

  params.clearGradient(); // TODO what's the purpose ?

  // Initiliazation of the structures for the CONJPOMDP algorithm
  Vec g(params.sumOfLengths());
  Vec h(params.sumOfLengths());
  Vec delta(params.sumOfLengths());
  computeGradient(params, cost, proppath, g);

  // Loop through the epochs
  for (int t=0; !early_stop && t<nupdates; t++) {

    // Make one iteration through the CONJPOMDP algorithm
    early_stop = ConjugateGradient::conjpomdp(
        computeGradient, 
        params, 
        cost,
        proppath, 
        starting_step_size, 
        epsilon,
        g,
        h,
        delta);

    // Display results TODO ugly copy/paste : to be cleaned ?
    meancost += cost->value;
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
      early_stop = early_stop || measure(t+1,meancost); // TODO is it a good idea to use this ?
      early_stop_i = (t+1)/every;
      lastmeancost << meancost;
      meancost.clear();
    }
  }
  return lastmeancost[0];
}

//
// computeGradient
//
// Given a propagation path and the parameters params,
// compute the gradient and store it in the "gradient" Vec
//
void ConjugateGradientOptimizer::computeGradient(
    VarArray params,
    Var cost,
    VarArray proppath,
    const Vec& gradient) {
  proppath.clearGradient(); // TODO What does that do exactly ?
  cost->gradient[0] = 1; // TODO Is this the right choice ?
  proppath.fbprop();
  params.copyGradientTo(gradient);
}

%> // end of namespace PLearn
