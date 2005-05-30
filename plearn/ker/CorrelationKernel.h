// -*- C++ -*-

// CorrelationKernel.h
//
// Copyright (C) 2005 Olivier Delalleau 
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
   * $Id: CorrelationKernel.h,v 1.3 2005/05/30 20:17:27 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file CorrelationKernel.h */


#ifndef CorrelationKernel_INC
#define CorrelationKernel_INC

#include <plearn/ker/Kernel.h>
#include <plearn/vmat/VMatLanguage.h>

namespace PLearn {

class CorrelationKernel: public Kernel
{

private:

  typedef Kernel inherited;
  
protected:

  mutable Mat correl;     //!< Used to store the correlations.
  mutable Mat pvalues;    //!< Used to store the pvalues.
  Vec mean_vec;           //!< Used to store the mean of each example (variable).
  Vec var_vec;            //!< Used to store the variance of each example (variable).
  real min_product_var;   //!< Value used to threshold products of variances.
  //! VPL program that transforms the similarity measure.
  PP<VMatLanguage> transform_prg;
  //! Fields of the VPL program.
  TVec<string> transform_prg_fields;
  //! Used to store the similarity measure.
  Vec result_vec;
  //! Used to store the similarity measure tranformed by 'transform_prg'.
  Vec result_transformed_vec;

  // *********************
  // * Protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...
    
public:

  // ************************
  // * Public build options *
  // ************************

  string correlation;
  string transform;
  real   var_threshold;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  // Make sure the implementation in the .cc initializes all fields to
  // reasonable default values.
  CorrelationKernel();

  // ******************
  // * Kernel methods *
  // ******************

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
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS.
  PLEARN_DECLARE_OBJECT(CorrelationKernel);

  // ************************
  // **** Kernel methods ****
  // ************************

  //! Compute K(x1,x2).
  virtual real evaluate(const Vec& x1, const Vec& x2) const;

  //! Overridden to precompute 'min_product_var' if necessary.
  virtual void setDataForKernelMatrix(VMat the_data);

  // *** SUBCLASS WRITING: ***
  // While in general not necessary, in case of particular needs 
  // (efficiency concerns for ex) you may also want to overload
  // some of the following methods:
  // virtual real evaluate_i_j(int i, int j) const;
  // virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const;
  // virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const;
  // virtual real evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x=-1, bool first_time = false) const;
  // virtual real evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x=-1, bool first_time = false) const;
  // virtual void computeGramMatrix(Mat K) const;
  // virtual void addDataForKernelMatrix(const Vec& newRow);
  // virtual void setParameters(Vec paramvec);
  // virtual Vec getParameters() const;
  

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(CorrelationKernel);
  
} // end of namespace PLearn

#endif

