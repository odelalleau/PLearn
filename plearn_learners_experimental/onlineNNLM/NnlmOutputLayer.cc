// -*- C++ -*-

// NnlmOutputLayer.cc
//
// Copyright (C) 2006 Pierre-Antoine Manzagol
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

// Authors: Pierre-Antoine Manzagol

/*! \file NnlmOutputLayer.cc */



#include "NnlmOutputLayer.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NnlmOutputLayer,
    "Implements the output layer for the NNLM.",
    "MULTI-LINE \nHELP");

NnlmOutputLayer::NnlmOutputLayer() :
    OnlineLearningModule(),
    vocabulary_size( -1 ),
    word_representation_size( -1 ),
    context_size( -1 ),
    cost( 0 ),
    start_discount_rate( 0.8 ),
    discount_decrease_constant( 0 ),
    coeff_class_conditional_uniform_mixture( 0.9 ),
    discount_rate( 0.8 ),
    log_p_g_r( 0.9 ),
    sum_log_p_g_r( 0 )
    
{
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

void NnlmOutputLayer::declareOptions(OptionList& ol)
{

    declareOption(ol, "vocabulary_size",
                  &NnlmOutputLayer::vocabulary_size,
                  OptionBase::buildoption,
                  "size of vocabulary used");

    declareOption(ol, "word_representation_size",
                  &NnlmOutputLayer::word_representation_size,
                  OptionBase::buildoption,
                  "size of the real distributed word representation");

    declareOption(ol, "context_size",
                  &NnlmOutputLayer::context_size,
                  OptionBase::buildoption,
                  "size of word context");

    declareOption(ol, "start_discount_rate",
                  &NnlmOutputLayer::start_discount_rate,
                  OptionBase::buildoption,
                  "Discount-rate of stochastic old gaussian values when computing the new values");

    declareOption(ol, "discount_decrease_constant",
                  &NnlmOutputLayer::discount_decrease_constant,
                  OptionBase::buildoption,
                  "Decrease constant of gaussian parameters discount rate");

    // * Learnt

    declareOption(ol, "step_number", &NnlmOutputLayer::step_number,
                  OptionBase::learntoption,
                  "The step number, incremented after each update.");

    declareOption(ol, "mu", &NnlmOutputLayer::mu,
                  OptionBase::learntoption,
                  "mu(i) -> moyenne empirique des x quand y=i" );
    declareOption(ol, "sigma2", &NnlmOutputLayer::sigma2,
                  OptionBase::learntoption,
                  "sigma2(i) -> variance empirique des x quand y=i" );
    declareOption(ol, "pi", &NnlmOutputLayer::pi,
                  OptionBase::learntoption,
                  "pi[i] -> moyenne empirique de y==i" );

    declareOption(ol, "sumX", &NnlmOutputLayer::sumX,
                  OptionBase::learntoption,
                  "sumX(i) -> sum_t x_t 1_{y==i}" );
    declareOption(ol, "sumX2", &NnlmOutputLayer::sumX2,
                  OptionBase::learntoption,
                  "sumX2(i) -> sum_t x_t^2 1_{y==i}" );
    declareOption(ol, "sumI", &NnlmOutputLayer::sumI,
                  OptionBase::learntoption,
                  "sumI(i) -> sum_t 1_{y==i}" );
    declareOption(ol, "s_sumI", &NnlmOutputLayer::s_sumI,
                  OptionBase::learntoption,
                  "sum_t 1" );


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void NnlmOutputLayer::build_()
{

    // *** Sanity checks
    if( input_size <= 0 ) // has not been initialized
    {
        PLERROR("NnlmOutputLayer::build_: 'input_size' <= 0 (%i).\n"
                "You should set it to a positive integer.\n", input_size);
    }
    else if( output_size != 1 )
    {
        PLERROR("NnlmOutputLayer::build_: 'output_size'(=%i) != 1\n"
                  , output_size);
    }
    else if( vocabulary_size <= 0 )
    {
        PLERROR("NnlmOutputLayer::build_: 'vocabulary_size' <= 0 (%i).\n"
                  , vocabulary_size);
    }


    // *** Parameters not initialized
    if( mu.size() == 0 )   {
        resetParameters();
    }

}

// ### Nothing to add here, simply calls build_
void NnlmOutputLayer::build()
{
    inherited::build();
    build_();
}


void NnlmOutputLayer::resetParameters()
{
    mu.resize( vocabulary_size, input_size);
    mu.fill( 0 );
    sigma2.resize( vocabulary_size, input_size);
    sigma2.fill( 0 );
    pi.resize( vocabulary_size );
    pi.fill( 0 );

    sumX.resize( vocabulary_size, input_size);
    sumX.fill( 0 );
    sumX2.resize( vocabulary_size, input_size);
    sumX2.fill( 0 );
    sumI.resize( vocabulary_size );
    sumI.fill( 0 );
    s_sumI = 0;

    vec1.resize(input_size);
    vec2.resize(input_size);
    context.resize(context_size);

    step_number = 0;

}

void NnlmOutputLayer::setCurrentWord(int the_current_word) 
{
    if( the_current_word >= vocabulary_size )
    {
        PLERROR("NnlmOutputLayer::setCurrentWord:'the_current_word'(=%i) >= \n"
                  "'vocabulary_size'(=%i)\n"
                  , the_current_word, vocabulary_size);
    }

    current_word = the_current_word;
}


void NnlmOutputLayer::setContext(const Vec& the_current_context) 
{
    context << the_current_context;

}

void NnlmOutputLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(mu, copies);
    deepCopyField(sigma2, copies);
    deepCopyField(pi, copies);

    deepCopyField(sumX, copies);
    deepCopyField(sumX2, copies);
    deepCopyField(sumI, copies);

    deepCopyField(vec1, copies);
    deepCopyField(vec2, copies);

    deepCopyField(context, copies);

    // ### Remove this line when you have fully implemented this method.
    //PLERROR("NnlmOutputLayer::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

//! given the input, compute the output (possibly resize it  appropriately)
//! Compute log p(r,i)
//! where   r is the context's semantic representation (the input)
//!         i is the current word
//!         p(r,i) = p(r|i) * p(i)
//!         p(r|i) = a * p_gaussian(r|i) + (1-a) / 2^n
//!         a = coeff_class_conditional_uniform_mixture
//!
void NnlmOutputLayer::fprop(const Vec& input, Vec& output) const
{

    // *** If not ready (need 2 bpropUpdate)
    // So we don't do a /0 with /sigma2(current_word, i)
    if( sumI[ current_word ] < 2 )  {
      output.fill(0.0);
      return;
    }

    // *** Sanity check
    int in_size = input.size();
    if( in_size != input_size ) {
        PLERROR("NnlmOutputLayer::fprop: 'input.size()' should be equal\n"
                " to 'input_size' (%i != %i)\n", in_size, input_size);
    }


    // * Compute gaussian's exponent - 'g' means gaussian
    real r;
    real g_exponent = 0.0;
    real det_g_covariance = 1.0;

    for(int i=0; i<input_size; i++) {
      r = input[i] - mu(current_word, i);
      g_exponent += r * r / sigma2(current_word, i);

      // determinant of covariance matrix
      det_g_covariance *= sigma2(current_word, i);
    }
    g_exponent *= -0.5;

    // * Compute normalizing factor
    real log_g_normalization = - 0.5 * ( (input_size) * safelog(2.0 * Pi) + safelog(det_g_covariance) );

    // * Compute log p_gaussian(r|i)
    real log_p_r_i = g_exponent + log_g_normalization;

    // * Compute log p(r|i)
    real a = coeff_class_conditional_uniform_mixture;
    log_p_r_i = logadd( safelog(a) + log_p_r_i , safelog(1-a) - (input_size) * safelog(2));

    // * Compute log p(r,i)
    real log_p_ri = safelog(pi[current_word]) + log_p_r_i;

    // * Compute output
    output[0] = -( log_p_ri );

    // * Compute posterior for coeff_class_conditional_uniform_mixture evaluation in the bpropUpdate
    // p(generated by gaussian| r) = a p_g(r|i) / p(r|i)
    log_p_g_r = safelog(a) + g_exponent + log_g_normalization - log_p_r_i;

}


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
void NnlmOutputLayer::bpropUpdate(const Vec& input, const Vec& output,
                               const Vec& output_gradient)
{
    int in_size = input.size();
    int out_size = output.size();
    int og_size = output_gradient.size();

    // size check
    if( in_size != input_size )
    {
        PLERROR("NnlmOutputLayer::bpropUpdate:'input.size()' should be equal\n"
                " to 'input_size' (%i != %i)\n", in_size, input_size);
    }
    if( out_size != output_size )
    {
        PLERROR("NnlmOutputLayer::bpropUpdate:'output.size()' should be"
                " equal\n"
                " to 'output_size' (%i != %i)\n", out_size, output_size);
    }
    if( og_size != output_size )
    {
        PLERROR("NnlmOutputLayer::bpropUpdate:'output_gradient.size()'"
                " should\n"
                " be equal to 'output_size' (%i != %i)\n",
                og_size, output_size);
    }


    // TODO is this any good? If so, at least change variable names
    discount_rate = start_discount_rate + step_number * discount_decrease_constant;

    // * Update parameters - using discount
    // discount * ancien + (1-discount) * nouveau
    // so we don't get a sigma that is zero
    if( sumI[ current_word ] >= 1 ) {

        for(int i=0; i<input_size; i++) {

            mu( current_word, i ) = discount_rate * mu( current_word, i ) + (1.0 - discount_rate) * input[i];

            // We reuse the old sigma instead of recomputing it with the new mu... is it a good idea?
            // If we consider we're tracking a moving target maybe.
            sigma2( current_word, i ) = discount_rate * sigma2( current_word, i ) +
                + (1.0-discount_rate) * ( input[i] - mu( current_word, i ) ) * ( input[i] - mu( current_word, i ) );
        }
    // initialize
    } else  {
        for(int i=0; i<input_size; i++) {
            mu( current_word, i ) = input[i];
            sigma2( current_word, i ) = 0.5;
        }
    }


    // * Update counts
    for(int i=0; i<input_size; i++) {
      sumX( current_word, i ) += input[i];
      sumX2( current_word, i ) += input[i]*input[i];
    }
    sumI[ current_word ] += 1;
    s_sumI += 1;

    if( sumI[ current_word ] >= 1 ) {
        pi[current_word] = (real)sumI[ current_word ] / (real)s_sumI;

        // Update coeff_class_conditional_uniform_mixture
        sum_log_p_g_r += log_p_g_r;
        coeff_class_conditional_uniform_mixture = sum_log_p_g_r / s_sumI;
    }


}

/*
//! this version allows to obtain the input gradient as well
//! N.B. THE DEFAULT IMPLEMENTATION IN SUPER-CLASS JUST RAISES A PLERROR.
void NnlmOutputLayer::bpropUpdate(const Vec& input, const Vec& output,
                               Vec& input_gradient,
                               const Vec& output_gradient)
{
}
*/


//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void NnlmOutputLayer::forget()
{
    resetParameters();

}

/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void NnlmOutputLayer::finalize()
{
}
*/

/* THIS METHOD IS OPTIONAL
//! in case bpropUpdate does not do anything, make it known
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS RETURNS false;
bool NnlmOutputLayer::bpropDoesNothing()
{
}
*/

/* THIS METHOD IS OPTIONAL
//! Similar to bpropUpdate, but adapt based also on the estimation
//! of the diagonal of the Hessian matrix, and propagates this
//! back. If these methods are defined, you can use them INSTEAD of
//! bpropUpdate(...)
//! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
//! JUST CALLS
//!     bbpropUpdate(input, output, input_gradient, output_gradient,
//!                  in_hess, out_hess)
//! AND IGNORES INPUT HESSIAN AND INPUT GRADIENT.
void NnlmOutputLayer::bbpropUpdate(const Vec& input, const Vec& output,
                                const Vec& output_gradient,
                                const Vec& output_diag_hessian)
{
}
*/

/* THIS METHOD IS OPTIONAL
//! Similar to bpropUpdate, but adapt based also on the estimation
//! of the diagonal of the Hessian matrix, and propagates this
//! back. If these methods are defined, you can use them INSTEAD of
//! bpropUpdate(...)
//! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
//! RAISES A PLERROR.
void NnlmOutputLayer::bbpropUpdate(const Vec& input, const Vec& output,
                                Vec& input_gradient,
                                const Vec& output_gradient,
                                Vec& input_diag_hessian,
                                const Vec& output_diag_hessian)
{
}
*/


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
