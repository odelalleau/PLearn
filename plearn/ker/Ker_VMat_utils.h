// -*- C++ -*-

// Ker_VMat_utils.h
//
// Copyright (C) 2004 Pascal Vincent 
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
   * $Id: Ker_VMat_utils.h,v 1.1 2004/09/27 20:19:27 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file Ker_VMat_utils.h */


#ifndef Ker_VMat_utils_INC
#define Ker_VMat_utils_INC

// Put includes here
//#include <pair>
#include <plearn/math/TVec.h>

namespace PLearn {
using namespace std;

  class Ker;
  class VMat;

/*!     The following methods can be used in a straightforward manner to compute a variety of useful things:
    Dot products between this vmat and a vector, find the K nearest neighbours to a vector, etc...
    Most methods take an optional last parameter ignore_this_row which may contain the index of a row that
    is to be excluded from the computation (this can be seful for leave-one-out evaluations for instance).
*/

/*!     This will compute for this vmat m a result vector (whose length must be tha same as m's)
    s.t. result[i] = ker( m(i).subVec(v1_startcol,v1_ncols) , v2) 
    i.e. the kernel value betweeen each (sub)row of m and v2
*/
 void evaluateKernel(Ker ker, VMat vm, int v1_startcol, int v1_ncols, 
                              const Vec& v2, const Vec& result, int startrow=0, int nrows=-1);

  //!   returns sum_i [ ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
 real evaluateKernelSum(Ker ker, VMat vm, int v1_startcol, int v1_ncols, 
                                 const Vec& v2, int startrow=0, int nrows=-1, int ignore_this_row=-1);

  //!  targetsum := sum_i [ m(i).subVec(t_startcol,t_ncols) * ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
  //!  and returns sum_i [ ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
 real evaluateKernelWeightedTargetSum(Ker ker, VMat vm, int v1_startcol, int v1_ncols, const Vec& v2, 
                                               int t_startcol, int t_ncols, Vec& targetsum, int startrow=0, int nrows=-1, int ignore_this_row=-1);
  
   
/*!     This will return the Top N kernel evaluated values (between vmat (sub)rows and v2) and their associated row_index.
    Result is returned as a vector of length N of pairs (kernel_value,row_index)
    Results are sorted with largest kernel value first
*/
 TVec< pair<real,int> > evaluateKernelTopN(int N, Ker ker, VMat vm, int v1_startcol, int v1_ncols, 
                                                    const Vec& v2, int startrow=0, int nrows=-1, int ignore_this_row=-1);

  //!  same as evaluateKernelTopN but will look for the N smallest values instead of top values.
  //!  results are sorted with smallest kernel value first
 TVec< pair<real,int> > evaluateKernelBottomN(int N, Ker ker, VMat vm, int v1_startcol, int v1_ncols, 
                                                       const Vec& v2, int startrow=0, int nrows=-1, int ignore_this_row=-1);



} // end of namespace PLearn

#endif
