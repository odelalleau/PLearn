// -*- C++ -*-

// SquaredErrorCostModule.cc
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

/*! \file SquaredErrorCostModule.cc */



#include "SquaredErrorCostModule.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    SquaredErrorCostModule,
    "Computes the sum of squared difference between input and target.",
    "");

SquaredErrorCostModule::SquaredErrorCostModule()
{
    output_size = 1;
}

void SquaredErrorCostModule::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &SquaredErrorCostModule::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void SquaredErrorCostModule::build_()
{
    target_size = input_size;
}

///////////
// build //
///////////
void SquaredErrorCostModule::build()
{
    inherited::build();
    build_();
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SquaredErrorCostModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


///////////
// fprop //
///////////
void SquaredErrorCostModule::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( ports_value.length() == 3 );
    Mat* pred = ports_value[0];
    Mat* target = ports_value[1];
    Mat* mse = ports_value[2];
    if (mse && mse->isEmpty()) {
        PLASSERT( pred && !pred->isEmpty() && target && !target->isEmpty() );
        mse->resize(pred->length(), 1);
        // TODO It may be possible to come up with a more efficient
        // implementation.
        for (int i = 0; i < pred->length(); i++) {
            (*mse)(i, 0) = powdistance( (*pred)(i), (*target)(i) );
        }
    }
    checkProp(ports_value);
}

void SquaredErrorCostModule::fprop(const Vec& input, const Vec& target,
                                   Vec& cost) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );
    cost.resize( output_size );

    cost[0] = powdistance( input, target );
}

/////////////////
// bpropUpdate //
/////////////////
void SquaredErrorCostModule::bpropUpdate(const Vec& input, const Vec& target,
                                         real cost, Vec& input_gradient,
                                         bool accumulate)
{
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }
    else
    {
        input_gradient.resize( input_size );
        input_gradient.clear();
    }

    // input_gradient = 2*(input - target)
    for( int i=0 ; i<input_size ; i++ )
    {
        input_gradient[i] += 2*(input[i] - target[i]);
    }
}

void SquaredErrorCostModule::bpropUpdate(const Mat& inputs, const Mat& targets,
            const Vec& costs, Mat& input_gradients, bool accumulate)
{
    PLASSERT( inputs.width() == input_size );
    PLASSERT( targets.width() == target_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == input_size &&
                      input_gradients.length() == inputs.length(),
                      "Cannot resize input_gradients AND accumulate into it" );
    }
    else
    {
        input_gradients.resize( inputs.length(), input_size );
        input_gradients.clear();
    }

    // input_gradient = 2*(input - target)
    // TODO This is a dumb unefficient implementation, for testing purpose.
    for (int k = 0; k < inputs.length(); k++)
        for( int i=0 ; i<input_size ; i++ )
        {
            input_gradients(k, i) += 2*(inputs(k, i) - targets(k, i));
        }
}

//////////////////
// bbpropUpdate //
//////////////////
void SquaredErrorCostModule::bbpropUpdate(const Vec& input, const Vec& target,
                                          real cost,
                                          Vec& input_gradient,
                                          Vec& input_diag_hessian,
                                          bool accumulate)
{
    if( accumulate )
    {
        PLASSERT_MSG( input_diag_hessian.size() == input_size,
                      "Cannot resize input_diag_hessian AND accumulate into it"
                    );
        input_diag_hessian += real(2.);
    }
    else
    {
        input_diag_hessian.resize( input_size );
        input_diag_hessian.fill( 2. );
    }

    bpropUpdate( input, target, cost, input_gradient, accumulate );
}

TVec<string> SquaredErrorCostModule::costNames()
{
    if (name == "" || name == classname())
        return TVec<string>(1, "mse");
    else
        return TVec<string>(1, name + ".mse");
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
