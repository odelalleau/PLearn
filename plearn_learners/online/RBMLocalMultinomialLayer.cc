// -*- C++ -*-

// RBMLocalMultinomialLayer.cc
//
// Copyright (C) 2007 Pascal Lamblin
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

// Author: Pascal Lamblin

/*! \file RBMPLayer.cc */



#include "RBMLocalMultinomialLayer.h"
#include <plearn/math/TMat_maths.h>
#include "RBMConnection.h"

namespace PLearn {
using namespace std;

// Helper functions, like the ones using Vecs, but with Mats
template <class T>
void softmax(const TMat<T>& x, const TMat<T>& y)
{
    int l = x.length();
    int w = x.width();
    PLASSERT( y.length() == l );
    PLASSERT( y.width() == w );

    if (l*w>0)
    {
        TMatElementIterator<real> xp = x.begin();
        TMatElementIterator<real> yp = y.begin();
        T maxx = max(x);
        real s = 0;

        for (int i=0; i<l*w; i++, xp++, yp++)
            s += ( (*yp) = safeexp((*xp) - maxx) );

        if (s == 0)
            PLERROR( "Trying to divide by 0 in softmax");
        s = 1.0 / s;

        for (yp = y.begin(); yp != y.end(); yp++)
            (*yp) *= s;
    }
}

template <class T>
T logadd(const TMat<T>& mat)
{
    if (mat.isEmpty())
        return LOG_INIT;

    TMatElementIterator<real> p_mat = mat.begin();
    T sum = *p_mat++;

    for (int i=1; i<mat.size(); i++, p_mat++)
        sum = logadd(sum, *p_mat);

    return sum;
}

int multinomial_sample(const PP<PRandom>& rg, const Mat& distribution)
{
    real u = rg->uniform_sample();
    TMatElementIterator<real> pi = distribution.begin();
    real s = *pi;
    int w = distribution.width();
    int n = distribution.size();
    int i = 0;

    while (s<u && i<n)
    {
        PLASSERT( *pi == distribution(i / w, i % w) );
        i++;
        pi++;
        s += *pi;
    }
    if (i == n)
        i = n - 1; // Improbable, but...
    return i;
}

template<class T>
void fill_one_hot(const TMat<T>& mat, int hotpos, T coldvalue, T hotvalue)
{
    PLASSERT_MSG( mat.isNotEmpty(), "Given mat must not be empty" );
    PLASSERT_MSG( hotpos >= 0, "hotpos out of mat range" );
    PLASSERT_MSG( mat.size() > 1 || hotpos <= 1, "hotpos out of mat range" );
    PLASSERT_MSG( hotpos < mat.size() || mat.size() == 1,
                  "hotpos out of mat range" );

    if (mat.size() == 1)
        mat(0,0) = (hotpos == 0 ? coldvalue : hotvalue);
    else
    {
        mat.fill(coldvalue);
        int w = mat.width();
        mat(hotpos / w, hotpos % w);
    }
}



PLEARN_IMPLEMENT_OBJECT(
    RBMLocalMultinomialLayer,
    "Layer in an RBM, consisting in one multinomial unit",
    "");

RBMLocalMultinomialLayer::RBMLocalMultinomialLayer( real the_learning_rate ) :
    inherited( the_learning_rate )
{
}

void RBMLocalMultinomialLayer::generateSample()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectation_is_up_to_date, "Expectation should be computed "
            "before calling generateSample()");

    for (int l=0; l<n_images; l++)
    {
        Mat expectation_image = expectation
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);
        Mat sample_image = sample
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);

        for (int i=0; i<images_length; i+=area_length)
            for (int j=0; j<images_width; j+=area_width)
            {
                Mat expectation_area =
                    expectation_image.subMat(i, j, area_length, area_width);
                Mat sample_area =
                    sample_image.subMat(i, j, area_length, area_width);
                int index = multinomial_sample(random_gen, expectation_area);
                fill_one_hot(sample_area, index, real(0), real(1));
            }
    }
}

void RBMLocalMultinomialLayer::generateSamples()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectations_are_up_to_date, "Expectations should be computed "
                        "before calling generateSamples()");

    PLASSERT( samples.width() == size && samples.length() == batch_size );

    for (int k = 0; k < batch_size; k++)
        for (int l=0; l<n_images; l++)
        {
            Mat expectation_image = expectations(k)
                .subVec(l*images_size, images_size)
                .toMat(images_length, images_width);
            Mat sample_image = samples(k)
                .subVec(l*images_size, images_size)
                .toMat(images_length, images_width);

            for (int i=0; i<images_length; i+=area_length)
                for (int j=0; j<images_width; j+=area_width)
                {
                    Mat expectation_area =
                        expectation_image.subMat(i, j, area_length, area_width);
                    Mat sample_area =
                        sample_image.subMat(i, j, area_length, area_width);
                    int index = multinomial_sample(random_gen,
                                                   expectation_area);
                    fill_one_hot(sample_area, index, real(0), real(1));
               }
        }
}

void RBMLocalMultinomialLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    for (int l=0; l<n_images; l++)
    {
        Mat activation_image = activation
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);
        Mat expectation_image = expectation
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);

        for (int i=0; i<images_length; i+=area_length)
            for (int j=0; j<images_width; j+=area_width)
                softmax(
                    activation_image.subMat(i, j, area_length, area_width),
                    expectation_image.subMat(i, j, area_length, area_width)
                    );
    }
    expectation_is_up_to_date = true;
}

void RBMLocalMultinomialLayer::computeExpectations()
{
    if( expectations_are_up_to_date )
        return;

    PLASSERT( expectations.width() == size
              && expectations.length() == batch_size );

    for (int k = 0; k < batch_size; k++)
        for (int l=0; l<n_images; l++)
        {
            Mat activation_image = activations(k)
                .subVec(l*images_size, images_size)
                .toMat(images_length, images_width);
            Mat expectation_image = expectations(k)
                .subVec(l*images_size, images_size)
                .toMat(images_length, images_width);

            for (int i=0; i<images_length; i+=area_length)
                for (int j=0; j<images_width; j+=area_width)
                    softmax(
                        activation_image.subMat(i, j, area_length, area_width),
                        expectation_image.subMat(i, j, area_length, area_width)
                        );
        }

    expectations_are_up_to_date = true;
}


void RBMLocalMultinomialLayer::fprop( const Vec& input, Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    // inefficient
    Vec input_plus_bias = input + bias;
    for (int l=0; l<n_images; l++)
    {
        Mat input_image = input_plus_bias
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);
        Mat output_image = output
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);

        for (int i=0; i<images_length; i+=area_length)
            for (int j=0; j<images_width; j+=area_width)
                softmax(
                    input_image.subMat(i, j, area_length, area_width),
                    output_image.subMat(i, j, area_length, area_width)
                    );
    }
}

///////////
// fprop //
///////////
void RBMLocalMultinomialLayer::fprop( const Vec& input, const Vec& rbm_bias,
                                      Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( rbm_bias.size() == input_size );
    output.resize( output_size );

    // inefficient
    Vec input_plus_bias = input + rbm_bias;
    for (int l=0; l<n_images; l++)
    {
        Mat input_image = input_plus_bias
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);
        Mat output_image = output
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);

        for (int i=0; i<images_length; i+=area_length)
            for (int j=0; j<images_width; j+=area_width)
                softmax(
                    input_image.subMat(i, j, area_length, area_width),
                    output_image.subMat(i, j, area_length, area_width)
                    );
    }
}

/////////////////
// bpropUpdate //
/////////////////
void RBMLocalMultinomialLayer::bpropUpdate(const Vec& input, const Vec& output,
                                           Vec& input_gradient,
                                           const Vec& output_gradient,
                                           bool accumulate)
{
    PLASSERT( input.size() == size );
    PLASSERT( output.size() == size );
    PLASSERT( output_gradient.size() == size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }
    else
    {
        input_gradient.resize( size );
        input_gradient.clear();
    }

    if( momentum != 0. )
        bias_inc.resize( size );

    for (int l=0; l<n_images; l++)
    {
        Mat output_image = output
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);
        Mat input_grad_image = input_gradient
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);
        Mat output_grad_image = output_gradient
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);
        Mat bias_image = bias
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);
        Mat bias_inc_image;
        if (momentum != 0)
            bias_inc_image = bias_inc
                .subVec(l*images_size, images_size)
                .toMat(images_length, images_width);

        for (int i=0; i<images_length; i+=area_length)
            for (int j=0; j<images_width; j+=area_width)
            {
                Mat output_area = output_image
                    .subMat(i, j, area_length, area_width);
                Mat input_grad_area = input_grad_image
                    .subMat(i, j, area_length, area_width);
                Mat output_grad_area = output_grad_image
                    .subMat(i, j, area_length, area_width);
                Mat bias_area = bias_image
                    .subMat(i, j, area_length, area_width);
                Mat bias_inc_area;
                if (momentum != 0)
                    bias_inc_area = bias_inc_image
                        .subMat(i, j, area_length, area_width);

                real outga_dot_outa = dot(output_grad_area, output_area);

                TMatElementIterator<real> pog = output_grad_area.begin();
                TMatElementIterator<real> po = output_area.begin();
                TMatElementIterator<real> pig = input_grad_area.begin();
                TMatElementIterator<real> pb = bias_area.begin();

                TMatElementIterator<real> pbi = bias_inc_area.begin();
/*
                TMatElementIterator<real> pbi;
                if (momentum != 0)
                    pbi = bias_inc_area.begin();
*/
                for (int m=0; m<area_size; m++, pog++, po++, pig++, pb++)
                {
                    real inga_m = (*pog - outga_dot_outa) * (*po);
                    *pig += inga_m;

                    if (momentum == 0)
                    {
                        // update the bias: bias -= learning_rate * input_grad
                        *pb -= learning_rate * (*pig);
                    }
                    else
                    {
                        // The update rule becomes:
                        // bias_inc = momentum * bias_inc
                        //            - learning_rate * input_grad
                        *pbi = momentum * (*pbi) - learning_rate * (*pig);
                        *pb += *pbi;
                        pbi++;
                    }
                }
            }
    }
}

void RBMLocalMultinomialLayer::bpropUpdate(const Mat& inputs,
                                           const Mat& outputs,
                                           Mat& input_gradients,
                                           const Mat& output_gradients,
                                           bool accumulate)
{
    PLASSERT( inputs.width() == size );
    PLASSERT( outputs.width() == size );
    PLASSERT( output_gradients.width() == size );

    int mbatch_size = inputs.length();
    PLASSERT( outputs.length() == mbatch_size );
    PLASSERT( output_gradients.length() == mbatch_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == size &&
                input_gradients.length() == inputs.length(),
                "Cannot resize input_gradient and accumulate into it." );
    }
    else
    {
        input_gradients.resize(inputs.length(), size);
        input_gradients.clear();
    }


    if( momentum != 0. )
        bias_inc.resize( size );

    // TODO see if we can have a speed-up by reorganizing the different steps

    // input_gradients[k][i] =
    //   (output_gradients[k][i]-output_gradients[k].outputs[k]) outputs[k][i]
    real mean_lr = learning_rate / mbatch_size;
    for (int l=0; l<n_images; l++)
    {
        Mat bias_image = bias
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);
        Mat bias_inc_image;
        if (momentum != 0)
            bias_inc_image = bias_inc
                .subVec(l*images_size, images_size)
                .toMat(images_length, images_width);

        for( int k=0; k<mbatch_size; k++ )
        {
            Mat output_image = outputs(k)
                .subVec(l*images_size, images_size)
                .toMat(images_length, images_width);
            Mat input_grad_image = input_gradients(k)
                .subVec(l*images_size, images_size)
                .toMat(images_length, images_width);
            Mat output_grad_image = output_gradients(k)
                .subVec(l*images_size, images_size)
                .toMat(images_length, images_width);

            for (int i=0; i<images_length; i+=area_length)
                for (int j=0; j<images_width; j+=area_width)
                {
                    Mat output_area = output_image
                        .subMat(i, j, area_length, area_width);
                    Mat input_grad_area = input_grad_image
                        .subMat(i, j, area_length, area_width);
                    Mat output_grad_area = output_grad_image
                        .subMat(i, j, area_length, area_width);
                    Mat bias_area = bias_image
                        .subMat(i, j, area_length, area_width);
                    Mat bias_inc_area;
                    if (momentum != 0)
                        bias_inc_area = bias_inc_image
                            .subMat(i, j, area_length, area_width);

                    real outga_dot_outa = dot(output_grad_area, output_area);

                    TMatElementIterator<real> pog = output_grad_area.begin();
                    TMatElementIterator<real> po = output_area.begin();
                    TMatElementIterator<real> pig = input_grad_area.begin();
                    TMatElementIterator<real> pb = bias_area.begin();

                    if (momentum == 0)
                    {
                        for (int i=0; i<area_size; i++, pog++, po++, pig++,
                                                   pb++)
                        {
                            real inga_i = (*pog - outga_dot_outa) * (*po);
                            *pig += inga_i;

                            // update the bias:
                            // bias -= learning_rate * input_grad
                            *pb -= mean_lr * (*pig);
                        }
                    }
                    else
                        PLCHECK_MSG(false,
                                    "Momentum and mini-batch not implemented");
                }
        }
    }
}

//! TODO: add "accumulate" here
void RBMLocalMultinomialLayer::bpropUpdate(const Vec& input,
                                           const Vec& rbm_bias,
                                           const Vec& output,
                                           Vec& input_gradient,
                                           Vec& rbm_bias_gradient,
                                           const Vec& output_gradient)
{
    PLASSERT( input.size() == size );
    PLASSERT( rbm_bias.size() == size );
    PLASSERT( output.size() == size );
    PLASSERT( output_gradient.size() == size );
    input_gradient.resize( size );
    rbm_bias_gradient.resize( size );

    for (int l=0; l<n_images; l++)
    {
        Mat output_image = output
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);
        Mat input_grad_image = input_gradient
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);
        Mat output_grad_image = output_gradient
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);
        Mat rbm_bias_image = rbm_bias
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);

        for (int i=0; i<images_length; i+=area_length)
            for (int j=0; j<images_width; j+=area_width)
            {
                Mat output_area = output_image
                    .subMat(i, j, area_length, area_width);
                Mat input_grad_area = input_grad_image
                    .subMat(i, j, area_length, area_width);
                Mat output_grad_area = output_grad_image
                    .subMat(i, j, area_length, area_width);
                Mat rbm_bias_area = rbm_bias_image
                    .subMat(i, j, area_length, area_width);

                real outga_dot_outa = dot(output_grad_area, output_area);

                TMatElementIterator<real> pog = output_grad_area.begin();
                TMatElementIterator<real> po = output_area.begin();
                TMatElementIterator<real> pig = input_grad_area.begin();
                TMatElementIterator<real> prb = rbm_bias_area.begin();

                for (int m=0; m<area_size; m++, pog++, po++, pig++, prb++)
                {
                    real inga_m = (*pog - outga_dot_outa) * (*po);
                    *pig += inga_m;

                    // update the bias: bias -= learning_rate * input_grad
                    *prb -= learning_rate * (*pig);
                }
            }
    }

    rbm_bias_gradient << input_gradient;
}

//////////////
// fpropNLL //
//////////////
real RBMLocalMultinomialLayer::fpropNLL(const Vec& target)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );

    real nll = 0;
    for (int l=0; l<n_images; l++)
    {
        Mat target_image = target
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);
        Mat expectation_image = expectation
            .subVec(l*images_size, images_size)
            .toMat(images_length, images_width);

        for (int i=0; i<images_length; i+=area_length)
            for (int j=0; j<images_width; j+= area_width)
            {
                Mat target_area = target_image
                    .subMat(i, j, area_length, area_width);
                Mat expectation_area = expectation_image
                    .subMat(i, j, area_length, area_width);

#ifdef BOUNDCHECK
                if (!target_area.hasMissing())
                {
                    PLASSERT_MSG( min(target_area) >= 0.,
                                  "Elements of \"target_areal\" should be"
                                  " positive" );
                    // Ensure the distribution probabilities sum to 1. We relax a
                    // bit the default tolerance as probabilities using
                    // exponentials could suffer numerical imprecisions.
                    if (!is_equal( sum(target_area), 1., 1., 1e-5, 1e-5 ))
                        PLERROR("In RBMLocalMultinomialLayer::fpropNLL -"
                                " Elements of \"target_area\" should sum to 1"
                                " (found a sum = %f)",
                                sum(target_area));
                }
#endif
                TMatElementIterator<real> p_tgt = target_area.begin();
                TMatElementIterator<real> p_exp = expectation_area.begin();
                for (int m=0; m<area_size; m++, p_tgt++, p_exp++)
                {
                    if (!fast_exact_is_equal(*p_tgt, 0))
                        nll -= *p_tgt * pl_log(*p_exp);
                }
            }
    }
    return nll;
}

void RBMLocalMultinomialLayer::fpropNLL(const Mat& targets, const Mat& costs_column)
{
    computeExpectations();

    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );

    for (int k=0; k<batch_size; k++) // loop over minibatch
    {
        real nll = 0;
        for (int l=0; l<n_images; l++)
        {
            Mat target_image = targets(k)
                .subVec(l*images_size, images_size)
                .toMat(images_length, images_width);
            Mat expectation_image = expectations(k)
                .subVec(l*images_size, images_size)
                .toMat(images_length, images_width);

            for (int i=0; i<images_length; i+=area_length)
                for (int j=0; j<images_width; j+= area_width)
                {
                    Mat target_area = target_image
                        .subMat(i, j, area_length, area_width);
                    Mat expectation_area = expectation_image
                        .subMat(i, j, area_length, area_width);

#ifdef BOUNDCHECK
                    if (!target_area.hasMissing())
                    {
                        PLASSERT_MSG( min(target_area) >= 0.,
                                      "Elements of \"target_areal\" should be"
                                      " positive" );
                        // Ensure the distribution probabilities sum to 1. We relax a
                        // bit the default tolerance as probabilities using
                        // exponentials could suffer numerical imprecisions.
                        if (!is_equal( sum(target_area), 1., 1., 1e-5, 1e-5 ))
                            PLERROR("In RBMLocalMultinomialLayer::fpropNLL -"
                                    " Elements of \"target_area\" should sum"
                                    " to 1 (found a sum = %f) at row %d",
                                    sum(target_area), k);
                    }
#endif
                    TMatElementIterator<real> p_tgt = target_area.begin();
                    TMatElementIterator<real> p_exp = expectation_area.begin();
                    for (int m=0; m<area_size; m++, p_tgt++, p_exp++)
                    {
                        if (!fast_exact_is_equal(*p_tgt, 0))
                            nll -= *p_tgt * pl_log(*p_exp);
                    }
                }
        }
        costs_column(k, 0) = nll;
    }
}

void RBMLocalMultinomialLayer::bpropNLL(const Vec& target, real nll,
                                        Vec& bias_gradient)
{
    computeExpectation();

    PLASSERT( target.size() == input_size );
    bias_gradient.resize( size );

    // bias_gradient = expectation - target
    substract(expectation, target, bias_gradient);
}

void RBMLocalMultinomialLayer::bpropNLL(const Mat& targets, const Mat& costs_column,
                                        Mat& bias_gradients)
{
    computeExpectations();

    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );
    bias_gradients.resize( batch_size, size );

    // bias_gradients = expectations - targets
    substract(expectations, targets, bias_gradients);
}

void RBMLocalMultinomialLayer::declareOptions(OptionList& ol)
{
    declareOption(ol, "n_images", &RBMLocalMultinomialLayer::n_images,
                  OptionBase::buildoption,
                  "Number of images in the layer.");

    declareOption(ol, "images_length",
                  &RBMLocalMultinomialLayer::images_length,
                  OptionBase::buildoption,
                  "Length of the images.");

    declareOption(ol, "images_width",
                  &RBMLocalMultinomialLayer::images_width,
                  OptionBase::buildoption,
                  "Width of the images.");

    declareOption(ol, "images_size",
                  &RBMLocalMultinomialLayer::images_size,
                  OptionBase::learntoption,
                  "images_width × images_length.");

    declareOption(ol, "area_length",
                  &RBMLocalMultinomialLayer::area_length,
                  OptionBase::buildoption,
                  "Length of the areas over which the multinomial is set.");

    declareOption(ol, "area_width",
                  &RBMLocalMultinomialLayer::area_width,
                  OptionBase::buildoption,
                  "Width of the areas over which the multinomial is set.");

    declareOption(ol, "area_size",
                  &RBMLocalMultinomialLayer::area_size,
                  OptionBase::learntoption,
                  "area_width × area_length.");

/*
    declareOption(ol, "size", &RBMLocalMultinomialLayer::size,
                  OptionBase::buildoption,
                  "Number of units.");
*/
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "size",
                  &RBMLocalMultinomialLayer::size,
                  OptionBase::learntoption,
                  "n_images × images_width × images_length.");

}

void RBMLocalMultinomialLayer::build_()
{
    PLCHECK_MSG(images_length % area_length == 0,
                "\"images_length\" should be a multiple of \"area_length\"");
    PLCHECK_MSG(images_width % area_width == 0,
                "\"images_width\" should be a multiple of \"area_width\"");

    images_size = images_length * images_width;
    area_size = area_length * area_width;
    size = images_size * n_images;
    n_areas = size / area_size;
}

void RBMLocalMultinomialLayer::build()
{
    inherited::build();
    build_();
}


void RBMLocalMultinomialLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

real RBMLocalMultinomialLayer::energy(const Vec& unit_values) const
{
    return -dot(unit_values, bias);
}


real RBMLocalMultinomialLayer::freeEnergyContribution(
    const Vec& activation_values) const
{
    PLASSERT( activation_values.size() == size );

    // result =
    //  -\sum_{i=0}^{n_areas-1} log(\sum_{j=0}^{area_size-1} exp(a_{ij}))
    real result = 0;
    Mat activation_images = activation_values
        .toMat(n_images*images_length, images_width);
    for (int i=0; i<n_areas; i++)
    {
        Mat activation_area = activation_images
            .subMat((i/images_width)*area_length,
                    (i*area_width) % images_width,
                    area_length,
                    area_width);

        result -= logadd(activation_area);
    }
    return result;
}

int RBMLocalMultinomialLayer::getConfigurationCount()
{
    real approx_count = pow(real(area_size), n_areas);
    int count = 1;
    if (approx_count > 1e30)
        count = INFINITE_CONFIGURATIONS;
    else
        for (int i=0; i<n_areas; i++)
            count *= area_size;

    return count;
}

void RBMLocalMultinomialLayer::getConfiguration(int conf_index, Vec& output)
{
    PLASSERT( output.length() == size );
    PLASSERT( conf_index >= 0 && conf_index < getConfigurationCount() );

    output.clear();
    Mat output_images = output.toMat(n_images*images_length, images_width);
    for (int i=0; i<n_areas; i++)
    {
        int area_conf_index = conf_index % area_size;
        conf_index /= area_size;

        Mat output_area = output_images
            .subMat((i/images_width)*area_length,
                    (i*area_width) % images_width,
                    area_length,
                    area_width );

        output_area(area_conf_index/area_width, area_conf_index%area_width)=1;
    }
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
