// -*- C++ -*-

// NatGradNNet.cc
//
// Copyright (C) 2007 Yoshua Bengio
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

// Authors: Yoshua Bengio

/*! \file NatGradNNet.cc */


#include "NatGradNNet.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NatGradNNet,
    "Multi-layer neural network trained with an efficient Natural Gradient optimization",
    "A separate covariance matrix is estimated for the gradients associated with the\n"
    "the input weights of each neuron, and a covariance matrix between the gradients\n"
    "on the neurons is also computed. These are combined to obtained an adjusted gradient\n"
    "on all the parameters. The class NatGradEstimator embodies the adjustment algorithm.\n"
    "Users may specify different options for the estimator that is used for correcting\n"
    "the neurons gradients and for the estimator that is used for correcting the\n"
    "parameters gradients (separately for each neuron).\n"
    );

NatGradNNet::NatGradNNet()
    : noutputs(-1),
      init_lrate(0.01),
      lrate_decay(0),
      output_type("NLL"),
      n_layers(-1)
{
    random_gen = new PRandom();
}

void NatGradNNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "noutputs", &NatGradNNet::noutputs,
                  OptionBase::buildoption,
                  "Number of outputs of the neural network, which can be derived from  output_type and targetsize_\n");

    declareOption(ol, "n_layers", &NatGradNNet::n_layers,
                  OptionBase::learntoption,
                  "Number of layers of weights (ie. 2 for a neural net with one hidden layer).\n"
                  "Needs not be specified explicitly (derived from hidden_layer_sizes).\n");

    declareOption(ol, "hidden_layer_sizes", &NatGradNNet::hidden_layer_sizes,
                  OptionBase::buildoption,
                  "Defines the architecture of the multi-layer neural network by\n"
                  "specifying the number of hidden units in each hidden layer.\n");

    declareOption(ol, "layer_sizes", &NatGradNNet::layer_sizes,
                  OptionBase::learntoption,
                  "Derived from hidden_layer_sizes, inputsize_ and noutputs\n");

    declareOption(ol, "layer_params", &NatGradNNet::layer_params,
                  OptionBase::learntoption,
                  "Parameters for each layer, organized as follows: layer_params[i] \n"
                  "is a matrix of dimension layer_sizes[i+1] x (layer_sizes[i]+1)\n"
                  "containing the neuron biases in its first column.\n");

    declareOption(ol, "init_lrate", &NatGradNNet::init_lrate,
                  OptionBase::buildoption,
                  "Initial learning rate\n");

    declareOption(ol, "lrate_decay", &NatGradNNet::lrate_decay,
                  OptionBase::buildoption,
                  "Learning rate decay factor\n");

    declareOption(ol, "neurons_natgrad", &NatGradNNet::neurons_natgrad,
                  OptionBase::buildoption,
                  "Optional NatGradEstimator for the neurons gradient.\n"
                  "If not provided, then the natural gradient correction\n"
                  "on the neurons gradient is not performed.\n");

    declareOption(ol, "params_natgrad_template", 
                  &NatGradNNet::params_natgrad_template,
                  OptionBase::buildoption,
                  "Optional template NatGradEstimator object for the gradient of the parameters inside each neuron\n"
                  "It is replicated in the params_natgrad vector, for each neuron\n"
                  "If not provided, then the neuron-specific natural gradient estimator is not used.\n");

    declareOption(ol, "params_natgrad_per_neuron", 
                  &NatGradNNet::params_natgrad_per_neuron,
                  OptionBase::learntoption,
                  "Vector of NatGradEstimator objects for the gradient of the parameters inside each neuron\n"
                  "They are copies of the params_natgrad_template provided by the user\n");

    declareOption(ol, "full_natgrad", &NatGradNNet::full_natgrad,
                  OptionBase::buildoption,
                  "NatGradEstimator for all the parameter gradients simultaneously.\n"
                  "This should not be set if neurons_natgrad or params_natgrad_template\n"
                  "is provided. If none of the NatGradEstimators is provided, then\n"
                  "regular stochastic gradient is performed.\n");


    declareOption(ol, "output_type", 
                  &NatGradNNet::output_type,
                  OptionBase::buildoption,
                  "type of output cost: 'NLL' for classification problems,\n"
                  "or 'MSE' for regression.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void NatGradNNet::build_()
{
    if (noutputs<0)
    {
        if (output_type=="MSE")
            noutputs = targetsize_;
        else if (output_type=="NLL")
            PLERROR("NatGradNNet: if output_type=NLL (classification), one \n"
                    "should provide noutputs = number of classes, or possibly\n"
                    "1 when 2 classes\n");
        else PLERROR("NatGradNNet: output_type should be NLL or MSE\n");
    }
    n_layers = hidden_layer_sizes.length()+2;
    layer_sizes.resize(n_layers);
    layer_sizes.subVec(1,n_layers-2) << hidden_layer_sizes;
    layer_sizes[0]=inputsize_;
    layer_sizes[n_layers-1]=noutputs;
    layer_params.resize(n_layers-1);
    int n_neurons=0;
    for (int i=0;i<n_layers-1;i++)
    {
        layer_params[i].resize(layer_sizes[i+1],layer_sizes[i]+1);
        n_neurons+=layer_sizes[i+1];
    }
    if (params_natgrad_template)
    {
        params_natgrad_per_neuron.resize(n_neurons);
        for (int i=0;i<n_neurons;i++)
            params_natgrad_per_neuron[i] = PLearn::deepCopy(params_natgrad_template);
        
    }
    neuron_gradients.resize(n_neurons);
    neuron_outputs_per_layer.resize(n_layers-1);
    neuron_gradients_per_layer.resize(n_layers-1);
    for (int i=0,k=0;i<n_layers-1;k+=layer_sizes[i+1],i++)
    {
        neuron_outputs_per_layer[i].resize(layer_sizes[i+1]);
        neuron_gradients_per_layer[i] = neuron_gradients.subVec(k,layer_sizes[i+1]);
    }

}

// ### Nothing to add here, simply calls build_
void NatGradNNet::build()
{
    inherited::build();
    build_();
}


void NatGradNNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("NatGradNNet::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int NatGradNNet::outputsize() const
{
    return noutputs;
}

void NatGradNNet::forget()
{
    //! (Re-)initialize the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!)
    /*!
      A typical forget() method should do the following:
      - call inherited::forget() to initialize its random number generator
        with the 'seed' option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */
    inherited::forget();
}

void NatGradNNet::train()
{
    // The role of the train method is to bring the learner up to
    // stage==nstages, updating train_stats with training costs measured
    // on-line in the process.

    /* TYPICAL CODE:

    static Vec input;  // static so we don't reallocate memory each time...
    static Vec target; // (but be careful that static means shared!)
    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()
    real weight;

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    while(stage<nstages)
    {
        // clear statistics of previous epoch
        train_stats->forget();

        //... train for 1 stage, and update train_stats,
        // using train_set->getExample(input, target, weight)
        // and train_stats->update(train_costs)

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }
    */
}


void NatGradNNet::computeOutput(const Vec& input, Vec& output) const
{
    // Compute the output from the input.
    // int nout = outputsize();
    // output.resize(nout);
    // ...
}

void NatGradNNet::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output.
// ...
}

TVec<string> NatGradNNet::getTestCostNames() const
{
    TVec<string> costs;
    if (output_type=="NLL")
    {
        costs.resize(2);
        costs[0]="NLL";
        costs[1]="class_error";
    }
    else if (output_type=="MSE")
    {
        costs.resize(1);
        costs[0]="MSE";
    }
    return costs;
}

TVec<string> NatGradNNet::getTrainCostNames() const
{
    return getTestCostNames();
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
