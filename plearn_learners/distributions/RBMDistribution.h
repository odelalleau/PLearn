// -*- C++ -*-

// RBMDistribution.h
//
// Copyright (C) 2008 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file RBMDistribution.h */


#ifndef RBMDistribution_INC
#define RBMDistribution_INC

#include <plearn_learners/distributions/UnconditionalDistribution.h>
#include <plearn_learners/online/ModuleLearner.h>
#include <plearn_learners/online/RBMModule.h>

namespace PLearn {

/**
 * The first sentence should be a BRIEF DESCRIPTION of what the class does.
 * Place the rest of the class programmer documentation here.  Doxygen supports
 * Javadoc-style comments.  See http://www.doxygen.org/manual.html
 *
 * @todo Write class to-do's here if there are any.
 *
 * @deprecated Write deprecated stuff here if there is any.  Indicate what else
 * should be used instead.
 */
class RBMDistribution : public UnconditionalDistribution
{
    typedef UnconditionalDistribution inherited;

public:
    //#####  Public Build Options  ############################################

    PP<RBMModule> rbm;

    real n_gibbs_chains;
    VMat sample_data;
    bool unnormalized_density;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMDistribution();


    //#####  UnconditionalDistribution Member Functions  ######################

    //! Return log of probability density log(p(y)).
    virtual real log_density(const Vec& x) const;

    //! Return survival function: P(Y>y).
    virtual real survival_fn(const Vec& y) const;

    //! Return cdf: P(Y<y).
    virtual real cdf(const Vec& y) const;

    //! Return E[Y].
    virtual void expectation(Vec& mu) const;

    //! Return Var[Y].
    virtual void variance(Mat& cov) const;

    //! Return a pseudo-random sample generated from the distribution.
    virtual void generate(Vec& y) const;

    //! Overridden for efficiency purpose.
    void generateN(const Mat& Y) const;

    //! Reset the random number generator used by generate() using the
    //! given seed.
    virtual void resetGenerator(long g_seed);

    // ### These methods may be overridden for efficiency purpose:
    /*
    //! Return probability density p(y)
    virtual real density(const Vec& y) const;
    */


    //#####  PLearner Member Functions  #######################################

    // ### Default version of inputsize returns learner->inputsize()
    // ### If this is not appropriate, you should uncomment this and define
    // ### it properly in the .cc
    // virtual int inputsize() const;

    //! (Re-)initializes the PDistribution in its fresh state (that state may
    //! depend on the 'seed' option) and sets 'stage' back to 0 (this is the
    //! stage of a fresh learner!).
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage == nstages, updating the train_stats collector with training
    //! costs measured on-line in the process.
    // ### You may remove this method if your distribution does not
    // ### implement it.
    virtual void train();


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(RBMDistribution);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    
    //! Associated learner to train the RBM.
    PP<ModuleLearner> learner;

    //! Vector of data passed to the fprop(..) method of the RBM.
    TVec<Mat*> ports_val;

    //! Temporary storage.
    mutable Mat work1, work2, work3;

    //! Temporary storage.
    mutable Vec workv1;

    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    // ...

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    // (PLEASE IMPLEMENT IN .cc)
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(RBMDistribution);

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
