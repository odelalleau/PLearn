// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Christopher Kermorvant
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

/*! \file SmoothedProbSparseMatrix.h */

#ifndef SmoothedProbSparseMatrix_INC
#define SmoothedProbSparseMatrix_INC

#include <plearn/math/ProbSparseMatrix.h>
#include <plearn/math/TMat.h>
#define PROB_PREC 0.0001
namespace PLearn {
using namespace std;

class SmoothedProbSparseMatrix : public ProbSparseMatrix
{
protected:
    // Smoothing Method :
    // 0 : no smoothing!
    // 1 : laplace
    // 2 : discount - backoff
    // 3 :  discount - backoff non shadowing
    int smoothingMethod;  

    // Precomputed nomalization sum;
    Vec normalizationSum;
    // Backoff distribution
    Vec backoffDist;
    Vec backoffNormalization;
    // Store discounted mass in case of Backoff smoothing
    Vec discountedMass;
public:

    bool checkCondProbIntegrity();
    SmoothedProbSparseMatrix(int n_rows = 0, int n_cols = 0, string name = "pXY", int mode = ROW_WISE, bool double_access = false);
    void normalizeCondLaplace(ProbSparseMatrix& nXY, bool clear_nXY = false);
    void normalizeCondBackoff(ProbSparseMatrix& nXY, real disc, Vec& bDist,bool clear_nXY,bool shadow);
    string getClassName() const { return "SmoothedProbSparseMatrix"; }
    real get(int i,int j);
    void write(PStream& out) const;
    void read(PStream& in);

};

// WARNING : do not use this object, it is in developement. CK-11/2003
class ComplementedProbSparseMatrix : public ProbSparseMatrix
{
protected:

    real grandSum;
    Vec rowSum;
    Vec columnSum;

public:
    void complement(ProbSparseMatrix& nXY, bool clear_nXY = false);
    real get(int i,int j);
    bool checkCondProbIntegrity();
};



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
