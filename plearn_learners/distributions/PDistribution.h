
// -*- C++ -*-

// PDistribution.h
//
// Copyright (C) 2003  Pascal Vincent 
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
   * $Id: PDistribution.h,v 1.2 2003/06/04 21:21:20 plearner Exp $ 
   ******************************************************* */

/*! \file PDistribution.h */
#ifndef PDistribution_INC
#define PDistribution_INC

#include "PLearner.h"

namespace PLearn <%
using namespace std;

class PDistribution: public PLearner
{
public:

  typedef PLearner inherited;

  // ************************
  // * public build options *
  // ************************

  //! A string where the characters have the following meaning:
  //! 'l'->log_density, 'd' -> density, 'c' -> cdf, 's' -> survival_fn
  string outputs_def;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  PDistribution();


  // ******************
  // * PLearner methods *
  // ******************

private: 
  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! simply calls inherited::build() then build_() 
  virtual void build();

  //! Provides a help message describing this class
  static string help();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  // Declares other standard object methods
  //  If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
  PLEARN_DECLARE_OBJECT_METHODS(PDistribution, "PDistribution", PLearner);


  // **************************
  // **** PLearner methods ****
  // **************************

  //! default returns train_set->inputsize()
  //! This is not appropriate if your distribution has no train_set
  //! and should in that case be verloaded to return somethingmeaningful.
  virtual int inputsize() const;
  
  //! returns 0
  virtual int targetsize() const; 

  //! returns outputs_def.length()
  virtual int outputsize() const;

  //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
  virtual void forget();

    
  //! The role of the train method is to bring the learner up to stage==nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  virtual void train();


  //! return log of probability density log(p(x))
  virtual double log_density(const Vec& x) const;

  //! return probability density p(x)
  //! [ default version returns exp(log_density(x)) ]
  virtual double density(const Vec& x) const;
  
  //! return survival function: P(X>x)
  virtual double survival_fn(const Vec& x) const;

  //! return cdf: P(X<x)
  virtual double cdf(const Vec& x) const;

  //! return E[X] 
  virtual void expectation(Vec& mu) const;

  //! return Var[X]
  virtual void variance(Mat& cov) const;

  //! Resets the random number generator used by generate using the given seed
  virtual void resetGenerator(long g_seed) const;
  
  //! return a pseudo-random sample generated from the distribution.
  virtual void generate(Vec& x) const;

  //! X must be a N x inputsize() matrix. that will be filled
  //! This will call generate N times to fill the N rows of the matrix. 
  void generateN(const Mat& X) const;

  //! Produces outputs according to what is specified in outputs_def
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Computes negative log likelihood (NLL) 
  //! assuming log-likelihood is first output.  
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;

  //! Returns [ "NLL" ]
  virtual TVec<string> getTestCostNames() const;

  //! Returns [ "NLL" ]
  virtual TVec<string> getTrainCostNames() const;

};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(PDistribution);
  
%> // end of namespace PLearn

#endif
