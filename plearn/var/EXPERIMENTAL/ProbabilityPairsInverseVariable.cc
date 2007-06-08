// -*- C++ -*-

// ProbabilityPairsInverseVariable.cc
//
// Copyright (C) 2007 Simon Lemieux, Pascal Vincent
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

// Authors: Simon Lemieux

/*! \file ProbabilityPairsInverseVariable.cc */


#include "ProbabilityPairsInverseVariable.h"

namespace PLearn {
using namespace std;

/** ProbabilityPairsInverseVariable **/

PLEARN_IMPLEMENT_OBJECT(
    ProbabilityPairsInverseVariable,
    "[x1,x2,x3,...,xn] -> [f(x1), f(x3), ..., f(xn)] with f(x) = (max-min)*x - min and with n even",
    "MULTI LINE\nHELP FOR USERS"
    );


ProbabilityPairsInverseVariable::ProbabilityPairsInverseVariable()
    : min(0.), max(1.)
{}

ProbabilityPairsInverseVariable::ProbabilityPairsInverseVariable(Variable* input, real min, real max)
    : inherited(input, input->length(), input->width()/2),
      min(min),max(max)
{
    build_();
}

ProbabilityPairsInverseVariable::ProbabilityPairsInverseVariable(Variable* input, real max)
    :inherited(input, input->length(), input->width()/2),
     min(0.), max(max)
{
    build_();
}

ProbabilityPairsInverseVariable::ProbabilityPairsInverseVariable(Variable* input)
    :inherited(input, input->length(), input->width()/2),
     min(0.), max(1.)
{
    build_();
}

void ProbabilityPairsInverseVariable::recomputeSize(int& l, int& w) const
{
        if (input) {
            l = input->length(); // the computed length of this Var
            w = input->width()/2; // the computed width
        } else
            l = w = 0;
}

// ### computes value from input's value
void ProbabilityPairsInverseVariable::fprop()
{
    for(int n=0; n<input->length(); n++)
        for(int i=0; i<input->width()/2; i++)
            matValue(n,i) = input->matValue(n,i*2)*(max-min) + min;
}

// ### computes input's gradient from gradient
void ProbabilityPairsInverseVariable::bprop()
{
    for(int n=0; n<input->length(); n++)
        for(int i=0; i<input->width()/2; i++)
            input->matGradient(n,i*2) += matGradient(n,i)*(max-min);
}

// ### You can implement these methods:
// void ProbabilityPairsInverseVariable::bbprop() {}
// void ProbabilityPairsInverseVariable::symbolicBprop() {}
// void ProbabilityPairsInverseVariable::rfprop() {}


// ### Nothing to add here, simply calls build_
void ProbabilityPairsInverseVariable::build()
{
    inherited::build();
    build_();
}

void ProbabilityPairsInverseVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### If you want to deepCopy a Var field:
    // varDeepCopyField(somevariable, copies);
}

void ProbabilityPairsInverseVariable::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

  
    declareOption(ol, "min", &ProbabilityPairsInverseVariable::min,
                  OptionBase::buildoption,
                  "The lower bound a value of the input should be. It will be used to calculate the corresponding probability of each input.");

    declareOption(ol, "max", &ProbabilityPairsInverseVariable::max,
                  OptionBase::buildoption,
                  "analog to min");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ProbabilityPairsInverseVariable::build_()
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
