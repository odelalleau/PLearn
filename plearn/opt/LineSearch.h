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
   * $Id: LineSearch.h,v 1.1 2003/04/11 22:04:06 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef LINESEARCH_INC
#define LINESEARCH_INC

#include "pl_math.h"
#include "VarArray.h"

//class VarArray;
//class Vec;   // TODO Is it possible to use forward declaration ?

namespace PLearn <%
using namespace std;

/*
 * CLASS LINESEARCH
 *
 * This class implements line search algorithms for use in Conjugate Gradient
 * methods for instance.
 * (well, only one little algorithm to start with)
 *
 */
class LineSearch {

  public:

    // The GSearch algorithm as described in
    // "Direct Gradient-Based Reinforcement Learning:
    // II. Gradient Ascent Algorithms and Experiments"
    // by J.Baxter, L. Weaver, P. Bartlett.
    // See ConjugateGradient.h for more explainations on the parameters.
    static void gSearch (void (*grad)(VarArray, Var, VarArray, const Vec&),
                              VarArray params,
                              Var costs,
                              VarArray proppath,
                              Vec search_direction, // current direction
                              real starting_step_size,
                              real epsilon);
};

%> // end of namespace PLearn

#endif 
