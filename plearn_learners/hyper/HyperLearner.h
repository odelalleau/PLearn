// -*- C++ -*-

// HyperLearner.h
// Copyright (C) 2003-2004 ApSTAT Technologies Inc.
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

// Author: Pascal Vincent

/* *******************************************************
 * $Id$
 ******************************************************* */
// Author: Pascal Vincent

/*! \file PLearn/plearn_learners/hyper/HyperLearner.h */

#ifndef HyperLearner_INC
#define HyperLearner_INC

#include "HyperCommand.h"
#include <plearn_learners/generic/EmbeddedLearner.h>
#include <plearn_learners/testers/PTester.h>

namespace PLearn {
using namespace std;

// ###### HyperLearner ########################################################

class HyperLearner: public EmbeddedLearner
{
    typedef EmbeddedLearner inherited;

private:
    //! This does the actual building.
    void build_();

protected:

    static void declareOptions(OptionList &ol);

public:

    // Build options
    PP<PTester> tester; //!< The kind of train/test to perform for each combination of hyper-parameters.
    TVec<string> option_fields; //!< learner option names to be reported in results table
    TVec<string> dont_restart_upon_change; //!< list of options that do not require calling build() and forget() upon changing their value.

    TVec< PP<HyperCommand> > strategy; //!< The strategy to follow to optimize hyper parameters

    bool provide_strategy_expdir; //!< should each strategy step be provided a directory expdir/Step#
    bool save_final_learner; //!< should final learner be saved in expdir/final_learner.psave
    bool save_strategy_learner; //!< should final learner be saved in expdir/Strat#/final_learner.psave
    bool reloaded; //!< needed for a warning
    // HyperLearner methods

    //! if true, we finalize the learner after training.
    bool finalize_learner;

    HyperLearner();

    inline void setLearner(PP<PLearner> learner)
    { tester->learner = learner; learner_ = learner; }

    // simply calls inherited::build() then build_()
    virtual void build();

    //! sets the specified options to the specified values
    //! Will also detect if any option that is NOT listed in dont_restart_upon_change
    //! gets modified. If so, build() and forget() will be called on the learner
    void setLearnerOptions(const TVec<string>& option_names, const TVec<string>& option_vals);

    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    //! Currently train_stats receives a single update, with the returned vector
    //! of the last strategy command...
    virtual void train();

    virtual void forget();
    virtual void finalize();

    //! Returns the getResultNames() of its last strategy command
    TVec<string> getTrainCostNames() const;

    PLEARN_DECLARE_OBJECT(HyperLearner);

    void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void run();

    //! Save the current HyperLearner in its expdir
    void auto_save();

}; // class HyperLearner

DECLARE_OBJECT_PTR(HyperLearner);


} // end of namespace PLearn

#endif // HyperLearner_INC


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
