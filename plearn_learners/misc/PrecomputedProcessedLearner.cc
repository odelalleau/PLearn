// -*- C++ -*-

// PrecomputedProcessedLearner.cc
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

/*! \file PrecomputedProcessedLearner.cc */

#define PL_LOG_MODULE_NAME "PrecomputedProcessedLearner"

#include "PrecomputedProcessedLearner.h"
#include <plearn/io/pl_log.h>
#include <plearn/vmat/PrecomputedVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn/sys/procinfo.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PrecomputedProcessedLearner,
    "Identity Learner with a cached 'processDataSet' method.",
    "This learner is functionally the identity learner: it does not learn\n"
    "anything, and its computeOutput() produces the same thing as its input.\n"
    "HOWEVER, it implements a special function: the results of calls to\n"
    "processDataSet produce a CACHED VMatrix of the results, thereby making\n"
    "further accesses much faster.\n"
    "\n"
    "The intended use of this learner is within a chain managed by a\n"
    "ChainedLearner, wherein one has the pattern\n"
    "\n"
    "- 1) Preprocessing, with fairly slow Python (PythonProcessedLearner)\n"
    "- 2) Precomputing (this class)\n"
    "- 3) NNet or other PLearner which makes multiple passes over its training\n"
    "     set.\n");

    
PrecomputedProcessedLearner::PrecomputedProcessedLearner()
    : m_precomp_type("memory")
{ }

void PrecomputedProcessedLearner::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "precomp_type", &PrecomputedProcessedLearner::m_precomp_type,
        OptionBase::buildoption,
        "Buffering type for precomputed train operations.  Can be 'memory',\n"
        "'dmat', or 'pmat'.  In the first case, a MemoryVMatrix is returned from\n"
        "calls to ProcessDataSet.  In the last two cases, a PrecomputedVMatrix\n"
        "is returned, with the appropriate precompute type set.  The metadatadir\n"
        "of the vmat is set to be the learner's expdir/ followed by\n"
        "'precomputed_processed.metadata'.\n");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void PrecomputedProcessedLearner::build_()
{
    if (m_precomp_type != "memory" &&
        m_precomp_type != "dmat"   &&
        m_precomp_type != "pmat" )
        PLERROR("%s: unknown value for option 'precomp_type' (= '%s');\n"
                "must be one of: 'memory', 'dmat', 'pmat'", __FUNCTION__,
                m_precomp_type.c_str());
}


// ### Nothing to add here, simply calls build_
void PrecomputedProcessedLearner::build()
{
    inherited::build();
    build_();
}


void PrecomputedProcessedLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


int PrecomputedProcessedLearner::outputsize() const
{
    return inputsize_;
}

void PrecomputedProcessedLearner::forget()
{
    inherited::forget();
}

void PrecomputedProcessedLearner::train()
{
    // No-op
}


void PrecomputedProcessedLearner::computeOutput(const Vec& input, Vec& output) const
{
    output << input;
}


void PrecomputedProcessedLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                                          const Vec& target, Vec& costs) const
{
    // No-op
}


TVec<string> PrecomputedProcessedLearner::getTestCostNames() const
{
    return TVec<string>();
}


TVec<string> PrecomputedProcessedLearner::getTrainCostNames() const
{
    return TVec<string>();
}


VMat PrecomputedProcessedLearner::processDataSet(VMat dataset) const
{
    VMat raw_processed = inherited::processDataSet(dataset);

    DBG_MODULE_LOG << "BEFORE PRECOMPUTE. Process memory: "
                   << getProcessDataMemory() << endl;

    MODULE_LOG << "Precomputing "
               << raw_processed.length() << 'x' << raw_processed.width()
               << " to " << m_precomp_type
               << endl;
    
    if (m_precomp_type == "memory") {
        Mat precomp = raw_processed.toMat();

        DBG_MODULE_LOG << "AFTER PRECOMPUTE. Process memory: "
                       << getProcessDataMemory() << endl;

        return VMat(precomp);
    }
    else {
        PPath expdir = getExperimentDirectory();
        if (expdir.empty())
            PLERROR("%s: an experiment directory must be set in order to use precomp_type '%s'",
                    __FUNCTION__, m_precomp_type.c_str());
        
        PP<PrecomputedVMatrix> precomp = new PrecomputedVMatrix;
        precomp->source = raw_processed;
        precomp->precomp_type = m_precomp_type;
        precomp->build();
        precomp->setMetaDataDir(expdir / "precomputed_processed.metadata");

        DBG_MODULE_LOG << "AFTER PRECOMPUTE. Process memory: "
                       << getProcessDataMemory() << endl;

        return (VMatrix*)precomp;
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
