// -*- C++ -*-

// NnlmOnlineLearner.cc
//
// Copyright (C) 2006 Pierre-Antoine Manzagol
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

// Authors: Pierre-Antoine Manzagol

/*! \file NnlmOnlineLearner.cc */


#include "NnlmOnlineLearner.h"

#include <plearn/math/PRandom.h>
#include <plearn/math/TMat_maths.h>
#include <plearn_learners/online/OnlineLearningModule.h>

#include <plearn/vmat/VMat.h>
// necessary?
#include <plearn_learners_experimental/onlineNNLM/NnlmWordRepresentationLayer.h>
#include <plearn_learners/online/GradNNetLayerModule.h>
#include <plearn_learners/online/TanhModule.h>
#include <plearn_learners/online/NLLErrModule.h>
#include <plearn_learners_experimental/onlineNNLM/NnlmOutputLayer.h>

#include <plearn_learners/distributions/NGramDistribution.h>
#include <plearn_learners/distributions/SymbolNode.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NnlmOnlineLearner,
    "Trains a Neural Network Language Model.",
    "MULTI-LINE \nHELP");


//////////////////////
// class wordAndFreq
//////////////////////
//! Used to sort words according to frequency, when determining candidates
class wordAndFreq {
public:
  wordAndFreq(int wt, int f) : wordtag(wt), frequency(f){};
  int wordtag;
  double frequency;
};
bool wordAndFreqGT(const wordAndFreq &a, const wordAndFreq &b) 
{
    return a.frequency > b.frequency;
}

////////////////////////
// NnlmOnlineLearner()
////////////////////////
NnlmOnlineLearner::NnlmOnlineLearner()
    :   PLearner(),
        word_representation_size( 50 ),
        semantic_layer_size( 200 ),
        wrl_slr( 0.1 ),
        wrl_dc( 0.0 ),
        wrl_wd_l1( 0.0 ),
        wrl_wd_l2( 0.0 ),
        sl_slr( 0.1 ),
        sl_dc( 0.0 ),
        sl_wd_l1( 0.0 ),
        sl_wd_l2( 0.0 ),
        str_gaussian_model_train_cost( "approx_discriminant" ),
        str_gaussian_model_learning( "non_discriminant" ),
        gaussian_model_sigma2_min(0.000001),
        gaussian_model_dl_slr(0.0001),
        shared_candidates_size( 0 ),
        ngram_candidates_size( 50 ),
        self_candidates_size( 0 ),
        sm_slr( 0.1 ),
        sm_dc( 0.0 ),
        sm_wd_l1( 0.0 ),
        sm_wd_l2( 0.0 ),
        vocabulary_size( -1 ),
        context_size( -1 ),
        nmodules( -1 ),
        output_nmodules( -1 ),
        model_type( -1 ),
        gaussian_model_cost( -1 ),
        gaussian_model_learning( -1 )
{
    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)

    random_gen = new PRandom();
}

///////////////////
// declareOptions
///////////////////
void NnlmOnlineLearner::declareOptions(OptionList& ol)
{

    // *** Build Options *** 

    // * Model type * 
    declareOption(ol, "str_input_model",
                  &NnlmOnlineLearner::str_input_model,
                  OptionBase::buildoption,
                  "Specifies what's used as input layer: wrl (default - word representation layer) or gnnl (gradnnetlayer).");
    declareOption(ol, "str_output_model",
                  &NnlmOnlineLearner::str_output_model,
                  OptionBase::buildoption,
                  "Specifies what's used on top of the semantic layer: 'softmax' or 'gaussian'(default).");

    // * Model size * 
    declareOption(ol, "word_representation_size",
                  &NnlmOnlineLearner::word_representation_size,
                  OptionBase::buildoption,
                  "Size of the real distributed word representation.");

    declareOption(ol, "semantic_layer_size",
                  &NnlmOnlineLearner::semantic_layer_size,
                  OptionBase::buildoption,
                  "Size of the semantic layer.");

    // * Same part parameters
    declareOption(ol, "wrl_slr",
                  &NnlmOnlineLearner::wrl_slr,
                  OptionBase::buildoption,
                  "Word representation layer start learning rate.");
    declareOption(ol, "wrl_dc",
                  &NnlmOnlineLearner::wrl_dc,
                  OptionBase::buildoption,
                  "Word representation layer decrease constant.");
    declareOption(ol, "wrl_wd_l1",
                  &NnlmOnlineLearner::wrl_wd_l1,
                  OptionBase::buildoption,
                  "Word representation layer L1 penalty factor.");
    declareOption(ol, "wrl_wd_l2",
                  &NnlmOnlineLearner::wrl_wd_l2,
                  OptionBase::buildoption,
                  "Word representation layer L2 penalty factor.");
    declareOption(ol, "sl_slr",
                  &NnlmOnlineLearner::sl_slr,
                  OptionBase::buildoption,
                  "Semantic layer start learning rate.");
    declareOption(ol, "sl_dc",
                  &NnlmOnlineLearner::sl_dc,
                  OptionBase::buildoption,
                  "Semantic layer decrease constant.");
    declareOption(ol, "sl_wd_l1",
                  &NnlmOnlineLearner::sl_wd_l1,
                  OptionBase::buildoption,
                  "Semantic layer L1 penalty factor.");
    declareOption(ol, "sl_wd_l2",
                  &NnlmOnlineLearner::sl_wd_l2,
                  OptionBase::buildoption,
                  "Semantic layer L2 penalty factor.");


    // * Gaussian model specific

    // - model behavior
    // TODO how about combining the two costs: maybe jumpstart with one
    declareOption(ol, "str_gaussian_model_train_cost",
                  &NnlmOnlineLearner::str_gaussian_model_train_cost,
                  OptionBase::buildoption,
                  "In case of a gaussian output module, specifies the cost used for training: 'approx_discriminant' (default - evaluates p(i|r) from p(r|i), using some candidates for normalization) or 'non_discriminant' (uses p(r|i)).");

    declareOption(ol, "str_gaussian_model_learning",
                  &NnlmOnlineLearner::str_gaussian_model_learning,
                  OptionBase::buildoption,
                  "In case of a gaussian output module, specifies the learning technique: 'discriminant' or 'non_discriminant' (default - evaluates empirical mu and sigma).");

    declareOption(ol, "gaussian_model_sigma2_min",
                  &NnlmOnlineLearner::gaussian_model_sigma2_min,
                  OptionBase::buildoption,
                  "In case of a gaussian output module, specifies the minimal sigma^2.");

    declareOption(ol, "gaussian_model_dl_slr",
                  &NnlmOnlineLearner::gaussian_model_dl_slr,
                  OptionBase::buildoption,
                  "In case of a gaussian output module with discriminant learning, this specifies the starting learning rate.");

    // - Candidate set sizes
    declareOption(ol, "shared_candidates_size",
                  &NnlmOnlineLearner::shared_candidates_size,
                  OptionBase::buildoption,
                  "Number of candidates drawn from frequent words in aproximated discriminant cost evaluation.");

    declareOption(ol, "ngram_candidates_size",
                  &NnlmOnlineLearner::ngram_candidates_size,
                  OptionBase::buildoption,
                  "Number of candidates drawn from the context (bigram) in aproximated discriminant cost evaluation.");

    declareOption(ol, "self_candidates_size",
                  &NnlmOnlineLearner::self_candidates_size,
                  OptionBase::buildoption,
                  "Number of candidates drawn from the nnlm in aproximated discriminant cost evaluation  (evaluated periodically). NOT IMPLEMENTED!!");

    // - Ngram (for evaluating ngram candidates) train set
    declareOption(ol, "ngram_train_set",
                  &NnlmOnlineLearner::ngram_train_set,
                  OptionBase::buildoption,
                  "Train set used for training the bigram used in the evaluation of the set of candidate words used for normalization   in the evaluated discriminant cost (ProcessSymbolicSequenceVMatrix) (ONLY BIGRAMS).");

    // * Softmax specific

    declareOption(ol, "sm_slr",
                  &NnlmOnlineLearner::sm_slr,
                  OptionBase::buildoption,
                  "Softmax layer start learning rate.");
    declareOption(ol, "sm_dc",
                  &NnlmOnlineLearner::sm_dc,
                  OptionBase::buildoption,
                  "Softmax layer decrease constant.");
    declareOption(ol, "sm_wd_l1",
                  &NnlmOnlineLearner::sm_wd_l1,
                  OptionBase::buildoption,
                  "Softmax layer L1 penalty factor.");
    declareOption(ol, "sm_wd_l2",
                  &NnlmOnlineLearner::sm_wd_l2,
                  OptionBase::buildoption,
                  "Softmax layer L2 penalty factor.");


    // *** Learnt Options *** 

    declareOption(ol, "modules", &NnlmOnlineLearner::modules,
                  OptionBase::buildoption,
                  "Layers of the learner");

    declareOption(ol, "output_modules", &NnlmOnlineLearner::output_modules,
                  OptionBase::buildoption,
                  "Output layers");

    // TODO Are there missing things here?

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

//////////
// build
//////////
void NnlmOnlineLearner::build()
{
    inherited::build();
    build_();
}


///////////
// build_
///////////
void NnlmOnlineLearner::build_()
{
    cout << "NnlmOnlineLearner::build_()" << endl;

    if( !train_set )  {
        return;
    }

    // *** Sanity Checks ***
    // *** Sanity Checks ***
    /*int word_representation_size
    int semantic_layer_size
    real wrl_slr;
    real wrl_dc;
    real wrl_wd_l1;
    real wrl_wd_l2;
    real sl_slr;
    real sl_dc;
    real sl_wd_l1;
    real sl_wd_l2;
    real gaussian_model_sigma2_min
    int shared_candidates_size;
    int ngram_candidates_size;
    int self_candidates_size;
    real sm_slr;
    real sm_dc;
    real sm_wd_l1;
    real sm_wd_l2;*/


    // *** Model related ***
    // *** Model related ***

    // * Model type *
    string mt = lowerstring( str_output_model );
    if(  mt == "gaussian" || mt == "" )  {
        model_type = MODEL_TYPE_GAUSSIAN;
    } else if( mt == "softmax" )  {
        model_type = MODEL_TYPE_SOFTMAX;
    } else  {
        PLERROR( "'%s' model type is unknown.\n", mt.c_str() );
    }


    if( model_type == MODEL_TYPE_GAUSSIAN ) {

        // * Gaussian model cost *
        string gmc = lowerstring( str_gaussian_model_train_cost );
        if( gmc == "approx_discriminant" || gmc == "" )  {
            gaussian_model_cost = GAUSSIAN_COST_APPROX_DISCR;
        } else if( gmc == "non_discriminant" )  {
            gaussian_model_cost = GAUSSIAN_COST_NON_DISCR;
        } else if( gmc == "discriminant" )  {
            gaussian_model_cost = GAUSSIAN_COST_DISCR;
        } else  {
            PLERROR( "'%s' gaussian model train cost is unknown.\n", gmc.c_str() );
        }

        // * Gaussian model learning *
        string gml = lowerstring( str_gaussian_model_learning );
        if( gml == "non_discriminant" || gml == "" )  {
            gaussian_model_learning = GAUSSIAN_LEARNING_EMPIRICAL;
        } else if( gml == "discriminant" )  {
            gaussian_model_learning = GAUSSIAN_LEARNING_DISCR;
        } else  {
            PLERROR( "'%s' gaussian model learning is unknown.\n", gml.c_str() );
        }
    }


    // *** Vocabulary size ***
    // *** Vocabulary size ***

    // the train set's dictionary_size +1 for the 'OOV' tag (tag 0) +1 for the 'missing' tag (tag 'dict_size+1')
    vocabulary_size = (train_set->getDictionary(0))->size()+2;

    if( verbosity > 0 ) {
        cout << "vocabulary_size " << vocabulary_size << endl;
    }

    // Ensure MINIMAL dictionary coherence, ie size, with ngram set
    if( model_type == MODEL_TYPE_GAUSSIAN ) {
        if( vocabulary_size != (ngram_train_set->getDictionary(0))->size()+2 )  {
            PLERROR("train_set and ngram_train_set have dictionaries of different sizes.\n");
        }
    }


    // *** Context size ***
    // *** Context size ***

    // The ProcessSymbolicSequenceVMatrix has only input. Last input is used as target.
    context_size = inputsize()-1;

    if( verbosity > 0 ) {
        cout << "context_size " << context_size << endl;
    }


    // *** Build modules and output_module ***
    // *** Build modules and output_module ***
    buildLayers();


    // *** Build candidates ***
    // *** Build candidates ***
    if( model_type == MODEL_TYPE_GAUSSIAN )  {

        buildCandidates();

        PP<NnlmOutputLayer> p_nol;
        if( !(p_nol = dynamic_cast<NnlmOutputLayer*>( (OnlineLearningModule*) output_modules[0] ) ) )
        {
            PLERROR("NnlmOnlineLearner::build_() - MODEL_TYPE_GAUSSIAN but output_modules[0] is not an NnlmOutputLayer");
        }

        // TODO clean this
        // point to the same place
        p_nol->shared_candidates = shared_candidates;
        p_nol->candidates = candidates;

        // TODO Set learning method - discriminant or non-discriminant

        // Set Cost 
        if( gaussian_model_cost == GAUSSIAN_COST_APPROX_DISCR ) {
            p_nol->setCost(GAUSSIAN_COST_APPROX_DISCR);
        } else if( gaussian_model_cost == GAUSSIAN_COST_NON_DISCR ) {
            p_nol->setCost(GAUSSIAN_COST_NON_DISCR);
        } else { //GAUSSIAN_COST_DISCR
            p_nol->setCost(GAUSSIAN_COST_DISCR);
        }

        evaluateGaussianCounts();

        // Not here, because forget will be called after and it resets mus and sigmas
        // Initialize mus and sigmas using 1 pass
        //reevaluateGaussianParameters();


        // ### Should only be evaluated once
        //p_nol->sumI << p_nol->test_sumI;
        //p_nol->s_sumI = p_nol->test_s_sumI;

    }

    cout << "NnlmOnlineLearner::build_() - DONE!" << endl;
}


////////////////
// buildLayers
////////////////
void NnlmOnlineLearner::buildLayers()
{

    // Do we have to build the layers, or did we load them?
    if( nmodules <= 0 ) {

        //------------------------------------------
        // 1) Fixed part - up to the semantic layer
        //------------------------------------------
        nmodules = 3;
        modules.resize( nmodules );



        string ilm = lowerstring( str_input_model );
        if( ilm == "wrl" || ilm == "" )  {
            // *** Word representation layer ***
            // *** Word representation layer ***
            PP< NnlmWordRepresentationLayer > p_wrl = new NnlmWordRepresentationLayer();

            p_wrl->input_size = context_size;
            p_wrl->output_size = context_size * word_representation_size;

            p_wrl->start_learning_rate = wrl_slr;
            p_wrl->decrease_constant = wrl_dc;
            //TODO
            //p_wrl->L1_penalty_factor = wrl_wd_l1;
            //p_wrl->L2_penalty_factor = wrl_wd_l2;
            p_wrl->vocabulary_size = vocabulary_size;
            p_wrl->word_representation_size = word_representation_size;
            p_wrl->context_size = context_size;
            p_wrl->random_gen = random_gen;

            modules[0] = p_wrl;

        } else if( ilm == "gnnl" )  {
            PP< GradNNetLayerModule > p_nnl = new GradNNetLayerModule();

            p_nnl->input_size = inputsize();
            p_nnl->output_size = inputsize() * word_representation_size;

            p_nnl->start_learning_rate = wrl_slr;
            p_nnl->decrease_constant = wrl_dc;
            p_nnl->L1_penalty_factor = wrl_wd_l1;
            p_nnl->L2_penalty_factor = wrl_wd_l2;

            p_nnl->init_weights_random_scale=sqrt(p_nnl->input_size);
            p_nnl->random_gen = random_gen;

            modules[0] = p_nnl;

        } else  {
            PLERROR( "'%s' input layer model is unknown.\n", ilm.c_str() );
        }



        // *** GradNNetLayer ***
        // *** GradNNetLayer ***
        PP< GradNNetLayerModule > p_nnl = new GradNNetLayerModule();

        p_nnl->input_size = context_size * word_representation_size;
        p_nnl->output_size = semantic_layer_size;

        p_nnl->start_learning_rate = sl_slr;
        p_nnl->decrease_constant = sl_dc;
        p_nnl->L1_penalty_factor = sl_wd_l1;
        p_nnl->L2_penalty_factor = sl_wd_l2;
        p_nnl->init_weights_random_scale=3.0*sqrt(p_nnl->input_size);
        p_nnl->random_gen = random_gen;

        modules[1] = p_nnl;


        // *** Tanh layer ***
        // *** Tanh layer ***
        PP< TanhModule > p_thm = new TanhModule();

        p_thm->input_size = semantic_layer_size;
        p_thm->output_size = semantic_layer_size;

        modules[2] = p_thm;


        //------------------------------------------
        // 2) Variable part - over semantic layer
        //------------------------------------------

        if( model_type == MODEL_TYPE_GAUSSIAN )  {

            output_nmodules = 1;
            output_modules.resize( output_nmodules );


            // *** NnlmOutputLayer ***
            PP< NnlmOutputLayer > p_nol = new NnlmOutputLayer();

            p_nol->input_size = semantic_layer_size;
            p_nol->output_size = 1;
            // the missing tag does NOT get an output (never is the target)
            p_nol->target_cardinality = vocabulary_size-1;
            p_nol->sigma2min = gaussian_model_sigma2_min;
            p_nol->context_cardinality = vocabulary_size;
            p_nol->dl_start_learning_rate = 0.0001;
            //TODO Set cost and learning 
            //int gaussian_model_cost;
            //int gaussian_model_learning;

            output_modules[0] = p_nol;
            output_modules[0]->build();

        } else {

            output_nmodules = 2;
            output_modules.resize( output_nmodules );

            // *** GradNNetLayer ***
            // *** GradNNetLayer ***
            PP< GradNNetLayerModule > p_sm_nnl = new GradNNetLayerModule();

            p_sm_nnl->input_size = semantic_layer_size;  
            // the missing tag does NOT get an output (never is the target)
            p_sm_nnl->output_size = vocabulary_size-1;

            p_sm_nnl->start_learning_rate = sm_slr;
            p_sm_nnl->decrease_constant = sm_dc;
            p_sm_nnl->L1_penalty_factor = sm_wd_l1;
            p_sm_nnl->L2_penalty_factor = sm_wd_l2;
            p_sm_nnl->init_weights_random_scale=3.0*sqrt(p_sm_nnl->input_size);
            p_sm_nnl->random_gen = random_gen;

            output_modules[0] = p_sm_nnl;
            output_modules[0]->build();


            // *** Softmax ***
            output_modules[1] = new NLLErrModule();
            // the missing tag does NOT get an output (never is the target)
            output_modules[1]->input_size = vocabulary_size-1;
            output_modules[1]->output_size = 1;

            output_modules[1]->build();

            //
            output_values.resize( 1 );
            output_gradients.resize( 1 );
            // TODO should improve this
            // +1 so we can add the target in the last spot
            output_values[0].resize( vocabulary_size );
            output_gradients[0].resize( vocabulary_size-1 );
        }
    } 

    // ***  Check on layer size compatibilities, resize values and gradients, and build ***
    // ***  Check on layer size compatibilities, resize values and gradients, and build ***
    // TODO Right now we simply check up to the semantic layer. And we don't check compatibility
    // with context_size and word_representation_size and semantic_layer_size.

    // variables
    values.resize( nmodules+1 );
    gradients.resize( nmodules+1 );

    // first values will be "input" values
    int size = context_size;
    values[0].resize( size );
    gradients[0].resize( size );

    for( int i=0 ; i<nmodules ; i++ )
    {
        PP<OnlineLearningModule> p_module = modules[i];

        if( p_module->input_size != size )
        {
            PLWARNING( "NnlmOnlineLearner::buildLayers(): module '%d'\n"
                       "has an input size of '%d', but previous layer's output"
                       " size\n"
                       "is '%d'. Resizing module '%d'.\n",
                       i, p_module->input_size, size, i);
            p_module->input_size = size;
        }

        p_module->estimate_simpler_diag_hessian = true;

        p_module->build();

        size = p_module->output_size;
        values[i+1].resize( size );
        gradients[i+1].resize( size );
    }


}

////////////////////
// buildCandidates
////////////////////
// TODO use higher order ngrams to build candidates. The present limitation is only on the candidates data structure.
void NnlmOnlineLearner::buildCandidates()
{
    if( model_type != MODEL_TYPE_GAUSSIAN )  {
        PLWARNING("NnlmOnlineLearner::buildCandidates() - model is not of gaussian type. Ignoring call.\n");
        return;
    }

    // *** Train ngram ***
    // *** Train ngram ***

    cout << "NnlmOnlineLearner::buildCandidates()" << endl;
    cout << "\ttraining ngram..." << endl;
    theNGram = new NGramDistribution();

    theNGram->n = ngram_train_set->inputsize();
    theNGram->smoothing = "no_smoothing";
    theNGram->nan_replace = true;
    theNGram->setTrainingSet( ngram_train_set );
    //theNGram->build(); Done in setTrainingSet

    theNGram->train();


    // *** Effective building ***
    // *** Effective building ***

    cout << "\tbuilding candidates..." << endl;

    shared_candidates.resize( shared_candidates_size );
    candidates.resize( vocabulary_size );

    std::vector< wordAndFreq > tmp;
    // temporary list containing the shared candidates
    list<int> l_tmp_shared_candidates;
    list<int>::iterator itr_tmp_shared_candidates;


    // * Determine most frequent words and so the shared_candidates
    TVec<int> unigram( 1 );
    TVec<int> unifreq( 1 );

    // wt means "word tag"
    // Note -> wt=vocabulary_size-1 corresponds to the (-1) tag in the NGramDistribution
    // we skip this tag, the 'missing' tag
    // NOTE Is this appropriate treatment?
    // I don't see how the missing values could occur anywhere except at the beginning so yes.
    for(int wt=0; wt<vocabulary_size-1; wt++)  {
        unigram[0] = wt;
        unifreq = (theNGram->tree)->freq(unigram);
        tmp.push_back( wordAndFreq(wt, unifreq[0]) );
    }

    std::sort(tmp.begin(), tmp.end(), wordAndFreqGT);

    //cout << "These are the shared candidates:" << endl;

    // HACK we don't check if itr has hit the end... unlikely vocabulary_size is smaller
    // than shared_candidates_size
    std::vector< wordAndFreq >::iterator itr_vec;
    itr_vec=tmp.begin();
    for(int i=0; i< shared_candidates_size; i++) {

        cout << (train_set->getDictionary(0))->getSymbol( itr_vec->wordtag ) << "\t";

        shared_candidates[i] = itr_vec->wordtag;
        l_tmp_shared_candidates.push_back(itr_vec->wordtag);
        itr_vec++;
    }

    tmp.clear();

    cout << endl;



    // * Add best candidates according to a bigram
    // wt means "word tag"
    // Note -> wt=vocabulary_size-1 corresponds to the (-1) tag in the NGramDistribution
    // we skip this tag, the 'missing' tag
    // NOTE Is this appropriate treatment?
    map<int, int> frequenciesCopy;
    map<int,int>::iterator itr;
    int n_candidates;

    for(int wt=-1; wt<vocabulary_size-1; wt++)  {

        // - fill list of candidates, then sort
        PP<SymbolNode> node = ((theNGram->tree)->getRoot())->child(wt);
        if(node)  {
            frequenciesCopy = node->getFrequencies();

            itr = frequenciesCopy.begin();
            while( itr != frequenciesCopy.end() ) {
                // -1 is the NGram's missing tag, our vocabulary_size-1 tag
                // Actually, we should not see it as a follower to anything except itself...
                if( itr->first != -1) {
                    tmp.push_back( wordAndFreq( itr->first, itr->second ) );
                } else  {
                    tmp.push_back( wordAndFreq( vocabulary_size-1, itr->second ) );
                }
                itr++;
            }
            std::sort(tmp.begin(), tmp.end(), wordAndFreqGT);

            // - resize candidates entry
            if( ngram_candidates_size < (int) tmp.size() )  {
                n_candidates = ngram_candidates_size; 
            } else  {
                n_candidates = tmp.size();
            }

            if(wt!=-1)  {
                candidates[wt].resize( n_candidates );
            } else  {
                candidates[ vocabulary_size-1 ].resize( n_candidates );
            }

            // - fill candidates entry

            itr_vec=tmp.begin();
            for(int i=0; i< n_candidates; i++) {
                //cout << (train_set->getDictionary(0))->getSymbol( itr_vec->wordtag ) << "\t";

                // ONLY ADD IF NOT IN THE SHARED CANDIDATES
                // Search the list.
                itr_tmp_shared_candidates = find( l_tmp_shared_candidates.begin(), l_tmp_shared_candidates.end(), itr_vec->wordtag);

                // if not found -> add it
                if (itr_tmp_shared_candidates == l_tmp_shared_candidates.end())
                {
if( itr_vec->wordtag > vocabulary_size -1 )
  cout << "NnlmOnlineLearner::buildCandidates() - problem " << itr_vec->wordtag <<endl;

                    if(wt!=-1)  {
                        candidates[wt][i] = itr_vec->wordtag;
                    } else  {
                        candidates[ vocabulary_size-1 ][i] = itr_vec->wordtag;
                    }
                // compensate for not adding this word
                } else  {
                    i--;
                    n_candidates--;
                }
                itr_vec++;
            }
            // compensate for not adding words
            if(wt!=-1)  {
                candidates[wt].resize( n_candidates );
            } else  {
                candidates[ vocabulary_size-1 ].resize( n_candidates );
            }

            tmp.clear();
        }
    }
    l_tmp_shared_candidates.clear();

}

////////////////////////////////
// evaluateGaussianCounts
////////////////////////////////
void NnlmOnlineLearner::evaluateGaussianCounts() const
{

    if( model_type != MODEL_TYPE_GAUSSIAN )  {
        PLWARNING( "NnlmOnlineLearner::evaluateGaussianCounts(): not a gaussian model. Ignoring call.\n");
        return;
    }

    Vec input( inputsize()-1 );
    Vec target( 1 );
    real weight;
    Vec output( outputsize() );   // the output of the semantic layer
    int nsamples = train_set->length();

    cout << "Evaluating gaussian counts..." << endl;

    PP<NnlmOutputLayer> p_nol;
    if( !(p_nol = dynamic_cast<NnlmOutputLayer*>( (OnlineLearningModule*) output_modules[0] ) ) )
    {
        PLERROR("NnlmOnlineLearner::evaluateGaussianCounts() - output_modules[0] is not an NnlmOutputLayer");
    }

    p_nol->resetClassCounts();

    // * Compute stats
    for( int sample=0 ; sample < nsamples ; sample++ )
    {
        myGetExample(train_set, sample, input, target, weight );

        p_nol->incrementClassCount( (int) target[0]);
    }

    // * Apply values 
    p_nol->applyClassCounts();

}

////////////////////////////////
// reevaluateGaussianParameters
////////////////////////////////
//! Reevaluates "fresh" gaussian mus and sigmas - make sure you want to do this
void NnlmOnlineLearner::reevaluateGaussianParameters() const
{
    cout << "Evaluating gaussian parameters..." << endl;

    if( model_type != MODEL_TYPE_GAUSSIAN )  {
        PLWARNING( "NnlmOnlineLearner::reevaluateGaussianParameters(): not a gaussian model. Ignoring call.\n");
        return;
    }

    Vec input( inputsize()-1 );
    Vec target( 1 );
    real weight;
    Vec output( outputsize() );   // the output of the semantic layer
    int nsamples = train_set->length();

    PP<NnlmOutputLayer> p_nol;
    if( !(p_nol = dynamic_cast<NnlmOutputLayer*>( (OnlineLearningModule*) output_modules[0] ) ) )
    {
        PLERROR("NnlmOnlineLearner::reevaluateGaussianParameters() - output_modules[0] is not an NnlmOutputLayer");
    }

    p_nol->resetTestVars();

    // * Compute stats
    for( int sample=0 ; sample < nsamples ; sample++ )
    {
        myGetExample(train_set, sample, input, target, weight );

        // * fprop
        computeOutput(input, output);

        p_nol->setTarget( (int) target[0]);
        //p_nol->setContext( (int) input[ (inputsize()-2) ] );

        p_nol->updateTestVars(output);
    }

    // * Apply values 
    // the missing tag does NOT get an output (never is the target)
    for(int wt=0; wt<vocabulary_size-1; wt++)  {
        p_nol->setTarget( wt );
        //p_nol->setContext( (int) input[ (inputsize()-2) ] );
        p_nol->applyTestVars();
    }

}


////////////////////////////////
// makeDeepCopyFromShallowCopy
////////////////////////////////
void NnlmOnlineLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(modules, copies);
    deepCopyField(values, copies);
    deepCopyField(gradients, copies);

    deepCopyField(output_modules, copies);
    deepCopyField(output_values, copies);
    deepCopyField(output_gradients, copies);

    // ### How about these?
    //ngram_train_set
    //theNGram
    //shared_candidates
    //candidates

}


///////////////
// outputsize
///////////////
//! This is the output size of the layer before the cost layer
int NnlmOnlineLearner::outputsize() const
{
    if( nmodules < 0 || values.length() <= nmodules )
        return -1;
    else
        return values[ nmodules ].length();
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

///////////
// forget
///////////
void NnlmOnlineLearner::forget()
{
    inherited::forget();

    // reset inputs
    values[0].clear();
    gradients[0].clear();
    // reset modules and outputs
    for( int i=0 ; i<nmodules ; i++ )
    {
        modules[i]->forget();
        values[i+1].clear();
        gradients[i+1].clear();
    }

    if( model_type == MODEL_TYPE_SOFTMAX )  {
        output_values[0].clear();
        output_gradients[0].clear();
    }
    for( int i=0 ; i<output_nmodules; i++ )
    {
        output_modules[i]->forget();
    }

    stage = 0;
}


/////////////////
// myGetExample
/////////////////

// Had trouble interfacing with ProcessSymbolicSequenceVMatrix's getExample.
// In particular, a source matrix of inputsize 1, targetsize 0, weightsize 0
// used in a ProcessSymb of leftcontext 3 would return an input of 4 and a
// target of size 0, even though its inputsize was set to 3 and targetsize to 1
//
void NnlmOnlineLearner::myGetExample(const VMat& example_set, int& sample, Vec& input, Vec& target,
real& weight) const
{
    static Vec row;
    // the actual inputsize is (inputsize()-1) and targetsize() is 1
    row.resize( inputsize() + weightsize() );

    example_set->getRow( sample, row);

    input << row.subVec( 0, inputsize()-1 );
    target << row.subVec( inputsize()-1, 1 );
    weight = 1.0;
    if( weightsize() )  {
        weight = row[ inputsize() ];
    }

    // *** SHOULD BE DONE IN PRETREATMENT!!! -> but we have a ProcessSymbolicSequenceVMatrix...
    // * Replace nan in input by '(train_set->getDictionary(0))->size()+1', 
    // the missing value tag
    for( int i=0 ; i < inputsize()-1 ; i++ ) {
      if( is_missing(input[i]) )  {
        input[i] = vocabulary_size - 1;
      }
    }
    // * Replace a 'nan' in the target by OOV
    // this nan should not be missing data (seeing the train_set is a
    // ProcessSymbolicSequenceVMatrix)
    // but the word "nan", ie "Mrs Nan said she would blabla"
    // *** Problem however - current vocabulary is full for train_set,
    // ie we train OOV on nan-word instances.
    // DO a pretreatment to replace Nan by *Nan* or something like it
    if( is_missing(target[0]) ) {
        target[0] = 0;
    }
    // *** SHOULD BE DONE IN PRETREATMENT!!!

    // set target for the nllerrmodule
    if( model_type == MODEL_TYPE_SOFTMAX )  {
        output_values[0][vocabulary_size-1]=target[0];
    }

}


//////////
// train
//////////
void NnlmOnlineLearner::train()
{
    if (!initTrain())
        return;

    Vec input( inputsize()-1 );
    Vec target( 1 );
    real weight;
    Vec output( outputsize() );   // the output of the semantic layer
    Vec train_costs( getTrainCostNames().length() );
    Vec out_gradient(1,1); // the gradient wrt the cost is '1'
    Vec gradient( semantic_layer_size );
    int nsamples = train_set->length();

    // Initialize mus and sigmas using 1 pass
    reevaluateGaussianParameters();

//---------------
    PP<GradNNetLayerModule> p_gnn;
    if( !(p_gnn = dynamic_cast<GradNNetLayerModule*>( (OnlineLearningModule*) modules[1] ) ) )
    {
        PLERROR("NnlmOnlineLearner::train - modules[1] is not a GradNNetLayerModule");
    }
    p_gnn->printVariance();
//---------------

    ProgressBar* pb = NULL;
    if(report_progress) {
        pb = new ProgressBar("Training", nsamples);
    }

    // *** For stages ***
    for( ; stage < nstages ; stage++ )
    {

        if(report_progress) {
            cout << "*** Stage " << stage << " ***" << endl;
            //cout << "uniform_mixture_coeff " << output_modules[0]->umc << " " << 1 - output_modules[0]->umc<< endl;
        }

        // * clear stats of previous epoch *
        train_stats->forget();

        // * for examples *
        for( int sample=0 ; sample < nsamples ; sample++ )
        {

            if(report_progress)
                pb->update(sample);

            // - Get example -
            myGetExample(train_set, sample, input, target, weight );

            // - Fixed part fprop -
            computeOutput(input, output);

            // - Variable part fprop - cost and gradient for this part -
            // (we don't want to duplicate some computations in gaussian model gradient evaluation)
            // In gaussian case, gradients[nmodules] is computed here.
            computeTrainCostsFromOutputs(input, output, target, train_costs );

            // - bpropUpdate -

            // Variable part
            if( model_type == MODEL_TYPE_GAUSSIAN )  {

                PP<NnlmOutputLayer> p_nol;
                if( !(p_nol = dynamic_cast<NnlmOutputLayer*>( (OnlineLearningModule*) output_modules[0] ) ) )
                {
                    PLERROR("NnlmOnlineLearner::train() - MODEL_TYPE_GAUSSIAN but output_modules[0] is not an NnlmOutputLayer");
                }

                if( gaussian_model_cost == GAUSSIAN_COST_APPROX_DISCR )  {
                    output_modules[0]->bpropUpdate( output, train_costs.subVec(1,1), out_gradient );
                    gradients[nmodules] << p_nol->ad_gradient;
                } else {  //if( gaussian_model_cost == GAUSSIAN_COST_NON_DISCR )
                    output_modules[0]->bpropUpdate( output, train_costs.subVec(0,1), out_gradient );
                    gradients[nmodules] << p_nol->nd_gradient;
                }

            } else  {
                output_modules[1]->bpropUpdate( output_values[0], train_costs, output_gradients[0], out_gradient );
                output_modules[0]->bpropUpdate( output, output_values[0].subVec( 0, vocabulary_size-1 ), gradients[nmodules], output_gradients[0] );
            }

            // Fixed (common to both models) part
            for( int i=nmodules-1 ; i>0 ; i-- ) {
                modules[i]->bpropUpdate( values[i], values[i+1], gradients[i], gradients[i+1] );
            }
            modules[0]->bpropUpdate( values[0], values[1], gradients[1] );


            // - Update stats -
            train_stats->update( train_costs );

        }// * for examples - END

        train_stats->finalize(); // finalize statistics for this epoch

    }// *** For stages - END


    if(pb)
        delete pb;


}


/////////
// test
/////////
void NnlmOnlineLearner::test(VMat testset, PP<VecStatsCollector> test_stats,
                    VMat testoutputs, VMat testcosts) const
{

    Vec input( inputsize()-1 );
    Vec output( outputsize() );
    Vec target( 1 );
    real weight;
    Vec test_costs( getTestCostNames().length() );
    real entropy = 0.0;
    real perplexity = 0.0;
    int nsamples = testset->length();


    // * Empty test set: we give -1 cost arbitrarily.
    if (nsamples == 0) {
        test_costs.fill(-1);
        test_stats->update(test_costs);
    }

    if( stage == 0 )  {
        // Initialize mus and sigmas using 1 pass
        reevaluateGaussianParameters();
    }


    // * TODO Should we do this?
    //reevaluateGaussianParameters();

    ProgressBar* pb = NULL;
    if(report_progress)
        pb = new ProgressBar("Testing learner",nsamples);

    for( int sample=0 ; sample < nsamples ; sample++ )
    {
        myGetExample(testset, sample, input, target, weight );

        // Always call computeOutputAndCosts, since this is better
        // behaved with stateful learners
        computeOutputAndCosts(input, target, output, test_costs);

        if(testoutputs)
            testoutputs->putOrAppendRow(sample,output);

        if(testcosts)
            testcosts->putOrAppendRow(sample, test_costs);

        if(test_stats)
            test_stats->update(test_costs,weight);

        if(report_progress)
            pb->update(sample);

        entropy += test_costs[0];

        // Do some outputing
        // Do some outputing
        if( sample < 50 ) {
            cout << "---> ";
            for( int i=0; i<inputsize()-1; i++)  {
                if( (int)input[i] == vocabulary_size - 1) {
                    cout << "\\missing\\ ";
                } else  {
                    cout << (testset->getDictionary(0))->getSymbol( (int)input[i] ) << " ";
                }
            }
            cout << "\t\t " << (testset->getDictionary(0))->getSymbol( (int)target[0] ) << " p(t|r) " << safeexp( - test_costs[0] ) << endl;

            if( model_type == MODEL_TYPE_GAUSSIAN )  {
                PP<NnlmOutputLayer> p_nol;
                if( !(p_nol = dynamic_cast<NnlmOutputLayer*>( (OnlineLearningModule*) output_modules[0] ) ) )
                {
                    PLERROR("NnlmOnlineLearner::test - MODEL_TYPE_GAUSSIAN but output_modules[0] is not an NnlmOutputLayer");
                }
                Vec candidates, probabilities;
                p_nol->getBestCandidates(output, candidates, probabilities);
                for(int i=0; i<candidates.size(); i++)  {
                    cout << "\t" << (testset->getDictionary(0))->getSymbol( (int)candidates[i] ) << " " << probabilities[i] << endl;
                }
            }
        }
        // Do some outputing - END
        // Do some outputing - END

    }

    entropy /= nsamples;
    perplexity = safeexp(entropy);

    cout << "entropy: " << entropy << " perplexity " << perplexity << endl;

    if(pb)
        delete pb;

}


//////////////////
// computeOutput
//////////////////
void NnlmOnlineLearner::computeOutput(const Vec& input, Vec& output) const
{
//cout << "************************************" << endl;

    // fprop
    values[0] << input;
    for( int i=0 ; i<nmodules ; i++ ) {
        modules[i]->fprop( values[i], values[i+1] );

//cout << "-= " << i << " =-" << endl;
//cout << values[i] << endl;        
    }
//cout << "-= " << nmodules << " =-" << endl;
//cout <<values[ nmodules ] << endl;

    // 
    output.resize( outputsize() );
    output << values[ nmodules ];

}


/////////////////////////////////////////////
// computeTrainCostsAndGradientsFromOutputs
/////////////////////////////////////////////
// Compute costs. In gaussian case, compute gradient wrt ouput.
void NnlmOnlineLearner::computeTrainCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{

    if( model_type == MODEL_TYPE_GAUSSIAN )  {

        PP<NnlmOutputLayer> p_nol;
        if( !(p_nol = dynamic_cast<NnlmOutputLayer*>( (OnlineLearningModule*) output_modules[0] ) ) )
        {
            PLERROR("NnlmOnlineLearner::computeTrainCostsAndGradientsFromOutputs - MODEL_TYPE_GAUSSIAN but output_modules[0] is not an NnlmOutputLayer");
        }

        p_nol->setTarget( (int) target[0] );
        p_nol->setContext( (int) input[ (int) (inputsize()-2) ] );

        p_nol->fprop( output, costs );

    } else  {
        Vec example_cost(1);
        // don't give the target to the gradnnetlayermodule
        Vec bob(vocabulary_size-1);
/*
    Vec out_tgt = output.copy();
    out_tgt.append( target );
    for( int i=0 ; i<ncosts ; i++ )
    {
        Vec cost(1);
        cost_modules[i]->fprop( out_tgt, cost );
        costs[i] = cost[0];
    }

*/
        //output_modules[0]->fprop( output, output_values[0].subVec( 0, vocabulary_size-1 ) );

        // output_values[0][vocabulary_size-1] contains the target index myGetExample
        output_modules[0]->fprop( output, bob );
        output_values[0].subVec( 0, vocabulary_size-1 ) << bob;
        output_modules[1]->fprop( output_values[0], example_cost);

        costs[0] = example_cost[0];
    }


}


//! Computes the test costs, ie the full disriminant cost (NLL).
//! See about how to include/print the perplexity.
// Compute the costs from *already* computed output.
// TODO should not iterate over the vocabulary. Properly set output'state and call fprop.
void NnlmOnlineLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    if( model_type == MODEL_TYPE_GAUSSIAN )  {

        PP<NnlmOutputLayer> p_nol;
        if( !(p_nol = dynamic_cast<NnlmOutputLayer*>( (OnlineLearningModule*) output_modules[0] ) ) )
        {
            PLERROR("NnlmOnlineLearner::computeCostsFromOutputs - MODEL_TYPE_GAUSSIAN but output_modules[0] is not an NnlmOutputLayer");
        }

        p_nol->setCost(GAUSSIAN_COST_DISCR);
        p_nol->setTarget( (int)target[0] );
        p_nol->fprop( output, costs);

        // Re-Set Cost 
        if( gaussian_model_cost == GAUSSIAN_COST_APPROX_DISCR ) {
            p_nol->setCost(GAUSSIAN_COST_APPROX_DISCR);
        } else  { //GAUSSIAN_COST_NON_DISCR
            p_nol->setCost(GAUSSIAN_COST_NON_DISCR);
        }

    } else  {
        Vec example_cost(1);

        Vec bob(vocabulary_size-1);

        output_modules[0]->fprop( output, bob );
        output_values[0].subVec( 0, vocabulary_size-1 ) << bob;
        // output_values[0][vocabulary_size-1] contains the target index from myGetExample
        output_modules[1]->fprop( output_values[0], example_cost);

        costs[0] = example_cost[0];
    }

}


/////////////////////
// getTestCostNames
/////////////////////
TVec<string> NnlmOnlineLearner::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).
    TVec<string> ret;
    ret.resize(1);
    ret[0] = "NLL";
    return ret;
}


//////////////////////
// getTrainCostNames
//////////////////////
TVec<string> NnlmOnlineLearner::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes
    // and for which it updates the VecStatsCollector train_stats
    // (these may or may not be exactly the same as what's returned by
    // getTestCostNames).
    TVec<string> ret;

    if( model_type == MODEL_TYPE_GAUSSIAN )  {
        ret.resize(2);
        ret[0] = "non_discriminant";
        ret[1] = "approx_discriminant";
    } else  {
        ret.resize(1);
        ret[0] = "NLL";
    }

    return ret;
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
