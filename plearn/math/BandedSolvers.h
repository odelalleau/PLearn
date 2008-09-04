// -*- C++ -*-

// BandedSolvers.h
//
// Copyright (C) 2004 Nicolas Chapados 
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

/*! \file BandedSolvers.h */


#ifndef BandedSolvers_INC
#define BandedSolvers_INC

// Put includes here
#include "TVec.h"

namespace PLearn {
using namespace std;

/**
 * Solver for a summetric Pentadiagonal system.  Code ported from Matlab by
 * NC.  The RHS is given by Y.  The solution is computed IN-PLACE and
 * overwrites the original Y, A, B, and C vectors.  The B and C vectors
 * must have the same length as A and Y, but the last element (for B) and
 * last two elements (for C) don't matter.
 *
 * Original Matlab code documented as follows:
 *
 * Author: Kurt Annen annen@web-reg.de
 * Date: 15/05/2004
 * Internet: www.web-reg.de
 * 
 * Solves the problem Ax=b when A is pentadiagonal and strongly nonsingular. 
 * This is much faster than \f$ x=A\y \f$ for large matrices.  
 * 
 * Reference: Späth, Helmuth "Numerik: Eine Einführung für Mathematiker und
 * Informatiker" S. 110 . Vieweg-Verlag Braunschweig/Wiesbaden (1994)
 * 
 * a = main diagonal
 * b = 2. diagonal
 * c = 3. diagonal
 */
template <class T>
void PentadiagonalSolveInPlace(const TVec<T>& y, const TVec<T>& a,
                               const TVec<T>& b, const TVec<T>& c)
{
    int n = a.size();
    int m = y.size();
    int o = b.size();
    int p = c.size();

    // Limitation of this routine for now
    if (m < 2)
        PLERROR("PentadiagonalSolve: vectors must have length at least two");
  
    if (n != m || n != o || n != p)
        PLERROR("PentadiagonalSolve: vector dimensions don't agree; they must "
                "all have the same length.");

    c[m-1] = 0;
    c[m-2] = 0;
    b[m-1] = 0;

    T h1=0;
    T h2=0;
    T h3=0;
    T h4=0;
    T h5=0;
    T hh1=0;
    T hh2=0;
    T hh3=0;
    T hh5=0;
    T z=0;
    T hb=0;
    T hc=0;

    for (int i=0 ; i<m ; ++i) {
        z=a[i]-h4*h1-hh5*hh2;
        hb=b[i];
        hh1=h1;
        h1=(hb-h4*h2)/z;
        b[i]=h1;
        hc=c[i];
        hh2=h2;
        h2=hc/z;
        c[i]=h2;
        a[i]=(y[i]-hh3*hh5-h3*h4)/z;
        hh3=h3;
        h3=a[i];
        h4=hb-h5*hh1;
        hh5=h5;
        h5=hc;
    }

    h2=0;
    h1=a[m-1];
    y[m-1]=h1;

    for (int i=m-1 ; i>=0 ; --i) {
        y[i]=a[i]-b[i]*h1-c[i]*h2;
        h2=h1;
        h1=y[i];
    }
}

} // end of namespace PLearn

#endif


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
