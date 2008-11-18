// -*- C++ -*-

// LinearRegressor.h
//
// Copyright (C) 2003  Yoshua Bengio 
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

/*! \file LinearRegressor.h */
#ifndef LinearRegressor_INC
#define LinearRegressor_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {
using namespace std;

class LinearRegressor: public PLearner
{
    typedef PLearner inherited;

    // Global storage used to save memory allocations.
    mutable Vec extendedinput; //!<  length 1+inputsize(), first element is 1.0 (used by the use method)
    mutable Vec input; //!<  extendedinput.subVec(1,inputsize())
    Vec train_costs;

protected:
    Mat XtX; //!<  can be re-used if train is called several times on the same data set
    Mat XtY; //!<  can be re-used if train is called several times on the same data set
    real sum_squared_y; //!< can be re-used if train is called several times on the same data set
    real sum_gammas; //!< sum of weights if weighted error, also for re-using training set with different weight decays

    //! Sum of squares of weights
    real weights_norm; 

    // *********************
    // * protected options *
    // *********************

    // ### declare protected option fields (such as learnt parameters) here

    //! The weight matrix computed by the regressor,
    //! (inputsize+1) x (outputsize)
    Mat weights;

    //! The Akaike Information Criterion computed at training time;
    //! Saved as a learned option to allow outputting AIC as a test cost.
    real AIC;

    //! The Bayesian Information Criterion computed at training time
    //! Saved as a learned option to allow outputting BIC as a test cost.
    real BIC;
  
    //! Estimate of the residual variance for each output variable.
    //! Saved as a learned option to allow outputting confidence intervals
    //! when model is reloaded and used in test mode.
    Vec resid_variance;
  
public:

    // ************************
    // * public build options *
    // ************************

    //! Whether to include a bias term in the regression (true by default)
    bool include_bias;
  
    //! Whether to use Cholesky decomposition for computing the solution
    //! (true by default)
    bool cholesky;

    //! Factor on the squared norm of parameters penalty (zero by default)
    real weight_decay;

    //! If true, the result of computeOutput*() functions is not the
    //! result of thre regression, but the learned regression parameters.
    //! (i.e. the matrix 'weights').  The matrix is flattened by rows.
    //! NOTE by Nicolas Chapados: this option is a bit of a hack and might
    //! be removed in the future.  Let me know if you come to rely on it.
    bool output_learned_weights;

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    LinearRegressor();


    // ******************
    // * PLearner methods *
    // ******************

private: 
    //! This does the actual building. 
    void build_();
    void resetAccumulators();

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
    PLEARN_DECLARE_OBJECT(LinearRegressor);


    // **************************
    // **** PLearner methods ****
    // **************************

    //! returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options)
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    //! This resizes the XtX and XtY matrices to signify that their content needs to be recomputed.
    //! When train is called repeatedly without an intervening call to forget(), it is assumed
    //! that the training set has not changed, and XtX and XtY are not recomputed, avoiding
    //! a possibly lengthy pass through the data. This is particarly useful when doing hyper-parameter
    //! optimization of the weight_decay.
    virtual void forget();
    
    //! The role of the train method is to bring the learner up to stage==nstages,
    //! updating the train_stats collector with training costs measured on-line in the process.
    //! See the forget() comment.
    virtual void train();

    //! Computes the output from the input
    //! output = weights * (1, input)
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output. 
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;

    //! Compute confidence intervals based on the NORMAL distribution
    virtual
    bool computeConfidenceFromOutput(const Vec& input, const Vec& output,
                                     real probability,
                                     TVec< pair<real,real> >& intervals) const;
  
    //! Returns the names of the costs computed by computeCostsFromOutpus (and thus the test method)
    virtual TVec<string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes and 
    //! for which it updates the VecStatsCollector train_stats
    virtual TVec<string> getTrainCostNames() const;

protected: 
    //! Declares this class' options
    static void declareOptions(OptionList& ol);

    //! Utility function to compute the AIC, BIC criteria from the
    //! squared error of the trained  model.  Store the result in
    //! AIC and BIC members of the object.
    void computeInformationCriteria(real squared_error, int n);

    //! Utility function to compute the variance of the residuals of the
    //! regression
    void computeResidualsVariance(const Vec& outputwise_sum_squared_Y);

    //! Inputsize that takes into account optional bias term
    int effective_inputsize() const
    { return inputsize() + (include_bias? 1 : 0); }
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(LinearRegressor);
  
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
