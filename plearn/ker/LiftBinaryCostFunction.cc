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
   * $Id: LiftBinaryCostFunction.cc,v 1.3 2004/04/02 19:56:54 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "LiftBinaryCostFunction.h"

// From Old Kernel.cc: all includes are putted in every file.
// To be revised manually 
/*#include <cmath>
#include "stringutils.h"
#include "Kernel.h"
#include "TMat_maths.h"
#include "PLMPI.h"*/
//////////////////////////
namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(LiftBinaryCostFunction, "ONE LINE DESCR", "NO HELP");
string LiftBinaryCostFunction::info() const { return "lift_binary_function"; }

real LiftBinaryCostFunction::evaluate(const Vec& output, const Vec& target) const
{
  int len = output.length();
  if ((target.length()!=1 && target.length()!=2) || len>2)
    PLERROR("For binary problems, the output and binary vectors must have length 1 or 2 (%d)", len);

  if (len == 1)
  {
    real out0 = output[0];
    if (make_positive_output) out0 = sigmoid(out0);
    return (target[0]>0.5) ? out0 : -out0;
  }
  else
  {
    real out1 = output[1];
    if (make_positive_output) out1 = sigmoid(out1);
    if (target.length()==2)
      return (target[1]>target[0]) ? out1 : -out1;
    else
      return (target[0]>0.5) ? out1 : -out1;
  }

  return 0.0;  // to make the compiler happy...
}


void LiftBinaryCostFunction::write(ostream& out) const
{
  writeHeader(out,"LiftBinaryCostFunction");
  writeField(out,"make_positive_output",make_positive_output);
  writeFooter(out,"LiftBinaryCostFunction");
}

void LiftBinaryCostFunction::oldread(istream& in)
{
  readHeader(in,"LiftBinaryCostFunction");
  readField(in,"make_positive_output",make_positive_output);
  readFooter(in,"LiftBinaryCostFunction");
}
// recognized option is "norm_to_use"

/*
void LiftBinaryCostFunction::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="make_positive_output")
    PLearn::read(in,make_positive_output);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void LiftBinaryCostFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "make_positive_output", &LiftBinaryCostFunction::make_positive_output, OptionBase::buildoption,
                   "TODO: Some comments");
    inherited::declareOptions(ol);
}



} // end of namespace PLearn

