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
   * $Id: ConjugateGradient.cc,v 1.1 2003/04/11 22:04:05 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ConjugateGradient.h"
#include "LineSearch.h"
#include "TMat_maths_impl.h"

namespace PLearn <%
using namespace std;

//
// conjpomdp
//
bool ConjugateGradient::conjpomdp (void (*grad)(VarArray, Var, VarArray, const Vec&),
                          VarArray params,
                          Var costs,
                          VarArray proppath,
                          real starting_step_size,
                          real epsilon,
                          Vec g,
                          Vec h,
                          Vec delta) {
  // First perform a line search in the current search direction (h)
  LineSearch::gSearch(grad, params, costs, proppath, h, starting_step_size, epsilon);

  (*grad)(params, costs, proppath, delta);
  real gamma = dot(delta - g, delta) / pownorm(g);
  h = delta + gamma * h;
  if (dot(h, delta) < 0) {
    h = delta;
  }
  g = delta;
  // We stop when the norm of the gradient is small enough
  return (pownorm(g) < epsilon);
};

%> // end of namespace PLearn
