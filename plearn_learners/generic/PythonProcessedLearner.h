// -*- C++ -*-

// PythonProcessedLearner.h
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

/*! \file PythonProcessedLearner.h */


#ifndef PythonProcessedLearner_INC
#define PythonProcessedLearner_INC

// Python includes must come first! (as per Python doc)
#include <plearn/python/PythonCodeSnippet.h>

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

/**
 *  Allows preprocessing operations to be carried out by a Python code snippet
 *
 *  This PLearner allows embedding a PythonCodeSnippet, and define Python
 *  operations to be carried out during the computeOutput.  The current
 *  implementation does not attempt to provide a full implementability of the
 *  PLearner protocol in Python -- only computeOutput is supported for now, and
 *  the intended use is to specify some fixed preprocessing inside a
 *  ChainedLearner, such as what would otherwise be performed by a
 *  VPLPreprocessedLearner.
 *
 *  The Python code snippet (see the option 'code' below) must define the
 *  following functions:
 *
 *   def getOutputNames(train_fieldnames, inputsize, targetsize, weightsize, extrasize):
 *       """Return the names of the outputs computed by the learner, namely
 *       the implementation of the PLearner::getOutputNames() function.
 *       This is called every time setTrainingSet() is called on the PLearner."""
 *
 *   def computeOutput(input):
 *       """Return the result of the computation.  The size of the output
 *       vector must be the same as the number of elements returned by the
 *       getOutputNames() function."""
 *
 *  The Python code snippet has access to the following (injected) interface:
 *
 *  - getParams(): return the map with the contents of the 'params' option.
 *
 *  - getParam(key): return the string value of params[key].
 *
 *  - setParam(key,value): set a new value for the 'params' element
 *    corresponding to key.
 */
class PythonProcessedLearner : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    /**
     *  The Python code snippet.  The functions described in the class
     *  documentation must be provided.  Note that, after an initial build(),
     *  changing this string calling build() again DOES NOT result in the
     *  recompilation of the code.
     */
    string m_code;

    /**
     *  General-purpose parameters that are injected into the Python code
     *  snippet and accessible via the getParam/setParam functions.  Can be
     *  used for passing processing arguments to the Python code.
     */
    map<string,string> m_params;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    PythonProcessedLearner();


    //#####  PLearner Member Functions  #######################################

    //! Calls the Python getOutputNames() function and remember the result
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    //! Currently a no-op
    virtual void forget();

    //! Currently a no-op
    virtual void train();

    //! Computes the output from the input: actually calls Python.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output: currently a no-op
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    //! Empty vector: no test costs are defined
    virtual TVec<std::string> getTestCostNames() const;

    //! Empty vector: no train costs are defined
    virtual TVec<std::string> getTrainCostNames() const;

    //! Returns the output names cached from the last setTrainingSet()
    virtual TVec<string> getOutputNames() const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(PythonProcessedLearner);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Injected into the Python code to return m_params.
    PythonObjectWrapper getParams(const TVec<PythonObjectWrapper>& args) const;

    //! Injected into the Python code to return the m_param value associated
    //! to the given key (sole argument).  Returns None if not found.
    PythonObjectWrapper getParam(const TVec<PythonObjectWrapper>& args) const;

    //! Injected into the Python code to set the m_param value associated
    //! to the given key.  Always return None.
    PythonObjectWrapper setParam(const TVec<PythonObjectWrapper>& args);
    
    //! If not already done, compile the Python snippet and inject the
    //! required stuff into the Python environment
    void compileAndInject();
    
protected:
    //! Cached version of the output names 
    TVec<string> m_outputnames;

    //! Actual Python environment
    PP<PythonCodeSnippet> python;

private:
    //! This does the actual building.
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PythonProcessedLearner);

} // end of namespace PLearn

#endif


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
