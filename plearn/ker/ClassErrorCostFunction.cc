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
   * $Id: ClassErrorCostFunction.cc,v 1.2 2004/02/20 21:11:45 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ClassErrorCostFunction.h"

// From Old Kernel.cc: all includes are putted in every file.
// To be revised manually 
#include <cmath>
#include "stringutils.h"
#include "Kernel.h"
#include "TMat_maths.h"
#include "PLMPI.h"
//////////////////////////
namespace PLearn {
using namespace std;


// **** ClassErrorCostFunction ****

PLEARN_IMPLEMENT_OBJECT(ClassErrorCostFunction, "ONE LINE DESCR", "NO HELP");
string ClassErrorCostFunction::info() const { return "class_error"; }

real ClassErrorCostFunction::evaluate(const Vec& output, const Vec& target) const
{
  if(output_is_classnum)
  {
    if(is_integer(output[0]))
      return output[0]==target[0] ?0 :1;
    else if(target[0]==1.)
      return output[0]>0.5 ?0 :1;
    else // target[0]==0 or -1
      return output[0]<=0.5 ?0 :1;
  }

  if(output.length()==1) // we assume the sign of output indicates the chosen class 
  {
    if(target[0]>0)
      return output[0]>0 ?0. :1.;
    else
      return output[0]<0 ?0. :1.;
  }
  else // we assume output gives a score for each class
  {
    int trueclass;
    if(target.length()==1)
      trueclass = int(target[0]);
    else
      trueclass = argmax(target);
    return argmax(output)==trueclass ?0. :1.;
  }
  return 1.; // to make the compiler happy
}


void ClassErrorCostFunction::write(ostream& out) const
{
  writeHeader(out,"ClassErrorCostFunction");
  writeField(out,"output_is_classnum",output_is_classnum);
  writeFooter(out,"ClassErrorCostFunction");
}

void ClassErrorCostFunction::oldread(istream& in)
{
  readHeader(in,"ClassErrorCostFunction");
  readField(in,"output_is_classnum",output_is_classnum);
  readFooter(in,"ClassErrorCostFunction");
}
// recognized option is "norm_to_use"

/*
void ClassErrorCostFunction::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="output_is_classnum")
    PLearn::read(in,output_is_classnum);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void
ClassErrorCostFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "output_is_classnum", &ClassErrorCostFunction::output_is_classnum, OptionBase::buildoption, "Output of learner is class number");
    inherited::declareOptions(ol);
}



} // end of namespace PLearn

