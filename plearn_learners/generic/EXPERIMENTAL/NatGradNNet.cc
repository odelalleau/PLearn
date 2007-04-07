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
      params_averaging_coeff(1.0),
      params_averaging_freq(5),
      init_lrate(0.01),
      lrate_decay(0),
      output_layer_lrate_scale(1),
      minibatch_size(1),
      output_type("NLL"),
      verbosity(0),
      n_layers(-1),
      cumulative_training_time(0)
{
    random_gen = new PRandom();
}

void NatGradNNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "noutputs", &NatGradNNet::noutputs,
                  OptionBase::buildoption,
                  "Number of outputs of the neural network, which can be derived from  output_type and targetsize_\n");

    declareOption(ol, "verbosity", &NatGradNNet::verbosity,
                  OptionBase::buildoption,
                  "Verbosity level\n");

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

    declareOption(ol, "cumulative_training_time", &NatGradNNet::cumulative_training_time,
                  OptionBase::learntoption,
                  "Cumulative training time since age=0, in seconds.\n");

    declareOption(ol, "layer_params", &NatGradNNet::layer_params,
                  OptionBase::learntoption,
                  "Training parameters for each layer, organized as follows: layer_params[i] \n"
                  "is a matrix of dimension layer_sizes[i+1] x (layer_sizes[i]+1)\n"
                  "containing the neuron biases in its first column.\n");

    declareOption(ol, "layer_mparams", &NatGradNNet::layer_mparams,
                  OptionBase::learntoption,
                  "Test parameters for each layer, organized like layer_params.\n"
                  "This is a moving average of layer_params, computed with\n"
                  "coefficient params_averaging_coeff. Thus the mparams are\n"
                  "a smoothed version of the params, and they are used only\n"
                  "during testing.\n");

    declareOption(ol, "params_averaging_coeff", &NatGradNNet::params_averaging_coeff,
                  OptionBase::buildoption,
                  "Coefficient used to control how fast we forget old parameters\n"
                  "in the moving average performed as follows:\n"
                  "mparams <-- (1-params_averaging_coeff)mparams + params_averaging_coeff*params\n");

    declareOption(ol, "params_averaging_freq", &NatGradNNet::params_averaging_freq,
                  OptionBase::buildoption,
                  "How often (in terms of number of minibatches, i.e. weight updates)\n"
                  "do we perform the moving average update calculation\n"
                  "mparams <-- (1-params_averaging_coeff)mparams + params_averaging_coeff*params\n");

    declareOption(ol, "init_lrate", &NatGradNNet::init_lrate,
                  OptionBase::buildoption,
                  "Initial learning rate\n");

    declareOption(ol, "lrate_decay", &NatGradNNet::lrate_decay,
                  OptionBase::buildoption,
                  "Learning rate decay factor\n");

    declareOption(ol, "output_layer_lrate_scale", &NatGradNNet::output_layer_lrate_scale,
                  OptionBase::buildoption,
                  "Scaling factor of the learning rate for the output layer. Values less than 1"
                  "mean that the output layer parameters have a smaller learning rate than the others.\n");

    declareOption(ol, "minibatch_size", &NatGradNNet::minibatch_size,
                  OptionBase::buildoption,
                  "Update the parameters only so often (number of examples).\n");

    declareOption(ol, "neurons_natgrad_template", &NatGradNNet::neurons_natgrad_template,
                  OptionBase::buildoption,
                  "Optional template NatGradEstimator for the neurons gradient.\n"
                  "If not provided, then the natural gradient correction\n"
                  "on the neurons gradient is not performed.\n");

    declareOption(ol, "neurons_natgrad_per_layer", 
                  &NatGradNNet::neurons_natgrad_per_layer,
                  OptionBase::learntoption,
                  "Vector of NatGradEstimator objects for the gradient on the neurons of each layer.\n"
                  "They are copies of the neuron_natgrad_template provided by the user.\n");

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
    if (!train_set)
        return;
    inputsize_ = train_set->inputsize();
    if (output_type=="MSE")
    {
        if (noutputs<0) noutputs = targetsize_;
        else PLASSERT_MSG(noutputs==targetsize_,"NatGradNNet: noutputs should be -1 or match data's targetsize");
    }
    else if (output_type=="NLL")
    {
        if (noutputs<0)
            PLERROR("NatGradNNet: if output_type=NLL (classification), one \n"
                    "should provide noutputs = number of classes, or possibly\n"
                    "1 when 2 classes\n");
    }
    else PLERROR("NatGradNNet: output_type should be NLL or MSE\n");

    
    while (hidden_layer_sizes[hidden_layer_sizes.length()-1]==0)
        hidden_layer_sizes.resize(hidden_layer_sizes.length()-1);
    n_layers = hidden_layer_sizes.length()+2;
    layer_sizes.resize(n_layers);
    layer_sizes.subVec(1,n_layers-2) << hidden_layer_sizes;
    layer_sizes[0]=inputsize_;
    layer_sizes[n_layers-1]=noutputs;
    layer_params.resize(n_layers-1);
    layer_mparams.resize(n_layers-1);
    layer_params_delta.resize(n_layers-1);
    layer_params_gradient.resize(n_layers-1);
    biases.resize(n_layers-1);
    weights.resize(n_layers-1);
    int n_neurons=0;
    int n_params=0;
    for (int i=0;i<n_layers-1;i++)
    {
        n_neurons+=layer_sizes[i+1];
        n_params+=layer_sizes[i+1]*(1+layer_sizes[i]);
    }
    all_params.resize(n_params);
    all_mparams.resize(n_params);
    all_params_gradient.resize(n_params);
    all_params_delta.resize(n_params);
    neuron_params.resize(n_neurons);
    neuron_params_delta.resize(n_neurons);
    neuron_params_gradient.resize(n_neurons);
    for (int i=0,k=0,p=0;i<n_layers-1;i++)
    {
        int np=layer_sizes[i+1]*(1+layer_sizes[i]);
        layer_params[i]=all_params.subVec(p,np).toMat(layer_sizes[i+1],layer_sizes[i]+1);
        layer_mparams[i]=all_mparams.subVec(p,np).toMat(layer_sizes[i+1],layer_sizes[i]+1);
        biases[i]=layer_params[i].subMatColumns(0,1);
        weights[i]=layer_params[i].subMatColumns(1,layer_sizes[i]); // weights[0] from layer 0 to layer 1
        layer_params_gradient[i]=all_params_gradient.subVec(p,np).toMat(layer_sizes[i+1],layer_sizes[i]+1);
        layer_params_delta[i]=all_params_delta.subVec(p,np);
        for (int j=0;j<layer_sizes[i+1];j++,k++)
        {
            neuron_params[k]=all_params.subVec(p,1+layer_sizes[i]);
            neuron_params_delta[k]=all_params_delta.subVec(p,1+layer_sizes[i]);
            neuron_params_gradient[k]=all_params_gradient.subVec(p,1+layer_sizes[i]);
            p+=1+layer_sizes[i];
        }
    }
    if (params_natgrad_template)
    {
        params_natgrad_per_neuron.resize(n_neurons);
        for (int i=0;i<n_neurons;i++)
            params_natgrad_per_neuron[i] = PLearn::deepCopy(params_natgrad_template);
    }
    if (neurons_natgrad_template && neurons_natgrad_per_layer.length()==0)
    {
        neurons_natgrad_per_layer.resize(n_layers); // 0 not used
        for (int i=1;i<n_layers;i++) // no need for correcting input layer
            neurons_natgrad_per_layer[i] = PLearn::deepCopy(neurons_natgrad_template);
    }
    neuron_gradients.resize(minibatch_size,n_neurons);
    neuron_outputs_per_layer.resize(n_layers); // layer 0 = input, layer n_layers-1 = output
    neuron_extended_outputs_per_layer.resize(n_layers); // layer 0 = input, layer n_layers-1 = output
    neuron_gradients_per_layer.resize(n_layers); // layer 0 not used
    neuron_extended_outputs_per_layer[0].resize(minibatch_size,1+layer_sizes[0]);
    neuron_outputs_per_layer[0]=neuron_extended_outputs_per_layer[0].subMatColumns(1,layer_sizes[0]);
    neuron_extended_outputs_per_layer[0].column(0).fill(1.0); // for biases
    for (int i=1,k=0;i<n_layers;k+=layer_sizes[i],i++)
    {
        neuron_extended_outputs_per_layer[i].resize(minibatch_size,1+layer_sizes[i]);
        neuron_outputs_per_layer[i]=neuron_extended_outputs_per_layer[i].subMatColumns(1,layer_sizes[i]);
        neuron_extended_outputs_per_layer[i].column(0).fill(1.0); // for biases
        neuron_gradients_per_layer[i] = 
            neuron_gradients.subMatColumns(k,layer_sizes[i]);
    }
    example_weights.resize(minibatch_size);
    TVec<string> train_cost_names = getTrainCostNames() ;
    train_costs.resize(minibatch_size,train_cost_names.length()-2 );

    Profiler::activate();
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

    deepCopyField(hidden_layer_sizes, copies);
    deepCopyField(layer_params, copies);
    deepCopyField(layer_mparams, copies);
    deepCopyField(neurons_natgrad_template, copies);
    deepCopyField(neurons_natgrad_per_layer, copies);
    deepCopyField(params_natgrad_template, copies);
    deepCopyField(params_natgrad_per_neuron, copies);
    deepCopyField(full_natgrad, copies);
    deepCopyField(layer_sizes, copies);
    deepCopyField(targets, copies);
    deepCopyField(example_weights, copies);
    deepCopyField(train_costs, copies);
    deepCopyField(neuron_outputs_per_layer, copies);
    deepCopyField(neuron_extended_outputs_per_layer, copies);
    deepCopyField(all_params, copies);
    deepCopyField(all_mparams, copies);
    deepCopyField(all_params_gradient, copies);
    deepCopyField(layer_params_gradient, copies);
    deepCopyField(neuron_gradients, copies);
    deepCopyField(neuron_gradients_per_layer, copies);
    deepCopyField(all_params_delta, copies);
    deepCopyField(neuron_params, copies);
    deepCopyField(neuron_params_gradient, copies);
    deepCopyField(neuron_params_delta, copies);
    deepCopyField(layer_params_delta, copies);
/*
    deepCopyField(, copies);
*/
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
    inherited::forget();
    for (int i=0;i<n_layers-1;i++)
    {
        real delta = 1/sqrt(real(layer_sizes[i]));
        random_gen->fill_random_uniform(weights[i],-delta,delta);
        biases[i].clear();
    }
    stage = 0;
    cumulative_training_time=0;
    if (params_averaging_coeff!=1.0)
        all_mparams << all_params;
}

void NatGradNNet::train()
{
    if (inputsize_<0)
        build();

    targets.resize(minibatch_size,targetsize());  // the train_set's targetsize()

    if(!train_set)
        PLERROR("In NNet::train, you did not setTrainingSet");
    
    if(!train_stats)
        setTrainStatsCollector(new VecStatsCollector());

    train_costs.fill(MISSING_VALUE) ;

    train_stats->forget();

    PP<ProgressBar> pb;

    Profiler::reset("training");
    Profiler::start("training");
    if( report_progress && stage < nstages )
        pb = new ProgressBar( "Training "+classname(),
                              nstages - stage );

    Vec costs_plus_time(train_costs.width()+2);
    costs_plus_time[train_costs.width()] = MISSING_VALUE;
    costs_plus_time[train_costs.width()+1] = MISSING_VALUE;
    Vec costs = costs_plus_time.subVec(0,train_costs.width());
    int nsamples = train_set->length();

    for( ; stage<nstages; stage++)
    {
        int sample = stage % nsamples;
        int b = stage % minibatch_size;
        Vec input = neuron_outputs_per_layer[0](b);
        Vec target = targets(b);
        Profiler::start("getting_data");
        train_set->getExample(sample, input, target, example_weights[b]);
        Profiler::end("getting_data");
        if (b+1==minibatch_size) // do also special end-case || stage+1==nstages)
        {
            onlineStep(stage, targets, train_costs, example_weights );
            for (int i=0;i<minibatch_size;i++)
            {
                costs << train_costs(b);
                train_stats->update( costs_plus_time );
                
            }
        }
        if (params_averaging_coeff!=1.0 && 
            b==minibatch_size-1 && 
            (stage+1)%(minibatch_size*params_averaging_freq)==0)
            multiplyScaledAdd(all_params, 1-params_averaging_coeff,
                              params_averaging_coeff, all_mparams);
        if( pb )
            pb->update( stage + 1 );
    }
    Profiler::end("training");
    if (verbosity>0)
        Profiler::report(cout);
    const Profiler::Stats& stats = Profiler::getStats("training");
    costs.fill(MISSING_VALUE);
    real cpu_time = (stats.user_duration+stats.system_duration)/60.0;
    cumulative_training_time += cpu_time;
    costs_plus_time[train_costs.width()] = cpu_time;
    costs_plus_time[train_costs.width()+1] = cumulative_training_time;
    train_stats->update( costs_plus_time );
    train_stats->finalize(); // finalize statistics for this epoch
}

void NatGradNNet::onlineStep(int t, const Mat& targets,
                             Mat& train_costs, Vec example_weights)
{
    // mean gradient over minibatch_size examples has less variance, can afford larger learning rate
    real lrate = sqrt(real(minibatch_size))*init_lrate/(1 + t*lrate_decay);
    PLASSERT(targets.length()==minibatch_size && train_costs.length()==minibatch_size && example_weights.length()==minibatch_size);
    fpropNet(minibatch_size,true);
    fbpropLoss(neuron_outputs_per_layer[n_layers-1],targets,example_weights,train_costs);
    for (int i=n_layers-1;i>0;i--)
    {
        // here neuron_gradients_per_layer[i] contains the gradient on activations (weighted sums)
        //      (minibatch_size x layer_size[i])

        Mat previous_neurons_gradient = neuron_gradients_per_layer[i-1];
        Mat previous_neurons_output = neuron_outputs_per_layer[i-1];
        real layer_lrate_factor = (i==n_layers-1)?output_layer_lrate_scale:1;
        // optionally correct the gradient on neurons using their covariance
        if (neurons_natgrad_template && neurons_natgrad_per_layer[i])
        {
            static Vec tmp;
            tmp.resize(layer_sizes[i]);
            for (int k=0;k<minibatch_size;k++)
            {
                Vec g_k = neuron_gradients_per_layer[i](k);
                (*neurons_natgrad_per_layer[i])(t-minibatch_size+1+k,g_k,tmp);
                g_k << tmp;
            }
        }
        if (i>1) // compute gradient on previous layer
        {
            // propagate gradients
            productScaleAcc(previous_neurons_gradient,neuron_gradients_per_layer[i],false,
                            weights[i-1],false,1,0);
            // propagate through tanh non-linearity
            for (int j=0;j<previous_neurons_gradient.length();j++)
            {
                real* grad = previous_neurons_gradient[j];
                real* out = previous_neurons_output[j];
                for (int k=0;k<previous_neurons_gradient.width();k++,out++)
                    grad[k] *= (1 - *out * *out); // gradient through tanh derivative
            }
        }
        // compute gradient on parameters, possibly update them
        if (full_natgrad || params_natgrad_template) 
        {
            productScaleAcc(layer_params_gradient[i-1],neuron_gradients_per_layer[i],true,
                            neuron_extended_outputs_per_layer[i-1],false,1,0);
            layer_params_gradient[i-1] *= 1.0/minibatch_size; // use the MEAN gradient
        } else // just regular stochastic gradient
            // compute gradient on weights and update them in one go (more efficient)
            productScaleAcc(layer_params[i-1],neuron_gradients_per_layer[i],true,
                            neuron_extended_outputs_per_layer[i-1],false,
                            -layer_lrate_factor*lrate/minibatch_size,1); // mean gradient, has less variance, can afford larger learning rate
    }
    if (full_natgrad)
    {
        (*full_natgrad)(t/minibatch_size,all_params_gradient,all_params_delta); // compute update direction by natural gradient
        if (output_layer_lrate_scale!=1.0)
            layer_params_delta[n_layers-2] *= output_layer_lrate_scale; // scale output layer's learning rate 
        multiplyAcc(all_params,all_params_delta,-lrate); // update
    } else if (params_natgrad_template)
    {
        for (int i=0;i<params_natgrad_per_neuron.length();i++)
        {
            NatGradEstimator& neuron_natgrad = *(params_natgrad_per_neuron[i]);
            neuron_natgrad(t/minibatch_size,neuron_params_gradient[i],neuron_params_delta[i]); // compute update direction by natural gradient
        }
        if (output_layer_lrate_scale!=1.0)
            layer_params_delta[n_layers-2] *= output_layer_lrate_scale; // scale output layer's learning rate 
        multiplyAcc(all_params,all_params_delta,-lrate); // update
    }
}

void NatGradNNet::computeOutput(const Vec& input, Vec& output) const
{
    neuron_outputs_per_layer[0](0) << input;
    fpropNet(1,false);
    output << neuron_outputs_per_layer[n_layers-1](0);
}

//! compute (pre-final-non-linearity) network top-layer output given input
void NatGradNNet::fpropNet(int n_examples, bool during_training) const
{
    PLASSERT_MSG(n_examples<=minibatch_size,"NatGradNNet::fpropNet: nb input vectors treated should be <= minibatch_size\n");
    for (int i=0;i<n_layers-1;i++)
    {
        Mat prev_layer = neuron_extended_outputs_per_layer[i];
        Mat next_layer = neuron_outputs_per_layer[i+1];
        if (n_examples!=minibatch_size)
        {
            prev_layer = prev_layer.subMatRows(0,n_examples);
            next_layer = next_layer.subMatRows(0,n_examples);
        }
        // try to use BLAS for the expensive operation
        productScaleAcc(next_layer, prev_layer, false, 
                        during_training?layer_params[i]:layer_mparams[i], 
                        true, 1, 0);
        // compute layer's output non-linearity
        if (i+1<n_layers-1)
            for (int k=0;k<n_examples;k++)
            {
                Vec L=next_layer(k);
                compute_tanh(L,L);
            }
        else if (output_type=="NLL")
            for (int k=0;k<n_examples;k++)
            {
                Vec L=next_layer(k);
                log_softmax(L,L);
            }
    }
}

//! compute train costs given the (pre-final-non-linearity) network top-layer output
void NatGradNNet::fbpropLoss(const Mat& output, const Mat& target, const Vec& example_weight, Mat& costs) const
{
    int n_examples = output.length();
    Mat out_grad = neuron_gradients_per_layer[n_layers-1];
    if (n_examples!=minibatch_size)
        out_grad = out_grad.subMatRows(0,n_examples);
    if (output_type=="NLL")
    {
        for (int i=0;i<n_examples;i++)
        {
            int target_class = int(round(target(i,0)));
            Vec outp = output(i);
            Vec grad = out_grad(i);
            exp(outp,grad); // map log-prob to prob
            costs(i,0) = -outp[target_class];
            costs(i,1) = (target_class == argmax(outp))?0:1;
            grad[target_class]-=1;
            if (example_weight[i]!=1.0)
                costs(i,0) *= example_weight[i];
        }
    }
    else // if (output_type=="MSE")
    {
        substract(output,target,out_grad);
        for (int i=0;i<n_examples;i++)
        {
            costs(i,0) = pownorm(out_grad(i));
            if (example_weight[i]!=1.0)
            {
                out_grad(i) *= example_weight[i];
                costs(i,0) *= example_weight[i];
            }
        }
    }
}

void NatGradNNet::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    Vec w(1);
    w[0]=1;
    Mat outputM = output.toMat(1,output.length());
    Mat targetM = target.toMat(1,output.length());
    Mat costsM = costs.toMat(1,costs.length());
    fbpropLoss(outputM,targetM,w,costsM);
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
    TVec<string> costs = getTestCostNames();
    costs.append("train_seconds");
    costs.append("cum_train_seconds");
    return costs;
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
