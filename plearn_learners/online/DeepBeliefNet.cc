// -*- C++ -*-

// DeepBeliefNet.cc
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

/*! \file DeepBeliefNet.cc */


#define PL_LOG_MODULE_NAME "DeepBeliefNet"
#include <plearn/io/pl_log.h>

#include "DeepBeliefNet.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DeepBeliefNet,
    "Neural net, learned layer-wise in a greedy fashion",
    "This version support different unit types, different connection types,\n"
    "and different cost functions, including the NLL in classification.\n");

DeepBeliefNet::DeepBeliefNet() :
    cd_learning_rate( 0. ),
    grad_learning_rate( 0. ),
    grad_decrease_ct( 0. ),
    grad_weight_decay( 0. ),
    use_classification_cost( true ),
    n_layers( 0 ),
    final_module_has_learning_rate( false ),
    final_cost_has_learning_rate( false ),
    nll_cost_index( -1 ),
    class_cost_index( -1 ),
    recons_cost_index( -1 ),
    final_cost_index( -1 )

{
    random_gen = new PRandom();
}

void DeepBeliefNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "cd_learning_rate", &DeepBeliefNet::cd_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during contrastive divergence"
                  " learning");

    declareOption(ol, "grad_learning_rate", &DeepBeliefNet::grad_learning_rate,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during"
                  " gradient descent");

    declareOption(ol, "grad_weight_decay", &DeepBeliefNet::grad_weight_decay,
                  OptionBase::buildoption,
                  "The weight decay used during the gradient descent");

    declareOption(ol, "n_classes", &DeepBeliefNet::n_classes,
                  OptionBase::buildoption,
                  "Number of classes in the training set\n"
                  "  - 0 means we are doing regression,\n"
                  "  - 1 means we have two classes, but only one output,\n"
                  "  - 2 means we also have two classes, but two outputs"
                  " summing to 1,\n"
                  "  - >2 is the usual multiclass case.\n"
                  );

    declareOption(ol, "training_schedule", &DeepBeliefNet::training_schedule,
                  OptionBase::buildoption,
                  "Number of examples to use during each phase of learning:\n"
                  "first the greedy phases, and then the gradient descent.\n");

    declareOption(ol, "use_classification_cost",
                  &DeepBeliefNet::use_classification_cost,
                  OptionBase::buildoption,
                  "If the first cost function is the NLL in classification,\n"
                  "pre-trained with CD, and using the last *two* layers to get"
                  " a better\n"
                  "approximation (undirected softmax) than layer-wise"
                  " mean-field.\n");

    declareOption(ol, "layers", &DeepBeliefNet::layers,
                  OptionBase::buildoption,
                  "The layers of units in the network");

    declareOption(ol, "connections", &DeepBeliefNet::connections,
                  OptionBase::buildoption,
                  "The weights of the connections between the layers");

    declareOption(ol, "classification_module",
                  &DeepBeliefNet::classification_module,
                  OptionBase::learntoption,
                  "The module computing the class probabilities (if"
                  " use_classification_cost)\n"
                  " on top of classification_module.\n"
                  );

    declareOption(ol, "classification_cost",
                  &DeepBeliefNet::classification_cost,
                  OptionBase::nosave,
                  "The module computing the classification cost function (NLL)"
                  " on top\n"
                  "of classification_module.\n"
                  );

    declareOption(ol, "joint_layer", &DeepBeliefNet::joint_layer,
                  OptionBase::nosave,
                  "Concatenation of layers[n_layers-2] and the target layer\n"
                  "(that is inside classification_module), if"
                  " use_classification_cost.\n"
                 );

    declareOption(ol, "final_module", &DeepBeliefNet::final_module,
                  OptionBase::buildoption,
                  "Optional module that takes as input the output of the last"
                  " layer\n"
                  "layers[n_layers-1), and its output is fed to final_cost,"
                  " and\n"
                  "concatenated with the one of classification_cost (if"
                  " present)\n"
                  "as output of the learner.\n"
                  "If it is not provided, then the last layer will directly be"
                  " put as\n"
                  "input of final_cost.\n"
                 );

    declareOption(ol, "final_cost", &DeepBeliefNet::final_cost,
                  OptionBase::buildoption,
                  "The cost function to be applied on top of the DBN (or of\n"
                  "final_module if provided). Its gradients will be"
                  " backpropagated\n"
                  "to final_module, then combined with the one of"
                  " classification_cost and\n"
                  "backpropagated to the layers.\n"
                  );

    declareOption(ol, "partial_costs", &DeepBeliefNet::partial_costs,
                  OptionBase::buildoption,
                  "The different cost functions to be applied on top of each"
                  " layer\n"
                  "(except the first one) of the RBM. These costs are not\n"
                  "back-propagated to previous layers.\n");

    declareOption(ol, "n_layers", &DeepBeliefNet::n_layers,
                  OptionBase::learntoption,
                  "Number of layers");

    /*
    declareOption(ol, "n_final_costs", &DeepBeliefNet::n_final_costs,
                  OptionBase::learntoption,
                  "Number of final costs");
     */

    /*
    declareOption(ol, "", &DeepBeliefNet::,
                  OptionBase::learntoption,
                  "");
     */

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DeepBeliefNet::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.

    MODULE_LOG << "build_() called" << endl;

    // Initialize some learnt variables
    n_layers = layers.length();

    if( training_schedule.length() != n_layers-1 )
    {
        MODULE_LOG << "training_schedule.length() != n_layers-1, resizing and"
            " zeroing" << endl;
        training_schedule.resize( n_layers-1 );
        training_schedule.fill( 0 );
    }

    build_layers_and_connections();

    // build the classification module, its cost and the joint layer
    if( use_classification_cost )
    {
        PLASSERT( n_classes >= 2 );
        build_classification_cost();
    }

    if( final_cost )
        build_final_cost();

    // build_costs(); /* ? */
}

void DeepBeliefNet::build_layers_and_connections()
{
    MODULE_LOG << "build_layers_and_connections() called" << endl;

    if( connections.length() != n_layers-1 )
        PLERROR("DeepBeliefNet::build_layers_and_connections() - \n"
                "connections.length() (%d) != n_layers-1 (%d).\n",
                connections.length(), n_layers-1);

    if( inputsize_ >= 0 )
        PLASSERT( layers[0]->size == inputsize() );

    activation_gradients.resize( n_layers );
    expectation_gradients.resize( n_layers );

    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        if( layers[i]->size != connections[i]->down_size )
            PLERROR("DeepBeliefNet::build_layers_and_connections() - \n"
                    "layers[%i]->size (%d) != connections[%i]->down_size (%d)."
                    "\n", i, layers[i]->size, i, connections[i]->down_size);

        if( connections[i]->up_size != layers[i+1]->size )
            PLERROR("DeepBeliefNet::build_layers_and_connections() - \n"
                    "connections[%i]->up_size (%d) != layers[%i]->size (%d)."
                    "\n", i, connections[i]->up_size, i+1, layers[i]->size);

        layers[i]->random_gen = random_gen;
        layers[i]->build();

        connections[i]->random_gen = random_gen;
        connections[i]->build();

        activation_gradients[i].resize( layers[i]->size );
        expectation_gradients[i].resize( layers[i]->size );
    }
    layers[n_layers-1]->random_gen = random_gen;
    activation_gradients[n_layers-1].resize( layers[n_layers-1]->size );
    expectation_gradients[n_layers-1].resize( layers[n_layers-1]->size );
}

void DeepBeliefNet::build_classification_cost()
{
    MODULE_LOG << "build_classification_cost() called" << endl;

    PP<RBMMatrixConnection> last_to_target = new RBMMatrixConnection();
    last_to_target->up_size = layers[n_layers-1]->size;
    last_to_target->down_size = n_classes;
    last_to_target->random_gen = random_gen;
    last_to_target->build();

    PP<RBMMultinomialLayer> target_layer = new RBMMultinomialLayer();
    target_layer->size = n_classes;
    target_layer->random_gen = random_gen;
    target_layer->build();

    classification_module = new RBMClassificationModule();
    classification_module->previous_to_last = connections[n_layers-2];
    classification_module->last_layer =
        (RBMBinomialLayer*) (RBMLayer*) layers[n_layers-1];
    classification_module->last_to_target = last_to_target;
    classification_module->target_layer = target_layer;
    classification_module->random_gen = random_gen;
    classification_module->build();

    classification_cost = new NLLCostModule();
    classification_cost->input_size = n_classes;
    classification_cost->target_size = 1;
    classification_cost->build();

    joint_layer = new RBMMixedLayer();
    joint_layer->sub_layers.resize( 2 );
    joint_layer->sub_layers[0] = layers[ n_layers-2 ];
    joint_layer->sub_layers[1] = target_layer;
    joint_layer->random_gen = random_gen;
    joint_layer->build();
}

void DeepBeliefNet::build_final_cost()
{
    MODULE_LOG << "build_final_cost() called" << endl;

    final_cost_gradient.resize( final_cost->input_size );

    if( final_module )
    {
        if( layers[n_layers-1]->size != final_module->input_size )
            PLERROR("DeepBeliefNet::build_final_cost() - \n"
                    "layers[%i]->size (%d) != final_module->input_size (%d)."
                    "\n", n_layers-1, layers[n_layers-1]->size,
                    final_module->input_size);

        if( final_module->output_size != final_cost->input_size )
            PLERROR("DeepBeliefNet::build_final_cost() - \n"
                    "final_module->output_size (%d) != final_cost->input_size."
                    "\n", n_layers-1, layers[n_layers-1]->size,
                    final_module->input_size);

        Object* obj = 0;
        OptionList::iterator it;
        string opt_id;

        // try to see if final_cost has an option named "learning_rate"
        final_module_has_learning_rate =
            final_cost->parseOptionName( "learning_rate", obj, it, opt_id );

        final_cost_has_learning_rate = final_module &&
            final_module->parseOptionName( "learning_rate", obj, it, opt_id );

    }
    else
    {
        if( layers[n_layers-1]->size != final_cost->input_size )
            PLERROR("DeepBeliefNet::build_final_cost() - \n"
                    "layers[%i]->size (%d) != final_cost->input_size (%d)."
                    "\n", n_layers-1, layers[n_layers-1]->size,
                    final_cost->input_size);
    }
    // TODO: check target size
}

// ### Nothing to add here, simply calls build_
void DeepBeliefNet::build()
{
    inherited::build();
    build_();
}


void DeepBeliefNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(, copies);

    deepCopyField(training_schedule, copies);
    deepCopyField(layers, copies);
    deepCopyField(connections, copies);
    deepCopyField(final_module, copies);
    deepCopyField(final_cost, copies);
    deepCopyField(partial_costs, copies);
    deepCopyField(classification_module, copies);
    deepCopyField(timer, copies);
    deepCopyField(classification_cost, copies);
    deepCopyField(joint_layer, copies);
    deepCopyField(activation_gradients, copies);
    deepCopyField(expectation_gradients, copies);
    deepCopyField(final_cost_gradient, copies);
}


int DeepBeliefNet::outputsize() const
{
    int out_size = 0;
    if( use_classification_cost )
        out_size += n_classes;

    if( final_module )
        out_size += final_module->output_size;
    else
        out_size += layers[n_layers-1]->size;

    return out_size;
/*
    return (n_classes > 0) ? n_classes
                           : targetsize();
*/
}

void DeepBeliefNet::forget()
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

    for( int i=0 ; i<n_layers-1 ; i++ )
        connections[i]->forget();

    if( use_classification_cost )
    {
        classification_cost->forget();
        classification_module->forget();
    }

    if( final_module )
        final_module->forget();

    if( final_cost )
        final_cost->forget();

    if( partial_costs )
        for( int i=0 ; i<n_layers-1 ; i++ )
            if( partial_costs[i] )
                partial_costs[i]->forget();

    stage = 0;
}

void DeepBeliefNet::train()
{
    MODULE_LOG << "train() called " << endl;
    MODULE_LOG << "  training_schedule = " << training_schedule << endl;

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

    Vec input( inputsize() );
    Vec target( targetsize() );
    real weight; // unused

    TVec<string> train_cost_names = getTrainCostNames() ;
    Vec train_costs( train_cost_names.length() );
    train_costs.fill(MISSING_VALUE) ;

    //give the indexes the right values

    if ( classification_cost )
        nll_cost_index = train_cost_names.find(classification_cost->name()[0]);
    if ( final_cost )
        final_cost_index = train_cost_names.find(final_cost->name()[0]);

    recons_cost_index = train_cost_names.find("recons_error");

    class_cost_index = train_cost_names.find("class_error");


    int nsamples = train_set->length();

    if( !initTrain() )
    {
        MODULE_LOG << "train() aborted" << endl;
        return;
    }

    PP<ProgressBar> pb;

    // clear stats of previous epoch
    train_stats->forget();

    /***** initial greedy training *****/
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        if( use_classification_cost && i == n_layers-2 )
            break; // we will do a joint supervised learning instead

        MODULE_LOG << "Training connection weights between layers " << i
            << " and " << i+1 << endl;

        int end_stage = min( training_schedule[i], nstages );

        MODULE_LOG << "  stage = " << stage << endl;
        MODULE_LOG << "  end_stage = " << end_stage << endl;
        MODULE_LOG << "  cd_learning_rate = " << cd_learning_rate << endl;

        if( report_progress && stage < end_stage )
            pb = new ProgressBar( "Training layer "+tostring(i)
                                  +" of "+classname(),
                                  end_stage - stage );

        layers[i]->setLearningRate( cd_learning_rate );
        connections[i]->setLearningRate( cd_learning_rate );
        layers[i+1]->setLearningRate( cd_learning_rate );

        for( ; stage<end_stage ; stage++ )
        {
            int sample = stage % nsamples;
            train_set->getExample(sample, input, target, weight);
            greedyStep( input, target, i );

            if( pb )
                if( i == 0 )
                    pb->update( stage + 1 );
                else
                    pb->update( stage - training_schedule[i-1] + 1 );
        }
    }

    // possible supervised part
    if( use_classification_cost )
    {
        MODULE_LOG << "Training the classification module" << endl;

        int end_stage = min( training_schedule[n_layers-2], nstages );

        MODULE_LOG << "  stage = " << stage << endl;
        MODULE_LOG << "  end_stage = " << end_stage << endl;
        MODULE_LOG << "  cd_learning_rate = " << cd_learning_rate << endl;

        if( report_progress && stage < end_stage )
             pb = new ProgressBar( "Training the classification module",
                                   end_stage - stage );

        // set appropriate learning rate
        setLearningRate( cd_learning_rate );

        int previous_stage = (n_layers < 3) ? 0
                                            : training_schedule[n_layers-3];
        for( ; stage<end_stage ; stage++ )
        {
            int sample = stage % nsamples;
            train_set->getExample( sample, input, target, weight );
            jointGreedyStep( input, target );

            if( pb )
                pb->update( stage - previous_stage + 1 );
        }
    }

    /**** compute reconstruction error*****/
    real train_recons_error = 0.0;

    RBMLayer * down_layer = get_pointer(layers[0]) ;
    RBMLayer * up_layer =  get_pointer(layers[1]) ;
    RBMConnection * parameters = get_pointer(connections[0]);

    for(int train_index = 0 ; train_index < nsamples ; train_index++)
    {

        train_set->getExample( train_index, input, target, weight );

          down_layer->expectation << input;

          // up
          parameters->setAsDownInput( down_layer->expectation );
          up_layer->getAllActivations( parameters );
          up_layer->generateSample();

          // down
          parameters->setAsUpInput( up_layer->sample );

          down_layer->getAllActivations( parameters );
          down_layer->computeExpectation();
          down_layer->generateSample();

          //    result += powdistance( input, down_layer->expectation );

          for( int i=0 ; i<input.size() ; i++ )
              train_recons_error += (input[i] - down_layer->expectation[i])
                                  * (input[i] - down_layer->expectation[i]);

    }

    train_recons_error /= nsamples ;


    /***** fine-tuning by gradient descent *****/
    if( stage >= nstages )
        return;

    MODULE_LOG << "Fine-tuning all parameters, by gradient descent" << endl;
    MODULE_LOG << "  stage = " << stage << endl;
    MODULE_LOG << "  nstages = " << nstages << endl;
    MODULE_LOG << "  grad_learning_rate = " << grad_learning_rate << endl;

    int init_stage = stage;
    if( report_progress && stage < nstages )
        pb = new ProgressBar( "Fine-tuning parameters of all layers of "
                              + classname(),
                              nstages - init_stage );

    setLearningRate( grad_learning_rate );

    int begin_sample = stage % nsamples;
    for( ; stage<nstages ; stage++ )
    {
        int sample = stage % nsamples;
        if( sample == begin_sample )
            train_stats->forget();
        if( !fast_exact_is_equal( grad_decrease_ct, 0. ) )
            setLearningRate( grad_learning_rate
                / (1. + grad_decrease_ct * (stage - init_stage) ) );

        train_set->getExample( sample, input, target, weight );
        fineTuningStep( input, target, train_costs );
        train_stats->update( train_costs );

        if( pb )
            pb->update( stage - init_stage + 1 );
    }

    //update the reconstruction error
    train_costs.fill( MISSING_VALUE );
    train_costs[recons_cost_index] = train_recons_error;
    train_stats->update( train_costs ) ;

    train_stats->finalize();
}

void DeepBeliefNet::greedyStep( const Vec& input, const Vec& target, int index )
{
    PLASSERT( index < n_layers );

    layers[0]->expectation << input;
    for( int i=0 ; i<index ; i++ )
    {
        connections[i]->setAsDownInput( layers[i]->expectation );
        layers[i+1]->getAllActivations( connections[i] );
        layers[i+1]->computeExpectation();
    }

    // TODO: add another learning rate?
    if( partial_costs && partial_costs[ index ] )
    {
        // Deterministic forward pass
        connections[ index ]->setAsDownInput( layers[ index ]->expectation );
        layers[ index+1 ]->getAllActivations( connections[ index ] );
        layers[ index+1 ]->computeExpectation();

        // put appropriate learning rate
        connections[ index ]->setLearningRate( grad_learning_rate );
        layers[ index+1 ]->setLearningRate( grad_learning_rate );

        // Backward pass
        real cost;
        partial_costs[ index+1 ]->fprop( layers[ index+1 ]->expectation,
                                         target, cost );

        partial_costs[ index+1 ]->bpropUpdate( layers[ index+1 ]->expectation,
                                               target, cost,
                                               expectation_gradients[ index+1 ]
                                             );

        layers[ index+1 ]->bpropUpdate( layers[ index+1 ]->activation,
                                        layers[ index+1 ]->expectation,
                                        activation_gradients[ index+1 ],
                                        expectation_gradients[ index+1 ] );

        connections[ index ]->bpropUpdate( layers[ index ]->expectation,
                                           layers[ index+1 ]->activation,
                                           expectation_gradients[ index ],
                                           activation_gradients[ index+1 ] );

        // put back old learning rate
        connections[ index ]->setLearningRate( cd_learning_rate );
        layers[ index+1 ]->setLearningRate( cd_learning_rate );
    }

    contrastiveDivergenceStep( layers[ index ],
                               connections[ index ],
                               layers[ index+1 ] );
}

void DeepBeliefNet::jointGreedyStep( const Vec& input, const Vec& target )
{
    PLASSERT( joint_layer );

    layers[0]->expectation << input;
    for( int i=0 ; i<n_layers-2 ; i++ )
    {
        connections[i]->setAsDownInput( layers[i]->expectation );
        layers[i+1]->getAllActivations( connections[i] );
        layers[i+1]->computeExpectation();
    }

    Vec joint_exp = joint_layer->expectation;
    joint_exp.subVec( 0, layers[ n_layers-2 ]->size )
        << layers[ n_layers-2 ]->expectation;

    fill_one_hot( joint_exp.subVec( layers[ n_layers-2 ]->size, n_classes ),
                  (int) round(target[0]), 0., 1. );

    if( partial_costs && partial_costs[ n_layers-1 ] )
    {
        // Deterministic forward pass
        classification_module->joint_connection->setAsDownInput(
            joint_layer->expectation );
        layers[ n_layers-1 ]->getAllActivations(
            get_pointer( classification_module->joint_connection ) );
        layers[ n_layers-1 ]->computeExpectation();

        // put appropriate learning rate
        classification_module->previous_to_last->setLearningRate(
            grad_learning_rate );
        layers[ n_layers-1 ]->setLearningRate( grad_learning_rate );

        // Backward pass
        real cost;
        partial_costs[ n_layers-1 ]->fprop( layers[ n_layers-1 ]->expectation,
                                            target, cost );

        partial_costs[ n_layers-1 ]->bpropUpdate(
            layers[ n_layers-1 ]->expectation, target, cost,
            expectation_gradients[ n_layers-1 ] );

        layers[ n_layers-1 ]->bpropUpdate( layers[ n_layers-1 ]->activation,
                                           layers[ n_layers-1 ]->expectation,
                                           activation_gradients[ n_layers-1 ],
                                           expectation_gradients[ n_layers-1 ]
                                         );

        classification_module->previous_to_last->bpropUpdate(
            layers[ n_layers-2 ]->expectation,
            layers[ n_layers-1 ]->activation,
            expectation_gradients[ n_layers-2 ],
            activation_gradients[ n_layers-1 ] );

        // put back old learning rate
        classification_module->previous_to_last->setLearningRate(
            cd_learning_rate );
        layers[ n_layers-1 ]->setLearningRate( cd_learning_rate );
    }

    contrastiveDivergenceStep(
        get_pointer( joint_layer ),
        get_pointer( classification_module->joint_connection ),
        layers[ n_layers-1 ] );
}

void DeepBeliefNet::fineTuningStep( const Vec& input, const Vec& target,
                                    Vec& train_costs )
{
    final_cost_value.resize(0);
    // fprop
    layers[0]->expectation << input;
    for( int i=0 ; i<n_layers-2 ; i++ )
    {
        connections[i]->setAsDownInput( layers[i]->expectation );
        layers[i+1]->getAllActivations( connections[i] );
        layers[i+1]->computeExpectation();
    }

    if( final_cost )
    {
        connections[ n_layers-2 ]->setAsDownInput(
            layers[ n_layers-2 ]->expectation );
        layers[ n_layers-1 ]->getAllActivations( connections[ n_layers-2 ] );
        layers[ n_layers-1 ]->computeExpectation();

        if( final_module )
        {
            final_module->fprop( layers[ n_layers-1 ]->expectation,
                                 final_cost_input );
            final_cost->fprop( final_cost_input, target, final_cost_value );

            final_cost->bpropUpdate( final_cost_input, target,
                                     final_cost_value[0],
                                     final_cost_gradient );
            final_module->bpropUpdate( layers[ n_layers-1 ]->expectation,
                                       final_cost_input,
                                       expectation_gradients[ n_layers-1 ],
                                       final_cost_gradient );
        }
        else
        {
            final_cost->fprop( layers[ n_layers-1 ]->expectation, target,
                               final_cost_value );

            final_cost->bpropUpdate( layers[ n_layers-1 ]->expectation,
                                     target, final_cost_value[0],
                                     expectation_gradients[ n_layers-1 ] );
        }

        train_costs[final_cost_index] = final_cost_value[0];

        layers[ n_layers-1 ]->bpropUpdate( layers[ n_layers-1 ]->activation,
                                           layers[ n_layers-1 ]->expectation,
                                           activation_gradients[ n_layers-1 ],
                                           expectation_gradients[ n_layers-1 ]
                                         );

        connections[ n_layers-2 ]->bpropUpdate(
            layers[ n_layers-2 ]->expectation,
            layers[ n_layers-1 ]->activation,
            expectation_gradients[ n_layers-2 ],
            activation_gradients[ n_layers-1 ] );
    }
    else  {
        expectation_gradients[ n_layers-2 ]->clear();
    }

    if( use_classification_cost )
    {
        classification_module->fprop( layers[ n_layers-2 ]->expectation,
                                      class_output );
        real nll_cost;

        // This doesn't work. gcc bug?
        // classification_cost->fprop( class_output, target, cost );
        classification_cost->CostModule::fprop( class_output, target,
                                                nll_cost );

        real class_error =
            ( argmax(class_output) == (int) round(target[0]) ) ? 0
                                                               : 1;

        train_costs[nll_cost_index] = nll_cost;
        train_costs[class_cost_index] = class_error;

        classification_cost->bpropUpdate( class_output, target, nll_cost,
                                          class_gradient );

        classification_module->bpropUpdate( layers[ n_layers-2 ]->expectation,
                                            class_output,
                                            class_input_gradient,
                                            class_gradient );

        expectation_gradients[n_layers-2] += class_input_gradient;
    }

    for( int i=n_layers-2 ; i>0 ; i-- )
    {
        layers[i]->bpropUpdate( layers[i]->activation,
                                layers[i]->expectation,
                                activation_gradients[i],
                                expectation_gradients[i] );

        connections[i-1]->bpropUpdate( layers[i-1]->expectation,
                                       layers[i]->activation,
                                       expectation_gradients[i-1],
                                       activation_gradients[i] );
    }
}

// assumes that down_layer->expectation is set
void DeepBeliefNet::contrastiveDivergenceStep(
    const PP<RBMLayer>& down_layer,
    const PP<RBMConnection>& connection,
    const PP<RBMLayer>& up_layer )
{
    // positive phase
    connection->setAsDownInput( down_layer->expectation );
    up_layer->getAllActivations( connection );
    up_layer->computeExpectation();
    up_layer->generateSample();

    // accumulate positive stats using the expectation
    // we deep-copy because the value will change during negative phase
    pos_down_values.resize( down_layer->size );
    pos_up_values.resize( up_layer->size );

    pos_down_values << down_layer->expectation;
    pos_up_values << up_layer->expectation;

    // down propagation, starting from a sample of up_layer
    connection->setAsUpInput( up_layer->sample );
    down_layer->getAllActivations( connection );
    down_layer->generateSample();

    // negative phase
    connection->setAsDownInput( down_layer->sample );
    up_layer->getAllActivations( connection );
    up_layer->computeExpectation();

    // accumulate negative stats
    // no need to deep-copy because the values won't change before update
    Vec neg_down_values = down_layer->sample;
    Vec neg_up_values = up_layer->expectation;

    // update
    down_layer->update( pos_down_values, neg_down_values );
    connection->update( pos_down_values, pos_up_values,
                        neg_down_values, neg_up_values );
    up_layer->update( pos_up_values, neg_up_values );
}


void DeepBeliefNet::computeOutput(const Vec& input, Vec& output) const
{
    // Compute the output from the input.
    output.resize(0);

    // fprop
    layers[0]->expectation << input;
    for( int i=0 ; i<n_layers-2 ; i++ )
    {
        connections[i]->setAsDownInput( layers[i]->expectation );
        layers[i+1]->getAllActivations( connections[i] );
        layers[i+1]->computeExpectation();
    }


    if( use_classification_cost )
        classification_module->fprop( layers[ n_layers-2 ]->expectation,
                                      output );

    if( final_cost )
    {
        connections[ n_layers-2 ]->setAsDownInput(
            layers[ n_layers-2 ]->expectation );
        layers[ n_layers-1 ]->getAllActivations( connections[ n_layers-2 ] );
        layers[ n_layers-1 ]->computeExpectation();

        if( final_module )
        {
            final_module->fprop( layers[ n_layers-1 ]->expectation,
                                 final_cost_input );
            output.append( final_cost_input );
        }
        else
        {
            output.append( layers[ n_layers-1 ]->expectation );
        }
    }
}

void DeepBeliefNet::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    // Compute the costs from *already* computed output.

    TVec<string> test_cost_names = getTestCostNames() ;

    int test_final_cost_index = -1 ;
    int test_class_cost_index = test_cost_names.find("class_error") ;

    costs.resize( test_cost_names.length() );
    costs.fill( MISSING_VALUE );

    if( use_classification_cost )
    {
        int test_nll_cost_index =
            test_cost_names.find(classification_cost->name()[0]) ;
        real nll_cost;
        classification_cost->CostModule::fprop( output.subVec(0, n_classes),
                target, nll_cost );

        real class_error =
            (argmax(output.subVec(0, n_classes)) == (int) round(target[0]))? 0
            : 1;
        if ( test_nll_cost_index != -1 )
            costs[test_nll_cost_index] = nll_cost;
        if ( test_class_cost_index != -1 )
            costs[test_class_cost_index] = class_error;
    }

    if( final_cost )
    {

        test_final_cost_index = test_cost_names.find(final_cost->name()[0]) ;
        int init = use_classification_cost ? n_classes : 0;
        Vec final_costs;
        final_cost->fprop( output.subVec( init, output.size() - init ),
                           target, final_cost_value );

        if ( final_cost_value.length() > 1 )
            PLERROR("DeepBeliefNet::computeCostsFromOutputs needs to be fixed"
                    "to handle final costs with more then one element") ;

        costs[test_final_cost_index] = final_cost_value[0] ;
        costs.append(final_cost_value[0]);
    }

}

TVec<string> DeepBeliefNet::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).

    TVec<string> cost_names;

    if( use_classification_cost )
    {
        cost_names.append( classification_cost->name() );
        cost_names.append( "class_error" );
    }

    if( final_cost )
        cost_names.append( final_cost->name() );
    return cost_names;
}

TVec<string> DeepBeliefNet::getTrainCostNames() const
{
    TVec<string> cost_names = getTestCostNames() ;
    cost_names.append("recons_error");
    return cost_names;
}


//#####  Helper functions  ##################################################

void DeepBeliefNet::setLearningRate( real the_learning_rate )
{
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        layers[i]->setLearningRate( the_learning_rate );
        connections[i]->setLearningRate( the_learning_rate );
    }
    layers[n_layers-1]->setLearningRate( the_learning_rate );

    if( use_classification_cost )
    {
        classification_module->joint_connection->setLearningRate(
            the_learning_rate );
        joint_layer->setLearningRate( the_learning_rate );
    }

    if( final_module_has_learning_rate )
        final_cost->setOption( "learning_rate",
                               tostring(the_learning_rate ) );

    if( final_cost_has_learning_rate )
        final_cost->setOption( "learning_rate",
                               tostring(the_learning_rate ) );
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
