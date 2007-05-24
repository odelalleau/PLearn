// -*- C++ -*-

// RBMModule.cc
//
// Copyright (C) 2007 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file RBMModule.cc */



#include "RBMModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMModule,
    "A Restricted Boltzmann Machine.",
    "An RBM contains a 'visible_layer', a 'hidden_layer' (both instances of a subclass\n"
    "of RBMLayer) and a 'connection' (an instance of a subclass of RBMConnection).\n"
    "It has two ports: the 'visible' port and the 'hidden' port.\n"
    "The RBM can be trained by gradient descent (wrt to gradients provided on\n"
    "the 'hidden' port) or by contrastive divergence.\n"
);

///////////////
// RBMModule //
///////////////
RBMModule::RBMModule():
    cd_learning_rate(0),
    grad_learning_rate(0),
    n_Gibbs_steps_CD(1)
{
}

void RBMModule::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "visible_layer", &RBMModule::visible_layer,
                  OptionBase::buildoption,
        "Visible layer of the RBM.");

    declareOption(ol, "hidden_layer", &RBMModule::hidden_layer,
                  OptionBase::buildoption,
        "Hidden layer of the RBM.");

    declareOption(ol, "connection", &RBMModule::connection,
                  OptionBase::buildoption,
        "Connection between the visible and hidden layers.");

    declareOption(ol, "reconstruction_connection", 
                  &RBMModule::reconstruction_connection,
                  OptionBase::buildoption,
        "Reconstuction connection between the hidden and visible layers.");

    declareOption(ol, "grad_learning_rate", &RBMModule::grad_learning_rate,
                  OptionBase::buildoption,
        "Learning rate for the gradient descent step.");

    declareOption(ol, "cd_learning_rate", &RBMModule::cd_learning_rate,
                  OptionBase::buildoption,
        "Learning rate for the constrastive divergence step.");

    declareOption(ol, "compute_contrastive_divergence", &RBMModule::compute_contrastive_divergence,
                  OptionBase::buildoption,
        "Compute the constrastive divergence in an output port.");

    declareOption(ol, "n_Gibbs_steps_CD", 
                  &RBMModule::n_Gibbs_steps_CD,
                  OptionBase::buildoption,
                  "Number of Gibbs sampling steps in negative phase of "
                  "contrastive divergence.");

    declareOption(ol, "min_n_Gibbs_steps", &RBMModule::min_n_Gibbs_steps,
                  OptionBase::buildoption,
                  "Used in generative mode (when visible_sample or hidden_sample is requested)\n"
                  "when one has to sample from the joint or a marginal of visible and hidden,\n"
                  "and thus a Gibbs chain has to be run. This option gives the minimum number\n"
                  "of Gibbs steps to perform in the chain before outputting a sample.\n");

    declareOption(ol, "n_Gibbs_steps_per_generated_sample", 
                  &RBMModule::n_Gibbs_steps_per_generated_sample,
                  OptionBase::buildoption,
                  "Used in generative mode (when visible_sample or hidden_sample is requested)\n"
                  "when one has to sample from the joint or a marginal of visible and hidden,\n"
                  "This option gives the number of steps to run in the Gibbs chain between\n"
                  "consecutive generated samples that are produced in output of the fprop method.\n"
                  "By default this is equal to min_n_Gibbs_steps.\n");

    declareOption(ol, "Gibbs_step", 
                  &RBMModule::Gibbs_step,
                  OptionBase::learntoption,
                  "Used in generative mode (when visible_sample or hidden_sample is requested)\n"
                  "when one has to sample from the joint or a marginal of visible and hidden,\n"
                  "Keeps track of the number of steps that have been ran since the beginning\n"
                  "of the chain.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void RBMModule::build_()
{
    PLASSERT( cd_learning_rate >= 0 && grad_learning_rate >= 0 );
    if (fast_exact_is_equal(cd_learning_rate, 0) &&
        fast_exact_is_equal(grad_learning_rate, 0) )
        PLWARNING("In RBMModule::build_ - 'cd_learning_rate' and "
                  "'grad_learning_rate' are set to 0, the RBM will not learn "
                  "much");
    if(visible_layer)
        visible_bias_grad.resize(visible_layer->size);

    // Forward random generator to underlying modules.
    if (random_gen) {
        if (hidden_layer && !hidden_layer->random_gen) {
            hidden_layer->random_gen = random_gen;
            hidden_layer->build();
            hidden_layer->forget();
        }
        if (visible_layer && !visible_layer->random_gen) {
            visible_layer->random_gen = random_gen;
            visible_layer->build();
            visible_layer->forget();
        }
        if (connection && !connection->random_gen) {
            connection->random_gen = random_gen;
            connection->build();
            connection->forget();
        }
        if (reconstruction_connection &&
                !reconstruction_connection->random_gen) {
            reconstruction_connection->random_gen = random_gen;
            reconstruction_connection->build();
            reconstruction_connection->forget();
        }
    }

    // buid ports and port_sizes

    ports.resize(0);
    addportname("visible");
    addportname("hidden.state");
    addportname("hidden_activations.state");
    addportname("visible_sample");
    addportname("hidden_sample");
    addportname("energy");
    // addportname("bias"); port to be added in the near future, to do conditional RBMs
    if(reconstruction_connection)
    {
        addportname("visible_reconstruction.state");
        addportname("visible_reconstruction_activations.state");
        addportname("reconstruction_error.state");
    }
    if (compute_contrastive_divergence)
    {
        addportname("contrastive_divergence");
        addportname("negative_phase_visible_samples.state");
        addportname("negative_phase_hidden_expectations.state");
    }

    port_sizes.resize(nPorts(), 2);
    port_sizes.fill(-1);
    if (visible_layer) {
        port_sizes(portname2index("visible"), 1) = visible_layer->size;
        port_sizes(portname2index("visible_sample"), 1) = visible_layer->size;
    }
    if (hidden_layer) {
        port_sizes(portname2index("hidden.state"), 1) = hidden_layer->size;
        port_sizes(portname2index("hidden_activations.state"), 1) = hidden_layer->size; 
        port_sizes(portname2index("hidden_sample"), 1) = hidden_layer->size; 
    }
    port_sizes(portname2index("energy"),1) = 1;
    if(reconstruction_connection)
    {
        if (visible_layer) {
            port_sizes(portname2index("visible_reconstruction.state"),1) = 
                visible_layer->size; 
            port_sizes(portname2index("visible_reconstruction_activations.state"),1) = 
                       visible_layer->size; 
        }
        port_sizes(portname2index("reconstruction_error.state"),1) = 1; 
    }
    if (compute_contrastive_divergence)
    {
        port_sizes(portname2index("contrastive_divergence"),1) = 1; 
        if (visible_layer) 
            port_sizes(portname2index("negative_phase_visible_samples.state"),1) = visible_layer->size; 
        if (hidden_layer)
            port_sizes(portname2index("negative_phase_hidden_expectations.state"),1) = hidden_layer->size; 
    }
}

///////////
// build //
///////////
void RBMModule::build()
{
    inherited::build();
    build_();
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RBMModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(hidden_layer,     copies);
    deepCopyField(visible_layer,    copies);
    deepCopyField(connection,       copies);
    deepCopyField(reconstruction_connection, copies);

    deepCopyField(hidden_exp_grad, copies);
    deepCopyField(hidden_act_grad, copies);
    deepCopyField(visible_exp_grad, copies);
    deepCopyField(visible_act_grad, copies);
    deepCopyField(visible_bias_grad, copies);

    deepCopyField(portname_to_index, copies);
}

///////////
// fprop //
///////////
void RBMModule::fprop(const Vec& input, Vec& output) const
{
    PLERROR("In RBMModule::fprop - Not implemented");
}

void RBMModule::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( ports_value.length() == nPorts() );
    PLASSERT( visible_layer );
    PLASSERT( hidden_layer );
    PLASSERT( connection );
    Mat* visible = ports_value[portname2index("visible")]; 
    Mat* hidden = ports_value[portname2index("hidden.state")];
    Mat* hidden_act = ports_value[portname2index("hidden_activations.state")];
    Mat* visible_sample = ports_value[portname2index("visible_sample")];
    Mat* hidden_sample = ports_value[portname2index("hidden_sample")];
    Mat* energy = ports_value[portname2index("energy")];
    Mat* visible_reconstruction = 0;
    Mat* visible_reconstruction_activations = 0;
    Mat* reconstruction_error = 0;
    if(reconstruction_connection)
    {
        visible_reconstruction = 
            ports_value[portname2index("visible_reconstruction.state")]; 
        visible_reconstruction_activations = 
            ports_value[portname2index("visible_reconstruction_activaitons.state")];
        reconstruction_error = 
            ports_value[portname2index("reconstruction_error.state")];
    }
    Mat* contrastive_divergence = 0;
    Mat* negative_phase_visible_samples = 0;
    Mat* negative_phase_hidden_expectations = 0;
    if (compute_contrastive_divergence)
    {
        contrastive_divergence = ports_value[portname2index("contrastive_divergence")]; 
        negative_phase_visible_samples = 
            ports_value[portname2index("negative_phase_visible_samples.state")];
        negative_phase_hidden_expectations = 
            ports_value[portname2index("negative_phase_hidden_expectations.state")];
    }

    bool hidden_expectations_are_computed=false;
    bool hidden_activations_are_computed=false;
    bool visible_activations_are_computed=false;
    bool found_a_valid_configuration = false;

    if (visible && !visible->isEmpty())
    // when an input is provided, that would restart the chain for unconditional sampling, from that example
    {
        Gibbs_step = 0; 
        visible_layer->setExpectations(*visible);
    }

    // COMPUTE ENERGY
    if (energy) 
    {
        PLASSERT_MSG( energy->isEmpty(), 
                      "RBMModule: the energy port can only be an output port\n" );
        if (visible && !visible->isEmpty())
        {
            int mbs = visible->length();
            if (hidden && !hidden->isEmpty()) 
                // FULLY OBSERVED CASE
                // we know x and h: energy(h,x) = b'x + c'h + h'Wx
                //  = visible_layer->energy(x) + hidden_layer->energy(h) + dot(h,hidden_layer->activation)
            {
                energy->resize(mbs,1);
                energy->clear();
                for (int i=0;i<mbs;i++)
                {
                    connection->setAsDownInputs(*visible);
                    hidden_layer->getAllActivations(connection, 0, true);
                    hidden_activations_are_computed=true;
                    (*energy)(i,0) = visible_layer->energy((*visible)(i)) + 
                        hidden_layer->energy((*hidden)(i)) + 
                        dot((*hidden)(i),hidden_layer->activations(i));
                }
            } else 
                // FREE-ENERGY(visible) CASE
                // we know x: free energy = -log sum_h e^{-energy(h,x)}
                //                        = b'x + sum_i log sigmoid(-c_i - W_i'x) .... FOR BINOMIAL HIDDEN LAYER
                //                        = visible_layer->energy(x) + sum_i log hidden_layer->expectation[i]
                // or more robustly,      = visible_layer->energy(x) - sum_i softplus(-hidden_layer->activation[i])
            {
                PLASSERT(hidden_layer->classname()=="RBMBinomialLayer");
                energy->resize(mbs,1);
                energy->clear();
                for (int i=0;i<mbs;i++)
                {
                    connection->setAsDownInputs(*visible);
                    hidden_layer->getAllActivations(connection, 0, true);
                    hidden_activations_are_computed=true;
                    (*energy)(i,0) = visible_layer->energy((*visible)(i));
                    for (int j=0;j<hidden_layer->size;j++)
                        (*energy)(i,0) += softplus(hidden_layer->activations(i,j));
                }
            }
        }
        else if (hidden && !hidden->isEmpty())
            // FREE-ENERGY(hidden) CASE
            // we know h: free energy = -log sum_x e^{-energy(h,x)}
            //                        = c'h + sum_i log sigmoid(-b_i - W_{.i}'h) .... FOR BINOMIAL INPUT LAYER
            //                        = hidden_layer->energy(h) + sum_i log visible_layer->expectation[i]
            // or more robustly,      = hidden_layer->energy(h) - sum_i softplus(-visible_layer->activation[i])
        {
            PLASSERT(visible_layer->classname()=="RBMBinomialLayer");
            int mbs = hidden->length();
            energy->resize(mbs,1);
            energy->clear();
            for (int i=0;i<mbs;i++)
            {
                connection->setAsUpInputs(*hidden);
                visible_layer->getAllActivations(connection, 0, true);
                visible_activations_are_computed=true; 
                (*energy)(i,0) = hidden_layer->energy((*hidden)(i));
                for (int j=0;j<visible_layer->size;j++)
                    (*energy)(i,0) += softplus(visible_layer->activations(i,j));
            }
        }
        else 
            PLERROR("RBMModule: unknown configuration to compute energy (currently\n"
                    "only possible if at least visible or hidden are provided).\n");
        found_a_valid_configuration = true;
    }
    // REGULAR FPROP
    // we are given the visible units and we want to compute the hidden
    // activation and/or the hidden expectation
    if ( visible && !visible->isEmpty() &&
         ((hidden && hidden->isEmpty() ) ||
          (hidden_act && hidden_act->isEmpty())) )
    {
        if (!hidden_activations_are_computed)
        {
            connection->setAsDownInputs(*visible); 
            hidden_layer->getAllActivations(connection, 0, true);
            hidden_activations_are_computed=true;
        }
        if (hidden_activations_are_computed && hidden_act) {
            // Also store hidden layer activations.
            PLASSERT( hidden_act->isEmpty() );
            const Mat& to_store = hidden_layer->activations;
            hidden_act->resize(to_store.length(), to_store.width());
            *hidden_act << to_store;
        }
        if (hidden) {
            PLASSERT( hidden->isEmpty() );
            hidden_layer->computeExpectations();
            hidden_expectations_are_computed=true;
            Mat& hidden_out = hidden_layer->getExpectations();
            hidden->resize(hidden_out.length(), hidden_out.width());
            *hidden << hidden_out;
        }
        // Since we return below, the other ports must be unused.
        //PLASSERT( !visible_sample && !hidden_sample );
        found_a_valid_configuration = true;
    } 
    // SAMPLING
    if ((visible_sample && visible_sample->isEmpty()) // it is asked to sample the visible units
        || (hidden_sample && hidden_sample->isEmpty())) // or to sample the hidden units
    {
        PLWARNING("In RBMModule::fprop - sampling in RBMModule has not been tested");
        if (hidden && !hidden->isEmpty()) // sample visible conditionally on hidden
        {
            connection->setAsUpInputs(*hidden);
            visible_layer->getAllActivations(connection, 0, true);
            visible_layer->generateSamples();
            visible_sample->resize(visible_layer->samples.length(),visible_layer->samples.width());
            *visible_sample << visible_layer->samples;
            Gibbs_step = 0; // that would restart the chain for unconditional sampling
        }
        else if (visible && !visible->isEmpty()) // if an input is provided, sample hidden conditionally
        {
            connection->setAsDownInputs(visible_layer->samples);
            hidden_layer->getAllActivations(connection, 0, true);
            hidden_layer->generateSamples();
            Gibbs_step = 0; // that would restart the chain for unconditional sampling
        }
        else // sample unconditionally: Gibbs sample after k steps
        {
            // the visible_layer->expectations contain the "state" from which we
            // start or continue the chain
            int min_n = min(Gibbs_step+n_Gibbs_steps_per_generated_sample,
                            min_n_Gibbs_steps);
            for (;Gibbs_step<min_n;Gibbs_step++)
            {
                connection->setAsDownInputs(visible_layer->samples);
                hidden_layer->getAllActivations(connection, 0, true);
                hidden_layer->generateSamples();
                connection->setAsUpInputs(hidden_layer->samples);
                visible_layer->getAllActivations(connection, 0, true);
                visible_layer->generateSamples();
            }
        }
        if (visible_sample && visible_sample->isEmpty()) // provide sample of the visible units
        {
            visible_sample->resize(visible_layer->samples.length(),
                                   visible_layer->samples.width());
            *visible_sample << visible_layer->samples;
        }
        if (hidden_sample && hidden_sample->isEmpty()) // provide sample of the hidden units
        {
            hidden_sample->resize(hidden_layer->samples.length(),
                                  hidden_layer->samples.width());
            *hidden_sample << hidden_layer->samples;
        }
        found_a_valid_configuration = true;
    }
    // COMPUTE CONTRASTIVE DIVERGENCE CRITERION
    if (contrastive_divergence)
    {
        PLASSERT_MSG( contrastive_divergence->isEmpty(), 
                      "RBMModule: the contrastive_divergence port can only be an output port\n" );
        if (visible && !visible->isEmpty())
        {
            int mbs = visible->length();
            Mat& hidden_expectations = hidden_layer->getExpectations();
            if (!hidden_activations_are_computed) // it must be because neither hidden nor hidden_act were asked
            {
                PLASSERT(!hidden_act);
                connection->setAsDownInputs(*visible); 
                hidden_layer->getAllActivations(connection, 0, true);
                hidden_activations_are_computed=true;
                // we need to save the hidden activations somewhere
                hidden_act_store.resize(mbs,hidden_layer->size);
                hidden_act_store << hidden_layer->activations;
                hidden_act = &hidden_act_store;
            }
            if (!hidden_expectations_are_computed) // it must be because hidden outputs were not asked
            {
                PLASSERT(!hidden);
                hidden_layer->computeExpectations();
                hidden_expectations_are_computed=true;
                // we need to save the hidden expectations somewhere
                hidden_exp_store.resize(mbs,hidden_layer->size);
                hidden_exp_store << hidden_expectations;
                hidden = &hidden_exp_store;
            }
            // perform negative phase
            for( int i=0; i<n_Gibbs_steps_CD; i++)
            {
                hidden_layer->generateSamples();
                // (Negative phase) Generate visible samples.
                connection->setAsUpInputs(hidden_layer->samples);
                visible_layer->getAllActivations(connection, 0, true);
                visible_layer->generateSamples();
                // compute corresponding hidden expectations.
                connection->setAsDownInputs(visible_layer->samples);
                hidden_layer->getAllActivations(connection, 0, true);
                hidden_layer->computeExpectations();
            }
            negative_phase_visible_samples->resize(mbs,visible_layer->size);
            *negative_phase_visible_samples << visible_layer->samples;
            negative_phase_hidden_expectations->resize(hidden_expectations.length(),
                                                       hidden_expectations.width());
            *negative_phase_hidden_expectations << hidden_expectations;

            // compute the energy (again for now only in the binomial case)
            PLASSERT(hidden_layer->classname()=="RBMBinomialLayer");

            // note that hidden_act and hidden may point to hidden_act_store and hidden_exp_store
            PLASSERT(!hidden_act->isEmpty()); 
            PLASSERT(!hidden->isEmpty());

            contrastive_divergence->resize(hidden_expectations.length(),1);
            // compute contrastive divergence itself
            for (int i=0;i<mbs;i++)
            {
                (*contrastive_divergence)(i,0) = 
                    // positive phase energy
                    visible_layer->energy((*visible)(i))
                    + hidden_layer->energy((*hidden)(i))
                    + dot((*hidden)(i),(*hidden_act)(i))
                    // minus
                    - 
                    // negative phase energy
                    (visible_layer->energy(visible_layer->samples(i))
                     + hidden_layer->energy(hidden_expectations(i))
                     + dot(hidden_expectations(i),hidden_layer->activations(i)));
            }
        }
        else
            PLERROR("RBMModule: unknown configuration to compute contrastive_divergence (currently\n"
                    "only possible if only visible are provided in input).\n");
        found_a_valid_configuration = true;
    }
    // COMPUTE AUTO-ASSOCIATOR RECONSTRUCTION ERROR
    if ( visible && !visible->isEmpty() && 
         ( ( visible_reconstruction && visible_reconstruction->isEmpty() ) || 
           ( visible_reconstruction_activations && 
             visible_reconstruction_activations->isEmpty() ) ||
           ( reconstruction_error && reconstruction_error->isEmpty() ) ) ) 
    {        
        // Autoassociator reconstruction cost
        PLASSERT( ports_value.length() == nPorts() );
        if (!hidden_activations_are_computed)
        {
            connection->setAsDownInputs(*visible); 
            hidden_layer->getAllActivations(connection, 0, true);
            hidden_activations_are_computed=true;
        }
        if(!hidden_expectations_are_computed)
        {
            hidden_layer->computeExpectations();
            hidden_expectations_are_computed=true;
        }
        if (hidden_activations_are_computed && hidden_act && hidden_act->isEmpty()) {
            const Mat& to_store = hidden_layer->activations;
            hidden_act->resize(to_store.length(), to_store.width());
            *hidden_act << to_store;
        }
        // Don't need to verify if they are asked in a port, this was done previously
        
        reconstruction_connection->setAsDownInputs(hidden_layer->getExpectations());
        visible_layer->getAllActivations(
            reconstruction_connection, 0, true);
        if(visible_reconstruction_activations) 
        {
            PLASSERT( visible_reconstruction_activations->isEmpty() );
            const Mat& to_store = visible_layer->activations;
            visible_reconstruction_activations->resize(to_store.length(), 
                                                       to_store.width());
            *visible_reconstruction_activations << to_store;
        }
        if (visible_reconstruction || reconstruction_error)
        {        
            visible_layer->computeExpectations();
            if(visible_reconstruction)
            {
                PLASSERT( visible_reconstruction->isEmpty() );
                const Mat& to_store = visible_layer->getExpectations();
                visible_reconstruction->resize(to_store.length(), 
                                                           to_store.width());
                *visible_reconstruction << to_store;
            }
            if(reconstruction_error)
            {
                PLASSERT( reconstruction_error->isEmpty() );
                reconstruction_error->resize(visible->length(),1);
                visible_layer->fpropNLL(*visible,
                                        *reconstruction_error);
            }
        }
        found_a_valid_configuration = true;
    }

    if (!found_a_valid_configuration)
        PLERROR("In RBMModule::fprop - Unknown port configuration");
}

////////////////////
// bpropAccUpdate //
////////////////////
void RBMModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                               const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_gradient.length() == nPorts() );
    Mat* visible_grad = ports_gradient[0];
    Mat* hidden_grad = ports_gradient[1];
    Mat* visible = ports_value[0];
    Mat* hidden = ports_value[1];
    Mat* hidden_act = ports_value[2];
    Mat* reconstruction_error_grad = 0;
    
    if(reconstruction_connection)
        reconstruction_error_grad = ports_gradient[8];

    if (visible && !visible->isEmpty() && 
        (hidden_grad && !hidden_grad->isEmpty() &&
         (!visible_grad || visible_grad->isEmpty())))
    {
        int mbs = visible->length();
        if (grad_learning_rate > 0) {
            setAllLearningRates(grad_learning_rate);
            PLASSERT( hidden && hidden_act );
            // Compute gradient w.r.t. activations of the hidden layer.
            hidden_layer->bpropUpdate(
                    *hidden_act, *hidden, hidden_act_grad, *hidden_grad,
                    false);
            Mat* visible_out = ports_value[0];
            // Compute gradient w.r.t. expectations of the visible layer (=
            // inputs).
            Mat* store_visible_grad = NULL;
            if (visible_grad) {
                PLASSERT( visible_grad->width() == visible_layer->size );
                store_visible_grad = visible_grad;
            } else {
                // We do not actually need to store the gradient, but since it
                // is required in bpropUpdate, we provide a dummy matrix to
                // store it.
                store_visible_grad = &visible_exp_grad;
            }
            store_visible_grad->resize(mbs,visible_layer->size);
            connection->bpropUpdate(
                    *visible_out, *hidden_act, *store_visible_grad,
                    hidden_act_grad, true);
        }
    } 

    if (cd_learning_rate > 0) {
        // Perform a step of contrastive divergence.
        PLASSERT( visible && !visible->isEmpty() );
            int mbs = visible->length();
            setAllLearningRates(cd_learning_rate);
            PLASSERT( ports_value.length() == nPorts() );
            Mat* negative_phase_visible_samples = 
                compute_contrastive_divergence?ports_value[reconstruction_connection?10:7]:0;
            Mat* negative_phase_hidden_expectations = 
                compute_contrastive_divergence?ports_value[reconstruction_connection?11:8]:0;
            PLASSERT( visible && hidden );
            if (!negative_phase_visible_samples || negative_phase_visible_samples->isEmpty())
            {
                // Generate hidden samples.
                hidden_layer->setExpectations(*hidden);
                for( int i=0; i<n_Gibbs_steps_CD; i++)
                {
                    hidden_layer->generateSamples();
                    // (Negative phase) Generate visible samples.
                    connection->setAsUpInputs(hidden_layer->samples);
                    visible_layer->getAllActivations(connection, 0, true);
                    visible_layer->generateSamples();
                    // compute corresponding hidden expectations.
                    connection->setAsDownInputs(visible_layer->samples);
                    hidden_layer->getAllActivations(connection, 0, true);
                    hidden_layer->computeExpectations();
                }
                if (!negative_phase_hidden_expectations)
                    negative_phase_hidden_expectations = &(hidden_layer->getExpectations());
                else
                {
                    PLASSERT(negative_phase_hidden_expectations->isEmpty());
                    negative_phase_hidden_expectations->resize(mbs,hidden_layer->size);
                    *negative_phase_hidden_expectations << hidden_layer->getExpectations();
                }
                if (!negative_phase_visible_samples)
                    negative_phase_visible_samples = &(visible_layer->samples);
                else
                {
                    PLASSERT(negative_phase_visible_samples->isEmpty());
                    negative_phase_visible_samples->resize(mbs,visible_layer->size);
                    *negative_phase_visible_samples << visible_layer->samples;
                }
            }
            // Perform update.
            visible_layer->update(*visible, *negative_phase_visible_samples);
            connection->update(*visible, *hidden,
                               *negative_phase_visible_samples,
                               *negative_phase_hidden_expectations);
            hidden_layer->update(*hidden, *negative_phase_hidden_expectations);
    }

    if (reconstruction_error_grad && !reconstruction_error_grad->isEmpty()
        && ( !visible_grad || visible_grad->isEmpty() ) ) {
        setAllLearningRates(grad_learning_rate);
        PLASSERT( reconstruction_connection != 0 );
        // Perform gradient descent on Autoassociator reconstruction cost
        PLASSERT( ports_value.length() == nPorts() );
        Mat* hidden = ports_value[1];
        Mat* hidden_act = ports_value[2];
        Mat* visible_reconstruction = ports_value[6];
        Mat* visible_reconstruction_activations = ports_value[7];
        Mat* reconstruction_error = ports_value[8];
        PLASSERT( hidden != 0 );
        PLASSERT( visible  && hidden_act &&
                  visible_reconstruction && visible_reconstruction_activations &&
                  reconstruction_error);
        int mbs = reconstruction_error_grad->length();

        // Backprop reconstruction gradient

        // Must change visible_layer's expectation
        visible_layer->getExpectations() << *visible_reconstruction;
        visible_layer->bpropNLL(*visible,*reconstruction_error,
                                visible_act_grad);

        // Combine with incoming gradient
        PLASSERT( (*reconstruction_error_grad).width() == 1 );
        real* m_i = visible_act_grad.data();
        real* vv;
        for(int i=0; i<visible_act_grad.length(); 
            i++, m_i+=visible_act_grad.mod())
        {
            vv = (*reconstruction_error_grad).data();
            for(int j=0; j<visible_act_grad.width(); 
                j++, vv += (*reconstruction_error_grad).mod())
                m_i[j] *= *vv;
        }

        // Visible bias update
        columnSum(visible_act_grad,visible_bias_grad);
        visible_layer->update(visible_bias_grad);
        // Reconstruction connection update
        reconstruction_connection->bpropUpdate(
            *hidden, *visible_reconstruction_activations,
            hidden_exp_grad, visible_act_grad, false);
        // Hidden layer bias update
        hidden_layer->bpropUpdate(*hidden_act,
                                  *hidden, hidden_act_grad,
                                  hidden_exp_grad, false);
        // Connection update
        if(visible_grad)
        {
            PLASSERT( visible_grad->width() == visible_layer->size );
            connection->bpropUpdate(
                *visible, *hidden_act,
                *visible_grad, hidden_act_grad, true);
        }
        else
        {
            visible_exp_grad.resize(mbs,visible_layer->size);        
            connection->bpropUpdate(
                *visible, *hidden_act,
                visible_exp_grad, hidden_act_grad, true);
        }
    }

    checkProp(ports_gradient);
}

////////////
// forget //
////////////
void RBMModule::forget()
{
    PLASSERT( hidden_layer && visible_layer && connection );
    hidden_layer->forget();
    visible_layer->forget();
    connection->forget();
    if (reconstruction_connection)
        reconstruction_connection->forget();
}

//////////////
// getPorts //
//////////////
const TVec<string>& RBMModule::getPorts()
{
    return ports;
}

///////////////////
// getPortsSizes //
///////////////////
const TMat<int>& RBMModule::getPortSizes()
{
    return port_sizes;
}

//////////////////////
// bpropDoesNothing //
//////////////////////
/* THIS METHOD IS OPTIONAL
bool RBMModule::bpropDoesNothing()
{
}
*/

/////////////////////////
// setAllLearningRates //
/////////////////////////
void RBMModule::setAllLearningRates(real lr)
{
    hidden_layer->setLearningRate(lr);
    visible_layer->setLearningRate(lr);
    connection->setLearningRate(lr);
    if(reconstruction_connection)
        reconstruction_connection->setLearningRate(lr);
}

/////////////////////
// setLearningRate //
/////////////////////
void RBMModule::setLearningRate(real dynamic_learning_rate)
{
    // Out of safety, force the user to go through the two different learning
    // rate. May need to be removed if it causes unwanted crashes.
    PLERROR("In RBMModule::setLearningRate - Do not use this method, instead "
            "explicitely use 'cd_learning_rate' and 'grad_learning_rate'");
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
