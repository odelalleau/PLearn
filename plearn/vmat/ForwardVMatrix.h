// ForwardVMatrix.h
// Copyright (C) 2002 Pascal Vincent
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
   * $Id: ForwardVMatrix.h,v 1.6 2004/03/23 23:08:08 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef ForwardVMatrix_INC
#define ForwardVMatrix_INC

#include "VMat.h"

namespace PLearn {
using namespace std;

//! This class is a simple wrapper to an underlying VMatrix of another type
//! All it does is forward the method calls

class ForwardVMatrix: public VMatrix
{
  typedef VMatrix inherited;

protected:
  VMat vm; // the underlying vmatrix to which all calls will be forwarded

public:

  PLEARN_DECLARE_OBJECT(ForwardVMatrix);

  ForwardVMatrix();

  //! This allows to set the underlying vmat
  void setVMat(VMat the_vm);

  //! returns the string associated with value val 
  //! for field# col. Or returns "" if no string is associated.
  virtual string getValString(int col, real val) const;
  
  //! return value associated with a string. Default returns NaN
  virtual real getStringVal(int col, const string & str) const;

  //! returns element as a string, even if its a number (which is always the case unless class is StrTableVMatrix
  virtual string getString(int row,int col) const;

  //! returns the whole string->value mapping
  map<string,real> getStringMapping(int col) const;

  virtual void computeStats(); 

  // default version calls savePMAT
  virtual void save(const string& filename) const;

  virtual void savePMAT(const string& pmatfile) const;
  virtual void saveDMAT(const string& dmatdir) const;
  virtual void saveAMAT(const string& amatfile) const;

  //!  This method must be implemented in all subclasses
  virtual real get(int i, int j) const; //!<  returns element (i,j)

  //!  This method must be implemented in all subclasses of writable matrices
  virtual void put(int i, int j, real value); //!<  sets element (i,j) to value

  //!  It is suggested that this method be implemented in subclasses to speed up accesses
  //!  (default version repeatedly calls get(i,j) which may have a significant overhead)
  virtual void getSubRow(int i, int j, Vec v) const; //!<  fills v with the subrow i lying between columns j (inclusive) and j+v.length() (exclusive)

/*!     It is suggested that this method be implemented in subclasses of writable matrices
    to speed up accesses
    (default version repeatedly calls put(i,j,value) which may have a significant overhead)
*/
  virtual void putSubRow(int i, int j, Vec v);
    
  //!  This method must be implemented for matrices that are allowed to grow
  virtual void appendRow(Vec v);

  //!  These methods do not usually need to be overridden in subclasses
  //!  (default versions call getSubRow, which should do just fine)
  virtual void getRow(int i, Vec v) const; //!<  copies row i into v (which must have appropriate length equal to the VMat's width)
  virtual void putRow(int i, Vec v);
  virtual void fill(real value);
  virtual void getMat(int i, int j, Mat m) const; //!<  copies the submatrix starting at i,j into m (which must have appropriate length and width)
  virtual void putMat(int i, int j, Mat m); //!<  copies matrix m at position i,j of this VMat

  //!  copies column i into v (which must have appropriate length equal to the VMat's length)
  virtual void getColumn(int i, Vec v) const;

/*!     returns a Mat with the same data as this VMat
    The default version of this method copies the data in a fresh Mat created in memory
    However this method will typically be overrided by subclasses (such as MemoryVMatrix) 
    whose internal representation is already a Mat in order to return this Mat directly to avoid 
    a new memory allocation and copy of elements. In this case, and in this case only, modifying 
    the elements of the returned Mat will logically result in modified elements in the original 
    VMatrix view of it. 
*/
  virtual Mat toMat() const;

  //!  The default implementation of this method does nothing
  //!  But subclasses may overload it to reallocate memory to exactly what is needed and no more.
  virtual void compacify();

  //!  in case the dimensions of an underlying vmat has changed, recompute it
  virtual void reset_dimensions();

/*!     default version returns a SubVMatrix referencing the current VMatrix
    however this can be overridden to provide more efficient shortcuts 
    (see MemoryVMatrix::subMat and SubVMatrix::subMat for examples)
*/
  virtual VMat subMat(int i, int j, int l, int w);

/*!     returns the dot product between row i1 and row i2 (considering only the inputsize first elements).
    The default version in VMatrix is somewhat inefficient, as it repeatedly calls get(i,j)
    The default version in RowBufferedVMatrix is a little better as it buffers the 2 Vecs between calls in case one of them is needed again.
    But the real strength of this method is for specialised and efficient versions in subbclasses. 
    This method is typically used by SmartKernels so that they can compute kernel values between input samples efficiently.
*/
  virtual real dot(int i1, int i2, int inputsize) const;

  //!  returns the result of the dot product between row i and the given vec (only v.length() first elements of row i are considered).
  virtual real dot(int i, const Vec& v) const;

  //!  Assigns the value of the Vars in the list
  //!  (the total size of all the vars in the list must equal width() )
  virtual void getRow(int i, VarArray& inputs) const;

  virtual void oldwrite(ostream& out) const;
  virtual void oldread(istream& in);

  void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);


/*!     The following methods can be used in a straightforward manner to compute a variety of useful things:
    Dot products between this vmat and a vector, find the K nearest neighbours to a vector, etc...
    Most methods take an optional last parameter ignore_this_row which may contain the index of a row that
    is to be excluded from the computation (this can be seful for leave-one-out evaluations for instance).
*/

/*!     This will compute for this vmat m a result vector (whose length must be tha same as m's)
    s.t. result[i] = ker( m(i).subVec(v1_startcol,v1_ncols) , v2) 
    i.e. the kernel value betweeen each (sub)row of m and v2
*/
  virtual void evaluateKernel(Ker ker, int v1_startcol, int v1_ncols, 
                              const Vec& v2, const Vec& result, int startrow=0, int nrows=-1) const;

  //!   returns sum_i [ ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
  virtual real evaluateKernelSum(Ker ker, int v1_startcol, int v1_ncols, 
                                 const Vec& v2, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;

  //!  targetsum := sum_i [ m(i).subVec(t_startcol,t_ncols) * ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
  //!  and returns sum_i [ ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
  virtual real evaluateKernelWeightedTargetSum(Ker ker, int v1_startcol, int v1_ncols, const Vec& v2, 
                                               int t_startcol, int t_ncols, Vec& targetsum, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;
  
   
/*!     This will return the Top N kernel evaluated values (between vmat (sub)rows and v2) and their associated row_index.
    Result is returned as a vector of length N of pairs (kernel_value,row_index)
    Results are sorted with largest kernel value first
*/
  virtual TVec< pair<real,int> > evaluateKernelTopN(int N, Ker ker, int v1_startcol, int v1_ncols, 
                                                    const Vec& v2, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;

  //!  same as evaluateKernelTopN but will look for the N smallest values instead of top values.
  //!  results are sorted with smallest kernel value first
  virtual TVec< pair<real,int> > evaluateKernelBottomN(int N, Ker ker, int v1_startcol, int v1_ncols, 
                                                       const Vec& v2, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;


/*!     result += transpose(X).Y
    Where X = this->subMatColumns(X_startcol,X_ncols)
    and   Y =  this->subMatColumns(Y_startcol,Y_ncols);
*/
  virtual void accumulateXtY(int X_startcol, int X_ncols, int Y_startcol, int Y_ncols, 
                             Mat& result, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;


/*!     A special case of method accumulateXtY
    result += transpose(X).X
    Where X = this->subMatColumns(X_startcol,X_ncols)
*/
  virtual void accumulateXtX(int X_startcol, int X_ncols, 
                             Mat& result, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;

  //!  compute fprop or fbprop of a sumOf operation
  virtual void evaluateSumOfFprop(Func f, Vec& output_result, int nsamples=-1);
  virtual void evaluateSumOfFbprop(Func f, Vec& output_result, Vec& output_gradient, int nsamples=-1);
 
};

DECLARE_OBJECT_PTR(ForwardVMatrix);

} // end of namespace PLearn

#endif

