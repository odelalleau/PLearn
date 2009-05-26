// -*- C++ -*-

// PseudolikelihoodRBM.cc
//
// Copyright (C) 2008 Hugo Larochelle
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

/*! \file PseudolikelihoodRBM.cc */


#define PL_LOG_MODULE_NAME "PseudolikelihoodRBM"
#include "PseudolikelihoodRBM.h"
#include <plearn_learners/online/RBMLayer.h>
#include <plearn/io/pl_log.h>
#include <plearn/math/TMat_sort.h>

#define minibatch_hack 0 // Do we force the minibatch setting? (debug hack)

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PseudolikelihoodRBM,
    "Restricted Boltzmann Machine trained by (generalized) pseudolikelihood.",
    "");

///////////////////
// PseudolikelihoodRBM //
///////////////////
PseudolikelihoodRBM::PseudolikelihoodRBM() :
    learning_rate( 0. ),
    decrease_ct( 0. ),
    cd_learning_rate( 0. ),
    cd_decrease_ct( 0. ),
    cd_n_gibbs( 1 ),
    persistent_cd_weight( 0. ),
    n_gibbs_chains( 1 ),
    use_mean_field_cd( false ),
    denoising_learning_rate( 0. ),
    denoising_decrease_ct( 0. ),
    fraction_of_masked_inputs( 0. ),
    only_reconstruct_masked_inputs( false ),
    n_classes( -1 ),
    input_is_sparse( false ),
    factorized_connection_rank( -1 ),
    n_selected_inputs_pseudolikelihood( -1 ),
    n_selected_inputs_cd( -1 ),
    //select_among_k_most_frequent( -1 ),
    compute_input_space_nll( false ),
    compute_Z_exactly( true ),
    use_ais_to_compute_Z( false ),
    n_ais_chains( 100 ),
    pseudolikelihood_context_size ( 0 ),
    pseudolikelihood_context_type( "uniform_random" ),
    k_most_correlated( -1 ),
    generative_learning_weight( 0 ),
    semi_sup_learning_weight( 0. ),
    nll_cost_index( -1 ),
    log_Z_cost_index( -1 ),
    log_Z_ais_cost_index( -1 ),
    log_Z_interval_lower_cost_index( -1 ),
    log_Z_interval_upper_cost_index( -1 ),
    class_cost_index( -1 ),
    training_cpu_time_cost_index ( -1 ),
    cumulative_training_time_cost_index ( -1 ),
    //cumulative_testing_time_cost_index ( -1 ),
    cumulative_training_time( 0 ),
    //cumulative_testing_time( 0 ),
    log_Z( MISSING_VALUE ),
    log_Z_ais( MISSING_VALUE ),
    log_Z_down( MISSING_VALUE ),
    log_Z_up( MISSING_VALUE ),
    Z_is_up_to_date( false ),
    Z_ais_is_up_to_date( false )
{
    random_gen = new PRandom();
}

////////////////////
// declareOptions //
////////////////////
void PseudolikelihoodRBM::declareOptions(OptionList& ol)
{
    declareOption(ol, "learning_rate", &PseudolikelihoodRBM::learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used for pseudolikelihood training.\n"
                  "Pseudolikelihood training assumes input_layer is a\n"
                  "RBMBinomialLayer. It will work even if it isn't,\n"
                  "but training won't be appropriate.\n");

    declareOption(ol, "decrease_ct", &PseudolikelihoodRBM::decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate.\n");

    declareOption(ol, "cd_learning_rate", &PseudolikelihoodRBM::cd_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used for contrastive divergence learning.\n");

    declareOption(ol, "cd_decrease_ct", &PseudolikelihoodRBM::cd_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the contrastive divergence "
                  "learning rate.\n");

    declareOption(ol, "cd_n_gibbs", &PseudolikelihoodRBM::cd_n_gibbs,
                  OptionBase::buildoption,
                  "Number of negative phase gibbs sampling steps.\n");

    declareOption(ol, "persistent_cd_weight", 
                  &PseudolikelihoodRBM::persistent_cd_weight,
                  OptionBase::buildoption,
                  "Weight of Persistent Contrastive Divergence, i.e. "
                  "weight of the prolonged gibbs chain.\n");

    declareOption(ol, "n_gibbs_chains", 
                  &PseudolikelihoodRBM::n_gibbs_chains,
                  OptionBase::buildoption,
                  "Number of gibbs chains maintained in parallel for "
                  "Persistent Contrastive Divergence.\n");

    declareOption(ol, "use_mean_field_cd", &PseudolikelihoodRBM::use_mean_field_cd,
                  OptionBase::buildoption,
                  "Indication that a mean-field version of Contrastive "
                  "Divergence (MF-CD) should be used.\n");

    declareOption(ol, "denoising_learning_rate", 
                  &PseudolikelihoodRBM::denoising_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used for denoising autoencoder learning.\n");

    declareOption(ol, "denoising_decrease_ct", 
                  &PseudolikelihoodRBM::denoising_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the denoising autoencoder "
                  "learning rate.\n");

    declareOption(ol, "fraction_of_masked_inputs", 
                  &PseudolikelihoodRBM::fraction_of_masked_inputs,
                  OptionBase::buildoption,
                  "Fraction of input components set to 0 for denoising "
                  "autoencoder learning.\n");

    declareOption(ol, "only_reconstruct_masked_inputs", 
                  &PseudolikelihoodRBM::only_reconstruct_masked_inputs,
                  OptionBase::buildoption,
                  "Indication that only the masked inputs should be reconstructed.\n");

    declareOption(ol, "n_classes", &PseudolikelihoodRBM::n_classes,
                  OptionBase::buildoption,
                  "Number of classes in the training set (for supervised learning).\n"
                  "If < 2, unsupervised learning will be performed.\n"
                  );

    declareOption(ol, "input_is_sparse", &PseudolikelihoodRBM::input_is_sparse,
                  OptionBase::buildoption,
                  "Indication that the input is in a sparse format. Input is also assumed\n"
                  "to be binary.\n"
                  );

    declareOption(ol, "factorized_connection_rank", &PseudolikelihoodRBM::factorized_connection_rank,
                  OptionBase::buildoption,
                  "Rank of factorized connection for sparse inputs.\n"
                  );    

    declareOption(ol, "n_selected_inputs_pseudolikelihood", 
                  &PseudolikelihoodRBM::n_selected_inputs_pseudolikelihood,
                  OptionBase::buildoption,
                  "Number of randomly selected inputs for pseudolikelihood cost."
                  "This option is ignored for pseudolikelihood_context_size > 0.\n"
                  );    

    declareOption(ol, "n_selected_inputs_cd", 
                  &PseudolikelihoodRBM::n_selected_inputs_cd,
                  OptionBase::buildoption,
                  "Number of randomly selected inputs for CD in sparse "
                  "input case.\n"
                  "Note that CD for sparse inputs assumes RBMBinomialLayer in "
                  "input.\n"
                  );    

    //declareOption(ol, "select_among_k_most_frequent", 
    //              &PseudolikelihoodRBM::select_among_k_most_frequent,
    //              OptionBase::buildoption,
    //              "Indication that inputs for pseudolikelihood cost are selected among the\n"
    //              "k most frequently active inputs.\n"
    //              );    

    declareOption(ol, "compute_input_space_nll", 
                  &PseudolikelihoodRBM::compute_input_space_nll,
                  OptionBase::buildoption,
                  "Indication that the input space NLL should be "
                  "computed during test. It will require a procedure to compute\n"
                  "the partition function Z, which can be exact (see compute_Z_exactly)\n"
                  "or approximate (see use_ais_to_compute_Z). If both are true,\n"
                  "exact computation will be used.\n"
                  );

    declareOption(ol, "compute_Z_exactly",
                  &PseudolikelihoodRBM::compute_Z_exactly,
                  OptionBase::buildoption,
                  "Indication that the partition function Z should be computed exactly.\n"
                  );

    declareOption(ol, "use_ais_to_compute_Z",
                  &PseudolikelihoodRBM::use_ais_to_compute_Z,
                  OptionBase::buildoption,
                  "Whether to use AIS (see Salakhutdinov and Murray ICML2008) to\n"
                  "compute Z. Assumes the input layer is an RBMBinomialLayer.\n"
                  );

    declareOption(ol, "n_ais_chains", 
                  &PseudolikelihoodRBM::n_ais_chains,
                  OptionBase::buildoption,
                  "Number of AIS chains.\n"
                  );

    declareOption(ol, "ais_beta_begin", 
                  &PseudolikelihoodRBM::ais_beta_begin,
                  OptionBase::buildoption,
                  "List of interval beginnings, used to specify the beta schedule.\n"
                  "Its first element is always set to 0.\n"
                  );

    declareOption(ol, "ais_beta_end", 
                  &PseudolikelihoodRBM::ais_beta_end,
                  OptionBase::buildoption,
                  "List of interval ends, used to specify the beta schedule.\n"
                  "Its last element is always set to 1.\n"
                  );

    declareOption(ol, "ais_beta_n_steps", 
                  &PseudolikelihoodRBM::ais_beta_n_steps,
                  OptionBase::buildoption,
                  "Number of steps in each of the beta interval, used to "
                  "specify the beta schedule.\n"
                  );

    declareOption(ol, "pseudolikelihood_context_size", 
                  &PseudolikelihoodRBM::pseudolikelihood_context_size,
                  OptionBase::buildoption,
                  "Number of additional input variables chosen to form the joint\n"
                  "condition likelihoods in generalized pseudolikelihood\n"
                  "(default = 0, which corresponds to standard pseudolikelihood).\n"
                  );

    declareOption(ol, "pseudolikelihood_context_type", 
                  &PseudolikelihoodRBM::pseudolikelihood_context_type,
                  OptionBase::buildoption,
                  "Type of context for generalized pseudolikelihood:\n"
                  "\"uniform_random\": context elements are picked uniformly randomly\n"
                  "\n"
                  "- \"most_correlated\": the most correlated (positively or negatively\n"
                  "                     elemenst with the current input element are picked\n"
                  "\n"
                  "- \"most_correlated_uniform_random\": context elements are picked uniformly\n"
                  "                                    among the k_most_correlated other input\n"
                  "                                    elements, for each current input\n"
                  );

    declareOption(ol, "k_most_correlated", 
                  &PseudolikelihoodRBM::k_most_correlated,
                  OptionBase::buildoption,
                  "Number of most correlated input elements over which to sample.\n"
                  );

    declareOption(ol, "generative_learning_weight", 
                  &PseudolikelihoodRBM::generative_learning_weight,
                  OptionBase::buildoption,
                  "Weight of generative learning.\n"
                  );

    declareOption(ol, "semi_sup_learning_weight", 
                  &PseudolikelihoodRBM::semi_sup_learning_weight,
                  OptionBase::buildoption,
                  "Weight on unlabeled examples update during unsupervised learning.\n"
                  "In other words, it's the same thing at generaitve_learning_weight,\n"
                  "but for the unlabeled examples.\n");

    declareOption(ol, "input_layer", &PseudolikelihoodRBM::input_layer,
                  OptionBase::buildoption,
                  "The binomial input layer of the RBM.\n");

    declareOption(ol, "hidden_layer", &PseudolikelihoodRBM::hidden_layer,
                  OptionBase::buildoption,
                  "The hidden layer of the RBM.\n");

    declareOption(ol, "connection", &PseudolikelihoodRBM::connection,
                  OptionBase::buildoption,
                  "The connection weights between the input and hidden layer.\n");

    declareOption(ol, "cumulative_training_time", 
                  &PseudolikelihoodRBM::cumulative_training_time,
                  //OptionBase::learntoption | OptionBase::nosave,
                  OptionBase::learntoption,
                  "Cumulative training time since age=0, in seconds.\n");

//    declareOption(ol, "cumulative_testing_time", 
//                  &PseudolikelihoodRBM::cumulative_testing_time,
//                  //OptionBase::learntoption | OptionBase::nosave,
//                  OptionBase::learntoption,
//                  "Cumulative testing time since age=0, in seconds.\n");


    declareOption(ol, "target_layer", &PseudolikelihoodRBM::target_layer,
                  OptionBase::learntoption,
                  "The target layer of the RBM.\n");

    declareOption(ol, "target_connection", &PseudolikelihoodRBM::target_connection,
                  OptionBase::learntoption,
                  "The connection weights between the target and hidden layer.\n");

    declareOption(ol, "U", &PseudolikelihoodRBM::U,
                  OptionBase::learntoption,
                  "First connection factorization matrix.\n");

    declareOption(ol, "V", &PseudolikelihoodRBM::V,
                  OptionBase::learntoption,
                  "If factorized_connection_rank > 0, second connection "
                  "factorization matrix. Otherwise, input connections.\n");

    declareOption(ol, "log_Z", &PseudolikelihoodRBM::log_Z,
                  OptionBase::learntoption,
                  "Normalisation constant, computed exactly (on log scale).\n");

    declareOption(ol, "log_Z_ais", &PseudolikelihoodRBM::log_Z_ais,
                  OptionBase::learntoption,
                  "Normalisation constant, computed by AIS (on log scale).\n");

    declareOption(ol, "log_Z_down", &PseudolikelihoodRBM::log_Z_down,
                  OptionBase::learntoption,
                  "Lower bound of confidence interval for log_Z.\n");

    declareOption(ol, "log_Z_up", &PseudolikelihoodRBM::log_Z_up,
                  OptionBase::learntoption,
                  "Upper bound of confidence interval for log_Z.\n");

    declareOption(ol, "Z_is_up_to_date", &PseudolikelihoodRBM::Z_is_up_to_date,
                  OptionBase::learntoption,
                  "Indication that the normalisation constant Z (computed exactly) "
                  "is up to date.\n");

    declareOption(ol, "Z_ais_is_up_to_date", &PseudolikelihoodRBM::Z_ais_is_up_to_date,
                  OptionBase::learntoption,
                  "Indication that the normalisation constant Z (computed with AIS) "
                  "is up to date.\n");

    declareOption(ol, "persistent_gibbs_chain_is_started", 
                  &PseudolikelihoodRBM::persistent_gibbs_chain_is_started,
                  OptionBase::learntoption,
                  "Indication that the prolonged gibbs chain for "
                  "Persistent Consistent Divergence is started, for each chain.\n");

//    declareOption(ol, "target_weights_L1_penalty_factor", 
//                  &PseudolikelihoodRBM::target_weights_L1_penalty_factor,
//                  OptionBase::buildoption,
//                  "Target weights' L1_penalty_factor.\n");
//
//    declareOption(ol, "target_weights_L2_penalty_factor", 
//                  &PseudolikelihoodRBM::target_weights_L2_penalty_factor,
//                  OptionBase::buildoption,
//                  "Target weights' L2_penalty_factor.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void PseudolikelihoodRBM::build_()
{
    MODULE_LOG << "build_() called" << endl;

    if( inputsize_ > 0 && targetsize_ >= 0)
    {
        if( compute_input_space_nll && targetsize() > 0 )
            PLERROR("In PseudolikelihoodRBM::build_(): compute_input_space_nll "
                    "is not compatible with targetsize() > 0");

        if( compute_input_space_nll && input_is_sparse )
            PLERROR("In PseudolikelihoodRBM::build_(): compute_input_space_nll "
                    "is not compatible with sparse inputs");

        if( pseudolikelihood_context_size < 0 )
            PLERROR("In PseudolikelihoodRBM::build_(): "
                    "pseudolikelihood_context_size should be >= 0.");

        if( pseudolikelihood_context_type != "uniform_random" &&
            pseudolikelihood_context_type != "most_correlated" &&
            pseudolikelihood_context_type != "most_correlated_uniform_random" )
            PLERROR("In PseudolikelihoodRBM::build_(): "
                    "pseudolikelihood_context_type is not valid.");

        if( pseudolikelihood_context_type == "most_correlated"
            && pseudolikelihood_context_size <= 0 )
            PLERROR("In PseudolikelihoodRBM::build_(): "
                    "pseudolikelihood_context_size should be > 0 "
                    "for \"most_correlated\" context type");        

        if( compute_input_space_nll && use_ais_to_compute_Z )
        {
            if( n_ais_chains <= 0 )
                PLERROR("In PseudolikelihoodRBM::build_(): "
                        "n_ais_chains should be > 0.");
            if( ais_beta_n_steps.length() == 0 )
                PLERROR("In PseudolikelihoodRBM::build_(): "
                        "AIS schedule should have at least 1 interval of betas.");
            if( ais_beta_n_steps.length() != ais_beta_begin.length() ||
                ais_beta_n_steps.length() != ais_beta_end.length() )
                PLERROR("In PseudolikelihoodRBM::build_(): "
                        "ais_beta_begin, ais_beta_end and ais_beta_n_steps should "
                        "all be of the same length.");
        }

        build_layers_and_connections();
        build_costs();

        // Activate the profiler
        Profiler::activate();
    }
}

/////////////////
// build_costs //
/////////////////
void PseudolikelihoodRBM::build_costs()
{
    cost_names.resize(0);
    
    int current_index = 0;
    if( compute_input_space_nll || targetsize() > 0 )
    {
        cost_names.append("NLL");
        nll_cost_index = current_index;
        current_index++;
        if( compute_Z_exactly )
        {
            cost_names.append("log_Z");
            log_Z_cost_index = current_index++;
        }
        
        if( use_ais_to_compute_Z )
        {
            cost_names.append("log_Z_ais");
            log_Z_ais_cost_index = current_index++;
            cost_names.append("log_Z_interval_lower");
            log_Z_interval_lower_cost_index = current_index++;
            cost_names.append("log_Z_interval_upper");
            log_Z_interval_upper_cost_index = current_index++;
        }
    }
    
    if( targetsize() > 0 )
    {
        cost_names.append("class_error");
        class_cost_index = current_index;
        current_index++;
    }

    cost_names.append("cpu_time");
    cost_names.append("cumulative_train_time");
    //cost_names.append("cumulative_test_time");

    training_cpu_time_cost_index = current_index;
    current_index++;
    cumulative_training_time_cost_index = current_index;
    current_index++;
    //cumulative_testing_time_cost_index = current_index;
    //current_index++;


    PLASSERT( current_index == cost_names.length() );
}

//////////////////////////////////
// build_layers_and_connections //
//////////////////////////////////
void PseudolikelihoodRBM::build_layers_and_connections()
{
    MODULE_LOG << "build_layers_and_connections() called" << endl;

    if( !input_layer )
        PLERROR("In PseudolikelihoodRBM::build_layers_and_connections(): "
                "input_layer must be provided");
    if( !hidden_layer )
        PLERROR("In PseudolikelihoodRBM::build_layers_and_connections(): "
                "hidden_layer must be provided");

    if( targetsize() == 1 )
    {
        if( n_classes <= 1 )
            PLERROR("In PseudolikelihoodRBM::build_layers_and_connections(): "
                    "n_classes should be > 1");
        if( !target_layer || target_layer->size != n_classes )
        {
            target_layer = new RBMMultinomialLayer();
            target_layer->size = n_classes;
            target_layer->random_gen = random_gen;
            target_layer->build();
            target_layer->forget();
        }
        
        if( !target_connection || 
            target_connection->up_size != hidden_layer->size ||
            target_connection->down_size != target_layer->size )
        {
            target_connection = new RBMMatrixConnection(); 
            target_connection->up_size = hidden_layer->size;
            target_connection->down_size = target_layer->size;
            target_connection->random_gen = random_gen;
            target_connection->build();
            target_connection->forget();
        }
    }
    else if ( targetsize() > 1 )
    {
        if( !target_layer || target_layer->size != targetsize() )
        {
            target_layer = new RBMBinomialLayer();
            target_layer->size = targetsize();
            target_layer->random_gen = random_gen;
            target_layer->build();
            target_layer->forget();
        }
        
        if( !target_connection || 
            target_connection->up_size != hidden_layer->size ||
            target_connection->down_size != target_layer->size )
        {
            target_connection = new RBMMatrixConnection(); 
            target_connection->up_size = hidden_layer->size;
            target_connection->down_size = target_layer->size;
            target_connection->random_gen = random_gen;
            target_connection->build();
            target_connection->forget();
        }
    }

    if( !connection && !input_is_sparse )
        PLERROR("PseudolikelihoodRBM::build_layers_and_connections(): \n"
                "connection must be provided");

    if( input_is_sparse )
    {
        if( factorized_connection_rank > 0 )
        {
            U.resize( hidden_layer->size, factorized_connection_rank );
            V.resize( inputsize(), factorized_connection_rank );
            Vx.resize( factorized_connection_rank );

            U_gradient.resize( hidden_layer->size, factorized_connection_rank );
            Vx_gradient.resize( factorized_connection_rank );
        }
        else
        {
            V.resize( inputsize(), hidden_layer->size );
        }
        input_is_active.resize( inputsize() );
        input_is_active.clear();
        hidden_act_non_selected.resize( hidden_layer->size );
        // CD option
        pos_hidden.resize( hidden_layer->size );
        pos_input_sparse.resize( input_layer->size );
        pos_input_sparse.clear();
    }
    else
    {
        if( connection->up_size != hidden_layer->size ||
            connection->down_size != input_layer->size )
            PLERROR("PseudolikelihoodRBM::build_layers_and_connections(): \n"
                    "connection's size (%d x %d) should be %d x %d",
                    connection->up_size, connection->down_size,
                    hidden_layer->size, input_layer->size);
        connection_gradient.resize( connection->up_size, connection->down_size );

        if( !connection->random_gen )
        {
            connection->random_gen = random_gen;
            connection->forget();
        }

        // CD option
        pos_hidden.resize( hidden_layer->size );
        pers_cd_hidden.resize( n_gibbs_chains );
        for( int i=0; i<n_gibbs_chains; i++ )
        {
            pers_cd_hidden[i].resize( hidden_layer->size );
        }
        if( persistent_gibbs_chain_is_started.length() != n_gibbs_chains )
        {
            persistent_gibbs_chain_is_started.resize( n_gibbs_chains );
            persistent_gibbs_chain_is_started.fill( false );
        }

        // Denoising autoencoder options
        transpose_connection = new RBMMatrixTransposeConnection;
        transpose_connection->rbm_matrix_connection = connection;
        transpose_connection->build();
        reconstruction_activation_gradient.resize(input_layer->size);
        hidden_layer_expectation_gradient.resize(hidden_layer->size);
        hidden_layer_activation_gradient.resize(hidden_layer->size);
        masked_autoencoder_input.resize(input_layer->size);
        autoencoder_input_indices.resize(input_layer->size);
        for(int i=0; i<input_layer->size; i++)
            autoencoder_input_indices[i] = i;
    }
        
    input_gradient.resize( input_layer->size );
    hidden_activation_pos_i.resize( hidden_layer->size );
    hidden_activation_neg_i.resize( hidden_layer->size );
    hidden_activation_gradient.resize( hidden_layer->size );
    hidden_activation_pos_i_gradient.resize( hidden_layer->size );
    hidden_activation_neg_i_gradient.resize( hidden_layer->size );


    // Generalized pseudolikelihood option
    context_indices.resize( input_layer->size - 1);
    if( pseudolikelihood_context_size > 0 )
    {
        context_indices_per_i.resize( input_layer->size, 
                                      pseudolikelihood_context_size );

        int n_conf = ipow(2, pseudolikelihood_context_size);
        nums_act.resize( 2 * n_conf );
        gnums_act.resize( 2 * n_conf );
        context_probs.resize( 2 * n_conf );
        hidden_activations_context.resize( 2*n_conf, hidden_layer->size );
        hidden_activations_context_k_gradient.resize( hidden_layer->size );
    }



    if( inputsize_ >= 0 )
        PLASSERT( input_layer->size == inputsize() );

    if( targetsize() > 0 )
    {
        class_output.resize( target_layer->size );
        class_gradient.resize( target_layer->size );
        target_one_hot.resize( target_layer->size );
        
        pos_target.resize( target_layer->size );
        neg_target.resize( target_layer->size );
    }

    if( !input_layer->random_gen )
    {
        input_layer->random_gen = random_gen;
        input_layer->forget();
    }

    if( !hidden_layer->random_gen )
    {
        hidden_layer->random_gen = random_gen;
        hidden_layer->forget();
    }
}

///////////
// build //
///////////
void PseudolikelihoodRBM::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PseudolikelihoodRBM::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(input_layer, copies);
    deepCopyField(hidden_layer, copies);
    deepCopyField(connection, copies);
    deepCopyField(cost_names, copies);
    deepCopyField(transpose_connection, copies);
    deepCopyField(target_layer, copies);
    deepCopyField(target_connection, copies);
    deepCopyField(U, copies);
    deepCopyField(V, copies);

    deepCopyField(target_one_hot, copies);
    deepCopyField(input_gradient, copies);
    deepCopyField(class_output, copies);
    deepCopyField(class_gradient, copies);
    deepCopyField(hidden_activation_pos_i, copies);
    deepCopyField(hidden_activation_neg_i, copies);
    deepCopyField(hidden_activation_gradient, copies);
    deepCopyField(hidden_activation_pos_i_gradient, copies);
    deepCopyField(hidden_activation_neg_i_gradient, copies);
    deepCopyField(connection_gradient, copies);
    deepCopyField(context_indices, copies);
    deepCopyField(context_indices_per_i, copies);
    deepCopyField(correlations_per_i, copies);
    deepCopyField(context_most_correlated, copies);
    deepCopyField(hidden_activations_context, copies);
    deepCopyField(hidden_activations_context_k_gradient, copies);
    deepCopyField(nums, copies);
    deepCopyField(nums_act, copies);
    deepCopyField(context_probs, copies);
    deepCopyField(gnums_act, copies);
    deepCopyField(conf, copies);
    deepCopyField(pos_input, copies);
    deepCopyField(pos_target, copies);
    deepCopyField(pos_hidden, copies);
    deepCopyField(neg_input, copies);
    deepCopyField(neg_target, copies);
    deepCopyField(neg_hidden, copies);
    deepCopyField(reconstruction_activation_gradient, copies);
    deepCopyField(hidden_layer_expectation_gradient, copies);
    deepCopyField(hidden_layer_activation_gradient, copies);
    deepCopyField(masked_autoencoder_input, copies);
    deepCopyField(autoencoder_input_indices, copies);
    deepCopyField(pers_cd_hidden, copies);
    deepCopyField(Vx, copies);
    deepCopyField(U_gradient, copies);
    deepCopyField(Vx_gradient, copies);
    deepCopyField(V_gradients, copies);
    deepCopyField(input_is_active, copies);
    deepCopyField(input_indices, copies);
    deepCopyField(input_is_selected, copies);
    deepCopyField(hidden_act_non_selected, copies);
    deepCopyField(pos_input_sparse, copies);
    deepCopyField(persistent_gibbs_chain_is_started, copies);
}


////////////////
// outputsize //
////////////////
int PseudolikelihoodRBM::outputsize() const
{
    return targetsize() > 0 ? target_layer->size : hidden_layer->size;
}

////////////
// forget //
////////////
void PseudolikelihoodRBM::forget()
{
    inherited::forget();

    input_layer->forget();
    hidden_layer->forget();
    if( connection )
        connection->forget();

    cumulative_training_time = 0;
    //cumulative_testing_time = 0;
    Z_is_up_to_date = false;
    Z_ais_is_up_to_date = false;

    persistent_gibbs_chain_is_started.fill( false );
    correlations_per_i.resize(0,0);

    if( U.size() != 0 )
    {
        real d = 1. / max( U.length(), U.width() );
        random_gen->fill_random_uniform( U, -d, d );
    }
    
    if( V.size() != 0 )
        V.clear();

    if( target_layer )
        target_layer->forget();

    if( target_connection )
        target_connection->forget();
}

///////////
// train //
///////////
void PseudolikelihoodRBM::train()
{
    MODULE_LOG << "train() called " << endl;

    MODULE_LOG << "stage = " << stage
               << ", target nstages = " << nstages << endl;

    PLASSERT( train_set );

    Vec input( inputsize() );
    Vec target( targetsize() );
    Vec extra( 1 );
    int target_index;
    real weight; // unused
    real lr;
    int weightsize = train_set->weightsize();

    //real mean_pseudolikelihood = 0;

    TVec<string> train_cost_names = getTrainCostNames() ;
    Vec train_costs( train_cost_names.length() );
    train_costs.fill(MISSING_VALUE) ;

    int nsamples = train_set->length();
    int init_stage = stage;
    if( !initTrain() )
    {
        MODULE_LOG << "train() aborted" << endl;
        return;
    }

    PP<ProgressBar> pb;

    // clear stats of previous epoch
    train_stats->forget();

    if( report_progress )
        pb = new ProgressBar( "Training "
                              + classname(),
                              nstages - stage );

    // Start the actual time counting
    Profiler::reset("training");
    Profiler::start("training");

    for( ; stage<nstages ; stage++ )
    {
        Z_is_up_to_date = false;
        Z_ais_is_up_to_date = false;
        train_set->getExample(stage%nsamples, input, target, weight);

        if( pb )
            pb->update( stage - init_stage + 1 );

        if( targetsize() == 1 )
        {
            target_one_hot.clear();
            if( !is_missing(target[0]) )
            {
                target_index = (int)round( target[0] );
                target_one_hot[ target_index ] = 1;
            }
        }
//        else
//        {

        // Discriminative learning is the sum of all learning rates
        lr = 0;

        if( !fast_exact_is_equal(decrease_ct, 0) ) 
            lr += learning_rate / (1.0 + stage * decrease_ct );
        else 
            lr += learning_rate;

        if( !fast_exact_is_equal(cd_decrease_ct, 0) ) 
            lr += cd_learning_rate / (1.0 + stage * cd_decrease_ct );
        else 
            lr += cd_learning_rate;
        
        if( !fast_exact_is_equal(denoising_decrease_ct, 0) ) 
            lr += denoising_learning_rate / (1.0 + stage * denoising_decrease_ct );
        else 
            lr += denoising_learning_rate;

        if( weightsize > 0 )
            lr *= weight;

        setLearningRate(lr);

        if( targetsize() == 1 && !is_missing(target[0]) )
        {
            Vec target_act = target_layer->activation;
            Vec hidden_act = hidden_layer->activation;

            // For gradient verification
            //Mat estimated_gradient(connection->up_size, connection->down_size);
            //{
            //    connection->setAsDownInput( input );
            //    hidden_layer->getAllActivations( 
            //        (RBMMatrixConnection*) connection );
            //    
            //    target_act = target_layer->activation;
            //    hidden_act = hidden_layer->activation;
            //    for( int i=0 ; i<target_layer->size ; i++ )
            //    {
            //        target_act[i] = target_layer->bias[i];
            //        // LATERAL CONNECTIONS CODE HERE!!
            //        real *w = &(target_connection->weights(0,i));
            //        // step from one row to the next in weights matrix
            //        int m = target_connection->weights.mod();                
            //        
            //        for( int j=0 ; j<hidden_layer->size ; j++, w+=m )
            //        {
            //            // *w = weights(j,i)
            //            hidden_activation_pos_i[j] = hidden_act[j] + *w;
            //        }
            //        target_act[i] -= hidden_layer->freeEnergyContribution(
            //            hidden_activation_pos_i);
            //    }
            //    
            //    target_layer->expectation_is_up_to_date = false;
            //    target_layer->computeExpectation();
            //    real true_nll = target_layer->fpropNLL(target_one_hot);
            //    
            //    estimated_gradient.fill(true_nll);
            //    
            //    real epsilon = 1e-5;
            //    for( int i1=0; i1<connection->up_size; i1++)
            //        for( int j1=0; j1<connection->down_size; j1++)
            //        {
            //            connection->weights(i1,j1) += epsilon;
            //            connection->setAsDownInput( input );
            //            hidden_layer->getAllActivations( 
            //                (RBMMatrixConnection*) connection );
            //            
            //            Vec target_act = target_layer->activation;
            //            Vec hidden_act = hidden_layer->activation;
            //            for( int i=0 ; i<target_layer->size ; i++ )
            //            {
            //                target_act[i] = target_layer->bias[i];
            //                // LATERAL CONNECTIONS CODE HERE!!
            //                real *w = &(target_connection->weights(0,i));
            //                // step from one row to the next in weights matrix
            //                int m = target_connection->weights.mod();                
            //                
            //                for( int j=0 ; j<hidden_layer->size ; j++, w+=m )
            //                {
            //                    // *w = weights(j,i)
            //                    hidden_activation_pos_i[j] = hidden_act[j] + *w;
            //                }
            //                target_act[i] -= hidden_layer->freeEnergyContribution(
            //                    hidden_activation_pos_i);
            //            }
            //            
            //            target_layer->expectation_is_up_to_date = false;
            //            target_layer->computeExpectation();
            //            real nll = target_layer->fpropNLL(target_one_hot);
            //            
            //            estimated_gradient(i1,j1) = (nll - estimated_gradient(i1,j1) )/epsilon;
            //            connection->weights(i1,j1) -= epsilon;
            //        }
            //}

            // For gradient verification of target connections
            //Mat estimated_target_gradient(target_connection->up_size, target_connection->down_size);
            //{
            //    connection->setAsDownInput( input );
            //    hidden_layer->getAllActivations( 
            //        (RBMMatrixConnection*) connection );
            //    
            //    target_act = target_layer->activation;
            //    hidden_act = hidden_layer->activation;
            //    for( int i=0 ; i<target_layer->size ; i++ )
            //    {
            //        target_act[i] = target_layer->bias[i];
            //        // LATERAL CONNECTIONS CODE HERE!!
            //        real *w = &(target_connection->weights(0,i));
            //        // step from one row to the next in weights matrix
            //        int m = target_connection->weights.mod();                
            //        
            //        for( int j=0 ; j<hidden_layer->size ; j++, w+=m )
            //        {
            //            // *w = weights(j,i)
            //            hidden_activation_pos_i[j] = hidden_act[j] + *w;
            //        }
            //        target_act[i] -= hidden_layer->freeEnergyContribution(
            //            hidden_activation_pos_i);
            //    }
            //    
            //    target_layer->expectation_is_up_to_date = false;
            //    target_layer->computeExpectation();
            //    real true_nll = target_layer->fpropNLL(target_one_hot);
            //    
            //    estimated_target_gradient.fill(true_nll);
            //    
            //    real epsilon = 1e-5;
            //    for( int i1=0; i1<target_connection->up_size; i1++)
            //        for( int j1=0; j1<target_connection->down_size; j1++)
            //        {
            //            target_connection->weights(i1,j1) += epsilon;
            //            connection->setAsDownInput( input );
            //            hidden_layer->getAllActivations( 
            //                (RBMMatrixConnection*) connection );
            //            
            //            Vec target_act = target_layer->activation;
            //            Vec hidden_act = hidden_layer->activation;
            //            for( int i=0 ; i<target_layer->size ; i++ )
            //            {
            //                target_act[i] = target_layer->bias[i];
            //                // LATERAL CONNECTIONS CODE HERE!!
            //                real *w = &(target_connection->weights(0,i));
            //                // step from one row to the next in weights matrix
            //                int m = target_connection->weights.mod();                
            //                
            //                for( int j=0 ; j<hidden_layer->size ; j++, w+=m )
            //                {
            //                    // *w = weights(j,i)
            //                    hidden_activation_pos_i[j] = hidden_act[j] + *w;
            //                }
            //                target_act[i] -= hidden_layer->freeEnergyContribution(
            //                    hidden_activation_pos_i);
            //            }
            //            
            //            target_layer->expectation_is_up_to_date = false;
            //            target_layer->computeExpectation();
            //            real nll = target_layer->fpropNLL(target_one_hot);
            //            
            //            estimated_target_gradient(i1,j1) = (nll - estimated_target_gradient(i1,j1) )/epsilon;
            //            target_connection->weights(i1,j1) -= epsilon;
            //        }
            //}

            // Multi-class classification
            
            if( input_is_sparse )
            {
                if( factorized_connection_rank > 0 )
                {
                    Vx.clear();
                    train_set->getExtra(stage%nsamples,extra);
                    input_is_active.clear();
                    for( int i=0; i<extra.length(); i++ )
                    {
                        Vx += V((int)extra[i]);
                        input_is_active[(int)extra[i]] = true;
                    }
                    
                    product(hidden_act,U,Vx);
                }
                else
                {
                    hidden_act.clear();
                    train_set->getExtra(stage%nsamples,extra);
                    for( int i=0; i<extra.length(); i++ )
                    {
                        hidden_act += V((int)extra[i]);
                        input_is_active[(int)extra[i]] = true;
                    }
                }
                hidden_act += hidden_layer->bias;
            }
            else
            {
                connection->setAsDownInput( input );
                hidden_layer->getAllActivations( 
                    (RBMMatrixConnection*) connection );
            }

            for( int i=0 ; i<target_layer->size ; i++ )
            {
                target_act[i] = target_layer->bias[i];
                // LATERAL CONNECTIONS CODE HERE!!
                real *w = &(target_connection->weights(0,i));
                // step from one row to the next in weights matrix
                int m = target_connection->weights.mod();                
                
                for( int j=0 ; j<hidden_layer->size ; j++, w+=m )
                {
                    // *w = weights(j,i)
                    hidden_activation_pos_i[j] = hidden_act[j] + *w;
                }
                target_act[i] -= hidden_layer->freeEnergyContribution(
                    hidden_activation_pos_i);
            }
            
            target_layer->expectation_is_up_to_date = false;
            target_layer->computeExpectation();
            real nll = target_layer->fpropNLL(target_one_hot);
            train_costs[nll_cost_index] = nll;
            train_costs[class_cost_index] = 
                (argmax(target_layer->expectation) == target_index)? 0 : 1;
            target_layer->bpropNLL(target_one_hot,nll,class_gradient);

            hidden_activation_gradient.clear();

            //Mat target_real_gradient(target_connection->up_size, target_connection->down_size);
            for( int i=0 ; i<target_layer->size ; i++ )
            {
                real *w = &(target_connection->weights(0,i));
                // step from one row to the next in weights matrix
                int m = target_connection->weights.mod();                
                
                for( int j=0 ; j<hidden_layer->size ; j++, w+=m )
                {
                    // *w = weights(j,i)
                    hidden_activation_pos_i[j] = hidden_act[j] + *w;
                }
                hidden_layer->freeEnergyContributionGradient(
                    hidden_activation_pos_i,
                    hidden_activation_pos_i_gradient,
                    -class_gradient[i],
                    false
                    );
                hidden_activation_gradient += hidden_activation_pos_i_gradient;

                // Update target connections
                w = &(target_connection->weights(0,i));
                //real* gw = &(target_real_gradient(0,i));
                //int gm = target_real_gradient.mod();
                for( int j=0 ; j<hidden_layer->size ; j++, w+=m )
                {
                    *w -= lr * hidden_activation_pos_i_gradient[j];
                    //*gw += hidden_activation_pos_i_gradient[j];
                    //gw += gm;
                }
                    
            }

            //real cos_ang = dot(connection_gradient.toVec(),estimated_gradient.toVec())
            //    / (norm(connection_gradient.toVec()) *norm(estimated_gradient.toVec()));
            //cout << "cos_ang=" << cos_ang << endl;
            //cout << "ang=" << acos(cos_ang) << endl;

            //real cos_target_ang = dot(target_real_gradient.toVec(),estimated_target_gradient.toVec())
            //    / (norm(target_real_gradient.toVec()) *norm(estimated_target_gradient.toVec()));
            //cout << "cos_target_ang=" << cos_target_ang << endl;
            //cout << "target_ang=" << acos(cos_target_ang) << endl;

            // Update target bias            
            multiplyScaledAdd(class_gradient, 1.0, -lr,
                              target_layer->bias);
            // Hidden bias update
            multiplyScaledAdd(hidden_activation_gradient, 1.0, -lr,
                              hidden_layer->bias);

            if( input_is_sparse )
            {
                if( factorized_connection_rank > 0 )
                {
                    externalProduct( U_gradient, hidden_activation_gradient,
                                     Vx );
                    transposeProduct( Vx_gradient, U, hidden_activation_gradient );
                    for( int i=0; i<extra.length(); i++ )
                    {
                        V((int)extra[i]) -= lr * Vx_gradient;
                        input_is_active[(int)extra[i]] = false;
                    }
                    
                    multiplyScaledAdd( U_gradient, 1.0, -lr,
                                       U );
                }
                else
                {
                    for( int i=0; i<extra.length(); i++ )
                    {
                        V((int)extra[i]) -= lr * hidden_activation_gradient;
                        input_is_active[(int)extra[i]] = false;
                    }
                }

            }
            else
            {
                externalProduct( connection_gradient, hidden_activation_gradient,
                                 input );

                // Connection weights update
                multiplyScaledAdd( connection_gradient, 1.0, -lr,
                                   connection->weights );
            }
        }
        if( targetsize() > 1 )
        {
            // Multi-task binary classification
            PLERROR("NNNNNNNNNNOOOOOOOOOOOOOOOOOOOOOO!!!!!!!!!!!!!!");
        }

        if( !fast_exact_is_equal(learning_rate, 0.) &&
            (targetsize() == 0 || generative_learning_weight > 0) )
        {
            if( !fast_exact_is_equal(decrease_ct, 0) )
                lr = learning_rate / (1.0 + stage * decrease_ct );
            else
                lr = learning_rate;

            if( targetsize() > 0 )
                lr *= generative_learning_weight;
            
            if( weightsize > 0 )
                lr *= weight;

            setLearningRate(lr);

            if( is_missing(target[0]) )
                PLERROR("In PseudolikelihoodRBM::train(): generative training with "
                        "unlabeled examples not supported for pseudolikehood training.");

            if( pseudolikelihood_context_size == 0 )
            {
                // Compute input_probs
                //
                //  a = W x + c
                //  for i in 1...d
                //      num_pos = b_i
                //      num_neg = 0
                //      for j in 1...h
                //          num_pos += softplus( a_j - W_ji x_i + W_ji)
                //          num_neg += softplus( a_j - W_ji x_i)
                //      p_i = exp(num_pos) / (exp(num_pos) + exp(num_neg))

                Vec hidden_act = hidden_layer->activation;

                real num_pos_act;
                real num_neg_act;
                real num_pos;
                real num_neg;
                real* a = hidden_layer->activation.data();
                real* a_pos_i = hidden_activation_pos_i.data();
                real* a_neg_i = hidden_activation_neg_i.data();
                real* w, *gw;
                int m;
                if( connection )
                    m = connection->weights.mod();
                real input_i, input_probs_i;
                real pseudolikelihood = 0;
                real* ga_pos_i = hidden_activation_pos_i_gradient.data();
                real* ga_neg_i = hidden_activation_neg_i_gradient.data();

                // Randomly select inputs
                if( n_selected_inputs_pseudolikelihood <= inputsize() &&
                    n_selected_inputs_pseudolikelihood > 0 )
                {
                    if ( input_indices.length() == 0 )
                    {
                        input_indices.resize(inputsize());
                        for( int i=0; i<input_indices.length(); i++ )
                            input_indices[i] = i;
                        
                    }
                 
                    // Randomly selected inputs
                    int tmp;
                    int k;
                    for (int j = 0; j < n_selected_inputs_pseudolikelihood; j++) 
                    {
                        k = j + 
                            random_gen->uniform_multinomial_sample(
                                inputsize() - j);
                        
                        tmp = input_indices[j];
                        input_indices[j] = input_indices[k];
                        input_indices[k] = tmp;
                    }
                }

                // Resize V_gradients
                if( input_is_sparse )
                {
                    int n_V_gradients;
                    if( n_selected_inputs_pseudolikelihood <= inputsize() &&
                        n_selected_inputs_pseudolikelihood > 0 )
                        n_V_gradients = n_selected_inputs_pseudolikelihood;
                    else
                        n_V_gradients = inputsize();

                    if( factorized_connection_rank > 0 )
                        V_gradients.resize(
                            n_V_gradients,
                            factorized_connection_rank );
                    else
                        V_gradients.resize(
                            n_V_gradients,
                            hidden_layer->size );
                }

                //Mat estimated_gradient;
                //Mat U_estimated_gradient;
                //{
                //    real epsilon=1e-5;
                //    // Empirically estimate gradient
                //    if( input_is_sparse )
                //    {
                //        estimated_gradient.resize(V.length(), V.width());
                //        U_estimated_gradient.resize(U.length(), U.width() );
                //
                //        int i=0;
                //        pseudolikelihood = 0;
                //
                //        // Compute activations
                //        if( input_is_sparse )
                //        {
                //            if( factorized_connection_rank > 0 )
                //            {
                //                Vx.clear();
                //                train_set->getExtra(stage%nsamples,extra);
                //                for( int i=0; i<extra.length(); i++ )
                //                {
                //                    Vx += V((int)extra[i]);
                //                    input_is_active[(int)extra[i]] = true;
                //                }
                //        
                //                product(hidden_act,U,Vx);
                //            }
                //            else
                //            {
                //                hidden_act.clear();
                //                train_set->getExtra(stage%nsamples,extra);
                //                for( int i=0; i<extra.length(); i++ )
                //                {
                //                    hidden_act += V((int)extra[i]);
                //                    input_is_active[(int)extra[i]] = true;
                //                }
                //            }
                //            hidden_act += hidden_layer->bias;
                //        }
                //        else
                //        {
                //            connection->setAsDownInput( input );
                //            hidden_layer->getAllActivations( 
                //                (RBMMatrixConnection*) connection );
                //        }
                //
                //        if( targetsize() == 1 )
                //            productAcc( hidden_layer->activation,
                //                        target_connection->weights,
                //                        target_one_hot );
                //        else if( targetsize() > 1 )
                //            productAcc( hidden_layer->activation,
                //                        target_connection->weights,
                //                        target );
                //
                //        for( int l=0; l<input_layer->size ; l++ )
                //        {
                //            if( n_selected_inputs_pseudolikelihood <= inputsize() &&
                //                n_selected_inputs_pseudolikelihood > 0 )
                //            {
                //                if( l >= n_selected_inputs_pseudolikelihood )
                //                    break;
                //                i = input_indices[l];
                //            }
                //            else
                //                i = l;
                //            
                //            num_pos_act = input_layer->bias[i];
                //            // LATERAL CONNECTIONS CODE HERE!
                //            num_neg_act = 0;
                //            if( input_is_sparse )
                //            {
                //                hidden_activation_pos_i << hidden_act;
                //                hidden_activation_neg_i << hidden_act;
                //                if( factorized_connection_rank > 0 )
                //                    if( input_is_active[i] )
                //                    {
                //                        input_i = 1;
                //                        productScaleAcc( hidden_activation_neg_i,
                //                                         U, V(i), -1.,1.);
                //                    }
                //                    else
                //                    {
                //                        input_i = 0;
                //                        productScaleAcc( hidden_activation_pos_i,
                //                                         U, V(i), 1.,1.);
                //                    }
                //                else
                //                    if( input_is_active[i] )
                //                    {
                //                        input_i = 1;
                //                        hidden_activation_neg_i -= V(i);
                //                    }
                //                    else
                //                    {
                //                        input_i = 0;
                //                        hidden_activation_pos_i += V(i);
                //                    }
                //            }
                //            else
                //            {
                //                w = &(connection->weights(0,i));
                //                input_i = input[i];
                //                for( int j=0; j<hidden_layer->size; j++,w+=m )
                //                {
                //                    a_pos_i[j] = a[j] - *w * ( input_i - 1 );
                //                    a_neg_i[j] = a[j] - *w * input_i;
                //                }
                //            }
                //            num_pos_act -= hidden_layer->freeEnergyContribution(
                //                hidden_activation_pos_i);
                //            num_neg_act -= hidden_layer->freeEnergyContribution(
                //                hidden_activation_neg_i);
                //            //num_pos = safeexp(num_pos_act);
                //            //num_neg = safeexp(num_neg_act);
                //            //input_probs_i = num_pos / (num_pos + num_neg);
                //            if( input_layer->use_fast_approximations )
                //                input_probs_i = fastsigmoid(
                //                    num_pos_act - num_neg_act);
                //            else
                //            {
                //                num_pos = safeexp(num_pos_act);
                //                num_neg = safeexp(num_neg_act);
                //                input_probs_i = num_pos / (num_pos + num_neg);
                //            }
                //            if( input_layer->use_fast_approximations )
                //                pseudolikelihood += tabulated_softplus( 
                //                    num_pos_act - num_neg_act ) 
                //                    - input_i * (num_pos_act - num_neg_act);
                //            else
                //                pseudolikelihood += softplus( 
                //                    num_pos_act - num_neg_act ) 
                //                    - input_i * (num_pos_act - num_neg_act);
                //
                //        }
                //
                //        estimated_gradient.fill(pseudolikelihood);
                //
                //        for( int i1=0; i1<estimated_gradient.length(); i1++)
                //            for( int j1=0; j1<estimated_gradient.width(); j1++)
                //            {
                //                V(i1,j1) += epsilon;
                //                pseudolikelihood = 0;
                //
                //                // Compute activations
                //                if( input_is_sparse )
                //                {
                //                    if( factorized_connection_rank > 0 )
                //                    {
                //                        Vx.clear();
                //                        train_set->getExtra(stage%nsamples,extra);
                //                        for( int i=0; i<extra.length(); i++ )
                //                        {
                //                            Vx += V((int)extra[i]);
                //                            input_is_active[(int)extra[i]] = true;
                //                        }
                //        
                //                        product(hidden_act,U,Vx);
                //                    }
                //                    else
                //                    {
                //                        hidden_act.clear();
                //                        train_set->getExtra(stage%nsamples,extra);
                //                        for( int i=0; i<extra.length(); i++ )
                //                        {
                //                            hidden_act += V((int)extra[i]);
                //                            input_is_active[(int)extra[i]] = true;
                //                        }
                //                    }
                //                    hidden_act += hidden_layer->bias;
                //                }
                //                else
                //                {
                //                    connection->setAsDownInput( input );
                //                    hidden_layer->getAllActivations( 
                //                        (RBMMatrixConnection*) connection );
                //                }
                //
                //                if( targetsize() == 1 )
                //                    productAcc( hidden_layer->activation,
                //                                target_connection->weights,
                //                                target_one_hot );
                //                else if( targetsize() > 1 )
                //                    productAcc( hidden_layer->activation,
                //                                target_connection->weights,
                //                                target );
                //
                //                for( int l=0; l<input_layer->size ; l++ )
                //                {
                //                    if( n_selected_inputs_pseudolikelihood <= inputsize() &&
                //                        n_selected_inputs_pseudolikelihood > 0 )
                //                    {
                //                        if( l >= n_selected_inputs_pseudolikelihood )
                //                            break;
                //                        i = input_indices[l];
                //                    }
                //                    else
                //                        i = l;
                //            
                //                    num_pos_act = input_layer->bias[i];
                //                    // LATERAL CONNECTIONS CODE HERE!
                //                    num_neg_act = 0;
                //                    if( input_is_sparse )
                //                    {
                //                        hidden_activation_pos_i << hidden_act;
                //                        hidden_activation_neg_i << hidden_act;
                //                        if( factorized_connection_rank > 0 )
                //                            if( input_is_active[i] )
                //                            {
                //                                input_i = 1;
                //                                productScaleAcc( hidden_activation_neg_i,
                //                                                 U, V(i), -1.,1.);
                //                            }
                //                            else
                //                            {
                //                                input_i = 0;
                //                                productScaleAcc( hidden_activation_pos_i,
                //                                                 U, V(i), 1.,1.);
                //                            }
                //                        else
                //                            if( input_is_active[i] )
                //                            {
                //                                input_i = 1;
                //                                hidden_activation_neg_i -= V(i);
                //                            }
                //                            else
                //                            {
                //                                input_i = 0;
                //                                hidden_activation_pos_i += V(i);
                //                            }
                //                    }
                //                    else
                //                    {
                //                        w = &(connection->weights(0,i));
                //                        input_i = input[i];
                //                        for( int j=0; j<hidden_layer->size; j++,w+=m )
                //                        {
                //                            a_pos_i[j] = a[j] - *w * ( input_i - 1 );
                //                            a_neg_i[j] = a[j] - *w * input_i;
                //                        }
                //                    }
                //                    num_pos_act -= hidden_layer->freeEnergyContribution(
                //                        hidden_activation_pos_i);
                //                    num_neg_act -= hidden_layer->freeEnergyContribution(
                //                        hidden_activation_neg_i);
                //                    //num_pos = safeexp(num_pos_act);
                //                    //num_neg = safeexp(num_neg_act);
                //                    //input_probs_i = num_pos / (num_pos + num_neg);
                //                    if( input_layer->use_fast_approximations )
                //                        input_probs_i = fastsigmoid(
                //                            num_pos_act - num_neg_act);
                //                    else
                //                    {
                //                        num_pos = safeexp(num_pos_act);
                //                        num_neg = safeexp(num_neg_act);
                //                        input_probs_i = num_pos / (num_pos + num_neg);
                //                    }
                //                    if( input_layer->use_fast_approximations )
                //                        pseudolikelihood += tabulated_softplus( 
                //                            num_pos_act - num_neg_act ) 
                //                            - input_i * (num_pos_act - num_neg_act);
                //                    else
                //                        pseudolikelihood += softplus( 
                //                            num_pos_act - num_neg_act ) 
                //                            - input_i * (num_pos_act - num_neg_act);
                //
                //                }
                //                V(i1,j1) -= epsilon;
                //                estimated_gradient(i1,j1) = (pseudolikelihood - estimated_gradient(i1,j1))
                //                    / epsilon;
                //            }
                //
                //        if( factorized_connection_rank > 0 )
                //        {
                //
                //        pseudolikelihood = 0;
                //
                //        // Compute activations
                //        if( input_is_sparse )
                //        {
                //            if( factorized_connection_rank > 0 )
                //            {
                //                Vx.clear();
                //                train_set->getExtra(stage%nsamples,extra);
                //                for( int i=0; i<extra.length(); i++ )
                //                {
                //                    Vx += V((int)extra[i]);
                //                    input_is_active[(int)extra[i]] = true;
                //                }
                //        
                //                product(hidden_act,U,Vx);
                //            }
                //            else
                //            {
                //                hidden_act.clear();
                //                train_set->getExtra(stage%nsamples,extra);
                //                for( int i=0; i<extra.length(); i++ )
                //                {
                //                    hidden_act += V((int)extra[i]);
                //                    input_is_active[(int)extra[i]] = true;
                //                }
                //            }
                //            hidden_act += hidden_layer->bias;
                //        }
                //        else
                //        {
                //            connection->setAsDownInput( input );
                //            hidden_layer->getAllActivations( 
                //                (RBMMatrixConnection*) connection );
                //        }
                //
                //        if( targetsize() == 1 )
                //            productAcc( hidden_layer->activation,
                //                        target_connection->weights,
                //                        target_one_hot );
                //        else if( targetsize() > 1 )
                //            productAcc( hidden_layer->activation,
                //                        target_connection->weights,
                //                        target );
                //
                //        for( int l=0; l<input_layer->size ; l++ )
                //        {
                //            if( n_selected_inputs_pseudolikelihood <= inputsize() &&
                //                n_selected_inputs_pseudolikelihood > 0 )
                //            {
                //                if( l >= n_selected_inputs_pseudolikelihood )
                //                    break;
                //                i = input_indices[l];
                //            }
                //            else
                //                i = l;
                //            
                //            num_pos_act = input_layer->bias[i];
                //            // LATERAL CONNECTIONS CODE HERE!
                //            num_neg_act = 0;
                //            if( input_is_sparse )
                //            {
                //                hidden_activation_pos_i << hidden_act;
                //                hidden_activation_neg_i << hidden_act;
                //                if( factorized_connection_rank > 0 )
                //                    if( input_is_active[i] )
                //                    {
                //                        input_i = 1;
                //                        productScaleAcc( hidden_activation_neg_i,
                //                                         U, V(i), -1.,1.);
                //                    }
                //                    else
                //                    {
                //                        input_i = 0;
                //                        productScaleAcc( hidden_activation_pos_i,
                //                                         U, V(i), 1.,1.);
                //                    }
                //                else
                //                    if( input_is_active[i] )
                //                    {
                //                        input_i = 1;
                //                        hidden_activation_neg_i -= V(i);
                //                    }
                //                    else
                //                    {
                //                        input_i = 0;
                //                        hidden_activation_pos_i += V(i);
                //                    }
                //            }
                //            else
                //            {
                //                w = &(connection->weights(0,i));
                //                input_i = input[i];
                //                for( int j=0; j<hidden_layer->size; j++,w+=m )
                //                {
                //                    a_pos_i[j] = a[j] - *w * ( input_i - 1 );
                //                    a_neg_i[j] = a[j] - *w * input_i;
                //                }
                //            }
                //            num_pos_act -= hidden_layer->freeEnergyContribution(
                //                hidden_activation_pos_i);
                //            num_neg_act -= hidden_layer->freeEnergyContribution(
                //                hidden_activation_neg_i);
                //            //num_pos = safeexp(num_pos_act);
                //            //num_neg = safeexp(num_neg_act);
                //            //input_probs_i = num_pos / (num_pos + num_neg);
                //            if( input_layer->use_fast_approximations )
                //                input_probs_i = fastsigmoid(
                //                    num_pos_act - num_neg_act);
                //            else
                //            {
                //                num_pos = safeexp(num_pos_act);
                //                num_neg = safeexp(num_neg_act);
                //                input_probs_i = num_pos / (num_pos + num_neg);
                //            }
                //            if( input_layer->use_fast_approximations )
                //                pseudolikelihood += tabulated_softplus( 
                //                    num_pos_act - num_neg_act ) 
                //                    - input_i * (num_pos_act - num_neg_act);
                //            else
                //                pseudolikelihood += softplus( 
                //                    num_pos_act - num_neg_act ) 
                //                    - input_i * (num_pos_act - num_neg_act);
                //
                //        }
                //
                //        U_estimated_gradient.fill(pseudolikelihood);
                //
                //        for( int i1=0; i1<U_estimated_gradient.length(); i1++)
                //            for( int j1=0; j1<U_estimated_gradient.width(); j1++)
                //            {
                //                U(i1,j1) += epsilon;
                //                pseudolikelihood = 0;
                //
                //                // Compute activations
                //                if( input_is_sparse )
                //                {
                //                    if( factorized_connection_rank > 0 )
                //                    {
                //                        Vx.clear();
                //                        train_set->getExtra(stage%nsamples,extra);
                //                        for( int i=0; i<extra.length(); i++ )
                //                        {
                //                            Vx += V((int)extra[i]);
                //                            input_is_active[(int)extra[i]] = true;
                //                        }
                //        
                //                        product(hidden_act,U,Vx);
                //                    }
                //                    else
                //                    {
                //                        hidden_act.clear();
                //                        train_set->getExtra(stage%nsamples,extra);
                //                        for( int i=0; i<extra.length(); i++ )
                //                        {
                //                            hidden_act += V((int)extra[i]);
                //                            input_is_active[(int)extra[i]] = true;
                //                        }
                //                    }
                //                    hidden_act += hidden_layer->bias;
                //                }
                //                else
                //                {
                //                    connection->setAsDownInput( input );
                //                    hidden_layer->getAllActivations( 
                //                        (RBMMatrixConnection*) connection );
                //                }
                //
                //                if( targetsize() == 1 )
                //                    productAcc( hidden_layer->activation,
                //                                target_connection->weights,
                //                                target_one_hot );
                //                else if( targetsize() > 1 )
                //                    productAcc( hidden_layer->activation,
                //                                target_connection->weights,
                //                                target );
                //
                //                for( int l=0; l<input_layer->size ; l++ )
                //                {
                //                    if( n_selected_inputs_pseudolikelihood <= inputsize() &&
                //                        n_selected_inputs_pseudolikelihood > 0 )
                //                    {
                //                        if( l >= n_selected_inputs_pseudolikelihood )
                //                            break;
                //                        i = input_indices[l];
                //                    }
                //                    else
                //                        i = l;
                //            
                //                    num_pos_act = input_layer->bias[i];
                //                    // LATERAL CONNECTIONS CODE HERE!
                //                    num_neg_act = 0;
                //                    if( input_is_sparse )
                //                    {
                //                        hidden_activation_pos_i << hidden_act;
                //                        hidden_activation_neg_i << hidden_act;
                //                        if( factorized_connection_rank > 0 )
                //                            if( input_is_active[i] )
                //                            {
                //                                input_i = 1;
                //                                productScaleAcc( hidden_activation_neg_i,
                //                                                 U, V(i), -1.,1.);
                //                            }
                //                            else
                //                            {
                //                                input_i = 0;
                //                                productScaleAcc( hidden_activation_pos_i,
                //                                                 U, V(i), 1.,1.);
                //                            }
                //                        else
                //                            if( input_is_active[i] )
                //                            {
                //                                input_i = 1;
                //                                hidden_activation_neg_i -= V(i);
                //                            }
                //                            else
                //                            {
                //                                input_i = 0;
                //                                hidden_activation_pos_i += V(i);
                //                            }
                //                    }
                //                    else
                //                    {
                //                        w = &(connection->weights(0,i));
                //                        input_i = input[i];
                //                        for( int j=0; j<hidden_layer->size; j++,w+=m )
                //                        {
                //                            a_pos_i[j] = a[j] - *w * ( input_i - 1 );
                //                            a_neg_i[j] = a[j] - *w * input_i;
                //                        }
                //                    }
                //                    num_pos_act -= hidden_layer->freeEnergyContribution(
                //                        hidden_activation_pos_i);
                //                    num_neg_act -= hidden_layer->freeEnergyContribution(
                //                        hidden_activation_neg_i);
                //                    //num_pos = safeexp(num_pos_act);
                //                    //num_neg = safeexp(num_neg_act);
                //                    //input_probs_i = num_pos / (num_pos + num_neg);
                //                    if( input_layer->use_fast_approximations )
                //                        input_probs_i = fastsigmoid(
                //                            num_pos_act - num_neg_act);
                //                    else
                //                    {
                //                        num_pos = safeexp(num_pos_act);
                //                        num_neg = safeexp(num_neg_act);
                //                        input_probs_i = num_pos / (num_pos + num_neg);
                //                    }
                //                    if( input_layer->use_fast_approximations )
                //                        pseudolikelihood += tabulated_softplus( 
                //                            num_pos_act - num_neg_act ) 
                //                            - input_i * (num_pos_act - num_neg_act);
                //                    else
                //                        pseudolikelihood += softplus( 
                //                            num_pos_act - num_neg_act ) 
                //                            - input_i * (num_pos_act - num_neg_act);
                //
                //                }
                //                U(i1,j1) -= epsilon;
                //                U_estimated_gradient(i1,j1) = (pseudolikelihood - U_estimated_gradient(i1,j1))
                //                    / epsilon;
                //            }
                //
                //
                //        }
                //    }
                //    else
                //    {
                //        estimated_gradient.resize(connection->up_size, connection->down_size);
                //
                //        int i=0;
                //        pseudolikelihood = 0;
                //
                //        // Compute activations
                //        if( input_is_sparse )
                //        {
                //            if( factorized_connection_rank > 0 )
                //            {
                //                Vx.clear();
                //                train_set->getExtra(stage%nsamples,extra);
                //                for( int i=0; i<extra.length(); i++ )
                //                {
                //                    Vx += V((int)extra[i]);
                //                    input_is_active[(int)extra[i]] = true;
                //                }
                //        
                //                product(hidden_act,U,Vx);
                //            }
                //            else
                //            {
                //                hidden_act.clear();
                //                train_set->getExtra(stage%nsamples,extra);
                //                for( int i=0; i<extra.length(); i++ )
                //                {
                //                    hidden_act += V((int)extra[i]);
                //                    input_is_active[(int)extra[i]] = true;
                //                }
                //            }
                //            hidden_act += hidden_layer->bias;
                //        }
                //        else
                //        {
                //            connection->setAsDownInput( input );
                //            hidden_layer->getAllActivations( 
                //                (RBMMatrixConnection*) connection );
                //        }
                //
                //        if( targetsize() == 1 )
                //            productAcc( hidden_layer->activation,
                //                        target_connection->weights,
                //                        target_one_hot );
                //        else if( targetsize() > 1 )
                //            productAcc( hidden_layer->activation,
                //                        target_connection->weights,
                //                        target );
                //
                //        for( int l=0; l<input_layer->size ; l++ )
                //        {
                //            if( n_selected_inputs_pseudolikelihood <= inputsize() &&
                //                n_selected_inputs_pseudolikelihood > 0 )
                //            {
                //                if( l >= n_selected_inputs_pseudolikelihood )
                //                    break;
                //                i = input_indices[l];
                //            }
                //            else
                //                i = l;
                //            
                //            num_pos_act = input_layer->bias[i];
                //            // LATERAL CONNECTIONS CODE HERE!
                //            num_neg_act = 0;
                //            if( input_is_sparse )
                //            {
                //                hidden_activation_pos_i << hidden_act;
                //                hidden_activation_neg_i << hidden_act;
                //                if( factorized_connection_rank > 0 )
                //                    if( input_is_active[i] )
                //                    {
                //                        input_i = 1;
                //                        productScaleAcc( hidden_activation_neg_i,
                //                                         U, V(i), -1.,1.);
                //                    }
                //                    else
                //                    {
                //                        input_i = 0;
                //                        productScaleAcc( hidden_activation_pos_i,
                //                                         U, V(i), 1.,1.);
                //                    }
                //                else
                //                    if( input_is_active[i] )
                //                    {
                //                        input_i = 1;
                //                        hidden_activation_neg_i -= V(i);
                //                    }
                //                    else
                //                    {
                //                        input_i = 0;
                //                        hidden_activation_pos_i += V(i);
                //                    }
                //            }
                //            else
                //            {
                //                w = &(connection->weights(0,i));
                //                input_i = input[i];
                //                for( int j=0; j<hidden_layer->size; j++,w+=m )
                //                {
                //                    a_pos_i[j] = a[j] - *w * ( input_i - 1 );
                //                    a_neg_i[j] = a[j] - *w * input_i;
                //                }
                //            }
                //            num_pos_act -= hidden_layer->freeEnergyContribution(
                //                hidden_activation_pos_i);
                //            num_neg_act -= hidden_layer->freeEnergyContribution(
                //                hidden_activation_neg_i);
                //            //num_pos = safeexp(num_pos_act);
                //            //num_neg = safeexp(num_neg_act);
                //            //input_probs_i = num_pos / (num_pos + num_neg);
                //            if( input_layer->use_fast_approximations )
                //                input_probs_i = fastsigmoid(
                //                    num_pos_act - num_neg_act);
                //            else
                //            {
                //                num_pos = safeexp(num_pos_act);
                //                num_neg = safeexp(num_neg_act);
                //                input_probs_i = num_pos / (num_pos + num_neg);
                //            }
                //            if( input_layer->use_fast_approximations )
                //                pseudolikelihood += tabulated_softplus( 
                //                    num_pos_act - num_neg_act ) 
                //                    - input_i * (num_pos_act - num_neg_act);
                //            else
                //                pseudolikelihood += softplus( 
                //                    num_pos_act - num_neg_act ) 
                //                    - input_i * (num_pos_act - num_neg_act);
                //
                //        }
                //
                //        estimated_gradient.fill(pseudolikelihood);
                //
                //        for( int i1=0; i1<estimated_gradient.length(); i1++)
                //            for( int j1=0; j1<estimated_gradient.width(); j1++)
                //            {
                //                connection->weights(i1,j1) += epsilon;
                //                pseudolikelihood = 0;
                //
                //                // Compute activations
                //                if( input_is_sparse )
                //                {
                //                    if( factorized_connection_rank > 0 )
                //                    {
                //                        Vx.clear();
                //                        train_set->getExtra(stage%nsamples,extra);
                //                        for( int i=0; i<extra.length(); i++ )
                //                        {
                //                            Vx += V((int)extra[i]);
                //                            input_is_active[(int)extra[i]] = true;
                //                        }
                //        
                //                        product(hidden_act,U,Vx);
                //                    }
                //                    else
                //                    {
                //                        hidden_act.clear();
                //                        train_set->getExtra(stage%nsamples,extra);
                //                        for( int i=0; i<extra.length(); i++ )
                //                        {
                //                            hidden_act += V((int)extra[i]);
                //                            input_is_active[(int)extra[i]] = true;
                //                        }
                //                    }
                //                    hidden_act += hidden_layer->bias;
                //                }
                //                else
                //                {
                //                    connection->setAsDownInput( input );
                //                    hidden_layer->getAllActivations( 
                //                        (RBMMatrixConnection*) connection );
                //                }
                //
                //                if( targetsize() == 1 )
                //                    productAcc( hidden_layer->activation,
                //                                target_connection->weights,
                //                                target_one_hot );
                //                else if( targetsize() > 1 )
                //                    productAcc( hidden_layer->activation,
                //                                target_connection->weights,
                //                                target );
                //
                //                for( int l=0; l<input_layer->size ; l++ )
                //                {
                //                    if( n_selected_inputs_pseudolikelihood <= inputsize() &&
                //                        n_selected_inputs_pseudolikelihood > 0 )
                //                    {
                //                        if( l >= n_selected_inputs_pseudolikelihood )
                //                            break;
                //                        i = input_indices[l];
                //                    }
                //                    else
                //                        i = l;
                //            
                //                    num_pos_act = input_layer->bias[i];
                //                    // LATERAL CONNECTIONS CODE HERE!
                //                    num_neg_act = 0;
                //                    if( input_is_sparse )
                //                    {
                //                        hidden_activation_pos_i << hidden_act;
                //                        hidden_activation_neg_i << hidden_act;
                //                        if( factorized_connection_rank > 0 )
                //                            if( input_is_active[i] )
                //                            {
                //                                input_i = 1;
                //                                productScaleAcc( hidden_activation_neg_i,
                //                                                 U, V(i), -1.,1.);
                //                            }
                //                            else
                //                            {
                //                                input_i = 0;
                //                                productScaleAcc( hidden_activation_pos_i,
                //                                                 U, V(i), 1.,1.);
                //                            }
                //                        else
                //                            if( input_is_active[i] )
                //                            {
                //                                input_i = 1;
                //                                hidden_activation_neg_i -= V(i);
                //                            }
                //                            else
                //                            {
                //                                input_i = 0;
                //                                hidden_activation_pos_i += V(i);
                //                            }
                //                    }
                //                    else
                //                    {
                //                        w = &(connection->weights(0,i));
                //                        input_i = input[i];
                //                        for( int j=0; j<hidden_layer->size; j++,w+=m )
                //                        {
                //                            a_pos_i[j] = a[j] - *w * ( input_i - 1 );
                //                            a_neg_i[j] = a[j] - *w * input_i;
                //                        }
                //                    }
                //                    num_pos_act -= hidden_layer->freeEnergyContribution(
                //                        hidden_activation_pos_i);
                //                    num_neg_act -= hidden_layer->freeEnergyContribution(
                //                        hidden_activation_neg_i);
                //                    //num_pos = safeexp(num_pos_act);
                //                    //num_neg = safeexp(num_neg_act);
                //                    //input_probs_i = num_pos / (num_pos + num_neg);
                //                    if( input_layer->use_fast_approximations )
                //                        input_probs_i = fastsigmoid(
                //                            num_pos_act - num_neg_act);
                //                    else
                //                    {
                //                        num_pos = safeexp(num_pos_act);
                //                        num_neg = safeexp(num_neg_act);
                //                        input_probs_i = num_pos / (num_pos + num_neg);
                //                    }
                //                    if( input_layer->use_fast_approximations )
                //                        pseudolikelihood += tabulated_softplus( 
                //                            num_pos_act - num_neg_act ) 
                //                            - input_i * (num_pos_act - num_neg_act);
                //                    else
                //                        pseudolikelihood += softplus( 
                //                            num_pos_act - num_neg_act ) 
                //                            - input_i * (num_pos_act - num_neg_act);
                //
                //                }
                //                connection->weights(i1,j1) -= epsilon;
                //                estimated_gradient(i1,j1) = (pseudolikelihood - estimated_gradient(i1,j1))
                //                    / epsilon;
                //            }
                //
                //    }
                //}

                // Compute activations
                if( input_is_sparse )
                {
                    if( factorized_connection_rank > 0 )
                    {
                        Vx.clear();
                        train_set->getExtra(stage%nsamples,extra);
                        for( int i=0; i<extra.length(); i++ )
                        {
                            Vx += V((int)extra[i]);
                            input_is_active[(int)extra[i]] = true;
                        }
                        
                        product(hidden_act,U,Vx);
                    }
                    else
                    {
                        hidden_act.clear();
                        train_set->getExtra(stage%nsamples,extra);
                        for( int i=0; i<extra.length(); i++ )
                        {
                            hidden_act += V((int)extra[i]);
                            input_is_active[(int)extra[i]] = true;
                        }
                    }
                    hidden_act += hidden_layer->bias;
                }
                else
                {
                    connection->setAsDownInput( input );
                    hidden_layer->getAllActivations( 
                        (RBMMatrixConnection*) connection );
                }

                if( targetsize() == 1 )
                        productAcc( hidden_layer->activation,
                                    target_connection->weights,
                                    target_one_hot );
                else if( targetsize() > 1 )
                    productAcc( hidden_layer->activation,
                                target_connection->weights,
                                target );

                // Clear gradients
                hidden_activation_gradient.clear();
                if( !input_is_sparse )
                {
                    connection_gradient.clear();
                    input_gradient.clear(); // If input is sparse, only the 
                                            // appropriage elements of this 
                                            // gradient will be used
                }
                
                if( factorized_connection_rank > 0 )
                {
                    U_gradient.clear();
                    Vx_gradient.clear();
                }
                V_gradients.clear();

                int i=0;
                pseudolikelihood = 0;
                for( int l=0; l<input_layer->size ; l++ )
                {
                    if( n_selected_inputs_pseudolikelihood <= inputsize() &&
                        n_selected_inputs_pseudolikelihood > 0 )
                    {
                        if( l >= n_selected_inputs_pseudolikelihood )
                            break;
                        i = input_indices[l];
                    }
                    else
                        i = l;

                    num_pos_act = input_layer->bias[i];
                    // LATERAL CONNECTIONS CODE HERE!
                    num_neg_act = 0;
                    if( input_is_sparse )
                    {
                        hidden_activation_pos_i << hidden_act;
                        hidden_activation_neg_i << hidden_act;
                        if( factorized_connection_rank > 0 )
                            if( input_is_active[i] )
                            {
                                input_i = 1;
                                productScaleAcc( hidden_activation_neg_i,
                                                U, V(i), -1.,1.);
                            }
                            else
                            {
                                input_i = 0;
                                productScaleAcc( hidden_activation_pos_i,
                                                U, V(i), 1.,1.);
                            }
                        else
                            if( input_is_active[i] )
                            {
                                input_i = 1;
                                hidden_activation_neg_i -= V(i);
                            }
                            else
                            {
                                input_i = 0;
                                hidden_activation_pos_i += V(i);
                            }
                    }
                    else
                    {
                        w = &(connection->weights(0,i));
                        input_i = input[i];
                        for( int j=0; j<hidden_layer->size; j++,w+=m )
                        {
                            a_pos_i[j] = a[j] - *w * ( input_i - 1 );
                            a_neg_i[j] = a[j] - *w * input_i;
                        }
                    }
                    num_pos_act -= hidden_layer->freeEnergyContribution(
                        hidden_activation_pos_i);
                    num_neg_act -= hidden_layer->freeEnergyContribution(
                        hidden_activation_neg_i);
                    //num_pos = safeexp(num_pos_act);
                    //num_neg = safeexp(num_neg_act);
                    //input_probs_i = num_pos / (num_pos + num_neg);
                    if( input_layer->use_fast_approximations )
                        input_probs_i = fastsigmoid(
                            num_pos_act - num_neg_act);
                    else
                    {
                        num_pos = safeexp(num_pos_act);
                        num_neg = safeexp(num_neg_act);
                        input_probs_i = num_pos / (num_pos + num_neg);
                    }

                    // Compute input_prob gradient
                    if( input_layer->use_fast_approximations )
                        pseudolikelihood += tabulated_softplus( 
                            num_pos_act - num_neg_act ) 
                            - input_i * (num_pos_act - num_neg_act);
                    else
                        pseudolikelihood += softplus( 
                            num_pos_act - num_neg_act ) 
                            - input_i * (num_pos_act - num_neg_act);
                    input_gradient[i] = input_probs_i - input_i;

                    hidden_layer->freeEnergyContributionGradient(
                        hidden_activation_pos_i,
                        hidden_activation_pos_i_gradient,
                        -input_gradient[i],
                        false);
                    hidden_activation_gradient += hidden_activation_pos_i_gradient;

                    hidden_layer->freeEnergyContributionGradient(
                        hidden_activation_neg_i,
                        hidden_activation_neg_i_gradient,
                        input_gradient[i],
                        false);
                    hidden_activation_gradient += hidden_activation_neg_i_gradient;

                    if( input_is_sparse )
                    {
                        if( factorized_connection_rank > 0 )
                        {
                            if( input_is_active[i] )
                            {
                                Vec vg = V_gradients(l);
                                transposeProductScaleAcc(
                                    vg, U, hidden_activation_neg_i_gradient,
                                    -1., 0);
                                externalProductScaleAcc( 
                                    U_gradient, 
                                    hidden_activation_neg_i_gradient,
                                    V(i), -1 );
                            }
                            else
                            {
                                Vec vg = V_gradients(l);
                                transposeProduct(
                                    vg, U, hidden_activation_pos_i_gradient);
                                externalProductAcc( 
                                    U_gradient, 
                                    hidden_activation_pos_i_gradient,
                                    V(i) );
                            }
                        }
                        else
                        {
                            if( input_is_active[i] )
                                V_gradients(l) -= 
                                    hidden_activation_neg_i_gradient;
                            else
                                V_gradients(l) += 
                                    hidden_activation_pos_i_gradient;
                        }
                    }
                    else
                    {
                        gw = &(connection_gradient(0,i));
                        for( int j=0; j<hidden_layer->size; j++,gw+=m )
                        {
                            *gw -= ga_pos_i[j] * ( input_i - 1 );
                            *gw -= ga_neg_i[j] * input_i;
                        }
                    }
                }

                // Hidden bias update
                multiplyScaledAdd(hidden_activation_gradient, 1.0, -lr,
                                  hidden_layer->bias);

                if( input_is_sparse )
                {
                    //Mat true_gradient(V.length(), V.width());
                    if( factorized_connection_rank > 0 )
                    {
                        // Factorized connection U update
                        externalProductAcc( U_gradient, 
                                            hidden_activation_gradient,
                                            Vx );
                        multiplyScaledAdd( U_gradient, 1.0, -lr, U );
                        
                        //real U_cos_ang = dot(U_gradient.toVec(),U_estimated_gradient.toVec())
                        //    / (norm(U_gradient.toVec()) *norm(U_estimated_gradient.toVec()));
                        //cout << "U_cos_ang=" << U_cos_ang << endl;
                        //cout << "U_ang=" << acos(U_cos_ang) << endl;

   
                        // Factorized connection V update
                        transposeProduct( Vx_gradient, U, 
                                          hidden_activation_gradient );
                        for( int e=0; e<extra.length(); e++ )
                        {
                            V((int)extra[e]) -= lr * Vx_gradient;
                            input_is_active[(int)extra[e]] = false;
                            //true_gradient((int)extra[e]) += Vx_gradient;
                        }
                    }
                    else
                    {
                        // Update input connection V
                        for( int e=0; e<extra.length(); e++ )
                        {
                            V((int)extra[e]) -= lr * hidden_activation_gradient;
                            input_is_active[(int)extra[e]] = false;
                            //true_gradient((int)extra[e]) += hidden_activation_gradient;
                        }
                    }
                    
                    for( int l=0; l<input_layer->size ; l++ )
                    {
                        if( n_selected_inputs_pseudolikelihood <= inputsize() 
                            && n_selected_inputs_pseudolikelihood > 0 )
                        {
                            if( l >= n_selected_inputs_pseudolikelihood )
                                break;
                            i = input_indices[l];
                        }
                        else
                            i = l;
                        // Extra V gradients
                        V(i) -= lr * V_gradients(l);
                        //true_gradient(i) += V_gradients(l);

                        // Input update
                        input_layer->bias[i] -= lr * input_gradient[i];
                    }
                    
                    //real cos_ang = dot(true_gradient.toVec(),estimated_gradient.toVec())
                    //    / (norm(true_gradient.toVec()) *norm(estimated_gradient.toVec()));
                    //cout << "cos_ang=" << cos_ang << endl;
                    //cout << "ang=" << acos(cos_ang) << endl;

                }
                else
                {
                    externalProductAcc( connection_gradient, hidden_activation_gradient,
                                        input );

                    //real cos_ang = dot(connection_gradient.toVec(),estimated_gradient.toVec())
                    //    / (norm(connection_gradient.toVec()) *norm(estimated_gradient.toVec()));
                    //cout << "cos_ang=" << cos_ang << endl;
                    //cout << "ang=" << acos(cos_ang) << endl;
                    
                    // Connection weights update
                    multiplyScaledAdd( connection_gradient, 1.0, -lr,
                                       connection->weights );
                    // Input bias update
                    multiplyScaledAdd(input_gradient, 1.0, -lr,
                                      input_layer->bias);
                }
                

                if( targetsize() == 1 )
                    externalProductScaleAcc( target_connection->weights, 
                                             hidden_activation_gradient,
                                             target_one_hot,
                                             -lr );
                if( targetsize() > 1 )
                    externalProductScaleAcc( target_connection->weights, 
                                             hidden_activation_gradient,
                                             target,
                                             -lr );

                // N.B.: train costs contains pseudolikelihood
                //       or pseudoNLL, not NLL
                if( compute_input_space_nll && targetsize() == 0 )
                    train_costs[nll_cost_index] = pseudolikelihood;
                //mean_pseudolikelihood += pseudolikelihood;
//                    cout << "input_gradient: " << input_gradient << endl;
//                    cout << "hidden_activation_gradient" << hidden_activation_gradient << endl;

            }
            else
            {
                if( input_is_sparse )
                    PLERROR("In PseudolikelihoodRBM::train(): "
                            "pseudolikelihood_context_size with > 0 "
                            "not implemented for sparse inputs");
                
                if( ( pseudolikelihood_context_type == "most_correlated" ||
                      pseudolikelihood_context_type == "most_correlated_uniform_random" )
                    && correlations_per_i.length() == 0 )
                {
                    Vec corr_input(inputsize());
                    Vec corr_target(targetsize());
                    real corr_weight;
                    Vec mean(inputsize());
                    mean.clear();
                    for(int t=0; t<train_set->length(); t++)
                    {
                        train_set->getExample(t,corr_input,corr_target,
                                              corr_weight);
                        mean += corr_input;
                    }
                    mean /= train_set->length();
                        
                    correlations_per_i.resize(inputsize(),inputsize());
                    correlations_per_i.clear();
                    Mat cov(inputsize(), inputsize());
                    cov.clear();
                    for(int t=0; t<train_set->length(); t++)
                    {
                        train_set->getExample(t,corr_input,corr_target,
                                              corr_weight);
                        corr_input -= mean;
                        externalProductAcc(cov,
                                           corr_input,corr_input);
                    }
                    //correlations_per_i /= train_set->length();

                    for( int i=0; i<inputsize(); i++ )
                        for( int j=0; j<inputsize(); j++)
                        {
                            correlations_per_i(i,j) = 
                                abs(cov(i,j)) 
                                / sqrt(cov(i,i)*cov(j,j));
                        }

                    if( pseudolikelihood_context_type == "most_correlated")
                    {
                        if( pseudolikelihood_context_size <= 0 )
                            PLERROR("In PseudolikelihoodRBM::train(): "
                                    "pseudolikelihood_context_size should be > 0 "
                                    "for \"most_correlated\" context type");
                        real current_min;
                        int current_min_position;
                        real* corr;
                        int* context;
                        Vec context_corr(pseudolikelihood_context_size);
                        context_indices_per_i.resize(
                            inputsize(),
                            pseudolikelihood_context_size);

                        // HUGO: this is quite inefficient for big 
                        // pseudolikelihood_context_sizes, should use a heap
                        for( int i=0; i<inputsize(); i++ )
                        {
                            current_min = REAL_MAX;
                            current_min_position = -1;
                            corr = correlations_per_i[i];
                            context = context_indices_per_i[i];
                            for( int j=0; j<inputsize(); j++ )
                            {
                                if( i == j )
                                    continue;

                                // Filling first pseudolikelihood_context_size elements
                                if( j - (j>i?1:0) < pseudolikelihood_context_size )
                                {
                                    context[j - (j>i?1:0)] = j;
                                    context_corr[j - (j>i?1:0)] = corr[j];
                                    if( current_min > corr[j] )
                                    {
                                        current_min = corr[j];
                                        current_min_position = j - (j>i?1:0);
                                    }
                                    continue;
                                }

                                if( corr[j] > current_min )
                                {
                                    context[current_min_position] = j;
                                    context_corr[current_min_position] = corr[j];
                                    current_min = 
                                        min( context_corr, 
                                             current_min_position );
                                }
                            }
                        }
                    }
                        
                    if( pseudolikelihood_context_type == 
                        "most_correlated_uniform_random" )
                    {
                        if( k_most_correlated < 
                            pseudolikelihood_context_size )
                            PLERROR("In PseudolikelihoodRBM::train(): "
                                    "k_most_correlated should be "
                                    ">= pseudolikelihood_context_size");

                        if( k_most_correlated > inputsize() - 1 )
                            PLERROR("In PseudolikelihoodRBM::train(): "
                                    "k_most_correlated should be "
                                    "< inputsize()");

                        real current_min;
                        int current_min_position;
                        real* corr;
                        int* context;
                        Vec context_corr( k_most_correlated );
                        context_most_correlated.resize( inputsize() );

                        // HUGO: this is quite inefficient for big 
                        // pseudolikelihood_context_sizes, should use a heap
                        for( int i=0; i<inputsize(); i++ )
                        {
                            context_most_correlated[i].resize( 
                                k_most_correlated );
                            current_min = REAL_MAX;
                            current_min_position = -1;
                            corr = correlations_per_i[i];
                            context = context_most_correlated[i].data();
                            for( int j=0; j<inputsize(); j++ )
                            {
                                if( i == j )
                                    continue;

                                // Filling first k_most_correlated elements
                                if( j - (j>i?1:0) <  k_most_correlated )
                                {
                                    context[j - (j>i?1:0)] = j;
                                    context_corr[j - (j>i?1:0)] = corr[j];
                                    if( current_min > corr[j] )
                                    {
                                        current_min = corr[j];
                                        current_min_position = j - (j>i?1:0);
                                    }
                                    continue;
                                }

                                if( corr[j] > current_min )
                                {
                                    context[current_min_position] = j;
                                    context_corr[current_min_position] = corr[j];
                                    current_min = 
                                        min( context_corr, 
                                             current_min_position );
                                }
                            }
                        }
                    }                        
                }

                if( pseudolikelihood_context_type == "uniform_random" ||
                    pseudolikelihood_context_type == "most_correlated_uniform_random" )
                {
                    // Generate contexts
                    if( pseudolikelihood_context_type == "uniform_random" )
                        for( int i=0; i<context_indices.length(); i++)
                            context_indices[i] = i;
                    int tmp,k;
                    int* c;
                    int n;
                    if( pseudolikelihood_context_type == "uniform_random" )
                    {
                        c = context_indices.data();
                        n = input_layer->size-1;
                    }
                    int* ci;
                    for( int i=0; i<context_indices_per_i.length(); i++)
                    {
                        if( pseudolikelihood_context_type == 
                            "most_correlated_uniform_random" )
                        {
                            c = context_most_correlated[i].data();
                            n = context_most_correlated[i].length();
                        }

                        ci = context_indices_per_i[i];
                        for (int j = 0; j < context_indices_per_i.width(); j++) 
                        {
                            k = j + 
                                random_gen->uniform_multinomial_sample(n - j);
                                
                            tmp = c[j];
                            c[j] = c[k];
                            c[k] = tmp;

                            if( pseudolikelihood_context_type 
                                == "uniform_random" )
                            {
                                if( c[j] >= i )
                                    ci[j] = c[j]+1;
                                else
                                    ci[j] = c[j];
                            }

                            if( pseudolikelihood_context_type == 
                                "most_correlated_uniform_random" )
                                ci[j] = c[j];
                        }
                    }
                }

                connection->setAsDownInput( input );
                hidden_layer->getAllActivations( 
                    (RBMMatrixConnection*) connection );

                if( targetsize() == 1 )
                    productAcc( hidden_layer->activation,
                                target_connection->weights,
                                target_one_hot );
                else if( targetsize() > 1 )
                    productAcc( hidden_layer->activation,
                                    target_connection->weights,
                                target );

                int n_conf = ipow(2, pseudolikelihood_context_size);
                //nums_act.resize( 2 * n_conf );
                //gnums_act.resize( 2 * n_conf );
                //context_probs.resize( 2 * n_conf );
                //hidden_activations_context.resize( 2*n_conf, hidden_layer->size );
                //hidden_activations_context_k_gradient.resize( hidden_layer->size );
                real* nums_data;
                real* gnums_data;
                real* cp_data;
                real* a = hidden_layer->activation.data();
                real* w, *gw, *gi, *ac, *bi, *gac;
                int* context_i;
                int m;
                int conf_index;
                real input_i, input_j,  log_Zi;
                real pseudolikelihood = 0;

                input_gradient.clear();
                hidden_activation_gradient.clear();
                connection_gradient.clear();
                gi = input_gradient.data();
                bi = input_layer->bias.data();
                for( int i=0; i<input_layer->size ; i++ )
                {
                    nums_data = nums_act.data();
                    cp_data = context_probs.data();
                    input_i = input[i];

                    if( connection ) 
                        m = connection->weights.mod();
                    // input_i = 1
                    for( int k=0; k<n_conf; k++)
                    {
                        *nums_data = bi[i];
                        *cp_data = input_i;
                        conf_index = k;
                        ac = hidden_activations_context[k];

                        w = &(connection->weights(0,i));
                        for( int j=0; j<hidden_layer->size; j++,w+=m )
                            ac[j] = a[j] - *w * ( input_i - 1 );

                        context_i = context_indices_per_i[i];
                        for( int l=0; l<pseudolikelihood_context_size; l++ )
                        {
                            input_j = input[*context_i];
                            w = &(connection->weights(0,*context_i));
                            if( conf_index & 1)
                            {
                                *cp_data *= input_j;
                                *nums_data += bi[*context_i];
                                for( int j=0; j<hidden_layer->size; j++,w+=m )
                                    ac[j] -=  *w * ( input_j - 1 );
                            }
                            else
                            {
                                *cp_data *= (1-input_j);
                                for( int j=0; j<hidden_layer->size; j++,w+=m )
                                    ac[j] -=  *w * input_j;
                            }

                            conf_index >>= 1;
                            context_i++;
                        }
                        *nums_data -= hidden_layer->freeEnergyContribution(
                            hidden_activations_context(k));
                        nums_data++;
                        cp_data++;
                    }

                    // input_i = 0
                    for( int k=0; k<n_conf; k++)
                    {
                        *nums_data = 0;
                        *cp_data = (1-input_i);
                        conf_index = k;
                        ac = hidden_activations_context[n_conf + k];
                        
                        w = &(connection->weights(0,i));
                        for( int j=0; j<hidden_layer->size; j++,w+=m )
                            ac[j] = a[j] - *w * input_i;

                        context_i = context_indices_per_i[i];
                        for( int l=0; l<pseudolikelihood_context_size; l++ )
                        {
                            w = &(connection->weights(0,*context_i));
                            input_j = input[*context_i];
                            if( conf_index & 1)
                            {
                                *cp_data *= input_j;
                                *nums_data += bi[*context_i];
                                for( int j=0; j<hidden_layer->size; j++,w+=m )
                                    ac[j] -=  *w * ( input_j - 1 );
                            }
                            else
                            {
                                *cp_data *= (1-input_j);
                                for( int j=0; j<hidden_layer->size; j++,w+=m )
                                    ac[j] -=  *w * input_j;
                            }

                            conf_index >>= 1;
                            context_i++;
                        }
                        *nums_data -= hidden_layer->freeEnergyContribution(
                            hidden_activations_context(n_conf + k));
                        nums_data++;
                        cp_data++;
                    }
                    

                    // Gradient computation
                    //exp( nums_act, nums);
                    //Zi = sum(nums);
                    //log_Zi = pl_log(Zi);
                    log_Zi = logadd(nums_act);

                    nums_data = nums_act.data();
                    gnums_data = gnums_act.data();
                    cp_data = context_probs.data();

                    // Compute input_prob gradient

                    m = connection_gradient.mod();
                    // input_i = 1                    
                    for( int k=0; k<n_conf; k++)
                    {
                        pseudolikelihood -= *cp_data * (*nums_data - log_Zi);
                        *gnums_data = (safeexp(*nums_data - log_Zi) - *cp_data);
                        gi[i] += *gnums_data;
                        
                        hidden_layer->freeEnergyContributionGradient(
                            hidden_activations_context(k),
                            hidden_activations_context_k_gradient,
                            -*gnums_data,
                            false);
                        hidden_activation_gradient += 
                            hidden_activations_context_k_gradient;
                        
                        gac = hidden_activations_context_k_gradient.data();
                        gw = &(connection_gradient(0,i));
                        for( int j=0; j<hidden_layer->size; j++,gw+=m )
                            *gw -= gac[j] * ( input_i - 1 );

                        context_i = context_indices_per_i[i];
                        for( int l=0; l<pseudolikelihood_context_size; l++ )
                        {
                            gw = &(connection_gradient(0,*context_i));
                            input_j = input[*context_i];
                            if( conf_index & 1)
                            {
                                gi[*context_i] += *gnums_data;
                                for( int j=0; j<hidden_layer->size; j++,gw+=m )
                                    *gw -= gac[j] * ( input_j - 1 );
                            }
                            else
                            {
                                for( int j=0; j<hidden_layer->size; j++,gw+=m )
                                    *gw -= gac[j] * input_j;
                            }
                            conf_index >>= 1;
                            context_i++;
                        }

                        nums_data++;
                        gnums_data++;
                        cp_data++;
                    }

                    // input_i = 0
                    for( int k=0; k<n_conf; k++)
                    {
                        pseudolikelihood -= *cp_data * (*nums_data - log_Zi);
                        *gnums_data = (safeexp(*nums_data - log_Zi) - *cp_data);
                        
                        hidden_layer->freeEnergyContributionGradient(
                            hidden_activations_context(n_conf + k),
                            hidden_activations_context_k_gradient,
                            -*gnums_data,
                            false);
                        hidden_activation_gradient += 
                            hidden_activations_context_k_gradient;
                        
                        gac = hidden_activations_context_k_gradient.data();
                        gw = &(connection_gradient(0,i));
                        for( int j=0; j<hidden_layer->size; j++,gw+=m )
                            *gw -= gac[j] *input_i;

                        context_i = context_indices_per_i[i];
                        for( int l=0; l<pseudolikelihood_context_size; l++ )
                        {
                            gw = &(connection_gradient(0,*context_i));
                            input_j = input[*context_i];
                            if( conf_index & 1)
                            {
                                gi[*context_i] += *gnums_data;
                                for( int j=0; j<hidden_layer->size; j++,gw+=m )
                                    *gw -= gac[j] * ( input_j - 1 );
                            }
                            else
                            {
                                for( int j=0; j<hidden_layer->size; j++,gw+=m )
                                    *gw -= gac[j] * input_j;
                            }

                            conf_index >>= 1;
                            context_i++;
                        }

                        nums_data++;
                        gnums_data++;
                        cp_data++;
                    }
                }

//                    cout << "input_gradient: " << input_gradient << endl;
//                    cout << "hidden_activation_gradient" << hidden_activation_gradient << endl;

                externalProductAcc( connection_gradient, hidden_activation_gradient,
                                    input );

                // Hidden bias update
                multiplyScaledAdd(hidden_activation_gradient, 1.0, -lr,
                                  hidden_layer->bias);
                // Connection weights update
                multiplyScaledAdd( connection_gradient, 1.0, -lr,
                                   connection->weights );
                // Input bias update
                multiplyScaledAdd(input_gradient, 1.0, -lr,
                                  input_layer->bias);

                if( targetsize() == 1 )
                    externalProductScaleAcc( target_connection->weights, 
                                             hidden_activation_gradient,
                                             target_one_hot,
                                             -lr );
                if( targetsize() > 1 )
                    externalProductScaleAcc( target_connection->weights, 
                                             hidden_activation_gradient,
                                             target,
                                             -lr );

                // N.B.: train costs contains pseudolikelihood
                //       or pseudoNLL, not NLL
                if( compute_input_space_nll && targetsize() == 0 )
                    train_costs[nll_cost_index] = pseudolikelihood;
            }
        }
    
        // CD learning
        if( !fast_exact_is_equal(cd_learning_rate, 0.) &&
            (targetsize() == 0 || generative_learning_weight > 0) )
        {
            if( input_is_sparse )
            {
                if( is_missing(target[0]) )
                    PLERROR("In PseudolikelihoodRBM::train(): generative training with "
                            "unlabeled examples not supported for CD training with "
                            "sparse inputs.");

                // Randomly select inputs
                if( n_selected_inputs_cd > inputsize() ||
                    n_selected_inputs_cd <= 0 )
                    PLERROR("In PseudolikelihoodRBM::train(): "
                            "n_selected_inputs_cd should be > 0 and "
                            "<= inputsize()" );

                if ( input_indices.length() == 0 )
                {
                    input_indices.resize(inputsize());
                    for( int i=0; i<input_indices.length(); i++ )
                        input_indices[i] = i;
                        
                }
                 
                // Randomly selected inputs
                int tmp;
                int k;
                for (int j = 0; j < n_selected_inputs_cd; j++) 
                {
                    k = j + 
                        random_gen->uniform_multinomial_sample(
                            inputsize() - j);
                        
                    tmp = input_indices[j];
                    input_indices[j] = input_indices[k];
                    input_indices[k] = tmp;
                }

                if( factorized_connection_rank > 0 )
                    PLERROR("In PseudolikelihoodRBM::train(): factorized "
                            "connection is not implemented for CD and "
                            "sparse inputs" );

                if( !fast_exact_is_equal(persistent_cd_weight, 0) )
                    PLERROR("In PseudolikelihoodRBM::train(): persistent CD "
                            "cannot be used for sparse inputs" );

                if( use_mean_field_cd )
                    PLERROR("In PseudolikelihoodRBM::train(): MF-CD "
                            "is not implemented for sparse inputs" );

                if( !fast_exact_is_equal(cd_decrease_ct, 0) )
                    lr = cd_learning_rate / (1.0 + stage * cd_decrease_ct );
                else
                    lr = cd_learning_rate;

                if( targetsize() > 0 )
                    lr *= generative_learning_weight;

                if( weightsize > 0 )
                    lr *= weight;

                setLearningRate(lr);

                // Positive phase
                if( targetsize() > 0 )
                    pos_target = target_one_hot;

                Vec hidden_act = hidden_layer->activation;
                hidden_act.clear();
                hidden_act_non_selected.clear();
                train_set->getExtra(stage%nsamples,extra);
                input_is_selected.resize( extra.length() );
                input_is_selected.clear();
                for( int i=0; i<extra.length(); i++ )
                {
                    hidden_act += V((int)extra[i]);
                    if( input_indices.subVec(0,n_selected_inputs_cd).find((int)extra[i]) >= 0 )
                    {
                        input_is_selected[i] = true;
                        pos_input_sparse[(int)extra[i]] = 1;
                    }
                    else
                        hidden_act_non_selected += V((int)extra[i]);                        
                }
                hidden_act += hidden_layer->bias;
                hidden_act_non_selected += hidden_layer->bias;

                if( targetsize() == 1 )
                    productAcc( hidden_layer->activation,
                                target_connection->weights,
                                target_one_hot );
                else if( targetsize() > 1 )
                    productAcc( hidden_layer->activation,
                                target_connection->weights,
                                target );

                hidden_layer->expectation_is_not_up_to_date();
                hidden_layer->computeExpectation();
                //pos_hidden.resize( hidden_layer->size );
                pos_hidden << hidden_layer->expectation;
                    
                // Negative phase
                real *w;
                Vec input_act = input_layer->activation;
                Vec input_sample = input_layer->sample;
                Vec hidden_sample = hidden_layer->sample;
                int in;
                for(int i=0; i<cd_n_gibbs; i++)
                {
                    // Down pass
                    hidden_layer->generateSample();
                    for (int j = 0; j < n_selected_inputs_cd; j++) 
                    {
                        in = input_indices[j];
                        w = V[in];
                        input_act[in] = input_layer->bias[in];
                        for( int k=0; k<hidden_layer->size; k++ )
                            input_act[in] += w[k] * hidden_sample[k];
                        
                        if( input_layer->use_fast_approximations )
                        {
                            input_sample[in] = random_gen->binomial_sample(
                                fastsigmoid( input_act[in] ));
                        }
                        else
                        {
                            input_sample[in] = random_gen->binomial_sample(
                                fastsigmoid( input_act[in] ));
                        }
                    }

                    // Up pass
                    hidden_act << hidden_act_non_selected;
                    for (int j = 0; j < n_selected_inputs_cd; j++) 
                    {
                        in = input_indices[j];
                        if( fast_exact_is_equal(input_sample[in], 1) )
                            hidden_act += V(in);
                    }

                    if( targetsize() > 0 )
                    {
                        // Down-up pass for target
                        target_connection->setAsUpInput( 
                            hidden_layer->sample );
                        target_layer->getAllActivations( 
                            (RBMMatrixConnection*) target_connection );
                        target_layer->computeExpectation();
                        target_layer->generateSample();
                        productAcc( hidden_act,
                                    target_connection->weights,
                                    target_layer->sample );
                    }
                    
                    hidden_layer->expectation_is_not_up_to_date();
                    hidden_layer->computeExpectation();
                }

                neg_hidden = hidden_layer->expectation;
                    
                hidden_layer->update(pos_hidden,neg_hidden);
                if( targetsize() > 0 )
                {
                    neg_target = target_layer->sample;
                    target_layer->update(pos_target,neg_target);
                    target_connection->update(pos_target,pos_hidden,
                                              neg_target,neg_hidden);
                }

                // Selected inputs connection update
                for (int j = 0; j < n_selected_inputs_cd; j++) 
                {
                    in = input_indices[j];
                    w = V[in];
                    for( int k=0; k<hidden_layer->size; k++ )
                        w[k] += lr * (pos_hidden[k] * pos_input_sparse[in] - 
                                    neg_hidden[k] * input_sample[in]);
                    input_layer->bias[in] += lr * ( pos_input_sparse[in] - 
                                                    input_sample[in]);
                }
                
                // Non-selected inputs connection update
                hidden_activation_gradient << neg_hidden;
                hidden_activation_gradient -= pos_hidden;
                hidden_activation_gradient *= -lr;
                for( int i=0; i<extra.length(); i++ )
                {
                    if( input_is_selected[i] = true )
                        pos_input_sparse[(int)extra[i]] = 0;
                    else
                        V((int)extra[i]) += hidden_activation_gradient;
                }
            }
            else
            {
                if( !fast_exact_is_equal(persistent_cd_weight, 1.) )
                {
                    if( !fast_exact_is_equal(cd_decrease_ct, 0) )
                        lr = cd_learning_rate / (1.0 + stage * cd_decrease_ct );
                    else
                        lr = cd_learning_rate;

                    if( targetsize() > 0 )
                        lr *= generative_learning_weight;
                    
                    lr *= (1-persistent_cd_weight);

                    if( weightsize > 0 )
                        lr *= weight;

                    setLearningRate(lr);

                    // Positive phase
                    pos_input = input;
                    if( targetsize() > 0)
                    {
                        if( is_missing(target[0]) )
                        {
                            // Sample from p(y|x)
                            lr *= semi_sup_learning_weight/generative_learning_weight;
                            // Get output probabilities
                            connection->setAsDownInput( input );
                            hidden_layer->getAllActivations( 
                                (RBMMatrixConnection*) connection );
                            
                            Vec target_act = target_layer->activation;
                            Vec hidden_act = hidden_layer->activation;
                            for( int i=0 ; i<target_layer->size ; i++ )
                            {
                                target_act[i] = target_layer->bias[i];
                                // LATERAL CONNECTIONS CODE HERE!!
                                real *w = &(target_connection->weights(0,i));
                                // step from one row to the next in weights matrix
                                int m = target_connection->weights.mod();                
                                
                                for( int j=0 ; j<hidden_layer->size ; j++, w+=m )
                                {
                                    // *w = weights(j,i)
                                    hidden_activation_pos_i[j] = hidden_act[j] + *w;
                                }
                                target_act[i] -= hidden_layer->freeEnergyContribution(
                                    hidden_activation_pos_i);
                            }
                            
                            target_layer->expectation_is_up_to_date = false;
                            target_layer->computeExpectation();
                            target_layer->generateSample();
                            target_one_hot << target_layer->sample;
                        }
                        pos_target = target_one_hot;
                    }
                    connection->setAsDownInput( input );
                    hidden_layer->getAllActivations( 
                        (RBMMatrixConnection*) connection );
                    if( targetsize() == 1 )
                        productAcc( hidden_layer->activation,
                                    target_connection->weights,
                                    target_one_hot );
                    else if( targetsize() > 1 )
                        productAcc( hidden_layer->activation,
                                    target_connection->weights,
                                    target );
                        
                    hidden_layer->computeExpectation();
                    //pos_hidden.resize( hidden_layer->size );
                    pos_hidden << hidden_layer->expectation;
                    
                    // Negative phase
                    for(int i=0; i<cd_n_gibbs; i++)
                    {
                        if( use_mean_field_cd )
                        {
                            connection->setAsUpInput( hidden_layer->expectation );
                        }
                        else
                        {
                            hidden_layer->generateSample();
                            connection->setAsUpInput( hidden_layer->sample );
                        }
                        input_layer->getAllActivations( 
                            (RBMMatrixConnection*) connection );
                        input_layer->computeExpectation();
                        // LATERAL CONNECTIONS CODE HERE!

                        if( use_mean_field_cd )
                        {
                            connection->setAsDownInput( input_layer->expectation );
                        }
                        else
                        {
                            input_layer->generateSample();
                            connection->setAsDownInput( input_layer->sample );
                        }

                        hidden_layer->getAllActivations( 
                            (RBMMatrixConnection*) connection );

                        if( targetsize() > 0 )
                        {
                            if( use_mean_field_cd )
                                target_connection->setAsUpInput( 
                                    hidden_layer->expectation );
                            else
                                target_connection->setAsUpInput( 
                                    hidden_layer->sample );
                            target_layer->getAllActivations( 
                                (RBMMatrixConnection*) target_connection );
                            target_layer->computeExpectation();
                            if( use_mean_field_cd )
                                productAcc( hidden_layer->activation,
                                            target_connection->weights,
                                            target_layer->expectation );
                            else
                            {
                                target_layer->generateSample();
                                productAcc( hidden_layer->activation,
                                            target_connection->weights,
                                            target_layer->sample );
                            }   
                        }
                        
                        hidden_layer->computeExpectation();
                    }
                    
                    if( use_mean_field_cd )
                        neg_input = input_layer->expectation;
                    else
                        neg_input = input_layer->sample;

                    neg_hidden = hidden_layer->expectation;
                    
                    input_layer->update(pos_input,neg_input);
                    hidden_layer->update(pos_hidden,neg_hidden);
                    connection->update(pos_input,pos_hidden,
                                       neg_input,neg_hidden);
                    if( targetsize() > 0 )
                    {
                        if( use_mean_field_cd )
                            neg_target = target_layer->expectation;
                        else
                            neg_target = target_layer->sample;
                        target_layer->update(pos_target,neg_target);
                        target_connection->update(pos_target,pos_hidden,
                                                  neg_target,neg_hidden);
                    }
                }

                if( !fast_exact_is_equal(persistent_cd_weight, 0.) )
                {
                    if( use_mean_field_cd )
                        PLERROR("In PseudolikelihoodRBM::train(): Persistent "
                                "Contrastive Divergence was not implemented for "
                                "MF-CD");

                    if( !fast_exact_is_equal(cd_decrease_ct, 0) )
                        lr = cd_learning_rate / (1.0 + stage * cd_decrease_ct );
                    else
                        lr = cd_learning_rate;
                    
                    if( targetsize() > 0 )
                        lr *= generative_learning_weight;

                    lr *= persistent_cd_weight;

                    if( weightsize > 0 )
                        lr *= weight;

                    setLearningRate(lr);

                    int chain_i = stage % n_gibbs_chains;

                    if( !persistent_gibbs_chain_is_started[chain_i] )
                    {  
                        // Start gibbs chain
                        connection->setAsDownInput( input );
                        hidden_layer->getAllActivations( 
                            (RBMMatrixConnection*) connection );
                        if( targetsize() == 1 )
                            productAcc( hidden_layer->activation,
                                        target_connection->weights,
                                        target_one_hot );
                        else if( targetsize() > 1 )
                            productAcc( hidden_layer->activation,
                                        target_connection->weights,
                                        target );
                        
                        hidden_layer->computeExpectation();
                        hidden_layer->generateSample();
                        pers_cd_hidden[chain_i] << hidden_layer->sample;
                        persistent_gibbs_chain_is_started[chain_i] = true;
                    }

                    if( fast_exact_is_equal(persistent_cd_weight, 1.) )
                    {
                        // Hidden positive sample was not computed previously
                        connection->setAsDownInput( input );
                        hidden_layer->getAllActivations( 
                            (RBMMatrixConnection*) connection );
                        if( targetsize() == 1 )
                            productAcc( hidden_layer->activation,
                                        target_connection->weights,
                                        target_one_hot );
                        else if( targetsize() > 1 )
                            productAcc( hidden_layer->activation,
                                        target_connection->weights,
                                        target );
                            
                        hidden_layer->computeExpectation();
                        pos_hidden << hidden_layer->expectation;
                    }

                    hidden_layer->sample << pers_cd_hidden[chain_i];
                    // Prolonged Gibbs chain
                    for(int i=0; i<cd_n_gibbs; i++)
                    {
                        connection->setAsUpInput( hidden_layer->sample );
                        input_layer->getAllActivations( 
                            (RBMMatrixConnection*) connection );
                        input_layer->computeExpectation();
                        // LATERAL CONNECTIONS CODE HERE!
                        input_layer->generateSample();
                        connection->setAsDownInput( input_layer->sample );
                        hidden_layer->getAllActivations( 
                            (RBMMatrixConnection*) connection );
                        if( targetsize() > 0 )
                        {
                            target_connection->setAsUpInput( hidden_layer->sample );
                            target_layer->getAllActivations( 
                                (RBMMatrixConnection*) target_connection );
                            target_layer->computeExpectation();
                            target_layer->generateSample();
                            productAcc( hidden_layer->activation,
                                        target_connection->weights,
                                        target_layer->sample );
                        }
                        hidden_layer->computeExpectation();
                        hidden_layer->generateSample();
                    }

                    pers_cd_hidden[chain_i] << hidden_layer->sample;

                    input_layer->update(input, input_layer->sample);
                    hidden_layer->update(pos_hidden,hidden_layer->expectation);
                    connection->update(input,pos_hidden,
                                       input_layer->sample,
                                       hidden_layer->expectation);
                    if( targetsize() > 0 )
                    {
                        target_layer->update(target_one_hot, target_layer->sample);
                        target_connection->update(target_one_hot,pos_hidden,
                                                  target_layer->sample,
                                                  hidden_layer->expectation);
                    }
                }
            }
        }
        
        if( !fast_exact_is_equal(denoising_learning_rate, 0.) &&
            (targetsize() == 0 || generative_learning_weight > 0) )
        {
            if( !fast_exact_is_equal(denoising_decrease_ct, 0) )
                lr = denoising_learning_rate / 
                    (1.0 + stage * denoising_decrease_ct );
            else
                lr = denoising_learning_rate;

            if( targetsize() > 0 )
                lr *= generative_learning_weight;

            if( weightsize > 0 )
                lr *= weight;

            setLearningRate(lr);
            if( targetsize() > 0 )
                PLERROR("In PseudolikelihoodRBM::train(): denoising "
                        "autoencoder training is not implemented for "
                        "targetsize() > 0"); 

            if( input_is_sparse )
                PLERROR("In PseudolikelihoodRBM::train(): denoising autoencoder "
                        "training is not implemented for sparse inputs");


            if( fraction_of_masked_inputs > 0 )
                random_gen->shuffleElements(autoencoder_input_indices);
                
            masked_autoencoder_input << input;
            if( fraction_of_masked_inputs > 0 )
            {
                for( int j=0 ; 
                     j < round(fraction_of_masked_inputs*input_layer->size) ; 
                     j++)
                    masked_autoencoder_input[ autoencoder_input_indices[j] ] = 0; 
            }

            // Somehow, doesn't compile without the fancy casts...
            ((RBMMatrixConnection *)connection)->RBMConnection::fprop( masked_autoencoder_input, 
                                                                       hidden_layer->activation );

            hidden_layer->fprop( hidden_layer->activation,
                                 hidden_layer->expectation );
                
            transpose_connection->fprop( hidden_layer->expectation,
                                         input_layer->activation );
            input_layer->fprop( input_layer->activation,
                                input_layer->expectation );
            input_layer->setExpectation( input_layer->expectation );

            real cost = input_layer->fpropNLL(input);
                
            input_layer->bpropNLL(input, cost, 
                                  reconstruction_activation_gradient);
            if( only_reconstruct_masked_inputs && 
                fraction_of_masked_inputs > 0 )
            {
                for( int j=(int)round(fraction_of_masked_inputs*input_layer->size) ; 
                     j < input_layer->size ; 
                     j++)
                    reconstruction_activation_gradient[ 
                        autoencoder_input_indices[j] ] = 0; 
            }
            input_layer->update( reconstruction_activation_gradient );

            transpose_connection->bpropUpdate( 
                hidden_layer->expectation,
                input_layer->activation,
                hidden_layer_expectation_gradient,
                reconstruction_activation_gradient );

            hidden_layer->bpropUpdate( hidden_layer->activation,
                                       hidden_layer->expectation,
                                       hidden_layer_activation_gradient,
                                       hidden_layer_expectation_gradient );
                
            connection->bpropUpdate( masked_autoencoder_input, 
                                     hidden_layer->activation,
                                     reconstruction_activation_gradient, // is not used afterwards...
                                     hidden_layer_activation_gradient );
        }

//        }
        train_stats->update( train_costs );
        
    }
    
    Profiler::end("training");
    const Profiler::Stats& stats = Profiler::getStats("training");
    real ticksPerSec = Profiler::ticksPerSecond();
    real cpu_time = (stats.user_duration+stats.system_duration)/ticksPerSec;
    cumulative_training_time += cpu_time;

    train_costs.fill(MISSING_VALUE);
    train_costs[training_cpu_time_cost_index] = cpu_time;
    train_costs[cumulative_training_time_cost_index] = cumulative_training_time;
    train_stats->update( train_costs );
    
    //cout << "mean_pseudolikelihood=" << mean_pseudolikelihood / (stage - init_stage) << endl;
    // Sums to 1 test
    //compute_Z();
    //conf.resize( input_layer->size );
    //Vec output,costs;
    //output.resize(outputsize());
    //costs.resize(getTestCostNames().length());
    //target.resize( targetsize() );
    //real sums = 0;
    //int input_n_conf = input_layer->getConfigurationCount();
    //for(int i=0; i<input_n_conf; i++)
    //{
    //    input_layer->getConfiguration(i,conf);
    //    computeOutput(conf,output);
    //    computeCostsFromOutputs( conf, output, target, costs );
    //    if( i==0 )
    //        sums = -costs[nll_cost_index];
    //    else
    //        sums = logadd( sums, -costs[nll_cost_index] );
    //    //sums += safeexp( -costs[nll_cost_index] );
    //}        
    //cout << "sums: " << safeexp(sums) << endl;
    //    //sums << endl;
    train_stats->finalize();
}

void PseudolikelihoodRBM::test(VMat testset, PP<VecStatsCollector> test_stats,
                               VMat testoutputs, VMat testcosts) const
{
    if( !input_is_sparse )
    {
        inherited::test( testset, test_stats, testoutputs, testcosts );
        return;
    }

    Profiler::pl_profile_start("PLearner::test");

    int len = testset.length();
    Vec input;
    Vec target;
    Vec extra;
    real weight;
    int out_size = outputsize() >= 0 ? outputsize() : 0;
    int target_index;

    if( targetsize() <= 0 )
        PLERROR("PseudolikelihoodRBM::test(): targetsize() must be "
            "> 0 for sparse inputs");

    Vec output(out_size);
    Vec costs(nTestCosts());

    if (test_stats) {
        // Set names of test_stats costs
        test_stats->setFieldNames(getTestCostNames());

        if (len == 0) {
            // Empty test set: we give -1 cost arbitrarily.
            costs.fill(-1);
            test_stats->update(costs);
        }
    }

    PP<ProgressBar> pb;
    if (report_progress)
        pb = new ProgressBar("Testing learner", len);

    PP<PRandom> copy_random_gen=0;
    if (use_a_separate_random_generator_for_testing && random_gen)
    {
        CopiesMap copies;
        copy_random_gen = random_gen->deepCopy(copies);
        random_gen->manual_seed(use_a_separate_random_generator_for_testing);
    }

    Vec target_act = target_layer->activation;
    Vec hidden_act = hidden_layer->activation;
    for (int l = 0; l < len; l++)
    {
        testset.getExample(l, input, target, weight);
        testset->getExtra(l, extra );

        if( targetsize() == 1 )
        {

            target_one_hot.clear();
            target_index = (int)round( target[0] );
            target_one_hot[ target_index ] = 1;

            if( factorized_connection_rank > 0 )
            {
                Vx.clear();
                for( int e=0; e<extra.length(); e++ )
                    Vx += V((int)extra[e]);
                
                product(hidden_act,U,Vx);
            }
            else
            {
                hidden_act.clear();
                for( int e=0; e<extra.length(); e++ )
                    hidden_act += V((int)extra[e]);
            }
            hidden_act += hidden_layer->bias;

            for( int i=0 ; i<target_layer->size ; i++ )
            {
                target_act[i] = target_layer->bias[i];
                // LATERAL CONNECTIONS CODE HERE!!
                real *w = &(target_connection->weights(0,i));
                // step from one row to the next in weights matrix
                int m = target_connection->weights.mod();                
                
                for( int j=0 ; j<hidden_layer->size ; j++, w+=m )
                {
                    // *w = weights(j,i)
                    hidden_activation_pos_i[j] = hidden_act[j] + *w;
                }
                target_act[i] -= hidden_layer->freeEnergyContribution(
                    hidden_activation_pos_i);
            }
            
            target_layer->expectation_is_up_to_date = false;
            target_layer->computeExpectation();
            output << target_layer->expectation;
            real nll = target_layer->fpropNLL(target_one_hot);
            costs.fill( MISSING_VALUE );
            costs[nll_cost_index] = nll;
            costs[class_cost_index] = 
                (argmax(target_layer->expectation) == target_index)? 0 : 1;
        }
        else if( targetsize() > 1 )
            PLERROR("PseudolikelihoodRBM::test(): targetsize() > 1 "
                    "not implemented yet for sparse inputs");
        costs[cumulative_training_time_cost_index] = cumulative_training_time;
        if (testoutputs) testoutputs->putOrAppendRow(l, output);
        if (testcosts) testcosts->putOrAppendRow(l, costs);
        if (test_stats) test_stats->update(costs, weight);
        if (report_progress) pb->update(l);
    }

    if (use_a_separate_random_generator_for_testing && random_gen)
        *random_gen = *copy_random_gen;

    Profiler::pl_profile_end("PLearner::test");

}

///////////////////
// computeOutput //
///////////////////
void PseudolikelihoodRBM::computeOutput(const Vec& input, Vec& output) const
{
    if( input_is_sparse )
        PLERROR("In PseudolikelihoodRBM::computeOutput(): "
                "not compatible with sparse inputs");

    // Compute the output from the input.
    if( targetsize() == 1 )
    {
        // Get output probabilities
        connection->setAsDownInput( input );
        hidden_layer->getAllActivations( 
            (RBMMatrixConnection*) connection );
        
        Vec target_act = target_layer->activation;
        Vec hidden_act = hidden_layer->activation;
        for( int i=0 ; i<target_layer->size ; i++ )
        {
            target_act[i] = target_layer->bias[i];
            // LATERAL CONNECTIONS CODE HERE!!
            real *w = &(target_connection->weights(0,i));
            // step from one row to the next in weights matrix
            int m = target_connection->weights.mod();                
            
            for( int j=0 ; j<hidden_layer->size ; j++, w+=m )
            {
                // *w = weights(j,i)
                hidden_activation_pos_i[j] = hidden_act[j] + *w;
            }
            target_act[i] -= hidden_layer->freeEnergyContribution(
                hidden_activation_pos_i);
        }
        
        target_layer->expectation_is_up_to_date = false;
        target_layer->computeExpectation();
        output << target_layer->expectation;
    }
    else if(targetsize() > 1 )
    {
        PLERROR("In PseudolikelihoodRBM::computeOutput(): not implemented yet for\n"
                "targetsize() > 1");
    }
    else
    {
        // Get hidden layer representation
        connection->setAsDownInput( input );
        hidden_layer->getAllActivations( (RBMMatrixConnection *) connection );
        hidden_layer->computeExpectation();
        output << hidden_layer->expectation;
    }
}


void PseudolikelihoodRBM::computeCostsFromOutputs(const Vec& input, 
                                                  const Vec& output,
                                                  const Vec& target, 
                                                  Vec& costs) const
{

    if( input_is_sparse )
        PLERROR("In PseudolikelihoodRBM::computeCostsFromOutputs(): "
                "not compatible with sparse inputs");

    // Compute the costs from *already* computed output.
    costs.resize( cost_names.length() );
    costs.fill( MISSING_VALUE );

    if( targetsize() == 1 )
    {
        if( !is_missing(target[0]) )
        {
            costs[class_cost_index] =
                (argmax(output) == (int) round(target[0]))? 0 : 1;
            costs[nll_cost_index] = -pl_log(output[(int) round(target[0])]);
        }
    }
    else if( targetsize() > 1 )
    {
        PLERROR("In PseudolikelihoodRBM::computeCostsFromOutputs(): not implemented yet for\n"
                "targetsize() > 1");
    }
    else
    {        
        if( compute_input_space_nll )
        {
            compute_Z();
            connection->setAsDownInput( input );
            hidden_layer->getAllActivations( (RBMMatrixConnection *) connection );
            costs[nll_cost_index] = hidden_layer->freeEnergyContribution(
                hidden_layer->activation) - dot(input,input_layer->bias);
            if( compute_Z_exactly )
                costs[nll_cost_index] += log_Z;
            else if( use_ais_to_compute_Z )
                costs[nll_cost_index] += log_Z_ais;
            else
                PLERROR("In PseudolikelihoodRBM::computeCostsFromOutputs(): "
                    "can't compute NLL without a mean to compute log(Z).");

            if( compute_Z_exactly )
            {
                costs[log_Z_cost_index] = log_Z;
            }
            if( use_ais_to_compute_Z )
            {
                costs[log_Z_ais_cost_index] = log_Z_ais;
                costs[log_Z_interval_lower_cost_index] = log_Z_down;
                costs[log_Z_interval_upper_cost_index] = log_Z_up;
            }
        }
    }
    costs[cumulative_training_time_cost_index] = cumulative_training_time;
}

TVec<string> PseudolikelihoodRBM::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).

    return cost_names;
}

TVec<string> PseudolikelihoodRBM::getTrainCostNames() const
{
    return cost_names;
}


//#####  Helper functions  ##################################################

void PseudolikelihoodRBM::setLearningRate( real the_learning_rate )
{
    input_layer->setLearningRate( the_learning_rate );
    hidden_layer->setLearningRate( the_learning_rate );
    if( connection ) 
        connection->setLearningRate( the_learning_rate );
    if( target_layer )
        target_layer->setLearningRate( the_learning_rate );
    if( target_connection )
        target_connection->setLearningRate( the_learning_rate );
}

void PseudolikelihoodRBM::compute_Z() const
{

    int input_n_conf = input_layer->getConfigurationCount(); 
    int hidden_n_conf = hidden_layer->getConfigurationCount();
    if( !Z_is_up_to_date && compute_Z_exactly &&
        input_n_conf == RBMLayer::INFINITE_CONFIGURATIONS && 
        hidden_n_conf == RBMLayer::INFINITE_CONFIGURATIONS )
        PLERROR("In PseudolikelihoodRBM::computeCostsFromOutputs: "
                "RBM's input and hidden layers are too big "
                "for exact NLL computations.");

    if( !Z_ais_is_up_to_date && use_ais_to_compute_Z )
    {
        log_Z_ais = 0;
        // This AIS code is based on the Matlab code of Russ, on his web page //

        // Compute base-rate RBM biases
        Vec input( inputsize() );
        Vec target( targetsize() );
        real weight;
        Vec base_rate_rbm_bias( inputsize() );
        base_rate_rbm_bias.clear();
        for( int i=0; i<train_set->length(); i++ )
        {
            train_set->getExample(i, input, target, weight);
            base_rate_rbm_bias += input;
        }
        base_rate_rbm_bias += 0.05*train_set->length();
        base_rate_rbm_bias /= 1.05*train_set->length();
        for( int j=0; j<inputsize(); j++ )
            base_rate_rbm_bias[j] = pl_log( base_rate_rbm_bias[j] ) - 
                pl_log( 1-base_rate_rbm_bias[j] );
        
        Mat ais_chain_init_samples( n_ais_chains,inputsize() );
        Vec ais_weights( n_ais_chains );
        ais_weights.clear(); // we'll work on log-scale
        real beg_beta, end_beta, beta, step_beta;
        int n_beta;
        
        // Start chains
        real p_j;
        for( int j=0; j<input_layer->size; j++ )
        {
            p_j = sigmoid( base_rate_rbm_bias[j] );
            for( int c=0; c<n_ais_chains; c++ )
                ais_chain_init_samples(c,j) = random_gen->binomial_sample( p_j );
        }
        input_layer->setBatchSize( n_ais_chains );
        input_layer->samples << ais_chain_init_samples;

        // Add importance weight contribution (denominator)
        productScaleAcc( ais_weights, input_layer->samples, false,
                         base_rate_rbm_bias, -1, 0 );
        ais_weights -= hidden_layer->size * pl_log(2);
        for( int k=0; k<ais_beta_n_steps.length(); k++ )
        {
            beg_beta = (k==0) ? 0 : ais_beta_begin[k];
            end_beta = (k == ais_beta_end.length()-1) ? 1 : ais_beta_end[k];
            if( beg_beta >= end_beta )
                PLERROR("In PseudolikelihoodRBM::compute_Z(): "
                        "the AIS beta schedule is not monotonically increasing.");

            n_beta = ais_beta_n_steps[k];
            if( n_beta == 0)
                PLERROR("In PseudolikelihoodRBM::compute_Z(): "
                        "one of the beta intervals has 0 steps.");
            step_beta = (end_beta - beg_beta)/n_beta;

            beta = beg_beta;
            for( int k_i=0; k_i < n_beta; k_i++ )
            {
                beta += step_beta;
                // Add importance weight contribution (numerator)
                productScaleAcc( ais_weights, input_layer->samples, false,
                                 base_rate_rbm_bias, (1-beta), 1 );
                productScaleAcc( ais_weights, input_layer->samples, false,
                                 input_layer->bias, beta, 1 );
                connection->setAsDownInputs(input_layer->samples);
                hidden_layer->getAllActivations( 
                    (RBMMatrixConnection *) connection, 0, true );
                hidden_layer->activations *= beta;
                for( int c=0; c<n_ais_chains; c++ )
                    ais_weights[c] -= hidden_layer->freeEnergyContribution( 
                        hidden_layer->activations(c) );
                // Get new chain sample
                hidden_layer->computeExpectations();
                hidden_layer->generateSamples();
                connection->setAsUpInputs(hidden_layer->samples);
                input_layer->getAllActivations( 
                    (RBMMatrixConnection *) connection, 0, true );
                for( int c=0; c<n_ais_chains; c++ )
                    multiplyScaledAdd(base_rate_rbm_bias,beta,
                                      (1-beta),input_layer->activations(c));
                input_layer->computeExpectations();
                input_layer->generateSamples();

                // Add importance weight contribution (denominator)
                productScaleAcc( ais_weights, input_layer->samples, false,
                                 base_rate_rbm_bias, -(1-beta), 1 );
                productScaleAcc( ais_weights, input_layer->samples, false,
                                 input_layer->bias, -beta, 1 );
                connection->setAsDownInputs(input_layer->samples);
                hidden_layer->getAllActivations( 
                    (RBMMatrixConnection *) connection, 0, true );
                hidden_layer->activations *= beta;
                for( int c=0; c<n_ais_chains; c++ )
                    ais_weights[c] += hidden_layer->freeEnergyContribution( 
                        hidden_layer->activations(c) );
            }
        }
        // Final importance weight contribution, at beta=1 (numerator)
        productScaleAcc( ais_weights, input_layer->samples, false,
                         input_layer->bias, 1, 1 );
        connection->setAsDownInputs(input_layer->samples);
        hidden_layer->getAllActivations( 
            (RBMMatrixConnection *) connection, 0, true );
        for( int c=0; c<n_ais_chains; c++ )
            ais_weights[c] -= hidden_layer->freeEnergyContribution( 
                hidden_layer->activations(c) );

        real log_r_ais = logadd(ais_weights) - pl_log(n_ais_chains);
        real log_Z_base =  hidden_layer->size * pl_log(2);
        for( int j=0; j<inputsize(); j++ )
            log_Z_base += softplus(base_rate_rbm_bias[j]);
        log_Z_ais = log_r_ais + log_Z_base;

        real offset = mean(ais_weights);
        PP<StatsCollector> stats = new StatsCollector();
        stats->forget();
        for( int c=0; c<n_ais_chains; c++ )
            stats->update(exp(ais_weights[c]-offset),1.);
        stats->finalize();
        real logstd_ais = pl_log(stats->getStat("STDDEV")) + 
            offset - pl_log(n_ais_chains)/2;
        log_Z_up = pl_log(exp(log_r_ais)+exp(logstd_ais)*3) + log_Z_base;
        log_Z_down = pl_log(exp(log_r_ais)-exp(logstd_ais)*3) + log_Z_base;

        Z_ais_is_up_to_date = true;
    }
    if( !Z_is_up_to_date && compute_Z_exactly )
    {
        log_Z = 0;
        if( input_n_conf < hidden_n_conf )
        {
            conf.resize( input_layer->size );
            for(int i=0; i<input_n_conf; i++)
            {
                input_layer->getConfiguration(i,conf);
                connection->setAsDownInput( conf );
                hidden_layer->getAllActivations( (RBMMatrixConnection *) connection );
                if( i == 0 )
                    log_Z = -hidden_layer->freeEnergyContribution(
                        hidden_layer->activation) + dot(conf,input_layer->bias);
                else
                    log_Z = logadd(-hidden_layer->freeEnergyContribution(
                                       hidden_layer->activation) 
                                   + dot(conf,input_layer->bias),
                                   log_Z);
            }
        }
        else
        {
            conf.resize( hidden_layer->size );
            for(int i=0; i<hidden_n_conf; i++)
            {
                hidden_layer->getConfiguration(i,conf);
                connection->setAsUpInput( conf );
                input_layer->getAllActivations( (RBMMatrixConnection *) connection );
                if( i == 0 )
                    log_Z = -input_layer->freeEnergyContribution(
                        input_layer->activation) + dot(conf,hidden_layer->bias);
                else
                    log_Z = logadd(-input_layer->freeEnergyContribution(
                                       input_layer->activation)
                                   + dot(conf,hidden_layer->bias),
                                   log_Z);
            }        
        }
        Z_is_up_to_date = true;
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
