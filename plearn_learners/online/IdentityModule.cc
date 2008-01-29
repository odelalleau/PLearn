// -*- C++ -*-

// IdentityModule.cc
//
// Copyright (C) 2007 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file IdentityModule.cc */



#include "IdentityModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    IdentityModule,
    "Module that simply replicates its input.",
    ""
);

IdentityModule::IdentityModule()
{}

////////////////////
// declareOptions //
////////////////////
void IdentityModule::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void IdentityModule::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.
}

// ### Nothing to add here, simply calls build_
void IdentityModule::build()
{
    inherited::build();
    build_();
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void IdentityModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

///////////
// fprop //
///////////
void IdentityModule::fprop(const Vec& input, Vec& output) const
{
    output.resize(input.length());
    output << input;
}

void IdentityModule::fprop(const Mat& inputs, Mat& outputs) {
    outputs.resize(inputs.length(), inputs.width());
    outputs << inputs;
}

////////////////////
// bpropAccUpdate //
////////////////////
void IdentityModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                    const TVec<Mat*>& ports_gradient)
{
    // Deal with 'standard case' only.
    PLASSERT( ports_gradient.length() == 2 );
    Mat* input_grad = ports_gradient[0];
    Mat* output_grad = ports_gradient[1];
    if (!input_grad)
        return;
    PLASSERT( output_grad && !output_grad->isEmpty() &&
              input_grad->isEmpty() );
    PLASSERT( input_grad->width() == output_grad->width() );
    input_grad->resize(output_grad->length(), input_grad->width());
    *input_grad += *output_grad;
}

/////////////////
// bpropUpdate //
/////////////////
/* THIS METHOD IS OPTIONAL
void IdentityModule::bpropUpdate(const Vec& input, const Vec& output,
                               Vec& input_gradient,
                               const Vec& output_gradient,
                               bool accumulate)
{
}
*/

/* THIS METHOD IS OPTIONAL
void IdentityModule::bpropUpdate(const Vec& input, const Vec& output,
                               const Vec& output_gradient)
{
}
*/

//////////////////
// bbpropUpdate //
//////////////////
/* THIS METHOD IS OPTIONAL
void IdentityModule::bbpropUpdate(const Vec& input, const Vec& output,
                                Vec& input_gradient,
                                const Vec& output_gradient,
                                Vec& input_diag_hessian,
                                const Vec& output_diag_hessian,
                                bool accumulate)
{
}
*/

/* THIS METHOD IS OPTIONAL
void IdentityModule::bbpropUpdate(const Vec& input, const Vec& output,
                                const Vec& output_gradient,
                                const Vec& output_diag_hessian)
{
}
*/

////////////
// forget //
////////////
void IdentityModule::forget()
{
    // Nothing to forget.
}

//////////////
// finalize //
//////////////
/* THIS METHOD IS OPTIONAL
void IdentityModule::finalize()
{
}
*/

//////////////////////
// bpropDoesNothing //
//////////////////////
/* THIS METHOD IS OPTIONAL
bool IdentityModule::bpropDoesNothing()
{
}
*/

/////////////////////
// setLearningRate //
/////////////////////
/* OPTIONAL
void IdentityModule::setLearningRate(real dynamic_learning_rate)
{
}
*/

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
