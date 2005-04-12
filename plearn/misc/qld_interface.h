// -*- C++ -*-

// qld_interface.h
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
   * $Id: qld_interface.h,v 1.2 2005/04/12 15:32:04 tihocan Exp $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file qld_interface.h */

#ifndef qld_interface_INC
#define qld_interface_INC

// From PLearn
#include <plearn/math/TVec.h>

/**
 *  External declaration of the Fortran QL0001 routine.
 *
 * **********************************************************************
 * 
 *                     !!!! NOTICE !!!!
 * 
 * 1. The routines contained in this file are due to Prof. K.Schittkowski
 *    of the University of Bayreuth, Germany (modification of routines
 *    due to Prof. MJD Powell at the University of Cambridge).  They can
 *    be freely distributed.
 * 
 * 2. A minor modification was performed at the University of Maryland. 
 *    It is marked in the code by "c umd".
 * 
 *                                      A.L. Tits and J.L. Zhou
 *                                      University of Maryland
 * 
 * **********************************************************************
 * 
 * 
 *             SOLUTION OF QUADRATIC PROGRAMMING PROBLEMS
 * 
 * 
 * 
 *   QL0001 SOLVES THE QUADRATIC PROGRAMMING PROBLEM
 * 
 *   MINIMIZE        .5*X'*C*X + D'*X
 *   SUBJECT TO      A(J)*X  +  B(J)   =  0  ,  J=1,...,ME
 *                   A(J)*X  +  B(J)  >=  0  ,  J=ME+1,...,M
 *                   XL  <=  X  <=  XU
 *   
 *   HERE C MUST BE AN N BY N SYMMETRIC AND POSITIVE MATRIX, D AN N-DIMENSIONAL
 *   VECTOR, A AN M BY N MATRIX AND B AN M-DIMENSIONAL VECTOR. THE ABOVE 
 *   SITUATION IS INDICATED BY IWAR(1)=1. ALTERNATIVELY, I.E. IF IWAR(1)=0,
 *   THE OBJECTIVE FUNCTION MATRIX CAN ALSO BE PROVIDED IN FACTORIZED FORM.
 *   IN THIS CASE, C IS AN UPPER TRIANGULAR MATRIX.
 * 
 *   THE SUBROUTINE REORGANIZES SOME DATA SO THAT THE PROBLEM CAN BE SOLVED
 *   BY A MODIFICATION OF AN ALGORITHM PROPOSED BY POWELL (1983).
 * 
 * 
 *   USAGE:
 * 
 *      QL0001(M,ME,MMAX,N,NMAX,MNN,C,D,A,B,XL,XU,X,U,IOUT,IFAIL,IPRINT,
 *             WAR,LWAR,IWAR,LIWAR)
 * 
 * 
 *   DEFINITION OF THE PARAMETERS:
 * 
 *   M :        TOTAL NUMBER OF CONSTRAINTS.
 *   ME :       NUMBER OF EQUALITY CONSTRAINTS.
 *   MMAX :     ROW DIMENSION OF A. MMAX MUST BE AT LEAST ONE AND GREATER
 *              THAN M.
 *   N :        NUMBER OF VARIABLES.
 *   NMAX :     ROW DIMENSION OF C. NMAX MUST BE GREATER OR EQUAL TO N.
 *   MNN :      MUST BE EQUAL TO M + N + N.
 *   C(NMAX,NMAX): OBJECTIVE FUNCTION MATRIX WHICH SHOULD BE SYMMETRIC AND
 *              POSITIVE DEFINITE. IF IWAR(1) = 0, C IS SUPPOSED TO BE THE
 *              CHOLESKEY-FACTOR OF ANOTHER MATRIX, I.E. C IS UPPER
 *              TRIANGULAR.
 *   D(NMAX) :  CONTAINS THE CONSTANT VECTOR OF THE OBJECTIVE FUNCTION.
 *   A(MMAX,NMAX): CONTAINS THE DATA MATRIX OF THE LINEAR CONSTRAINTS.
 *   B(MMAX) :  CONTAINS THE CONSTANT DATA OF THE LINEAR CONSTRAINTS.
 *   XL(N),XU(N): CONTAIN THE LOWER AND UPPER BOUNDS FOR THE VARIABLES.
 *   X(N) :     ON RETURN, X CONTAINS THE OPTIMAL SOLUTION VECTOR.
 *   U(MNN) :   ON RETURN, U CONTAINS THE LAGRANGE MULTIPLIERS. THE FIRST
 *              M POSITIONS ARE RESERVED FOR THE MULTIPLIERS OF THE M
 *              LINEAR CONSTRAINTS AND THE SUBSEQUENT ONES FOR THE 
 *              MULTIPLIERS OF THE LOWER AND UPPER BOUNDS. ON SUCCESSFUL
 *              TERMINATION, ALL VALUES OF U WITH RESPECT TO INEQUALITIES 
 *              AND BOUNDS SHOULD BE GREATER OR EQUAL TO ZERO.
 *   IOUT :     INTEGER INDICATING THE DESIRED OUTPUT UNIT NUMBER, I.E.
 *              ALL WRITE-STATEMENTS START WITH 'WRITE(IOUT,... '.
 *   IFAIL :    SHOWS THE TERMINATION REASON.
 *      IFAIL = 0 :   SUCCESSFUL RETURN.
 *      IFAIL = 1 :   TOO MANY ITERATIONS (MORE THAN 40*(N+M)).
 *      IFAIL = 2 :   ACCURACY INSUFFICIENT TO SATISFY CONVERGENCE
 *                    CRITERION.
 *      IFAIL = 5 :   LENGTH OF A WORKING ARRAY IS TOO SHORT.
 *      IFAIL > 10 :  THE CONSTRAINTS ARE INCONSISTENT.
 *   IPRINT :   OUTPUT CONTROL.
 *      IPRINT = 0 :  NO OUTPUT OF QL0001.
 *      IPRINT > 0 :  BRIEF OUTPUT IN ERROR CASES.
 *   WAR(LWAR) : REAL WORKING ARRAY. THE LENGTH LWAR SHOULD BE GRATER THAN
 *               3*NMAX*NMAX/2 + 10*NMAX + 2*MMAX.
 *   IWAR(LIWAR): INTEGER WORKING ARRAY. THE LENGTH LIWAR SHOULD BE AT
 *              LEAST N.
 *              IF IWAR(1)=0 INITIALLY, THEN THE CHOLESKY DECOMPOSITION
 *              WHICH IS REQUIRED BY THE DUAL ALGORITHM TO GET THE FIRST
 *              UNCONSTRAINED MINIMUM OF THE OBJECTIVE FUNCTION, IS
 *              PERFORMED INTERNALLY. OTHERWISE, I.E. IF IWAR(1)=1, THEN
 *              IT IS ASSUMED THAT THE USER PROVIDES THE INITIAL FAC-
 *              TORIZATION BY HIMSELF AND STORES IT IN THE UPPER TRIAN-
 *              GULAR PART OF THE ARRAY C.
 * 
 *   A NAMED COMMON-BLOCK  /CMACHE/EPS   MUST BE PROVIDED BY THE USER,
 *   WHERE EPS DEFINES A GUESS FOR THE UNDERLYING MACHINE PRECISION.
 * 
 * 
 *   AUTHOR:    K. SCHITTKOWSKI,
 *              MATHEMATISCHES INSTITUT,
 *              UNIVERSITAET BAYREUTH,
 *              8580 BAYREUTH,
 *              GERMANY, F.R.
 * 
 * 
 *   VERSION:   1.4  (MARCH, 1987)  
 * 
 *********************************************************************/

extern "C"
int ql0001_(int *m,      int *me,     int *mmax,
            int *n,      int *nmax,   int *mnn,
            double *c,   double *d,   double *a,
            double *b,   double *xl,  double *xu,
            double *x,   double *u,
            int *iout,   int *ifail,  int *iprint,
            double *war, int *lwar,
            int *iwar,   int *liwar,
            double *eps1);

namespace PLearn {
using namespace std;

//! Low-Level PLearn Interface for QLD  
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
  Vec& X,                      //!< optimal solution on return (resized)
  Vec& U,                      //!< lagrange multipliers on return (resized)
  Vec WAR = Vec(),             //!< real working array; resized automatically
  TVec<int> IWAR = TVec<int>() //!< int working array; resized automatically
  );                     

} // end of namespace PLearn

#endif
