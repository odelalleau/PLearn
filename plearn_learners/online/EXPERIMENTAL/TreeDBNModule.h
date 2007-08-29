// -*- C++ -*-

// TreeDBNModule.h
//
// Copyright (C) 2007 Vytenis Sakenas
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

// Authors: Vytenis Sakenas

/*! \file TreeDBNModule.h */


#ifndef TreeDBNModule_INC
#define TreeDBNModule_INC

#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/RBMModule.h>
#include <plearn/sys/Profiler.h>

namespace PLearn {


class TreeDBNModule : public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

	//! Array of RBMModule's used for every layer	
	TVec <PP<RBMModule> > modules;
	
	//! From how many parents the upper layer RBM takes its input.
	int n_parents_per_node;
	
	//! NOT IMPLEMENTED. Defines how many parent modules adjacent
	//! upper modules share
	int n_shared_parents;
	
	//! Value that multiplies the gradient of energy if it is computed.
	real gradient_multiplier;
	
	//! Whether to propagate gradient throug all hierarchical structure.
	bool propagate_gradient;

	//! Whether to compute and propagate energy gradient.
	bool propagate_energy_gradient;
	
	//! If true, gradient is propagated through all hierarchy, not only rightmost branch
	bool propagate_full_gradient;
	
	//! Ports that should be provided by module. It is mapping
	//! ("external_name", "rbm_name.rbm_port")
	TVec< pair<string, string > > ports;
	
public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    TreeDBNModule();

    // Your other public member functions go here

    //! Perform a fprop step.
    //! Each Mat* pointer in the 'ports_value' vector can be one of:
    //! - a full matrix: this is data that is provided to the module, and can
    //!                  be used to compute other ports' values
    //! - an empty matrix: this is what we want to compute
    //! - a NULL pointer: this is data that is not available, but whose value
    //!                   does not need to be returned (or even computed)
    //! The default version will either:
    //! - call the mini-batch versions of standard fprop if 'ports_value' has
    //!   size 2, with the first value being provided (and the second being
    //!   the desired output)
    //! - crash otherwise
    void fprop(const TVec<Mat*>& ports_value);

    //! Perform a back propagation step (also updating parameters according to
    //! the provided gradient).
    //! The matrices in 'ports_value' must be the same as the ones given in a
    //! previous call to 'fprop' (and thus they should in particular contain
    //! the result of the fprop computation). However, they are not necessarily
    //! the same as the ones given in the LAST call to 'fprop': if there is a
    //! need to store an internal module state, this should be done using a
    //! specific port to store this state.
    //! Each Mat* pointer in the 'ports_gradient' vector can be one of:
    //! - a full matrix  : this is the gradient that is provided to the module,
    //!                    and can be used to compute other ports' gradient.
    //! - an empty matrix: this is a gradient we want to compute and accumulate
    //!                    into. This matrix must have length 0 and a width
    //!                    equal to the width of the corresponding matrix in
    //!                    the 'ports_value' vector (we can thus accumulate
    //!                    gradients using PLearn's ability to keep intact
    //!                    stored values when resizing a matrix' length).
    //! - a NULL pointer : this is a gradient that is not available, but does
    //!                    not need t(const TVec<Mat*>& ports_value);o be returned (or even computed).
    //! The default version tries to use the standard mini-batch bpropUpdate
    //! method, when possible.
    virtual void bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient);

    /* Optional

    //! Given the input, compute the output (possibly resize it appropriately)
    //! SOON TO BE DEPRECATED, USE fprop(const TVec<Mat*>& ports_value)
    virtual void fprop(const Vec& input, Vec& output) const;

    //! Given a batch of inputs, compute the outputs
    //! SOON TO BE DEPRECATED, USE fprop(const TVec<Mat*>& ports_value)
    virtual void fprop(const Mat& inputs, Mat& outputs);
    */

    /* Optional
    //! SOON TO BE DEPRECATED, USE bpropAccUpdate(const TVec<Mat*>& ports_value,
    //!                                           const TVec<Mat*>& ports_gradient)
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

    //! Batch version
    //! SOON TO BE DEPRECATED, USE bpropAccUpdate(const TVec<Mat*>& ports_value,
    //!                                           const TVec<Mat*>& ports_gradient)
    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients,
                             const Mat& output_gradients,
                             bool accumulate=false);
    */

    /* Optional
    //! SOON TO BE DEPRECATED, USE bpropAccUpdate(const TVec<Mat*>& ports_value,
    //!                                           const TVec<Mat*>& ports_gradient)
       A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
       JUST CALLS
            bpropUpdate(input, output, input_gradient, output_gradient)
       AND IGNORES INPUT GRADIENT.
    //! This version does not obtain the input gradient.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             const Vec& output_gradient);

    //! Batch version
    //! SOON TO BE DEPRECATED, USE bpropAccUpdate(const TVec<Mat*>& ports_value,
    //!                                           const TVec<Mat*>& ports_gradient)
    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             const Mat& output_gradients);
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

    /* Optional
       Default implementation prints a warning and does nothing
    //! If this class has a learning rate (or something close to it), set it.
    //! If not, you can redefine this method to get rid of the warning.
    virtual void setLearningRate(real dynamic_learning_rate);
    */

    //! Return the list of ports in the module.
    //! The default implementation returns a pair ("input", "output") to handle
    //! the most common case.
    virtual const TVec<string>& getPorts();
	
	//! Initialize tree to perform sampling.
	//! gibbsTop - number of Gibbs steps to do on the top layer.
	void initSampling(int gibbsTop);
	
	//! Returns a sample of the visible layer.
	//! nGibbs - number of Gibbs steps to do before sampling
	Vec sample(int nGibbs);

	//! Clears all caches.
	void clearCache();


    /* Optional
    //! Return the size of all ports, in the form of a two-column matrix, where
    //! each row represents a port, and the two numbers on a row are
    //! respectively its length and its width (with -1 representing an
    //! undefined or variable value).
    //! The default value fills this matrix with:
    //!     - in the first column (lengths): -1
    //!     - in the second column (widths):
    //!         - -1 if nPorts() < 2
    //!         - 'input_size' for the first row and 'output_size' for the
    //!           second row if nPorts() >= 2
    virtual const TMat<int>& getPortSizes();

    //! Return the index (as in the list of ports returned by getPorts()) of
    //! a given port.
    //! If 'port' does not exist, -1 is returned.
    //  ### Default implementation performs a simple linear search in
    //  ### getPorts().
    virtual int getPortIndex(const string& port);
    */

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(TreeDBNModule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);
	
	static void declareMethods(RemoteMethodMap& rmm);
	
	// Number of layers.
	int n_layers;
	
	TVec<string> port_names;
	TVec<int> port_rbms;		// Index of rbm that writes to this port, or -1 if none does.
	TVec<int> port_index;		// Index of rbms port that writes to this port.
	
	// Number of rbms in layer.
	TVec<int> layer_sizes;
	
	// Steps after which current rbm output will be reused.
	// Same as count of lowest layer inputs that this rbm depends on.
	TVec <int> step_size;
	
	// Last input provided to the module. Is keeped for determining if input is shifted
	// during subsequent fprop calls.
	Vec last_full_input;
	
	// Fprop and bprop data that is used when calling individual
	// rbm methods
	TVec < TVec<Mat*> > fprop_data;
	
	TVec < TVec<Mat*> > bprop_data;
	
	// Same as above but saved for reuse between subsequent
	// calls to fprop
	TVec < TVec<Mat*> > fprop_data_cache;
	
	TVec < TVec<Mat*> > bprop_data_cache;
	
	int time;		// Current pseudo-time of module

	TVec <int> mbi_time;	// Pseudo-time for each sample in the minibatch.
	
	// In ..[i][j] Keeps length of fprop_data for layer i rbm after processing j samples in the minibatch
	TMat <int> mod_batch_length;	
	
	// Storage for actual matrices
	TVec <Mat> mats;
	TVec <Mat> cache_mats;

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();
	
	//! Checks whether b is a shifted to the left by k 
	bool check_shift(Vec &a, Vec &b, int k);
	
	//! Used to compute row number in fprop matrix for a sample
	int hash(int t, int k, int i);

	void appendPort(string name, int rbm_index, string port_name, int port_width);
	
	//! Helper function to create a matrix
	Mat* createMatrix(int length, int width, TVec <Mat> &mats);

	//! Updates cache after fprop
	void updateCache();

	//! Fprop that does not use any optimization
	void full_fprop(const TVec<Mat*>& ports_value);
	
private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(TreeDBNModule);

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
