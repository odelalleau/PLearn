// -*- C++ -*-

// RBMParameters.h
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

/*! \file RBMParameters.h */


#ifndef RBMParameters_INC
#define RBMParameters_INC

#include <plearn/base/Object.h>
#include "OnlineLearningModule.h"

namespace PLearn {
using namespace std;

class RBMParameters;
typedef PP<RBMParameters> RBMParams;

// forward declaration
class RBMLayer;

/**
 * Virtual class for the parameters between two layers of an RBM.
 *
 * @todo: yes
 */
class RBMParameters: public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

    //! Each character of this string describes the type of an up unit:
    //!   - 'l' if the energy function of this unit is linear (binomial or
    //!     multinomial unit),
    //!   - 'q' if it is quadratic (for a gaussian unit)
    string up_units_types;

    //! Same meaning as "up_units_types", but with down units
    string down_units_types;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMParameters();

    //! Constructor from two string prototypes
    RBMParameters( string down_types, string up_types );

/*
    //! Constructor from two existing RBMLayers
    RBMParameters( PP<RBMLayer> down, PP<RBMLayer> up );
// */

    // Your other public member functions go here

    //! Sets input_vec to input, and going_up to false
    virtual void setAsUpInput( const Vec& input ) const;

    //! Sets input_vec to input, and going_up to true
    virtual void setAsDownInput( const Vec& input ) const;

    //! Gets size of up layer
    inline int upLayerSize() const
    {
        return up_layer_size;
    }

    //! Gets size of down layer
    inline int downLayerSize() const
    {
        return down_layer_size;
    }

    //! Accumulates positive phase statistics to *_pos_stats
    virtual void accumulatePosStats( const Vec& down_values,
                                     const Vec& up_values ) = 0;

    //! Accumulates negative phase statistics to *_neg_stats
    virtual void accumulateNegStats( const Vec& down_values,
                                     const Vec& up_values ) = 0;

    //! Updates parameters according to contrastive divergence gradient
    virtual void update() = 0;

    //! Clear all information accumulated during stats
    virtual void clearStats() = 0;

    //! Computes the activation vector of unit "i"
    //! (i indexes an up unit if "going_up", else a down unit)
    virtual void computeUnitActivations( int i,
                                         const Vec& activations ) const = 0;

    //! Computes the activation vector of all units
    virtual void computeUnitActivations( const Vec& all_activations ) const=0;

    //! given the input, compute the output (possibly resize it  appropriately)
    virtual void fprop(const Vec& input, Vec& output) const;

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_ABSTRACT_OBJECT(RBMParameters);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Not Options  #####################################################

    //! Points to current input vector
    mutable Vec input_vec;

    //! Tells if input_vec comes from down (true) or up (false)
    mutable bool going_up;

    //! Number of examples accumulated in *_pos_stats
    int pos_count;
    //! Number of examples accumulated in *_neg_stats
    int neg_count;

    //! Number of units on up layer
    int up_layer_size;

    //! Number of units on down layer
    int down_layer_size;


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
DECLARE_OBJECT_PTR(RBMParameters);

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
