
// -*- C++ -*-

// HyperOptimize.h
//
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

/*! \file HyperOptimize.h */
#ifndef HyperOptimize_INC
#define HyperOptimize_INC

#include "HyperCommand.h"
#include <plearn/vmat/Splitter.h>
#include "OptionsOracle.h"

namespace PLearn {
using namespace std;

class HyperOptimize: public HyperCommand
{
protected:

    VMat resultsmat;

public:

    typedef HyperCommand inherited;
    PLEARN_DECLARE_OBJECT(HyperOptimize);


    // ************************
    // * public build options *
    // ************************

    int which_cost;
    int min_n_trials;
    PP<OptionsOracle> oracle;
    bool provide_tester_expdir;  // should tester be provided with an expdir for each test run
    TVec< PP<HyperCommand> > sub_strategy; //!< A possible sub-strategy to optimize other hyper parameters
    bool rerun_after_sub;
    bool provide_sub_expdir; // should sub_strategy be provided an expdir
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
    // (Please implement in .cc)
    void build_();

protected:
    //! Declares this class' options
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);

    void createResultsMat();
    void reportResult(int trialnum,  const Vec& results);
    Vec runTest(int trialnum);

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
