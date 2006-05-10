// -*- C++ -*-

// ConstantVMatrix.cc
//
// Copyright (C) 2005 Nicolas Chapados
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
 * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $
 ******************************************************* */

// Authors: Nicolas Chapados

/*! \file ConstantVMatrix.cc */

#include "ConstantVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ConstantVMatrix,
    "VMatrix that returns a constant element.",
    "This VMatrix returns a constant element (specified upon construction\n"
    "through the constant_output option)");

ConstantVMatrix::ConstantVMatrix()
    : inherited(), constant_output(MISSING_VALUE)
{ }

ConstantVMatrix::ConstantVMatrix(int the_length, int the_width, real constant)
    : inherited(the_length, the_width), constant_output(constant)
{ }

////////////////////
// declareOptions //
////////////////////
void ConstantVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "constant_output", &ConstantVMatrix::constant_output,
                  OptionBase::buildoption,
                  "The constant output to return");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ConstantVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ConstantVMatrix::build_()
{
}

/////////
// get //
/////////
real ConstantVMatrix::get(int i, int j) const
{
    return constant_output;
}

///////////////
// getSubRow //
///////////////
void ConstantVMatrix::getSubRow(int i, int j, Vec v) const
{
    v.fill(constant_output);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ConstantVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
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
