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
   * $Id: CCCostVariable.h,v 1.1 2004/11/12 20:04:34 larocheh Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef CCCostVariable_INC
#define CCCostVariable_INC

#include "NaryVariable.h"

namespace PLearn {
using namespace std;


class CCCostVariable: public NaryVariable
{
  typedef NaryVariable inherited;
  
public:
  
  VMat distr;
  Func f_error;
  Func f_candidate;
  // To avoid allocation/deallocations in fprop/bprop
  Vec input_value;
  Vec input_gradient;
  Vec error_output_value;
  Vec candidate_output_value;
  Vec error_correlations;
  Vec adjusted_gradient;
private:
  Vec mean_error;
  real mean_candidate;
    
public:
  //!  protected default constructor for persistence
  CCCostVariable() : distr(), f_error(), f_candidate() {}
  //!  constructor
  CCCostVariable(VMat the_distr, Func the_f, Func f_candidate);
    
  PLEARN_DECLARE_OBJECT(CCCostVariable);
  static void declareOptions(OptionList &ol);

  virtual void build();

  virtual void recomputeSize(int& l, int& w) const;
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
  virtual void fprop();
  virtual void bprop();
  virtual void fbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
    
  void printInfo(bool print_gradient);

protected:
  void build_();
};

DECLARE_OBJECT_PTR(CCCostVariable);

//!  sumOf
inline Var cccost(VMat distr, Func the_f_error, Func the_f_candidate)
{ return new CCCostVariable(distr,the_f_error,the_f_candidate); }

} // end of namespace PLearn

#endif 
