// -*- C++ -*-

// GenericNearestNeighbors.h
//
// Copyright (C) 2004 Nicolas Chapados
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
   * $Id: GenericNearestNeighbors.h,v 1.3 2004/12/25 08:03:29 chapados Exp $ 
   ******************************************************* */

// Authors: Nicolas Chapados


#ifndef GenericNearestNeighbors_INC
#define GenericNearestNeighbors_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

/**
 * This class provides an abstract base class for nearest-neighbors-type
 * algorithms.  The basic abstraction is that from a test point, one can
 * ask to find the "K" nearest points from the training set.  (Specified
 * through the "num_neighbors" option).  Although per se, this class (and
 * its descendants) only FIND the nearest neighbors, the design is such
 * that it can be embedded in concrete algorithms that perform
 * classification or regression.  The separation between "neighborhood
 * finding" and "how to use the neighbors" allows multiple instantiations
 * of, say, K-Nearest-Neighbors classification, using several (exact and
 * approximate) neighbors-finding algorithms.
 *
 * There are a number of options that control how the output vectors are
 * generated.  (See below).  For a given neighbor found, the output vector
 * is always the concatenation of one or more of (in that order):
 *
 * - The input vector from the training set (option "copy_input")
 * - The target vector from the training set (option "copy_target")
 * - The weight from the training set (option "copy_weight"); note that
 *   if the training set DOES NOT contain a weight, but copy_weight is
 *   set to 'true', then a weight of 1.0 is always inserted.  This
 *   simplifies client code who may then assume that a weight is always
 *   present if requested
 * - The index (row number) of the example from the training set (option
 *   "copy_weight")
 *
 * If more than one neighbor is requested, the complete output vector of
 * this learner is simply the concatenation of the above template for
 * creating one output vector.
 *
 * The learner's costs are dependent on the derived classes.  It is
 * suggested that, at least, the similarity measure (Kernel value) between
 * the test and train points be output.
 *
 */ 
class GenericNearestNeighbors: public PLearner
{
  typedef PLearner inherited;

protected:
  //! Internal buffer for constructing the output vector
  mutable Vec currow;
  
public:
  //#####  Public Build Options  ############################################

  //! Number of nearest-neighbors to compute.  This is usually called "K".
  //! The output vector is simply the concatenation of all found neighbors.
  int num_neighbors;

  //! If true, the output contains a copy of the found input vector(s).
  //! (Default = false)
  bool copy_input;

  //! If true, the output contains a copy of the found target vector(s).
  //! (Default = true)
  bool copy_target;

  //! If true, the output contains a copy of the found weight.  If no
  //! weight is present in the training set, a weight of 1.0 is put.
  //! (Default = true)
  bool copy_weight;

  //! If true, the output contains the index of the found neighbor
  //! (as the row number, zero-based, in the training set.)
  bool copy_index;
  

public:
  //#####  Object Methods  ##################################################
  
  //! Default constructor.
  GenericNearestNeighbors();


  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  // Declares other standard object methods.
  // If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT.
  PLEARN_DECLARE_ABSTRACT_OBJECT(GenericNearestNeighbors);


public:
  //#####  PLearner Methods  ################################################

  //! Returns the size of this learner's output, (which typically
  //! may depend on its inputsize(), targetsize() and set options).
  virtual int outputsize() const;

private: 
  //! This does the actual building. 
  void build_();

protected: 
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

  //! From a vector of indexes into the training set, expand them into
  //! an output vector for the learner, and take into account all the
  //! options.  One can pass less than num_neighbors indexes, in which
  //! case the rest of the output vector is filled with missings.
  void constructOutputVector(const TVec<int>& indexes, Vec& output) const;
};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(GenericNearestNeighbors);
  
} // end of namespace PLearn

#endif
