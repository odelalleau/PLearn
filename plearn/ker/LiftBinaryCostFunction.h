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
   * $Id: LiftBinaryCostFunction.h,v 1.1 2003/12/15 22:08:32 dorionc Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef LiftBinaryCostFunction_INC
#define LiftBinaryCostFunction_INC

#include "Kernel.h"

namespace PLearn <%
using namespace std;



/*!   This class allows us to compute the lift (used by Bell Canada) of
  a binary classification problem.  Since we need the output AND the target
  to be represented by a single cost (the method evaluate returns only a
  real), we will use a trick.
  cost =  output[class#1], if target = 1
       = -output[class#1], if target = 0
  or, if the make_positive_output flag is true:
  cost =  sigmoid(output[class#1]), if target = 1
       = -sigmoid(output[class#1]), if target = 0
*/
class LiftBinaryCostFunction: public Kernel
{
  PLEARN_DECLARE_OBJECT(LiftBinaryCostFunction);
 protected:
  bool make_positive_output;
 public:
  LiftBinaryCostFunction(bool make_pos_output=false) : make_positive_output(make_pos_output) {}
  virtual string info() const;
    //virtual void readOptionVal(istream& in, const string& optionname);
    static void declareOptions(OptionList &ol);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "make_positive_output"
           
  virtual real evaluate(const Vec& output, const Vec& target) const;

};
DECLARE_OBJECT_PTR(LiftBinaryCostFunction);

inline CostFunc class_lift(bool make_positive=false) 
{ return new LiftBinaryCostFunction(make_positive); }

%> // end of namespace PLearn

#endif

