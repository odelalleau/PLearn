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
   * $Id: MulticlassErrorCostFunction.cc,v 1.3 2004/04/02 19:56:54 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "MulticlassErrorCostFunction.h"

/*#include <cmath>
#include "stringutils.h"
#include "Kernel.h"
#include "TMat_maths.h"
#include "PLMPI.h"*/
//////////////////////////
namespace PLearn {
using namespace std;


// **** MulticlassErrorCostFunction ****

PLEARN_IMPLEMENT_OBJECT(MulticlassErrorCostFunction, "ONE LINE DESCR", "NO HELP");
string MulticlassErrorCostFunction::info() const { return "multiclass_error"; }

real MulticlassErrorCostFunction::evaluate(const Vec& output, const Vec& target) const
{
  if (output.length() != target.length())
    PLERROR("In MulticlassErrorCostFunction::evaluate: Output vec and target vec must have the same length (%d!=%d", output.length(),target.length());

  real cost = 0.0;
  for (int i=0; i<output.length(); i++)
  {
    real output_i = output[i];
    int target_i = (int)target[i];
    cost += (target_i==1) ? output_i<0.5 : output_i>0.5;
  }
  return cost;
}


void MulticlassErrorCostFunction::write(ostream& out) const
{
  writeHeader(out,"MulticlassErrorCostFunction");
  writeFooter(out,"MulticlassErrorCostFunction");
}

void MulticlassErrorCostFunction::oldread(istream& in)
{
  readHeader(in,"MulticlassErrorCostFunction");
  readFooter(in,"MulticlassErrorCostFunction");
}



} // end of namespace PLearn

