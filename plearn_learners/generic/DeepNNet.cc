// -*- C++ -*-

// DeepNNet.cc
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
 * $Id$ 
 ******************************************************* */

// Authors: Yoshua Bengio

/*! \file DeepNNet.cc */

#include "DeepNNet.h"
#include <time.h>                 //!< For clock().
#include <plearn/base/tostring.h>
#include <plearn/math/random.h>
#include <plearn/math/pl_math.h>

namespace PLearn {
using namespace std;

DeepNNet::DeepNNet() 
/* ### Initialize all fields to their default value here */
    :   training_time(0),
        n_layers(3),
        n_outputs(1),
        default_n_units_per_hidden_layer(10),
        L1_regularizer(1e-5),
        initial_learning_rate(1e-4),
        learning_rate_decay(1e-6),
        layerwise_learning_rate_adaptation(0),
        normalize_per_unit(0),
        normalize_percentage(0),
        normalize_activations(0),
        output_cost("mse"),
        add_connections(true),
        remove_connections(true),
        initial_sparsity(0.9),
        connections_adaptation_frequency(0),
        init_scale(1)
{
}

PLEARN_IMPLEMENT_OBJECT(DeepNNet, 
                        "Deep multi-layer neural networks with sparse adaptive connections", 
                        "This feedforward neural network can have many layers, but its weight\n"
                        "matrices are sparse and can be optionally adapted (adding new connections\n"
                        "where that would create the largest gradient).");

void DeepNNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "n_layers", &DeepNNet::n_layers, OptionBase::buildoption,
                  "Number of layers, including the output but not input layer");

    declareOption(ol, "n_outputs", &DeepNNet::n_outputs, OptionBase::buildoption,
                  "Number of units of output layer");

    declareOption(ol, "default_n_units_per_hidden_layer", &DeepNNet::default_n_units_per_hidden_layer, 
                  OptionBase::buildoption, "If n_units_per_layer is not specified, it is given by this value for all hidden layers");

    declareOption(ol, "n_units_per_layer", &DeepNNet::n_units_per_layer, OptionBase::buildoption,
                  "Number of units per layer, including the output but not input layer.\n"
                  "The last (output) layer number of units is the output size.");

    declareOption(ol, "L1_regularizer", &DeepNNet::L1_regularizer, OptionBase::buildoption,
                  "amount of penalty on sum_{l,i,j} |weights[l][i][j]|");

    declareOption(ol, "initial_learning_rate", &DeepNNet::initial_learning_rate, OptionBase::buildoption,
                  "learning_rate = initial_learning_rate/(1 + iteration*learning_rate_decay)\n"
                  "where iteration is incremented after each example is presented");

    declareOption(ol, "learning_rate_decay", &DeepNNet::learning_rate_decay, OptionBase::buildoption,
                  "see the comment for initial_learning_rate.");

    declareOption(ol, "layerwise_learning_rate_adaptation", &DeepNNet::layerwise_learning_rate_adaptation,
                  OptionBase::buildoption, "if 0 use stochastic gradient as usual, otherwise correct the\n"
                  "learning rates layerwise by multiplying by the ratio of average gradient norm\n"
                  "of the top layer by the i-th layer, to the power layerwise_learning_rate_adaptation.");

    declareOption(ol, "normalize_per_unit", &DeepNNet::normalize_per_unit,
                  OptionBase::buildoption, "Try balancing the norm of the weight gradient vectors per unit, rather than per weight\n");

    declareOption(ol, "normalize_percentage", &DeepNNet::normalize_percentage,
                  OptionBase::buildoption, "Try balancing the ratio the gradient to the weight squared, rather than the norm of the gradient\n");

    declareOption(ol, "normalize_activations", &DeepNNet::normalize_activations,
                  OptionBase::buildoption, "Try balancing the norm of the gradient on the activations, per layer\n");

    declareOption(ol, "output_cost", &DeepNNet::output_cost, OptionBase::buildoption,
                  "String-valued option specifies output non-linearity and cost:\n"
                  "  'mse': mean squared error for regression with linear outputs\n"
                  "  'NLL': negative log-likelihood of P(class|input) with softmax outputs");

    declareOption(ol, "add_connections", &DeepNNet::add_connections, OptionBase::buildoption,
                  "whether to add connections when the potential connections average"
                  "gradient becomes larger in magnitude than that of existing connections");

    declareOption(ol, "remove_connections", &DeepNNet::remove_connections, OptionBase::buildoption,
                  "whether to remove connections when their weight becomes too small");

    declareOption(ol, "initial_sparsity", &DeepNNet::initial_sparsity, OptionBase::buildoption,
                  "initial fraction of weights that are set to 0.");

    declareOption(ol, "connections_adaptation_frequency", &DeepNNet::connections_adaptation_frequency, 
                  OptionBase::buildoption, "after how many examples do we try to adapt connections?\n"
                  "if set to 0, this is interpreted as the training set size.");

    declareOption(ol, "init_scale", &DeepNNet::init_scale, OptionBase::buildoption,
                  "scaling factor of random initial weights range.");

    declareOption(ol, "sources", &DeepNNet::sources, OptionBase::learntoption, 
                  "The learned connectivity matrix at each layer\n"
                  "(source[l][i] = vector of indices of inputs of neuron i at layer l");

    declareOption(ol, "weights", &DeepNNet::weights, OptionBase::learntoption, 
                  "The learned weights at each layer\n"
                  "(weights[l][i] = vector of weights of inputs of neuron i at layer l");

    declareOption(ol, "biases", &DeepNNet::biases, OptionBase::learntoption, 
                  "The learned biases at each layer\n"
                  "(biases[l] = vector of biases of neurons at layer l");

    declareOption(ol, "layerwise_lr_factor", &DeepNNet::layerwise_lr_factor, OptionBase::learntoption, 
                  "The multiplicative factor for the learning rate at each layer");

    declareOption(ol, "layerwise_gradient_norm_ma", &DeepNNet::layerwise_gradient_norm_ma, OptionBase::learntoption, 
                  "The (moving) average of squared gradients at each layer");

    declareOption(ol, "training_time", &DeepNNet::training_time, OptionBase::learntoption, 
                  "The time spent during training (in seconds)");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DeepNNet::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.

    // these would be -1 if a train_set has not be set already
    if (inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
    {
        if (add_connections)
        {
            avg_weight_gradients.resize(n_layers);
            for (int l=0;l<n_layers;l++)
                avg_weight_gradients[l].resize(n_units_per_layer[l+1],n_units_per_layer[l]);
        }

        if (layerwise_learning_rate_adaptation>0)
        {
            layerwise_lr_factor.resize(n_layers);
            layerwise_gradient_norm_ma.resize(n_layers);
            layerwise_gradient_norm.resize(n_layers);
            n_weights_of_layer.resize(n_layers);
            layerwise_lr_factor.fill(1.0);
            layerwise_gradient_norm_ma.clear();
        }
        bool do_initialize = false;

        if (sources.length() != n_layers) // in case we are called after loading the object we don't need to do this:
        {
            if (n_units_per_layer.length()==0)
            {
                n_units_per_layer.resize(n_layers);
                for (int l=0;l<n_layers-1;l++)
                    n_units_per_layer[l] = default_n_units_per_hidden_layer;
                n_units_per_layer[n_layers-1] = n_outputs;
            }
            sources.resize(n_layers);
            weights.resize(n_layers);
            biases.resize(n_layers);
            for (int l=0;l<n_layers;l++)
            {
                sources[l].resize(n_units_per_layer[l]);
                weights[l].resize(n_units_per_layer[l]);
                biases[l].resize(n_units_per_layer[l]);
                int n_previous = (l==0)? int((1-initial_sparsity)*inputsize_) :
                    int((1-initial_sparsity)*n_units_per_layer[l-1]);
                for (int i=0;i<n_units_per_layer[l];i++)
                {
                    sources[l][i].resize(n_previous);
                    weights[l][i].resize(n_previous);
                }
            }
            do_initialize = true;
        }
        activations.resize(n_layers+1);
        activations[0].resize(inputsize_);
        activations.resize(n_layers+1);
        activations_gradients.resize(n_layers);
        for (int l=0;l<n_layers;l++)
        {
            activations[l+1].resize(n_units_per_layer[l]);
            activations_gradients[l].resize(n_units_per_layer[l]);
        }
        if (do_initialize)
            initializeParams();
    }

}

void DeepNNet::initializeParams(bool set_seed)
{
    if (set_seed) {
        if (seed_>=0)
            manual_seed(seed_);
        else
            PLearn::seed();
    }
    for (int l=0;l<n_layers;l++)
    {
        biases[l].clear();
        int n_previous = (l==0)?inputsize_:n_units_per_layer[l-1];
        int n_next = n_units_per_layer[l];
        if (initial_sparsity>0)
        {
            // first assign randomly some connections to each of the next layer unit
            int n_in = 1+int(0.66 * (1-initial_sparsity) * n_previous);
            if (n_in>n_previous) n_in=n_previous;
            int n_out = 1+int(0.66 * (1-initial_sparsity) * n_next);
            if (n_out>n_next) n_out=n_next;
            for (int i=0;i<n_next;i++)
            {
                sources[l][i].resize(n_in);
                random_subset_indices(sources[l][i],n_previous);
            }
            // then assign randomly some connections from each of the previous layer unit
            TVec<int> dest(n_out);
            for (int j=0;j<n_previous;j++)
            {
                random_subset_indices(dest,n_next);
                for (int k=0;k<n_out;k++)
                    if (!sources[l][dest[k]].contains(j))
                        sources[l][dest[k]].append(j);
            }
            for (int i=0;i<n_next;i++)
            {
                int n_in = sources[l][i].length();
                real delta = init_scale/sqrt((real)n_in);
                weights[l][i].resize(n_in);
                if (n_layers==1)
                    weights[l][i].fill(0);
                else
                    fill_random_uniform(weights[l][i],-delta,delta);
            }
        }
        else // fully connected, mostly for debugging
        {
            // real delta = 1.0/sqrt((real)n_previous);
            real delta = init_scale/n_previous;
            for (int i=0;i<n_next;i++)
            {
                sources[l][i].resize(n_previous);
                weights[l][i].resize(n_previous);
                for (int j=0;j<n_previous;j++)
                    sources[l][i][j] = j;
                fill_random_uniform(weights[l][i],-delta,delta);
            }
        }
        if (layerwise_learning_rate_adaptation>0)
        {
            n_weights_of_layer[l]=0;
            for (int i=0;i<n_next;i++)
                n_weights_of_layer[l] += sources[l][i].length();
        }
    }
}

// ### Nothing to add here, simply calls build_
void DeepNNet::build()
{
    inherited::build();
    build_();
}


void DeepNNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(sources, copies);
    deepCopyField(weights, copies);
    deepCopyField(biases, copies);
    deepCopyField(layerwise_lr_factor, copies);
    deepCopyField(activations, copies);
    deepCopyField(activations_gradients, copies);
    deepCopyField(avg_weight_gradients, copies);
    deepCopyField(layerwise_gradient_norm_ma, copies);
    deepCopyField(layerwise_gradient_norm, copies);
    deepCopyField(n_weights_of_layer, copies);
    deepCopyField(n_units_per_layer, copies);
    deepCopyField(output_cost, copies);
}


int DeepNNet::outputsize() const
{
    return n_units_per_layer[n_units_per_layer.length()-1];
}

void DeepNNet::forget()
{
    if (train_set) initializeParams();
    stage = 0;
    training_time = 0;
}

void DeepNNet::fprop() const
{
    for (int l=0;l<n_layers;l++)
    {
        int n_u = n_units_per_layer[l];
        Vec biases_l = biases[l];
        Vec previous_layer = activations[l];
        Vec next_layer = activations[l+1];
        for (int i=0;i<n_u;i++)
        {
            TVec<int> sources_i = sources[l][i];
            Vec weights_i = weights[l][i];
            int n_sources = sources_i.length();
            real s=biases_l[i];
            for (int k=0;k<n_sources;k++)
                s += previous_layer[sources_i[k]] * weights_i[k];
            if (l+1<n_layers)
                next_layer[i] = tanh(s);
            else next_layer[i] = s;
        }
    }
    if (output_cost == "NLL")
    {
        Vec output = activations[n_layers];
        softmax(output,output);
    }
}
    
void DeepNNet::train()
{
    // The role of the train method is to bring the learner up to stage==nstages,
    // updating train_stats with training costs measured on-line in the process.
    clock_t start_train = clock();
    static Vec target;
    static Vec train_costs;
    target.resize(targetsize());
    if (output_cost=="mse")
        train_costs.resize(1);
    else
        train_costs.resize(2);
    real example_weight;
  
    if(!train_stats)  // make a default stats collector, in case there's none
        train_stats = new VecStatsCollector();

    if(nstages<stage) // asking to revert to a previous stage!
        forget();  // reset the learner to stage=0
    int initial_stage = stage;

    PP<ProgressBar> pb;
    if (report_progress) {
        pb = new ProgressBar("Training " + classname() + " from stage " + tostring(stage) + " to " + tostring(nstages), nstages - stage);
    }

    int n_examples = train_set->length();

    int t=stage*n_examples;

    while(stage<nstages)
    {
        // clear statistics of previous epoch
        train_stats->forget();

        if (layerwise_learning_rate_adaptation>0)
            layerwise_gradient_norm.clear();

        // train for 1 stage, and update train_stats,
        for (int ex=0;ex<n_examples;ex++, t++)
        {
            // get the (input,target) pair
            train_set->getExample(ex, activations[0], target, example_weight);

            // fprop
            fprop();

            // compute cost

            if (output_cost == "mse")
            {
                substract(activations[n_layers],target,activations_gradients[n_layers-1]);
                train_costs[0] = example_weight*pownorm(activations_gradients[n_layers-1]);
                activations_gradients[n_layers-1] *= 2*example_weight; // 2 from the square
            }
            else if (output_cost == "NLL")
            {
                Vec output = activations[n_layers];
                int target_class = int(target[0]);
                real p_target = output[target_class];
                train_costs[0] = example_weight*(-safelog(p_target));
                int recognized_class = argmax(output);
                train_costs[1] = example_weight*(recognized_class!=target_class);
                activations_gradients[n_layers-1] << output;
                activations_gradients[n_layers-1][target_class] -= 1;
            }
            else PLERROR("DeepNNet: unknown output_cost = %s, expected mse or NLL",output_cost.c_str());

            // bprop + update + track avg gradient

            learning_rate = initial_learning_rate / (1 + t*learning_rate_decay);

            if (layerwise_learning_rate_adaptation>0 && normalize_activations)
            {
                int l=n_layers-1;
                Vec ag = activations_gradients[n_layers-1];
                real& gn = layerwise_gradient_norm[l];
                for (int i=0;i<n_outputs;i++)
                {
                    real g = ag[i];
                    gn += g*g;
                }
            }
            for (int l=n_layers-1;l>=0;l--)
            {
                Vec biases_l = biases[l];
                Vec next_layer = activations[l+1];
                Vec previous_layer = activations[l];
                int n_next = next_layer.length();
                int n_previous = previous_layer.length();
                Vec next_layer_gradient = activations_gradients[l];
                Vec previous_layer_gradient;
                if (l>0) 
                {
                    previous_layer_gradient = activations_gradients[l-1];
                    previous_layer_gradient.clear();
                }
                real layer_learning_rate = learning_rate;
                if (layerwise_learning_rate_adaptation>0)
                    layer_learning_rate *= layerwise_lr_factor[l];

                for (int i=0;i<n_next;i++)
                {
                    TVec<int> sources_i = sources[l][i];
                    Vec weights_i = weights[l][i];
                    int n_sources = sources_i.length();
                    real g_i = next_layer_gradient[i];
                    biases_l[i] -= learning_rate * g_i;
                    for (int k=0;k<n_sources;k++)
                    {
                        real w = weights_i[k];
                        int j=sources_i[k];
                        real sign_w = (w>0)?1:-1;
                        real grad = g_i * previous_layer[j];
                        weights_i[k] -= layer_learning_rate * (grad + L1_regularizer*sign_w);
                        if (l>0)   // THE IF COULD BE FACTORED OUT (more ugly but more efficient)
                            previous_layer_gradient[j] += g_i * w;
                        if (layerwise_learning_rate_adaptation>0 && !normalize_activations)  // THE IF COULD BE FACTORED OUT (more ugly but more efficient)
                        {
                            if (normalize_percentage)
                                layerwise_gradient_norm[l] += grad*grad/(w*w);
                            else
                                layerwise_gradient_norm[l] += grad*grad;
                        }
                    }
                }
                if (l>0)
                    for (int j=0;j<n_previous;j++) 
                    {
                        real a = previous_layer[j];
                        real& g = previous_layer_gradient[j];
                        g *= (1 - a*a);
                        if (layerwise_learning_rate_adaptation>0 && normalize_activations)
                            layerwise_gradient_norm[l-1] += g*g;
                    }
            }
            if (layerwise_learning_rate_adaptation>0)
            {
                for (int l=0;l<n_layers;l++)
                {
                    if (normalize_activations || normalize_per_unit)
                        layerwise_gradient_norm[l] /= n_units_per_layer[l]; // maybe we want larger weights, hence larger gradients where there are less terms in the sum, i.e. less weights
                    else // normalize per weight
                        layerwise_gradient_norm[l] /= n_weights_of_layer[l];
                    layerwise_gradient_norm_ma[l] = (1-learning_rate) * layerwise_gradient_norm_ma[l] + learning_rate * layerwise_gradient_norm[l];
                    layerwise_lr_factor[l] = pow(layerwise_gradient_norm_ma[n_layers-1]/layerwise_gradient_norm_ma[l],
                                                 0.5*layerwise_learning_rate_adaptation);
                }
            }
            train_stats->update(train_costs);
        }

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
        if (report_progress)
            pb->update(stage - initial_stage);
    }
    training_time += real(clock() - start_train) / real(CLOCKS_PER_SEC);
}


void DeepNNet::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(outputsize());
    activations[0] << input;
    fprop();
    output << activations[n_layers];
}    

void DeepNNet::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const
{
    costs.resize(0);
    if (output_cost == "mse")
    {
        costs.append(powdistance(output,target));
    }
    else if (output_cost == "NLL")
    {
        int target_class = int(target[0]);
        real p_target = output[target_class];
        costs.append(-safelog(p_target));
        int recognized_class = argmax(output);
        costs.append(recognized_class!=target_class);
    }
    else PLERROR("DeepNNet: unknown output_cost = %s, expected mse or NLL",output_cost.c_str());
    costs.append(real(nstages));
    costs.append(training_time);
}                                

TVec<string> DeepNNet::getTestCostNames() const
{
    TVec<string> names;
    if (output_cost == "mse")
    {
        names.append("mse");
    } else // "NLL"
    {
        names.append("NLL");
        names.append("class_error");
    }
    names.append("nstages");
    names.append("training_time");
    return names;
}

TVec<string> DeepNNet::getTrainCostNames() const
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
