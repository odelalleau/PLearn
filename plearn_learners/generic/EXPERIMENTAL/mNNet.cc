// -*- C++ -*-

// mNNet.cc
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

// Authors: Yoshua Bengio, PAM

/*! \file mNNet.cc */

#include "mNNet.h"
//#include <plearn/math/pl_erf.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    mNNet,
    "Multi-layer neural network based on matrix-matrix multiplications",
    "This is a LEAN neural network. No bells, no whistles.\n"
    );

mNNet::mNNet()
    : noutputs(-1),
      init_lrate(0.0),
      lrate_decay(0.0),
      minibatch_size(1),
      output_type("NLL"),
      output_layer_L1_penalty_factor(0.0),
      n_layers(-1),
      cumulative_training_time(0.0)
{
    random_gen = new PRandom();
}

void mNNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "noutputs", &mNNet::noutputs,
                  OptionBase::buildoption,
                  "Number of outputs of the neural network, which can be derived from output_type and targetsize_\n");

    declareOption(ol, "hidden_layer_sizes", &mNNet::hidden_layer_sizes,
                  OptionBase::buildoption,
                  "Defines the architecture of the multi-layer neural network by\n"
                  "specifying the number of hidden units in each hidden layer.\n");

    declareOption(ol, "init_lrate", &mNNet::init_lrate,
                  OptionBase::buildoption,
                  "Initial learning rate\n");

    declareOption(ol, "lrate_decay", &mNNet::lrate_decay,
                  OptionBase::buildoption,
                  "Learning rate decay factor\n");

    // TODO Why this dependance on test_minibatch_size?
    declareOption(ol, "minibatch_size", &mNNet::minibatch_size,
                  OptionBase::buildoption,
                  "Update the parameters only so often (number of examples).\n"
                  "Must be greater or equal to test_minibatch_size\n");

    declareOption(ol, "output_type", 
                  &mNNet::output_type,
                  OptionBase::buildoption,
                  "type of output cost: 'cross_entropy' for binary classification,\n"
                  "'NLL' for classification problems, or 'MSE' for regression.\n");

    declareOption(ol, "output_layer_L1_penalty_factor",
                  &mNNet::output_layer_L1_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L1 regularization term, i.e.\n"
                  "minimize L1_penalty_factor * sum_{ij} |weights(i,j)| during training.\n"
                  "Gets multiplied by the learning rate. Only on output layer!!");

    declareOption(ol, "n_layers", &mNNet::n_layers,
                  OptionBase::learntoption,
                  "Number of layers of weights plus 1 (ie. 3 for a neural net with one hidden layer).\n"
                  "Needs not be specified explicitly (derived from hidden_layer_sizes).\n");

    declareOption(ol, "layer_sizes", &mNNet::layer_sizes,
                  OptionBase::learntoption,
                  "Derived from hidden_layer_sizes, inputsize_ and noutputs\n");

    declareOption(ol, "layer_params", &mNNet::layer_params,
                  OptionBase::learntoption,
                  "Parameters used while training, for each layer, organized as follows: layer_params[i] \n"
                  "is a matrix of dimension layer_sizes[i+1] x (layer_sizes[i]+1)\n"
                  "containing the neuron biases in its first column.\n");

    declareOption(ol, "cumulative_training_time", &mNNet::cumulative_training_time,
                  OptionBase::learntoption,
                  "Cumulative training time since age=0, in seconds.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

// TODO - reloading an object will not work! layer_params will juste get lost.
void mNNet::build_()
{
    // *** Sanity checks ***

    if (!train_set)
        return;
    if (output_type=="MSE")
    {
        if (noutputs<0) noutputs = targetsize_;
        else PLASSERT_MSG(noutputs==targetsize_,"mNNet: noutputs should be -1 or match data's targetsize");
    }
    else if (output_type=="NLL")
    {
        // TODO add a check on noutput's value
        if (noutputs<0)
            PLERROR("mNNet: if output_type=NLL (classification), one \n"
                    "should provide noutputs = number of classes, or possibly\n"
                    "1 when 2 classes\n");
    }
    else if (output_type=="cross_entropy")
    {
        if(noutputs!=1)
            PLERROR("mNNet: if output_type=cross_entropy, then \n"
                    "noutputs should be 1.\n");
    }
    else PLERROR("mNNet: output_type should be cross_entropy, NLL or MSE\n");

    if( output_layer_L1_penalty_factor < 0. )
        PLWARNING("mNNet::build_ - output_layer_L1_penalty_factor is negative!\n");

    // *** Determine topology ***
    inputsize_ = train_set->inputsize();
    while (hidden_layer_sizes.length()>0 && hidden_layer_sizes[hidden_layer_sizes.length()-1]==0)
        hidden_layer_sizes.resize(hidden_layer_sizes.length()-1);
    n_layers = hidden_layer_sizes.length()+2; 
    layer_sizes.resize(n_layers);
    layer_sizes.subVec(1,n_layers-2) << hidden_layer_sizes;
    layer_sizes[0]=inputsize_;
    layer_sizes[n_layers-1]=noutputs;

    // *** Allocate memory for params and gradients ***
    int n_params=0;
    int n_neurons=0;
    for (int i=0;i<n_layers-1;i++)    {
        n_neurons+=layer_sizes[i+1];
        n_params+=layer_sizes[i+1]*(1+layer_sizes[i]);
    }
    all_params.resize(n_params);
    all_params_gradient.resize(n_params);

    // *** Set handles ***
    layer_params.resize(n_layers-1);
    layer_params_gradient.resize(n_layers-1);
    biases.resize(n_layers-1);
    weights.resize(n_layers-1);

    for (int i=0,p=0;i<n_layers-1;i++)    {
        int np=layer_sizes[i+1]*(1+layer_sizes[i]);
        layer_params[i]=all_params.subVec(p,np).toMat(layer_sizes[i+1],layer_sizes[i]+1);
        biases[i]=layer_params[i].subMatColumns(0,1);
        weights[i]=layer_params[i].subMatColumns(1,layer_sizes[i]); // weights[0] from layer 0 to layer 1
        layer_params_gradient[i]=all_params_gradient.subVec(p,np).toMat(layer_sizes[i+1],layer_sizes[i]+1);
        p+=np;
    }

    // *** Allocate memory for outputs and gradients on neurons ***
    neuron_extended_outputs.resize(minibatch_size,layer_sizes[0]+1+n_neurons+n_layers);
    neuron_gradients.resize(minibatch_size,n_neurons);

    // *** Set handles and biases ***
    neuron_outputs_per_layer.resize(n_layers); // layer 0 = input, layer n_layers-1 = output
    neuron_extended_outputs_per_layer.resize(n_layers); // layer 0 = input, layer n_layers-1 = output
    neuron_gradients_per_layer.resize(n_layers); // layer 0 not used

    int k=0, kk=0;
    for (int i=0;i<n_layers;i++)
    {
        neuron_extended_outputs_per_layer[i] = neuron_extended_outputs.subMatColumns(k,1+layer_sizes[i]);
        neuron_extended_outputs_per_layer[i].column(0).fill(1.0); // for biases
        neuron_outputs_per_layer[i]=neuron_extended_outputs_per_layer[i].subMatColumns(1,layer_sizes[i]);
        k+=1+layer_sizes[i];
        if(i>0) {
            neuron_gradients_per_layer[i] = neuron_gradients.subMatColumns(kk,layer_sizes[i]);
            kk+=layer_sizes[i];
        }
    }

    Profiler::activate();

}

// ### Nothing to add here, simply calls build_
void mNNet::build()
{
    inherited::build();
    build_();
}


void mNNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(hidden_layer_sizes, copies);
    deepCopyField(layer_sizes, copies);
    deepCopyField(all_params, copies);
    deepCopyField(biases, copies);
    deepCopyField(weights, copies);
    deepCopyField(layer_params, copies);
    deepCopyField(all_params_gradient, copies);
    deepCopyField(layer_params_gradient, copies);
    deepCopyField(neuron_gradients, copies);
    deepCopyField(neuron_gradients_per_layer, copies);
    deepCopyField(neuron_extended_outputs, copies);
    deepCopyField(neuron_extended_outputs_per_layer, copies);
    deepCopyField(neuron_outputs_per_layer, copies);
    deepCopyField(targets, copies);
    deepCopyField(example_weights, copies);
    deepCopyField(train_costs, copies);
}


int mNNet::outputsize() const
{
    return noutputs;
}

void mNNet::forget()
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
    cumulative_training_time=0.0;
}

void mNNet::train()
{

    if (inputsize_<0)
        build();
    if(!train_set)
        PLERROR("In NNet::train, you did not setTrainingSet");
    if(!train_stats)
        setTrainStatsCollector(new VecStatsCollector());

    targets.resize(minibatch_size,targetsize());  // the train_set's targetsize()
    example_weights.resize(minibatch_size);

    TVec<string> train_cost_names = getTrainCostNames() ;
    train_costs.resize(minibatch_size,train_cost_names.length()-2); 
    train_costs.fill(MISSING_VALUE) ;
    Vec costs_plus_time(train_costs.width()+2);
    costs_plus_time[train_costs.width()] = MISSING_VALUE;
    costs_plus_time[train_costs.width()+1] = MISSING_VALUE;
    Vec costs = costs_plus_time.subVec(0,train_costs.width());

    train_stats->forget();

    int b, sample, nsamples;
    nsamples = train_set->length();
    Vec input,target;   // TODO discard these variables.

    Profiler::reset("training");
    Profiler::start("training");

    for( ; stage<nstages; stage++)
    {
        sample = stage % nsamples;
        b = stage % minibatch_size;
        input = neuron_outputs_per_layer[0](b);
        target = targets(b);
        train_set->getExample(sample, input, target, example_weights[b]);
        if (b+1==minibatch_size) // TODO do also special end-case || stage+1==nstages)
        {
            onlineStep(stage, targets, train_costs, example_weights );
            for (int i=0;i<minibatch_size;i++)  {
                costs << train_costs(b);    // TODO Is the copy necessary? Might be
                                            // better to waste some memory in
                                            // train_costs instead
                train_stats->update( costs_plus_time );
            }
        }
    }

    Profiler::end("training");
    if (verbosity>0)
        Profiler::report(cout);
    // Take care of the timing stats.
    const Profiler::Stats& stats = Profiler::getStats("training");
    costs.fill(MISSING_VALUE);
    real ticksPerSec = Profiler::ticksPerSecond();
    real cpu_time = (stats.user_duration+stats.system_duration)/ticksPerSec;
    cumulative_training_time += cpu_time;
    costs_plus_time[train_costs.width()] = cpu_time;
    costs_plus_time[train_costs.width()+1] = cumulative_training_time;
    train_stats->update( costs_plus_time );
    train_stats->finalize(); // finalize statistics for this epoch
}

void mNNet::onlineStep(int t, const Mat& targets,
                             Mat& train_costs, Vec example_weights)
{
    PLASSERT(targets.length()==minibatch_size && train_costs.length()==minibatch_size && example_weights.length()==minibatch_size);

    fpropNet(minibatch_size);
    fbpropLoss(neuron_outputs_per_layer[n_layers-1],targets,example_weights,train_costs);
    bpropUpdateNet(t);

    l1regularizeOutputs();
}

void mNNet::computeOutput(const Vec& input, Vec& output) const
{
    neuron_outputs_per_layer[0](0) << input;
    fpropNet(1);
    output << neuron_outputs_per_layer[n_layers-1](0);
}

//! compute (pre-final-non-linearity) network top-layer output given input
void mNNet::fpropNet(int n_examples) const
{
    PLASSERT_MSG(n_examples<=minibatch_size,"mNNet::fpropNet: nb input vectors treated should be <= minibatch_size\n");
    for (int i=0;i<n_layers-1;i++)
    {
        Mat prev_layer = neuron_extended_outputs_per_layer[i];
        Mat next_layer = neuron_outputs_per_layer[i+1];
        if (n_examples!=minibatch_size) {
            prev_layer = prev_layer.subMatRows(0,n_examples);
            next_layer = next_layer.subMatRows(0,n_examples);
        }

        // try to use BLAS for the expensive operation
        productScaleAcc(next_layer, prev_layer, false, layer_params[i], true, 1, 0);

        // compute layer's output non-linearity
        if (i+1<n_layers-1) {
            for (int k=0;k<n_examples;k++)  {
                Vec L=next_layer(k);
                compute_tanh(L,L);
            }
        }   else if (output_type=="NLL")    {
            for (int k=0;k<n_examples;k++)  {
                Vec L=next_layer(k);
                log_softmax(L,L);
            }
        }   else if (output_type=="cross_entropy")  {
            for (int k=0;k<n_examples;k++)  {
                Vec L=next_layer(k);
                log_sigmoid(L,L);
            }
         }
    }
}

//! compute train costs given the (pre-final-non-linearity) network top-layer output
void mNNet::fbpropLoss(const Mat& output, const Mat& target, const Vec& example_weight, Mat& costs) const
{
    int n_examples = output.length();
    Mat out_grad = neuron_gradients_per_layer[n_layers-1];
    if (n_examples!=minibatch_size)
        out_grad = out_grad.subMatRows(0,n_examples);
    int target_class;
    Vec outp, grad;
    if (output_type=="NLL") {
        for (int i=0;i<n_examples;i++)  {
            target_class = int(round(target(i,0)));
            #ifdef BOUNDCHECK
            if(target_class>=noutputs)
                PLERROR("In mNNet::fbpropLoss one target value %d is higher then allowed by nout %d",
                        target_class, noutputs);
            #endif          
            outp = output(i);
            grad = out_grad(i);
            exp(outp,grad); // map log-prob to prob
            costs(i,0) = -outp[target_class];
            costs(i,1) = (target_class == argmax(outp))?0:1;
            grad[target_class]-=1;
            if (example_weight[i]!=1.0)
                costs(i,0) *= example_weight[i];
        }
    }
    else if(output_type=="cross_entropy")   {
        for (int i=0;i<n_examples;i++)  {
            target_class = int(round(target(i,0)));
            outp = output(i);
            grad = out_grad(i);
            exp(outp,grad); // map log-prob to prob
            if( target_class == 1 ) {
                costs(i,0) = - outp[0];
                costs(i,1) = (grad[0]>0.5)?0:1;
            }   else    {
                costs(i,0) = - pl_log( 1.0 - grad[0] );
                costs(i,1) = (grad[0]>0.5)?1:0;
            }
            grad[0] -= (real)target_class; // ?
            if (example_weight[i]!=1.0)
                costs(i,0) *= example_weight[i];
        }
    }
    else // if (output_type=="MSE")
    {
        substract(output,target,out_grad);
        for (int i=0;i<n_examples;i++)  {
            costs(i,0) = pownorm(out_grad(i));
            if (example_weight[i]!=1.0) {
                out_grad(i) *= example_weight[i];
                costs(i,0) *= example_weight[i];
            }
        }
    }
}

//! Performs the backprop update. Must be called after the fbpropNet.
void mNNet::bpropUpdateNet(int t)
{
    // mean gradient over minibatch_size examples has less variance
    // can afford larger learning rate (divide by sqrt(minibatch)
    // instead of minibatch)
    real lrate = init_lrate/(1 + t*lrate_decay);
    lrate /= sqrt(real(minibatch_size));

    for (int i=n_layers-1;i>0;i--)  {
        // here neuron_gradients_per_layer[i] contains the gradient on
        // activations (weighted sums)
        //      (minibatch_size x layer_size[i])
        Mat previous_neurons_gradient = neuron_gradients_per_layer[i-1];
        Mat next_neurons_gradient = neuron_gradients_per_layer[i];
        Mat previous_neurons_output = neuron_outputs_per_layer[i-1];

        if (i>1) // if not first hidden layer then compute gradient on previous layer
        {
            // propagate gradients
            productScaleAcc(previous_neurons_gradient,next_neurons_gradient,false,
                            weights[i-1],false,1,0);
            // propagate through tanh non-linearity
            // TODO IN NEED OF OPTIMIZATION
            for (int j=0;j<previous_neurons_gradient.length();j++)  {
                real* grad = previous_neurons_gradient[j];
                real* out = previous_neurons_output[j];
                for (int k=0;k<previous_neurons_gradient.width();k++,out++)
                    grad[k] *= (1 - *out * *out); // gradient through tanh derivative
            }
        }
        // compute gradient on parameters and update them in one go (more
        // efficient)
        productScaleAcc(layer_params[i-1],next_neurons_gradient,true,
                            neuron_extended_outputs_per_layer[i-1],false,
                            -lrate,1);
    }
}

//! Computes the gradients without doing the update.
//! Must be called after fbpropLoss
void mNNet::bpropNet(int t)
{
    for (int i=n_layers-1;i>0;i--)  {
        // here neuron_gradients_per_layer[i] contains the gradient on
        // activations (weighted sums)
        //      (minibatch_size x layer_size[i])
        Mat previous_neurons_gradient = neuron_gradients_per_layer[i-1];
        Mat next_neurons_gradient = neuron_gradients_per_layer[i];
        Mat previous_neurons_output = neuron_outputs_per_layer[i-1];

        if (i>1) // if not first hidden layer then compute gradient on previous layer
        {
            // propagate gradients
            productScaleAcc(previous_neurons_gradient,next_neurons_gradient,false,
                            weights[i-1],false,1,0);
            // propagate through tanh non-linearity
            // TODO IN NEED OF OPTIMIZATION
            for (int j=0;j<previous_neurons_gradient.length();j++)  {
                real* grad = previous_neurons_gradient[j];
                real* out = previous_neurons_output[j];
                for (int k=0;k<previous_neurons_gradient.width();k++,out++)
                    grad[k] *= (1 - *out * *out); // gradient through tanh derivative
            }
        }
        // compute gradient on parameters 
        productScaleAcc(layer_params_gradient[i-1],next_neurons_gradient,true,
                            neuron_extended_outputs_per_layer[i-1],false,
                            1,0);
    }
}

void mNNet::l1regularizeOutputs()
{
    // mean gradient over minibatch_size examples has less variance
    // can afford larger learning rate (divide by sqrt(minibatch)
    // instead of minibatch)
    real lrate = init_lrate/(1 + stage*lrate_decay);
    lrate /= sqrt(real(minibatch_size));

    // Output layer L1 regularization
    if( output_layer_L1_penalty_factor != 0. )    {
        real L1_delta = lrate * output_layer_L1_penalty_factor;
        real* m_i = layer_params[n_layers-2].data();
        for(int i=0; i<layer_params[n_layers-2].length();i++,m_i+=layer_params[n_layers-2].mod())  {
            for(int j=0; j<layer_params[n_layers-2].width(); j++)   {
                if( m_i[j] > L1_delta )
                    m_i[j] -= L1_delta;
                else if( m_i[j] < -L1_delta )
                    m_i[j] += L1_delta;
                else
                    m_i[j] = 0.;
            }
        }
    }
}

void mNNet::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    Vec w(1);
    w[0]=1;
    Mat outputM = output.toMat(1,output.length());
    Mat targetM = target.toMat(1,output.length());
    Mat costsM = costs.toMat(1,costs.length());
    fbpropLoss(outputM,targetM,w,costsM);
}

void mNNet::computeOutputs(const Mat& input, Mat& output) const
{
    PLASSERT(test_minibatch_size<=minibatch_size);
    neuron_outputs_per_layer[0].subMat(0,0,input.length(),input.width()) << input;
    fpropNet(input.length());
    output << neuron_outputs_per_layer[n_layers-1].subMat(0,0,output.length(),output.width());
}
void mNNet::computeOutputsAndCosts(const Mat& input, const Mat& target, 
                                      Mat& output, Mat& costs) const
{//TODO
    int n=input.length();
    PLASSERT(target.length()==n);
    output.resize(n,outputsize());
    costs.resize(n,nTestCosts());
    computeOutputs(input,output);

    Vec w(n);
    w.fill(1);
    fbpropLoss(output,target,w,costs);
}

TVec<string> mNNet::getTestCostNames() const
{
    TVec<string> costs;
    if (output_type=="NLL")
    {
        costs.resize(3);
        costs[0]="NLL";
        costs[1]="class_error";
    }
    else if (output_type=="cross_entropy")  {
        costs.resize(3);
        costs[0]="cross_entropy";
        costs[1]="class_error";
    }
    else if (output_type=="MSE")
    {
        costs.resize(1);
        costs[0]="MSE";
    }
    return costs;
}

TVec<string> mNNet::getTrainCostNames() const
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
