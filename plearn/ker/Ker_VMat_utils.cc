// -*- C++ -*-

// Ker_VMat_utils.cc
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
   * $Id: Ker_VMat_utils.cc,v 1.1 2004/09/27 20:19:27 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file Ker_VMat_utils.cc */


#include "Ker_VMat_utils.h"
#include <plearn/ker/Kernel.h>
#include <plearn/vmat/VMat.h>
#include <plearn/math/TVec.h>
#include <plearn/math/TopNI.h>
#include <plearn/math/BottomNI.h>

namespace PLearn {
using namespace std;

    // This will compute for this vmat m a result vector (whose length must be tha same as m's)
    // s.t. result[i] = ker( m(i).subVec(v1_startcol,v1_ncols) , v2) 
    // i.e. the kernel value betweeen each (sub)row of m and v2
void evaluateKernel(Ker ker, VMat vm, int v1_startcol, int v1_ncols, 
                             const Vec& v2, const Vec& result, int startrow, int nrows)
{
  int l = vm->length();
  int endrow = (nrows>0) ?startrow+nrows :l;
  if(result.length() != endrow-startrow)
    PLERROR("In evaluateKernel length of result vector does not match the row range");

  Vec v1(v1_ncols);
  for(int i=startrow; i<endrow; i++)
  {
    vm->getSubRow(i,v1_startcol,v1);
    result[i] = ker(v1,v2);
  }
}

    //  returns sum_i [ ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
real evaluateKernelSum(Ker ker, VMat vm, int v1_startcol, int v1_ncols, 
                                const Vec& v2, int startrow, int nrows, int ignore_this_row)
{
  int l = vm->length();
  int endrow = (nrows>0) ?startrow+nrows :l;
  double result = 0.;
  Vec v1(v1_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      vm->getSubRow(i,v1_startcol,v1);
      result += ker(v1,v2);
    }
  return (real)result;
}
    
    // targetsum := sum_i [ m(i).subVec(t_startcol,t_ncols) * ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
    // and returns sum_i [ ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
real evaluateKernelWeightedTargetSum(Ker ker, VMat vm, int v1_startcol, int v1_ncols, const Vec& v2, 
                                                 int t_startcol, int t_ncols, Vec& targetsum, int startrow, int nrows, int ignore_this_row)
{
  int l = vm->length();
  int endrow = (nrows>0) ?startrow+nrows :l;
  targetsum.clear();
  double result = 0.;
  Vec v1(v1_ncols);
  Vec target(t_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      vm->getSubRow(i,v1_startcol,v1);
      vm->getSubRow(i,t_startcol,target);
      real kerval = ker(v1,v2);
      result += kerval;
      multiplyAcc(targetsum, target, kerval);
    }
  return (real)result;
}
  
TVec< pair<real,int> > evaluateKernelTopN(int N, Ker ker, VMat vm, int v1_startcol, int v1_ncols, 
                                                   const Vec& v2, int startrow, int nrows, int ignore_this_row)
{
  int l = vm->length();
  int endrow = (nrows>0) ?startrow+nrows :l;
  TopNI<real> extrema(N);
  Vec v1(v1_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      vm->getSubRow(i,v1_startcol,v1);
      real kerval = ker(v1,v2);
      extrema.update(kerval,i);
    }
  extrema.sort();
  return extrema.getTopN();
}

TVec< pair<real,int> > evaluateKernelBottomN(int N, Ker ker, VMat vm, int v1_startcol, int v1_ncols, 
                                                      const Vec& v2, int startrow, int nrows, int ignore_this_row)
{
  int l = vm->length();
  int endrow = (nrows>0) ?startrow+nrows :l;
  BottomNI<real> extrema(N);
  Vec v1(v1_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      vm->getSubRow(i,v1_startcol,v1);
      real kerval = ker(v1,v2);
      extrema.update(kerval,i);
    }
  extrema.sort();
  return extrema.getBottomN();
}




} // end of namespace PLearn
