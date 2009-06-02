// -*- C++ -*-

// SaltPepperNoiseVariable.cc
//
// Copyright (C) 2009 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file SaltPepperNoiseVariable.cc */


#include "SaltPepperNoiseVariable.h"

namespace PLearn {
using namespace std;

/** SaltPepperNoiseVariable **/

PLEARN_IMPLEMENT_OBJECT(
    SaltPepperNoiseVariable,
    "ONE LINE USER DESCRIPTION",
    "MULTI LINE\nHELP FOR USERS"
    );

SaltPepperNoiseVariable::SaltPepperNoiseVariable()
    :corruption_prob(0),
     pepper_prob(0.5),
     salt_value(0),
     pepper_value(1)
{
    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

// constructor from input variable and parameters
// SaltPepperNoiseVariable::SaltPepperNoiseVariable(Variable* input, param_type the_parameter,...)
// ### replace with actual parameters
//  : inherited(input, this_variable_length, this_variable_width),
//    parameter(the_parameter),
//    ...
//{
//    // ### You may (or not) want to call build_() to finish building the
//    // ### object
//}

void SaltPepperNoiseVariable::recomputeSize(int& l, int& w) const
{
    if (input) 
    {
        l = input.length();
        w = input.width(); // the computed width
    } 
    else
        l = w = 0;
}

// ### computes value from input's value
void SaltPepperNoiseVariable::fprop()
{
    checkContiguity();

    if(random_gen.isNull())
        random_gen = PRandom::common(false);


    int n = value.length();
    corrupted.resize(n);

    for(int i=0; i<n; i++)
    {
        if(random_gen->uniform_sample()<corruption_prob)
        {
            if(random_gen->uniform_sample()<pepper_prob)
                valuedata[i] = pepper_value;
            else
                valuedata[i] = salt_value;

            corrupted[i] = true;
        }
        else
        {
            valuedata[i] = input->valuedata[i];
            corrupted[i] = false;
        }
    }
}

// ### computes input's gradient from gradient
void SaltPepperNoiseVariable::bprop()
{
    int n = gradient.length();
    for(int i=0; i<n; i++)
    {
        if(!corrupted[i])
            input->gradientdata[i] += gradientdata[i];
    }
}

// ### You can implement these methods:
// void SaltPepperNoiseVariable::bbprop() {}
// void SaltPepperNoiseVariable::symbolicBprop() {}
// void SaltPepperNoiseVariable::rfprop() {}


// ### Nothing to add here, simply calls build_
void SaltPepperNoiseVariable::build()
{
    inherited::build();
    build_();
}

void SaltPepperNoiseVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    deepCopyField(random_gen, copies);

    // ### If you want to deepCopy a Var field:
    // varDeepCopyField(somevariable, copies);
}

void SaltPepperNoiseVariable::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "corruption_prob", &SaltPepperNoiseVariable::corruption_prob,
                  OptionBase::buildoption,
                  "The probability of corrupting each input element");
    declareOption(ol, "pepper_prob", &SaltPepperNoiseVariable::pepper_prob,
                  OptionBase::buildoption,
                  "If an element is to be corrupted, it will be set to pepper_value with probability pepper_prob and to salt_value with probability 1-pepper_prob.");
    declareOption(ol, "salt_value", &SaltPepperNoiseVariable::salt_value,
                  OptionBase::buildoption,
                  "The value to which to set all inputs that have been elected to be corrupted to salt.");
    declareOption(ol, "pepper_value", &SaltPepperNoiseVariable::pepper_value,
                  OptionBase::buildoption,
                  "The value to which to set all inputs that have been elected to be corrupted to pepper.");
    declareOption(ol, "random_gen", &SaltPepperNoiseVariable::random_gen,
                  OptionBase::buildoption,
                  "Random number generator. If null, the PRandom::common(false) generator will be used.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void SaltPepperNoiseVariable::build_()
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
