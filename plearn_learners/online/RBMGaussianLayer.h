// -*- C++ -*-

// RBMGaussianLayer.h
//
// Copyright (C) 2006 Dan Popovici & Pascal Lamblin
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

// Authors: Dan Popovici & Pascal Lamblin

/*! \file RBMGaussianLayer.h */


#ifndef RBMGaussianLayer_INC
#define RBMGaussianLayer_INC

#include "RBMLayer.h"

namespace PLearn {
using namespace std;

/**
 * Layer in an RBM formed with binomial units
 *
 * @todo: yes
 */
class RBMGaussianLayer: public RBMLayer
{
    typedef RBMLayer inherited;

public:
    //#####  Public Build Options  ############################################

    real min_quadratic_term;


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMGaussianLayer();

    //! Constructor from the number of units in the multinomial
    RBMGaussianLayer( int the_size );


    // Your other public member functions go here

    //! Uses "rbmp" to obtain the activations of unit "i" of this layer.
    //! This activation vector is computed by the "i+offset"-th unit of "rbmp"
    virtual void getUnitActivations( int i, PP<RBMParameters> rbmp,
                                     int offset=0 );

    //! Uses "rbmp" to obtain the activations of all units in this layer.
    //! Unit 0 of this layer corresponds to unit "offset" of "rbmp".
    virtual void getAllActivations( PP<RBMParameters> rbmp, int offset=0 ) ;

    //! compute a sample, and update the sample field
    virtual void generateSample() ;

    //! compute the expectation
    virtual void computeExpectation() ;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(RBMGaussianLayer);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Not Options  #####################################################

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
DECLARE_OBJECT_PTR(RBMGaussianLayer);

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
