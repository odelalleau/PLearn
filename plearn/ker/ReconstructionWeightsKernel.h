// -*- C++ -*-

// ReconstructionWeightsKernel.h
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
   * $Id: ReconstructionWeightsKernel.h,v 1.3 2004/07/20 13:01:14 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file ReconstructionWeightsKernel.h */


#ifndef ReconstructionWeightsKernel_INC
#define ReconstructionWeightsKernel_INC

#include "Kernel.h"
#include "SelectRowsVMatrix.h"

namespace PLearn {
using namespace std;

class ReconstructionWeightsKernel: public Kernel
{

private:

  typedef Kernel inherited;

  //! True iff build() has been called but build_() has not been called yet.
  bool build_in_progress;
  
protected:

  // *********************
  // * Protected options *
  // *********************

  // Fields below are not options.

  //! The kernel used to compute distances (equal to 'distance_kernel' if
  //! specified, and otherwise the usual Euclidean distance).
  Ker dist_ker;

  //! The kernel used to compute dot products (equal to 'dot_product_kernel'
  //! if specified, and otherwise the usual DotProductKernel).
  Ker dp_ker;
    
  //! The indices of the neighbors of each data point.
  TMat<int> neighbors;

  //! The element i is a matrix whose first column is the list of the points
  //! which have i among their neighbors, and the second column is the index
  //! of i in these neighbors.
  //! The first column is sorted by increasing index, and does not contain i.
  TVec< TMat<int> > is_neighbor_of;

  //! Points toward a subset of the training data (typically, a neighborhood).
  PP<SelectRowsVMatrix> sub_data;

  //! The matrix with the weights W_{ij}.
  Mat weights;

public:

  // ************************
  // * Public build options *
  // ************************

  Ker distance_kernel;
  Ker dot_product_kernel;
  int knn;
  real regularizer;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  // Make sure the implementation in the .cc initializes all fields to
  // reasonable default values.
  ReconstructionWeightsKernel();

  // ******************
  // * Kernel methods *
  // ******************

private: 

  //! This does the actual building. 
  void build_();

protected: 
  
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(ReconstructionWeightsKernel);

  // **************************
  // **** Kernel methods ****
  // **************************

  //! Compute K(x1,x2).
  virtual real evaluate(const Vec& x1, const Vec& x2) const;

  virtual real evaluate_i_j(int i, int j) const;

  virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const;

  virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const;

  virtual void setDataForKernelMatrix(VMat the_data);

  //! Return sum_k K(x_k, x_i) * K(x_k, x_j).
  virtual real evaluate_sum_k_i_k_j(int i, int j) const;

  //! Fill 'lle_mat', which must be of size (n x n), with entries (i,j) equal to
  //! W_{ij} + W_{ji} - \sum_k W_{ki} W_{kj}
  //! (this is used in LLE to compute the kernel Gram matrix).
  virtual void computeLLEMatrix(const Mat& lle_mat) const;

  // *** SUBCLASS WRITING: ***
  // While in general not necessary, in case of particular needs 
  // (efficiency concerns for ex) you may also want to overload
  // some of the following methods:
  // virtual real evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x=-1, bool first_time = false) const;
  // virtual real evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x=-1, bool first_time = false) const;
  // virtual void computeGramMatrix(Mat K) const;
  // virtual void addDataForKernelMatrix(const Vec& newRow);
  // virtual void setParameters(Vec paramvec);
  // virtual Vec getParameters() const;

protected:

  //! Precompute the weights W_{ij}.
  void computeWeights();
  
  //! Compute the reconstruction weights for a vector, given its nearest neighbors.
  void reconstruct(const Vec& x, const TVec<int>& neighbors, Vec& w) const;

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(ReconstructionWeightsKernel);
  
} // end of namespace PLearn

#endif

