// -*- C++ -*-

// RBMJointGenericParameters.h
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

/*! \file PLearn/plearn_learners/online/DEPRECATED/RBMJointGenericParameters.h */


#ifndef RBMJointGenericParameters_INC
#define RBMJointGenericParameters_INC

#include "RBMGenericParameters.h"

namespace PLearn {
using namespace std;

//class RBMLayer;

/**
 * Stores and learns the parameters between two layers of an RBM.
 *
 * @todo: yes
 * @deprecated Use ../RBMMixedConnection.h instead
 */
class RBMJointGenericParameters: public RBMGenericParameters
{
    typedef RBMGenericParameters inherited;

public:
    //#####  Public Build Options  ############################################

    //! RBMParameters between the target and the upper layer
    PP<RBMGenericParameters> target_params;

    //! RBMParameters between the conditioning input and the upper layer
    PP<RBMGenericParameters> cond_params;


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMJointGenericParameters( real the_learning_rate=0 );

    //! Constructor from two string prototymes
    RBMJointGenericParameters( PP<RBMGenericParameters>& the_target_params,
                               PP<RBMGenericParameters>& the_cond_params,
                               real the_learning_rate=0 );

    // Your other public member functions go here

    //! Sets input_vec to input, target_given_cond and going_up to false
    virtual void setAsUpInput( const Vec& input ) const;

    //! Sets input_vec to input, target_given_cond to false, going_up to true
    virtual void setAsDownInput( const Vec& input ) const;

    //! Sets input_vec to input, and target_given_cond to true
    virtual void setAsCondInput( const Vec& input ) const;

    //! Computes the vectors of activation of "length" units,
    //! starting from "start", and concatenates them into "activations".
    //! "start" indexes an up unit if "going_up", else a down unit.
    virtual void computeUnitActivations( int start, int length,
                                         const Vec& activations ) const;

    //! given the input, compute the output (possibly resize it  appropriately)
    virtual void fprop(const Vec& input, Vec& output) const;

    //! Adapt based on the output gradient: this method should only
    //! be called just after a corresponding fprop; it should be
    //! called with the same arguments as fprop for the first two arguments
    //! (and output should not have been modified since then).
    //! Since sub-classes are supposed to learn ONLINE, the object
    //! is 'ready-to-be-used' just after any bpropUpdate.
    //! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
    //! JUST CALLS
    //!     bpropUpdate(input, output, input_gradient, output_gradient)
    //! AND IGNORES INPUT GRADIENT.
    // virtual void bpropUpdate(const Vec& input, const Vec& output,
    //                          const Vec& output_gradient);

    //! this version allows to obtain the input gradient as well
    //! N.B. THE DEFAULT IMPLEMENTATION IN SUPER-CLASS JUST RAISES A PLERROR.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient,
                             const Vec& output_gradient);

    //! reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();

    //! optionally perform some processing after training, or after a
    //! series of fprop/bpropUpdate calls to prepare the model for truly
    //! out-of-sample operation.  THE DEFAULT IMPLEMENTATION PROVIDED IN
    //! THE SUPER-CLASS DOES NOT DO ANYTHING.
    // virtual void finalize();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(RBMJointGenericParameters);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Learned Options  #################################################

    //#####  Not Options  #####################################################

    //! If true, we do not compute the up activations from a down example
    //! nor down activations from up example, but we compute the value of the
    //! "target" part of the down layer from the "conditioning" part,
    //! by summing over all the possible values of the up layer.
    mutable bool target_given_cond;

    //! size of the target part of down layer
    int target_size;

    //! size of the conditioning part of down layer
    int cond_size;

    //! stores output activations
    mutable Vec out_act;

protected:
    //#####  Protected Member Functions  ######################################
    //! Computes the activations vector of unit "i", assuming it is linear
    //! "i" indexes a unit in "target" if "target_given_cond", else
    //! an up unit if "going_up", else a down unit.
    virtual void computeLinearUnitActivations( int i, const Vec& activations )
        const;

    //! Computes the activations vector of unit "i", assuming it is quadratic
    //! "i" indexes a unit in "target" if "target_given_cond", else
    //! an up unit if "going_up", else a down unit.
    virtual void computeQuadraticUnitActivations( int i,
                                                  const Vec& activations )
        const;

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! Builds up_units_types and down_units_types from the embedded
    //! RBMParameters
    void build_units_types();

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(RBMJointGenericParameters);

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
