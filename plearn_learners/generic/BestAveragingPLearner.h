// -*- C++ -*-

// BestAveragingPLearner.h
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

/*! \file BestAveragingPLearner.h */


#ifndef BestAveragingPLearner_INC
#define BestAveragingPLearner_INC

#include <plearn/vmat/Splitter.h>
#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

/**
 *  Select the M "best" of N trained PLearners based on a train cost
 *
 *  This PLearner takes N raw models (themselved PLearners) and trains them all
 *  on the same data (or various splits given by an optional Splitter), then
 *  selects the M "best" models based on a train cost.  At compute-output time,
 *  it outputs the arithmetic mean of the outputs of the selected models (which
 *  works fine for regression).
 *
 *  The train costs of this learner are simply the concatenation of the train
 *  costs of all sublearners.  We also add the following costs: the cost
 *  'selected_i', where 0 <= i < M, contains the index of the selected model
 *  (between 0 and N-1).
 *
 *  The test costs of this learner is, for now, just the mse.
 */
class BestAveragingPLearner : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    /**
     *  The set of all learners to train, given in extension.  If this option
     *  is specified, the learner template (see below) is ignored.  Note that
     *  these objects ARE NOT deep-copied before being trained.
     */
    TVec< PP<PLearner> > m_learner_set;

    /**
     *  If 'learner_set' is not specified, a template PLearner used to
     *  instantiate 'learner_set'.  When instantiation is being carried out,
     *  the seed is set sequentially from 'initial_seed'.  The instantiation
     *  sequence is as follows:
     *
     *  - (1) template is deep-copied
     *  - (2) seed is set
     *  - (3) build() is called
     *  - (4) forget() is called
     *
     *  The expdir is set from the BestAveragingPLearner's expdir (if any)
     *  by suffixing '/learner_i'.
     */
    PP<PLearner> m_learner_template;

    /**
     *  If learners are instantiated from 'learner_template', the initial seed
     *  value to set into the learners before building them.  The seed is
     *  incremented by one from that starting point for each successive learner
     *  that is being instantiated.  If this value is <= 0, it is used as-is
     *  without being incremented.
     */
    long m_initial_seed;

    /**
     *  Total number of learners to instantiate from learner_template (if
     *  'learner_set' is not specified.
     */
    int m_total_learner_num;

    /**
     *  Number of BEST train-time learners to keep and average at
     *  compute-output time.
     */
    int m_best_learner_num;

    /**
     *  Statistic specification to use to compare the training performance
     *  between learners.  For example, if all learners have a 'mse' measure,
     *  this would be "E[mse]".  It is assumed that all learners make available
     *  the statistic under the same name.
     */
    string m_comparison_statspec;

    /**
     *  Optional splitter that can be used to create the individual training
     *  sets for the learners.  If this is specified, it is assumed that the
     *  splitter returns a number of splits equal to the number of learners.
     *  Each split is used as a learner's training set.  If not specified,
     *  all learners receive the same training set (passed to setTrainingSet)
     */
    PP<Splitter> m_splitter;
    
public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    BestAveragingPLearner();


    //#####  PLearner Member Functions  #######################################

    //! Forwarded to inner learners.
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    //! New train-stats-collector are instantiated for sublearners
    virtual void setTrainStatsCollector(PP<VecStatsCollector> statscol);

    //! Forwarded to inner learners (suffixed with '/learner_i')
    virtual void setExperimentDirectory(const PPath& the_expdir);

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training costs
    //! measured on-line in the process.
    virtual void train();

    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutputs (and
    //! thus the test method).
    virtual TVec<std::string> getTestCostNames() const;

    //! The train costs are simply the concatenation of all sublearner's
    //! train costs.
    virtual TVec<std::string> getTrainCostNames() const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(BestAveragingPLearner);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Learned Options  #################################################

    /// Cached outputsize, determined from the inner learners
    mutable int m_cached_outputsize;

    /// List of train costs values for each learner in 'learner_set'
    Vec m_learner_train_costs;
    
    /// Learners that have been found to be the best and are being kept
    TVec< PP<PLearner> > m_best_learners;

    /// Buffer for compute output of inner model
    mutable Vec m_output_buffer;
    
protected:
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //! This does the actual building.
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(BestAveragingPLearner);

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
