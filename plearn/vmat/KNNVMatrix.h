// -*- C++ -*-

// KNNVMatrix.h
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
   * $Id: KNNVMatrix.h,v 1.3 2004/02/20 21:14:44 chrish42 Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file KNNVMatrix.h */


#ifndef KNNVMatrix_INC
#define KNNVMatrix_INC

#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;

class KNNVMatrix: public SourceVMatrix
{

public:

  typedef SourceVMatrix inherited;

protected:

  // *********************
  // * protected options *
  // *********************

  Mat nn;
    
public:

  // ************************
  // * public build options *
  // ************************

  int knn;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  KNNVMatrix();

  // ******************
  // * VMatrix methods *
  // ******************

private: 

  //! This does the actual building. 
  void build_();

protected: 

  //! Declares this class' options
  static void declareOptions(OptionList& ol);

  //! Return the index in the source matrix of the sample number i in
  //! this matrix.
  inline int getSourceIndexOf(int i) const;

  //! Return the tag of the sample number p in a bag:
  //!   p == 0      => 1
  //!   p == knn-1  => 2
  //!   otherwise   => 0  
  //! (If knn == 1, always return 3).
  inline int getTag(int p) const;

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
  PLEARN_DECLARE_OBJECT(KNNVMatrix);


  // **************************
  // **** VMatrix methods ****
  // **************************

  //! get element at i-th row, j-th column
  //! (Please implement in .cc)
  virtual real get(int i, int j) const;

  //! get part or all of the i-th, starting at the j-th column,
  //! with v.length() elements; these elements are put in v.
  //! (Please implement in .cc)
  //! (default version repeatedly calls get(i,j) which may have a significant overhead)
  virtual void getSubRow(int i, int j, Vec v) const;

  //! Needed because it's a SourceVMatrix.
  virtual void getRow(int i, Vec v) const;

  // ** Methods that can be optionally implemented
  // ** (their default implementation throws an error)
  //

  //!  in case the dimensions of an underlying vmat has changed, recompute it
  // virtual void reset_dimensions();

  // ** for writable VMatrices

  //! Set element (i,j) to given value 
  // virtual void put(int i, int j, real value);

  //! (default version repeatedly calls put(i,j,value) which may have a significant overhead)
  // virtual void putSubRow(int i, int j, Vec v);

  //! copies matrix m at position i,j of this VMat
  // virtual void putMat(int i, int j, Mat m);

  //! This method must be implemented for matrices that are allowed to grow
  // virtual void appendRow(Vec v);

  //! fill with given value
  // virtual void fill(real value);

  //! For matrices stored on disk, this should flush all pending buffered write operations
  // virtual void flush();

  //! will call putRow if i<length() and appendRow if i==length()
  // void putOrAppendRow(int i, Vec v);

  //! will call putRow if i<length()
  //! if i>= length(), it will call appendRow with 0 filled rows as many times as necessary before 
  //! it can append row i 
  // void forcePutRow(int i, Vec v);

  // ** Some other VMatrix methods, that can be redefined if a more 
  // ** efficient implementation is possible with 
  // ** this particular subclass.
  //

  //! default version returns a SubVMatrix referencing the current VMatrix
  //! however this can be overridden to provide more efficient shortcuts 
  //! (see MemoryVMatrix::subMat and SubVMatrix::subMat for examples)
  // virtual VMat subMat(int i, int j, int l, int w);

  //! copies the submatrix starting at i,j into m (which must have appropriate length and width)
  // virtual void getMat(int i, int j, Mat m) const;

  //! copies column i into v (which must have appropriate length equal to the VMat's length)
  // virtual void getColumn(int i, Vec v) const;

  //! returns a Mat with the same data as this VMat
  //! The default version of this method copies the data in a fresh Mat created in memory
  //! However this method will typically be overrided by subclasses (such as MemoryVMatrix) 
  //! whose internal representation is already a Mat in order to return this Mat directly to avoid 
  //! a new memory allocation and copy of elements. In this case, and in this case only, modifying 
  //! the elements of the returned Mat will logically result in modified elements in the original 
  //! VMatrix view of it. 
  // virtual Mat toMat() const;

  //!  The default implementation of this method does nothing
  //!  But subclasses may overload it to reallocate memory to exactly what is needed and no more.
  // virtual void compacify();

  //! returns the dot product between row i1 and row i2 (considering only the inputsize first elements).
  //! The default version in VMatrix is somewhat inefficient, as it repeatedly calls get(i,j)
  //! The default version in RowBufferedVMatrix is a little better as it buffers the 2 Vecs between calls in case one of them is needed again.
  //! But the real strength of this method is for specialised and efficient versions in subbclasses. 
  //! This method is typically used by SmartKernels so that they can compute kernel values between input samples efficiently.
  // virtual real dot(int i1, int i2, int inputsize) const;

  //! returns the result of the dot product between row i and the given vec (only v.length() first elements of row i are considered).
  // virtual real dot(int i, const Vec& v) const;
};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(KNNVMatrix);
  
} // end of namespace PLearn

#endif
