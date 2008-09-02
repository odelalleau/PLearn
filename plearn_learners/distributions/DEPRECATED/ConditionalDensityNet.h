// -*- C++ -*-

// ConditionalDensityNet.h
//
// Copyright (C) 2004 Université de Montréal
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

// Authors: Yoshua Bengio

/*! \file PLearn/plearn_learners/distributions/DEPRECATED/ConditionalDensityNet.h */


#ifndef ConditionalDensityNet_INC
#define ConditionalDensityNet_INC

#include "PDistribution.h"
#include <plearn/opt/Optimizer.h>

namespace PLearn {
using namespace std;

class ConditionalDensityNet: public PDistribution
{

private:

    typedef PDistribution inherited;  

protected:

    // *********************
    // * protected options *
    // *********************

    Var input;  // Var(inputsize())
    Var target; // Var(targetsize()-weightsize())
    Var sampleweight; // Var(1) if train_set->hasWeights()
    Var w1; // bias and weights of first hidden layer
    Var w2; // bias and weights of second hidden layer
    Var wout; // bias and weights of output layer
    Var wdirect; // bias and weights for direct in-to-out connection

    Var output; // output layer contains the parameters of the distribution:
    Var outputs; // contains the result of computeOutput, e.g. expectation, or cdf curve
    Var a, pos_a; // output parameter, scalar constant part
    Var b, pos_b; // output parameters, step height parameters
    Var c, pos_c; // output parameters, step smoothing parameters
    Var density;
    Var cumulative;
    Var expected_value;

    VarArray costs; // all costs of interest
    VarArray penalties;
    Var training_cost; // weighted scalar costs[0] including penalties
    Var test_costs; // hconcat(costs)

    VarArray invars;
    VarArray params;  // all arameter input vars

public :
  
    Vec paramsvalues; // values of all parameters
   
protected:

    Var centers, centers_M, steps, steps_M, steps_0, steps_gradient, steps_integral, delta_steps, cum_numerator, cum_denominator;

    // the cond. distribution step multiplicative parameters 
    // are relative to the unconditional cdf step heights, at the mu positions, contained in this source var
    // (computed at the beginning of training).
    Vec unconditional_cdf;
    // unconditional_cdf[i] - unconditional_cdf[i-1], for use to scale the steps of the cdf
    Var unconditional_delta_cdf; 

    // coefficients that scale the pos_c, = initial_hardness/(mu[i]-mu[i-1])
    Var initial_hardnesses;

    // for debugging
    Var prev_centers, prev_centers_M, scaled_prev_centers, 
        scaled_prev_centers_M, minus_prev_centers_0, minus_scaled_prev_centers_0;

public:

    VarArray y_values; // values at which output probability curve is sampled
    Var mu; // output parameters, step location parameters
    mutable Func f; // input -> output
    mutable Func test_costf; // input & target -> output & test_costs
    mutable Func output_and_target_to_cost; // output & target -> cost

    mutable Func cdf_f; // target -> cumulative
    mutable Func mean_f; // output -> expected value
    mutable Func density_f; // target -> density
    mutable Func in2distr_f; // input -> parameters of output distr
    VarArray output_and_target;
    Vec output_and_target_values;
    Var totalcost;
    Var mass_cost;
    Var pos_y_cost;

    // ************************
    // * public build options *
    // ************************

    // ***** OPTIONS PASTED FROM NNET ************** 

    int nhidden;    // number of hidden units in first hidden layer (default:0)
    int nhidden2;   // number of hidden units in second hidden layer (default:0)

    real weight_decay; // default: 0
    real bias_decay;   // default: 0 
    real layer1_weight_decay; // default: MISSING_VALUE
    real layer1_bias_decay;   // default: MISSING_VALUE
    real layer2_weight_decay; // default: MISSING_VALUE
    real layer2_bias_decay;   // default: MISSING_VALUE
    real output_layer_weight_decay; // default: MISSING_VALUE
    real output_layer_bias_decay;   // default: MISSING_VALUE
    real direct_in_to_out_weight_decay; // default: MISSING_VALUE

    string penalty_type; // default: "L2_square"
    bool L1_penalty; // default: false - deprecated, set "penalty_type" to "L1"
    bool direct_in_to_out; // should we include direct input to output connecitons? default: false

    // Build options related to the optimization:
    PP<Optimizer> optimizer; // the optimizer to use (no default)

    int batch_size; // how many samples to use to estimate gradient before an update
    // 0 means the whole training set (default: 1)

    // ***** OPTIONS SPECIFIC TO CONDITIONALDENSITYNET ************** 

    real c_penalization;

    // maximum value that Y can take (minimum value is 0 by default).
    real maxY;

    // threshold value of Y for which we might want to compute P(Y>thresholdY), with outputs_def='t'
    real thresholdY;

    // this weight between 0 and 1 controls the balance of the cost function
    // between minimizing the negative log-likelihood and minimizing squared error
    // if 1 then perform maximum likelihood, if 0 then perform least square optimization
    real log_likelihood_vs_squared_error_balance; 

    // whether to model the mass point with a separate parameter
    bool separate_mass_point;

    // number of terms in the output density function
    int n_output_density_terms;

    real generate_precision;

    // the type of steps used to build the cumulative
    // allowed values are:
    //  - sigmoid_steps: g(y,theta,i) = sigmoid(s(c_i)*(y-mu_i))\n"
    //  - sloped_steps: g(y,theta,i) = s(s(c_i)*(mu_i-y))-s(s(c_i)*(mu_i-y))\n"
    string steps_type;

    // how to initialize the mu_i and how to select the curve points:
    //   - uniform: at regular intervals in [0,maxY]
    //   - log-scale: as the exponential of values at regular intervals in log scale, using the formula:
    //       i-th position = (exp(scale*(i+1-n_output_density_terms)/n_output_density_terms)-exp(-scale))/(1-exp(-scale))
    string centers_initialization;
    string curve_positions;
    real scale;

    // approximate unconditional probability of Y=0 (mass point), used
    // to initialize the parameters
    real unconditional_p0;

    // whether to learn the mu or keep them at their initial values
    bool mu_is_fixed;

    // initial value of softplus(c) (used only in initializeParams())
    real initial_hardness;

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    ConditionalDensityNet();


    // *************************
    // * PDistribution methods *
    // *************************

private: 

    //! This does the actual building. 
    void build_();

protected: 

    //! Declares this class' options
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
    PLEARN_DECLARE_OBJECT(ConditionalDensityNet);

    // ******************************
    // **** PDistribution methods ***
    // ******************************

    //! Set the value for the input part of a conditional probability.
    virtual void setInput(const Vec& input) const;

    // **************************
    // **** PDistribution methods ****
    // **************************
 
    //! return log of probability density log(p(x))
    virtual real log_density(const Vec& x) const;

    //! return survival fn = P(X>x)
    virtual real survival_fn(const Vec& x) const;

    //! return survival fn = P(X<=x)
    virtual real cdf(const Vec& x) const;

    //! return E[X] 
    virtual void expectation(Vec& mu) const;

    //! return Var[X]
    virtual void variance(Mat& cov) const;

    //! Resets the random number generator used by generate using the given seed
    virtual void resetGenerator(long g_seed);

    //! return a pseudo-random sample generated from the distribution.
    virtual void generate(Vec& x) const;

  
    // **************************
    // **** Learner methods ****
    // **************************

    // Default version of inputsize returns learner->inputsize()
    // If this is not appropriate, you should uncomment this and define
    // it properly in the .cc
    // virtual int inputsize() const;

    //! (Re-)initializes the PDistribution in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    //! You may remove this method if your distribution does not implement it
    virtual void forget();

    /*
      virtual int outputsize() const;
    */
    
    void initializeParams();
    void initialize_mu(Vec& mu_);

    //! The role of the train method is to bring the learner up to stage==nstages,
    //! updating the train_stats collector with training costs measured on-line in the process.
    //! You may remove this method if your distribution does not implement it
    virtual void train();

    /*
      virtual void computeOutput(const Vec& input, Vec& output) const;
    */

    /*
      virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
      Vec& output, Vec& costs) const;
    */

    virtual TVec<string> getTrainCostNames() const;
    // virtual TVec<string> getTestCostNames() const;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ConditionalDensityNet);
  
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
