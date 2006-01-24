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

  ninputs_onehot=0;

    // With these values, will not learn
    start_learning_rate=0.0;
    decrease_constant=0.0;

    ninputs_onehot=0;
    lambda=0.0001;

    labeled_train_set_indices.resize(0);
    //m_tvec_auxiliaryLearners.resize(0);

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

void StructuralLearner::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "w", &StructuralLearner::w, OptionBase::learntoption,
                   "Weights of the linear classifier: f(x) = wt x + vt theta x");
    declareOption(ol, "v", &StructuralLearner::v, OptionBase::learntoption,
                   "Weights of the linear classifier: f(x) = wt x + vt theta x");
    declareOption(ol, "theta_t", &StructuralLearner::theta_t, OptionBase::learntoption,
                   "structure parameter of the linear classifier: f(x) = wt x + vt theta x");
    declareOption(ol, "start_learning_rate", &StructuralLearner::start_learning_rate, OptionBase::buildoption,
                   "Starting learning rate of the stochastic gradient descent");
    declareOption(ol, "decrease_constant", &StructuralLearner::decrease_constant, OptionBase::buildoption,
                   "Decrease constant of the stochastic learning rate");

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


    // ***** Determine onehot input size
    ninputs_onehot = 0;
    // * Just unigrams for now!
    for(int i=0; i<inputsize_; i++) {
      ninputs_onehot += (train_set->getDictionary(i))->size()+1;     // + 1 !!! ok for OOV?
    }

    // ***** Resize vectors
    input.resize(inputsize());
    target.resize(targetsize());
    costs.resize(getTrainCostNames().length());
    before_softmax.resize(outputsize());
    output.resize(outputsize());


    // ***** If this is the first call -> Build the labeled train set
    if(labeled_train_set_indices.size()==0) {
      std::cerr << "building labeled train set...";
      // *** Select all labelled instances
      for(int e=0; e<train_set->length(); e++)  {
    
        train_set->getExample(e, input, target, weight);
    
        // * if the example is labeled - add it
        if(!is_missing(target[2]))
          labeled_train_set_indices.push_back(e);
      }

      labeled_train_set = new SelectRowsVMatrix(train_set, labeled_train_set_indices);
      std::cerr << "done" << std::endl;
      std::cout << "StructuralLearner train_set->length() " << train_set->length() << " labeled_train_set->length() " << labeled_train_set->length() << std::endl;
    }
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
  // This generic PLearner method does a number of standard stuff useful for
  // (almost) any learner, and return 'false' if no training should take
  // place. See PLearner.h for more details.
  if (!initTrain())
      return;

  // first call to train -> compute theta
  if( theta_t.size()==0 )  {

    /*theta_t.resize(ninputs_onehot, 50);
    // ***** 1) Optional Auxiliary Label creation - train classifiers using labelled data and a specific feature map
    // We don't use this yet, we only use the auxiliary problems of type unsupervised-"predict observable substructures"

    // ***** 2) For all auxiliary problems assign auxiliary labels to unlabelled data
    // For now, we hardcoded our learners to use the input adequately as target
    

    // ***** 3) Compute theta 
    std::cerr << "StructuralLearner::train() - Compute theta" << std::endl;

    // We just iterate once for this - it should be enough to capture what we need
    // -> Pour le VRAI SVD ASO 
    // utiliser m_tvec_auxiliaryLearners.push_back(*auxiliaryLearner);
    // setter v et theta dans le learner (retirer le hardcodé)

    // *** a) Train auxiliary learners

    // --- Prédicteur de mot courant
    PP<StructuralAuxiliaryLearner> wordPredictor = new StructuralAuxiliaryLearner();
    wordPredictor->setTrainingSet(train_set);

    // - Faire l'entraînement
    // -> Train it (roughly done)

    // should really improve this hacky init...
    real best_error=1.1;
    real current_error=1.0;

    while(current_error < best_error)  {
      best_error = current_error;

      wordPredictor->nstages++;
      wordPredictor->train();

      current_error = (wordPredictor->getTrainStatsCollector()->getMean())[0];

std::cerr << "Auxiliary stage " << wordPredictor->nstages << " current_error " << current_error << std::endl;
    } 


    // *** b) Perform SVD
    std::cerr << "StructuralLearner::train() - Perform SVD" << std::endl;

    // --- Concaténer les vecteurs
    Mat U = sqrt(lambda) * wordPredictor->u_t;

    // --- Try and free some memory (smart pointers)
    wordPredictor=NULL;

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
    D.resize(0);*/

    // Random theta
    theta_t.resize(ninputs_onehot, 50);
    real delta;
    int is = theta_t.size();
    delta = 1.0 / sqrt(real(is));
    fill_random_uniform(theta_t, -delta, delta);

  }


  std::cout << "theta_t.length() " << theta_t.length() << " theta_t.width() " << theta_t.width() << std::endl;

  // ***** 4) Train target classifier
  std::cerr << "StructuralLearner::train() - Training target classifier" << std::endl;

  // ***** Resize w, v and theta_t if necessary
  if(w.length() != ninputs_onehot || w.width() != outputsize()) {
    w.resize(ninputs_onehot, outputsize());
    v.resize(50, outputsize());
    initializeParams(seed_);
  }

  real update1, update2;
  real learning_rate;

  while(stage<nstages)
  {

    std::cerr << "StructuralLearner::train() - stage is " << stage << std::endl;

    train_stats->forget();
    learning_rate = start_learning_rate / (1+decrease_constant*stage);
    
    for(int e=0; e<labeled_train_set->length(); e++)  {

      if( (e%10000) == 0)  {
        std::cerr << "%";
      }

      labeled_train_set->getExample(e, input, target, weight);

 
      //std::cerr << "A" << std::endl;

      // *** train for 1 stage, and update train_stats,
      
      // 1) compute the output of the sparse neural network
      computeOutput(input, output) ; 
  
      //std::cerr << "B" << std::endl;

      // 2) compute the cost      
      computeCostsFromOutputs(input, output, target, costs);
      train_stats->update(costs);

      //std::cerr << "C" << std::endl;

      int index_O = 1;//dictionary->getId("O");
      real other_weight_fraction = 0.2;
      if(target[2] == index_O)
      {
        update1 = learning_rate * other_weight_fraction * output[(int)target[2]];
        update2 = learning_rate * other_weight_fraction * (output[(int)target[2]]-1);
      }
      else
      {
        update1 = learning_rate * (1-other_weight_fraction) * output[(int)target[2]];
        update2 = learning_rate * (1-other_weight_fraction) * (output[(int)target[2]]-1);
      } 

      //std::cerr << "D" << std::endl;

      // Pour les xj différents de 0 (k is a modified index - 0/1 encoding)
  
      // for all output neurons
      for(int i=0; i<outputsize(); i++) {

        // Update w
        // for all inputs (k is a modified index - onehot encoding)
        for(int j=0, k=0; j<inputsize(); j++)  {
          if(i!=target[2])  {
              if(!is_missing(input[j]))
                  w(k+(int)input[j], i) -= update1;
          }
          else  {
              if(!is_missing(input[j]))
                  w(k+(int)input[j], i) -= update2;
          }
          k += (train_set->getDictionary(j))->size()+1;
        }

        // Update v
        // for all inputs (k is a modified index - onehot encoding)
        real theta_x_j=0.0;
        for(int j=0; j<50; j++)  {

          // for all inputs (k is a modified index - 0/1 encoding)
          for(int k=0, l=0; k<inputsize(); k++)  {
            if(!is_missing(input[k]))
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

      std::cerr << std::endl;
      
      ++stage;
      train_stats->finalize(); // finalize statistics for this epoch        
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
  softmax(before_softmax,output);

}    

void StructuralLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
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
