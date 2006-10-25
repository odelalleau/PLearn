// -*- C++ -*-

// qld_interface.cc
//
// Copyright (C) 2005 ApSTAT Technologies Inc. 
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
 * $Id$ 
 ******************************************************* */

// Authors: Nicolas Chapados

/*! \file qld_interface.cc */

#include "qld_interface.h"
#include "math.h"

// From PLearn
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

void qld_interface(
    Mat A,                       //!< linear constraints data matrix
    Vec B,                       //!< linear constraints constants
    int ME,                      //!< number of equality constraints
    Mat C,                       //!< objective function matrix (SPD)
    Vec D,                       //!< objective function constants
    Vec XL,                      //!< lower bounds for the variables
    Vec XU,                      //!< upper bounds for the variables
    int& iout,                   //!< desired output unit number (e.g. 1)
    int& ifail,                  //!< termination reason
    int& iprint,                 //!< output control (0=no output)
    Vec& X,                      //!< optional solution on return
    Vec& U,                      //!< lagrange multipliers on return
    Vec WAR,                     //!< real working array; resized automatically
    TVec<int> IWAR               //!< int working array; resized automatically
    )
{
    int M = A.length();
    int N = A.width();

    PLASSERT( M >= 1 && N >= 1 );
    PLASSERT( M == B.length() );
    PLASSERT( N == C.width()  &&  N == C.length() );
    PLASSERT( N == D.length() );
    PLASSERT( N == XL.length() && N == XU.length() );
  
    int MMAX  = max(M,1);
    int NMAX  = N;
    int MNN   = M + N + N;
    int LWAR  = 3 * NMAX * NMAX/2 + 10*NMAX + 2*(MMAX+1);
    int LIWAR = N;

    // In this first version, transpose matrix A
    Mat Atrans = transpose(A);
    X.resize(N);
    U.resize(MNN);
    WAR.resize(LWAR);
    IWAR.resize(LIWAR);
    IWAR[0] = 0;                   // QLD performs Cholesky itself
    double eps = DBL_EPSILON;

    ql0001_(&M,          &ME,       &MMAX,
            &N,          &NMAX,     &MNN,
            C.data(),    D.data(),  Atrans.data(),
            B.data(),    XL.data(), XU.data(),
            X.data(),    U.data(),
            &iout,       &ifail,    &iprint,
            WAR.data(),  &LWAR,
            IWAR.data(), &LIWAR,
            &eps);
}


} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
