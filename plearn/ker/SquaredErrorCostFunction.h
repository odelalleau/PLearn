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
   * $Id: SquaredErrorCostFunction.h,v 1.1 2003/12/15 22:08:32 dorionc Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef SquaredErrorCostFunction_INC
#define SquaredErrorCostFunction_INC

#include "Kernel.h"
#include "SelectedOutputCostFunction.h"

namespace PLearn <%
using namespace std;

//!  *********************************************************
//!  The following 'kernels' are rather used as cost functions


class SquaredErrorCostFunction: public Kernel
{
protected:
  //!  index in target vector of the target value to use to compute the squared error
  //!  (if -1, sum all the squared errors)
  int targetindex;
  bool classification;
  real hotvalue, coldvalue;

public:
  SquaredErrorCostFunction(int the_targetindex=-1)
    :targetindex(the_targetindex), classification(false), hotvalue(1), coldvalue(0) {};

  //!  Constructor for classification (target is interpreted as onehot)
  SquaredErrorCostFunction(real hot_value, real cold_value)
    : targetindex(-1), classification(true), hotvalue(hot_value), coldvalue(cold_value) {};

  PLEARN_DECLARE_OBJECT(SquaredErrorCostFunction);
  virtual string info() const;
  virtual real evaluate(const Vec& output, const Vec& target) const; 
    //virtual void readOptionVal(istream& in, const string& optionname);
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  //!  recognized option is "targetindex"
  
  static void declareOptions(OptionList &ol);
};
DECLARE_OBJECT_PTR(SquaredErrorCostFunction);

inline CostFunc squared_classification_error(real hot_value=0.8, real cold_value=0.2)
{
  return new SquaredErrorCostFunction(hot_value, cold_value);
}

inline CostFunc squared_error(int singleoutputindex=-1)
{ 
  if(singleoutputindex>=0)
    return new SelectedOutputCostFunction(new SquaredErrorCostFunction(),singleoutputindex); 
  else
    return new SquaredErrorCostFunction();
}

%> // end of namespace PLearn

#endif

