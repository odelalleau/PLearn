// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// PLearn (A C++ Machine Learning Library)
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
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearnLibrary/PLearnAlgo/GaussianDistribution.h */

#ifndef GaussianDistribution_INC
#define GaussianDistribution_INC

#include "PDistribution.h"

namespace PLearn {
using namespace std;

// This is a density estimation learner.
// It uses a compact representation of a Gaussian, by keeping only the k
// top eigenvalues and associated eigenvectors of the covariance matrix.
// All other eigenvalues are kept at the level of the k+1 th eigenvalue
// Optionally, a constant sigma is first added to the diagonal of the covariance matrix.

class GaussianDistribution: public PDistribution
{
    typedef PDistribution inherited;

public:

    // Possibly "Learned" parameters
    Vec mu;
    Vec eigenvalues;
    Mat eigenvectors;

    // Build options
    int k; // maximum number of eigenvectors to keep
    real gamma;
    real min_eig;
    bool use_last_eig;
    float ignore_weights_below; //!< When doing a weighted fitting (weightsize==1), points with a weight below this value will be ignored

public:
    GaussianDistribution();

    PLEARN_DECLARE_OBJECT(GaussianDistribution);
    void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void forget();
    virtual void train();
    virtual real log_density(const Vec& x) const;

    //! return a pseudo-random sample generated from the distribution.
    virtual void generate(Vec& x) const;

    //! Overridden so that it does not necessarily need a training set.
    virtual int inputsize() const;

    //! Public build.
    virtual void build();

protected:
    static void declareOptions(OptionList& ol);

private:

    //! Internal build.
    void build_();

};

DECLARE_OBJECT_PTR(GaussianDistribution);

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
