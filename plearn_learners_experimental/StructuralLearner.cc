// -*- C++ -*-

// StructuralLearner.cc
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

/* *******************************************************      
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Pierre-Antoine Manzagol

/*! \file StructuralLearner.cc */


#include "StructuralLearner.h"
#include <plearn/math/plapack.h>
#include <plearn/math/random.h>


namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    StructuralLearner,
    "ONE LINE DESCRIPTION",
    "MULTI-LINE \nHELP");

StructuralLearner::StructuralLearner() 
/* ### Initialize all fields to their default value here */
{

  std::cerr << "StructuralLearner::StructuralLearner()" << std::endl;

  // With these values, will not learn
  start_learning_rate=0.0;
  decrease_constant=0.0;
  lambda=0.0001;
  
  labeled_train_set_indices.resize(0);
  //m_tvec_auxiliaryLearners.resize(0);
  
  // ### You may (or not) want to call build_() to finish building the object
  // ### (doing so assumes the parent classes' build_() have been called too
  // ### in the parent classes' constructors, something that you must ensure)
}

void StructuralLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "ws", &StructuralLearner::ws, OptionBase::learntoption,
                   "Weights of the linear classifier: f(x) = wt x + vt theta x");
    declareOption(ol, "vs", &StructuralLearner::vs, OptionBase::learntoption,
                   "Weights of the linear classifier: f(x) = wt x + vt theta x");
    declareOption(ol, "thetas", &StructuralLearner::thetas, OptionBase::learntoption,
                   "structure parameter of the linear classifier: f(x) = wt x + vt theta x");
    declareOption(ol, "start_learning_rate", &StructuralLearner::start_learning_rate, OptionBase::buildoption,
                   "Starting learning rate of the stochastic gradient descent");
    declareOption(ol, "decrease_constant", &StructuralLearner::decrease_constant, OptionBase::buildoption,
                   "Decrease constant of the stochastic learning rate");
    declareOption(ol, "auxiliary_task_train_set", &StructuralLearner::auxiliary_task_train_set, OptionBase::buildoption,
                   "Training set for auxiliary task");
    declareOption(ol, "epsilon", &StructuralLearner::epsilon, OptionBase::buildoption,
                   "Threshold to determine convergence of stochastic descent");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

// For now everything is done in the train. For sure, that's not a good, for example if we want to
// reload ...
void StructuralLearner::build_()
{
  std::cerr << "StructuralLearner::build_()" << std::endl;

  if(train_set)
  {
    // ***** Sanity checks
    if(weightsize_ < 0)
      PLWARNING("In StructuralLearner::build_(): negative weightsize_");
    if(weightsize_ > 0)
      PLWARNING("In StructuralLearner::build_(): does not support weighting of the training set");
    if(targetsize_ < 0)
      PLWARNING("In StructuralLearner::build_(): negative targetsize_");
    if(targetsize_ > 1)
      PLWARNING("In StructuralLearner::build_(): multi-target learning is not supported, only one (hardcoded) target will be considered");

    // ***** Resize vectors
    input.resize(inputsize());
    target.resize(targetsize());
    costs.resize(getTrainCostNames().length());
    before_softmax.resize(outputsize());
    output.resize(outputsize());

    // TODO:
    // - resize and initialize ws, vs and thetas and thetas_times_x
    // - fill bag_of_words_over_chunks    
    // - create auxiliary task (if auxiliary_task_train_set != 0)
  }
}

// ### Nothing to add here, simply calls build_
void StructuralLearner::build()
{
    inherited::build();
    build_();
}


void StructuralLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(v, copies);
    deepCopyField(w, copies);
    deepCopyField(theta_t, copies);

    deepCopyField(input, copies);
    deepCopyField(target, copies);
    deepCopyField(before_softmax, copies);
    deepCopyField(output, copies);
    deepCopyField(costs, copies);

    deepCopyField(labeled_train_set_indices, copies);
    deepCopyField(labeled_train_set, copies);

    // ### Remove this line when you have fully implemented this method.
    //PLERROR("StructuralLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int StructuralLearner::outputsize() const
{
    return(train_set->getDictionary(inputsize_)->size()+1);
}

void StructuralLearner::forget()
{
    //! (Re-)initialize the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    /*!
      A typical forget() method should do the following:
      - initialize a random number generator with the seed option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */
  if(w.size()!=0)
    initializeParams();
    
  stage = 0;

}
    
void StructuralLearner::train()
{
  if (!initTrain())
      return;

  // Compute thetas over auxiliary task,
  // if an auxiliary problem is given
  if( auxiliary_task_train_set && stage == 0)  {
      
      // Preprocessing of auxiliary task should be done by now!

      // Train initial weights ws
      std::cerr << "StructuralLearner::train() - Perform SVD" << std::endl;    

      real best_error=REAL_MAX;
      real current_error=REAL_MAX;
      
      while(current_error < best_error)  {
          best_error = current_error;
      
      }
      
      // Now, using computed theta to bias training

      // Perform SVD
      std::cerr << "StructuralLearner::train() - Performing SVD" << std::endl;
      
      // --- Concat
      Mat U = sqrt(lambda) * U;
      Mat U_t = transpose(U);
      
      std::cout << "U_t.length() " << U_t.length() << " U_t.width() " << U_t.width() << std::endl;
      
      // --- Faire la SVD
      Mat V1, V2;
      Vec D;
      SVD(U_t, V1, D, V2, 'A');
      
      std::cout << "V1.length() " << V1.length() << " V1.width() " << V1.width() << std::endl;
      
      theta_t = V1.subMatRows(0, 50);
      theta_t = transpose(theta_t);
      
      U.resize(0, 0);
      U_t.resize(0, 0);
      V1.resize(0, 0);
      V2.resize(0, 0);
      D.resize(0);  


  }
  else
  {
      // Train target classifier
      std::cerr << "StructuralLearner::train() - Training target classifier" << std::endl;
      
      while(stage<nstages)
      {
          
          std::cerr << "StructuralLearner::train() - stage is " << stage << std::endl;
          
          train_stats->forget();
          learning_rate = start_learning_rate / (1+decrease_constant*stage);
          
          for(int i=0; i<train_set->length(); i++)  {
              
              train_set->getExample(i, input, target, weight);
              // 1) compute the output
              computeOutput(input, output) ; 
              // 2) compute the cost      
              computeCostsFromOutputs(input, output, target, costs);
              train_stats->update(costs);

              // 3) Update weights
                            
              // Pour les xj différents de 0 (k is a modified index - 0/1 encoding)

              for(int i=0; i<outputsize(); i++) 
              {
                  for(int f=0; f<feats.length(); f++)
                  {                      
                      current_features = feats[f].data();
                      // Update w
                      for(int j=0; j<feats[f].length(); j++)  {
                          if(i!=target[2])  {
                              ws[f](current_features[j], i) -= learning_rate*output[i] + (lambda != 0 ? 2*lambda*ws[f](current_features[j], i) : 0);
                          }
                          else  {
                              ws[f](current_features[j], i) -= learning_rate*(output[i]-1) + (lambda != 0 ? 2*lambda*ws[f](current_features[j], i) : 0);
                          }
                      }                                                             

                      // Update v
                      real theta_x_j=0.0;
                      for(int j=0; j<50; j++)  {
                          for(int k=0, l=0; k<inputsize(); k++)  {
                              theta_x_j += theta_t(l+(int)input[k], j); //w[i][k+(int)input[j]];
                              l += (train_set->getDictionary(k))->size()+1;
                          }
                      
                      
                          if(i!=target[2])  {
                              v(j, i) -= update1 * theta_x_j;
                          }
                          else  {
                              v(j, i) -= update2 * theta_x_j;
                          }
                      }
                  }
              }
          }
          
          std::cerr << std::endl;
          
          ++stage;
          train_stats->finalize(); // finalize statistics for this epoch        
      }
  }
  
  
}


void StructuralLearner::computeOutput(const Vec& input, Vec& output) const
{
  int nout = outputsize(); 
  int nin = inputsize();

  real v_t_theta_elmt;

  // - Just unigrams for now!
  for(int i=0; i<nout; i++) {
      before_softmax[i] = 0;
  }

  // for all output neurons
  for(int i=0; i<nout; i++) {
    // for all inputs (k is a modified index - 0/1 encoding)
    for(int j=0, k=0; j<nin; j++)  {
        if(!is_missing(input[j])) {
            before_softmax[i] += w(k+(int)input[j], i); //w[i][k+(int)input[j]];

            v_t_theta_elmt = 0.0;
            for(int ii=0; ii<50; ii++) {
              v_t_theta_elmt+= v(ii, i) * theta_t(k+(int)input[j], ii);
            } 
            before_softmax[i] += v_t_theta_elmt;
    
            k += (train_set->getDictionary(j))->size()+1;
      }
    }
  }
 
  // compute theta * x
  for(int f=0; f<feats.length(); f++)
  {
      unsigned int *current_features = feats[f].data();
      for(int j=0; j<50; j++)
      {
          
      }
  }

 softmax(before_softmax,output);

}    

void StructuralLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
    // Don't forget about epsilon
    int index_O = 1;

// Compute the costs from *already* computed output. 
  int argout = argmax(output);
  costs[0] = -safeflog( output[(int)target[2]] );
  costs[1] = argout == target[2] ? 0 : 1; //class_error(output,target);
  if(argout != index_O) costs[2] = costs[1];
  else costs[2] = MISSING_VALUE;
  if(target[2] != index_O) costs[3] = costs[1];
  else costs[3] = MISSING_VALUE; 
   
}                                

TVec<string> StructuralLearner::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutpus
    // (these may or may not be exactly the same as what's returned by getTrainCostNames).
    TVec<string> ret;
    ret.resize(4);
    ret[0] = "NLL";
    ret[1] = "class_error";
    ret[2] = "precision";
    ret[3] = "recall";
    return ret;
}

TVec<string> StructuralLearner::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes and 
    // for which it updates the VecStatsCollector train_stats
    // (these may or may not be exactly the same as what's returned by getTestCostNames).
    TVec<string> ret;
    ret.resize(4);
    ret[0] = "NLL";
    ret[1] = "class_error";
    ret[2] = "precision";
    ret[3] = "recall";
    return ret;
}






/** 
* @brief Takes an example as input and returns the corresponding features. These are onehot encoded
* and so it is the active indices that are returned. The function returns the features' onehot encoded
* length.
* 
* @param input the example's input
* @param target the example's target
* @param BagOfWordsInThreeSyntacticChunkWindow active indices in a bag of words for the words in a 3 syntactic chunk window
* @param theFeatureGroups the features (the indices that are active) organized by groups - output
* @param option specifies whether some features are masked (default "" is none)
* 
* @returns the features' onehot encoded length
*
* @note check with Hugo: fl+=(train_set->getDictionary(0))->size()+2;
* @todo take option into account
**/
int StructuralLearner::computeFeatures(const Vec& input, const Vec& target, const TVec<unsigned int>& BagOfWordsInThreeSyntacticChunkWindow, TVec< TVec<unsigned int> >& theFeatureGroups, string option) const
{

	unsigned int fl=0;		// length of the onehot encoded features (stands for "features' length")
	TVec<unsigned int> currentFeatureGroup;
	
	// We have 8 feature groups
	theFeatureGroups.resize(0);
	theFeatureGroups.resize(8);
	
	
	// *** Wordtag features ***
	// Wordtags in a 5 word window with a onehot encoding
	// Derived from the wordtags input[0], input[3], input[6], input[9], input[12]
        currentFeatureGroup = theFeatureGroups[0];
	for(int i=0, ii=0; i<5; i++)  {
		ii=3*i;
		
		if( !is_missing(input[ii]) ) {
			currentFeatureGroup.push_back( (unsigned int)(fl + input[ii]) );
		}
		else	{
			currentFeatureGroup.push_back( fl + (train_set->getDictionary(ii))->size() + 1 );  // explicitly say it's missing
		}
		fl += (train_set->getDictionary(ii))->size()+2; // +1 for OOV and +1 for missing
	}//for wordtags 


	// *** POS features ***
	// POStags in a 5 word window with a onehot encoding
	// Derived from the postags input[1], input[4], input[7], input[10], input[13]
        currentFeatureGroup = theFeatureGroups[1];
	for(int i=0, ii=0; i<5; i++)  {
		ii=3*i+1;
		
		if( !is_missing(input[ii]) ) {
			currentFeatureGroup.push_back( (unsigned int)(fl + input[ii]) );
		}
		else	{
			currentFeatureGroup.push_back( fl + (train_set->getDictionary(ii))->size() + 1 );  // explicitly say it's missing
		}
		fl += (train_set->getDictionary(ii))->size()+2; // +1 for OOV and +1 for missing
	}//for postags 

/*
	// *** Char type features ***
	// Char type features in a 5 word window - 4 features (1 if true, 0 if not):
	//		-1st letter capitalized
	//		-All letters capitalized
	//		-All digits
	//		-All digits and '.'  ','
	// Derived from the words (wordtags in input[0], input[3], input[6], input[9], input[12])
	currentFeatureGroup = theFeatureGroups[2];
	for(int i=0, ii=0; i<5; i++)  {
		ii=3*i;
		
		// Word is not missing, test for 4 features
		if( !is_missing(input[ii]) ) {
			
			// Get the word
			(train_set->getDictionary(ii))->getSymbol(input[ii]);
			
		}

		fl+=4; 
	}//for 5 word window


	// *** Prefix features ***
	// Prefix features - 4 initial caracters, onehot-encoded (we only consider letters)
	// Derived from the words (wordtags in input[0], input[3], input[6], input[9], input[12])
	for(int i=0, ii=0; i<5; i++)  {
	{
		ii=3*i;
		
		// Word is not missing, look at 4 1st caracters
		if( !is_missing(input[ii]) ) {
			
			// Get the word
			(train_set->getDictionary(ii))->getSymbol(input[ii]);
			
			// for 4 1st chars, if they exist!
			// Check that Char - 'a' or 'A' is between 0 and 25
			// Hugo - what to do if special char or if shorter than 4... have 2 bits for that?
			
		}

		fl+=26; 
	}//for 5 word window

//!!!!!!!!!!!Check with Hugo that push_back works as in stl (=makes a copy?)
activeIndicesGroups.push_back(currentActiveIndicesGroup);
currentActiveIndicesGroup.resize(0);


	// *** Suffix features ***
	// Suffix features - 4 last caracters, onehot-encoded (we only consider letters)
	// Derived from the words (wordtags in input[0], input[3], input[6], input[9], input[12])
	for(int i=0, ii=0; i<5; i++)  {
	{
		ii=3*i;
		
		// Word is not missing, look at 4 last caracters
		if( !is_missing(input[ii]) ) {
			
			// Get the word
			(train_set->getDictionary(ii))->getSymbol(input[ii]);
			
			// for 4 last chars, if they exist!
			// Check that Char - 'a' or 'A' is between 0 and 25
			// Hugo - what to do if special char or if shorter than 4... have 2 bits for that? if so, change the +26
			
		}

		fl+=26; 
	}//for 5 word window

//!!!!!!!!!!!Check with Hugo that push_back works as in stl (=makes a copy?)
activeIndicesGroups.push_back(currentActiveIndicesGroup);
currentActiveIndicesGroup.resize(0);


	// *** "Bag of words in a 3 syntactic chunk window" features ***
	// we have this from preprocessing
	for(int i=0; i<wordsIn3SyntacticContext.length(); i++)	{
		currentActiveIndicesGroup.push_back( fl + wordsIn3SyntacticContext[i] );
	}
	
	fl += (train_set->getDictionary(0))->size()+1; // +1 and not 2 because none of these words can be missing
//!!!!!!!!!!!Check with Hugo that push_back works as in stl (=makes a copy?)
activeIndicesGroups.push_back(currentActiveIndicesGroup);
currentActiveIndicesGroup.resize(0);


	// *** Label features ***
	// Labels of the 2 words on the left - should always be in the target (if we are decoding, then the target
	// should hold what we have predicted
	if( !is_missing(target[0]) ) {
		currentActiveIndicesGroup.push_back( fl + target[0] );
	}
	fl += (train_set->getDictionary(inputsize_))->size()+1;
	if( !is_missing(target[1]) ) {
		currentActiveIndicesGroup.push_back( fl + target[1] );
	}
	fl += (train_set->getDictionary(inputsize_))->size()+1;
//!!!!!!!!!!!Check with Hugo that push_back works as in stl (=makes a copy?)
activeIndicesGroups.push_back(currentActiveIndicesGroup);
currentActiveIndicesGroup.resize(0);


	// *** Previous occurences features ***
	// ...
	
*/
	return fl;
}

/*
//PA - need to integrate this
int StructuralLearner::determineWordsIn3SyntacticContext(VMat example_set, TVec< TVec<unsigned int> >& wordsIn3SyntacticContext_set)	{
	
	TVec< unsigned int > leftSyntacticChunkBagOfWords;
	TVec< unsigned int > CurrentSyntacticChunkBagOfWords;
	TVec< unsigned int > RightSyntacticChunkBagOfWords;
	
	TVec< unsigned int > wordsIn3SyntacticContext;

	input[8] is current chunk
	
	// set currentSyntacticChunk
	//compute CurrentSyntacticChunkBagOfWords and RightSyntacticChunkBagOfWords
	// then cat into wordsIn3SyntacticContext
	
	for(int e=0; e<train_set->length(); e++)  {
		train_set->getExample(e, input, target, weight);
	
		// We encounter a new chunk
		if( input[8] != currentSyntacticChunk )	{		// input[8] is the current syntactic chunk - never a missing value
			leftSyntacticChunkBagOfWords = CurrentSyntacticChunkBagOfWords;
			CurrentSyntacticChunkBagOfWords = RightSyntacticChunkBagOfWords;
			// set currentSyntacticChunk
			// compute new RightSyntacticChunkBagOfWords
			// readjust wordsIn3SyntacticContext by cating all 3 (insure unicity? YES!)
		}
		
		wordsIn3SyntacticContext_set.push_back(wordsIn3SyntacticContext);
		
	}//for the examples
		
		
	return 0;
}

*/



//////////////////////
// initializeParams //
//////////////////////
void StructuralLearner::initializeParams(bool set_seed)
{
    if (set_seed) {
        if (seed_>=0)
            manual_seed(seed_);
        else
            PLearn::seed();
    }

  // initialize weights
  if (train_set) {
    real delta;
    int is = w.size();
    delta = 1.0 / sqrt(real(is));
    fill_random_uniform(w, -delta, delta);

    is = v.size();
    delta = 1.0 / sqrt(real(is));
    fill_random_uniform(w, -delta, delta);
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
