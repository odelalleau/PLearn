// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1999-2002 Christian Jauvin
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

/*! \file ProbSparseMatrix.h */

#ifndef ProbSparseMatrix_INC
#define ProbSparseMatrix_INC

#include "DoubleAccessSparseMatrix.h"

namespace PLearn <%
using namespace std;

class ProbSparseMatrix : public DoubleAccessSparseMatrix<real>
{

public:

  ProbSparseMatrix(int n_rows = 0, int n_cols = 0, string name = "pXY", int mode = ROW_WISE, bool double_access = false);

  void incr(int i, int j, real inc = 1.0, bool warning = true);

  void set(int i, int j, real value, bool warning = true);

  bool checkCondProbIntegrity();

  bool checkJointProbIntegrity();

  void normalizeCond(ProbSparseMatrix& nXY, bool clear_nXY = false);

  void normalizeJoint(ProbSparseMatrix& nXY, bool clear_nXY = false);

  void normalizeCond();

  void normalizeJoint();

  string getClassName() const { return "ProbSparseMatrix"; }

};

template <class T> 
inline PStream& operator<<(PStream &out, const ProbSparseMatrix &p)
{ 
  p.write(out); 
  return out;
}

template <class T> 
inline PStream& operator>>(PStream &in, ProbSparseMatrix &p)
{ 
  p.read(in); 
  return in;
}

class PSMat : public PP<ProbSparseMatrix>
{

public:

  PSMat(int n_rows = 0, int n_cols = 0, string name = "pXY", int mode = ROW_WISE, bool double_access = false) : PP<ProbSparseMatrix>(new ProbSparseMatrix(n_rows, n_cols, name, mode, double_access)) {}

  PSMat(ProbSparseMatrix* p) : PP<ProbSparseMatrix>(p) {}

};

%> // end of namespace PLearn

#endif
