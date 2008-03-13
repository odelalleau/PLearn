// -*- C++ -*-

// InferenceRBM.cc
//
// Copyright (C) 2008 Pascal Lamblin
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

/*! \file InferenceRBM.cc */

#define PL_LOG_MODULE_NAME "InferenceRBM"

#include "InferenceRBM.h"
#include <plearn/io/pl_log.h>
#include <plearn/base/RemoteDeclareMethod.h>


namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    InferenceRBM,
    "RBM to be used when doing joint supervised learning by CD.",
    "We have input, target and hidden layer. We can compute hidden given\n"
    "(input, target), target given input, or hidden given input."
    );

InferenceRBM::InferenceRBM():
    n_gibbs_steps(0),
    input_size(0),
    target_size(0),
    visible_size(0),
    hidden_size(0)
{
}

// ### Nothing to add here, simply calls build_
void InferenceRBM::build()
{
    inherited::build();
    build_();
}

void InferenceRBM::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(trainvec, copies);

    deepCopyField(input_layer, copies);
    deepCopyField(target_layer, copies);
    deepCopyField(hidden_layer, copies);
    deepCopyField(input_to_hidden, copies);
    deepCopyField(target_to_hidden, copies);
    deepCopyField(random_gen, copies);
    deepCopyField(visible_layer, copies);
    deepCopyField(visible_to_hidden, copies);
    deepCopyField(v0, copies);
    deepCopyField(h0, copies);

}

void InferenceRBM::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &InferenceRBM::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");


    declareOption(ol, "input_layer", &InferenceRBM::input_layer,
                  OptionBase::buildoption,
                  "Input layer (part of visible)");

    declareOption(ol, "target_layer", &InferenceRBM::target_layer,
                  OptionBase::buildoption,
                  "Target layer (part of visible)");

    declareOption(ol, "hidden_layer", &InferenceRBM::hidden_layer,
                  OptionBase::buildoption,
                  "Hidden layer");

    declareOption(ol, "input_to_hidden", &InferenceRBM::input_to_hidden,
                  OptionBase::buildoption,
                  "Connection between input and hidden layers");

    declareOption(ol, "target_to_hidden", &InferenceRBM::target_to_hidden,
                  OptionBase::buildoption,
                  "Connection between target and hidden layers");

    declareOption(ol, "exp_method", &InferenceRBM::exp_method,
                  OptionBase::buildoption,
                  "How to compute hidden and target expectation given input.\n"
                  "Possible values are:\n"
                  "    - \"exact\": exact inference, O(target_size), default\n"
                  "    - \"gibbs\": estimation by Gibbs sampling\n"
                  );

    declareOption(ol, "n_gibbs_steps", &InferenceRBM::n_gibbs_steps,
                  OptionBase::buildoption,
                  "Number of Gibbs steps to use if exp_method==\"gibbs\"");

    declareOption(ol, "random_gen", &InferenceRBM::random_gen,
                  OptionBase::buildoption,
                  "Random numbers generator");

    declareOption(ol, "use_fast_approximations",
                  &InferenceRBM::use_fast_approximations,
                  OptionBase::buildoption,
                  "Whether to use fast approximations in softplus computation");


    declareOption(ol, "visible_layer", &InferenceRBM::visible_layer,
                  OptionBase::learntoption,
                  "Visible layer (input+target)");

    declareOption(ol, "visible_to_hidden", &InferenceRBM::visible_to_hidden,
                  OptionBase::learntoption,
                  "Connection between visible and hidden layers");

    declareOption(ol, "input_size", &InferenceRBM::input_size,
                  OptionBase::learntoption,
                  "Size of input_layer");

    declareOption(ol, "target_size", &InferenceRBM::target_size,
                  OptionBase::learntoption,
                  "Size of target_layer");

    declareOption(ol, "visible_size", &InferenceRBM::visible_size,
                  OptionBase::learntoption,
                  "Size of visible_layer");

    declareOption(ol, "hidden_size", &InferenceRBM::hidden_size,
                  OptionBase::learntoption,
                  "Size of hidden_layer");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


////////////////////
// declareMethods //
////////////////////
void InferenceRBM::declareMethods(RemoteMethodMap& rmm)
{
    // Insert a backpointer to remote methods; note that this
    // different than for declareOptions()
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(
        rmm, "hiddenExpGivenVisible",
        &InferenceRBM::hiddenExpGivenVisible,
        (BodyDoc("Computes the hidden layer's expectation given the visible"),
         ArgDoc ("visible", "Visible layer's values")));

    declareMethod(
        rmm, "hiddenExpGivenInput",
        &InferenceRBM::hiddenExpGivenInput,
        (BodyDoc("Computes the hidden layer's expectation given the input"),
         ArgDoc ("input", "Input layer's values")));

    declareMethod(
        rmm, "hiddenExpGivenInputTarget",
        &InferenceRBM::hiddenExpGivenInputTarget,
        (BodyDoc("Computes the hidden layer's expectation given the input\n"
                 "and the target"),
         ArgDoc ("input", "Input layer's values"),
         ArgDoc ("target", "Target (as an index)")));

    declareMethod(
        rmm, "targetExpGivenInput",
        &InferenceRBM::targetExpGivenInput,
        (BodyDoc("Computes the target layer's expectation given the input"),
         ArgDoc ("input", "Input layer's values")));

    declareMethod(
        rmm, "getHiddenExpGivenVisible",
        &InferenceRBM::getHiddenExpGivenVisible,
        (BodyDoc("Computes the hidden layer's expectation given the visible"),
         ArgDoc ("visible", "Visible layer's values"),
         RetDoc ("Hidden layer's expectation")));

    declareMethod(
        rmm, "getHiddenExpGivenInput",
        &InferenceRBM::getHiddenExpGivenInput,
        (BodyDoc("Computes the hidden layer's expectation given the input"),
         ArgDoc ("input", "Input layer's values"),
         RetDoc ("Hidden layer's expectation")));

    declareMethod(
        rmm, "getHiddenExpGivenInputTarget",
        &InferenceRBM::getHiddenExpGivenInputTarget,
        (BodyDoc("Computes the hidden layer's expectation given the input\n"
                 "and the target"),
         ArgDoc ("input", "Input layer's values"),
         ArgDoc ("target", "Target (as an index)"),
         RetDoc ("Hidden layer's expectation")));

    declareMethod(
        rmm, "getTargetExpGivenInput",
        &InferenceRBM::getTargetExpGivenInput,
        (BodyDoc("Computes the target layer's expectation given the input"),
         ArgDoc ("input", "Input layer's values"),
         RetDoc ("Target layer's expectation")));

    declareMethod(
        rmm, "supCDStep", &InferenceRBM::supCDStep,
        (BodyDoc("Performs one step of CD and updates the parameters"),
         ArgDoc ("visible", "Visible layer's values")));

    declareMethod(
        rmm, "setLearningRate", &InferenceRBM::setLearningRate,
        (BodyDoc("Sets the learning rate of underlying modules"),
         ArgDoc ("the_learning_rate", "The learning rate")));
}


void InferenceRBM::build_()
{
    MODULE_LOG << "build_() called" << endl;

    if( !input_layer || !target_layer || !hidden_layer
        || !input_to_hidden || !target_to_hidden )
    {
        MODULE_LOG << "build_() aborted because layers and connections were"
            " not set" << endl;
        return;
    }

    //! Check (and set) sizes
    input_size = input_layer->size;
    target_size = target_layer->size;
    visible_size = input_size + target_size;
    hidden_size = hidden_layer->size;

    PLASSERT(input_to_hidden->down_size == input_size);
    PLASSERT(input_to_hidden->up_size == hidden_size);
    PLASSERT(target_to_hidden->down_size == target_size);
    PLASSERT(target_to_hidden->up_size == hidden_size);

    //! Build visible layer
    visible_layer = new RBMMixedLayer();
    visible_layer->sub_layers.resize(2);
    visible_layer->sub_layers[0] = input_layer;
    visible_layer->sub_layers[1] = target_layer;
    visible_layer->build();
    PLASSERT(visible_layer->size == visible_size);

    //! Build visible_to_hidden connection
    visible_to_hidden = new RBMMixedConnection();
    visible_to_hidden->sub_connections.resize(1,2);
    visible_to_hidden->sub_connections(0,0) = input_to_hidden;
    visible_to_hidden->sub_connections(0,1) = target_to_hidden;
    visible_to_hidden->build();
    PLASSERT(visible_to_hidden->down_size == visible_size);
    PLASSERT(visible_to_hidden->up_size == hidden_size);

    //! If we have a random_gen, share it with the ones who do not
    if (random_gen)
    {
       if (input_layer->random_gen.isNull())
       {
           input_layer->random_gen = random_gen;
           input_layer->forget();
       }
       if (target_layer->random_gen.isNull())
       {
           target_layer->random_gen = random_gen;
           target_layer->forget();
       }
       if (visible_layer->random_gen.isNull())
       {
           visible_layer->random_gen = random_gen;
           visible_layer->forget();
       }
       if (hidden_layer->random_gen.isNull())
       {
           hidden_layer->random_gen = random_gen;
           hidden_layer->forget();
       }
       if (input_to_hidden->random_gen.isNull())
       {
           input_to_hidden->random_gen = random_gen;
           input_to_hidden->forget();
       }
       if (target_to_hidden->random_gen.isNull())
       {
           target_to_hidden->random_gen = random_gen;
           target_to_hidden->forget();
       }
       if (visible_to_hidden->random_gen.isNull())
       {
           visible_to_hidden->random_gen = random_gen;
           visible_to_hidden->forget();
       }
    }

}

void InferenceRBM::hiddenExpGivenVisible(const Mat& visible)
{
    PLASSERT(visible.width() == visible_size);

    visible_to_hidden->setAsDownInputs(visible);
    hidden_layer->getAllActivations(get_pointer(visible_to_hidden), 0, true);
    hidden_layer->computeExpectations();
}

void InferenceRBM::hiddenExpGivenInputTarget(const Mat& input,
                                             const TVec<int>& target)
{
    int batch_size = input.length();
    PLASSERT(input.width() == input_size);
    PLASSERT(target.length() == batch_size);

    input_to_hidden->setAsDownInputs(input);
    hidden_layer->getAllActivations(get_pointer(input_to_hidden), 0, true);

    for (int k=0; k<batch_size; k++)
        hidden_layer->activations(k) += target_to_hidden->weights(target[k]);

    hidden_layer->expectations_are_up_to_date = false;
    hidden_layer->computeExpectations();
}

void InferenceRBM::targetExpGivenInput(const Mat& input)
{
    PLASSERT(input.width() == input_size);
    int batch_size = input.length();

    // input contains samples (or expectations) from input_layer
    input_to_hidden->setAsDownInputs(input);

    // hidden_layer->activations = bias + input_to_hidden.weights * input
    hidden_layer->getAllActivations(get_pointer(input_to_hidden), 0, true);

    target_layer->setBatchSize(batch_size);

    // target_layer->activations[k][i] =
    //      bias[i] + sum_j softplus(W_ji + hidden_layer->activations[k][j])
    Mat hidden_act = hidden_layer->activations;
    Mat target_act = target_layer->activations;
    Vec target_b = target_layer->bias;
    Mat t_to_h_w = target_to_hidden->weights;

    for (int k=0; k<batch_size; k++)
    {
        target_act(k) << target_b;

        real* target_act_k = target_act[k];
        real* hidden_act_kj = hidden_act[k];
        for (int j=0; j<hidden_size; j++, hidden_act_kj++)
        {
            real* target_act_ki = target_act_k; // copy
            real* t_to_h_w_ji = t_to_h_w[j];
            for (int i=0; i<target_size; i++, target_act_ki++, t_to_h_w_ji++)
            {
                PLASSERT(*target_act_ki == target_act(k,i));
                PLASSERT(*t_to_h_w_ji == t_to_h_w(j,i));
                PLASSERT(*hidden_act_kj == hidden_act(k,j));

                if (use_fast_approximations)
                    *target_act_ki +=
                        tabulated_softplus(*t_to_h_w_ji + *hidden_act_kj);
                else
                    *target_act_ki += softplus(*t_to_h_w_ji + *hidden_act_kj);
            }
        }
    }

    target_layer->expectations_are_up_to_date = false;
    target_layer->computeExpectations();
}

void InferenceRBM::hiddenExpGivenInput(const Mat& input)
{
    PLASSERT(input.width() == input_size);
    int batch_size = input.length();

    targetExpGivenInput(input);
    Mat target_exp = target_layer->getExpectations();

    Mat visible(batch_size, visible_size);
    visible.subMatColumns(0, input_size) << input;

    Mat hidden_exp(batch_size, hidden_size);

    for (int i=0; i<target_size; i++)
    {
        visible.subMatColumns(input_size, target_size).clear();
        visible.column(input_size+i).fill(1.);

        hiddenExpGivenVisible(visible);

        for (int k=0; k<batch_size; k++)
            hidden_exp(k) += target_exp(k,i) * hidden_layer->getExpectations()(k);
    }

    hidden_layer->setExpectations(hidden_exp);
}

Mat InferenceRBM::getHiddenExpGivenVisible(const Mat& visible)
{
    hiddenExpGivenVisible(visible);
    return hidden_layer->getExpectations();
}

Mat InferenceRBM::getHiddenExpGivenInputTarget(const Mat& input,
                                               const TVec<int>& target)
{
    hiddenExpGivenInputTarget(input, target);
    return hidden_layer->getExpectations();
}

Mat InferenceRBM::getTargetExpGivenInput(const Mat& input)
{
    targetExpGivenInput(input);
    return target_layer->getExpectations();
}

Mat InferenceRBM::getHiddenExpGivenInput(const Mat& input)
{
    hiddenExpGivenInput(input);
    return hidden_layer->getExpectations();
}

void InferenceRBM::supCDStep(const Mat& visible)
{
    PLASSERT(visible.width() == visible_size);
    int batch_size = visible.length();

    v0.resize(batch_size,visible_size);
    v0 << visible;

    // positive phase
    hiddenExpGivenVisible(visible);
    h0.resize(batch_size, hidden_size);
    h0 << hidden_layer->getExpectations();

    // Down propagation
    visible_to_hidden->setAsUpInputs(h0);
    visible_layer->getAllActivations(get_pointer(visible_to_hidden), 0, true);
    visible_layer->computeExpectations();
    visible_layer->generateSamples();

    // Negative phase
    hiddenExpGivenVisible(visible_layer->samples);

    // Update
    visible_layer->update(v0, visible_layer->samples);
    visible_to_hidden->update(v0, h0, visible_layer->samples,
                              hidden_layer->getExpectations());
    hidden_layer->update(h0, hidden_layer->getExpectations());
}

void InferenceRBM::unsupCDStep(const Mat& input)
{
    PLCHECK_MSG(false, "Not implemented yet");
}

void InferenceRBM::setLearningRate(real the_learning_rate)
{
    visible_layer->setLearningRate(the_learning_rate);
    visible_to_hidden->setLearningRate(the_learning_rate);
    hidden_layer->setLearningRate(the_learning_rate);
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
