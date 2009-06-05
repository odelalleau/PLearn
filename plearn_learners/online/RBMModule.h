// -*- C++ -*-

// RBMModule.h
//
// Copyright (C) 2007 Olivier Delalleau
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

/*! \file RBMModule.h */


#ifndef RBMModule_INC
#define RBMModule_INC

#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/RBMConnection.h>
#include <plearn_learners/online/RBMLayer.h>
#include <map>

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
class RBMModule : public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

    PP<RBMLayer> hidden_layer;
    PP<RBMLayer> visible_layer;
    PP<RBMConnection> connection;
    PP<RBMConnection> reconstruction_connection;

    real cd_learning_rate;
    real grad_learning_rate;
    bool tied_connection_weights;

    bool compute_contrastive_divergence;
    bool compare_true_gradient_with_cd;
    int n_steps_compare;

    //! Number of Gibbs sampling steps in negative phase
    //! of contrastive divergence.
    int n_Gibbs_steps_CD;

    //! used to generate samples from the RBM
    int min_n_Gibbs_steps;
    int n_Gibbs_steps_per_generated_sample;

    bool compute_log_likelihood;
    bool minimize_log_likelihood;

    //#####  Public Learnt Options  ############################################
    //! used to generate samples from the RBM
    int Gibbs_step;
    real log_partition_function;
    bool partition_function_is_stale;

    bool deterministic_reconstruction_in_cd;
    bool stochastic_reconstruction;

    bool standard_cd_grad;
    bool standard_cd_bias_grad;
    bool standard_cd_weights_grad;


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMModule();

    // Your other public member functions go here

    //! Perform one CD_k update, given the Markov chain statistics
    virtual void CDUpdate(const Mat& v_0, const Mat& h_0,
                          const Mat& v_k, const Mat& h_k);

    //! given the input, compute the output (possibly resize it appropriately)
    virtual void fprop(const Vec& input, Vec& output) const;

    /* Optional
       THE DEFAULT IMPLEMENTATION IN SUPER-CLASS JUST RAISES A PLERROR.
    //! Adapt based on the output gradient, and obtain the input gradient.
    //! The flag indicates wether the input_gradient is accumulated or set.
    //! This method should only be called just after a corresponding
    //! fprop; it should be called with the same arguments as fprop
    //! for the first two arguments (and output should not have been
    //! modified since then).
    //! Since sub-classes are supposed to learn ONLINE, the object
    //! is 'ready-to-be-used' just after any bpropUpdate.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient,
                             const Vec& output_gradient,
                             bool accumulate=false);
    */

    /* Optional
       A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
       JUST CALLS
            bpropUpdate(input, output, input_gradient, output_gradient)
       AND IGNORES INPUT GRADIENT.
    //! This version does not obtain the input gradient.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             const Vec& output_gradient);
    */

    /* Optional
       N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
       RAISES A PLERROR.
    //! Similar to bpropUpdate, but adapt based also on the estimation
    //! of the diagonal of the Hessian matrix, and propagates this
    //! back. If these methods are defined, you can use them INSTEAD of
    //! bpropUpdate(...)
    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              Vec& input_gradient,
                              const Vec& output_gradient,
                              Vec& input_diag_hessian,
                              const Vec& output_diag_hessian,
                              bool accumulate=false);
    */

    /* Optional
       N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS,
       WHICH JUST CALLS
            bbpropUpdate(input, output, input_gradient, output_gradient,
                         out_hess, in_hess)
       AND IGNORES INPUT HESSIAN AND INPUT GRADIENT.
    //! This version does not obtain the input gradient and diag_hessian.
    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              const Vec& output_gradient,
                              const Vec& output_diag_hessian);
    */


    //! Reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();


    /* Optional
       THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT
       DO ANYTHING.
    //! Perform some processing after training, or after a series of
    //! fprop/bpropUpdate calls to prepare the model for truly out-of-sample
    //! operation.
    virtual void finalize();
    */

    /* Optional
       THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS RETURNS false
    //! In case bpropUpdate does not do anything, make it known
    virtual bool bpropDoesNothing();
    */

    //! Throws an error (please use explicitely the two different kinds of
    //! learning rates available here).
    virtual void setLearningRate(real dynamic_learning_rate);

    //! Overridden.
    virtual void fprop(const TVec<Mat*>& ports_value);

    //! Overridden.
    virtual void bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient);

    //! Returns all ports in a RBMModule.
    virtual const TVec<string>& getPorts();

    //! The ports' sizes are given by the corresponding RBM layers.
    virtual const TMat<int>& getPortSizes();

    //! Return the index (as in the list of ports returned by getPorts()) of
    //! a given port.
    //! If 'port' does not exist, -1 is returned.
    virtual int getPortIndex(const string& port);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(RBMModule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


protected:

    Mat* hidden_bias;
    Mat* weights;

    //! Used to store gradient w.r.t. expectations of the hidden layer.
    Mat hidden_exp_grad;

    //! Used to store gradient w.r.t. activations of the hidden layer.
    Mat hidden_act_grad;

    //! Used to store gradient w.r.t. expectations of the visible layer.
    Mat visible_exp_grad;

    //! Used to store gradient w.r.t. activations of the visible layer.
    Mat visible_act_grad;

    //! Used to store gradient w.r.t. bias of visible layer
    Vec visible_bias_grad;

    //! Used to cache the hidden layer expectations and activations
    Mat hidden_exp_store;
    Mat hidden_act_store;
    Mat* hidden_act;
    bool hidden_activations_are_computed;

    bool hidden_is_output;

    //! Used to store the contrastive divergence gradient w.r.t. weights.
    Mat store_weights_grad;

    //! Used to store the contrastive divergence gradient w.r.t. hidden bias.
    Mat store_hidden_bias_grad;

    //! List of port names.
    TVec<string> ports;

    //! Map from a port name to its index in the 'ports' vector.
    map<string, int> portname_to_index;

    //! Used to store inputs generated to compute the free energy.
    Mat energy_inputs;

    //! P(x) for all possible configurations x of visible layer. Used only when
    //! 'compare_true_gradient_with_cd' is true (computed at the same time as
    //! the partition function).
    Vec all_p_visible;

    //! Used to store P(h|x) for all values of h and all values of x.
    //! Element (i,j) is P(h_i | x_j).
    Mat all_hidden_cond_prob;

    //! Used to store P(x|h) for all values of h and all values of x.
    //! Element (i,j) is P(x_i | h_j).
    Mat all_visible_cond_prob;

    //! Used to store P(h_t|x) for all values of h_t and some values of x.
    //! Element (i,j) is P(h_ti | x_j).
    Mat p_ht_given_x;

    //! Used to store P(x_t|x) for all values of x_t and some values of x.
    //! Element (i,j) is P(x_ti | x_j).
    Mat p_xt_given_x;

    //#####  Protected Member Functions  ######################################

    //! Add a new port to the 'portname_to_index' map and 'ports' vector.
    void addPortName(const string& name);

    //! Forward the given learning rate to all elements of the layers
    //! and to the reconstruction connections (NOT of the connection weights).
    void setLearningRatesOnlyForLayers(real lr);

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

public:
    //! Forward the given learning rate to all elements of this module.
    void setAllLearningRates(real lr);

    //! Compute activations on the hidden layer based on the provided
    //! visible input.
    //! If 'hidden_bias' is not null nor empty, then it is used as an
    //! additional bias for hidden activations.
    void computeHiddenActivations(const Mat& visible);

    //! Compute activations on the visible layer.
    //! If 'using_reconstruction_connection' is true, then we use the
    //! reconstruction connection to compute these activations. Otherwise, we
    //! use the normal connection, in a 'top->down' fashion.
    void computeVisibleActivations(const Mat& hidden,
                                   bool using_reconstruction_connection=false);

    //! Compute activations on the hidden layer based on the provided visible
    //! input during positive phase. This method is called to ensure hidden
    //! hidden activations are computed only once, and during a fprop it should
    //! always be called with the same 'visible' input.
    //! If 'hidden_act' is not null, it is filled with the computed hidden
    //! activations.
    void computePositivePhaseHiddenActivations(const Mat& visible);

    //! Sample hidden layer data based on the provided 'visible' inputs.
    void sampleHiddenGivenVisible(const Mat& visible);

    //! Sample visible layer data based on the provided 'hidden' inputs.
    void sampleVisibleGivenHidden(const Mat& hidden);

    //! Compute free energy on the visible layer and store it in the 'energy'
    //! matrix.
    //! The 'positive_phase' boolean is used to save computations when we know
    //! we are in the positive phase of fprop.
    void computeFreeEnergyOfVisible(const Mat& visible, Mat& energy,
                                    bool positive_phase = true);

    //! Compute free energy on the hidden layer and store it in the 'energy'
    //! matrix.
    void computeFreeEnergyOfHidden(const Mat& hidden, Mat& energy);

    //! Compute energy of the joint (visible, hidden) configuration and store
    //! it in the 'energy' matrix.
    //! The 'positive_phase' boolean is used to save computations when we know
    //! we are in the positive phase of fprop.
    void computeEnergy(const Mat& visible, const Mat& hidden, Mat& energy,
                       bool positive_phase = true);

    void computePartitionFunction();

    //! See remote documentation.
    //! Note that this is really a convenience method only, to avoid having to
    //! obtain this likelihood through the ports.
    Vec computeLogLikelihoodOfVisible(const Mat& visible);

    //! Compute probabilities of all hidden configurations given some visible
    //! inputs. The 'p_hidden' matrix is filled such that the (i,j)-th element
    //! is P(hidden_configuration_i | visible_j).
    void computeAllHiddenProbabilities(const Mat& visible,
                                       const Mat& p_hidden);

    void computeNegLogPVisibleGivenPHidden(Mat visible, Mat hidden, Mat* neg_log_phidden, Mat& neg_log_pvisible_given_phidden);

protected:
    static void declareMethods(RemoteMethodMap& rmm);


private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(RBMModule);

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
