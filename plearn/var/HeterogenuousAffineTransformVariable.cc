// -*- C++ -*-

// HeterogenuousAffineTransformVariable.cc
//
// Copyright (C) 2006 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file HeterogenuousAffineTransformVariable.cc */


#include "HeterogenuousAffineTransformVariable.h"

namespace PLearn {
using namespace std;

/** HeterogenuousAffineTransformVariable **/

PLEARN_IMPLEMENT_OBJECT(
    HeterogenuousAffineTransformVariable,
    "Affine transform with continuous and discrete input",
    "Affine transform that supports continuous and discrete input. The input\n"
    "(varray[0]) can contain a continuous value or the index of a symbol (e.g. a word).\n"
    "This is determined by the field input_is_discrete, which contains the information\n"
    "about which components are discrete.\n" 
    "All the weights (varray[1..end], with varray.last() being the biases) have the\n"
    "same width, which corresponds to the size of this variable. The input must be a\n"
    "row or column vector, and this variable is either going to be a row or column\n"
    "vector, depending on the input."
    );

HeterogenuousAffineTransformVariable::HeterogenuousAffineTransformVariable()
{}

// constructors from input variables.
 HeterogenuousAffineTransformVariable::HeterogenuousAffineTransformVariable(Var input, VarArray weights, TVec<bool> the_input_is_discrete)
     : inherited(input & weights, 1, 1), input_is_discrete(the_input_is_discrete)
//     : inherited(input & weights, input->length() != 1 ? weights[0]->width() : 1 , input->width() != 1 ? weights[0]->width() : 1)
 {
     build();
 }

void HeterogenuousAffineTransformVariable::recomputeSize(int& l, int& w) const
{
    if (varray.size() > 1) 
    {
        if(varray[0]->width() != 1)
        {
            l = 1;
            w = varray[1]->width();
        }
        else
        {
            l = varray[1]->width() ;
            w = 1;
        }

    } else
        l = w = 0;
}

void HeterogenuousAffineTransformVariable::fprop()
{
    matValue << varray.last()->matValue;
    int n = size();
    int l = varray.length()-1;
    for(int i=1; i<l; i++)
    {
        if(input_is_discrete[i-1])
        {
            real* row = varray[i]->matValue.row((int)varray[0]->valuedata[i-1]).data();
            for(int j=0; j<n; j++)
                valuedata[j] += *row++;
        }
        else
        {
            for(int j=0; j<n; j++)
                valuedata[j] += varray[0]->valuedata[i-1]*varray[i]->valuedata[j];
        }
    }
}

void HeterogenuousAffineTransformVariable::bprop()
{
    varray.last()->gradient += gradient;
    int n = size();
    int l = varray.length()-1;
    for(int i=1; i<l; i++)
    {
        if(input_is_discrete[i-1])
        {
            int r = (int)varray[0]->valuedata[i-1];
            real* row = varray[i]->matGradient.row(r).data();
            for(int j=0; j<n; j++)
                row[j] += gradientdata[j] ;
            varray[i]->updateRow(r);
        }
        else
        {
            for(int j=0; j<n; j++)
            {
                varray[i]->gradientdata[j] += gradientdata[j]*varray[0]->valuedata[i-1];
                varray[0]->gradientdata[i-1] += gradientdata[j]*varray[i]->valuedata[j];
            }
        }
    }

}

void HeterogenuousAffineTransformVariable::build()
{
    inherited::build();
    build_();
}

void HeterogenuousAffineTransformVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(input_is_discrete, copies);
    //PLERROR("HeterogenuousAffineTransformVariable::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void HeterogenuousAffineTransformVariable::declareOptions(OptionList& ol)
{    
    declareOption(ol, "input_is_discrete", &HeterogenuousAffineTransformVariable::input_is_discrete,
                  OptionBase::buildoption,
                  "Indication whether each component of the input is discrete or not.");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void HeterogenuousAffineTransformVariable::build_()
{
    if(varray[0]->size() != varray.length()-2)
        PLERROR("In HeterogenuousAffineTransformVariable::build_(): The number of weight variables (%d) and input size (%d) is not the same", varray.length()-2, varray[0]->size());
    if(input_is_discrete.length() != varray[0]->size())
        PLERROR("In HeterogenuousAffineTransformVariable::build_(): input_is_discrete size (%d) and input size (%d) does not match", input_is_discrete.length(), varray[0]->size());
    if(!varray[0]->isVec())
        PLERROR("In HeterogenuousAffineTransformVariable::build_(): input should be a vector");
    for(int i=1; i<varray.length(); i++)
    {
        if(varray[i]->width() != varray[1]->width())
            PLERROR("In HeterogenuousAffineTransformVariable::build_(): %dth weight matrix has width %d, should be %d", i, varray[i]->width(), size());
        if(i<varray.length()-1 && input_is_discrete[i-1])
            varray[i-1]->allowPartialUpdates();
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
