// -*- C++ -*-

// DeepNonLocalManifoldParzen.h
//
// Copyright (C) 2007 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file DeepNonLocalManifoldParzen.h */


#ifndef DeepNonLocalManifoldParzen_INC
#define DeepNonLocalManifoldParzen_INC

#include <plearn/vmat/ClassSubsetVMatrix.h>
#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/CostModule.h>
#include <plearn_learners/online/NLLCostModule.h>
#include <plearn_learners/online/RBMClassificationModule.h>
#include <plearn_learners/online/RBMLayer.h>
#include <plearn_learners/online/RBMMixedLayer.h>
#include <plearn_learners/online/RBMConnection.h>
#include <plearn/misc/PTimer.h>

namespace PLearn {

/**
 * Neural net, trained layer-wise to predict the manifold structure of the data.
 * This information is used in a Manifold Parzen Windows classifier.
 */
class DeepNonLocalManifoldParzen : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! Contrastive divergence learning rate
    real cd_learning_rate;
    
    //! Contrastive divergence decrease constant
    real cd_decrease_ct;

    //! The learning rate used during the autoassociator gradient descent training
    real greedy_learning_rate;

    //! The decrease constant of the learning rate used during the autoassociator
    //! gradient descent training. When a hidden layer has finished its training,
    //! the learning rate is reset to it's initial value.
    real greedy_decrease_ct;

    //! The learning rate used during the fine tuning gradient descent
    real fine_tuning_learning_rate;

    //! The decrease constant of the learning rate used during fine tuning
    //! gradient descent
    real fine_tuning_decrease_ct;

    //! Number of examples to use during each phase of greedy pre-training.
    //! The number of fine-tunig steps is defined by nstages.
    TVec<int> training_schedule;

    //! The layers of units in the network
    TVec< PP<RBMLayer> > layers;

    //! The weights of the connections between the layers
    TVec< PP<RBMConnection> > connections;

    //! The reconstruction weights of the autoassociators
    TVec< PP<RBMConnection> > reconstruction_connections;

    //! Number of nearest neighbors to use to learn
    //! the manifold structure.
    int k_neighbors;

    //! Dimensionality of the manifold
    real n_components;

    //! Minimum value for the noise variance.
    real min_sigma_noise;

    //! Number of classes
    int n_classes;

    //! Output weights L1 penalty factor
    real output_connections_l1_penalty_factor;

    //! Output weights L2 penalty factor
    real output_connections_l2_penalty_factor;

    //! Indication that the parameters for the manifold parzen
    //! windows estimator should be saved during test, to speed up testing.
    bool save_manifold_parzen_parameters;

    //! Indication that the saved manifold parzen parameters are up to date.
    bool manifold_parzen_parameters_are_up_to_date;

    //#####  Public Learnt Options  ###########################################

    //! Number of layers
    int n_layers;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    DeepNonLocalManifoldParzen();

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

    /**
     *  Precomputes the representations of the training set examples, 
     *  to speed up nearest neighbors searches in that space.
     */
    virtual void updateTrainSetRepresentations() const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;

    /**
     *  Declares the training set.  Then calls build() and forget() if
     *  necessary.  Also sets this learner's inputsize_ targetsize_ weightsize_
     *  from those of the training_set.  Note: You shouldn't have to override
     *  this in subclasses, except in maybe to forward the call to an
     *  underlying learner.
     */
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    void greedyStep( const Vec& input, const Vec& target, int index, 
                     Vec train_costs, int stage, Vec similar_example,
                     Vec dissimilar_example);

    void fineTuningStep( const Vec& input, const Vec& target,
                         Vec& train_costs, Mat nearest_neighbors);

    void computeRepresentation( const Vec& input, 
                                Vec& representation, int layer) const;

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(DeepNonLocalManifoldParzen);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Not Options  #####################################################

    //! Stores the activations of the input and hidden layers
    //! (at the input of the layers)
    mutable TVec<Vec> activations;

    //! Stores the expectations of the input and hidden layers
    //! (at the output of the layers)
    mutable TVec<Vec> expectations;

    //! Stores the gradient of the cost wrt the activations of 
    //! the input and hidden layers
    //! (at the input of the layers)
    mutable TVec<Vec> activation_gradients;

    //! Stores the gradient of the cost wrt the expectations of 
    //! the input and hidden layers
    //! (at the output of the layers)
    mutable TVec<Vec> expectation_gradients;

    //! Reconstruction activations
    mutable Vec reconstruction_activations;
    
    //! Reconstruction activation gradients
    mutable Vec reconstruction_activation_gradients;

    //! Reconstruction expectation gradients
    mutable Vec reconstruction_expectation_gradients;

    //! Output weights
    mutable PP<OnlineLearningModuling> output_connections;
    
    //! Example representation
    mutable Vec input_representation;

    //! Example representation at the previous layer, in a greedy step
    Vec previous_input_representation;

    //! All outputs that give the components and sigma_noise values.
    mutable Vec all_outputs;

    //! All outputs' gradients
    Vec all_outputs_gradient;

    //! Variables for density of a Gaussian
    Mat F, F_copy;
    Vec mu;
    Vec pre_sigma_noise;

    //! Variables for the SVD and gradient computation
    Mat Ut, U, V, z, invSigma_F, invSigma_z;
    Vec temp_ncomp, diff_neighbor_input, sm_svd, sn, S;
    Vec uk, fk, uk2, inv_sigma_zj, zj, inv_sigma_fk;

    //! Positive down statistic
    Vec pos_down_val;
    //! Positive up statistic
    Vec pos_up_val;
    //! Negative down statistic
    Vec neg_down_val;
    //! Negative up statistic
    Vec neg_up_val;

    // Saved components of manifold parzen windows

    //! Eigenvectors
    TVec<Mat> eigenvectors;
    //! Eigenvalues
    Mat eigenvalues;
    //! Sigma noises
    Vec sigma_noises;
    //! Mus
    Mat mus;

    //! Datasets for each class
    TVec< PP<ClassSubsetVMatrix> > class_datasets;

    //! Proportions of examples from the other classes (columns), for each
    //! class (rows)
    Vec class_proportions;

    //! Nearest neighbors for each training example
    TMat<int> nearest_neighbors_indices;

    //! Nearest neighbors for each test example
    mutable TVec<int> test_nearest_neighbors_indices;

    //! Nearest neighbor votes for test example
    TVec<int> test_votes;

    //! Data set mapped to last hidden layer space
    mutable Mat train_set_representations;
    mutable VMat train_set_representations_vmat;
    mutable TVec<int> train_set_targets;

    //! Indication that train_set_representations is up to date
    mutable bool train_set_representations_up_to_date;

    //! Stages of the different greedy phases
    TVec<int> greedy_stages;

    //! Currently trained layer (1 means the first hidden layer,
    //! n_layers means the output layer)
    int currently_trained_layer;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

    void build_layers_and_connections();

    void build_classification_cost();

    void bprop_to_bases(const Mat& R, const Mat& M, 
                        const Vec& v1, 
                        const Vec& v2, real alpha);
        
    void setLearningRate( real the_learning_rate );

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here    
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(DeepNonLocalManifoldParzen);

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
