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
#include <plearn_learners/online/GradNNetLayerModule.h> // only diff is in the prandom seed -> 1
#include <plearn_learners/online/TanhModule.h>
#include <plearn_learners_experimental/onlineNNLM/NnlmOutputLayer.h>

#include <plearn_learners/distributions/NGramDistribution.h>
#include <plearn_learners/distributions/SymbolNode.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NnlmOnlineLearner,
    "Trains a Naive Bayes Neural Network Language Model (NBNNLM).",
    "MULTI-LINE \nHELP");



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



NnlmOnlineLearner::NnlmOnlineLearner()
    :   PLearner(),
        vocabulary_size( -1 ),
        word_representation_size( -1 ),
        context_size( -1 ),
        context_layer_size( 200 ),
        shared_candidates_size( 0 ),
        ngram_candidates_size( 50 ),
        self_candidates_size( 0 )
{
    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)

    // ### If this learner needs to generate random numbers, uncomment the
    // ### line below to enable the use of the inherited PRandom object.
    random_gen = new PRandom();
}

void NnlmOnlineLearner::declareOptions(OptionList& ol)
{
// NOT an option... computed from the train_set's dictionary
/*
    declareOption(ol, "vocabulary_size",
                  &NnlmOnlineLearner::vocabulary_size,
                  OptionBase::buildoption,
                  "size of vocabulary used - defines the virtual input size");
*/
    declareOption(ol, "word_representation_size",
                  &NnlmOnlineLearner::word_representation_size,
                  OptionBase::buildoption,
                  "size of the real distributed word representation");

// NOT an option... it's the train_set's input size
/*    declareOption(ol, "context_size",
                  &NnlmOnlineLearner::context_size,
                  OptionBase::buildoption,
                  "size of word context");
*/

    declareOption(ol, "shared_candidates_size",
                  &NnlmOnlineLearner::shared_candidates_size,
                  OptionBase::buildoption,
                  "Number of candidates in aproximated discriminant cost evaluation drawn from frequent words.");

    declareOption(ol, "ngram_candidates_size",
                  &NnlmOnlineLearner::ngram_candidates_size,
                  OptionBase::buildoption,
                  "Number of candidates in aproximated discriminant cost evaluation drawn from the context (bigram).");

    declareOption(ol, "self_candidates_size",
                  &NnlmOnlineLearner::self_candidates_size,
                  OptionBase::buildoption,
                  "Number of candidates in aproximated discriminant cost evaluation drawn from self candidates (evaluated periodically). NOT IMPLEMENTED!!");


    declareOption(ol, "ngram_train_set",
                  &NnlmOnlineLearner::ngram_train_set,
                  OptionBase::buildoption,
                  "train set used for training the ngram used in the evaluation of the sets of words used for normalization in the evaluated discriminant cost. This ProcessSymbolicSequenceVMatrix defines the ngram used (if inputsize is 3 then we have a trigram).");

    

    declareOption(ol, "modules", &NnlmOnlineLearner::modules,
                  OptionBase::buildoption,
                  "Layers of the learner");

    declareOption(ol, "output_module", &NnlmOnlineLearner::output_module,
                  OptionBase::buildoption,
                  "Output layer");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void NnlmOnlineLearner::buildLayers()
{

    // NnlmWordRep + GradNNet + Tanh
    nmodules = 3;
    modules.resize( nmodules );

    // *** Word representation layer
    PP< NnlmWordRepresentationLayer > p_wrl = new NnlmWordRepresentationLayer();

    // This is done in the NnlmWordRepresentationLayer  
    // BUT in the build... so we need to do it here, because size compality check
    // is performed before the build
    p_wrl->input_size = inputsize()-1;
    p_wrl->output_size = context_size * word_representation_size;
    p_wrl->vocabulary_size = vocabulary_size;
    p_wrl->word_representation_size = word_representation_size;
    p_wrl->context_size = context_size;
    p_wrl->random_gen = random_gen;
    p_wrl->start_learning_rate = 10^3;

    modules[0] = p_wrl;


    // *** GradNNetLayer

    PP< GradNNetLayerModule > p_nnl = new GradNNetLayerModule();

    p_nnl->input_size = context_size * word_representation_size;
    // HARDCODED FOR NOW
    p_nnl->output_size = context_layer_size;
    p_nnl->start_learning_rate = 10^3;
    p_nnl->init_weights_random_scale=1;
    p_nnl->random_gen = random_gen;

    modules[1] = p_nnl;


    // *** Tanh layer

    PP< TanhModule > p_thm = new TanhModule();
    // HARDCODED FOR NOW
    p_thm->input_size = context_layer_size;
    p_thm->output_size = context_layer_size;

    modules[2] = p_thm;


    // ***  Check on layer size compatibilities and resize values and gradients
    // variables

    values.resize( nmodules+1 );
    gradients.resize( nmodules+1 );

    // first values will be "input" values
    int size = inputsize()-1;
    values[0].resize( size );
    gradients[0].resize( size );

    for( int i=0 ; i<nmodules ; i++ )
    {
        PP<OnlineLearningModule> p_module = modules[i];

        if( p_module->input_size != size )
        {
            PLWARNING( "StackedModulesLearner::buildLayers(): module '%d'\n"
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


    // ***  Cost module
    output_module = new NnlmOutputLayer();

    output_module->vocabulary_size= vocabulary_size;
    output_module->word_representation_size = word_representation_size;
    output_module->context_size = context_size;
    // HARDCODED FOR NOW 
    output_module->input_size = context_layer_size;
    output_module->output_size = 1;

    output_module->build();

}

void NnlmOnlineLearner::buildCandidates()
{

    // *** Train ngram
    // Should load it instead
    cout << "training ngram" << endl;

    theNGram = new NGramDistribution();

    theNGram->n = ngram_train_set->inputsize();
    theNGram->smoothing = "no_smoothing";
    theNGram->nan_replace = true;
    theNGram->setTrainingSet( ngram_train_set );
    //theNGram->build(); Done in setTrainingSet

    theNGram->train();


    // *** Effective building
    cout << "building candidates" << endl;

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

    cout << " These are the shared candidates" << endl;
    cout << "---------------------------------" << endl;

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

    cout << "---------------------------------" << endl;
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
void NnlmOnlineLearner::build_()
{

    if( train_set )
    {

        // *** Pretreat train_set
        // Non functional code - problem is with setting a value
        // Does ProcessSymbolicVMatrix have a proper put function
/*        int dict_size = (train_set->getDictionary(0))->size();

        for( int i = 0; i < train_set->length(); i++ )  {

            // * Replace nan input by missing input tag
            for( int j = 0; j < inputsize()-1; j++ )  {
              if( is_missing( train_set(i,j) ) )  {
                // DOES NOT COMPILE !!!
                //train_set(i,j) = dict_size + 1;
                // PUT OVERLOADED???
                //train_set->put(i,j, dict_size + 1 );
              }
            }

            // * Replace nan target by OOV
            // this nan should not be missing data (seeing the train_set is a
            // ProcessSymbolicSequenceVMatrix)
            // but the word "nan", ie "Mrs Nan said she would blabla"
            // *** Problem however - current vocabulary is full for train_set,
            // ie we train OOV on nan-word instances.
            // DO a pretreatment to replace Nan by *Nan* or something like it
            if( is_missing( train_set(i, inputsize()-1) ) ) {
              //SAME!!!
              train_set( i, inputsize()-1 ) = 0;
            }

        }
*/

        // *** Sanity Checks
        // on context_size, word_representation_size


        // *** Set some variables

        // vocabulary size (voc +1 for OOV ( 0 tag ) +1 for missing ( dict_size+1 tag) )
        vocabulary_size = (train_set->getDictionary(0))->size()+2;

        cout << "vocabulary_size = " << vocabulary_size << endl;

        context_size = inputsize()-1;

        // *** Build modules and output_module
        buildLayers();


        // *** Build candidates
        buildCandidates();


    }

}


void NnlmOnlineLearner::build()
{
    inherited::build();
    build_();
}


void NnlmOnlineLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(modules, copies);
    deepCopyField(output_module, copies);

    deepCopyField(values, copies);
    deepCopyField(gradients, copies);

    // ### Remove this line when you have fully implemented this method.
    //PLERROR("NnlmOnlineLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

//! This is the output size of the layer before the cost layer
int NnlmOnlineLearner::outputsize() const
{
    // Compute and return the size of this learner's output (which typically
    // may depend on its inputsize(), targetsize() and set options).

    if( nmodules < 0 || values.length() <= nmodules )
        return -1;
    else
        return values[ nmodules ].length();
}

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

    output_module->forget();

    stage = 0;

}

// Had trouble interfacing with ProcessSymbolicSequenceVMatrix's getExample.
// In particular, a source matrix of inputsize 1, targetsize 0, weightsize 0
// used in a ProcessSymb of leftcontext 3 would return an input of 4 and a
// target of size 0, even though its inputsize was set to 3 and targetsize to 1
//
void NnlmOnlineLearner::myGetExample(VMat& example_set, int& sample, Vec& input, Vec& target,
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

}

// TODO I've misunderstood how the costs work (don't update for each example! Compute a cost for the stage!). Fix this.
// Don't train the neural part until stage 2. This way, we get some valid mus, sigmas, etc for the output layer.
void NnlmOnlineLearner::train()
{

    if (!initTrain())
        return;

    // *** Variables ***

    Vec input( inputsize()-1 );   // the word context
    Vec target( 1 );              // the word to predict
    real weight;

    int nsamples = train_set->length();
    Vec output( outputsize() );   // the output of the semantic layer
    Vec train_costs( getTrainCostNames().length() );

    // * intermediate variables
    Vec neglogprob_tr(1);   // - log p(word=t(arget), representation=r)

    Vec non_discr_gradient( context_layer_size );
    Vec approx_discr_gradient( context_layer_size );
    Vec out_gradient(1,1); // the gradient wrt the cost is '1'

    // * pb
    ProgressBar* pb = NULL;
    if(report_progress) {
        pb = new ProgressBar("Training", nsamples);
    }


    // *** For stages
    for( ; stage < nstages ; stage++ )
    {
cout << "#################################################################" << endl;
cout << "#################################################################" << endl;
cout << "#################################################################" << endl;
cout << "#################################################################" << endl;

        if(report_progress) {
            cout << endl << "stage " << stage << endl;
            cout << "uniform_mixture_coeff " << output_module->umc << endl;
        }
        // * clear stats of previous epoch
        train_costs.fill(0);
        train_stats->forget();

        // * for examples
        for( int sample=0 ; sample < nsamples ; sample++ )
        {

            if(report_progress)
                pb->update(sample);

            // *** Get example
            myGetExample(train_set, sample, input, target, weight );


            // *** Compute training costs and gradients

            // * fprop
            computeOutput(input, output);

            // * Non-discriminant cost and gradient
            output_module->setCurrentWord( (int) target[0]);
            output_module->fprop( output, neglogprob_tr);

            train_costs[0] += neglogprob_tr[0];

            // so we don't do a /0 with  /sigma2(current_word, i)
            //if( output_module->sumI[ (int) target[0] ] < 2 )  {
            if( stage < 2 )  {
              non_discr_gradient.fill(0.0);
            } else  {
                for(int i=0; i<context_layer_size; i++) {
                    non_discr_gradient[i] = ( output[i] - output_module->mu( (int) target[0], i) ) 
                                      / output_module->sigma2( (int) target[0], i);

                    // modification for mixture with uniform * p(r,g|i) / p(r|i)
                    non_discr_gradient[i] = non_discr_gradient[i] * safeexp( output_module->log_p_rg_i - output_module->log_p_r_i );

                }
            }

            // * Approximated discriminant cost and gradient -> using candidate words
            if( stage < 2 )  {
                train_costs[1] = 0.0;
                approx_discr_gradient.fill(0.0);
            } else  {
                computeApproximateDiscriminantCostAndGradient(input, target, output, neglogprob_tr[0], train_costs, 
                                       non_discr_gradient, approx_discr_gradient);
            }

            // *** Update stats
            train_stats->update( train_costs );

            // *** Select which gradient
            //gradients[ nmodules ] << non_discr_gradient;
            gradients[ nmodules ] << approx_discr_gradient;

            if( sample < 100 ) {
                cout << "mu[ (int) target[0] ]" << endl;
                cout << output_module->mu( (int) target[0] ) << endl;
cout << "-----------------------------------------------------" << endl;
                cout << "sigma2[ (int) target[0] ]" << endl;
                cout << output_module->sigma2( (int) target[0] ) << endl;
cout << "-----------------------------------------------------" << endl;

                cout << "non_discr_gradient" << endl;
                cout << non_discr_gradient << endl;
cout << "-----------------------------------------------------" << endl;
                cout << "approx_discr_gradient" << endl;
                cout << approx_discr_gradient << endl;
cout << "*****************************************************" << endl;
            }


            // *** Perform update -> bpropUpdate
            
            output_module->bpropUpdate( output,
                                          train_costs.subVec(0,1),
                                          out_gradient );

            for( int i=nmodules-1 ; i>0 ; i-- )
                modules[i]->bpropUpdate( values[i], values[i+1],
                                         gradients[i], gradients[i+1] );

            modules[0]->bpropUpdate( values[0], values[1], gradients[1] );

        }// * for examples - END

        train_stats->finalize(); // finalize statistics for this epoch
        pout << "train_stats " << train_stats << endl;

    }// *** For stages - END


    if(pb)
        delete pb;


}

//! Computes the approximate discriminant cost and its gradient
void NnlmOnlineLearner::computeApproximateDiscriminantCostAndGradient(Vec input, Vec target, Vec output, real nd_cost, Vec train_costs, Vec nd_gradient, Vec ad_gradient)
{

    //static int step = 0;


    // *** 
    // so we don't do a /0 with  /sigma2(current_word, i)
    if( output_module->sumI[ (int) target[0] ] < 2 )  {
        train_costs[1] = 0.0;
        ad_gradient.fill(0.0);
        return;
    }


    // *** We can compute cost and gradient
    int c;
    Vec neglogprob_cr(1);         // - log p(word=c, representation=r)
    real log_sumprob = -nd_cost;   // log \sum_c p(c,r), where r the output (context representation) and c the candidates
    real alpha;
    Vec gradient_tmp( context_layer_size ); // used in the computation of the approx. discriminant gradient
    Vec gradient_tmp_pos( context_layer_size ); // used in the computation of the approx. discriminant gradient
    Vec gradient_tmp_neg( context_layer_size ); // used in the computation of the approx. discriminant gradient
    gradient_tmp.fill(-REAL_MAX);
    gradient_tmp_pos.fill(-REAL_MAX);
    gradient_tmp_neg.fill(-REAL_MAX);


    for(int i=0; i<context_layer_size; i++) {
        alpha = output[i] - output_module->mu( (int)target[0] , i);

        if( alpha > 0)  {
            //gradient_tmp_pos[i] = -nd_cost + safelog( alpha ) - safelog( output_module->sigma2( (int)target[0], i) );
            gradient_tmp_pos[i] = output_module->log_p_rg_i + safelog( alpha ) - safelog( output_module->sigma2( (int)target[0], i) );
        } else  {
            //gradient_tmp_neg[i] = -nd_cost + safelog( -alpha ) - safelog( output_module->sigma2( (int)target[0], i) );
            gradient_tmp_neg[i] = output_module->log_p_rg_i + safelog( -alpha ) - safelog( output_module->sigma2( (int)target[0], i) );
        }

    }

    // *** Compute normalization

    for( int i=0; i< shared_candidates.length(); i++ )
    {
        c = shared_candidates[i];

        if( c != (int) target[0] )  {
            output_module->setCurrentWord( c );
            output_module->fprop( output, neglogprob_cr );

            log_sumprob = logadd(log_sumprob, -neglogprob_cr[0]);

            // so we don't do a /0 with  /sigma2(current_word, i)
            if( output_module->sumI[ c ] >= 2 )  {
                for(int j=0; j<context_layer_size; j++) {
                    alpha = output[j] - output_module->mu( c, j);

                    if( alpha > 0)  {
                        //gradient_tmp_pos[j] = logadd( gradient_tmp_pos[j], 
                        //        -neglogprob_cr[0] + safelog( alpha ) - safelog( output_module->sigma2( c, j) ) );
                        gradient_tmp_pos[j] = logadd( gradient_tmp_pos[j], 
                                output_module->log_p_rg_i + safelog( alpha ) - safelog( output_module->sigma2( c, j) ) );
                    } else  {
                        //gradient_tmp_neg[j] = logadd( gradient_tmp_neg[j], 
                        //        -neglogprob_cr[0] + safelog( -alpha ) - safelog( output_module->sigma2( c, j) ) );
                        gradient_tmp_neg[j] = logadd( gradient_tmp_neg[j], 
                                output_module->log_p_rg_i + safelog( -alpha ) - safelog( output_module->sigma2( c, j) ) );
                    }

                }
            }

        } // if not target - END
    }


    // TODO CONSIDER SHARED CANDIDATES ALSO!!!
    for( int i=0; i< candidates[ (int) input[ (int) (inputsize()-2) ] ].length(); i++ )
    {
        c = candidates[ (int) input[ (int) (inputsize()-2) ] ][i];

        if( c != (int) target[0] )  {
            output_module->setCurrentWord( c );
            output_module->fprop( output, neglogprob_cr );


            log_sumprob = logadd(log_sumprob, -neglogprob_cr[0]);

            // so we don't do a /0 with  /sigma2(current_word, i)
            if( output_module->sumI[ c ] >= 2 )  {
                for(int j=0; j<context_layer_size; j++) {
                    alpha = output[j] - output_module->mu( c, j);

                    if( alpha > 0)  {
                        //gradient_tmp_pos[j] = logadd( gradient_tmp_pos[j], 
                        //        -neglogprob_cr[0] + safelog( alpha ) - safelog( output_module->sigma2( c, j) ) );
                        gradient_tmp_pos[j] = logadd( gradient_tmp_pos[j], 
                                output_module->log_p_rg_i + safelog( alpha ) - safelog( output_module->sigma2( c, j) ) );

                    } else  {
                        //gradient_tmp_neg[j] = logadd( gradient_tmp_neg[j], 
                        //        -neglogprob_cr[0] + safelog( -alpha ) - safelog( output_module->sigma2( c, j) ) );
                        gradient_tmp_neg[j] = logadd( gradient_tmp_neg[j], 
                                output_module->log_p_rg_i + safelog( -alpha ) - safelog( output_module->sigma2( c, j) ) );

                    }
                }
            }

        } // if not target - END
    }



    // *** The approximate discriminant cost
    train_costs[1] = nd_cost + log_sumprob;


    // *** The corresponding gradient
    for(int j=0; j<context_layer_size; j++) {
        if( gradient_tmp_pos[j] > gradient_tmp_neg[j] ) {
            gradient_tmp[j] = logsub( gradient_tmp_pos[j], gradient_tmp_neg[j] );
            ad_gradient[j] = nd_gradient[j] - safeexp( gradient_tmp[j] - log_sumprob);
        } else  {
            gradient_tmp[j] = logsub( gradient_tmp_neg[j], gradient_tmp_pos[j] );
            ad_gradient[j] = nd_gradient[j] + safeexp( gradient_tmp[j] - log_sumprob);
        }

    }

/*    if( step < 15)  {
        cout << "gradient_tmp_pos " << endl << gradient_tmp_pos << endl << endl; 
        cout << "gradient_tmp_neg " << endl << gradient_tmp_neg << endl << endl;
    }*/

/*    step++;
    step = step%100000;*/
}



void NnlmOnlineLearner::test(VMat testset, PP<VecStatsCollector> test_stats,
                    VMat testoutputs, VMat testcosts) const
{

    Vec input( inputsize()-1 );
    Vec output( outputsize() );
    Vec target( 1 );
    real weight;

    Vec test_costs( getTestCostNames().length() );
    int nsamples = testset->length();
    real entropy = 0.0;
    real perplexity = 0.0;

    ProgressBar* pb = NULL;
    if(report_progress)
        pb = new ProgressBar("Testing learner",nsamples);

    // * Empty test set: we give -1 cost arbitrarily.
    if (nsamples == 0) {
        test_costs.fill(-1);
        test_stats->update(test_costs);
    }

    for( int sample=0 ; sample < nsamples ; sample++ )
    {
        myGetExample(testset, sample, input, target, weight );


  /*      // * fprop
        computeOutput(input, output);

        // * Set the output_module's "state"
        // Cost is dependent on the word we want to predict
        // If we want an evaluated cost, we probably have to set the context also
        //output_module->setCurrentWord( (int) target[0] );
        //output_module->setContext

        // * Compute cost
        output_module->computeDiscriminantCost(input, output, target, test_costs );

        if(sample<15)
          cout << "test cost for sample " << sample << " " << test_costs[0] << endl;
*/

        // Always call computeOutputAndCosts, since this is better
        // behaved with stateful learners
        computeOutputAndCosts(input,target,output,test_costs);


        if(testoutputs)
            testoutputs->putOrAppendRow(sample,output);

        if(testcosts)
            testcosts->putOrAppendRow(sample, test_costs);

        if(test_stats)
            test_stats->update(test_costs,weight);

        if(report_progress)
            pb->update(sample);

        entropy += test_costs[0];

        if( sample < 100 ) {
            cout << sample << " p(i|r) " << safeexp( - test_costs[0] ) << endl;
        }


    }


    entropy /= nsamples;
    perplexity = safeexp(entropy);

    cout << "entropy: " << entropy << " perplexity " << perplexity << endl;

    if(pb)
        delete pb;


    cout << "test -> context not set" << endl;

}



void NnlmOnlineLearner::computeOutput(const Vec& input, Vec& output) const
{
    // Compute the output from the input.
    // int nout = outputsize();
    // output.resize(nout);
    // ...
    // Compute the output from the input.
    // int nout = outputsize();
    // output.resize(nout);

    values[0] << input;


    // fprop
    for( int i=0 ; i<nmodules ; i++ )
        modules[i]->fprop( values[i], values[i+1] );

    output.resize( outputsize() );
    output << values[ nmodules ];

}






//! Computes the test costs, ie the full disriminant cost (NLL).
//! See about how to include/print the perplexity.
// Compute the costs from *already* computed output.
void NnlmOnlineLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{

    Vec nl_p_i;
    Vec nl_p_j;
    real l_sum;

    nl_p_i.resize( getTestCostNames().length() );
    nl_p_j.resize( getTestCostNames().length() );


    // * Compute numerator
    output_module->setCurrentWord( (int)target[0] );
    output_module->fprop( output, nl_p_i);

    // * Compute denominator
    // Normalize over whole vocabulary

    l_sum = 0.0;

    for(int w=0; w<vocabulary_size; w++)  {

        output_module->setCurrentWord( w );
        output_module->fprop( output, nl_p_j);

        // !!!!!!!!!!!!!!!!!!!!!!!1111
        // we are adding neg logs... any difference?
        //nl_sum = logadd(nl_sum, nl_p_j[0]);
        // Let's try this...
        l_sum = logadd(l_sum, -nl_p_j[0]);

    }

    costs[0] = nl_p_i[0] + l_sum;

    

}

TVec<string> NnlmOnlineLearner::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).
    // ...

    TVec<string> ret;
    ret.resize(1);
    ret[0] = "NLL";
    return ret;

}

TVec<string> NnlmOnlineLearner::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes
    // and for which it updates the VecStatsCollector train_stats
    // (these may or may not be exactly the same as what's returned by
    // getTestCostNames).

    TVec<string> ret;
    ret.resize(2);
    ret[0] = "nonDisc";
    ret[1] = "NLLeval";
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
