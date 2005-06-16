// -*- C++ -*-

// ThresholdedKernel.h
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
   * $Id: ThresholdedKernel.h,v 1.3 2005/06/16 18:34:50 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file ThresholdedKernel.h */


#ifndef ThresholdedKernel_INC
#define ThresholdedKernel_INC

#include <plearn/ker/SourceKernel.h>

namespace PLearn {

class ThresholdedKernel: public SourceKernel
{

private:

  typedef SourceKernel inherited;
  
protected:

  // *********************
  // * Protected options *
  // *********************

  //! Element i is equal to K(x_i, n_knn(x_i)) with K the source kernel.
  Vec knn_kernel_values;

  //! Used to store kernel values.
  Vec k_x_xi;

  //! Points to the same data as k_x_xi;
  Mat k_x_xi_mat;

  //! The value K(x, n_knn(x)).
  mutable real k_x_threshold;
    
public:

  // ************************
  // * Public build options *
  // ************************

  int knn;
  int max_size_for_full_gram;
  string method;
  real threshold;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  ThresholdedKernel();

  // ************************
  // * SourceKernel methods *
  // ************************

private: 

  //! This does the actual building. 
  void build_();

protected: 
  
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

  //! Replace all elements in the Gram matrix K by 'threshold' when they meet
  //! the thresholding condition defined by the thresholding method.
  virtual void thresholdGramMatrix(const Mat& K) const;

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
  PLEARN_DECLARE_OBJECT(ThresholdedKernel);

  // ******************************
  // **** SourceKernel methods ****
  // ******************************

  //! Overridden.
  // The default behavior of a SourceKernel is to forward all calls to the
  // underlying kernel. When writing a subclass, you will probably want to
  // override the following methods. The only method you really need to implement
  // is evaluate().
  virtual real evaluate(const Vec& x1, const Vec& x2) const;
  virtual real evaluate_i_j(int i, int j) const;
  virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const;
  virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const;
  virtual real evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x=-1, bool first_time = false) const;
  virtual real evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x=-1, bool first_time = false) const;

  //! Overridden for a more efficient implementation when the underlying kernel
  //! has cached its Gram matrix.
  virtual void computeGramMatrix(Mat K) const;

  //! Overridden to precompute nearest neighbors in the dataset.
  virtual void setDataForKernelMatrix(VMat the_data);
  
  // You may also want to override these methods if you don't want them
  // to be directly forwarded to the underlying kernel.
  // virtual void addDataForKernelMatrix(const Vec& newRow);
  // virtual void setParameters(Vec paramvec);
  // virtual Vec getParameters() const;

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(ThresholdedKernel);
  
} // end of namespace PLearn

#endif


