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
   * $Id: ConjugateGradientOptimizer.h,v 1.1 2003/04/11 22:04:06 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


#ifndef CONJUGATEGRADIENTOPTIMIZER_INC
#define CONJUGATEGRADIENTOPTIMIZER_INC

#include "Optimizer.h"

namespace PLearn <%
using namespace std;


/*
 * CLASS CONJUGATEGRADIENTOPTIMIZER
 *
 * An optimizer using the conjugate gradient method.
 * Currently only implements one method, but will hopefully evolve to be able
 * to use other algorithms.
 *
 */
class ConjugateGradientOptimizer : public Optimizer {

  typedef Optimizer inherited;

public:

  // Options (also available through setOption)
  real starting_step_size;  // initial step for line search
  real epsilon;             // gradient resolution

  // Constructors and other usual stuff
  ConjugateGradientOptimizer(
      real the_starting_step_size=0.01, 
      real the_epsilon=0.01,
      int n_updates=1, const string& filename="", 
      int every_iterations=1);
  
  ConjugateGradientOptimizer(
      VarArray the_params, 
      Var the_cost,
      real the_starting_step_size=0.01, 
      real the_epsilon=0.01,
      int n_updates=1, const string& filename="", 
      int every_iterations=1);

  ConjugateGradientOptimizer(
      VarArray the_params, 
      Var the_cost, 
      VarArray the_update_for_measure,
      real the_starting_step_size=0.01, 
      real the_epsilon=0.01,
      int n_updates=1, const string& filename="", 
      int every_iterations=1);

  DECLARE_NAME_AND_DEEPCOPY(ConjugateGradientOptimizer);

  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies) { inherited::makeDeepCopyFromShallowCopy(copies); }

  virtual void build()
  {
    inherited::build();
    build_();
  }

private:

  void build_() {}
    
public:

  virtual void oldwrite(ostream& out) const;
  virtual void oldread(istream& in);
  virtual real optimize();

  // Function used to compute the gradient
  // The gradient is stored in the last parameter
  static void computeGradient(
      VarArray params,
      Var cost,
      VarArray proppath,
      const Vec& gradient);

protected:
  static void declareOptions(OptionList& ol);

};

%> // end of namespace PLearn

#endif
