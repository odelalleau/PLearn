// -*- C++ -*-

// RealFunctionsProcessedVMatrix.cc
//
// Copyright (C) 2006 Xavier Saint-Mleux
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

// Author: Xavier Saint-Mleux

/*! \file RealFunctionsProcessedVMatrix.cc */


#include "RealFunctionsProcessedVMatrix.h"

namespace PLearn {
using namespace std;


RealFunctionsProcessedVMatrix::RealFunctionsProcessedVMatrix(VMat source_,
                                                             const TVec<RealFunc>& functions_,
                                                             bool put_raw_input_,
                                                             bool put_non_input_,
                                                             bool call_build_)
    :inherited(source_, call_build_),
     functions(functions_),
     put_raw_input(put_raw_input_),
     put_non_input(put_non_input_)
{
    if( call_build_ )
        build_();
}


PLEARN_IMPLEMENT_OBJECT(RealFunctionsProcessedVMatrix,
                        "","");
////////////////////
// declareOptions //
////////////////////
void RealFunctionsProcessedVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "functions",
                  &RealFunctionsProcessedVMatrix::functions,
                  OptionBase::buildoption,
                  "The functions used to process the inputs from the source VMat.");

    declareOption(ol, "put_raw_input", &RealFunctionsProcessedVMatrix::put_raw_input,
                  OptionBase::buildoption,
                  "Whether to include in the input part of this VMatrix the"
                  " raw input part\n"
                  "of 'source'.\n");

    declareOption(ol, "put_non_input", &RealFunctionsProcessedVMatrix::put_non_input,
                  OptionBase::buildoption,
                  "Whether to include in this VMatrix the original target and"
                  " weights.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void RealFunctionsProcessedVMatrix::build_()
{
    if(source)
    {
        updateMtime(source);
        length_= source->length();
        if(functions)
        {
            width_= functions->length();
            if(put_raw_input)
                width_+= source->inputsize();
            if(put_non_input)
                width_+= source->targetsize() + source->weightsize();
        }
    }
}

///////////
// build //
///////////
void RealFunctionsProcessedVMatrix::build()
{
    inherited::build();
    build_();
}

void RealFunctionsProcessedVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(input, copies);
    deepCopyField(targ, copies);
    deepCopyField(functions, copies);
}





void RealFunctionsProcessedVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getExample(i, input, targ, weight);
    Vec v2;
    evaluate_functions(functions, input, v2);
    int l0= 0, l1= v2.length();
    v.subVec(l0, l1) << v2;
    l0+= l1;
    if(put_raw_input)
    {
        l1= input.length();
        v.subVec(l0, l1) << input;
        l0+= l1;
    }
    if(put_non_input)
    {
        l1= targ.length();
        v.subVec(l0, l1) << targ;
        if(0 < weightsize_)
            v[l0+l1]= weight;
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
