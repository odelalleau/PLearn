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
   * $Id: ClassDistanceProportionCostFunction.h,v 1.2 2004/02/20 21:11:45 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef ClassDistanceProportionCostFunction_INC
#define ClassDistanceProportionCostFunction_INC

#include "Kernel.h"

namespace PLearn {
using namespace std;



/*!   This is very similar to ClassMarginCostFunction when output.length()>1,
  except than rather than return the negated difference between the score
  of the right class and the highest score among all other classes, we
  return:
  WARNING: This doesn't make much sense if scores have different signs (they should either be all positive or all negative)
*/

/*!   This is useful for multiclass problems when the scores represent
  negative distances to each class, It returns
  distance_to_right_class/(distance_to_right_class+distance_to_closest_other_classes)
*/
class ClassDistanceProportionCostFunction: public Kernel
{
public:
  ClassDistanceProportionCostFunction() {}
  PLEARN_DECLARE_OBJECT(ClassDistanceProportionCostFunction);
  virtual real evaluate(const Vec& output, const Vec& target) const;
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
};
DECLARE_OBJECT_PTR(ClassDistanceProportionCostFunction);

//!  if outputs are neg distances to each class: dist_to_correct_class/(dist_to_correct_class+dist_to_closest_other_class)
inline CostFunc class_distance_proportion() 
{ 
  return new ClassDistanceProportionCostFunction(); 
}


} // end of namespace PLearn

#endif

