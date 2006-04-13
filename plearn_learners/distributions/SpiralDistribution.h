
// -*- C++ -*-

// SpiralDistribution.h
//
// Copyright (C) 2003  Pascal Vincent 
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

/*! \file SpiralDistribution.h */
#ifndef SpiralDistribution_INC
#define SpiralDistribution_INC

#include "UnconditionalDistribution.h"

namespace PLearn {
using namespace std;

class SpiralDistribution: public UnconditionalDistribution
{
public:

    typedef UnconditionalDistribution inherited;

    // ************************
    // * public build options *
    // ************************

    // see implementation of and declareOptions() methods in .cc for description
    // of these options
    real lambda;
    real alpha;
    real tmin;
    real tmax;
    real sigma;
    real uniformity;
    bool include_t;

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    SpiralDistribution();

    // ******************
    // * PDistribution methods *
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

    // ************************
    // **** Object methods ****
    // ************************

    //! simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods
    //  If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
    PLEARN_DECLARE_OBJECT(SpiralDistribution);

  
    // ** specific methods ** 
    //! computes (x,y) = ( lambda*sin(alpha*t), lambda*cos(alpha*t) )
    void curve(real t, real& x, real& y) const;

    // **************************
    // **** PDistribution methods ****
    // **************************

    //! return log of probability density log(p(x))
    virtual real log_density(const Vec& x) const;

    //! return survival fn = P(X>x)
    virtual real survival_fn(const Vec& x) const;

    //! return survival fn = P(X<x)
    virtual real cdf(const Vec& x) const;

    //! return E[X] 
    virtual void expectation(Vec& mu) const;

    //! return Var[X]
    virtual void variance(Mat& cov) const;

    //! return a pseudo-random sample generated from the distribution.
    virtual void generate(Vec& x) const;
  
    // **************************
    // **** Learner methods ****
    // **************************

    //! inputsize is 2 (or 3 if include_t is set to true)
    virtual int inputsize() const;

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(SpiralDistribution);
  
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
