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
   * $Id: Var.h,v 1.3 2002/09/11 04:16:23 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/Var.h */

#ifndef Var_INC
#define Var_INC

#include "Variable.h"
#include "SourceVariable.h"
#include "UnaryVariable.h"
#include "BinaryVariable.h"
#include "NaryVariable.h"

namespace PLearn <%
using namespace std;


inline ostream& operator<<(ostream& out, const Var& v)
{
  string name=v->getName();
  if (name != "")
    cout<<name<<endl;
  if (v->width()==1)
    out << v->value;
  else
    out << v->matValue;
  return out;
}


inline Var var(real init_value) { Var v(1); v=init_value; return v; }


/*! 

This part contains the more "user-friendly interface" to the underlying classes

Var is a wrapper around Variable that handles reference-counting and freeing automatically
propagationPath is a function that computes and returns the VarArray corresponding to an update propagation

Several operator overloading and function-like syntaxes allow the use of Vars with the usual intuitive expression notations.

 */

/*! *******************************
  * user-friendly Var interface *
  *******************************
*/

//!  A Var is just a smart pointer to a Variable
//!  with a few other goodies.

/*! * Various operator overloadings and inline functions that provide intuitive notation * */

//! first row of transformation is the bias.
inline Var affine_transform(Var vec, Var transformation)
{ 
 if (vec->isVec())
    return new AffineTransformVariable(vec, transformation); 
    else return new MatrixAffineTransformVariable(vec, transformation);
}

//! weight decay and bias decay terms
//! This has not been tested yet [Pascal: a tester].
inline Var affine_transform_weight_penalty(Var transformation, real weight_decay, real bias_decay=0)
{ return new AffineTransformWeightPenalty(transformation, weight_decay, bias_decay); } 

inline Var onehot_squared_loss(Var network_output, Var classnum, real coldval=0., real hotval=1.)
{ 
  if (network_output->isVec())  
  return new OneHotSquaredLoss(network_output, classnum, coldval, hotval);
  else return new MatrixOneHotSquaredLoss(network_output, classnum, coldval, hotval);
}

inline Var classification_loss(Var network_output, Var classnum)
{ return new ClassificationLossVariable(network_output, classnum); }

inline Var multiclass_loss(Var network_output, Var targets)
{ return new MulticlassLossVariable(network_output, targets); }

inline Var cross_entropy(Var network_output, Var targets)
{ return new CrossEntropyVariable(network_output, targets); }

inline Var reshape(Var v, int newlength, int newwidth)
{ return new ReshapeVariable(v,newlength,newwidth); }

inline Var operator+(Var v1, Var v2)
{ 
  if(v2->isScalar())
    return new PlusScalarVariable(v1,v2);
  else if(v1->isScalar())
    return new PlusScalarVariable(v2,v1);
  else if(v2->isRowVec())
    return new PlusRowVariable(v1,v2);
  else if(v1->isRowVec())
    return new PlusRowVariable(v2,v1);
  else if(v2->isColumnVec())
    return new PlusColumnVariable(v1,v2);
  else if(v1->isColumnVec())
    return new PlusColumnVariable(v2,v1);
  else
    return new PlusVariable(v1,v2);
}

inline void operator+=(Var& v1, const Var& v2)
{
  if (!v2.isNull())
  {
    if (v1.isNull())
      v1 = v2;
    else 
      v1 = v1 + v2;
  }
}

inline Var operator-(Var v1, Var v2)
{ 
  if(v2->isScalar())
    return new MinusScalarVariable(v1,v2);
  else if(v1->isScalar())
    return new PlusScalarVariable(new NegateElementsVariable(v2), v1);
  else if(v2->isRowVec())
    return new MinusRowVariable(v1,v2);
  else if(v1->isRowVec())
    return new NegateElementsVariable(new MinusRowVariable(v2,v1));
  else if(v2->isColumnVec())
    return new MinusColumnVariable(v1,v2);
  else if(v1->isColumnVec())
    return new PlusColumnVariable(new NegateElementsVariable(v2), v1);
  else
    return new MinusVariable(v1,v2);
}

inline Var operator-(Var v)
{ return new NegateElementsVariable(v); }

inline void operator-=(Var& v1, const Var& v2)
{
  if (!v2.isNull())
  {
    if (v1.isNull())
      v1 = -v2;
    else
      v1 = v1 - v2;
  }
}

inline Var operator+(Var v, real cte)
{ return new PlusConstantVariable(v,cte); }

inline Var operator+(real cte, Var v)
{ return new PlusConstantVariable(v,cte); }

inline Var operator-(Var v, real cte)
{ return new PlusConstantVariable(v,-cte); }

inline Var operator-(real cte, Var v)
{ return new PlusConstantVariable(new NegateElementsVariable(v),cte); }

//!  result[i] = 1 if v1[i]==cte, 0 otherwise
inline Var operator==(Var v1, real cte)
{  return new EqualConstantVariable(v1,cte); }

/*!   First case: v1 and v2 are two vectors of length() l
     resulting Var is 1 if for all i=0 to l-1,
     v1->value[i] == v2->value[i], 0 otherwise
  Second case: one of v1 or v2 is a scalar variable (length() 1)
  and the other is a vector of length() l
     resulting Var is a vector of length() l, doing an element-wise comparison
*/
inline Var isequal(Var v1, Var v2)
{
  if(v2->isScalar())
    return new EqualScalarVariable(v1,v2);
  else if(v1->isScalar())
    return new EqualScalarVariable(v2,v1);
  else
    return new EqualVariable(v1,v2);
}

//!  result[i] = 1 if v1[i]==cte, 0 otherwise
inline Var operator==(real cte, Var v1)
{  return new EqualConstantVariable(v1,cte); }

//!  result[i] = 1 if v1[i]!=cte, 0 otherwise
inline Var operator!=(Var v1, real cte)
{  return new UnequalConstantVariable(v1,cte); }

//!  result[i] = 1 if v1[i]!=cte, 0 otherwise
inline Var operator!=(real cte, Var v1)
{  return new UnequalConstantVariable(v1,cte); }

inline Var operator!=(Var v1, Var v2)
{ return (1.0 - isequal(v1,v2) ); }

//!  synomym
inline Var isdifferent(Var v1, Var v2)
{ return (1.0 - isequal(v1,v2) ); }

inline Var matrixInverse(Var v)
{
  return new MatrixInverseVariable(v);
}

inline Var leftPseudoInverse(Var v)
{
  return new LeftPseudoInverseVariable(v);
}

inline Var rightPseudoInverse(Var v)
{
  return new RightPseudoInverseVariable(v);
}

//!  dot product
inline Var dot(Var v1, Var v2)
{ return new DotProductVariable(v1,v2); }

//!  general matrix product
inline Var product(Var v1, Var v2)
{  return new ProductVariable(v1,v2); }

//!  element-wise multiplications
inline Var operator*(Var v1, Var v2)
{ 
  if(v2->isScalar())
    return new TimesScalarVariable(v1,v2);
  else if(v1->isScalar())
    return new TimesScalarVariable(v2,v1);
  else if(v2->isColumnVec())
    return new TimesColumnVariable(v1,v2);
  else if(v1->isColumnVec())
    return new TimesColumnVariable(v2,v1);
  else if(v2->isRowVec())
    return new TimesRowVariable(v1,v2);
  else if(v1->isRowVec())
    return new TimesRowVariable(v2,v1);
  else //!<  v1 and v2 must have the same dimensions (it is checked by the constructor of TimesVariable)
    return new TimesVariable(v1,v2); 
}

inline Var operator*(Var v, real cte)
{ return new TimesConstantVariable(v,cte); }

inline Var operator*(real cte, Var v)
{ return new TimesConstantVariable(v,cte); }

inline Var invertElements(Var v)
{ return new InvertElementsVariable(v); }

inline Var operator/(Var v, real cte)
{ return new TimesConstantVariable(v, 1.0/cte); }

inline Var operator/(real cte, Var v)
{
  if(cte==1.0)
    return invertElements(v);
  else
    return cte*invertElements(v);
}

inline Var operator/(Var v1, Var v2)
{
  if(v1->length()==v2->length() && v1->width()==v2->width())
    return new DivVariable(v1,v2);
  else
    return v1*invertElements(v2);
}

inline Var transpose(Var v)
{ return new SubMatTransposeVariable(v,0,0,v->length(),v->width()); }

inline Var tanh(Var v)
{ return new TanhVariable(v); }

inline Var sigmoid(Var v)
{ return new SigmoidVariable(v); }

inline Var softplus(Var v)
{ return new SoftplusVariable(v); }

inline Var square(Var v)
{ return new SquareVariable(v); }

inline Var sumsquare(Var v)
{ return new SumSquareVariable(v); }

inline Var weighted_sumsquare(Var v, Var w)
{ return new WeightedSumSquareVariable(v,w); }

inline Var squareroot(Var v)
{ return new SquareRootVariable(v);}

inline Var exp(Var v)
{ return new ExpVariable(v); }

inline Var log(Var v)
{ return new LogVariable(v); }

inline Var plogp(Var v)
{ return new PLogPVariable(v); }

inline Var pow(Var v, real power)
{ return new PowVariable(v,power); }

inline Var pow(Var v, Var power)
{ return new PowVariableVariable(v,power); }

inline Var sqrt(Var v)
{ return pow(v,0.5); }

inline Var sum(Var v)
{ 
  if (v->isScalar())
    return v;
  else
    return new SumVariable(v); 
}

inline Var columnSum(Var v)
{ 
  if(v->isRowVec())
    return v;
  else
    return new ColumnSumVariable(v); 
}

inline Var rowSum(Var v)
{ 
  if(v->isColumnVec())
    return v;
  else
    return new RowSumVariable(v); 
}

inline Var mean(Var v)
{ return sum(v)/v->nelems(); }

inline Var abs(Var v)
{ return new AbsVariable(v); }

inline Var min(Var v)
{ return new MinVariable(v); }

inline Var max(Var v)
{ return new MaxVariable(v); }

inline Var max(Var v1, Var v2)
{ return new Max2Variable(v1,v2); }

inline Var argmin(Var v)
{ return new ArgminVariable(v); }

inline Var argmax(Var v)
{ return new ArgmaxVariable(v); }

inline Var cutAboveThreshold(Var v, real threshold)
{ return new CutAboveThresholdVariable(v,threshold); }

inline Var cutBelowThreshold(Var v, real threshold)
{ return new CutBelowThresholdVariable(v,threshold); }

inline Var positive(Var v)
{ return cutBelowThreshold(v,0.0); }

inline Var negative(Var v)
{ return cutAboveThreshold(v,0.0); }

inline Var isAboveThreshold(Var v, real threshold=0, real truevalue=1, real falsevalue=0)
{ return new IsAboveThresholdVariable(v,threshold,truevalue,falsevalue); }

inline Var operator>=(Var v, real threshold)
{ return new IsAboveThresholdVariable(v, threshold, 1, 0); }

inline Var operator<=(Var v, real threshold)
{ return new IsAboveThresholdVariable(v, threshold, 0, 1); }

inline Var operator>(Var v1, Var v2)
{ return new IsLargerVariable(v1, v2); }

inline Var operator<(Var v1, Var v2)
{ return new IsSmallerVariable(v1, v2); }

inline Var onehot(int the_length, Var hotindex, real coldvalue=0.0, real hotvalue=1.0)
{ return new OneHotVariable(the_length, hotindex, coldvalue, hotvalue); }

inline Var det(Var m)
{ return new DeterminantVariable(m); }

///* OLD implementation 
inline Var old_softmax(Var input)
{ 
  //!  This is supposed to be numerically more stable, but is it really backpropable (it would be if dividing by a constnat, but with this max ?)
  Var maxin = max(input);
  Var exp_input = exp(input-maxin);
  return exp_input/sum(exp_input); 
}
//*/

// Now we have a special box!
inline Var softmax(Var v)
{
  return new SoftmaxVariable(v);
}

// Double special box!
inline Var log_softmax(Var v)
{
    // Returns a variable equivalent to -log(softmax(v))
    return new LogSoftmaxVariable(v);
}

/*!   a soft version of the ordinary max(x1,x2) mathematical operation
  where the hardness parameter controls how close to an actual max(x1,x2)
  we are (i.e. as hardness --> infty we quickly get max(x1,x2), but
  as hardness --> 0 we get the simple average of x1 and x2.
*/
inline Var softmax(Var x1, Var x2, Var hardness)
{
  Var w=sigmoid(hardness*(x1-x2));
  return x1*w + x2*(1-w);
}

inline Var softmax(Var input, Var index)
{ 
  if (input->isVec())
    return new SoftmaxLossVariable(input, index);
  else return new MatrixSoftmaxLossVariable(input, index);
}

inline Var neg_log_pi(Var p, Var index)
{
  return -log(p[index]);
}

inline Var softmax(Var input, int index)
{ 
  return 1.0/sum(exp(input-input[index])); //!<  should be numerically more stable
}

inline Var logadd(Var input)
{ return new LogSumVariable(input); }

inline Var logadd(Var input1, Var input2)
{ return new LogAddVariable(input1, input2); }

inline Var sign(Var input) { return new SignVariable(input); }

inline Var duplicateScalar(Var v, int the_length, int the_width)
{ 
  if(!v->isScalar())
    PLERROR("In duplicateScalar: v is not a scalar var");
  if(the_length*the_width==1)
    return v;
  else
    return new DuplicateScalarVariable(v,the_length,the_width); 
}

inline Var duplicateRow(Var v, int the_length)
{ 
  if(!v->isRowVec())
    PLERROR("In duplicateRow: v is not a single-row var");
  if(the_length==1)
    return v;
  else
    return new DuplicateRowVariable(v,the_length); 
}

inline Var duplicateColumn(Var v, int the_width)
{ 
  if(!v->isColumnVec())
    PLERROR("In duplicateColumn: v is not a single-column var");
  if(the_width==1)
    return v;
  else
    return new DuplicateColumnVariable(v,the_width); 
}

//!  general extension of a matrix in any direction
inline Var extend(Var v, int top_extent, int bottom_extent, int left_extent, int right_extent, real fill_value=0.0)
{ return new ExtendedVariable(v,top_extent,bottom_extent,left_extent,right_extent,fill_value); }

//!  simple extension of a vector (same semantic as old extend, when we only had vectors)
inline Var extend(Var v, real extension_value = 1.0, int n_extend = 1)
{ 
  if(v->isColumnVec())
    return new ExtendedVariable(v,0,n_extend,0,0,extension_value); 
  else if(v->isRowVec())
    return new ExtendedVariable(v,0,0,0,n_extend,extension_value); 
  PLERROR("In extend(Var v, real extension_value = 1.0, int n_extend = 1) v is not a vector (single row or single column)");
  return Var();
}

inline Var pownorm(Var input, real n=2.0)
{
  if(n==2.0)
    return sum(square(input));
  else if(n==1.0)
    return sum(abs(input));
  else
    return sum(pow(abs(input),n));
}

inline Var norm(Var input, real n=2.0)
{
  if(n==2.0)
    return sqrt(sum(square(input)));
  else if(n==1.0)
    return sum(abs(input));
  else
    return pow(sum(pow(abs(input),n)),1.0/n);
}

inline Var entropy(Var v, bool normalize = true)
{
  if(normalize)
    {
      Var absx = abs(v);
      Var normalized = absx/sum(absx);
      return sum(plogp(normalized))*(-1.0/log(2.0));
    }
  else
    return sum(plogp(v))*(-1.0/log(2.0));
}

//!  if values = [x1,x2,...,x10], the resulting variable is 
//!  [(x1+x2)/2,(x2+x3)/2, ... (x9+x10)/2]
inline Var interValues(Var values)
{ return new InterValuesVariable(values); }

inline Var distance(Var input1, Var input2, real n)
{ return norm(input1-input2, n); }

inline Var powdistance(Var input1, Var input2, real n)
{ return pownorm(input1-input2, n); }

inline Var productTranspose(Var& m1, Var& m2)
{
  return new ProductTransposeVariable(m1, m2);
}

inline Var transposeProduct(Var& m1, Var& m2)
{
  return new TransposeProductVariable(m1, m2);
}

inline Var convolve(Var input, Var mask)
{ return new ConvolveVariable(input, mask); }

inline Var subsample(Var input, int subsample_factor)
{ return new SubsampleVariable(input, subsample_factor); }

inline Var erf(Var v)
{ return new ErfVariable(v); }

inline Var accessElement(const Vec& v, Var index)
{ return new VecElementVariable(v,index); }

inline Var accessRow(const Mat& m, Var index)
{ return new MatRowVariable(m,index); }

inline Var vconcat(const VarArray& varray)
{ return new ConcatRowsVariable(varray); }

inline Var hconcat(const VarArray& varray)
{ return new ConcatColumnsVariable(varray); }

//!  sumOf

inline Var sumOf(VMat distr, Func f, int nsamples)
{ return new SumOfVariable(distr,f,nsamples); }

//!  deprecated old version do not use!
inline Var sumOf(Var output, const VarArray& inputs, VMat distr, int nsamples, VarArray parameters=VarArray())
{ return sumOf(distr,Func(inputs,output),nsamples); }

//!  meanOf

inline Var meanOf(VMat distr, Func f, int nsamples)
{ return new SumOfVariable(distr,f/nsamples,nsamples); }

//!  deprecated old version do not use!
inline Var meanOf(Var output, const VarArray& inputs, VMat distr, int nsamples, VarArray parameters=VarArray())
{ return meanOf(distr, Func(inputs,output), nsamples); }

//!  concatOf

inline Var concatOf(VMat distr, Func f)
{ return new ConcatOfVariable(distr,f); }

//!  deprecated old version, do not use!
inline Var concatOf(Var output, const VarArray& inputs, VMat distr, int nsamples, VarArray parameters=VarArray())
{ return concatOf(distr,Func(inputs,output)); }


/*! returns the value of v within the_values_of_v that gives the lowest
   value of expression (which may depend on inputs). */
inline Var argminOf(Var v, Var expression, Var values_of_v, VarArray inputs)
{ return new ArgminOfVariable(v, expression, values_of_v, inputs); }

inline Var matrixElements(Var expression, const Var& i, const Var& j,
                             int ni, int nj, const VarArray& parameters)
{ return new MatrixElementsVariable(expression, i, j, ni, nj, parameters); }

//!  IT WOULD BE NICE IF WE COULD REDEFINE (:?)
inline Var ifThenElse(Var If, Var Then, Var Else)
{ return new IfThenElseVariable(If,Then,Else); }

template <> void deepCopyField(Var& field, CopiesMap& copies);

%> // end of namespace PLearn

#endif

