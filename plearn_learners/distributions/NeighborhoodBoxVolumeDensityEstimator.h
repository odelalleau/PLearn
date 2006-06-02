// -*- C++ -*-

// NeighborhoodBoxVolumeDensityEstimator.h
//
// Copyright (C) 2006 Pascal Vincent
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
 * $Id: NeighborhoodBoxVolumeDensityEstimator.h 5388 2006-04-13 21:28:14Z lamblin $
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file NeighborhoodBoxVolumeDensityEstimator.h */


#ifndef NeighborhoodBoxVolumeDensityEstimator_INC
#define NeighborhoodBoxVolumeDensityEstimator_INC

#include <plearn_learners/distributions/UnconditionalDistribution.h>
#include <plearn/ker/Kernel.h>
#include <plearn_learners/nearest_neighbors/GenericNearestNeighbors.h>

namespace PLearn {

class NeighborhoodBoxVolumeDensityEstimator: public UnconditionalDistribution
{

private:

    typedef UnconditionalDistribution inherited;

    //! Global storage to save memory allocations.
    mutable Vec trainsample;

protected:

    // *********************
    // * protected options *
    // *********************

    PP<GenericNearestNeighbors> NN;
    mutable Vec NN_outputs;


public:

    // ************************
    // * public build options *
    // ************************

    int nneighbors;
    real min_radius;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    NeighborhoodBoxVolumeDensityEstimator();

    // *************************
    // * PDistribution methods *
    // *************************

private:

    //! This does the actual building.
    // ### Please implement in .cc.
    void build_();

protected:

    //! Returns true if input is contained in the box specified by center and radius
    bool box_contains(const Vec& center, const Vec& radius, const Vec& input) const;


    //! Declare this class' options.
    // ### Please implement in .cc.
    static void declareOptions(OptionList& ol);

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! Simply call inherited::build() then build_().
    virtual void build();

    //! Transform a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declare other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS.
    PLEARN_DECLARE_OBJECT(NeighborhoodBoxVolumeDensityEstimator);

    // *******************************
    // **** PDistribution methods ****
    // *******************************

    //! Return log of probability density log(p(y | x)).
    virtual real log_density(const Vec& x) const;

    // **************************
    // **** PLearner methods ****
    // **************************

    //! The role of the train method is to bring the learner up to stage == nstages,
    //! updating the train_stats collector with training costs measured on-line in the process.
    // ### You may remove this method if your distribution does not implement it.
    virtual void train();

    virtual void forget();

};

// Declare a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(NeighborhoodBoxVolumeDensityEstimator);

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
