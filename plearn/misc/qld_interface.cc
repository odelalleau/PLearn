// -*- C++ -*-

// qld_interface.cc
//
// Copyright (C) 2004 ApSTAT Technologies Inc. 
// All rights reserved.
//
// This program may not be modified, translated, 
// copied or distributed in whole or in part or 
// as part of a derivative work, in any form, 
// whether as source code or binary code or any 
// other form of translation, except if authorized 
// by a prior agreement signed with ApSTAT Technologies Inc.

/* *******************************************************      
   * $Id: qld_interface.cc,v 1.1 2005/04/06 23:40:45 chapados Exp $ 
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

  assert( M >= 1 && N >= 1 );
  assert( M == B.length() );
  assert( N == C.width()  &&  N == C.length() );
  assert( N == D.length() );
  assert( N == XL.length() && N == XU.length() );
  
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
