// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
//
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
   * $Id: ExtendedVMatrix.cc,v 1.5 2004/06/29 19:51:32 tihocan Exp $
   ******************************************************* */

#include "ExtendedVMatrix.h"

namespace PLearn {
using namespace std;


/** ExtendedVMatrix **/

PLEARN_IMPLEMENT_OBJECT(ExtendedVMatrix, "ONE_LINE_DESC", "ONE_LINE_HELP");

ExtendedVMatrix::ExtendedVMatrix()
  : top_extent(0), bottom_extent(0), left_extent(0), right_extent(0), fill_value(0)
{
}

ExtendedVMatrix::ExtendedVMatrix(VMat the_distr, int the_top_extent, int the_bottom_extent, 
                                 int the_left_extent, int the_right_extent,
                                 real the_fill_value)
  : inherited(the_distr->length()+the_top_extent+the_bottom_extent,
              the_distr->width()+the_left_extent+the_right_extent),
    distr(the_distr), top_extent(the_top_extent), bottom_extent(the_bottom_extent), 
    left_extent(the_left_extent), right_extent(the_right_extent), fill_value(the_fill_value)
{
}

void
ExtendedVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &ExtendedVMatrix::distr, OptionBase::buildoption, "");
    declareOption(ol, "top_extent", &ExtendedVMatrix::top_extent, OptionBase::buildoption, "");
    declareOption(ol, "bottom_extent", &ExtendedVMatrix::bottom_extent, OptionBase::buildoption, "");
    declareOption(ol, "left_extent", &ExtendedVMatrix::left_extent, OptionBase::buildoption, "");
    declareOption(ol, "right_extent", &ExtendedVMatrix::right_extent, OptionBase::buildoption, "");
    declareOption(ol, "fill_value", &ExtendedVMatrix::fill_value, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void
ExtendedVMatrix::build()
{
    inherited::build();
    build_();
}

void
ExtendedVMatrix::build_()
{
}

void ExtendedVMatrix::getNewRow(int i, Vec& v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In ExtendedVMatrix::getNewRow OUT OF BOUNDS");
  if(v.length() != width())
    PLERROR("In ExtendedVMatrix::getNewRow v.length() must be equal to the VMat's width");
#endif

  if(i<top_extent || i>=length()-bottom_extent)
    v.fill(fill_value);
  else
    {
      Vec subv = v.subVec(left_extent,distr->width());
      distr->getRow(i-top_extent,subv);
      if(left_extent>0)
        v.subVec(0,left_extent).fill(fill_value);
      if(right_extent>0)
        v.subVec(width()-right_extent,right_extent).fill(fill_value);
    }
}

} // end of namespcae PLearn
