// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1999-2002 Christian Jauvin
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

/*! \file DoubleAccessSparseMatrix.h */

#ifndef DoubleAccessSparseMatrix_INC
#define DoubleAccessSparseMatrix_INC

#include "Object.h"

namespace PLearn <%
using namespace std;

#define ROW_WISE 1
#define COLUMN_WISE 2

/*!   

  This is a sparse matrix implemented as a STL vector of STL maps. Thus, each element 
  of the vector is a sparse vector that only holds non-null elements (no space is wasted 
  for null elements, as their "existance" is marked by their absence from the table).

  For conveniance, it is accessible column-wise, row-wise, or both. If you want to manipulate 
  the sparse matrix with both accesses, note that there is an important space trade-off, because 
  of the need of keeping two separate tables in memory (one being simply the transpose of the 
  other).

  There are two flags for manipulating the access and preferences for the possible 
  operations that can be applied to the matrix. The first one is the boolean 'double_access' : 
  if it is set to 'true', the matrix can be acessed either by its columns or by its rows.
  The second flag that must be set is 'mode' (that can take the values ROW_WISE or COLUMN_WISE) :
  it plays two roles, depending on the value of 'double_access'. (1) If 'double_access' is set 
  to 'false', 'mode' will determine which of both matrices is the one to work with. (2) If
  'double_access' is set to 'true', it will determine the preference of certain access-specific
  operations. For instance, the operator '()' will return a row or a column, depending on the 
  value of the flag. The sub-class ProbSparseMatrix will also take some actions depending on 
  its value.
  
  Note that you can define the null-element (of parametrized type T) to be anything that you
  wish. When 'setting' or 'getting' the matrix, this user-defined null-element will be used 
  for the comparison. (Thus you can have a sparse matrix full of 1s, with only sparse 0s).

*/

template<class T>
class DoubleAccessSparseMatrix: public Object
{

protected:

  vector<map<int, T> > rows;
  vector<map<int, T> > cols;
  string name;
  int mode;
  bool double_access;
  int height;
  int width;
  T null_elem;

public:

  typedef Object inherited;
  
  DoubleAccessSparseMatrix(int n_rows = 0, int n_cols = 0, string _name = "<no-name>", int _mode = ROW_WISE, bool _double_access = false, T _null_elem = 0);

  virtual ~DoubleAccessSparseMatrix() {}

  static void declareOptions(OptionList& ol);

  virtual void resize(int n_rows, int n_cols);
  
  virtual void clear();

  virtual void clearRow(int i);

  virtual void clearCol(int j);

  virtual T get(int i, int j);

  virtual T operator()(int i, int j) { return get(i, j); }

  virtual bool exists(int i, int j);

  virtual void set(int i, int j, T value);

  virtual void incr(int i, int j, T inc);

  virtual void removeElem(int i, int j);

  virtual map<int, T>& operator()(int k);

  virtual map<int, T>& getRow(int i);
  
  virtual map<int, T>& getCol(int j);

  virtual int size();

  virtual T sumRow(int i);

  virtual T sumCol(int j);

  virtual T* getAsCompressedVec();

  virtual void getAsMaxSizedCompressedVecs(int max_size, vector<pair<T*, int> >& vectors);

  virtual void addCompressedVec(T* compressed_vec, int n_elems);

  virtual void setCompressedVec(T* compressed_vec, int n_elems);
 
  virtual T sumOfElements();

  virtual int getHeight() const { return height; }

  virtual int getWidth() const { return width; }

  virtual void setDoubleAccessible(bool da);

  virtual bool isDoubleAccessible() { return double_access; }

  virtual void setMode(int new_mode);

  virtual int getMode() { return mode; }

  virtual void setName(string n) { name = n; }
  
  virtual string getName() { return name; }

};

%> // end of namespace PLearn

#include "DoubleAccessSparseMatrix_impl.h"

#endif
