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
   * $Id: UnfoldedSumOfVariable.h,v 1.7 2004/04/27 16:05:34 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef UnfoldedSumOfVariable_INC
#define UnfoldedSumOfVariable_INC

#include "NaryVariable.h"

namespace PLearn {
using namespace std;


class UnfoldedSumOfVariable: public NaryVariable
{
  typedef NaryVariable inherited;

public:
  //protected:
  Var input_matrix;
  Var bag_size;
  Func f;
  int max_bag_size;

  TVec<VarArray> inputs;  // all the input Var's
  VarArray outputs;       // and the corresponding output Var's, 
  TVec<VarArray> f_paths; // the duplicates of f prop. path for each input/output pair: inputs[i]->outputs[i]

public:
  //!  protected default constructor for persistence
  UnfoldedSumOfVariable() : max_bag_size(0) {}
  //! Sum_{i=0 to bag_size} f(i-th row of input_matrix)
  UnfoldedSumOfVariable(Var inputmatrix, Var bagsize, Func the_f, int maxbagsize);
    
  PLEARN_DECLARE_OBJECT(UnfoldedSumOfVariable);
  static void declareOptions(OptionList& ol);

  virtual void build();

  virtual void recomputeSize(int& l, int& w) const;
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual void fprop();
  virtual void bprop();
    
  void printInfo(bool print_gradient);

private:
  void build_();
};

DECLARE_OBJECT_PTR(UnfoldedSumOfVariable);

inline Var unfoldedSumOf(Var input_matrix, Var bag_size, Func f, int max_bag_size)
{ return new UnfoldedSumOfVariable(input_matrix,bag_size,f,max_bag_size); }

} // end of namespace PLearn

#endif 
