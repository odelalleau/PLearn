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
   * $Id: SelectedOutputCostFunction.cc,v 1.3 2004/04/02 19:56:54 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SelectedOutputCostFunction.h"

/*#include <cmath>
#include "stringutils.h"
#include "Kernel.h"
#include "TMat_maths.h"
#include "PLMPI.h"*/
//////////////////////////
namespace PLearn {
using namespace std;



PLEARN_IMPLEMENT_OBJECT(SelectedOutputCostFunction, "ONE LINE DESCR", "NO HELP");
void SelectedOutputCostFunction::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Kernel::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(costfunc, copies);
}

string SelectedOutputCostFunction::info() const { return "selected_output[" + tostring(outputindex) + "] "+costfunc->info(); }

real SelectedOutputCostFunction::evaluate(const Vec& output, const Vec& target) const
{ return costfunc(output.subVec(outputindex,1),target.subVec(outputindex,1)); }


void SelectedOutputCostFunction::write(ostream& out) const
{
  writeHeader(out,"SelectedOutputCostFunction");
  writeField(out,"outputindex",outputindex);
  writeField(out,"costfunc",costfunc);
  writeFooter(out,"SelectedOutputCostFunction");
}

void SelectedOutputCostFunction::oldread(istream& in)
{
  readHeader(in,"SelectedOutputCostFunction");
  readField(in,"outputindex",outputindex);
  readField(in,"costfunc",costfunc);
  readFooter(in,"SelectedOutputCostFunction");
}
// recognized option is "norm_to_use"

/*
void SelectedOutputCostFunction::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="outputindex")
    PLearn::read(in,outputindex);
  else
    inherited::readOptionVal(in, optionname);  
}
*/

void SelectedOutputCostFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "outputindex", &SelectedOutputCostFunction::outputindex, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}



} // end of namespace PLearn

