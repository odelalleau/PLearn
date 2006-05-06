// -*- C++ -*-

// PTester.h
// 
// Copyright (C) 2002 Pascal Vincent, Frederic Morin
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

/* *******************************************************      
 * $Id$ 
 ******************************************************* */

/*! \file PTester.h */
#ifndef PTester_INC
#define PTester_INC

#include <plearn/base/Object.h>
#include <plearn/vmat/Splitter.h>
#include <plearn/vmat/VMat.h>
#include <plearn_learners/generic/PLearner.h>

namespace PLearn {
using namespace std;

class PTester: public Object
{    

private:

    typedef Object inherited;

    //! The original statnames option.
    //! It is private because it is safer to access stats from getStatNames,
    //! since the 'statmask' option may modify the stats.
    TVec<string> statnames;

protected:

    //! The 'real' statnames: these are obtained from 'statnames' by a
    //! processing at build time, taking into account the 'statmask' option.
    TVec<string> statnames_processed;

public:

    // ************************
    // * public build options *
    // ************************
  
    // See declareOptions method in .cc for the role of these options.

    //! Path of this tester's experiment directory in which to save all tester results (will be created if it does not already exist)
    VMat dataset;
    PPath expdir;  
    TVec<string> final_commands;
    PP<VecStatsCollector> global_template_stats_collector;
    PP<PLearner> learner;
    bool provide_learner_expdir;
    bool report_stats;
    bool save_data_sets;
    bool save_initial_learners;
    bool save_initial_tester;
    bool save_learners;
    bool save_stat_collectors;
    bool save_test_costs;
    bool save_test_outputs;
    bool call_forget_in_run;
    PP<Splitter> splitter;
    TVec<TVec<string> > statmask;
    PP<VecStatsCollector> template_stats_collector;

    //! Whether to save 95% confidence intervals for the test outputs;
    //! make sense mostly if 'save_test_outputs' is also true.  The
    //! intervals are saved in a file SETNAME_confidence.pmat (default=false)
    bool save_test_confidence;
  
    //! whether or not to train or just test
    bool train; 

    // ****************
    // * Constructors *
    // ****************

    // Default constructor
    PTester();


    // ******************
    // * Object methods *
    // ******************

private: 
    //! This does the actual building. 
    // (Please implement in .cc)
    void build_();

protected: 
    //! Declares this class' options
    static void declareOptions(OptionList& ol);

    //! Declare the methods that are remote-callable
    static void declareMethods(RemoteMethodMap& rmm);

    //! Utility function to compute confidence intervals over a test set
    //! and save results in VMat
    void computeConfidence(VMat test_set, VMat confidence);

public:
    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(PTester);

    /**
     *  The experiment directory is the directory in which files related to
     *  this model are to be saved.  If it is an empty string, it is understood
     *  to mean that the user doesn't want any file created by this learner.
     */
    void setExperimentDirectory(const PPath& the_expdir);
  
    //! This returns the currently set expdir (see setExperimentDirectory)
    PPath getExperimentDirectory() const { return expdir; }


    //! Return the statnames (potentially modified by statmask, if provided).
    TVec<string> getStatNames();

    //! runs the tester
    virtual void run();

    /**
     *  Performs the test, and returns the global stats specified in statnames.
     *  If 'call_forget' is set to false then the call to setTrainingSet()
     *  won't call forget and build.  This is useful for continuation of an
     *  incremental training (such as after increasing the number of epochs
     *  (nstages) ), or generally when trying different option values that
     *  don't require the learning to be restarted from scratch.  However
     *  call_forget will be forced to true (even if passed as false) if the
     *  splitter returns more than one split.
     *
     *  Returns a vector of test statistics corresponding to the requested
     *  statnames
     */
    Vec perform(bool call_forget=true);

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

DECLARE_OBJECT_PTR(PTester);


//! The specification of a statistic to compute (as can be specified as a string in PTester)

class StatSpec
{
public:
    string extstat;  //! "external" stat, to be computed over splits
    string setname;  //! "train" or "test1" or "test2" ...
    int setnum;      //! data set on which to compute stat: 0 :train, 1: test1, ...

    //! "internal" stat to be computed over examples the given a train or test set of a split. Ex.: "E[costname]"
    //! This string will at some point be used to call the VecStatsCollector's getStat(...) method.
    string intstatname;  

    StatSpec() : setnum(-1) {}

    void init(const string& statname);

    string statName()
    { return extstat + "[" + setname + "." + intstatname + "]"; }

private:

    //! will determine extstat, intstat, setnum and costname from statname 
    void parseStatname(const string& statname);
};

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
