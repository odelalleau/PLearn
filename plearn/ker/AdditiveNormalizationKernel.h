// -*- C++ -*-

// AdditiveNormalizationKernel.h
//
// Copyright (C) 2004 Olivier Delalleau 
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
   * $Id: AdditiveNormalizationKernel.h,v 1.6 2004/07/21 17:04:44 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file AdditiveNormalizationKernel.h */


#ifndef AdditiveNormalizationKernel_INC
#define AdditiveNormalizationKernel_INC

#include "SourceKernel.h"

namespace PLearn {
using namespace std;

class AdditiveNormalizationKernel: public SourceKernel
{

private:

  typedef SourceKernel inherited;

  //! Used to store the values of the source kernel.
  mutable Vec all_k_x;
  
protected:

  // *********************
  // * Protected options *
  // *********************

  Vec average_col;
  Vec average_row;
  real total_average;
  real total_average_unbiased;

  // Fields below are not options.

  //! The last average computed in evaluate_i_x_again().
  mutable real avg_evaluate_i_x_again;

  //! The last average computed in evaluate_x_i_again().
  mutable real avg_evaluate_x_i_again;

  //! A multiplicative factor to scale the result (1 or -1/2).
  real factor;

public:

  // ************************
  // * Public build options *
  // ************************

  bool data_will_change;
  bool double_centering;
  bool remove_bias;
  bool remove_bias_in_evaluate;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  AdditiveNormalizationKernel();

  //! Created from an existing kernel.
  AdditiveNormalizationKernel
    (Ker the_source, bool remove_bias = false, bool remove_bias_in_evaluate = false,
     bool double_centering = false);

  // ************************
  // * SourceKernel methods *
  // ************************

private: 

  //! This does the actual building. 
  void build_();

protected: 
  
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

  //! Return the average of K(x,x_i) or K(x_i,x), depending on the value of
  //! 'on_row' (true or false, respectively).
  inline real computeAverage(const Vec& x, bool on_row, real squared_norm_of_x = -1) const;

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(AdditiveNormalizationKernel);

  // ******************************
  // **** SourceKernel methods ****
  // ******************************

  //! Overridden.
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
  virtual real evaluate_i_j(int i, int j) const;
  virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const;
  virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const;
  virtual real evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x=-1, bool first_time = false) const;
  virtual real evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x=-1, bool first_time = false) const;
  virtual void computeGramMatrix(Mat K) const;
  virtual void setDataForKernelMatrix(VMat the_data);
  
  // You may also want to override these methods if you don't want them
  // to be directly forwarded to the underlying kernel.
  // virtual void addDataForKernelMatrix(const Vec& newRow);
  // virtual void setParameters(Vec paramvec);
  // virtual Vec getParameters() const;

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(AdditiveNormalizationKernel);
  
} // end of namespace PLearn

#endif


