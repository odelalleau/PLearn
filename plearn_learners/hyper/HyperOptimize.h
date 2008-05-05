
// -*- C++ -*-

// HyperOptimize.h
//
// Copyright (C) 2003-2006 ApSTAT Technologies Inc.
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
// Documentation: Nicolas Chapados

/* *******************************************************
 * $Id$
 ******************************************************* */

/*! \file HyperOptimize.h */
#ifndef HyperOptimize_INC
#define HyperOptimize_INC

#include "HyperCommand.h"
#include "OptionsOracle.h"
#include <plearn/misc/PTimer.h>
#include <plearn/vmat/Splitter.h>

namespace PLearn {
using namespace std;
/**
 *  Carry out an hyper-parameter optimization according to an Oracle
 *
 *  HyperOptimize is part of a sequence of HyperCommands (specified within an
 *  HyperLearner) to optimize a validation cost over settings of
 *  hyper-parameters provided by an Oracle.  [NOTE: The "underlying learner" is
 *  the PLearner object (specified within the enclosing HyperLearner) whose
 *  hyper-parameters we are trying to optimize.]
 *
 *  The sequence of steps followed by HyperOptimize is as follows:
 *
 *  - 1) Gather a "trial" from an Oracle.  A "trial" is a full setting of
 *    hyperparameters (option name/value pairs) that the underlying learner
 *    should be trained with.
 *
 *  - 2) Set the options within the underlying learner that correspond to
 *    the current trial.
 *
 *  - 3) Train and test the underlying learner.  The tester used for this
 *    purpose is a PTester specified in the enclosing HyperLearner.  By
 *    default, we rely on that PTester's Splitter as well; however, an
 *    overriding Splitter may be specified within the HyperCommand.
 *
 *  - 4) After training/testing, measure the cost to optimize, given by the
 *    'which_cost' option.  This specifies an index into the test statistics
 *    given by the 'statnames' option in PTester.  The measured cost gives
 *    the performance of the current trial, i.e. how well does perform the
 *    current setting of hyper-parameters.
 *
 *  - 5) Repeat steps 1-4 until the Oracle tells us "no more trials".
 *
 *  - 6) Find the best setting of hyper-parameters among all those tried.
 *    ("best" defined as that which minimises the cost measured in Step 4).
 *
 *  - 7) Set the underlying learner within the enclosing HyperLearner to be
 *    the BEST ONE found in Step 6.
 *
 *  Optionally, instead of a plain Train/Test in Step 3, a SUB-STRATEGY may be
 *  invoked.  This can be viewed as a "sub-routine" for hyperoptimization and
 *  can be used to implement a form of conditioning: given the current setting
 *  for hyper-parameters X,Y,Z, find the best setting of hyper-parameters
 *  T,U,V.  The most common example is for doing early-stopping when training a
 *  neural network: a first-level HyperOptimize command can use an
 *  ExplicitListOracle to jointly optimize over weight-decays and the number of
 *  hidden units.  A sub-strategy can then be used with an EarlyStoppingOracle
 *  to find the optimal number of training stages (epochs) for each combination
 *  of weight-decay/hidden units.
 *
 *  Note that after optimization, the matrix of all trials is available through
 *  the option 'resultsmat' (which is declared as nosave).  This is available
 *  even if no expdir has been declared.
 */
class HyperOptimize: public HyperCommand
{
    typedef HyperCommand inherited;

protected:
    //! Store the results computed for each trial
    VMat resultsmat;
    real best_objective;
    Vec best_results;
    PP<PLearner> best_learner;
    int trialnum;
    TVec<string> option_vals;
    PP<PTimer> auto_save_timer;

public:

    PLEARN_DECLARE_OBJECT(HyperOptimize);

    // ************************
    // * public build options *
    // ************************

    int which_cost_pos;
    string which_cost;
    int min_n_trials;
    PP<OptionsOracle> oracle;
    bool provide_tester_expdir;  // should tester be provided with an expdir for each test run
    TVec< PP<HyperCommand> > sub_strategy; //!< A possible sub-strategy to optimize other hyper parameters
    bool rerun_after_sub;
    bool provide_sub_expdir; // should sub_strategy be provided an expdir
    bool save_best_learner;
    int auto_save;
    int auto_save_test;
    int auto_save_diff_time;
    PP<Splitter> splitter;  // (if not specified, use default splitter specified in PTester)

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    HyperOptimize();


    // ******************
    // * HyperCommand methods *
    // ******************

private:
    //! This does the actual building.
    void build_();

protected:
    //! Declares this class' options
    static void declareOptions(OptionList& ol);

    void getResultsMat();
    void reportResult(int trialnum,  const Vec& results);
    Vec runTest(int trialnum);

/*    
    //! for parallel optimize
    void launchTest(int trialnum, PP<RemotePLearnServer> server,
                    map<PP<RemotePLearnServer>, int>& testers_ids);
*/

public:

    //! Sets the expdir and calls createResultsMat.
    virtual void setExperimentDirectory(const PPath& the_expdir);

    //! Returns the names of the results returned by the optimize() method.
    virtual TVec<string> getResultNames() const;

    virtual Vec optimize();

    // simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(HyperOptimize);

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
