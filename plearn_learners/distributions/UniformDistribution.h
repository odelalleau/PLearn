// -*- C++ -*-

// UniformDistribution.h
//
// Copyright (C) 2004 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file UniformDistribution.h */


#ifndef UniformDistribution_INC
#define UniformDistribution_INC

#include "UnconditionalDistribution.h"

namespace PLearn {
using namespace std;

class UniformDistribution: public UnconditionalDistribution
{

private:

    typedef UnconditionalDistribution inherited;

protected:

    // *********************
    // * protected options *
    // *********************

    mutable int counter;

    // Fields below are not options.

public:

    // ************************
    // * public build options *
    // ************************

    Vec max;
    Vec min;
    int mesh_size;
    int n_dim;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    UniformDistribution();

    // *************************
    // * PDistribution methods *
    // *************************

private:

    //! This does the actual building.
    void build_();

protected:

    //! Declare this class' options.
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
    PLEARN_DECLARE_OBJECT(UniformDistribution);

    // *******************************
    // **** PDistribution methods ****
    // *******************************

    //! Return log of probability density log(p(x)).
    virtual real log_density(const Vec& x) const;

    //! Return survival fn = P(X>x).
    virtual real survival_fn(const Vec& x) const;

    //! Return survival fn = P(X<x).
    virtual real cdf(const Vec& x) const;

    //! Return E[X].
    virtual void expectation(Vec& mu) const;

    //! Return Var[X].
    virtual void variance(Mat& cov) const;

    //! Reset the random number generator used by generate() using the given seed.
    virtual void resetGenerator(long g_seed);

    //! Return a pseudo-random sample generated from the distribution.
    virtual void generate(Vec& x) const;

};

// Declare a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(UniformDistribution);

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
