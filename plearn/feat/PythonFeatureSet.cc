// -*- C++ -*-

// PythonFeatureSet.cc
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

/*! \file PythonFeatureSet.cc */


#include "PythonFeatureSet.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PythonFeatureSet,
    "FeatureSet with features being defined using a python script.",
    "The python script should define a getNewFeaturesString function\n"
    "which has to output a vector of string features for the input token.\n"
    );

PythonFeatureSet::PythonFeatureSet()
    : python()
{}

// ### Nothing to add here, simply calls build_
void PythonFeatureSet::build()
{
    inherited::build();
    build_();
}

void PythonFeatureSet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    //deepCopyField(python, copies);
    //PLERROR("PythonFeatureSet::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void PythonFeatureSet::declareOptions(OptionList& ol)
{
    declareOption(ol, "code", &PythonFeatureSet::code,
                  OptionBase::buildoption,
                  "Snippet of python code that outputs the features");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void PythonFeatureSet::build_()
{
    if (code != "")
    {
        python = new PythonCodeSnippet(code);
        PLASSERT( python );
        python->build();
    }

}

void PythonFeatureSet::getNewFeaturesString(string token, TVec<string>& feats_str)
{
    f_str.resize(1);
    f_str[0] = token;
    if(!python) build_();
    f_str = python->invoke("getNewFeaturesString",f_str).as<TVec<string> >();
    feats_str.resize(f_str.length());
    feats_str << f_str;
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
