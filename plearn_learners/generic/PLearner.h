// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// PLearner.h
//
// Copyright (C) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio, Nicolas Chapados, Charles Dugas, Rejean Ducharme, Universite de Montreal
// Copyright (C) 2001,2002 Francis Pieraut, Jean-Sebastien Senecal
// Copyright (C) 2002 Frederic Morin, Xavier Saint-Mleux, Julien Keable
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
   * $Id: PLearner.h,v 1.16 2003/10/31 20:50:42 plearner Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnAlgo/PLearner.h */

#ifndef PLearner_INC
#define PLearner_INC

#include "Object.h"
#include "VMat.h"
#include "Splitter.h"
#include "TMat.h"
#include "VecStatsCollector.h"

namespace PLearn <%
using namespace std;

/*!     The base class for learning algorithms, which
    should be the main "products" of PLearn.
*/
  class PLearner: public Object
  {
  private:
    mutable int n_train_costs_;
    mutable int n_test_costs_;

  public:

    typedef Object inherited;
    
    // Build options

    //! Path of the directory associated with this learner, in which
    //! it should save any file it wishes to create.
    //! The directory will be created if it does not already exist.
    //! If expdir is the empty string (the default), then the learner
    //! should not create *any* file. Note that, anyway, most file creation and
    //! reporting are handled at the level of the PTester class rather than
    //! at the learner's.
    string expdir; 

    long seed_; //!< the seed used for the random number generator in initializing the learner (see forget() method).
    int stage; //!< The current training stage, since last fresh initialization (forget()):
               //!< 0 means untrained, n often means after n epochs or optimization steps, etc...
               //!< The true meaning is learner-dependant.
               //!< You should never modify this option directly!
               //!< It is the role of forget() to bring it back to 0,
               //!< and the role of train() to bring it up to 'nstages'...
    int nstages;//!< The stage until which train() should train this learner and return.
                //!< The meaning of 'stage' is learner-dependent, but for learners whose
                //!< training is incremental (such as involving incremental optimization),
                //!< it is typically synonym with the number of 'epochs', i.e. the number
                //!< of passages of the optimization process through the whole training set,
                //!< since the last fresh initialisation.

    bool report_progress; //!< should progress in learning and testing be reported in a ProgressBar
    int verbosity; //! Level of verbosity. If 0, should not write anything on cerr. If >0 may write some info on the steps performed (the amount of detail written depends on the value of this option).

  protected:

    //! The training set as set by setTrainingSet 
    /*!
    Data-sets are seen as matrices whose columns or fields are layed out as
    follows: a number of input fields, followed by (optional) target fields,
    followed by a (optional) weight field (to weigh each example).
    The sizes of those areas are given by the VMatrix options
    inputsize targetsize, and weightsize, which are typically used by the
    learner upon building.
    */
    VMat train_set;  
    
    VMat validation_set;

    //! The stats_collector responsible for collecting train cost statistics during training.
    //! This is typically set by some external training harness that wants to collect some stats.
    PP<VecStatsCollector> train_stats;

  public:

    PLearner();
    virtual ~PLearner();

    //! Declares the train_set
    //! Then calls build() and forget() if necessary
    //! Note: You shouldn't have to overload this in subclasses, except in maybe to forward the call to an underlying learner.
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    //! Returns the current train_set
    inline VMat getTrainingSet() const { return train_set; }

    //! Set the validation set (optionally) for learners that are able to use it directly 
    virtual void setValidationSet(VMat validset);

    //! Returns the current validation set
    VMat getValidationSet() const { return validation_set; }

    //! Sets the statistics collector whose update() method will be called 
    //! during training.
    //! Note: You shouldn't have to overload this in subclasses, except maybe to forward the call to an underlying learner.
    virtual void setTrainStatsCollector(PP<VecStatsCollector> statscol);

    //! Returns the train stats collector
    inline PP<VecStatsCollector> getTrainStatsCollector()
    { return train_stats; }

    //! The experiment directory is the directory in which files 
    //! related to this model are to be saved.     
    //! If it is an empty string, it is understood to mean that the 
    //! user doesn't want any file created by this learner.
    virtual void setExperimentDirectory(const string& the_expdir);

    //! This returns the currently set expdir (see setExperimentDirectory)
    string getExperimentDirectory() const { return expdir; }

    //! Default returns train_set->inputsize()
    virtual int inputsize() const;

    //! Default returns train_set->targetsize()
    virtual int targetsize() const; 

    //! SUBCLASS WRITING: overload this so that it returns 
    //! the size of this learner's output, as a function of 
    //! its inputsize(), targetsize() and set options
    virtual int outputsize() const =0;

  public:

    //!  **** SUBCLASS WRITING: ****
    //! This method should be redefined in subclasses, to just call inherited::build() and then build_()
    virtual void build();

  private:
    /*!       **** SUBCLASS WRITING: ****
    This method should finalize building of the object,
    according to set 'options', in *any* situation. 

    Typical situations include:
      - Initial building of an object from a few user-specified options
      - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
      - Updating or "re-building" of an object after a few "tuning" options 
        (such as hyper-parameters) have been modified.

      You can assume that the parent class' build_() has already been called.

      A typical build method will want to know the inputsize(), targetsize() and outputsize(),
      and may also want to check whether train_set->hasWeights(). All these methods require a 
      train_set to be set, so the first thing you may want to do, is check if(train_set), before
      doing any heavy building... 

      Note: build() is always called by setTrainingSet.
    */
    void build_();
    
  public:
    //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    /*!       *** SUBCLASS WRITING: ***
      A typical forget() method should do the following:
         - initialize a random number generator with the seed option
         - initialize the learner's parameters, using this random generator
         - stage = 0;
      This method is typically called by the build_() method, after 
      it has finished setting up the parameters, and if it deemed
      useful to set or reset the learner in its fresh state. 
      (remember build may be called after modifying options that do not 
      necessarily require the learner to restart from a fresh state...)
      forget is also called by the setTrainingSet method, after calling build(),
      so it will generally be called TWICE during setTrainingSet!
    */
    virtual void forget() =0;

    
    //! The role of the train method is to bring the learner up to stage==nstages,
    //! updating the stats with training costs measured on-line in the process.

    /*! *** SUBCLASS WRITING: ***

      TYPICAL CODE:

      static Vec input;  // static so we don't reallocate/deallocate memory each time...
      static Vec target;
      input.resize(inputsize());    // the train_set's inputsize()
      target.resize(targetsize());  // the train_set's targetsize()
      real weight;

      if(!train_stats)  // make a default stats collector, in case there's none
         train_stats = new VecStatsCollector();

      if(nstages<stage) // asking to revert to a previous stage!
         forget();  // reset the learner to stage=0


      while(stage<nstages)
        {
          // clear statistics of previous epoch
          train_stats->forget(); 
          
          //... train for 1 stage, and update train_stats,
          // using train_set->getSample(input, target, weight);
          // and train_stats->update(train_costs)
          
          ++stage;
          train_stats->finalize(); // finalize statistics for this epoch
        }


    */
    virtual void train() =0;


    //! *** SUBCLASS WRITING: ***
    //! This should be defined in subclasses to compute the output from the input
    virtual void computeOutput(const Vec& input, Vec& output) const =0;

    //! *** SUBCLASS WRITING: ***
    //! This should be defined in subclasses to compute the weighted costs from
    //! already computed output. The costs should correspond to the cost names 
    //! returned by getTestCostNames()
    //! NOTE: In exotic cases, the cost may also depend on some info in the input, 
    //! that's why the method also gets so see it.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const =0;
                                
    //! Default calls computeOutput and computeCostsFromOutputs
    //! You may overload this if you have a more efficient way to 
    //! compute both output and weighted costs at the same time.
    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                       Vec& output, Vec& costs) const;

    //! Default calls computeOutputAndCosts
    //! This may be overloaded if there is a more efficient way to compute the costs
    //! directly, without computing the whole output vector.
    virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;
    
  
    //! Computes outputs for the input part of testset.
    //! testset is not required to contain a target part.
    //! The default version repeatedly calls computeOutput
    virtual void use(VMat testset, VMat outputs) const;

    //! Performs test on testset, updating test cost statistics,
    //! and optionally filling testoutputs and testcosts
    //! The default version repeatedly calls computeOutputAndCosts or computeCostsOnly
    virtual void test(VMat testset, PP<VecStatsCollector> test_stats, 
                      VMat testoutputs=0, VMat testcosts=0) const;

    //! *** SUBCLASS WRITING: ***
    //! This should return the names of the costs computed by computeCostsFromOutpus
    virtual TVec<string> getTestCostNames() const =0;

    //! *** SUBCLASS WRITING: ***
    //! This should return the names of the objective costs that the train method computes and 
    //! for which it updates the VecStatsCollector train_stats
    virtual TVec<string> getTrainCostNames() const =0;

    //! Caches getTestCostNames().size() in an internal variable
    //! the first time it is called, and then returns the content of this variable.
    virtual int nTestCosts() const;

    //! Caches getTrainCostNames().size() in an internal variable
    //! the first time it is called, and then returns the content of this variable.
    virtual int nTrainCosts() const;

    //! returns the index of the given cost in the vector of testcosts
    //! calls PLERROR (throws a PLearnException) if requested cost is not found.
    int getTestCostIndex(const string& costname) const;

    //! returns the index of the given cost in the vector of traincosts (objectives)
    //! calls PLERROR (throws a PLearnException) if requested cost is not found.
    int getTrainCostIndex(const string& costname) const;

  protected:
    static void declareOptions(OptionList& ol);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  public:
    //!  Does the necessary operations to transform a shallow copy (this)
    //!  into a deep copy by deep-copying all the members that need to be.
    PLEARN_DECLARE_ABSTRACT_OBJECT(PLearner);
  };

  DECLARE_OBJECT_PTR(PLearner);


//! Parses a statname of the form "E[E[test1.class_error]]" or "V[ MIN [train.squared_error]]"
//! If the external stat is omitted in statname, it will be assumed to be "E[...]"
//! It will set extat and intstat to be for ex. "E"
//! setnum will be 0 for "train" and 1 for "test1", 2 for "test2", ...
// void parse_statname(const string& statname, string& extstat, string& intstat, int& setnum, string& errorname);

// Vec trainTestLearner(PP<PLearner> learner, const VMat &dataset, PP<Splitter> splitter, TVec<string> statnames);


%> // end of namespace PLearn

#endif





