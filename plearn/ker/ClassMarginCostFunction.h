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
   * $Id: ClassMarginCostFunction.h,v 1.4 2004/04/07 23:15:58 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef ClassMarginCostFunction_INC
#define ClassMarginCostFunction_INC

#include "Kernel.h"

namespace PLearn {
using namespace std;



/*!   * If output and target both have length 1, when binary classification 
    with targets -1 and +1 and the sign of the output is considered,
    then margin is output[0]*target[0]
    If binary targets is {0,1} and outputs>0, then the margin is:
    (output[0]+1)*(target[0]+1)/4
    However, if the flag output_is_positive is true then output is
    replaced by output[0]-0.5 in the above expressions.
  * If output has length>1 and target has length 1, then output is understood 
    as giving a score for each class while target is the index of the correct
    class (numbered from 0). Then margin is the difference between the score 
    of the correct class and the highest score among the other classes.
  * If both output and target have a length>1 then output is understood 
    as giving a score for each class while the correct class is given by 
    argmax(target). Then margin is the difference between the score 
    of the correct class and the highest score among the other classes.
  In all cases, as this is a cost function, we return -margin.
*/
class ClassMarginCostFunction: public Kernel
{
  typedef Kernel inherited;

 public:
  bool binary_target_is_01;
  bool output_is_positive;
  ClassMarginCostFunction(bool the_binary_target_is_01=false, 
                          bool out_is_positive=false)
    : binary_target_is_01(the_binary_target_is_01), 
    output_is_positive(out_is_positive) {}

  PLEARN_DECLARE_OBJECT(ClassMarginCostFunction);

  virtual string info() const
    { return "class_margin"; }
  virtual real evaluate(const Vec& output, const Vec& target) const;  

protected:
  //!  recognized option are "binary_target_is_01" and "output_is_positive"
  static void declareOptions(OptionList &ol);  
};

DECLARE_OBJECT_PTR(ClassMarginCostFunction);

//!  difference between correct class score and max of other class' scores
inline CostFunc class_margin(bool binary_target_is_01=false, 
                             bool output_is_positive=false) 
{ return new ClassMarginCostFunction(binary_target_is_01,output_is_positive); }

} // end of namespace PLearn

#endif

