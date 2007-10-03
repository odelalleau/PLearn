// -*- C++ -*-
// mNNet.h
//
// Copyright (C) 2007 Yoshua Bengio
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

// Authors: Yoshua Bengio, PAM

/*! \file mNNet.h */


#ifndef mNNet_INC
#define mNNet_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

/**
 * Multi-layer neural network based on matrix-matrix multiplications.
 */
class mNNet : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! number of outputs to the network
    int noutputs;

    //! sizes of hidden layers
    TVec<int> hidden_layer_sizes;

    //! initial learning rate
    real init_lrate;

    //! learning rate decay factor
    real lrate_decay;

    //! update the parameters only so often
    int minibatch_size;

    //! type of output cost: "NLL" for classification problems, "MSE" for regression
    string output_type;

    //! L1 penalty applied to the output layer's parameters
    real output_layer_L1_penalty_factor;

public:
    //#####  Public Not Build Options  ########################################

public:
    //#####  Public Member Functions  #########################################

    mNNet();


    //#####  PLearner Member Functions  #######################################

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training costs
    //! measured on-line in the process.
    virtual void train();

    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;
    virtual void computeOutputs(const Mat& input, Mat& output) const;
    virtual void computeOutputsAndCosts(const Mat& input, const Mat& target, 
                                        Mat& output, Mat& costs) const;

    //! Computes the costs from already computed output.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;

    // *** SUBCLASS WRITING: ***
    // While in general not necessary, in case of particular needs
    // (efficiency concerns for ex) you may also want to overload
    // some of the following methods: computeOutputAndCosts(),
    // computeCostsOnly(), test(), nTestCosts(), nTrainCosts(),
    // resetInternalState(), isStatefulLearner()

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(mNNet);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here

    //! number of layers of weights (2 for a neural net with one hidden layer)
    int n_layers;

    //! layer sizes (derived from hidden_layer_sizes, inputsize_ and outputsize_)
    TVec<int> layer_sizes;

    //! All the parameters in one vector
    Vec all_params;
    //! Alternate access to the params - one matrix per layer
    TVec<Mat> biases;
    TVec<Mat> weights;
    //! Alternate access to the params - one matrix of dimension
    //! layer_sizes[i+1] x (layer_sizes[i]+1) per layer
    //! (neuron biases in the first column)
    TVec<Mat> layer_params;

    //! Gradient structures - reflect the parameter structures 
    Vec all_params_gradient; 
    TVec<Mat> layer_params_gradient;

    //! Outputs of the neurons
    Mat neuron_gradients;   // one row per example of a minibatch, has
                            // concatenation of layer 0, layer 1, ... gradients.
    TVec<Mat> neuron_gradients_per_layer;   // pointing into neuron_gradients
                                            // (one row per example of a minibatch)

    //! Gradients on the neurons - same structure as for outputs
    mutable Mat neuron_extended_outputs;
    mutable TVec<Mat> neuron_extended_outputs_per_layer;    // with 1's in the
                                                            // first pseudo-neuron, for doing biases
    mutable TVec<Mat> neuron_outputs_per_layer;  

    Mat targets; // one target row per example in a minibatch
    Vec example_weights; // one element per example in a minibatch
    Mat train_costs; // one row per example in a minibatch

    //! Holds training time, an additional cost
    real cumulative_training_time;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! One minibatch training step
    virtual void onlineStep(int t, const Mat& targets, Mat& train_costs, Vec example_weights);

    //! compute a minibatch of size n_examples network top-layer output given layer 0 output (= network input)
    //! (note that log-probabilities are computed for classification tasks, output_type=NLL)
    virtual void fpropNet(int n_examples) const;

    //! compute train costs given the network top-layer output
    //! and write into neuron_gradients_per_layer[n_layers-2], gradient on pre-non-linearity activation
    virtual void fbpropLoss(const Mat& output, const Mat& target, const Vec& example_weights, Mat& train_costs) const;

    virtual void bpropUpdateNet(const int t);
    virtual void bpropNet(const int t);
    
    void l1regularizeOutputs();

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    // (PLEASE IMPLEMENT IN .cc)
    void build_();

private:
    //#####  Private Data Members  ############################################

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(mNNet);

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
