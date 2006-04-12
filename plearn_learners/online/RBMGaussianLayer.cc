// -*- C++ -*-

// RBMGaussianLayer.cc
//
// Copyright (C) 2006 Pascal Lamblin & Dan Popovici
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

// Authors: Pascal Lamblin & Dan Popovici

/*! \file RBMPLayer.cc */



#include "RBMGaussianLayer.h"
#include <plearn/math/TMat_maths.h>
#include "RBMParameters.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMGaussianLayer,
    "Layer in an RBM, consisting in Gaussian units",
    "");

RBMGaussianLayer::RBMGaussianLayer()
{
}

RBMGaussianLayer::RBMGaussianLayer( int the_size )
{
    size = the_size;
    units_types = string( the_size, 'q' );
    activations.resize( 2*the_size );
    sample.resize( the_size );
    expectation.resize( the_size );
    expectation_is_up_to_date = false;
}

void RBMGaussianLayer::getUnitActivations( int i, PP<RBMParameters> rbmp )
{
    Vec activation = activations.subVec( 2*i, 2 );
    rbmp->computeUnitActivations( i, activation );
    expectation_is_up_to_date = false;
}

void RBMGaussianLayer::getUnitActivations( PP<RBMParameters> rbmp )
{
    rbmp->computeUnitActivations( activations );
    expectation_is_up_to_date = false;
}

void RBMGaussianLayer::computeSample()
{
    for( int i=0 ; i<size ; i++ )
        sample[i] = random_gen->gaussian_mu_sigma( activations[2*i],
                                                   activations[2*i + 1] );
}

void RBMGaussianLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    for( int i=0 ; i<size ; i++ )
        expectation[i] = activations[2*i];

    expectation_is_up_to_date = true;
}


void RBMGaussianLayer::declareOptions(OptionList& ol)
{
/*
    declareOption(ol, "size", &RBMGaussianLayer::size,
                  OptionBase::buildoption,
                  "Number of units.");
*/
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMGaussianLayer::build_()
{
    if( size < 0 )
        size = units_types.size();
    if( size != (int) units_types.size() )
        units_types = string( size, 'q' );

    activations.resize( 2*size );
    sample.resize( size );
    expectation.resize( size );
    expectation_is_up_to_date = false;
}

void RBMGaussianLayer::build()
{
    inherited::build();
    build_();
}


void RBMGaussianLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


} // end of namespace PLearn


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
