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

// Authors: Hugo Larochelle

/*! \file StackedSVDNet.cc */

#include "StackedSVDNet.h"

#define PL_LOG_MODULE_NAME "StackedSVDNet"
#include <plearn/io/pl_log.h>
#include <plearn/math/plapack.h>


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
    global_output_layer(false),
    relative_min_improvement(1e-3),
    n_layers( 0 )
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

    declareOption(ol, "global_output_layer", 
                  &StackedSVDNet::global_output_layer,
                  OptionBase::buildoption,
                  "Indication that the output layer (given by the final module)\n"
                  "should have as input all units of the network (including the"
                  "input units).\n");

    declareOption(ol, "relative_min_improvement", 
                  &StackedSVDNet::relative_min_improvement,
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

        reconstruction_costs.resize(batch_size,1);    

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
                
            activation_gradients[i].resize( batch_size, layers[i]->size );
            expectation_gradients[i].resize( batch_size, layers[i]->size );
        }

        if( !final_cost )
            PLERROR("StackedSVDNet::build_costs() - \n"
                    "final_cost should be provided.\n");

        final_cost_inputs.resize( batch_size, final_cost->input_size );
        final_cost_value.resize( final_cost->output_size );
        final_cost_values.resize( batch_size, final_cost->output_size );
        final_cost_gradients.resize( batch_size, final_cost->input_size );
        final_cost->setLearningRate( fine_tuning_learning_rate );

        if( !(final_cost->random_gen) )
        {
            final_cost->random_gen = random_gen;
            final_cost->forget();
        }

        if( !final_module )
            PLERROR("StackedSVDNet::build_costs() - \n"
                    "final_module should be provided.\n");
    
        if(global_output_layer)
        {
            int sum = 0;
            for(int i=0; i<layers.length(); i++)
                sum += layers[i]->size;
            if( sum != final_module->input_size )
                PLERROR("StackedSVDNet::build_costs() - \n"
                        "final_module should have an input_size of %d.\n", 
                        sum);

            global_output_layer_input.resize(sum);
            global_output_layer_inputs.resize(batch_size,sum);
            global_output_layer_input_gradients.resize(batch_size,sum);
            expectation_gradients[n_layers-1] = 
                global_output_layer_input_gradients.subMat(
                    0, sum-layers[n_layers-1]->size, 
                    batch_size, layers[n_layers-1]->size);
        }
        else
        {
            if( layers[n_layers-1]->size != final_module->input_size )
                PLERROR("StackedSVDNet::build_costs() - \n"
                        "final_module should have an input_size of %d.\n", 
                        layers[n_layers-1]->size);
        }

        if( final_module->output_size != final_cost->input_size )
            PLERROR("StackedSVDNet::build_costs() - \n"
                    "final_module should have an output_size of %d.\n", 
                    final_cost->input_size);

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

    deepCopyField(layers, copies);
    deepCopyField(final_module, copies);
    deepCopyField(final_cost, copies);
    deepCopyField(connections, copies);
    deepCopyField(rbm_connections, copies);
    deepCopyField(activation_gradients, copies);
    deepCopyField(expectation_gradients, copies);
    deepCopyField(reconstruction_layer, copies);
    deepCopyField(reconstruction_targets, copies);
    deepCopyField(reconstruction_costs, copies);
    deepCopyField(reconstruction_activation_gradient, copies);
    deepCopyField(reconstruction_activation_gradients, copies);
    deepCopyField(reconstruction_input_gradients, copies);
    deepCopyField(global_output_layer_input, copies);
    deepCopyField(global_output_layer_inputs, copies);
    deepCopyField(global_output_layer_input_gradients, copies);
    deepCopyField(final_cost_inputs, copies);
    deepCopyField(final_cost_value, copies);
    deepCopyField(final_cost_values, copies);
    deepCopyField(final_cost_gradients, copies);
    
    //PLERROR("In StackedSVDNet::makeDeepCopyFromShallowCopy(): "
    //        "not implemented yet.");
}


int StackedSVDNet::outputsize() const
{
    return final_module->output_size;
}

void StackedSVDNet::forget()
{
    inherited::forget();

    connections.resize(0);
    rbm_connections.resize(0);
    
    for(int i=0; i<layers.length(); i++)
        layers[i]->forget();

    final_module->forget();
    final_cost->forget();

    stage = 0;
}

void StackedSVDNet::train()
{
    MODULE_LOG << "train() called " << endl;

    Vec input( inputsize() );
    Vec target( targetsize() );
    real weight; // unused
    Mat inputs( batch_size, inputsize() );
    Mat targets( batch_size, targetsize() );

    TVec<string> train_cost_names = getTrainCostNames() ;
    Vec train_costs( train_cost_names.length() );
    train_costs.fill(MISSING_VALUE) ;

    PP<ProgressBar> pb;

    // clear stats of previous epoch
    train_stats->forget();

    real lr = 0;
    int init_stage;

    /***** initial greedy training *****/
    if(stage == 0)
    {
        connections.resize(n_layers-1);
        rbm_connections.resize(n_layers-1);
        TVec< Vec > biases(n_layers-1);
        for( int i=0 ; i<n_layers-1 ; i++ )
        {
            MODULE_LOG << "Training connection weights between layers " << i
                       << " and " << i+1 << endl;

            connections[i] = new RBMMatrixConnection();
            connections[i]->up_size = layers[i]->size;
            connections[i]->down_size = layers[i]->size;
            connections[i]->random_gen = random_gen;
            connections[i]->build();
            for(int j=0; j < layers[i]->size; j++)
                connections[i]->weights(j,j) = 0;

            rbm_connections[i] = (RBMMatrixConnection *) connections[i];

            CopiesMap map;
            reconstruction_layer = layers[ i ]->deepCopy( map );
            reconstruction_targets.resize( batch_size, layers[ i ]->size );
            reconstruction_activation_gradient.resize( layers[ i ]->size );
            reconstruction_activation_gradients.resize( 
                batch_size, layers[ i ]->size );
            reconstruction_input_gradients.resize( 
                batch_size, layers[ i ]->size );

            lr = greedy_learning_rate;
            connections[i]->setLearningRate( lr );
            reconstruction_layer->setLearningRate( lr );

            real cost = 0;
            real last_cost = 0;
            int nupdates = 0;
            int nepochs = 0;
            while( nepochs < 2 ||
                   (last_cost - cost) / last_cost >= relative_min_improvement )
            {
                train_stats->forget();
                for(int sample = 0; 
                    sample < train_set.length()/batch_size; 
                    sample++)
                {
                    if( !fast_exact_is_equal( greedy_decrease_ct , 0 ) )
                    {
                        lr = greedy_learning_rate/(1 + greedy_decrease_ct 
                                                   * nupdates);
                        connections[i]->setLearningRate( lr );
                        reconstruction_layer->setLearningRate( lr );                
                    }
                    
                    for(int j=0; j<batch_size; j++)
                    {
                        train_set->getExample(sample*batch_size + j, 
                                              input, target, weight);
                        inputs(j) << input;
                        targets(j) << target;
                    }
                    greedyStep( inputs, targets, i, train_costs );
                    nupdates++;
                    train_stats->update( train_costs );
                }
                train_stats->finalize();
                nepochs++;
                last_cost = cost;
                cost = train_stats->getMean()[i];
                if(verbosity > 2)
                    cout << "reconstruction error at iteration " << nepochs << 
                        ": " << 
                        cost << " or " << cost/layers[i]->size << " (rel)" << endl;
            }

            // Fill in the empty diagonal
            for(int j=0; j<layers[i]->size; j++)
            {
                connections[i]->weights(j,j) = maxabs(connections[i]->weights(j));
            }

            if(layers[i]->size != layers[i+1]->size)
            {
                Mat A,U,Vt;
                Vec S;
                A.resize( reconstruction_layer->size, 
                          reconstruction_layer->size+1);
                A.column( 0 ) << reconstruction_layer->bias;
                A.subMat( 0, 1, reconstruction_layer->size, 
                          reconstruction_layer->size ) << 
                    connections[i]->weights;
                SVD( A, U, S, Vt );
                connections[ i ]->up_size = layers[ i+1 ]->size;
                connections[ i ]->down_size = layers[ i ]->size;
                connections[ i ]->build();
                connections[ i ]->weights << Vt.subMat( 
                    0, 1, layers[ i+1 ]->size, Vt.width()-1 );
                biases[ i ].resize( layers[i+1]->size );
                for(int j=0; j<biases[ i ].length(); j++)
                    biases[ i ][ j ] = Vt(j,0);

                for(int j=0; j<connections[ i ]->up_size; j++)
                {
                    connections[ i ]->weights( j ) *= S[ j ];
                    biases[ i ][ j ] *= S[ j ];
                }
            }
            else
            {
                biases[ i ].resize( layers[ i+1 ]->size );
                biases[ i ] << reconstruction_layer->bias;
            }
        }
        stage++;
        for(int i=0; i<biases.length(); i++)
        {
            layers[ i+1 ]->bias << biases[ i ];
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
            for( int sample = 0; 
                 sample<train_set->length()/batch_size; 
                 sample++)
            {
                if( !fast_exact_is_equal( fine_tuning_decrease_ct, 0. ) )
                    setLearningRate( fine_tuning_learning_rate
                                     / (1. + fine_tuning_decrease_ct * stage ) );

                for(int j=0; j<batch_size; j++)
                {
                    train_set->getExample(sample*batch_size + j, 
                                          input, target, weight);
                    inputs(j) << input;
                    targets(j) << target;
                }
                fineTuningStep( inputs, targets, train_costs );
                train_stats->update( train_costs );
                
                if( pb )
                    pb->update( stage - init_stage + 1 );
            }
            if(verbosity > 2)
                cout << "error at stage " << stage << ": " << 
                    train_stats->getMean() << endl;

        }
    }
    
    train_stats->finalize();
}

void StackedSVDNet::greedyStep( const Mat& inputs, const Mat& targets, int index, Vec train_costs )
{
    PLASSERT( index < n_layers );

    layers[ 0 ]->setExpectations( inputs );
    
    for( int i=0 ; i<index ; i++ )
    {
        connections[ i ]->setAsDownInputs( layers[i]->getExpectations() );
        layers[ i+1 ]->getAllActivations( rbm_connections[i], 0, true );
        layers[ i+1 ]->computeExpectations();
    }
    reconstruction_targets << layers[ index ]->getExpectations();
    
    connections[ index ]->setAsDownInputs( layers[ index ]->getExpectations() );
    reconstruction_layer->getAllActivations( rbm_connections[ index ], 0, true );
    reconstruction_layer->computeExpectations();
    
    reconstruction_layer->fpropNLL( layers[ index ]->getExpectations(), 
                                    reconstruction_costs);
    train_costs[index] = sum( reconstruction_costs )/batch_size;

    reconstruction_layer->bpropNLL( 
        layers[ index ]->getExpectations(), reconstruction_costs,
        reconstruction_activation_gradients );

    columnMean( reconstruction_activation_gradients, 
                reconstruction_activation_gradient );
    reconstruction_layer->update( reconstruction_activation_gradient );

    connections[ index ]->bpropUpdate( 
        layers[ index ]->getExpectations(), 
        layers[ index ]->activations, 
        reconstruction_input_gradients, 
        reconstruction_activation_gradients);

    // Set diagonal to zero
    for(int i=0; i<connections[ index ]->up_size; i++)
        connections[ index ]->weights(i,i) = 0;
}

void StackedSVDNet::fineTuningStep( const Mat& inputs, const Mat& targets,
                                    Vec& train_costs )
{
    // fprop
    layers[ 0 ]->setExpectations( inputs );
    
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        connections[ i ]->setAsDownInputs( layers[i]->getExpectations() );
        layers[ i+1 ]->getAllActivations( rbm_connections[i], 0, true );
        layers[ i+1 ]->computeExpectations();
    }

    if( global_output_layer )
    {
        int offset = 0;
        for(int i=0; i<layers.length(); i++)
        {
            global_output_layer_inputs.subMat(0, offset, 
                                              batch_size, layers[i]->size)
                << layers[i]->getExpectations();
            offset += layers[i]->size;
        }
        final_module->fprop( global_output_layer_inputs, final_cost_inputs );
    }
    else
    {
        final_module->fprop( layers[ n_layers-1 ]->getExpectations(),
                             final_cost_inputs );
    }
    final_cost->fprop( final_cost_inputs, targets, final_cost_values );

    columnMean( final_cost_values, 
                final_cost_value );
    train_costs.subVec(train_costs.length()-final_cost_value.length(),
                       final_cost_value.length()) << final_cost_value;

    final_cost->bpropUpdate( final_cost_inputs, targets,
                             final_cost_value,
                             final_cost_gradients );
    
    if( global_output_layer )
    {
        final_module->bpropUpdate( global_output_layer_inputs,
                                   final_cost_inputs,
                                   global_output_layer_input_gradients,
                                   final_cost_gradients );     
    }
    else
    {
        final_module->bpropUpdate( layers[ n_layers-1 ]->getExpectations(),
                                   final_cost_inputs,
                                   expectation_gradients[ n_layers-1 ],
                                   final_cost_gradients );
    }

    int sum = final_module->input_size - layers[ n_layers-1 ]->size;
    for( int i=n_layers-1 ; i>0 ; i-- )
    {
        if( global_output_layer && i != n_layers-1 )
        {
            expectation_gradients[ i ] +=  
                global_output_layer_input_gradients.subMat(
                    0, sum - layers[i]->size,
                    batch_size, layers[i]->size);
            sum -= layers[i]->size;
        }
                

        layers[ i ]->bpropUpdate( layers[ i ]->activations,
                                  layers[ i ]->getExpectations(),
                                  activation_gradients[ i ],
                                  expectation_gradients[ i ] );

        connections[ i-1 ]->bpropUpdate( layers[ i-1 ]->getExpectations(),
                                         layers[ i ]->activations,
                                         expectation_gradients[ i-1 ],
                                         activation_gradients[ i ] );
    }
}

void StackedSVDNet::computeOutput(const Vec& input, Vec& output) const
{
    // fprop
    layers[ 0 ]->expectation <<  input ;
    layers[ 0 ]->expectation_is_up_to_date = true;
    
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        connections[ i ]->setAsDownInput( layers[i]->expectation );
        layers[ i+1 ]->getAllActivations( rbm_connections[i], 0, false );
        layers[ i+1 ]->computeExpectation();
    }

    if(global_output_layer)
    {
        int offset = 0;
        for(int i=0; i<layers.length(); i++)
        {
            global_output_layer_input.subVec(offset, layers[i]->size)
                << layers[i]->expectation;
            offset += layers[i]->size;
        }
        final_module->fprop( global_output_layer_input, output );
    }
    else
    {
        final_module->fprop( layers[ n_layers-1 ]->expectation,
                             output );
    }
}

void StackedSVDNet::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    //Assumes that computeOutput has been called

    costs.resize( getTestCostNames().length() );
    costs.fill( MISSING_VALUE );
    
    final_cost->fprop( output, target, final_cost_value );
    costs.subVec(costs.length()-final_cost_value.length(),
                 final_cost_value.length()) <<
        final_cost_value;
}

TVec<string> StackedSVDNet::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).

    TVec<string> cost_names(0);

    for( int i=0; i<layers.size()-1; i++)
        cost_names.push_back("reconstruction_error_" + tostring(i+1));
    
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
