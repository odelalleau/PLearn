// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
   * $Id: UnfoldedFuncVariable.h,v 1.5 2004/02/23 23:59:35 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef UnfoldedFuncVariable_INC
#define UnfoldedFuncVariable_INC

#include "NaryVariable.h"

namespace PLearn {
using namespace std;


class UnfoldedFuncVariable: public NaryVariable
{
protected:
    //!  protected default constructor for persistence
  UnfoldedFuncVariable();

public:
  //protected:
  Var input_matrix;
  Func f;
  bool transpose;

  TVec<VarArray> inputs; // all the input Var's
  TVec<Var> outputs; // and the corresponding output Var's, 
  TVec<VarArray> f_paths; // the duplicates of f prop. path for each input/output pair: inputs[i]->outputs[i]

public:
  //! concatenate_{i=0 to n_unfold} f(i-th row of input_matrix)
  UnfoldedFuncVariable(Var inputmatrix, Func the_f, bool transpose);
    
  PLEARN_DECLARE_OBJECT(UnfoldedFuncVariable);
  virtual void build();
  virtual void recomputeSize(int& l, int& w) const;
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual void fprop();
  virtual void bprop();
    
  void printInfo(bool print_gradient);

  static void declareOptions(OptionList& ol);

private:
  void build_();
};

inline Var unfoldedFuncOf(Var input_matrix, Func f, bool transpose = false)
{ return new UnfoldedFuncVariable(input_matrix, f, transpose); }

} // end of namespace PLearn

#endif 
