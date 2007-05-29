// -*- C++ -*-

// OnlineLearningModule.h
//
// Copyright (C) 2005 Yoshua Bengio
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file OnlineLearningModule.h */


#ifndef OnlineLearningModule_INC
#define OnlineLearningModule_INC

#include <plearn/base/Object.h>
#include <plearn/math/PRandom.h>

namespace PLearn {

/**
 *  Learn to map inputs to outputs, online, using caller-provided gradients.
 *
 *  This pure virtual class (i.e. an interface) can basically do two things:
 *    * map an input to an output
 *    * modify itself when told in what direction the output should have
 *      changed (i.e. output gradient),  while optionally giving back the
 *      information about how the input should also have changed
 *      (i.e. input gradient)
 *
 */
class OnlineLearningModule : public Object
{
    typedef Object inherited;

public:
    // BICK UGLY HACK to let modules know if we are performing training of testing
    static bool during_training;

    //#####  Public Build Options  ############################################

    //! input size
    int input_size;

    //! output size
    int output_size;

    string name;

    //! compute simpler estimation of diagonal of the input Hessian matrix,
    //! using only the first (positive) part in:
    //! d²C/dx² ~= d²C/dy² (dy/dx)² [+ dC/dy d²y/dx²]
    bool estimate_simpler_diag_hessian;

    /**
     * Path of the directory associated with this module, in which it should
     * save any file it wishes to create.  The directory will be created if
     * it does not already exist.  If expdir is the empty string (the
     * default), then the module should not create *any* file.
     */
    PPath expdir;

    //! optional random generator, possibly shared among several modules
    PP<PRandom> random_gen;

public:
    //#####  Public Member Fun ctions  #########################################

    //! Default constructor.
    //! For safety, an error is raised if 'the_name' is empty and 'call_build_'
    //! is true, since the default value of 'name' should be the class name,
    //! and it is not available in the constructor.
    OnlineLearningModule(const string& the_name = "",
                         bool call_build_ = false);

    //! given the input, compute the output (possibly resize it appropriately)
    //! SOON TO BE DEPRECATED, USE fprop(const TVec<Mat*>& ports_value)
    virtual void fprop(const Vec& input, Vec& output) const;

    //! Mini-batch fprop.
    //! Default implementation raises an error.
    //! SOON TO BE DEPRECATED, USE fprop(const TVec<Mat*>& ports_value)
    virtual void fprop(const Mat& inputs, Mat& outputs);

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
    virtual void fprop(const TVec<Mat*>& ports_value);

    //! SOON TO BE DEPRECATED, USE bpropAccUpdate(const TVec<Mat*>& ports_value,
    //!                                           const TVec<Mat*>& ports_gradient)
    //! Adapt based on the output gradient: this method should only
    //! be called just after a corresponding fprop; it should be
    //! called with the same arguments as fprop for the first two arguments
    //! (and output should not have been modified since then).
    //! Since sub-classes are supposed to learn ONLINE, the object
    //! is 'ready-to-be-used' just after any bpropUpdate.
    //! N.B. The DEFAULT IMPLEMENTATION JUST CALLS
    //!   bpropUpdate(input, output, input_gradient, output_gradient)
    //! AND IGNORES INPUT GRADIENT.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             const Vec& output_gradient);

    //! SOON TO BE DEPRECATED, USE bpropAccUpdate(const TVec<Mat*>& ports_value,
    //!                                           const TVec<Mat*>& ports_gradient)
    //! Batch version
    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             const Mat& output_gradients);

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
    //!                    not need to be returned (or even computed).
    //! The default version tries to use the standard mini-batch bpropUpdate
    //! method, when possible.
    virtual void bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient);

    //! Same as 'bpropAccUpdate', except that gradients are not accumulated.
    //! This method just calls 'bpropAccUpdate' after properly resizing and
    //! clearing the gradient matrices that need to be computed.
    //! Contrary to 'bpropAccUpdate', the empty matrices (those we want to
    //! compute) need not have the correct width, since we resize them here.
    void bpropUpdate(const TVec<Mat*>& ports_value,
                     const TVec<Mat*>& ports_gradient);

    //! SOON TO BE DEPRECATED, USE bpropAccUpdate(const TVec<Mat*>& ports_value,
    //!                                           const TVec<Mat*>& ports_gradient)
    //! this version allows to obtain the input gradient as well
    //! N.B. THE DEFAULT IMPLEMENTATION JUST RAISES A PLERROR.
    //! The flag indicates whether the input_gradients gets
    //! accumulated into or set with the computed derivatives.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient, const Vec& output_gradient,
                             bool accumulate=false);
    //! SOON TO BE DEPRECATED, USE bpropAccUpdate(const TVec<Mat*>& ports_value,
    //!                                           const TVec<Mat*>& ports_gradient)
    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients, const Mat& output_gradients,
                             bool accumulate=false);

    //! Similar to bpropUpdate, but adapt based also on the estimation
    //! of the diagonal of the Hessian matrix, and propagates this
    //! back. If these methods are defined, you can use them INSTEAD of
    //! bpropUpdate(...)
    //! THE DEFAULT IMPLEMENTATION PROVIDED HERE JUST CALLS
    //!   bbpropUpdate(input, output,
    //!                input_gradient, output_gradient,
    //!                in_hess, out_hess)
    //! AND IGNORES INPUT HESSIAN AND INPUT GRADIENT
    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              const Vec& output_gradient,
                              const Vec& output_diag_hessian);

    //! this version allows to obtain the input gradient and diag_hessian
    //! The flag indicates whether the input_gradient and input_diag_hessian
    //! gets accumulated into or set with the computed derivatives.
    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              Vec& input_gradient,
                              const Vec& output_gradient,
                              Vec& input_diag_hessian,
                              const Vec& output_diag_hessian,
                              bool accumulate=false);

    //! reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget() = 0;


    //! optionally perform some processing after training, or after a
    //! series of fprop/bpropUpdate calls to prepare the model for truly
    //! out-of-sample operation
    virtual void finalize() {}

    // in case bpropUpdate does not do anything, make it known
    virtual bool bpropDoesNothing() { return false; }

    // Allows to change the learning rate, if the derived class has one and
    // allows to do so.
    virtual void setLearningRate(real dynamic_learning_rate);

    //! Return the list of ports in the module.
    //! The default implementation returns a pair ("input", "output") to handle
    //! the most common case.
    virtual const TVec<string>& getPorts();

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
    // Default implementation performs a simple linear search in getPorts().
    virtual int getPortIndex(const string& port);

    //! Return the width of a specific port.
    int getPortWidth(const string& port);

    //! Return the length of a specific port.
    int getPortLength(const string& port);

    //! Return the number of ports in the module.
    int nPorts();

    //! Return name of the i-th port.
    string getPortName(int i);

    //! This method may be called at the end of the 'fprop' or 'bpropAccUpdate'
    //! methods (respectively with 'ports_value' or 'ports_gradient' as
    //! argument) in order to ensure all required ports have been properly
    //! computed (otherwise, an error is thrown).
    void checkProp(const TVec<Mat*>& ports_data);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_ABSTRACT_OBJECT(OnlineLearningModule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

protected:

    //! Used to store the size of each port (may be used in sub-classes).
    TMat<int> port_sizes;

    // Also used in CostModule for instance
    mutable Vec tmp_input_gradient;
    mutable Mat tmpm_input_gradient;
    mutable Vec tmp_input_diag_hessian;


private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(OnlineLearningModule);

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
