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
   * $Id: ConjugateGradient.h,v 1.2 2003/04/14 18:50:45 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef CONJUGATEGRADIENT_INC
#define CONJUGATEGRADIENT_INC

#include "pl_math.h"
#include "VarArray.h"

namespace PLearn <%
using namespace std;

/*
 * CLASS CONNJUGATEGRADIENT
 *
 * This class implements conjugate gradient algorithms.
 * (well, only one so far)
 *
 */
class ConjugateGradient {

  public:

    // The CONJPOMDP algorithm as described in
    // "Direct Gradient-Based Reinforcement Learning:
    // II. Gradient Ascent Algorithms and Experiments"
    // by J.Baxter, L. Weaver, P. Bartlett.
    // This is just one single iteration of the algorithm.
    // Return true when the convergence is obtained.
    static bool conjpomdp (
        // the given grad function needs to compute the gradient
        void (*grad)(VarArray params, Var cost, VarArray proppath, const Vec& gradient),
        VarArray params,    // the current parameters of the model
        Var costs,          // the cost to optimize
        VarArray proppath,  // the propagation path from params to costs
        real starting_step_size, // the initial step size for the line search
        real epsilon,       // the gradient resolution (stop criterion)
        Vec g,              // storage place, first initialized as the gradient
        Vec h,              // same as g
        Vec delta);         // storage place, size of the gradient
    // TODO Would be better to have a function to initialize g
};

%> // end of namespace PLearn

#endif 
