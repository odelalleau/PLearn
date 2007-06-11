// -*- C++ -*-

// CubicSpline.h
//
// Copyright (C) 2007 Christian Dorion
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

// Authors: Christian Dorion
//   Migrated from Johann Hibschman's <johann@physics.berkeley.edu> Python
//   'spline' module.


/*! \file CubicSpline.h */


#ifndef CubicSpline_INC
#define CubicSpline_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

/**
 * Unidimensional cubic spline learner.
 *
 * This learner fits a unidimensional cubic spline to a given set of
 * points. That is, inputsize() must be one and the inputs are considered to be
 * the x values, while targetsize() must also be one and the targets are
 * considered to be the y values. The spline is fitted to the (x,y)-pairs so
 * formed. X values don't need to be ordered; the ordering is ensured within
 * the train method.
 */
class CubicSpline : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! The slope to enforce at the leftmost node -- Default: NaN [None]
    bool m_low_slope;

    //! The slope to enforce at the rightmost node -- Default: NaN [None]
    bool m_high_slope;
    

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    CubicSpline();


    //#####  PLearner Member Functions  #######################################

    //! Returns the size of this learner's output: 1 -> a single interpolated value
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    virtual void forget();

    //! Fit the splines to the *last* input point
    virtual void train();

    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method). [Empty]
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and for which it updates the VecStatsCollector train_stats. [Empty]
    virtual TVec<std::string> getTrainCostNames() const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(CubicSpline);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    //! A buffer containing the last training set inputs
    Vec m_inputs;

    //! A buffer containing the last training set targets
    Vec m_targets;    
    
    //! The learnt coefficients
    Vec m_coefficients;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // ...
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(CubicSpline);

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
