// -*- C++ -*-

// HintonDeepBeliefNet.h
//
// Copyright (C) 2006 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file HintonDeepBeliefNet.h */


#ifndef HintonDeepBeliefNet_INC
#define HintonDeepBeliefNet_INC

#include <plearn_learners/distributions/PDistribution.h>

namespace PLearn {

class RBMLayer;
class RBMMixedLayer;
class RBMMultinomialLayer;
class RBMParameters;
class RBMLLParameters;
class RBMJointLLParameters;

/**
 * Does the same thing as Hinton's deep belief nets
 *
 * @todo Yes
 *
 */
class HintonDeepBeliefNet : public PDistribution
{
    typedef PDistribution inherited;

public:
    //#####  Public Build Options  ############################################

    //! The learning rate used during greedy learning
    real learning_rate;

    //! The learning rate used during the gradient descent
    real fine_tuning_learning_rate;

    //! The weight decay
    real weight_decay;

    //! The method used to initialize the weights:
    //!   - "uniform_linear" = a uniform law in [-1/d, 1/d]
    //!   - "uniform_sqrt"   = a uniform law in [-1/sqrt(d), 1/sqrt(d)]
    //!   - "zero"           = all weights are set to 0
    //! Where d = max( up_layer_size, down_layer_size )
    string initialization_method;

    //! Number of layers, including input layer and last layer, but not target
    //! layer
    int n_layers;

    //! Layers that learn representations of the input,
    //! layers[0] is input layer, layers[n_layers-1] is last layer
    TVec< PP<RBMLayer> > layers;

    //! Last layer, learning joint representations of input and target
    PP<RBMLayer> last_layer;

    //! Target (or label) layer
    PP<RBMMultinomialLayer> target_layer;

    //! Concatenation of target_layer and layers[n_layers-2]
    PP<RBMMixedLayer> joint_layer;

    //! RBMParameters linking the unsupervised layers.
    //! params[i] links layers[i] and layers[i+1]
    TVec< PP<RBMLLParameters> > params;

    //! Parameters linking target_layer and last_layer
    PP<RBMLLParameters> target_params;

    //! Parameters linking joint_layer and last_layer.
    //! Contains params[n_layers-2] and target_params.
    PP<RBMJointLLParameters> joint_params;

    //! Number of examples to use during each of the different greedy
    //! steps of the training phase.
    TVec<int> training_schedule;

    //! Method for fine-tuning the whole network after greedy learning.
    //! One of:
    //!   - "none"
    //!   - "CD" or "contrastive_divergence"
    //!   - "EGD" or "error_gradient_descent"
    //!   - "WS" or "wake_sleep"
    string fine_tuning_method;

//    bool use_sample_rather_than_expectation_in_positive_phase_statistics;

    //! Vector providing information on which information to use during the
    //! contrastive divergence step:
    //!   - 0 means that we use the expectation only,
    //!   - 1 means that we sample (for the next step), but we use the
    //!     expectation in the CD update formula,
    //!   - 2 means that we use the sample only.
    //! The order of the arguments matches the steps of CD:
    //!   - visible unit during positive phase (you should keep it to 0),
    //!   - hidden unit during positive phase,
    //!   - visible unit during negative phase,
    //!   - hidden unit during negative phase (you should keep it to 0).
    TVec<int> use_sample_or_expectation;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    HintonDeepBeliefNet();


    //#####  PDistribution Member Functions  ##################################

    //! Return probability density p(y | x)
    virtual real density(const Vec& y) const;

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
    virtual void train();

    //! Compute a cost, depending on the type of the first output :
    //! if it is the density or the log-density: NLL
    //! if it is the expectation: NLL and class error
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    virtual TVec<string> getTestCostNames() const;
    virtual TVec<string> getTrainCostNames() const;

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(HintonDeepBeliefNet);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    // ...

    //! gradients of cost wrt the activations (output of params)
    mutable TVec< Vec > activation_gradients;

    //! gradients of cost wrt the expectations (output of layers)
    mutable TVec< Vec > expectation_gradients;

    //! gradient wrt output activations
    mutable Vec output_gradient;


protected:
    //#####  Protected Member Functions  ######################################

    virtual void contrastiveDivergenceStep(const PP<RBMLayer>& down_layer,
                                           const PP<RBMParameters>& parameters,
                                           const PP<RBMLayer>& up_layer);

    virtual void greedyStep( const Vec& predictor, int params_index );
    virtual void jointGreedyStep( const Vec& input );
    virtual void fineTuneByGradientDescent( const Vec& input,
                                            const Vec& train_costs );

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

    //! Build the layers
    void build_layers();

    //! Build the parameters if needed
    void build_params();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(HintonDeepBeliefNet);

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
