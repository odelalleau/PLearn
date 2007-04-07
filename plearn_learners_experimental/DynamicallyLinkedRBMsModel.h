// -*- C++ -*-


// DynamicallyLinkedRBMsModel.h
//
// Copyright (C) 2006 Stanislas Lauly
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

/*! \file DynamicallyLinkedRBMsModel.h */



#ifndef DynamicallyLinkedRBMsModel_INC
#define DynamicallyLinkedRBMsModel_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/RBMClassificationModule.h>
#include <plearn_learners/online/RBMLayer.h>
#include <plearn_learners/online/RBMConnection.h>

#include <plearn_learners/online/GradNNetLayerModule.h>

namespace PLearn {

/**
 * Model made of RBMs linked through time.
 */
class DynamicallyLinkedRBMsModel : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! The learning rate used during RBM contrastive divergence learning phase
    real rbm_learning_rate;

    //! The learning rate used during the dynamic links learning phase
    real dynamic_learning_rate;

    //! The learning rate used during the fine tuning phase
    real fine_tuning_learning_rate;

    // TODO The weight decay used during the gradient descent 
    //real grad_weight_decay;

    //! Number of epochs for rbm phase
    int rbm_nstages;

    //! Number of epochs for dynamic phase
    int dynamic_nstages;

    //! Number of epochs for fine tuning phase
    int fine_tuning_nstages;

    //! The visible layer of the RBMs
    PP<RBMLayer> visible_layer;

    //! The hidden layer of the RBMs
    PP<RBMLayer> hidden_layer;

    //! OnlineLearningModule corresponding to dynamic links
    //! between RBMs' hidden layers
    PP<GradNNetLayerModule> dynamic_connections;

    //! OnlineLearningModule corresponding to dynamic links
    //! between RBMs' visible layers
    PP<GradNNetLayerModule> visible_connections;

    //! Copy OnlineLearningModule corresponding to dynamic links
    //! between RBMs' hidden layers
    PP<GradNNetLayerModule> dynamic_connections_copy;

    //! The weights of the connections between the RBM visible and hidden layers
    PP<RBMConnection> connections;

    //#####  Public Learnt Options  ###########################################

    //! Size of the visible layer
    int visible_size;

    //! Number of symbols for each symbolic field of train_set
    TVec<int> symbol_sizes;

    //#####  Not Options  #####################################################


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    DynamicallyLinkedRBMsModel();


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

    //! Computes the costs from already computed output.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;

    //! Clamps the visible units based on an input vector
    void clamp_visible_units(const Vec& input) const;

    //! Updates the RBM parameters in the rbm training phase,
    //! after the visible units have been clamped.
    //! Outputs the negative log-likelihood of the visible training example
    //! given the down phase expectation of the visible unit.
    real rbm_update();

    //! Updates the dynamic connections in the dynamic training
    //! phase, after the visible units have been clamped
    //! Outputs the negative log-likelihood of the hidden representation
    //! of training example t given a sample from the hidden representation
    //! of training example t-1.
    real dynamic_connections_update();

    //! Updates both the RBM parameters and the 
    //! dynamic connections in the fine tuning phase,
    //! after the visible units have been clamped
    real fine_tuning_update();

    virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
                      VMat testoutputs=0, VMat testcosts=0) const;

    


    // *** SUBCLASS WRITING: ***
    // While in general not necessary, in case of particular needs
    // (efficiency concerns for ex) you may also want to overload
    // some of the following methods:
    // virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
    //                                    Vec& output, Vec& costs) const;
    // virtual void computeCostsOnly(const Vec& input, const Vec& target,
    //                               Vec& costs) const;
    // virtual int nTestCosts() const;
    // virtual int nTrainCosts() const;
    // virtual void resetInternalState();
    // virtual bool isStatefulLearner() const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(DynamicallyLinkedRBMsModel);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Not Options  #####################################################

    //! Stores conditional bias
    mutable Vec cond_bias;

    //! Stores visible conditional bias
    mutable Vec visi_cond_bias;

    //! Stores bias gradient
    mutable Vec bias_gradient;

     //! Stores bias gradient
    mutable Vec visi_bias_gradient;

    //! Stores hidden layer target in dynamic learning phase
    mutable Vec hidden_layer_target;

    //! Stores input gradient of dynamic connections
    mutable Vec input_gradient;
    
    //! Stores previous hidden layer value
    mutable Vec previous_input;
    
    //! Stores previous hidden layer value
    mutable Vec previous_hidden_layer;

    //! Stores previous visible layer value
    mutable Vec previous_visible_layer;

    //! Stores a sample from the hidden layer
    mutable Vec hidden_layer_sample;

    //! Stores a sample from the visible layer
    mutable Vec visible_layer_sample;

    //! Store a copy of the positive phase values
    mutable Vec pos_down_values;
    mutable Vec pos_up_values;

    //! Parameter of the dynamic connection
    mutable Vec alpha;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(DynamicallyLinkedRBMsModel);

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
