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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "ExtendedVariable.h"
#include "SubMatVariable.h"

namespace PLearn {
using namespace std;



/** ExtendedVariable **/

PLEARN_IMPLEMENT_OBJECT(ExtendedVariable,
                        "Variable that extends the input variable by appending rows at "
                        "its top and bottom and columns at its left and right.",
                        "NO HELP");

ExtendedVariable::ExtendedVariable(Variable* input, int the_top_extent, int the_bottom_extent,
                                   int the_left_extent, int the_right_extent, real the_fill_value)
    : inherited(input, input->length()+the_top_extent+the_bottom_extent, input->width()+the_left_extent+the_right_extent),
      top_extent(the_top_extent), bottom_extent(the_bottom_extent), 
      left_extent(the_left_extent), right_extent(the_right_extent),
      fill_value(the_fill_value)
{
    build_();
}

void
ExtendedVariable::build()
{
    inherited::build();
    build_();
}

void
ExtendedVariable::build_()
{
    if(top_extent<0 || bottom_extent<0 || left_extent<0 || right_extent<0)
        PLERROR("In ExtendedVariable: given extents must be >=0");
    for(int k=0; k<nelems(); k++)
        valuedata[k] = fill_value;
}

void
ExtendedVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "top_extent", &ExtendedVariable::top_extent, OptionBase::buildoption, "");
    declareOption(ol, "bottom_extent", &ExtendedVariable::bottom_extent, OptionBase::buildoption, "");
    declareOption(ol, "left_extent", &ExtendedVariable::left_extent, OptionBase::buildoption, "");
    declareOption(ol, "right_extent", &ExtendedVariable::right_extent, OptionBase::buildoption, "");
    declareOption(ol, "fill_value", &ExtendedVariable::fill_value, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void ExtendedVariable::recomputeSize(int& l, int& w) const
{
    if (input) {
        l = input->length() + top_extent + bottom_extent;
        w = input->width() + left_extent + right_extent;
    } else
        l = w = 0;
}

void ExtendedVariable::fprop()
{
    if(width()==input->width()) // optimized code for this special case (no left or right extents)
    {
        real* data = valuedata+top_extent*width();
        for(int k=0; k<input->nelems(); k++)
            data[k] = input->valuedata[k];
    }
    else // general case
    {
        real* rowdata=valuedata+top_extent*width()+left_extent;
        int inputk=0;
        for(int i=0; i<input->length(); i++)
        {
            for(int j=0; j<input->width(); j++)
                rowdata[j] = input->valuedata[inputk++];
            rowdata += width();
        }
    }
}


void ExtendedVariable::bprop()
{
    if(width()==input->width()) // optimized code for this special case (no left or right extents)
    {
        real* data = gradientdata+top_extent*width();
        for(int k=0; k<input->nelems(); k++)
            input->gradientdata[k] += data[k];
    }
    else // general case
    {
        real* rowdata = gradientdata+top_extent*width()+left_extent;
        int inputk = 0;
        for(int i=0; i<input->length(); i++)
        {
            for(int j=0; j<input->width(); j++)
                input->gradientdata[inputk++] += rowdata[j]; 
            rowdata += width();
        }
    }
}


void ExtendedVariable::symbolicBprop()
{
    input->accg(new SubMatVariable(g,top_extent,left_extent,input->length(),input->width()));
}


void ExtendedVariable::rfprop()
{
    if (rValue.length()==0) resizeRValue();
    if(width()==input->width()) // optimized code for this special case (no left or right extents)
    {
        real* data = rvaluedata+top_extent*width();
        for(int k=0; k<input->nelems(); k++)
            data[k] = input->rvaluedata[k];
    }
    else // general case
    {
        real* rowdata=rvaluedata+top_extent*width()+left_extent;
        int inputk=0;
        for(int i=0; i<input->length(); i++)
        {
            for(int j=0; j<input->width(); j++)
                rowdata[j] = input->rvaluedata[inputk++];
            rowdata += width();
        }
    }
}

} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
