// -*- C++ -*-

// PDistribution.h
//
// Copyright (C) 2003 Pascal Vincent
// Copyright (C) 2004-2005 University of Montreal
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

/*! \file PDistribution.h */
#ifndef PDistribution_INC
#define PDistribution_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {
using namespace std;

/**
 *  Base class for PLearn probability distributions.
 *  
 *  PDistributions derive from PLearner, as some of them may be fitted to data by
 *  training, but they have additional methods allowing e.g. to compute density
 *  or generate data points.
 *  
 *  By default, a PDistribution may be conditional to a predictor part x, in
 *  order to represent the conditional distribution of P(Y | X = x). An
 *  unconditional distribution should derive from UnconditionalDistribution as it
 *  has a simpler interface.
 *  
 *  Since we want to be able to compute for instance P(Y = y | X = x), both the
 *  predictor part 'x' and the predicted part 'y' must be considered as input
 *  from the PLearner framework point of view. Thus one must specify the size of
 *  the predictor part by the 'predictor_size' option, and the size of the
 *  predicted by the 'predicted_size' option, satisfying the following equality:
 *  
 *      predictor_size + predicted_size == inputsize  (1)
 *  
 *  Optionally, 'predictor_size' or 'predicted_size' (but not both) may be set to
 *  -1, and the PDistribution will automatically guess the other size so that
 *  equation (1) is satisfied (actually, in order to preserve the user-provided
 *  values of 'predictor_size' and 'predicted_size', the guessed values are
 *  stored in the learnt options 'n_predictor' and 'n_predicted'). This way,
 *  unconditional distributions can be created by setting 'predictor_size' to 0
 *  and 'predicted_size' to -1.
 *  
 *  The default implementations of the learner-type methods for computing
 *  outputs and costs work as follows:
 *    - the 'outputs_def' option allows to choose what is in the output
 *      (e.g. log density, expectation, ...)
 *    - the cost is a vector of size 1 containing only the negative log-
 *      likelihood (NLL), i.e. -log(P(y|x)).
 *  
 *  For conditional distributions, the input must always be made of both the
 *  'predictor' part (x) and the 'predicted' part (y), even if the output may not
 *  need the predicted part (e.g. to compute E[Y | X = x]).  The exception is
 *  when computeOutput(..) needs to be called successively with the same value of
 *  'x': in this case, after a first call with both 'x' and 'y', one may only
 *  provide 'y' as input in later calls, and 'x' will be assumed to be
 *  unchanged. Or, alternatively, one can set the 'predictor_part' option first,
 *  either through the options system or using the setPredictor(..) method.
 *  
 *  IMPLEMENTATION NOTES:
 *    Note that many methods are declared as 'const' because of the 'const'
 *    plague, but are actually not true 'const' methods.
 *    This is also why a lot of stuff is mutable.
 *  TODO: Would it be possible to remove some 'const' stuff for cleaner code?
 */
class PDistribution: public PLearner
{

private:

    typedef PLearner inherited;

protected:
    //! Global storage to save memory allocations.
    mutable Vec store_expect, store_result;
    mutable Mat store_cov;

protected:

    //! The step when plotting the curve (upper case outputs_def).
    real delta_curve;

    mutable Vec predictor_part;     //!< Used to store the x part in p(y|x).
    mutable Vec predicted_part;     //!< Used to store the y part in p(y|x).

    // *********************
    // * protected options *
    // *********************

    //! User-provided sizes of the 'predictor' and 'predicted' sizes.
    //! Since they may be equal to '-1', subclasses should generally only use
    //! the learnt options 'n_predictor' and 'n_predicted' below.
    int predictor_size;
    int predicted_size;

    //! Learnt sizes of the 'predictor' and 'predicted' sizes. These are the
    //! options to use in PDistribution subclasses code. They always verify:
    //!     n_predictor + n_predicted == inputsize()
    int n_predictor;
    int n_predicted;

public:

    // ************************
    // * public build options *
    // ************************

    real lower_bound, upper_bound;
    int n_curve_points;
    string outputs_def; // TODO Replace this by a TVec<string>

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    PDistribution();

    // ********************
    // * PLearner methods *
    // ********************

private:

    //! This does the actual building.
    void build_();

protected:

    //! Declares this class' options.
    static void declareOptions(OptionList& ol);

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! Simply calls inherited::build() then build_().
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods
    PLEARN_DECLARE_OBJECT(PDistribution);

    // **************************
    // **** PLearner methods ****
    // **************************

    //! Returned value depends on outputs_def.
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages,
    //! updating the train_stats collector with training costs measured on-line
    //! in the process.
    virtual void train();

    //! Produce outputs according to what is specified in outputs_def.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes negative log likelihood (NLL). If the first output is neither
    //! the log density nor the density, an error will be raised.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    // *******************************
    // **** PDistribution methods ****
    // *******************************

protected:

    //! Split an input into the part corresponding to the predictor (in
    //! 'predictor_part'), and the predicted (in 'predicted_part').
    //! Also call setPredictor(..) with the new predictor part.
    //! If 'input' turns out to only have a predicted part (i.e. its length is
    //! equal to 'n_predicted'), then no predictor part will be set (it is
    //! assumed to stay the same as before).
    void splitCond(const Vec& input) const;

    //! Called in computeOutput when an unknown character is found.
    // TODO Can we find a better way to perform this?
    virtual void unknownOutput(char def, const Vec& input, Vec& output, int& k) const;

public:

    //! Set the 'predictor' and 'predicted' sizes for this distribution.
    //! 'the_predictor_size' is the size of the predictor, i.e. of x in p(y|x).
    //! 'the_predicted_size' is the size of the predicted, i.e. of y in p(y|x).
    //! This is a virtual method: if 'call_parent' is set to true, then the
    //! inherited::setPredictorPredictedSizes(..) method will also be called,
    //! with the same arguments: this is useful in the build process, where
    //! each class is able to call only its own method by setting 'call_parent'
    //! to false.
    //! If 'call_parent' is true, returns 'true' iff the predictor or predicted
    //! sizes have been modified from their previous value.
    //! If 'call_parent' is false, returns 'false'.
    virtual bool setPredictorPredictedSizes(int the_predictor_size,
                                            int the_predicted_size,
                                            bool call_parent = true);

    //! Set the value for the predictor part of a conditional probability.
    //! This needs to be implemented in subclasses if there is something
    //! special to do (like precomputing some data).
    //! The default behavior is just to fill 'predictor_part' with the first
    //! 'n_predictor' elements of 'predictor'.
    //! As with 'setPredictorPredictedSizes(..)', the boolean 'call_parent'
    //! indicates whether or not one should call inherited::setPredictor(..)
    //! with the same arguments.
    virtual void setPredictor(const Vec& predictor, bool call_parent = true)
                              const;

    //! Return [ "NLL" ] (the only cost computed by a PDistribution).
    virtual TVec<string> getTestCostNames() const;

    //! Return [ ].
    virtual TVec<string> getTrainCostNames() const;

    //! Return log of probability density log(p(y | x)).
    virtual real log_density(const Vec& y) const;

    //! Return probability density p(y | x) (default version simply returns
    //! exp(log_density(y))).
    virtual real density(const Vec& y) const;

    //! Return survival function: P(Y>y | x).
    virtual real survival_fn(const Vec& y) const;

    //! Return cdf: P(Y<y | x).
    virtual real cdf(const Vec& y) const;

    //! Return E[Y | x].
    virtual void expectation(Vec& mu) const;

    //! Return E[Y | x] where Y is the missing part in the 'input' vector, and
    //! x in the observed part (thus discarding any current setting of
    //! predictor and predicted sizes).
    //! The values in return vector 'mu' are ordered like the missing values
    //! in 'input'.
    //! This method must be implemented in sub-classes, as the default behavior
    //! is to throw an error.
    virtual void missingExpectation(const Vec& input, Vec& mu);

    //! Return Var[Y | x].
    virtual void variance(Mat& cov) const;

    //! Reset the random number generator used by generate() using the given
    //! seed.
    //! Default behavior is to call random_gen->manual_seed(g_seed) and to save
    //! the given seed.
    //! This method is called in build().
    //! Exception: if 'g_seed' is zero, then do nothing.
    virtual void resetGenerator(long g_seed);

    //! Return a pseudo-random sample generated from the conditional
    //! distribution, of density p(y | x).
    virtual void generate(Vec& y) const;

    //! X must be a N x n_predicted matrix. that will be filled.
    //! This will call generate N times to fill the N rows of the matrix.
    void generateN(const Mat& Y) const;

    //! Generates a pseudo-random sample (x,y) from the JOINT distribution,
    //! of density p(x, y)
    //! i.e., generates a predictor and a predicted part, regardless of any
    //! previously set predictor.
    virtual void generateJoint(Vec& xy);

    //! Generates a pseudo-random sample (x,y) from the JOINT distribution,
    //! of density p(x, y), split in two vectors
    //! i.e., generates a predictor and a predicted part, regardless of any
    //! previously set predictor.
    void generateJoint(Vec& x, Vec& y);

    //! Generates a pseudo-random sample x from the marginal distribution of
    //! predictors, of density p(x),
    //! i.e., generates a predictor part, regardless of any previously set
    //! predictor.
    virtual void generatePredictor(Vec& x);

    //! Generates a pseudo-random sample y from the marginal distribution of
    //! predicted parts, of density p(y) (and NOT p(y | x)).
    //! i.e., generates a predicted part, regardless of any previously set
    //! predictor.
    virtual void generatePredicted(Vec& y);

    //! Generates a pseudo-random sample x from the reversed conditional
    //! distribution, of density p(x | y) (and NOT p(y | x)).
    //! i.e., generates a "predictor" part given a "predicted" part, regardless
    //! of any previously set predictor.
    virtual void generatePredictorGivenPredicted(Vec& x, const Vec& y);

    //! 'Get' accessor for n_predicted.
    int getNPredicted() { return n_predicted; }

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PDistribution);

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
