// -*- C++ -*-

// StackedSVDNet.cc
//
// Copyright (C) 2007 Hugo Larochelle
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

/*! \file StackedSVDNet.cc */


#define PL_LOG_MODULE_NAME "StackedSVDNet"
#include <plearn/io/pl_log.h>

#include "StackedSVDNet.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    StackedSVDNet,
    "Neural net, initialized with SVDs of logistic auto-regressions.",
    ""
    );

StackedSVDNet::StackedSVDNet() :
    greedy_learning_rate( 0. ),
    greedy_decrease_ct( 0. ),
    fine_tuning_learning_rate( 0. ),
    fine_tuning_decrease_ct( 0. ),
    batch_size(50),
    minimum_relative_improvement(1e-3),
    n_layers( 0 ),
    currently_trained_layer( 0 )
{
    // random_gen will be initialized in PLearner::build_()
    random_gen = new PRandom();
    nstages = 0;
}

void StackedSVDNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "greedy_learning_rate", 
                  &StackedSVDNet::greedy_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the logistic auto-regression "
                  "gradient descent training"
        );
    
    declareOption(ol, "greedy_decrease_ct", 
                  &StackedSVDNet::greedy_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during the "
                  "logistic auto-regression gradient descent training. "
        );

    declareOption(ol, "fine_tuning_learning_rate", 
                  &StackedSVDNet::fine_tuning_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the fine tuning gradient descent");

    declareOption(ol, "fine_tuning_decrease_ct", 
                  &StackedSVDNet::fine_tuning_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "fine tuning\n"
                  "gradient descent.\n");

    declareOption(ol, "batch_size", 
                  &StackedSVDNet::batch_size,
                  OptionBase::buildoption,
                  "Size of mini-batch for gradient descent");

    declareOption(ol, "minimum_relative_improvement", 
                  &StackedSVDNet::minimum_relative_improvement,
                  OptionBase::buildoption,
                  "Minimum relative improvement convergence criteria \n"
                  "for the logistic auto-regression.");

    declareOption(ol, "layers", &StackedSVDNet::layers,
                  OptionBase::buildoption,
                  "The layers of units in the network. The first element\n"
                  "of this vector should be the input layer and the\n"
                  "subsequent elements should be the hidden layers. The\n"
                  "should not be included in this layer.\n");

    declareOption(ol, "final_module", &StackedSVDNet::final_module,
                  OptionBase::buildoption,
                  "Module that takes as input the output of the last layer\n"
                  "(layers[n_layers-1), and feeds its output to final_cost\n"
                  "which defines the fine-tuning criteria.\n"
                 );

    declareOption(ol, "final_cost", &StackedSVDNet::final_cost,
                  OptionBase::buildoption,
                  "The cost function to be applied on top of the neural network\n"
                  "(i.e. at the output of final_module). Its gradients will be \n"
                  "backpropagated to final_module and then backpropagated to\n"
                  "the layers.\n"
                  );

    declareOption(ol, "connections", &StackedSVDNet::connections,
                  OptionBase::learntoption,
                  "The weights of the connections between the layers");

    declareOption(ol, "n_layers", &StackedSVDNet::n_layers,
                  OptionBase::learntoption,
                  "Number of layers");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void StackedSVDNet::build_()
{

    MODULE_LOG << "build_() called" << endl;

    if(inputsize_ > 0 && targetsize_ > 0)
    {
        // Initialize some learnt variables
        n_layers = layers.length();
        
        if( weightsize_ > 0 )
            PLERROR("StackedSVDNet::build_() - \n"
                    "usage of weighted samples (weight size > 0) is not\n"
                    "implemented yet.\n");
        
        if(layers[0]->size != inputsize_)
            PLERROR("StackedSVDNet::build_layers_and_connections() - \n"
                    "layers[0] should have a size of %d.\n",
                    inputsize_);
    
        activations.resize( n_layers );
        expectations.resize( n_layers );
        activation_gradients.resize( n_layers );
        expectation_gradients.resize( n_layers );

        for( int i=0 ; i<n_layers ; i++ )
        {
            if( !(layers[i]->random_gen) )
            {
                layers[i]->random_gen = random_gen;
                layers[i]->forget();
            }

            if(i>0 && layers[i]->size > layers[i-1]->size)
                PLERROR("In StackedSVDNet::build()_: "
                    "layers must have decreasing sizes from bottom to top.");
                
            activations[i].resize( batch_size, layers[i]->size );
            expectations[i].resize( batch_size, layers[i]->size );
            activation_gradients[i].resize( batch_size, layers[i]->size );
            expectation_gradients[i].resize( batch_size, layers[i]->size );
        }

        if( !final_cost )
            PLERROR("StackedSVDNet::build_costs() - \n"
                    "final_cost should be provided.\n");

        final_cost_gradient.resize( final_cost->input_size );
        final_cost->setLearningRate( fine_tuning_learning_rate );

        if( !(final_cost->random_gen) )
        {
            final_cost->random_gen = random_gen;
            final_cost->forget();
        }

        if( !final_module )
            PLERROR("StackedSVDNet::build_costs() - \n"
                    "final_module should be provided.\n");
    
        if( layers[n_layers-1]->size != final_module->input_size )
            PLERROR("StackedSVDNet::build_costs() - \n"
                    "final_module should have an input_size of %d.\n", 
                    layers[n_layers-1]->size);
    
        if( final_module->output_size != final_cost->input_size )
            PLERROR("StackedSVDNet::build_costs() - \n"
                    "final_module should have an output_size of %d.\n", 
                    final_module->input_size);

        final_module->setLearningRate( fine_tuning_learning_rate );

        if( !(final_module->random_gen) )
        {
            final_module->random_gen = random_gen;
            final_module->forget();
        }


        if(targetsize_ != 1)
            PLERROR("StackedSVDNet::build_costs() - \n"
                    "target size of %d is not supported.\n", targetsize_);    
    }
}

// ### Nothing to add here, simply calls build_
void StackedSVDNet::build()
{
    inherited::build();
    build_();
}


void StackedSVDNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(, copies);

    deepCopyField(training_schedule, copies);
    deepCopyField(layers, copies);
    deepCopyField(connections, copies);
    deepCopyField(reconstruction_connections, copies);
    deepCopyField(final_module, copies);
    deepCopyField(final_cost, copies);
    deepCopyField(partial_costs, copies);
    deepCopyField(partial_costs_weights, copies);
    deepCopyField(activations, copies);
    deepCopyField(expectations, copies);
    deepCopyField(activation_gradients, copies);
    deepCopyField(expectation_gradients, copies);
    deepCopyField(reconstruction_activations, copies);
    deepCopyField(reconstruction_expectations, copies);
    deepCopyField(reconstruction_activation_gradients, copies);
    deepCopyField(reconstruction_expectation_gradients, copies);
    deepCopyField(partial_costs_positions, copies);
    deepCopyField(partial_cost_value, copies);
    deepCopyField(final_cost_input, copies);
    deepCopyField(final_cost_value, copies);
    deepCopyField(final_cost_gradient, copies);
    deepCopyField(greedy_stages, copies);
    
    PLERROR("In StackedSVDNet::makeDeepCopyFromShallowCopy(): "
            "not implemented yet.");
}


int StackedSVDNet::outputsize() const
{
    return final_module->output_size;
}

void StackedSVDNet::forget()
{
    inherited::forget();

    connections.resize(0);
    
    final_module->forget();
    final_cost->forget();

    stage = 0;
}

void StackedSVDNet::train()
{
    MODULE_LOG << "train() called " << endl;
    MODULE_LOG << "  training_schedule = " << training_schedule << endl;

    Vec input( inputsize() );
    Vec target( targetsize() );
    real weight; // unused

    TVec<string> train_cost_names = getTrainCostNames() ;
    Vec train_costs( train_cost_names.length() );
    train_costs.fill(MISSING_VALUE) ;

    int nsamples = train_set->length();
    int sample;

    PP<ProgressBar> pb;

    // clear stats of previous epoch
    train_stats->forget();

    real lr = 0;
    int init_stage;

    /***** initial greedy training *****/
    if(stage == 0)
    {
        connections.resize(n_layers-1);
        TVec< Vec > biases(n_layers-1);
        for( int i=0 ; i<n_layers-1 ; i++ )
        {
            MODULE_LOG << "Training connection weights between layers " << i
                       << " and " << i+1 << endl;

            connections[i] = new RBMMatrixConnection();
            connections[i]->up_size = layers[i]->size;
            connections[i]->down_size = layers[i]->size;
            connections[i]->build();
            for(int j=0; j < layers[i]->size; j++)
                connections[i]->weights(j,j) = 0;

            lr = greedy_learning_rate;
            layers[i]->setLearningRate( lr );
            connections[i]->setLearningRate( lr );
            layers[i+1]->setLearningRate( lr );

            real cost = 30;
            real last_cost = 100;
            int nupdates = 0;
            int nepochs = 0;
            while( nepochs < 2 ||
                   (last_cost - cost) / last_cost >= minimum_relative_improvement )
            {
                train_stats->forget();
                for(int sample = 0; sample < train_set.length(); sample++)
                {
                    if( !fast_exact_is_equal( greedy_decrease_ct , 0 ) )
                    {
                        lr = greedy_learning_rate/(1 + greedy_decrease_ct 
                                                   * nupdates);
                        layers[i]->setLearningRate( lr );
                        connections[i]->setLearningRate( lr );
                        reconstruction_connections[i]->setLearningRate( lr );
                        layers[i+1]->setLearningRate( lr );                
                    }

                    train_set->getExample(sample, input, target, weight);
                    greedyStep( input, target, sample, train_costs );
                    nupdates++;
                    train_stats->update( train_costs );
                }
                train_stats->finalize();
                nepochs++;
                last_cost = cost;
                cost = train_stats->mean()[0];
            }
            Mat A,U,S,Vt;
            A.resize(layers[i]->size,layers[i]->size+1);
            A.column(0) << layers[i]->bias;
            A.subMat(0,1,layers[i]->size,layers[i]->size) << 
                connections[i]->weights;
            SVD(connections[i]->weights,U,S,V);
            connections[i]->up_size = layers[i+1]->size;
            connections[i]->down_size = layers[i]->size;
            connections[i]->build();
            connection[i]->weights << Vt.subRows(0,layers[i+1]->size);
            biases[i].resize(layers[i+1]->size);
            biases[i] << Vt.column(0).subVec(0,layers[i+1]->size);
            for(int j=0; j<connections[i]->up_size; j++)
            {
                connections[i]->weights(j) *= S(j,j);
                biases[i][j] *= S(j,j);
            }
        }
        stage++;
        for(int i=0; i<biases.length(); i++)
        {
            layers[i]->bias << biases[i];
        }
    }

    /***** fine-tuning by gradient descent *****/
    if( stage < nstages )
    {

        MODULE_LOG << "Fine-tuning all parameters, by gradient descent" << endl;
        MODULE_LOG << "  stage = " << stage << endl;
        MODULE_LOG << "  nstages = " << nstages << endl;
        MODULE_LOG << "  fine_tuning_learning_rate = " << 
            fine_tuning_learning_rate << endl;

        init_stage = stage;
        if( report_progress && stage < nstages )
            pb = new ProgressBar( "Fine-tuning parameters of all layers of "
                                  + classname(),
                                  nstages - init_stage );

        setLearningRate( fine_tuning_learning_rate );
        train_costs.fill(MISSING_VALUE);
        for( ; stage<nstages ; stage++ )
        {
            sample = stage % nsamples;
            if( !fast_exact_is_equal( fine_tuning_decrease_ct, 0. ) )
                setLearningRate( fine_tuning_learning_rate
                                 / (1. + fine_tuning_decrease_ct * stage ) );

            train_set->getExample( sample, input, target, weight );
            fineTuningStep( input, target, train_costs );
            train_stats->update( train_costs );

            if( pb )
                pb->update( stage - init_stage + 1 );
        }
    }
    
    train_stats->finalize();
}

void StackedSVDNet::greedyStep( const Vec& input, const Vec& target, int index, Vec train_costs )
{
    PLASSERT( index < n_layers );

    expectations[0] << input;
    for( int i=0 ; i<index + 1; i++ )
    {
        connections[i]->fprop( expectations[i], activations[i+1] );
        layers[i+1]->fprop(activations[i+1],expectations[i+1]);
    }

    reconstruction_connections[ index ]->fprop( expectations[ index + 1],
                                                reconstruction_activations);
    layers[ index ]->fprop( reconstruction_activations,
                            layers[ index ]->expectation);
    
    layers[ index ]->expectation_is_up_to_date = true;
    train_costs[index] = layers[ index ]->fpropNLL(expectations[index]);

    layers[ index ]->bpropNLL(expectations[index], train_costs[index],
                                  reconstruction_activation_gradients);

    layers[ index ]->update(reconstruction_activation_gradients);

    // // This is a bad update! Propagates gradient through sigmoid again!
    // layers[ index ]->bpropUpdate( reconstruction_activations, 
    //                                   layers[ index ]->expectation,
    //                                   reconstruction_activation_gradients,
    //                                   reconstruction_expectation_gradients);

    reconstruction_connections[ index ]->bpropUpdate( 
        expectations[ index + 1], 
        reconstruction_activations, 
        reconstruction_expectation_gradients, //reused
        reconstruction_activation_gradients);

    if(!fast_exact_is_equal(l1_neuron_decay,0))
    {
        // Compute L1 penalty gradient on neurons
        real* hid = expectations[ index + 1 ].data();
        real* grad = reconstruction_expectation_gradients.data();
        int len = expectations[ index + 1 ].length();
        for(int i=0; i<len; i++)
        {
            if(*hid > l1_neuron_decay_center)
                *grad -= l1_neuron_decay;
            else if(*hid < l1_neuron_decay_center)
                *grad += l1_neuron_decay;
            hid++;
            grad++;
        }
    }

    // Update hidden layer bias and weights
    layers[ index+1 ]->bpropUpdate( activations[ index + 1 ],
                                    expectations[ index + 1 ],
                                    reconstruction_activation_gradients, // reused
                                    reconstruction_expectation_gradients);    

    connections[ index ]->bpropUpdate( 
        expectations[ index ],
        activations[ index + 1 ],
        reconstruction_expectation_gradients, //reused
        reconstruction_activation_gradients);

    // Set diagonal to zero!!!
}

void StackedSVDNet::fineTuningStep( const Vec& input, const Vec& target,
                                    Vec& train_costs )
{
    // fprop
    expectations[0] << input;
    for( int i=0 ; i<n_layers-1; i++ )
    {
        connections[i]->fprop( expectations[i], activations[i+1] );
        layers[i+1]->fprop(activations[i+1],expectations[i+1]);
    }

    final_module->fprop( expectations[ n_layers-1 ],
                         final_cost_input );
    final_cost->fprop( final_cost_input, target, final_cost_value );

    train_costs.subVec(train_costs.length()-final_cost_value.length(),
                       final_cost_value.length()) <<
        final_cost_value;

    final_cost->bpropUpdate( final_cost_input, target,
                             final_cost_value[0],
                             final_cost_gradient );
    final_module->bpropUpdate( expectations[ n_layers-1 ],
                               final_cost_input,
                               expectation_gradients[ n_layers-1 ],
                               final_cost_gradient );

    for( int i=n_layers-1 ; i>0 ; i-- )
    {
        layers[i]->bpropUpdate( activations[i],
                                expectations[i],
                                activation_gradients[i],
                                expectation_gradients[i] );

        connections[i-1]->bpropUpdate( expectations[i-1],
                                       activations[i],
                                       expectation_gradients[i-1],
                                       activation_gradients[i] );
    }
}

void StackedSVDNet::computeOutput(const Vec& input, Vec& output) const
{
    // fprop

    expectations[0] << input;

    for(int i=0 ; i<currently_trained_layer-1 ; i++ )
    {
        connections[i]->fprop( expectations[i], activations[i+1] );
        layers[i+1]->fprop(activations[i+1],expectations[i+1]);
    }

    if( currently_trained_layer<n_layers )
    {
        connections[currently_trained_layer-1]->fprop( 
            expectations[currently_trained_layer-1], 
            activations[currently_trained_layer] );
        layers[currently_trained_layer]->fprop(
            activations[currently_trained_layer],
            output);
    }
    else        
        final_module->fprop( expectations[ currently_trained_layer - 1],
                             output );
}

void StackedSVDNet::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    //Assumes that computeOutput has been called

    costs.resize( getTestCostNames().length() );
    costs.fill( MISSING_VALUE );

    if(compute_all_test_costs)
    {
        for(int i=0; i<currently_trained_layer-1; i++)
        {
            reconstruction_connections[ i ]->fprop( expectations[ i+1 ],
                                                    reconstruction_activations);
            layers[ i ]->fprop( reconstruction_activations,
                                    layers[ i ]->expectation);
            
            layers[ i ]->expectation_is_up_to_date = true;
            costs[i] = layers[ i ]->fpropNLL(expectations[ i ]);
            
            if( partial_costs && partial_costs[i])
            {
                partial_costs[ i ]->fprop( expectations[ i + 1],
                                           target, partial_cost_value );
                costs.subVec(partial_costs_positions[i],
                             partial_cost_value.length()) << 
                    partial_cost_value;
            }
        }
    }

    if( currently_trained_layer<n_layers )
    {
        reconstruction_connections[ currently_trained_layer-1 ]->fprop( 
            output,
            reconstruction_activations);
        layers[ currently_trained_layer-1 ]->fprop( 
            reconstruction_activations,
            layers[ currently_trained_layer-1 ]->expectation);
        
        layers[ currently_trained_layer-1 ]->expectation_is_up_to_date = true;
        costs[ currently_trained_layer-1 ] = 
            layers[ currently_trained_layer-1 ]->fpropNLL(
                expectations[ currently_trained_layer-1 ]);

        if( partial_costs && partial_costs[ currently_trained_layer-1 ] )
        {
            partial_costs[ currently_trained_layer-1 ]->fprop( 
                output,
                target, partial_cost_value );
            costs.subVec(partial_costs_positions[currently_trained_layer-1],
                         partial_cost_value.length()) << partial_cost_value;
        }
    }
    else
    {
        final_cost->fprop( output, target, final_cost_value );        
        costs.subVec(costs.length()-final_cost_value.length(),
                     final_cost_value.length()) <<
            final_cost_value;
    }
}

TVec<string> StackedSVDNet::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).

    TVec<string> cost_names(0);

    for( int i=0; i<layers.size()-1; i++)
        cost_names.push_back("reconstruction_error_" + tostring(i+1));
    
    for( int i=0 ; i<partial_costs.size() ; i++ )
    {
        TVec<string> cost_names = partial_costs[i]->name();
        for(int j=0; j<cost_names.length(); j++)
            cost_names.push_back("partial_cost_" + tostring(i+1) + "_" + 
                cost_names[j]);
    }

    cost_names.append( final_cost->name() );

    return cost_names;
}

TVec<string> StackedSVDNet::getTrainCostNames() const
{
    return getTestCostNames() ;    
}


//#####  Helper functions  ##################################################

void StackedSVDNet::setLearningRate( real the_learning_rate )
{
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        layers[i]->setLearningRate( the_learning_rate );
        connections[i]->setLearningRate( the_learning_rate );
    }
    layers[n_layers-1]->setLearningRate( the_learning_rate );

    final_cost->setLearningRate( fine_tuning_learning_rate );
    final_module->setLearningRate( fine_tuning_learning_rate );
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
