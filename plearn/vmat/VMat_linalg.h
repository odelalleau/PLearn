// -*- C++ -*-

// VMat_linalg.h
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
   * $Id: VMat_linalg.h,v 1.1 2004/09/27 20:19:28 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file VMat_linalg.h */


#ifndef VMat_linalg_INC
#define VMat_linalg_INC

#include <plearn/math/TMat.h>
#include <plearn/math/TMat_maths_impl.h>

namespace PLearn {
using namespace std;

  class VMat;

//!  computes M1'.M2
Mat transposeProduct(VMat m1, VMat m2);

//!  computes M'.M
Mat transposeProduct(VMat m);

//!  computes M1'.V2
Vec transposeProduct(VMat m1, Vec v2);

//!  computes M1.M2'
Mat productTranspose(VMat m1, VMat m2);

//!  computes M1.M2
Mat product(Mat m1, VMat m2);

//!  returns M1'
VMat transpose(VMat m1);

/*!   computes the result of the linear regression into theta_t
  Parameters must have the following sizes:
  inputs(l,n)
  outputs(l,m)
  theta_t(n+1,m)
  XtX(n+1,n+1)
  XtY(n+1,m)
  The n+1 is due to the inclusion of the bias terms in the matrices (first row of theta_t)
  If use_precomputed_XtX_XtY is false, then they are computed. Otherwise
  they are used as they are (typically passed precomputed from a previous
  call made with a possibly different weight_decay).
  Returns average of squared loss.
*/
real linearRegression(VMat inputs, VMat outputs, real weight_decay, Mat theta_t, 
                      bool use_precomputed_XtX_XtY, Mat XtX, Mat XtY, real& sum_squared_Y,
                      bool return_squared_loss=false, int verbose_computation_every=0,
                      bool cholesky = true);
                      

//!  Version that does all the memory allocations of XtX, XtY and theta_t. Returns theta_t
Mat linearRegression(VMat inputs, VMat outputs, real weight_decay);

//! Linear regression where each input point is given a different importance weight (the gammas);
//! returns weighted average of squared loss
real weightedLinearRegression(VMat inputs, VMat outputs, VMat gammas,
                              real weight_decay, Mat theta_t, bool use_precomputed_XtX_XtY, Mat XtX,
                              Mat XtY, real& sum_squared_Y, real& sum_gammas, bool return_squared_loss=false, 
                              int verbose_computation_every=0, bool cholesky = true);

//!  Version that does all the memory allocations of XtX, XtY and theta_t. Returns theta_t
Mat weightedLinearRegression(VMat inputs, VMat outputs, VMat gammas, real weight_decay);


} // end of namespace PLearn

#endif
