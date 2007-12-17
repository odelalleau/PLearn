// -*- C++ -*-

// BernoulliSampleVariable.cc
//
// Copyright (C) 2007 Pascal Vincent
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

/*! \file BernoulliSampleVariable.cc */


#include "BernoulliSampleVariable.h"

namespace PLearn {
using namespace std;

/** BernoulliSampleVariable **/

PLEARN_IMPLEMENT_OBJECT(
    BernoulliSampleVariable,
    "Input gives a vector of parameters, each of which is the probability parameter of a Bernoulli. Output is sample from those." ,
    "\n"
    );


//! Constructor

BernoulliSampleVariable::BernoulliSampleVariable(Variable* input)
    : inherited(input, input->length(), input->width()),
      random_gen(0)
{
    build_();
}

void BernoulliSampleVariable::recomputeSize(int& l, int& w) const
{
    if (input) {
        l = input->length();
        w = input->width() ;
    } else
        l = w = 0;
}

// ### computes value from input's value
void BernoulliSampleVariable::fprop()
{
    checkContiguity();

    if(random_gen.isNull())
        random_gen = PRandom::common(false);

    int l = nelems();
    real* valueptr = valuedata;
    real* inputvalueptr = input->valuedata;
    for(int i=0; i<l; i++)
        *valueptr++ = random_gen->binomial_sample(*inputvalueptr++);
}

// ### computes input's gradient from gradient
void BernoulliSampleVariable::bprop()
{}    

// ### You can implement these methods:
// void BernoulliSampleVariable::bbprop() {}
// void BernoulliSampleVariable::symbolicBprop() {}
// void BernoulliSampleVariable::rfprop() {}


// ### Nothing to add here, simply calls build_
void BernoulliSampleVariable::build()
{
    inherited::build();
    build_();
}

void BernoulliSampleVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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

void BernoulliSampleVariable::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    declareOption(ol, "random_gen", &BernoulliSampleVariable::random_gen,
                  OptionBase::buildoption,
                  "Random number generator. If null, the PRandom::common(false) generator will be used.");
            
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void BernoulliSampleVariable::build_()
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
