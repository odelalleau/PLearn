// -*- C++ -*-

// MixtureDistribution.h
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

/*! \file MixtureDistribution.h */


#ifndef MixtureDistribution_INC
#define MixtureDistribution_INC

#include <plearn_learners/distributions/PDistribution.h>

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
class MixtureDistribution : public PDistribution
{
    typedef PDistribution inherited;

public:
    //#####  Public Build Options  ############################################

    TVec< PP<PDistribution> > distributions;
    Vec weights;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    MixtureDistribution();

    //#####  PDistribution Member Functions  ##################################

    //! Return log of probability density log(p(y | x)).
    virtual real log_density(const Vec& y) const;

    //! Return survival function: P(Y>y | x).
    virtual real survival_fn(const Vec& y) const;

    //! Return cdf: P(Y<y | x).
    virtual real cdf(const Vec& y) const;

    //! Return E[Y | x].
    virtual void expectation(Vec& mu) const;

    //! Return Var[Y | x].
    virtual void variance(Mat& cov) const;

    //! Return a pseudo-random sample generated from the conditional
    //! distribution, of density p(y | x).
    virtual void generate(Vec& y) const;

    //### Override this method if you need it (and if your distribution can
    //### handle it. Default version calls PLERROR.
    //! Generates a pseudo-random sample x from the reversed conditional
    //! distribution, of density p(x | y) (and NOT p(y | x)).
    //! i.e., generates a "predictor" part given a "predicted" part, regardless
    //! of any previously set predictor.
    // virtual void generatePredictorGivenPredicted(Vec& x, const Vec& y);

    //! Reset the random number generator used by generate() using the
    //! given seed.
    virtual void resetGenerator(long g_seed);

    //! Set the 'predictor' and 'predicted' sizes for this distribution.
    //### See help in PDistribution.h.
    virtual bool setPredictorPredictedSizes(int the_predictor_size,
                                            int the_predicted_size,
                                            bool call_parent = true);

    //! Set the value for the predictor part of a conditional probability.
    //### See help in PDistribution.h.
    virtual void setPredictor(const Vec& predictor, bool call_parent = true)
                              const;

    // ### These methods may be overridden for efficiency purpose:
    /*
    //### Default version calls exp(log_density(y))
    //! Return probability density p(y | x)
    virtual real density(const Vec& y) const;

    //### Default version calls setPredictorPredictedSises(0,-1) and generate
    //! Generates a pseudo-random sample (x,y) from the JOINT distribution,
    //! of density p(x, y)
    //! i.e., generates a predictor and a predicted part, regardless of any
    //! previously set predictor.
    virtual void generateJoint(Vec& xy);

    //### Default version calls generateJoint and discards y
    //! Generates a pseudo-random sample x from the marginal distribution of
    //! predictors, of density p(x),
    //! i.e., generates a predictor part, regardless of any previously set
    //! predictor.
    virtual void generatePredictor(Vec& x);

    //### Default version calls generateJoint and discards x
    //! Generates a pseudo-random sample y from the marginal distribution of
    //! predicted parts, of density p(y) (and NOT p(y | x)).
    //! i.e., generates a predicted part, regardless of any previously set
    //! predictor.
    virtual void generatePredicted(Vec& y);
    */


    //#####  PLearner Member Functions  #######################################

    // ### Default version of inputsize returns learner->inputsize()
    // ### If this is not appropriate, you should uncomment this and define
    // ### it properly in the .cc
    // virtual int inputsize() const;

    /**
     * (Re-)initializes the PDistribution in its fresh state (that state may
     * depend on the 'seed' option).  And sets 'stage' back to 0 (this is the
     * stage of a fresh learner!).
     * ### You may remove this method if your distribution does not
     * ### implement it.
     */
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
    PLEARN_DECLARE_OBJECT(MixtureDistribution);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    // ...

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Re-obtain the sizes of the predictor and predicted parts from the first
    //! distribution in the 'distributions' vector.
    //! This method is used to re-obtain sizes after things may have changed
    //! (e.g. after a build(), forget() or train()).
    void getSizes() const;

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    //! Vector to store temporary data.
    mutable Vec work;

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(MixtureDistribution);

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
