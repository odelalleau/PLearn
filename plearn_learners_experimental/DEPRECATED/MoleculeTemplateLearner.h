// -*- C++ -*-

// MoleculeTemplateLearner.h
//
// Copyright (C) 2005 Dan Popovici 
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

// Authors: Dan Popovici

/*! \file MoleculeTemplateLearner.h */


#ifndef MoleculeTemplateLearner_INC
#define MoleculeTemplateLearner_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/opt/Optimizer.h>
#include "Molecule.h"
#include "Template.h"


namespace PLearn {

class MoleculeTemplateLearner: public PLearner
{

private:

  typedef PLearner inherited;
  
protected:

  // *********************
  // * protected options *
  // *********************

  
  Var input_index ; 
  VarArray mu, sigma ; 
  VarArray mu_S, sigma_S ; 
  VarArray sigma_square_S ; 
  VarArray sigma_square ; 
  mutable VarArray S ; 
  VarArray S_after_scaling ; 
  VarArray params ; 
  VarArray penalties;
  Var V , W , V_b , W_b, V_direct ; 
  Var hl ;
  Var y ; // the output 
  Var y_before_transfer ;
  Var training_cost;
  Var test_costs ;  
  Var target ; 
  Var temp_S ; 
  VarArray costs;
  VarArray temp_output ; 
  int n_actives ; 
  int n_inactives ; 
  Vec S_std  ; 
  Vec sigma_s_vec ; 

  Func f_output ;     
  Func output_target_to_costs ;
  Func test_costf ;

  vector<PMolecule> Molecules ; 
//  PMolecule  molecule ; 

    
public:
    
  // ************************
  // * public build options *
  // ************************


  int nhidden; 
  real weight_decay ; 
  int noutputs , batch_size ;
  int n_active_templates ;  // the number of templates for the actives
  int n_inactive_templates ; // the number of templates for the inactives 
  int n_templates ; // the total number of templates  
  real scaling_factor ; 
  real lrate2 ; 

  bool training_mode ; 
  bool builded ; 

  // Build options related to the optimization:
  PP<Optimizer> optimizer; // the optimizer to use (no default)

  TVec<MoleculeTemplate> templates ; 
  Vec paramsvalues ;


  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  // (Make sure the implementation in the .cc
  // initializes all fields to reasonable default values)
  MoleculeTemplateLearner();


  // ********************
  // * PLearner methods *
  // ********************

private: 

  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  
  //! Declares this class' options.
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  // Declares other standard object methods.
  // If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT.
  PLEARN_DECLARE_OBJECT(MoleculeTemplateLearner);


  // **************************
  // **** PLearner methods ****
  // **************************

  //! Returns the size of this learner's output, (which typically
  //! may depend on its inputsize(), targetsize() and set options).
  // (PLEASE IMPLEMENT IN .cc)
  virtual int outputsize() const;

  //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0 (this is the stage of a fresh learner!).
  // (PLEASE IMPLEMENT IN .cc)
  virtual void forget();

    
  //! The role of the train method is to bring the learner up to stage==nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  // (PLEASE IMPLEMENT IN .cc)
  virtual void train();


  //! Computes the output from the input.
  // (PLEASE IMPLEMENT IN .cc)
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Computes the costs from already computed output. 
  // (PLEASE IMPLEMENT IN .cc)
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
          const Vec& target, Vec& costs) const;

  virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
          Vec& output, Vec& costs) const;
    
  void compute_S_mean_std(Vec & mean , Vec & std) ;
 
  //! Returns the names of the costs computed by computeCostsFromOutpus (and thus the test method).
  // (PLEASE IMPLEMENT IN .cc)
  virtual TVec<std::string> getTestCostNames() const;

  //! Returns the names of the objective costs that the train method computes and 
  //! for which it updates the VecStatsCollector train_stats.
  // (PLEASE IMPLEMENT IN .cc)
  virtual TVec<std::string> getTrainCostNames() const;
    
  virtual void test(VMat testset, PP<VecStatsCollector> test_stats, 
                      VMat testoutputs=0, VMat testcosts=0)const ;
  
  void initializeParams() ;


  // *** SUBCLASS WRITING: ***
  // While in general not necessary, in case of particular needs 
  // (efficiency concerns for ex) you may also want to overload
  // some of the following methods:
  // virtual void computeOutputAndCosts(const Vec& input, const Vec& target, Vec& output, Vec& costs) const;
  // virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;
  // virtual void test(VMat testset, PP<VecStatsCollector> test_stats, VMat testoutputs=0, VMat testcosts=0) const;
  // virtual int nTestCosts() const;
  // virtual int nTrainCosts() const;
  // virtual void resetInternalState();
  // virtual bool isStatefulLearner() const;

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(MoleculeTemplateLearner);
  
} // end of namespace PLearn

#endif
