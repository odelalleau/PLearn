// -*- C++ -*-

// ExtractNNetParamsVMatrix.cc
//
// Copyright (C) 2005 Olivier Delalleau
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
 * $Id$
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file ExtractNNetParamsVMatrix.cc */


#include "ExtractNNetParamsVMatrix.h"

namespace PLearn {
using namespace std;


//////////////////////////////
// ExtractNNetParamsVMatrix //
//////////////////////////////
ExtractNNetParamsVMatrix::ExtractNNetParamsVMatrix()
    : extract_w1(false),
      extract_w2(false),
      extract_wdirect(false),
      extract_wout(false)
{}

PLEARN_IMPLEMENT_OBJECT(ExtractNNetParamsVMatrix,
                        "Extract the (learned) parameters of a Neural Network.",
                        "Currently, it can only extract ONE set of parameters, among w1, w2, wdirect\n"
                        "and wout. In the future, it may be possible to extract simultaneously all\n"
                        "those parameters."
    );

///////////////
// getNewRow //
///////////////
void ExtractNNetParamsVMatrix::getNewRow(int i, const Vec& v) const
{
    v << data(i);
}

////////////////////
// declareOptions //
////////////////////
void ExtractNNetParamsVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "nnet", &ExtractNNetParamsVMatrix::nnet, OptionBase::buildoption,
                  "The neural network whose parameters are extracted.");

    declareOption(ol, "extract_w1", &ExtractNNetParamsVMatrix::extract_w1, OptionBase::buildoption,
                  "Whether to extract w1.");

    declareOption(ol, "extract_w2", &ExtractNNetParamsVMatrix::extract_w2, OptionBase::buildoption,
                  "Whether to extract w2.");

    declareOption(ol, "extract_wdirect", &ExtractNNetParamsVMatrix::extract_wdirect, OptionBase::buildoption,
                  "Whether to extract wdirect.");

    declareOption(ol, "extract_wout", &ExtractNNetParamsVMatrix::extract_wout, OptionBase::buildoption,
                  "Whether to extract wout.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "length", &ExtractNNetParamsVMatrix::length_, OptionBase::nosave,
                    "Overwritten at build time.");

    redeclareOption(ol, "width", &ExtractNNetParamsVMatrix::width_, OptionBase::nosave,
                    "Overwritten at build time.");

}

////////////
// build_ //
////////////
void ExtractNNetParamsVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ExtractNNetParamsVMatrix::build_()
{
    if (nnet) {
        Mat m;
        if (extract_w1)
            m = nnet->getW1();
        else if (extract_w2)
            m = nnet->getW2();
        else if (extract_wdirect)
            m = nnet->getWdirect();
        else if (extract_wout)
            m = nnet->getWout();
        data.resize(m.length(), m.width());
        data << m;
        length_ = data.length();
        width_ = data.width();
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ExtractNNetParamsVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    // deepCopyField(trainvec, copies);
    PLERROR("ExtractNNetParamsVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
