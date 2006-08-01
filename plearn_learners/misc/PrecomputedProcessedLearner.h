// -*- C++ -*-

// PrecomputedProcessedLearner.h
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

/*! \file PrecomputedProcessedLearner.h */

#ifndef PrecomputedProcessedLearner_INC
#define PrecomputedProcessedLearner_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

/**
 *  Identity Learner with a cached 'processDataSet' method.
 *
 *  This learner is functionally the identity learner: it does not learn
 *  anything, and its computeOutput() produces the same thing as its input.
 *  HOWEVER, it implements a special function: the results of calls to
 *  processDataSet produce a CACHED VMatrix of the results, thereby making
 *  further accesses much faster.
 *
 *  The intended use of this learner is within a chain managed by a
 *  ChainedLearner, wherein one has the pattern
 *
 *  - 1) Preprocessing, with fairly slow Python (PythonProcessedLearner)
 *  - 2) Precomputing (this class)
 *  - 3) NNet or other PLearner which makes multiple passes over its training
 *       set.
 */
class PrecomputedProcessedLearner : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    /**
     *  Buffering type for precomputed train operations.  Can be 'memory',
     *  'dmat', or 'pmat'.  In the first case, a MemoryVMatrix is returned from
     *  calls to ProcessDataSet.  In the last two cases, a PrecomputedVMatrix
     *  is returned, with the appropriate precompute type set.  The metadatadir
     *  of the vmat is set to be the learner's expdir/ followed by
     *  'precomputed_processed.metadata'.
     */
    string m_precomp_type;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    PrecomputedProcessedLearner();


    //#####  PLearner Member Functions  #######################################

    /// Return the inputsize from the last setTrainingSet.
    virtual int outputsize() const;

    /// No-op in this learner
    virtual void forget();

    /// No-op in this learner
    virtual void train();

    //! Computes the output from the input: identity function
    virtual void computeOutput(const Vec& input, Vec& output) const;

    /// No-op in this learner
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    /// This learner does not have any test costs
    virtual TVec<std::string> getTestCostNames() const;

    /// This learner does not have any train costs
    virtual TVec<std::string> getTrainCostNames() const;

    /**
     *  Process a full dataset (possibly containing input,target,weight,extra
     *  parts). Returns the PRECOMPUTED processed view of that dataset; this is
     *  done by passing the result from the inherited processDataSet through
     *  the appropriate VMatrix (depending on precomp_type).
     */
    virtual VMat processDataSet(VMat dataset) const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(PrecomputedProcessedLearner);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //! This does the actual building.
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PrecomputedProcessedLearner);

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
