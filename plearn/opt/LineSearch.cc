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
   * $Id: LineSearch.cc,v 1.1 2003/04/11 22:04:06 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include <iostream.h>

#include "LineSearch.h"
#include "TMat_maths_impl.h"
#include "VarArray.h"

namespace PLearn <%
using namespace std;


//
// gSearch
//
void LineSearch::gSearch (void (*grad)(VarArray, Var, VarArray, const Vec&),
                                   VarArray params,
                                   Var costs,
                                   VarArray proppath,
                                   Vec search_direction,
                                   real starting_step_size,
                                   real epsilon) {
  real step = starting_step_size;
  real sp, sm, pp, pm;
  Vec initial_values(params.sumOfLengths());

  // Backup the initial paremeters values
  params.copyTo(initial_values);

  params.update(step, search_direction);
  Vec delta(params.sumOfLengths());
  (*grad)(params, costs, proppath, delta);
  real prod = dot(delta, search_direction);

  if (prod > 0) {
    // Step back to bracket the minimum
    while (prod > epsilon) {
      sp = step;
      pp = prod;
      step = step / 2;
      // TODO Find a more efficient way to do this !
      params.copyFrom(initial_values);
      params.update(step, search_direction);
      (*grad)(params, costs, proppath, delta);
    }
    sm = step;
    prod = dot(delta, search_direction);
    pm = prod;
  }
  else {
    // Step forward to bracket the maximum
    while (prod < -epsilon) {
      sm = step;
      pm = prod;
      step = step * 2;
      params.copyFrom(initial_values);
      params.update(step, search_direction);
      (*grad)(params, costs, proppath, delta);
    }
    sp = step;
    prod = dot(delta, search_direction);
    pp = prod;
  }

  if (pm < 0 && pp > 0)
    step = (sm*pp - sp*pm) / (pp - pm);
  else
    step = (sm+sp) / 2;

  params.copyFrom(initial_values);
  params.update(step, search_direction);
}

%> // end of namespace PLearn
      

