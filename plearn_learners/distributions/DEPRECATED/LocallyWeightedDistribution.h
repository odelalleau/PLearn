// -*- C++ -*-

// LocallyWeightedDistribution.h
// 
// Copyright (C) 2002 Pascal Vincent
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

/*! \file PLearn/plearn_learners/distributions/DEPRECATED/LocallyWeightedDistribution.h */
#ifndef LocallyWeightedDistribution_INC
#define LocallyWeightedDistribution_INC

#include "Distribution.h"

namespace PLearn {
using namespace std;

#ifdef __INTEL_COMPILER
#pragma warning(disable:654)  // Get rid of compiler warning.
#endif

class LocallyWeightedDistribution: public Distribution
{

private:

    typedef Distribution inherited;

    //! Global storage to save memory allocations.
    mutable Vec trainsample, weights;

public:


    // ************************
    // * public build options *
    // ************************

    //! The kernel that will be used to locally weigh the samples
    Ker weighting_kernel;

    //! The distribution that will be trained with local weights
    PP<Distribution> localdistr; 

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    LocallyWeightedDistribution();


    // ******************
    // * Object methods *
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
    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(LocallyWeightedDistribution);

    // *******************
    // * Distribution methods *
    // *******************

    //! trains the model
    virtual void train(VMat training_set); 

    //! return log of probability density log(p(x))
    virtual double log_density(const Vec& x) const;
   
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(LocallyWeightedDistribution);
  
} // end of namespace PLearn

#ifdef __INTEL_COMPILER
#pragma warning(default:654)
#endif

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
