// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal

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
   * $Id: UnaryVariable.h,v 1.5 2002/09/11 19:35:36 wangxian Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef UnaryVariable_INC
#define UnaryVariable_INC

#include "VarArray.h"
#include "TMat_maths.h"

namespace PLearn <%
using namespace std;

class UnaryVariable: public Variable
{
  typedef Variable inherited;

protected:
  //!  Default constructor for persistence
  UnaryVariable() {}
  
protected:
  Var input;

public:
  UnaryVariable(Variable* v, int thelength, int thewidth);

  DECLARE_ABSTRACT_NAME_AND_DEEPCOPY(UnaryVariable);
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
  void printInfo(bool print_gradient) 
    { 
      cout << getName() << "[" << (void*)this << "] " << *this
           << " (" << (void*)input << ") = " << value;
      if (print_gradient) 
        cout << " gradient=" << gradient;
      cout << endl; 
    }
  virtual void resizeRValue();
};





/*! * Different views of a mat Var * */

//!  Variable that views another variable, but with a different length() and width() 
//!  (the only restriction being that length()*width() remain the same)
class ReshapeVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  ReshapeVariable() {}
  int length_, width_;

public:
  ReshapeVariable(Variable* v, int the_length, int the_width);
  DECLARE_NAME_AND_DEEPCOPY(ReshapeVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();

};



class SubMatVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  SubMatVariable() : startk () {}

protected:
  int startk;
  int length_, width_;
public:
  SubMatVariable(Variable* v, int i, int j, int the_length, int the_width);
  DECLARE_NAME_AND_DEEPCOPY(SubMatVariable);

  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

class SubMatTransposeVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  SubMatTransposeVariable() : startk() {}

protected:
  int startk;
  int length_, width_;
public:
  SubMatTransposeVariable(Variable* v, int i, int j, int the_length, int the_width);
  DECLARE_NAME_AND_DEEPCOPY(SubMatTransposeVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};


/*! * Indexing an external Mat or Vec with a variable * */

//!  Variable that is the row of matrix mat indexed by variable input
class MatRowVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  MatRowVariable() {}

protected:
  Mat m;

public:
  MatRowVariable(const Mat& mat, Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(MatRowVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();

};

//!  Variable that is the element of vector vec indexed by variable input
class VecElementVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  VecElementVariable() {}

protected:
  Vec v;

public:
  VecElementVariable(const Vec& vec, Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(VecElementVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();

};

/*! * ExtendedVariable * */

/*!   Variable that extends the input variable by appending rows at 
  its top and bottom and columns at its left and right.
  The appended rows/columns are filled with the given fill_value
  This can be used for instance to easily implement the usual trick 
  to include the bias in the weights vectors, by appending a 1 to the inputs.
  It is also used in the symbolic bprop of SubMatVariable.
*/
class ExtendedVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  ExtendedVariable() : top_extent(), bottom_extent(), left_extent(),
                       right_extent(), fill_value() {}

public:
  int top_extent;
  int bottom_extent;
  int left_extent;
  int right_extent;
  real fill_value; 

  ExtendedVariable(Variable* input, 
                   int the_top_extent, int the_bottom_extent, 
                   int the_left_extent, int the_right_extent, 
                   real the_fill_value);
  DECLARE_NAME_AND_DEEPCOPY(ExtendedVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};



/*! * Duplicates... * */

class DuplicateScalarVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  DuplicateScalarVariable() {}
  int length_, width_;

public:
  DuplicateScalarVariable(Variable* input, int thelength, int thewidth);
  DECLARE_NAME_AND_DEEPCOPY(DuplicateScalarVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();

};

class DuplicateRowVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  DuplicateRowVariable() : n_duplicates() {}
  int length_;

public:
  int n_duplicates;

  DuplicateRowVariable(Variable* input, int thelength);
  DECLARE_NAME_AND_DEEPCOPY(DuplicateRowVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();

};

class DuplicateColumnVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  DuplicateColumnVariable() : n_duplicates() {}
  int width_;

public:
  int n_duplicates;

  DuplicateColumnVariable(Variable* input, int thewidth);
  DECLARE_NAME_AND_DEEPCOPY(DuplicateColumnVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();

};


/*! * Sum reductions... * */

class SumVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  SumVariable() {}

public:
  SumVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(SumVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//!  result is a single row that contains the sum of each column of the input
class ColumnSumVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  ColumnSumVariable() {}

public:
  ColumnSumVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(ColumnSumVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();

};

//!  result is a single column that contains the sum of each row of the input
class RowSumVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  RowSumVariable() {}

public:
  RowSumVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(RowSumVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();

};


/*! * Plus... * */

//!  adds a scalar constant to a matrix var
class PlusConstantVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  PlusConstantVariable() : cst() {}

public:
  virtual string info() const;

public:
  real cst; //!<  The constant

  PlusConstantVariable(Variable* input, real c);
  DECLARE_NAME_AND_DEEPCOPY(PlusConstantVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};


/*! * Times... * */

//!  multiplies a matrix var by a scalar constant
class TimesConstantVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  TimesConstantVariable() : cst() {}

public:
  virtual string info() const; 

public:
  real cst; //!<  The constant

  TimesConstantVariable(Variable* input, real c);
  DECLARE_NAME_AND_DEEPCOPY(TimesConstantVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//! weight decay terms for affine transforms
class AffineTransformWeightPenalty: public UnaryVariable
  {
  protected:
    typedef UnaryVariable inherited;
    //!  Default constructor for persistence
    AffineTransformWeightPenalty() {}
    
    real weight_decay_;
    real bias_decay_;

  public:
    DECLARE_NAME_AND_DEEPCOPY(AffineTransformWeightPenalty);
    AffineTransformWeightPenalty(Variable* affinetransform, real weight_decay, real bias_decay=0.) 
      :UnaryVariable(affinetransform, 1,1),weight_decay_(weight_decay),bias_decay_(bias_decay)
    {}
  virtual void recomputeSize(int& l, int& w) const;    
    virtual void fprop();
    virtual void bprop();
};

/*! * Elementwise negation and inversion... * */

class NegateElementsVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  NegateElementsVariable() {}

public:
  NegateElementsVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(NegateElementsVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

class InvertElementsVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  InvertElementsVariable() {}

public:
  InvertElementsVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(InvertElementsVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

/*! * Matrix inversions... * */

class MatrixInverseVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  MatrixInverseVariable() {}

public:
  MatrixInverseVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(MatrixInverseVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

class LeftPseudoInverseVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  LeftPseudoInverseVariable() {}

public:
  LeftPseudoInverseVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(LeftPseudoInverseVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

class RightPseudoInverseVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  RightPseudoInverseVariable() {}

public:
  RightPseudoInverseVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(RightPseudoInverseVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

/*! * Simple unary transforms... * */

class TanhVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  TanhVariable() {}

public:
  TanhVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(TanhVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

class SigmoidVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  SigmoidVariable() {}

public:
  SigmoidVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(SigmoidVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//!  This is the primitive of a sigmoid: log(1+exp(x))
class SoftplusVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  SoftplusVariable() {}

public:
  SoftplusVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(SoftplusVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

class SquareVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  SquareVariable() {}

public:
  SquareVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(SquareVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  //!  here don't approximate, do d2C/dx^2 = 4 x^2 d2C/dy^2 + 2 dC/dy 
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

class SumSquareVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  SumSquareVariable() {}

public:
  SumSquareVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(SumSquareVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  //virtual void bbprop();
  virtual void symbolicBprop();
  //virtual void rfprop();
};


class SquareRootVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  SquareRootVariable() {}

public:
  SquareRootVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(SquareRootVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  //!  here don't approximate, do d2C/dx^2 = 4 x^2 d2C/dy^2 + 2 dC/dy 
  //virtual void bbprop();  
};


class AbsVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  AbsVariable() {}

public:
  AbsVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(AbsVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

///////////////////////////////////////!<  

class MaxVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  MaxVariable() {}

public:
  MaxVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(MaxVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

class MinVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  MinVariable() {}

public:
  MinVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(MinVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

/*!   This variable computes the index of the maximum value in the input
  It is a scalar variable if the input is a vector (single row or single column)
  Otherwise it is a single-column of length() 2 containing the (i,j) coordinate of the max 
*/
class ArgmaxVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  ArgmaxVariable() {}

public:
  ArgmaxVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(ArgmaxVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

/*!   This variable computes the index of the minimum value in the input
  It is a scalar variable if the input is a vector (single row or single column)
  Otherwise it is a single-column of length() 2 containing the (i,j) coordinate of the min 
*/
class ArgminVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  ArgminVariable() {}

public:
  ArgminVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(ArgminVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

class CutAboveThresholdVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  CutAboveThresholdVariable() : threshold() {}

protected:
  real threshold;
public:
  CutAboveThresholdVariable(Variable* input, real the_threshold);
  DECLARE_NAME_AND_DEEPCOPY(CutAboveThresholdVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

class CutBelowThresholdVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  CutBelowThresholdVariable() : threshold() {}

protected:
  real threshold;
public:
  CutBelowThresholdVariable(Variable* input, real the_threshold);
  DECLARE_NAME_AND_DEEPCOPY(CutBelowThresholdVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

class ExpVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  ExpVariable() {}

public:
  ExpVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(ExpVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  //virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

class LogVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  LogVariable() {}

public:
  LogVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(LogVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

class LogSumVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  LogSumVariable() {}

protected:
  Vec input_softmax;
public:
  LogSumVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(LogSumVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

//!  returns the elementwise x*log(x) in a (hopefully!) numerically stable way
//!  This can be used to compute the Entropy for instance
class PLogPVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  PLogPVariable() {}

public:
  PLogPVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(PLogPVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

//!  elementwise pow
//!  (returns 0 wherever input is negative)
class PowVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  PowVariable() : power() {}

protected:
  real power;

public:
  PowVariable(Variable* input, real the_power);
  DECLARE_NAME_AND_DEEPCOPY(PowVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

//!  The argument must be a square matrix Var
//!  and the result is its determinant
class DeterminantVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  DeterminantVariable() {}

public:
  DeterminantVariable(Var m);
  DECLARE_NAME_AND_DEEPCOPY(DeterminantVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};
  
//!  if values = [x1,x2,...,x10], the resulting variable is 
//!  [(x1+x2)/2,(x2+x3)/2, ... (x9+x10)/2]
class InterValuesVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  InterValuesVariable() {}

public:
  InterValuesVariable(Variable* values);
  DECLARE_NAME_AND_DEEPCOPY(InterValuesVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};
  
//!  Does elementwise newx_i = (x_i>=threshold ?truevalue :falsevalue);
class IsAboveThresholdVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  IsAboveThresholdVariable() : threshold(), truevalue(), falsevalue() {}

protected:
  real threshold, truevalue, falsevalue;
public:
  IsAboveThresholdVariable(Variable* input, real the_threshold, real the_truevalue, real the_falsevalue);
  DECLARE_NAME_AND_DEEPCOPY(IsAboveThresholdVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};


//!  Represents a vector of a given lenth, that has value 1 at the index 
//!  given by another variable and 0 everywhere else 
class OneHotVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  OneHotVariable() : hotvalue(), coldvalue() {}

protected:
  real hotvalue;
  real coldvalue;
  int length_;

public:
  OneHotVariable(int thelength, Variable* index, real the_coldvalue, real the_hotvalue);
  DECLARE_NAME_AND_DEEPCOPY(OneHotVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//!  A scalar var;  equal 1 if input1==input2, 0 otherwise
class EqualConstantVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  EqualConstantVariable() : eqv() {}

public:
  virtual string info() const; 

protected:
  real eqv;
public:
  EqualConstantVariable(Variable* input1, real input2);
  DECLARE_NAME_AND_DEEPCOPY(EqualConstantVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

//!  A scalar var;  equal 1 if input1!=c, 0 otherwise
class UnequalConstantVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  UnequalConstantVariable() : c() {}

public:
  virtual string info() const; 

protected:
  real c;
public:
  UnequalConstantVariable(Variable* input1, real c);
  DECLARE_NAME_AND_DEEPCOPY(UnequalConstantVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

//!  A subsample var; equals subrample(input, the_subsamplefactor)
class SubsampleVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  SubsampleVariable() : subsamplefactor() {}

protected:
  int subsamplefactor;

public:
  SubsampleVariable(Variable* input,int the_subsamplefactor);
  DECLARE_NAME_AND_DEEPCOPY(SubsampleVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

class ErfVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  ErfVariable() {}

public:
  ErfVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(ErfVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

//!  sign(x) = 1 if x>0, -1 if x<0, 0 if x=0, all done element by element.
class SignVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  SignVariable() {}

public:
  SignVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(SignVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

class SoftmaxVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  SoftmaxVariable() {}

public:
  SoftmaxVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(SoftmaxVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

class MatrixSoftmaxVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  MatrixSoftmaxVariable() {}

public:
  MatrixSoftmaxVariable(Variable* input);
  DECLARE_NAME_AND_DEEPCOPY(MatrixSoftmaxVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

class LogSoftmaxVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
    //! Default constructor for pestilence
    LogSoftmaxVariable() {};
public:
    LogSoftmaxVariable(Variable *input);
    DECLARE_NAME_AND_DEEPCOPY(LogSoftmaxVariable);
  virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();
    virtual void bbprop();
    virtual void symbolicBprop();
}; // class LogSoftmaxVariable

%> // end of namespace PLearn

#endif





