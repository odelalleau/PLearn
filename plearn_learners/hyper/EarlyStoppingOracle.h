
// -*- C++ -*-

// EarlyStoppingOracle.h
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

/*! \file EarlyStoppingOracle.h */
#ifndef EarlyStoppingOracle_INC
#define EarlyStoppingOracle_INC

#include "OptionsOracle.h"

namespace PLearn {
using namespace std;

class EarlyStoppingOracle: public OptionsOracle
{
protected:
    // *********************
    // * protected options *
    // *********************

    //! if the values option was specified, then this points to it
    //! if the range option was specified, this is an extensive list of the values
    //! implied by the range (this field is built by the build_ method).
    TVec<string> option_values;

    //! number of trials returned so far
    //! (This is also the index of the next trial to be returned from options_values)
    int nreturned;

    // these are used by checkEarlyStoppingCondition
    real previous_objective; // objective value reached at previous step
    real best_objective; // the best objective value reached so far
    int best_step;       // the "step" for which best result was obtained (i.e. the value of nreturned when we got the best)

    bool met_early_stopping; // will be set to true as soon as we meet the early stopping conditions

public:

    typedef OptionsOracle inherited;

    // ************************
    // * public build options *
    // ************************

    string option; //!< the name of the option to change
    TVec<string> values;   //!< a list of values to try in sequence
    Vec range;  //!< a numerical range of the form [ start, end ] or [ start, end, step ]

    //!  early-stopping parameters
    real min_value; //!<  minimum error beyond which we stop
    real max_value; //!<  maximum error beyond which we stop
    real max_degradation; //!<  maximum allowed degradation in error from last best value
    real relative_max_degradation;
    real min_improvement; //!<  required minimum improvement from last step
    real relative_min_improvement;
    int max_degraded_steps; //!< max. nb of steps beyond best found
    int min_n_steps; //!< min. nb of steps before early stopping

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    EarlyStoppingOracle();


    // ******************
    // * OptionsOracle methods *
    // ******************

private:
    //! This does the actual building.
    // (Please implement in .cc)
    void build_();

protected:
    //! Declares this class' options
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);

public:

    //! returns the set of names of options this generator generates
    virtual TVec<string>  getOptionNames() const;

    virtual TVec<string> generateNextTrial(const TVec<string>& older_trial, real obtained_objective);

    virtual void forget();

    // simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(EarlyStoppingOracle);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(EarlyStoppingOracle);

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
