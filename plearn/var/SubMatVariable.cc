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
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** SubMatVariable **/

PLEARN_IMPLEMENT_OBJECT(
    SubMatVariable,
    "Takes a submatrix of an input variable",
    "This Variable performs creates a view of a subset of an input variable.\n"
    "The starting row and column in the input variable must be specified, as\n"
    "well as the new number of rows and columns.\n"
    "\n"
    "Variables of this kind can also be created from C++ through the subMat\n"
    "function.\n");

SubMatVariable::SubMatVariable(Variable* v, int i, int j, int the_length, int the_width)
    : inherited(v, the_length, the_width),
      startk(i*v->width()+j),
      length_(the_length),
      width_(the_width),
      i_(i),
      j_(j)
{
    build_();
}

void SubMatVariable::build()
{
    inherited::build();
    build_();
}

void SubMatVariable::build_()
{
    if (input) {
        // input is v from constructor
        if(i_ < 0 || i_ + length() > input->length() || j_ < 0 || j_ + width() > input->width())
            PLERROR("In SubMatVariable: requested sub-matrix is out of matrix bounds");
        startk = i_ * input->width() + j_;
    }
}

void
SubMatVariable::declareOptions(OptionList &ol)
{
    declareOption(
        ol, "length_", &SubMatVariable::length_, OptionBase::buildoption,
        "New length (number of rows) of the SubVMatrix variable");

    declareOption(
        ol, "width_", &SubMatVariable::width_, OptionBase::buildoption,
        "New width (number of columns) of the SubVMatrix variable");

    declareOption(
        ol, "i_", &SubMatVariable::i_, OptionBase::buildoption,
        "Starting ROW in the input variable");
    
    declareOption(
        ol, "j_", &SubMatVariable::j_, OptionBase::buildoption,
        "Starting COLUMN in the input variable");
    
    declareOption(
        ol, "startk", &SubMatVariable::startk, OptionBase::learntoption,
        "Should not be set directly: this is equal to the starting element\n"
        "in the INPUT MATRIX corresponding to the settings of i_ and j_.");

    inherited::declareOptions(ol);
}

void SubMatVariable::recomputeSize(int& l, int& w) const
{ l=length_; w=width_; }

void SubMatVariable::fprop()
{
    if(width()==input->width()) // optimized version for this special case
    {
        real* inputdata = input->valuedata+startk;
        for(int k=0; k<nelems(); k++)
            valuedata[k] = inputdata[k];
    }
    else // general version
    {
        real* inputrowdata = input->valuedata+startk;
        int thisk=0;
        for(int i=0; i<length(); i++)
        {
            for(int j=0; j<width(); j++)
                valuedata[thisk++] = inputrowdata[j];
            inputrowdata += input->width();
        }
    }
}


void SubMatVariable::bprop()
{
    if(width()==input->width()) // optimized version for this special case
    {
        real* inputgradient = input->gradientdata+startk;
        for(int k=0; k<nelems(); k++)
            inputgradient[k] += gradientdata[k]; 
    }
    else // general version
    {
        real* inputrowgradient = input->gradientdata+startk;
        int thisk=0;
        for(int i=0; i<length(); i++)
        {
            for(int j=0; j<width(); j++)
                inputrowgradient[j] += gradientdata[thisk++];
            inputrowgradient += input->width();
        }
    }
}


void SubMatVariable::bbprop()
{
    if (input->diaghessian.length()==0)
        input->resizeDiagHessian();
    if(width()==input->width()) // optimized version for this special case
    {
        real* inputdiaghessian = input->diaghessian.data()+startk;
        for(int k=0; k<nelems(); k++)
            inputdiaghessian[k] += diaghessiandata[k]; 
    }
    else // general version
    {
        real* inputrowdiaghessian = input->diaghessiandata+startk;
        int thisk=0;
        for(int i=0; i<length(); i++)
        {
            for(int j=0; j<width(); j++)
                inputrowdiaghessian[j] += diaghessiandata[thisk++];
            inputrowdiaghessian += input->width();
        }
    }
}


void SubMatVariable::symbolicBprop()
{
    int i = startk/input->width();
    int j = startk%input->width();
    int topextent = i;
    int bottomextent = input->length()-(i+length());
    int leftextent = j;
    int rightextent = input->width()-(j+width());
    input->accg(extend(g,topextent,bottomextent,leftextent,rightextent));
}


void SubMatVariable::rfprop()
{
    if (rValue.length()==0) resizeRValue();
    if(width()==input->width()) // optimized version for this special case
    {
        real* inputrdata = input->rvaluedata+startk;
        for(int k=0; k<nelems(); k++)
            rvaluedata[k] = inputrdata[k];
    }
    else // general version
    {
        real* inputrowrdata = input->rvaluedata+startk;
        int thisk=0;
        for(int i=0; i<length(); i++)
        {
            for(int j=0; j<width(); j++)
                rvaluedata[thisk++] = inputrowrdata[j];
            inputrowrdata += input->width();
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
