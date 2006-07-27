// -*- C++ -*-

// PythonProcessedLearner.cc
//
// Copyright (C) 2006 Nicolas Chapados
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

// Authors: Nicolas Chapados

/*! \file PythonProcessedLearner.cc */


#include "PythonProcessedLearner.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PythonProcessedLearner,
    "Allows preprocessing operations to be carried out by a Python code snippet",
    "This PLearner allows embedding a PythonCodeSnippet, and define Python\n"
    "operations to be carried out during the computeOutput.  The current\n"
    "implementation does not attempt to provide a full implementability of the\n"
    "PLearner protocol in Python -- only computeOutput is supported for now, and\n"
    "the intended use is to specify some fixed preprocessing inside a\n"
    "ChainedLearner, such as what would otherwise be performed by a\n"
    "VPLPreprocessedLearner.\n"
    "\n"
    "The Python code snippet (see the option 'code' below) must define the\n"
    "following functions:\n"
    "\n"
    " def getOutputNames(train_fieldnames, inputsize, targetsize, weightsize, extrasize):\n"
    "     \"\"\"Return the names of the outputs computed by the learner, namely\n"
    "     the implementation of the PLearner::getOutputNames() function.\n"
    "     This is called every time setTrainingSet() is called on the PLearner.\"\"\"\n"
    "\n"
    " def computeOutput(input):\n"
    "     \"\"\"Return the result of the computation.  The size of the output\n"
    "     vector must be the same as the number of elements returned by the\n"
    "     getOutputNames() function.\"\"\"\n"
    "\n"
    "The Python code snippet has access to the following (injected) interface:\n"
    "\n"
    "- getParams(): return the map with the contents of the 'params' option.\n"
    "\n"
    "- getParam(key): return the string value of params[key].\n"
    "\n"
    "- setParam(key,value): set a new value for the 'params' element\n"
    "  corresponding to key.\n"
    );

PythonProcessedLearner::PythonProcessedLearner()
    : m_code(""),
      python()
{ }

void PythonProcessedLearner::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "code", &PythonProcessedLearner::m_code, OptionBase::buildoption,
        "The Python code snippet.  The functions described in the class\n"
        "documentation must be provided.  Note that, after an initial build(),\n"
        "changing this string calling build() again DOES NOT result in the\n"
        "recompilation of the code.\n");

    declareOption(
        ol, "params", &PythonProcessedLearner::m_params, OptionBase::buildoption,
        "General-purpose parameters that are injected into the Python code\n"
        "snippet and accessible via the getParam/setParam functions.  Can be\n"
        "used for passing processing arguments to the Python code.\n");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


//#####  build  ###############################################################

void PythonProcessedLearner::build()
{
    inherited::build();
    build_();
}

void PythonProcessedLearner::build_()
{
    // First step, compile the Python code
    compileAndInject();
    assert( python );

    // Ensure that the required functions are defined
    const char* FUNC_ERR = "%s: the Python code snippet must define the function '%s'";
    if (! python->isInvokable("getOutputNames"))
        PLERROR(FUNC_ERR, __FUNCTION__, "getOutputNames");
    if (! python->isInvokable("computeOutput"))
        PLERROR(FUNC_ERR, __FUNCTION__, "computeOutput");
}


void PythonProcessedLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(m_outputnames, copies);

    // Recompile the Python code in a fresh environment
    python = 0;
    compileAndInject();
}


void PythonProcessedLearner::setTrainingSet(VMat training_set, bool call_forget)
{
    inherited::setTrainingSet(training_set, call_forget);
    VMat trset = getTrainingSet();
    assert( trset  );
    assert( python );
    
    TVec<string> fields = trset->fieldNames();
    int inputsize  = trset->inputsize();
    int targetsize = trset->targetsize();
    int weightsize = trset->weightsize();
    int extrasize  = trset->extrasize();

    m_outputnames = python->invoke("getOutputNames", fields, inputsize, targetsize,
                                   weightsize, extrasize).as< TVec<string> >();
}


int PythonProcessedLearner::outputsize() const
{
    assert( python );
    return m_outputnames.size();
}

void PythonProcessedLearner::forget()
{
    // no-op in current version
}

void PythonProcessedLearner::train()
{
    // No-op in current version
}


void PythonProcessedLearner::computeOutput(const Vec& input, Vec& output) const
{
    assert( python );
    Vec processed = python->invoke("computeOutput", input).as<Vec>();
    output << processed;
}

void PythonProcessedLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                                     const Vec& target, Vec& costs) const
{
    // No-op in current version
}

TVec<string> PythonProcessedLearner::getTestCostNames() const
{
    return TVec<string>();
}

TVec<string> PythonProcessedLearner::getTrainCostNames() const
{
    return TVec<string>();
}

TVec<string> PythonProcessedLearner::getOutputNames() const
{
    return m_outputnames;
}


//#####  getParams  ###########################################################

PythonObjectWrapper
PythonProcessedLearner::getParams(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 0)
        PLERROR("PythonProcessedLearner::getParams: expected 0 argument; got %d",
                args.size());

    return PythonObjectWrapper(m_params);
}


//#####  getParam  ############################################################

PythonObjectWrapper
PythonProcessedLearner::getParam(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 1)
        PLERROR("PythonProcessedLearner::getParam: expected 1 argument; got %d",
                args.size());
    string key = args[0].as<string>();
    map<string,string>::const_iterator found = m_params.find(key);
    if (found != m_params.end())
        return PythonObjectWrapper(found->second);
    else
        return PythonObjectWrapper();        // None
}


//#####  setParam  ############################################################

PythonObjectWrapper
PythonProcessedLearner::setParam(const TVec<PythonObjectWrapper>& args)
{
    if (args.size() != 2)
        PLERROR("PythonProcessedLearner::setParam: expected 2 arguments; got %d",
                args.size());
    string key   = args[0].as<string>();
    string value = args[1].as<string>();
    m_params[key] = value;
    return PythonObjectWrapper();
}


//#####  compileAndInject  ####################################################

void PythonProcessedLearner::compileAndInject()
{
    if (! python) {
        python = new PythonCodeSnippet(m_code);
        assert( python );
        python->build();
        python->inject("getParams",    this, &PythonProcessedLearner::getParams);
        python->inject("getParam",     this, &PythonProcessedLearner::getParam);
        python->inject("setParam",     this, &PythonProcessedLearner::setParam);
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
