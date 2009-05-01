// -*- C++ -*-

// DenoisingRecurrentNet.h
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

// Authors: Stanislas Lauly

/*! \file DenoisingRecurrentNet.h */



#ifndef DenoisingRecurrentNet_INC
#define DenoisingRecurrentNet_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/RBMClassificationModule.h>
#include <plearn_learners/online/RBMLayer.h>
#include <plearn_learners/online/RBMMixedLayer.h>
#include <plearn_learners/online/RBMConnection.h>
#include <plearn_learners/online/RBMMatrixConnection.h>
#include <plearn/vmat/AutoVMatrix.h>
#include <plearn_learners/online/RBMMatrixTransposeConnection.h>

#include <plearn_learners/online/GradNNetLayerModule.h>

namespace PLearn {

/**
 * Model made of RBMs linked through time.
 */
class DenoisingRecurrentNet : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    ////! The learning rate used during RBM contrastive divergence learning phase
    //real rbm_learning_rate;

    ////! Number of epochs for rbm phase
    //int rbm_nstages;
    //int nCost;

    //! The training weights of each target layers
    Vec target_layers_weights;
    
    //! Indication that a mask indicating which target to predict
    //! is present in the input part of the VMatrix dataset.
    //! no loinger an option: this is set to true only for (encoding=="raw_masked_supervised")
    bool use_target_layers_masks;

    //! Value of the first input component for end-of-sequence delimiter
    real end_of_sequence_symbol;

    //! The input layer of the model
    PP<RBMLayer> input_layer;

    //! The target layers of the model
    TVec< PP<RBMLayer> > target_layers;

    //! The hidden layer of the model
    PP<RBMLayer> hidden_layer;

    //! The second hidden layer of the model (optional) 
    PP<RBMLayer> hidden_layer2;

    //! The RBMConnection between the first hidden layers, through time
    PP<RBMConnection> dynamic_connections;

    //! The RBMConnection for the reconstruction between the hidden layers, through time
    PP<RBMConnection> dynamic_reconstruction_connections;

    //! The RBMConnection between the first and second hidden layers (optional)
    PP<RBMConnection> hidden_connections;

    //! The RBMConnection from input_layer to hidden_layer
    PP<RBMConnection> input_connections;

    //! The RBMConnection from input_layer to hidden_layer
    TVec< PP<RBMConnection> > target_connections;

    //#####  Public Learnt Options  ###########################################

    //! Number of elements in the target part of a VMatrix associated
    //! to each target layer
    TVec<int> target_layers_n_of_target_elements;

    //! Number of symbols for each symbolic field of train_set
    TVec<int> input_symbol_sizes;
    
    //! Number of symbols for each symbolic field of train_set
    TVec< TVec<int> > target_symbol_sizes;

    //! Chooses what type of encoding to apply to an input sequence
    //! Possibilities: "timeframe", "note_duration", "note_octav_duration", "raw_masked_supervised"
    string encoding;

    bool noise;

    //! Optional (default=0) factor of L1 regularization term
    real L1_penalty_factor;

    //! Optional (default=0) factor of L2 regularization term
    real L2_penalty_factor;
    
    //! Input window size
    int input_window_size;

    // Phase greedy (unsupervised)
    bool tied_input_reconstruction_weights;
    double input_noise_prob;
    double input_reconstruction_lr;
    double hidden_noise_prob;
    double hidden_reconstruction_lr;
    bool tied_hidden_reconstruction_weights;

    // Phase noisy recurrent (supervised): uses input_noise_prob
    // this phase *also* uses dynamic_gradient_scale_factor;
    double noisy_recurrent_lr;    
    double dynamic_gradient_scale_factor;
    
    // Phase recurrent no noise (supervised fine tuning)
    double recurrent_lr;

    // When training with trainUnconditionalPredictor, this is simply used to store the avg encoded frame
    Vec mean_encoded_vec;

    // learnt bias for input reconstruction
    Vec input_reconstruction_bias;

    // learnt bias for hidden reconstruction
    Vec hidden_reconstruction_bias;

    Vec hidden_reconstruction_bias2;
    
    double prediction_cost_weight;
    double input_reconstruction_cost_weight;
    double hidden_reconstruction_cost_weight;

    double nb_stage_reconstruction;
    double nb_stage_target;

    //#####  Not Options  #####################################################


public:
    //#####  Public static Functions  #########################################
        
    // Finding sequence end indexes
    static void locateSequenceBoundaries(VMat dataset, TVec<int>& boundaries, real end_of_sequence_symbol);

    // encodings

    //! encodes sequence according to specified encoding option
    //! (declared const because it needs to be called in test)
    void encodeSequence(Mat sequence, Mat& encoded_seq) const;

    static void encode_onehot_note_octav_duration(Mat sequence, Mat& encoded_sequence, int prepend_zero_rows,
                                                  bool use_silence, int octav_nbits, int duration_nbits=20);
    
    static void encode_onehot_timeframe(Mat sequence, Mat& encoded_sequence, 
                                        int prepend_zero_rows, bool use_silence=false); 


    static int duration_to_number_of_timeframes(int duration);
    static int getDurationBit(int duration);


    // input noise injection
    void inject_zero_forcing_noise(Mat sequence, double noise_prob) const;

    // noise injection
    void inject_zero_forcing_noise(Vec sequence, double noise_prob) const;

    inline static Vec getInputWindow(Mat sequence, int startpos, int winsize)
    { return sequence.subMatRows(startpos, winsize).toVec(); }
          
    // 
    inline static void getNoteAndOctave(int midi_number, int& note, int& octave)
    {
        note = midi_number%12;
        octave = midi_number/12;
    }
    


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    DenoisingRecurrentNet();


    //#####  PLearner Member Functions  #######################################

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    void setTrainingSet(VMat training_set, bool call_forget=true);

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training costs
    //! measured on-line in the process.
    virtual void train();

    //! Sets the learning of all layers and connections
    //! Remembers it by copying value to current_learning_rate
    void setLearningRate( real the_learning_rate );

    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the number of sequences in the training_set
    int nSequences() const
    { return trainset_boundaries.length(); }

    //! Returns the ith sequence
    void getSequence(int i, Mat& seq) const;

    //! Generate music in a folder
    void generate(int t, int n);

    //! Generate music in a folder
    void generateArtificial();

//    //! Generate a part of the data in a folder
//    void gen();

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;

    //! Use the partition
    void partition(TVec<double> part, TVec<double> periode, TVec<double> vel ) const;
    
    //! Clamps the layer units based on a layer vector
    void clamp_units(const Vec layer_vector, PP<RBMLayer> layer,
                     TVec<int> symbol_sizes) const;

    //! Clamps the layer units based on a layer vector
    //! and provides the associated mask in the correct format.
    void clamp_units(const Vec layer_vector, PP<RBMLayer> layer,
                     TVec<int> symbol_sizes, const Vec original_mask,
                     Vec &formated_mask) const;
    
    //! Updates both the RBM parameters and the 
    //! dynamic connections in the recurrent tuning phase,
    //! after the visible units have been clamped
    void recurrentUpdate(real input_reconstruction_weight,
                         real hidden_reconstruction_cost_weight,
                         real temporal_gradient_contribution,
                         real prediction_cost_weight,
                         real inputAndDynamicPart,
                         Vec train_costs,
                         Vec train_n_items);

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
    PLEARN_DECLARE_OBJECT(DenoisingRecurrentNet);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Not Options  #####################################################
    mutable double current_learning_rate;

    //! Store external data;
    PP<AutoVMatrix> data;
   
    mutable TVec< Mat > acc_target_connections_gr;

    mutable Mat acc_input_connections_gr;

    mutable Mat acc_dynamic_connections_gr;

    mutable Mat acc_reconstruction_dynamic_connections_gr;

    //! Stores accumulate target bias gradient
    mutable Vec acc_target_bias_gr;

    //! Stores accumulate hidden bias gradient
    mutable Vec acc_hidden_bias_gr;

    //! Stores accumulate reconstruction bias gradient
    mutable Vec acc_recons_bias_gr;

    //! Stores bias gradient
    mutable Vec bias_gradient;
    
     //! Stores bias gradient
    mutable Vec visi_bias_gradient;

    //! Stores hidden gradient of dynamic connections
    mutable Vec hidden_gradient;
    
    //! Stores hidden gradient of dynamic connections coming from time t+1
    mutable Vec hidden_temporal_gradient;

    //! List of hidden layers values
    // mutable TVec< Vec > hidden_list;
    mutable Mat hidden_list;
    // mutable TVec< Vec > hidden_act_no_bias_list;
    mutable Mat hidden_act_no_bias_list;

    //! List of second hidden layers values
    // mutable TVec< Vec > hidden2_list;
    mutable Mat hidden2_list;
    // mutable TVec< Vec > hidden2_act_no_bias_list;
    mutable Mat hidden2_act_no_bias_list;

    //! List of target prediction values
    // mutable TVec< TVec< Vec > > target_prediction_list;
    mutable TVec<Mat> target_prediction_list;
    // mutable TVec< TVec< Vec > > target_prediction_act_no_bias_list;
    mutable TVec<Mat> target_prediction_act_no_bias_list;

    //! List of inputs values
    mutable TVec< Vec > input_list;

    //! List of inputs values
    // mutable TVec< TVec< Vec > > targets_list;
    mutable TVec<Mat> targets_list;

    //! List of the nll of the input samples in a sequence
    mutable Mat nll_list;

    //! List of all targets' masks
    // mutable TVec< TVec< Vec > > masks_list;
    mutable TVec< Mat > masks_list;

    //! Contribution of dynamic weights to hidden layer activation
    mutable Vec dynamic_act_no_bias_contribution;

    TVec<int> trainset_boundaries;
    mutable TVec<int> testset_boundaries;

    mutable Mat seq; // contains the current train or test sequence
    mutable Mat encoded_seq; // contains encoded version of current train or test sequence (possibly corrupted by noise)
    mutable Mat clean_encoded_seq; // copy of clean sequence contains encoded version of current train or test sequence

    //mutable Vec input_reconstruction_activation; // temporary Vec to hold input reconstruction activation (before softmax)
    mutable Vec input_reconstruction_prob;       // temporary Vec to hold input reconstruction prob (after applying softmax)
    mutable Vec hidden_reconstruction_prob;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();


    void applyMultipleSoftmaxToInputWindow(Vec input_reconstruction_activation, Vec input_reconstruction_prob);

    // note: the following functions are declared const because they have
    // to be called by test (which is const). Similarly, the members they 
    // manipulate are all declared mutable.
    void recurrentFprop(Vec train_costs, Vec train_n_items, bool useDynamicConnections=true) const;

    //! does encoding if needed and populates the list.
    void encodeSequenceAndPopulateLists(Mat seq, bool doNoise) const;

    //! encodes seq, then populates: inputslist, targets_list, masks_list
    void encodeAndCreateSupervisedSequence(Mat seq) const;

    //! For the (backward testing) raw_masked_supervised case. Populates: input_list, targets_list, masks_list
    void splitRawMaskedSupervisedSequence(Mat seq, bool doNoise) const;

    void encode_artificialData(Mat seq) const; 

    void resize_lists(int l) const;

    void trainUnconditionalPredictor();
    void unconditionalFprop(Vec train_costs, Vec train_n_items) const;

    Mat getTargetConnectionsWeightMatrix(int tar);

    Mat getInputConnectionsWeightMatrix();

    Mat getDynamicConnectionsWeightMatrix();

    Mat getDynamicReconstructionConnectionsWeightMatrix();

    void updateTargetLayer( Vec& grad, 
                            Vec& bias , 
                            real& lr );
    
    void bpropUpdateConnection(const Vec& input, 
                               const Vec& output,
                               Vec& input_gradient,
                               const Vec& output_gradient,
                               Mat& weights,
                               Mat& acc_weights_gr,
                               int& down_size,
                               int& up_size,
                               real& lr,
                               bool accumulate,
                               bool using_penalty_factor);

    void bpropUpdateHiddenLayer(const Vec& input, 
                                const Vec& output,
                                Vec& input_gradient,
                                const Vec& output_gradient,
                                Vec& bias,
                                real& lr);


    void applyWeightPenalty(Mat& weights, Mat& acc_weights_gr, int& down_size, int& up_size, real& lr);

    //! Builds input_reconstruction_prob from hidden (using reconstruction_weights which is  nhidden x ninputs, and input_reconstruction_bias)
    //! then backpropagates reconstruction cost (after comparison with clean_input) with learning rate input_reconstruction_lr
    //! accumulates gradient in hidden_gradient, and updates reconstruction_weights and input_reconstruction_bias
    //! Also computes neg log cost and returns it
    
    double fpropUpdateInputReconstructionFromHidden(Vec hidden, Mat& reconstruction_weights, Mat& acc_weights_gr, Vec& input_reconstruction_bias, Vec& input_reconstruction_prob, 
                                                Vec clean_input, Vec hidden_gradient, double input_reconstruction_cost_weight, double lr);

    
    //! Builds input_reconstruction_prob from hidden (using reconstruction_weights which is  nhidden x ninputs, and input_reconstruction_bias)
    //! Also computes neg log cost and returns it
    double fpropInputReconstructionFromHidden(Vec hidden, Mat reconstruction_weights, Vec& input_reconstruction_bias, Vec& input_reconstruction_prob, 
                                              Vec clean_input);

    //! Backpropagates reconstruction cost (after comparison with clean_input) with learning rate input_reconstruction_lr
    //! accumulates gradient in hidden_gradient, and updates reconstruction_weights and input_reconstruction_bias
    void updateInputReconstructionFromHidden(Vec hidden, Mat& reconstruction_weights, Mat& acc_weights_gr, Vec& input_reconstruction_bias, Vec input_reconstruction_prob, 
                                             Vec clean_input, Vec hidden_gradient, double input_reconstruction_cost_weight, double lr);

    double fpropHiddenReconstructionFromLastHidden(Vec theInput, Vec hidden, Mat reconstruction_weights, Mat& acc_weights_gr, Vec& reconstruction_bias, Vec& reconstruction_bias2, Vec hidden_reconstruction_activation_grad, Vec& reconstruction_prob, 
                                                                          Vec clean_input, Vec hidden_gradient, double hidden_reconstruction_cost_weight, double lr);
    
    double fpropHiddenSymmetricDynamicMatrix(Vec hidden, Mat reconstruction_weights, Vec& reconstruction_prob, 
                                                                          Vec clean_input, Vec hidden_gradient, double hidden_reconstruction_cost_weight, double lr);
private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(DenoisingRecurrentNet);

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
