// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: BinaryVariable.h,v 1.1 2002/07/30 09:01:28 plearner Exp $
   * PRIMARY AUTHOR: Pascal Vincent
   * CONTRIBUTORS: Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef BinaryVariable_INC
#define BinaryVariable_INC

#include "VarArray.h"
//#include "TMat_maths.h"
#include "Mat.h"
#include "TMat_maths_specialisation.h"

namespace PLearn <%
using namespace std;


class BinaryVariable: public Variable
{
  typedef Variable inherited;

protected:
  //!  Default constructor for persistence
  BinaryVariable() {}
  
protected:
  Var input1;
  Var input2;

public:
  BinaryVariable(Variable* v1, Variable* v2, int thelength, int thewidth);
  DECLARE_ABSTRACT_NAME_AND_DEEPCOPY(BinaryVariable);

  void sizeprop();
  virtual void setParents(const VarArray& parents);

  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual bool markPath();
  virtual void buildPath(VarArray& proppath);
  virtual VarArray sources();
  virtual VarArray random_sources();
  virtual VarArray ancestors();
  virtual void unmarkAncestors();
  virtual VarArray parents();
  void printInfo(bool print_gradient) { 
    cout << getName() << "[" << (void*)this << "] " << *this
         << "(" << (void*)input1 << "," << (void*)input2
         << ") = " << value;
    if (print_gradient) cout << " gradient=" << gradient;
    cout << endl; 
  }
  virtual void resizeRValue();
}; 



//!  Variable that is the row of the input1 variable indexed 
//!  by the input2 variable 
class VarRowVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  VarRowVariable() {}

public:
  VarRowVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(VarRowVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  //  virtual void rprop();
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

/*!   Variable that is a subset of a matrix's rows
  input1 : matrix from which rows are selected
  input2 : vector whose elements are row indices in input1
*/
class VarRowsVariable: public BinaryVariable
{
 protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  VarRowsVariable() {}

 public:
  VarRowsVariable(Variable *input1, Variable *input2);
  DECLARE_NAME_AND_DEEPCOPY(VarRowsVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  //  virtual void rprop();
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

/*!   Variable that is a subset of a matrix's columns
  input1 : matrix from which columns are selected
  input2 : vector whose elements are row indices in input1
*/
class VarColumnsVariable: public BinaryVariable
{
 protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  VarColumnsVariable() {}

 public:
  VarColumnsVariable(Variable *input1, Variable *input2);
  DECLARE_NAME_AND_DEEPCOPY(VarColumnsVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  //  virtual void rprop();
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

/*!   Variable that is the element of the input1 variable indexed 
  by the input2 variable.
  If input2 has 2 elements, they are interpreted as the (i,j) indexes
  If input2 is a scalar, it is interpreted as the k index of a vec view of input1
*/
class VarElementVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  VarElementVariable() {}

public:
  VarElementVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(VarElementVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  //  virtual void rprop();
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};


/*! * Variables positionned inside a larger zero variable ... * */

/*!   A variable of length() 'length()', and of width() input1->nelems(), filled with zeros 
  except for the row indexed by input2, which is a copy of input1
  (this is used in particular for the symbolic bprop of VarRowVariable) 
*/
class RowAtPositionVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  RowAtPositionVariable() {}

public:
  RowAtPositionVariable(Variable* input1, Variable* input2, int the_length);
  DECLARE_NAME_AND_DEEPCOPY(RowAtPositionVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  //  virtual void rprop();
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();

};

/*!   A variable of size length() x width(), filled with zeros except for the 
  single element indexed by input2 =(i,j) or input2 = (k).
  This element will be a copied from input1 (a scalar) 
  (this is used in particular for the symbolic bprop of VarElementVariable) 
*/
class ElementAtPositionVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  ElementAtPositionVariable() {}

public:
  ElementAtPositionVariable(Variable* input1, Variable* input2, int the_length, int the_width);
  DECLARE_NAME_AND_DEEPCOPY(ElementAtPositionVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};


//!  adds a scalar var to a matrix var
class PlusScalarVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  PlusScalarVariable() {}

public:
  PlusScalarVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(PlusScalarVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//!  adds a single-row var to each row of a matrix var
class PlusRowVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  PlusRowVariable() {}

public:
  PlusRowVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(PlusRowVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  //  virtual void rprop();
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//!  adds a single-column var to each column of a matrix var
class PlusColumnVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  PlusColumnVariable() {}

public:
  PlusColumnVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(PlusColumnVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  //  virtual void rprop();
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//!  adds 2 matrix vars of same size
class PlusVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  PlusVariable() {}

public:
  PlusVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(PlusVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  //  virtual void rprop();
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
};

/*! * Minus... * */

class MinusScalarVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  MinusScalarVariable() {}

public:
  MinusScalarVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(MinusScalarVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

class MinusRowVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  MinusRowVariable() {}

public:
  MinusRowVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(MinusRowVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
};

class MinusTransposedColumnVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  MinusTransposedColumnVariable() {}

public:
  MinusTransposedColumnVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(MinusTransposedColumnVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

class MinusColumnVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  MinusColumnVariable() {}

public:
  MinusColumnVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(MinusColumnVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
};

class MinusVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  MinusVariable() {}

public:
  MinusVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(MinusVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
};


//!  multiplies a matrix var by a scalar var
class TimesScalarVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  TimesScalarVariable() {}

public:
  TimesScalarVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(TimesScalarVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//!  multiplies each row of a matrix var elementwise with a single row variable
class TimesRowVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  TimesRowVariable() {}

public:
  TimesRowVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(TimesRowVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//!  multiplies each column of a matrix var elementwise with a single column variable
class TimesColumnVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  TimesColumnVariable() {}

public:
  TimesColumnVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(TimesColumnVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//!  multiplies 2 matrix vars of same size elementwise
class TimesVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  TimesVariable() {}

public:
  TimesVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(TimesVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

/*! * Div... * */

//!  divides 2 matrix vars of same size elementwise
class DivVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  DivVariable() {}

public:
  DivVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(DivVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

/*! * Dot and Matrix products... * */

//!  Dot product between 2 vectors (or possibly 2 matrices, which are then simply seen as vectors) 
class DotProductVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  DotProductVariable() {}

public:
  DotProductVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(DotProductVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//! Affine transformation of a vector variable.
//! Should work for both column and row vectors: result vector will be of same kind (row or col)
//! First row of transformation matrix contains bias b, following rows contain linear-transformation T
//! Will compute b + x.T (if you consider b and x to be row vectors)
//! which is equivalent to b + 
class AffineTransformVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  AffineTransformVariable() {}

public:
  AffineTransformVariable(Variable* vec, Variable* transformation):
    BinaryVariable(vec, transformation, 
                   vec->isRowVec()?1:transformation->width(),
                   vec->isColumnVec()?1:transformation->width())
  {
    if(!vec->isVec())
      PLERROR("In AffineTransformVariable: expecting a vector Var (row or column) as first argument");
  }
  DECLARE_NAME_AND_DEEPCOPY(AffineTransformVariable);

  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();

};

//! Affine transformation of a MATRIX variable.
//! Should work for both column and row vectors: result vector will be of same kind (row or col)
//! First row of transformation matrix contains bias b, following rows contain linear-transformation T
//! Will compute b + x.T 
//! which is equivalent to b + 
class MatrixAffineTransformVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  MatrixAffineTransformVariable() {}

public:
  MatrixAffineTransformVariable(Variable* mat, Variable* transformation):
    BinaryVariable(mat, transformation, transformation->width(), mat->width())
  {}
  DECLARE_NAME_AND_DEEPCOPY(MatrixAffineTransformVariable);

  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

//!  Matrix product
class ProductVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  ProductVariable() {}

public:
  ProductVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(ProductVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//!  Matrix product between matrix1 and transpose of matrix2
class ProductTransposeVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  ProductTransposeVariable() {}

public:
  ProductTransposeVariable(Variable* matrix1, Variable* matrix2);
  DECLARE_NAME_AND_DEEPCOPY(ProductTransposeVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//!  Matrix product between transpose of matrix1 and matrix2
class TransposeProductVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  TransposeProductVariable() {}

public:
  TransposeProductVariable(Variable* matrix1, Variable* matrix2);
  DECLARE_NAME_AND_DEEPCOPY(TransposeProductVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};


/*! * Efficient and numerically stable loss functions * */

//! Computes sum(square_i(netout[i]-(i==classnum ?hotval :coldval)) 
//! This is used typically in a classification setting where netout is a Var 
//! of network outputs, and classnum is the target class number.
class OneHotSquaredLoss: public BinaryVariable
{
protected:
  typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  OneHotSquaredLoss() {}

  real coldval_, hotval_;

public:
  OneHotSquaredLoss(Variable* netout, Variable* classnum, real coldval=0., real hotval=1.);
  DECLARE_NAME_AND_DEEPCOPY(OneHotSquaredLoss);
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

class MatrixOneHotSquaredLoss: public BinaryVariable
{
protected:
  typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  MatrixOneHotSquaredLoss() {}
  real coldval_, hotval_;

public:
  MatrixOneHotSquaredLoss(Variable* netout, Variable* classnum, real coldval=0., real hotval=1.);
  DECLARE_NAME_AND_DEEPCOPY(MatrixOneHotSquaredLoss);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
};

//! Indicator(classnum==argmax(netout))
class ClassificationLossVariable: public BinaryVariable
{
protected:
  typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  ClassificationLossVariable() {}

public:
  ClassificationLossVariable(Variable* netout, Variable* classnum);
  DECLARE_NAME_AND_DEEPCOPY(ClassificationLossVariable);
  virtual void fprop();

  //! can't bprop through a hard classification error...
  virtual void bprop() {}
};

//! cost = sum_i {cost_i}, with
//! cost_i = 1  if (target_i == 1  &&  output_i < 1/2)
//! cost_i = 1  if (target_i == 0  &&  output_i > 1/2)
//! cost_i = 0  otherwise

class MulticlassLossVariable: public BinaryVariable
{
protected:
  typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  MulticlassLossVariable() {}

public:
  MulticlassLossVariable(Variable* netout, Variable* target);
  DECLARE_NAME_AND_DEEPCOPY(MulticlassLossVariable);
  virtual void fprop();

  //! can't bprop through a hard classification error...
  virtual void bprop() {}
};

//! cost = - sum_i {target_i * log(output_i) + (1-target_i) * log(1-output_i)}
class CrossEntropyVariable: public BinaryVariable
{
protected:
  typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  CrossEntropyVariable() {}

public:
  CrossEntropyVariable(Variable* netout, Variable* target);
  DECLARE_NAME_AND_DEEPCOPY(CrossEntropyVariable);
  virtual void fprop();
  virtual void bprop();
};

/*
//! Computes -log(exp(netoutput[target])/sum(exp(netoutput))) in an efficient and numericly stable manner
//         = - netoutput[target] - logadd(output)
class NegLogSoftmaxLoss: public BinaryVariable
{
  
};
*/


/*!   elementwise pow: power may be a scalar or a var 
  with same dimensions as the input
  (returns 0 wherever input is negative)
*/
class PowVariableVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  PowVariableVariable() {}

public:
  PowVariableVariable(Variable* input, Variable* power);
  DECLARE_NAME_AND_DEEPCOPY(PowVariableVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

/*!   elementwise max over 2 elements:
   max(v1,v2)[i] = max(v1[i],v2[i])
  with same dimensions as the input vectors
  (both must have same length())
*/
class Max2Variable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  Max2Variable() {}

public:
  Max2Variable(Variable* input, Variable* power);
  DECLARE_NAME_AND_DEEPCOPY(Max2Variable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};


//!  output = log(exp(input1)+exp(input2)) but it is
//!  computed in such a way as to preserve precision
class LogAddVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  LogAddVariable() {}

public:
  LogAddVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(LogAddVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};


//!  A scalar var;  equal 1 if input1==input2, 0 otherwise
class EqualVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  EqualVariable() {}

public:
  EqualVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(EqualVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

//!  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IsLargerVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  IsLargerVariable() {}

public:
  IsLargerVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(IsLargerVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop(); 
  virtual void symbolicBprop(); 
};

class IsSmallerVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  IsSmallerVariable() {}
  
public:
  IsSmallerVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(IsSmallerVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};
//!  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//!  A scalar var;  equal 1 if input1==input2, 0 otherwise
class EqualScalarVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  EqualScalarVariable() {}

public:
  EqualScalarVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(EqualScalarVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop(); 
};


//!  A convolve var; equals convolve(input, mask)
class ConvolveVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  ConvolveVariable() {}

public:
  ConvolveVariable(Variable* input, Variable* mask);
  DECLARE_NAME_AND_DEEPCOPY(ConvolveVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

class SoftmaxLossVariable: public BinaryVariable
{
protected:
  typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  SoftmaxLossVariable() {}

public:
  SoftmaxLossVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(SoftmaxLossVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};


class MatrixSoftmaxVariable: public BinaryVariable
{
protected:
  typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  MatrixSoftmaxVariable() {}
public:
  MatrixSoftmaxVariable(Variable* input1, Variable* input2);
  DECLARE_NAME_AND_DEEPCOPY(MatrixSoftmaxVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};



/********************************************
 * Weighted Unary Variables
 ********************************************/

class WeightedSumSquareVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  WeightedSumSquareVariable() {}

public:
  WeightedSumSquareVariable(Variable* input, Variable* weights);
  DECLARE_NAME_AND_DEEPCOPY(WeightedSumSquareVariable);
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};




%> // end of namespace PLearn

#endif





