// -*- C++ -*-

// RunICPVariable.cc
//
// Copyright (C) 2006 Olivier Delalleau
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

/*! \file RunICPVariable.cc */


#include "RunICPVariable.h"

namespace PLearn {
using namespace std;

/** RunICPVariable **/

PLEARN_IMPLEMENT_OBJECT(
    RunICPVariable,
    "ONE LINE USER DESCRIPTION",
    "MULTI LINE\nHELP FOR USERS"
    );

RunICPVariable::RunICPVariable()
    /* ### Initialize all fields to their default value */
{
    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

// constructor from input variable.
RunICPVariable::RunICPVariable(Variable* input)
// ### replace with actual parameters
//  : inherited(input, this_variable_length, this_variable_width),
//    parameter(default_value),
//    ...
{
    // ### You may (or not) want to call build_() to finish building the object
}

// constructor from input variable and parameters
// RunICPVariable::RunICPVariable(Variable* input, param_type the_parameter,...)
// ### replace with actual parameters
//  : inherited(input, this_variable_length, this_variable_width),
//    parameter(the_parameter),
//    ...
//{
//    // ### You may (or not) want to call build_() to finish building the
//    // ### object
//}

void RunICPVariable::recomputeSizes(int& l, int& w) const
{
    // ### usual code to put here is:
    /*
        if (input) {
            l = ... ; // the computed length of this Var
            w = ... ; // the computed width
        } else
            l = w = 0;
    */
}

// ### computes value from input's value
void RunICPVariable::fprop()
{
    // ### remove this line when implemented
    PLERROR("In RunICPVariable - fprop() must be implemented.");
}

// ### computes input's gradient from gradient
void RunICPVariable::bprop()
{
    // ### remove this line when implemented
    PLERROR("In RunICPVariable - bprop() must be implemented.");
}

// ### You can implement these methods:
// void RunICPVariable::bbprop() {}
// void RunICPVariable::symbolicBprop() {}
// void RunICPVariable::rfprop() {}


// ### Nothing to add here, simply calls build_
void RunICPVariable::build()
{
    inherited::build();
    build_();
}

void RunICPVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### If you want to deepCopy a Var field:
    // varDeepCopyField(somevariable, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("RunICPVariable::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void RunICPVariable::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &RunICPVariable::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RunICPVariable::build_()
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
