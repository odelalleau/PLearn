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
   * $Id: UnaryVariable.cc,v 1.6 2002/09/11 04:25:48 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "DisplayUtils.h"
#include "UnaryVariable.h"
#include "Var.h"
#include "TMat_maths.h"
#include "pl_erf.h"

namespace PLearn <%
using namespace std;



IMPLEMENT_NAME_AND_DEEPCOPY(AffineTransformWeightPenalty);

/** UnaryVariable **/

UnaryVariable::UnaryVariable(Variable* v, int thelength, int thewidth)
  : Variable(thelength,thewidth), input(v) 
{}

IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY(UnaryVariable);

void UnaryVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "UnaryVariable");
  Variable::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, input);
  readFooter(in, "UnaryVariable");
}

void UnaryVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "UnaryVariable");
  Variable::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, input);
  writeFooter(out, "UnaryVariable");
}

void UnaryVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Variable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(input, copies);
}

bool UnaryVariable::markPath()
{
  if(!marked)
    marked = input->markPath();
  return marked;
}

void UnaryVariable::buildPath(VarArray& proppath)
{
  if(marked)
    {
      input->buildPath(proppath);
      //cout<<"add :"<<this->getName()<<endl;
      proppath.append(Var(this));
      clearMark();
    }
}

VarArray UnaryVariable::sources() 
{ 
  if (marked)
    return VarArray(0,0);
  marked = true;
  return input->sources(); 
}

VarArray UnaryVariable::random_sources() 
{ 
  if (marked)
    return VarArray(0,0);
  marked = true;
  return input->random_sources(); 
}

VarArray UnaryVariable::ancestors() 
{ 
  if (marked)
    return VarArray(0,0);
  marked = true;
  return input->ancestors() & (VarArray)Var(this);
}

void UnaryVariable::unmarkAncestors()
{ 
  if (marked)
    {
      marked = false;
      input->unmarkAncestors();
    }
}

VarArray UnaryVariable::parents() 
{ 
  if (input->marked)
    return VarArray(0,0);
  else
    return input;
}

void UnaryVariable::resizeRValue()
{
  inherited::resizeRValue();
  if (!input->rvaluedata) input->resizeRValue();
}

/** ReshapeVariable **/

ReshapeVariable::ReshapeVariable(Variable* v, int the_length, int the_width)
  :UnaryVariable(v, the_length, the_width), length_(the_length), width_(the_width)
{
  if(length()*width() != input.length()*input.width())
    PLERROR("In ReshapeVariable: length()*width() is different from the original var's length()*width()");
}

IMPLEMENT_NAME_AND_DEEPCOPY(ReshapeVariable);

void ReshapeVariable::recomputeSize(int& l, int& w) const
{ l=length_; w=width_; }

void ReshapeVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ReshapeVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ReshapeVariable");
}

void ReshapeVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ReshapeVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ReshapeVariable");
}

void ReshapeVariable::fprop()
{
  for(int k=0; k<nelems(); k++)
    valuedata[k] = input->valuedata[k];
}

void ReshapeVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    input->gradientdata[k] += gradientdata[k];
}

void ReshapeVariable::symbolicBprop()
{
  input->accg(new ReshapeVariable(g,input->length(),input->width()));
}

/** SubMatVariable **/

SubMatVariable::SubMatVariable(Variable* v, int i, int j, int the_length, int the_width)
  :UnaryVariable(v, the_length, the_width), startk(i*v->width()+j), length_(the_length), width_(the_width)
{
  if(i<0 || i+length()>v->length() || j<0 || j+width()>v->width())
    PLERROR("In SubMatVariable: requested sub-matrix is out of matrix bounds");
}

IMPLEMENT_NAME_AND_DEEPCOPY(SubMatVariable);

void SubMatVariable::recomputeSize(int& l, int& w) const
{ l=length_; w=width_; }

void SubMatVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "SubMatVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, startk);
  readFooter(in, "SubMatVariable");
}

void SubMatVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "SubMatVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, startk);
  writeFooter(out, "SubMatVariable");
}


void SubMatVariable::fprop()
{
  if(width()==input->width()) // optimized version for this special case
    {
      real* inputdata = input->valuedata+startk;
      for(int k=0; k<nelems(); k++)
        valuedata[k] = inputdata[k];
    }
  else // general version
    {
      real* inputrowdata = input->valuedata+startk;
      int thisk=0;
      for(int i=0; i<length(); i++)
        {
          for(int j=0; j<width(); j++)
            valuedata[thisk++] = inputrowdata[j];
          inputrowdata += input->width();
        }
    }
}

void SubMatVariable::bprop()
{
  if(width()==input->width()) // optimized version for this special case
    {
      real* inputgradient = input->gradientdata+startk;
      for(int k=0; k<nelems(); k++)
        inputgradient[k] += gradientdata[k]; 
    }
  else // general version
    {
      real* inputrowgradient = input->gradientdata+startk;
      int thisk=0;
      for(int i=0; i<length(); i++)
        {
          for(int j=0; j<width(); j++)
            inputrowgradient[j] += gradientdata[thisk++];
          inputrowgradient += input->width();
        }
    }
}

void SubMatVariable::bbprop()
{
  if (input->diaghessian.length()==0)
    input->resizeDiagHessian();
  if(width()==input->width()) // optimized version for this special case
    {
      real* inputdiaghessian = input->diaghessian.data()+startk;
      for(int k=0; k<nelems(); k++)
        inputdiaghessian[k] += diaghessiandata[k]; 
    }
  else // general version
    {
      real* inputrowdiaghessian = input->diaghessiandata+startk;
      int thisk=0;
      for(int i=0; i<length(); i++)
        {
          for(int j=0; j<width(); j++)
            inputrowdiaghessian[j] += diaghessiandata[thisk++];
          inputrowdiaghessian += input->width();
        }
    }
}

void SubMatVariable::symbolicBprop()
{
  int i = startk/input->width();
  int j = startk%input->width();
  int topextent = i;
  int bottomextent = input->length()-(i+length());
  int leftextent = j;
  int rightextent = input->width()-(j+width());
  input->accg(extend(g,topextent,bottomextent,leftextent,rightextent));
}

void SubMatVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  if(width()==input->width()) // optimized version for this special case
    {
      real* inputrdata = input->rvaluedata+startk;
      for(int k=0; k<nelems(); k++)
        rvaluedata[k] = inputrdata[k];
    }
  else // general version
    {
      real* inputrowrdata = input->rvaluedata+startk;
      int thisk=0;
      for(int i=0; i<length(); i++)
        {
          for(int j=0; j<width(); j++)
            rvaluedata[thisk++] = inputrowrdata[j];
          inputrowrdata += input->width();
        }
    }
}

/** SubMatTransposeVariable **/

SubMatTransposeVariable::SubMatTransposeVariable(Variable* v, int i, int j, int the_length, int the_width)
  :UnaryVariable(v, the_width, the_length), startk(i*v->length()+j), length_(the_length), width_(the_width)
{
  if(i<0 || i+the_length>v->length() || j<0 || j+the_width>v->width())
    PLERROR("In SubMatTransposeVariable: requested sub-matrix is out of matrix bounds");
}

IMPLEMENT_NAME_AND_DEEPCOPY(SubMatTransposeVariable);

void SubMatTransposeVariable::recomputeSize(int& l, int& w) const
{ l=width_; w=length_; }

void SubMatTransposeVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "SubMatTransposeVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, startk);
  readFooter(in, "SubMatTransposeVariable");
}

void SubMatTransposeVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "SubMatTransposeVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, startk);
  writeFooter(out, "SubMatTransposeVariable");
}

void SubMatTransposeVariable::fprop()
{
  if(input->length()==1 || input->width()==1) // optimized version for this special case...
    {
      real* inputdata = input->valuedata+startk;
      for(int k=0; k<nelems(); k++)
        valuedata[k] = inputdata[k];
    }
  else // general case
    {
      real* inputrowdata = input->valuedata+startk;
      int thiskcolstart = 0; // element index of start of column in this var
      for(int i=0; i<width(); i++) // the width() of this var is the length() of the submat
        {
          int thisk = thiskcolstart++;
          for(int j=0; j<length(); j++, thisk+=width()) // the length() of this var is the width() of the submat
            valuedata[thisk] = inputrowdata[j];
          inputrowdata += input->width();
        }
    }
}

void SubMatTransposeVariable::bprop()
{
  if(input->length()==1 || input->width()==1) // optimized version for this special case...
    {
      real* inputdata = input->gradientdata+startk;
      for(int k=0; k<nelems(); k++)
        inputdata[k] += gradientdata[k];
    }
  else // general case
    {
      real* inputrowdata = input->gradientdata+startk;
      int thiskcolstart = 0; // element index of start of column in this var
      for(int i=0; i<width(); i++) // the width() of this var is the length() of the submat
        {
          int thisk = thiskcolstart++;
          for(int j=0; j<length(); j++, thisk+=width()) // the length() of this var is the width() of the submat
            inputrowdata[j] += gradientdata[thisk];
          inputrowdata += input->width();
        }
    }
}

void SubMatTransposeVariable::symbolicBprop()
{
  int i = startk/input->width();
  int j = startk%input->width();
  int topextent = i;
  int bottomextent = input->length()-(i+width()); // the width() of this var is the length() of the submat
  int leftextent = j;
  int rightextent = input->width()-(j+length()); // the length() of this var is the width() of the submat
  input->accg(extend(transpose(g),topextent,bottomextent,leftextent,rightextent));
}

void SubMatTransposeVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  if(input->length()==1 || input->width()==1) // optimized version for this special case...
    {
      real* inputdata = input->rvaluedata+startk;
      for(int k=0; k<nelems(); k++)
        rvaluedata[k] = inputdata[k];
    }
  else // general case
    {
      real* inputrowdata = input->rvaluedata+startk;
      int thiskcolstart = 0; // element index of start of column in this var
      for(int i=0; i<width(); i++) // the width() of this var is the length() of the submat
        {
          int thisk = thiskcolstart++;
          for(int j=0; j<length(); j++, thisk+=width()) // the length() of this var is the width() of the submat
            rvaluedata[thisk] = inputrowdata[j];
          inputrowdata += input->width();
        }
    }
}

/** MatRowVariable **/

MatRowVariable::MatRowVariable(const Mat& mat, Variable* input)
  : UnaryVariable(input, mat.width(), 1), m(mat)
{
  if(!input->isScalar())
    PLERROR("IN MatRowVariable(const Mat& mat, Variable* input) input must have nelems() 1 as it is supposed to be an integer index");
}

IMPLEMENT_NAME_AND_DEEPCOPY(MatRowVariable);

void MatRowVariable::recomputeSize(int& l, int& w) const
{ l=m.width(); w=1; }

void MatRowVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MatRowVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, m);
  readFooter(in, "MatRowVariable");
}

void MatRowVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MatRowVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, m);
  writeFooter(out, "MatRowVariable");
}

void MatRowVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  UnaryVariable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(m, copies);
}


void MatRowVariable::fprop()
{  value << m(int(input->value[0])); }

void MatRowVariable::bprop() {}
void MatRowVariable::symbolicBprop() {}

/** VecElementVariable **/

VecElementVariable::VecElementVariable(const Vec& vec, Variable* input)
  : UnaryVariable(input, 1,1), v(vec)
{
  if(!input->isScalar())
    PLERROR("IN VecElementVariable(const Vec& vec, Variable* input) input must have nelems() 1 as it is supposed to be an integer index");
}

IMPLEMENT_NAME_AND_DEEPCOPY(VecElementVariable);
void VecElementVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }

void VecElementVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "VecElementVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, v);
  readFooter(in, "VecElementVariable");
}

void VecElementVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "VecElementVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, v);
  writeFooter(out, "VecElementVariable");
}

void VecElementVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  UnaryVariable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(v, copies);
}


void VecElementVariable::fprop()
{  valuedata[0] = v[int(input->value[0])]; }

void VecElementVariable::bprop() {}
void VecElementVariable::symbolicBprop() {}


/** ExtendedVariable **/

ExtendedVariable::ExtendedVariable(Variable* input, int the_top_extent,
  int the_bottom_extent, int the_left_extent, int the_right_extent, real the_fill_value)
  :UnaryVariable(input, input->length()+the_top_extent+the_bottom_extent, input->width()+the_left_extent+the_right_extent),
   top_extent(the_top_extent), bottom_extent(the_bottom_extent), 
   left_extent(the_left_extent), right_extent(the_right_extent),
   fill_value(the_fill_value)
{
  if(top_extent<0 || bottom_extent<0 || left_extent<0 || right_extent<0)
    PLERROR("In ExtendedVariable: given extents must be >=0");
  for(int k=0; k<nelems(); k++)
    valuedata[k] = fill_value;
}

IMPLEMENT_NAME_AND_DEEPCOPY(ExtendedVariable);
void ExtendedVariable::recomputeSize(int& l, int& w) const
{ l=input->length()+top_extent+bottom_extent; w=input->width()+left_extent+right_extent; }

void ExtendedVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ExtendedVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, top_extent);
  PLearn::deepRead(in, old2new, bottom_extent);
  PLearn::deepRead(in, old2new, left_extent);
  PLearn::deepRead(in, old2new, right_extent);
  PLearn::deepRead(in, old2new, fill_value);
  readFooter(in, "ExtendedVariable");
}

void ExtendedVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ExtendedVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, top_extent);
  PLearn::deepWrite(out, already_saved, bottom_extent);
  PLearn::deepWrite(out, already_saved, left_extent);
  PLearn::deepWrite(out, already_saved, right_extent);
  PLearn::deepWrite(out, already_saved, fill_value);
  writeFooter(out, "ExtendedVariable");
}

void ExtendedVariable::fprop()
{
  if(width()==input->width()) // optimized code for this special case (no left or right extents)
    {
      real* data = valuedata+top_extent*width();
      for(int k=0; k<input->nelems(); k++)
        data[k] = input->valuedata[k];
    }
  else // general case
    {
      real* rowdata=valuedata+top_extent*width()+left_extent;
      int inputk=0;
      for(int i=0; i<input->length(); i++)
        {
          for(int j=0; j<input->width(); j++)
            rowdata[j] = input->valuedata[inputk++];
          rowdata += width();
        }
    }
}

void ExtendedVariable::bprop()
{
  if(width()==input->width()) // optimized code for this special case (no left or right extents)
    {
      real* data = gradientdata+top_extent*width();
      for(int k=0; k<input->nelems(); k++)
        input->gradientdata[k] += data[k];
    }
  else // general case
    {
      real* rowdata = gradientdata+top_extent*width()+left_extent;
      int inputk = 0;
      for(int i=0; i<input->length(); i++)
        {
          for(int j=0; j<input->width(); j++)
            input->gradientdata[inputk++] += rowdata[j]; 
          rowdata += width();
        }
    }
}

void ExtendedVariable::symbolicBprop()
{
  input->accg(new SubMatVariable(g,top_extent,left_extent,input->length(),input->width()));
}

void ExtendedVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  if(width()==input->width()) // optimized code for this special case (no left or right extents)
    {
      real* data = rvaluedata+top_extent*width();
      for(int k=0; k<input->nelems(); k++)
        data[k] = input->rvaluedata[k];
    }
  else // general case
    {
      real* rowdata=rvaluedata+top_extent*width()+left_extent;
      int inputk=0;
      for(int i=0; i<input->length(); i++)
        {
          for(int j=0; j<input->width(); j++)
            rowdata[j] = input->rvaluedata[inputk++];
          rowdata += width();
        }
    }
}

/** DuplicateScalarVariable **/

DuplicateScalarVariable::DuplicateScalarVariable(Variable* input, int thelength, int thewidth)
  :UnaryVariable(input, thelength, thewidth), length_(thelength), width_(thewidth)
{
  if (!input->isScalar())
    PLERROR("In DuplicateScalarVariable input is not a scalar");
}

IMPLEMENT_NAME_AND_DEEPCOPY(DuplicateScalarVariable);
void DuplicateScalarVariable::recomputeSize(int& l, int& w) const
{ l=length_; w=width_; }

void DuplicateScalarVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "DuplicateScalarVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "DuplicateScalarVariable");
}

void DuplicateScalarVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "DuplicateScalarVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "DuplicateScalarVariable");
}

void DuplicateScalarVariable::fprop()
{
  real val = input->valuedata[0];
  for(int k=0; k<nelems(); k++)
    valuedata[k] = val;
}

void DuplicateScalarVariable::bprop()
{
  real& inputgrad = input->gradientdata[0];
  for(int k=0; k<nelems(); k++)
    inputgrad += gradientdata[k];
}

void DuplicateScalarVariable::symbolicBprop()
{
  input->accg(sum(g));
}

/** DuplicateRowVariable **/

DuplicateRowVariable::DuplicateRowVariable(Variable* input, int thelength)
  :UnaryVariable(input, thelength, input->width()), length_(thelength)
{
  if (!input->isRowVec())
    PLERROR("In DuplicateRowVariable input is not a row");
}

IMPLEMENT_NAME_AND_DEEPCOPY(DuplicateRowVariable);
void DuplicateRowVariable::recomputeSize(int& l, int& w) const
{ l=length_; w=input->width(); }

void DuplicateRowVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "DuplicateRowVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, n_duplicates);
  readFooter(in, "DuplicateRowVariable");
}

void DuplicateRowVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "DuplicateRowVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, n_duplicates);
  writeFooter(out, "DuplicateRowVariable");
}

void DuplicateRowVariable::fprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      valuedata[k] = input->valuedata[j];
}

void DuplicateRowVariable::bprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      input->gradientdata[j] += gradientdata[k];
}

void DuplicateRowVariable::symbolicBprop()
{
  input->accg(columnSum(g));
}

/** DuplicateColumnVariable **/

DuplicateColumnVariable::DuplicateColumnVariable(Variable* input, int thewidth)
  :UnaryVariable(input, input->length(), thewidth), width_(thewidth)
{
  if (!input->isColumnVec())
    PLERROR("In DuplicateColumnVariable input is not a column");
}

IMPLEMENT_NAME_AND_DEEPCOPY(DuplicateColumnVariable);
void DuplicateColumnVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=width_; }

void DuplicateColumnVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "DuplicateColumnVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, n_duplicates);
  readFooter(in, "DuplicateColumnVariable");
}

void DuplicateColumnVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "DuplicateColumnVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, n_duplicates);
  writeFooter(out, "DuplicateColumnVariable");
}

void DuplicateColumnVariable::fprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      valuedata[k] = input->valuedata[i];
}

void DuplicateColumnVariable::bprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      input->gradientdata[i] += gradientdata[k];
}

void DuplicateColumnVariable::symbolicBprop()
{
  input->accg(rowSum(g));
}


/** SumVariable **/

SumVariable::SumVariable(Variable* input)
  :UnaryVariable(input, 1, 1) {}

IMPLEMENT_NAME_AND_DEEPCOPY(SumVariable);
void SumVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }

void SumVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "SumVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "SumVariable");
}

void SumVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "SumVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "SumVariable");
}

void SumVariable::fprop()
{
  real sum = 0.0;
  for(int k=0; k<input->nelems(); k++)
    sum += input->valuedata[k];
  valuedata[0] = sum;
}

void SumVariable::bprop()
{
  real grd = gradientdata[0];
  for(int k=0; k<input->nelems(); k++)
    input->gradientdata[k] += grd;
}

void SumVariable::symbolicBprop()
{
  input->accg(g);
}

// R(sum_i x_i) = sum_i R(x_i)
void SumVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  real sum = 0.0;
  for(int k=0; k<input->nelems(); k++)
    sum += input->rvaluedata[k];
  rvaluedata[0] = sum;
}

/** ColumnSumVariable **/

ColumnSumVariable::ColumnSumVariable(Variable* input)
  :UnaryVariable(input, 1, input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(ColumnSumVariable);
void ColumnSumVariable::recomputeSize(int& l, int& w) const
{ l=1; w=input->width(); }

void ColumnSumVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ColumnSumVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ColumnSumVariable");
}

void ColumnSumVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ColumnSumVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ColumnSumVariable");
}

void ColumnSumVariable::fprop()
{
  value.clear();
  int k=0;
  for(int i=0; i<input->length(); i++)
    for(int j=0; j<input->width(); j++, k++)
      valuedata[j] += input->valuedata[k];
}

void ColumnSumVariable::bprop()
{
  int k=0;
  for(int i=0; i<input->length(); i++)
    for(int j=0; j<input->width(); j++, k++)
      input->gradientdata[k] += gradientdata[j];
}

void ColumnSumVariable::symbolicBprop()
{
  input->accg(g);
}

/** RowSumVariable **/

RowSumVariable::RowSumVariable(Variable* input)
  :UnaryVariable(input, input->length(), 1) {}

IMPLEMENT_NAME_AND_DEEPCOPY(RowSumVariable);
void RowSumVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=1; }

void RowSumVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "RowSumVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "RowSumVariable");
}

void RowSumVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "RowSumVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "RowSumVariable");
}

void RowSumVariable::fprop()
{
  value.clear();
  int k=0;
  for(int i=0; i<input->length(); i++)
    for(int j=0; j<input->width(); j++, k++)
      valuedata[i] += input->valuedata[k];
}

void RowSumVariable::bprop()
{
  int k=0;
  for(int i=0; i<input->length(); i++)
    for(int j=0; j<input->width(); j++, k++)
      input->gradientdata[k] += gradientdata[i];
}

void RowSumVariable::symbolicBprop()
{
  input->accg(g);
}

/** PlusConstantVariable **/

string PlusConstantVariable::info() const
{ return string("PlusConstant (+ ")+tostring(cst)+")"; }

PlusConstantVariable::PlusConstantVariable(Variable* input, real c)
  :UnaryVariable(input, input->length(), input->width()), cst(c) 
{}

IMPLEMENT_NAME_AND_DEEPCOPY(PlusConstantVariable);
void PlusConstantVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void PlusConstantVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "PlusConstantVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, cst);
  readFooter(in, "PlusConstantVariable");
}

void PlusConstantVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "PlusConstantVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, cst);
  writeFooter(out, "PlusConstantVariable");
}

void PlusConstantVariable::fprop()
{
  for(int k=0; k<nelems(); k++)
    valuedata[k] = input->valuedata[k] + cst;
}

void PlusConstantVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    input->gradientdata[k] += gradientdata[k];
}

void PlusConstantVariable::symbolicBprop()
{
  input->accg(g);
}

// R(x+c) = R(x)
void PlusConstantVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  for(int k=0; k<nelems(); k++)
    rvaluedata[k] = input->rvaluedata[k];
}

/** TimesConstantVariable **/

string TimesConstantVariable::info() const
{ return string("TimesConstant (* ")+tostring(cst)+")"; }

TimesConstantVariable::TimesConstantVariable(Variable* input, real c)
  :UnaryVariable(input, input->length(), input->width()), cst(c) 
{}

IMPLEMENT_NAME_AND_DEEPCOPY(TimesConstantVariable);
void TimesConstantVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void TimesConstantVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "TimesConstantVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, cst);
  readFooter(in, "TimesConstantVariable");
}

void TimesConstantVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "TimesConstantVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, cst);
  writeFooter(out, "TimesConstantVariable");
}

void TimesConstantVariable::fprop()
{
  for(int k=0; k<nelems(); k++)
    valuedata[k] = input->valuedata[k] * cst;
}

void TimesConstantVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    input->gradientdata[k] += cst*gradientdata[k];
}

void TimesConstantVariable::symbolicBprop()
{
  input->accg(g*cst);
}

// R(cx) = cR(x)
void TimesConstantVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  for(int k=0; k<nelems(); k++)
    rvaluedata[k] = input->rvaluedata[k]*cst;
}

/** NegateElementsVariable **/

NegateElementsVariable::NegateElementsVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(NegateElementsVariable);
void NegateElementsVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void NegateElementsVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "NegateElementsVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "NegateElementsVariable");
}

void NegateElementsVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "NegateElementsVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "NegateElementsVariable");
}

void NegateElementsVariable::fprop()
{
  for(int k=0; k<nelems(); k++)
    valuedata[k] = -input->valuedata[k];
}

void NegateElementsVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    input->gradientdata[k] -= gradientdata[k];
}

void NegateElementsVariable::bbprop()
{
  if (input->diaghessian.length()==0)
    input->resizeDiagHessian();
  for(int k=0; k<nelems(); k++)
    input->diaghessiandata[k] -= diaghessiandata[k];
}

void NegateElementsVariable::symbolicBprop()
{
  input->accg(-g);
}

// R(-x) = -R(x)
void NegateElementsVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  for(int k=0; k<nelems(); k++)
    rvaluedata[k] = -input->rvaluedata[k];
}

/** InvertElementsVariable **/

InvertElementsVariable::InvertElementsVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(InvertElementsVariable);
void InvertElementsVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void InvertElementsVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "InvertElementsVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "InvertElementsVariable");
}

void InvertElementsVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "InvertElementsVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "InvertElementsVariable");
}

void InvertElementsVariable::fprop()
{
  for(int k=0; k<nelems(); k++)
    valuedata[k] = 1.0/input->valuedata[k];
}

void InvertElementsVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    {
      real inputvalue_k = input->valuedata[k];
      input->gradientdata[k] -= gradientdata[k]/(inputvalue_k*inputvalue_k);
    }
}

void InvertElementsVariable::symbolicBprop()
{
  Var v = input * input;
  input->accg(-g/v);
}

// R(1/x) = -1/(x^2) R(x)
void InvertElementsVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  for(int k=0; k<nelems(); k++)
    {
    real value_k = valuedata[k];
    rvaluedata[k] = - input->rvaluedata[k] * value_k * value_k;
    }
}

/** MatrixInverseVariable **/

MatrixInverseVariable::MatrixInverseVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) 
{}

IMPLEMENT_NAME_AND_DEEPCOPY(MatrixInverseVariable);
void MatrixInverseVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void MatrixInverseVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MatrixInverseVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "MatrixInverseVariable");
}

void MatrixInverseVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MatrixInverseVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MatrixInverseVariable");
}

void MatrixInverseVariable::fprop()
{
  inverse(input->matValue,matValue);
}

void MatrixInverseVariable::bprop()
{
  PLERROR("MatrixInverseVariable: bprop not implemented yet");
}

void MatrixInverseVariable::symbolicBprop()
{
  PLERROR("MatrixInverseVariable: symbolicBprop not implemented yet");
}

/** LeftPseudoInverseVariable **/

LeftPseudoInverseVariable::LeftPseudoInverseVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) 
{
  if (input->width() < input->length())
    PLERROR("LeftPseudoInverseVariable(Var): input width_(%d) < length_(%d)",
          input->width(), input->length());
}

IMPLEMENT_NAME_AND_DEEPCOPY(LeftPseudoInverseVariable);
void LeftPseudoInverseVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void LeftPseudoInverseVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "LeftPseudoInverseVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "LeftPseudoInverseVariable");
}

void LeftPseudoInverseVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "LeftPseudoInverseVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "LeftPseudoInverseVariable");
}

void LeftPseudoInverseVariable::fprop()
{
  leftPseudoInverse(input->matValue,matValue);
}

void LeftPseudoInverseVariable::bprop()
{
  PLERROR("LeftPseudoInverseVariable: bprop not implemented yet");
}

void LeftPseudoInverseVariable::symbolicBprop()
{
  PLERROR("LeftPseudoInverseVariable: symbolicBprop not implemented yet");
}

/** RightPseudoInverseVariable **/

RightPseudoInverseVariable::RightPseudoInverseVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) 
{
  if (input->width() > input->length())
    PLERROR("RightPseudoInverseVariable(Var): input width_(%d) > length_(%d)",
          input->width(), input->length());
}

IMPLEMENT_NAME_AND_DEEPCOPY(RightPseudoInverseVariable);
void RightPseudoInverseVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void RightPseudoInverseVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "RightPseudoInverseVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "RightPseudoInverseVariable");
}

void RightPseudoInverseVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "RightPseudoInverseVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "RightPseudoInverseVariable");
}

void RightPseudoInverseVariable::fprop()
{
  rightPseudoInverse(input->matValue,matValue);
}

void RightPseudoInverseVariable::bprop()
{
  PLERROR("RightPseudoInverseVariable: bprop not implemented yet");
}

void RightPseudoInverseVariable::symbolicBprop()
{
  PLERROR("RightPseudoInverseVariable: symbolicBprop not implemented yet");
}

/** TanhVariable **/

TanhVariable::TanhVariable(Variable* input) 
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(TanhVariable);
void TanhVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void TanhVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "TanhVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "TanhVariable");
}

void TanhVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "TanhVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "TanhVariable");
}

void TanhVariable::fprop()
{
  int l = nelems();
  real* inputptr = input->valuedata;
  real* ptr = valuedata;
  for(int i=0; i<l; i++)
    *ptr++ = tanh(*inputptr++);
}

void TanhVariable::bprop()
{
  int l = nelems();
  real* inputgradientptr = input->gradientdata;
  real* gradientptr = gradientdata;
  real* valueptr = valuedata;
  for(int i=0; i<l; i++)
    *inputgradientptr++ += *gradientptr++ * (1.0-square(*valueptr++));
}

void TanhVariable::bbprop()
{
  if (input->diaghessian.length()==0)
    input->resizeDiagHessian();
  for(int i=0; i<nelems(); i++)
    {
      real yi=valuedata[i];
      real fprime=(1-yi*yi);
      input->diaghessiandata[i] += diaghessiandata[i] * fprime * fprime;
    }
}

void TanhVariable::symbolicBprop()
{
  Var v(this);
  input->accg(g * (1. - square(v)));
}

// R(tanh(x)) = (1-tanh(x)^2)R(x)
void TanhVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  int l = nelems();
  real* inputptr = input->rvaluedata;
  real* valueptr = valuedata;
  real* ptr = rvaluedata;
  for(int i=0; i<l; i++)
    *ptr++ = *inputptr++ * (1.0 - square(*valueptr++));
}

/** SigmoidVariable **/

SigmoidVariable::SigmoidVariable(Variable* input) 
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(SigmoidVariable);
void SigmoidVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void SigmoidVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "SigmoidVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "SigmoidVariable");
}

void SigmoidVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "SigmoidVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "SigmoidVariable");
}

void SigmoidVariable::fprop()
{
  int l = nelems();
  real* valueptr = valuedata;
  real* inputvalueptr = input->valuedata;
  for(int i=0; i<l; i++)
    *valueptr++ = sigmoid(*inputvalueptr++);
}

void SigmoidVariable::bprop()
{
  int l = nelems();
  real* inputgradientptr = input->gradientdata;
  real* gradientptr = gradientdata;
  real* valueptr = valuedata;
  for(int i=0; i<l; i++)
    {
      real val = *valueptr++;
      *inputgradientptr++ += *gradientptr++ * val*(1.0-val);
    }
}

void SigmoidVariable::bbprop()
{
  if (input->diaghessian.length()==0)
    input->resizeDiagHessian();
  for(int i=0; i<nelems(); i++)
  {
    real yi = valuedata[i];
    real fprime = yi*(1-yi);
    input->gradientdata[i] += gradientdata[i] * fprime * fprime;
  }
}

void SigmoidVariable::symbolicBprop()
{
  Var v(this);
  input->accg(g*v*(1. - v));
}

// R{sigmoid(x)} = f(x)(1-f(x))R(x)
void SigmoidVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  int l = nelems();
  real* inputptr = input->rvaluedata;
  real* inputvalueptr = valuedata;
  real* ptr = rvaluedata;
  for(int i=0; i<l; i++)
  {
    real val = *inputvalueptr++;
    *ptr++ = *inputptr++ * val * (1.0 - val);
  }
}

/** SoftplusVariable **/

SoftplusVariable::SoftplusVariable(Variable* input) 
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(SoftplusVariable);
void SoftplusVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void SoftplusVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "SoftplusVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "SoftplusVariable");
}

void SoftplusVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "SoftplusVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "SoftplusVariable");
}

void SoftplusVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    valuedata[i] = softplus(input->valuedata[i]);
}

void SoftplusVariable::bprop()
{
  for(int i=0; i<nelems(); i++)
    input->gradientdata[i] += gradientdata[i] * sigmoid(input->valuedata[i]);
}

void SoftplusVariable::symbolicBprop()
{
  Var v(this);
  input->accg(g*sigmoid(input));
}

/** SquareVariable **/

SquareVariable::SquareVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(SquareVariable);
void SquareVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void SquareVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "SquareVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "SquareVariable");
}

void SquareVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "SquareVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "SquareVariable");
}

void SquareVariable::fprop()
{
  int n=nelems();
  for(int i=0; i<n; i++)
    valuedata[i] = input->valuedata[i]*input->valuedata[i];
}

void SquareVariable::bprop()
{
  int n=nelems();
  for(int i=0; i<n; i++)
    input->gradientdata[i] += 2.0 * input->valuedata[i] * gradientdata[i];
}

void SquareVariable::bbprop()
{
  if (input->diaghessian.length()==0)
    input->resizeDiagHessian();
  int n=nelems();
  for(int i=0; i<n; i++)
  {
    real input_i = input->valuedata[i];
    input->diaghessiandata[i] += 4.0 * input_i * input_i * diaghessiandata[i]
      + 2.0 * gradientdata[i];
  }
}

void SquareVariable::symbolicBprop()
{
  input->accg(2. * (g * input));
}

void SquareVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  int n=nelems();
  for(int i=0; i<n; i++)
    rvaluedata[i] = 2*input->valuedata[i]*input->rvaluedata[i];
}

/** SumSquareVariable **/

SumSquareVariable::SumSquareVariable(Variable* input)
  :UnaryVariable(input, 1, 1) {}

IMPLEMENT_NAME_AND_DEEPCOPY(SumSquareVariable);
void SumSquareVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }

void SumSquareVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "SumSquareVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "SumSquareVariable");
}

void SumSquareVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "SumSquareVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "SumSquareVariable");
}

void SumSquareVariable::fprop()
{
  int n=input->nelems();
  *valuedata= 0;
  for(int i=0; i<n; i++)
    *valuedata+= input->valuedata[i]*input->valuedata[i];
}

void SumSquareVariable::bprop()
{
  int n=input->nelems();
  for(int i=0; i<n; i++)
    input->gradientdata[i]+= 2.0 * input->valuedata[i] * *gradientdata;
}

void SumSquareVariable::symbolicBprop()
{
  input->accg(2.0 * (g*input));
}


/** SquareRootVariable **/

SquareRootVariable::SquareRootVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(SquareRootVariable);
void SquareRootVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void SquareRootVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "SquareRootVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "SquareRootVariable");
}

void SquareRootVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "SquareRootVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "SquareRootVariable");
}

void SquareRootVariable::fprop()
{
  int n=nelems();
  for(int i=0; i<n; i++)
    valuedata[i] = sqrt(input->valuedata[i]);
}

// dC/dx = dC/dy * 1/2 * 1/sqrt(x)
void SquareRootVariable::bprop()
{
  if (rValue.length()==0) resizeRValue();
  int n=nelems();
  for(int i=0; i<n; i++)
    {
      input->gradientdata[i] += 0.5/sqrt(input->valuedata[i]) * gradientdata[i];
      rvaluedata[i] = 2*input->valuedata[i]*input->rvaluedata[i];
    }
}
//!                          2                                -3
//! d2C/dx2 = d2C/dx2*(dy/dx)  + dC/dy * 1/2 *-1/2 * 1/sqrt(x)
//! Not verified yet: needs TimesScalarVariable's and DivVariable's fprop and bprop
// void SquareRootVariable::bbprop()
// {
//   if (input->diaghessian.length()==0)
//     input->resizeDiagHessian();
//   int n=nelems();
//   for(int i=0; i<n; i++)
//   {
//     real input_i = input->valuedata[i];
//     input->diaghessiandata[i] += 1/(4*input_i) * diaghessiandata[i]
//                                  + (-0.25) * pow( i/sqrt(input_i), 3 ) * gradientdata[i];
//   }
// }


/** AbsVariable **/

AbsVariable::AbsVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(AbsVariable);
void AbsVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void AbsVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "AbsVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "AbsVariable");
}

void AbsVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "AbsVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "AbsVariable");
}

void AbsVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    {
      real val = input->valuedata[i];
      valuedata[i] = val>=0 ?val :-val;
    }
}

void AbsVariable::bprop()
{
  for(int i=0; i<nelems(); i++)
    {
      real val = input->valuedata[i];
      if(val>=0)        
        input->gradientdata[i] += gradientdata[i];
      else
        input->gradientdata[i] -= gradientdata[i];
    }
}

void AbsVariable::symbolicBprop()
{
  input->accg(ifThenElse(input>=0., g, -g));
}

// R{abs(x)} = R(x) while x >= 0
// R{abs(x)} = -R(x) while x < 0
void AbsVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  int n=nelems();
  for(int i=0; i<n; i++)
    if (input->valuedata[i] < 0) 
        rvaluedata[i] = -input->rvaluedata[i];
        else 
        rvaluedata[i] = input->rvaluedata[i];
}

/** MinVariable **/

MinVariable::MinVariable(Variable* input)
  :UnaryVariable(input, 1, 1) {}

IMPLEMENT_NAME_AND_DEEPCOPY(MinVariable);
void MinVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }

void MinVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MinVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "MinVariable");
}

void MinVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MinVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MinVariable");
}

void MinVariable::fprop()
{
  real minval = input->valuedata[0];
  for(int i=1; i<input->nelems(); i++)
    {
      real val = input->valuedata[i];
      if(val<minval)
        minval = val;
    }
  valuedata[0] = minval;
}

void MinVariable::bprop()
{
  real minval = valuedata[0];
  for(int i=0; i<input->nelems(); i++)
    {
      if(input->valuedata[i]==minval)
        input->gradientdata[i] += gradientdata[0];
    }
}

void MinVariable::symbolicBprop()
{
  input->accg(new ElementAtPositionVariable(g, argmin(input), input->length(), input->width()));
}

/** MaxVariable **/

MaxVariable::MaxVariable(Variable* input)
  :UnaryVariable(input, 1, 1) {}

IMPLEMENT_NAME_AND_DEEPCOPY(MaxVariable);
void MaxVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }

void MaxVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MaxVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "MaxVariable");
}

void MaxVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MaxVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MaxVariable");
}

void MaxVariable::fprop()
{
  real maxval = input->valuedata[0];
  for(int i=1; i<input->nelems(); i++)
    {
      real val = input->valuedata[i];
      if(val>maxval)
        maxval = val;
    }
  valuedata[0] = maxval;
}

void MaxVariable::bprop()
{
  real maxval = valuedata[0];
  for(int i=0; i<input->nelems(); i++)
    {
      if(input->valuedata[i]==maxval)
        input->gradientdata[i] += gradientdata[0];
    }
}

void MaxVariable::symbolicBprop()
{
  input->accg(new ElementAtPositionVariable(g, argmax(input), input->length(), input->width()));
}

/** ArgminVariable **/

ArgminVariable::ArgminVariable(Variable* input)
  :UnaryVariable(input, input->isVec()?1:2, 1) {}

IMPLEMENT_NAME_AND_DEEPCOPY(ArgminVariable);
void ArgminVariable::recomputeSize(int& l, int& w) const
{ l=input->isVec()?1:2; w=1; }

void ArgminVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ArgminVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ArgminVariable");
}

void ArgminVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ArgminVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ArgminVariable");
}

void ArgminVariable::fprop()
{
  real minval = input->valuedata[0];
  if (input->isVec())
    {
      int argmin = 0;
      for(int i=1; i<input->nelems(); i++)
        {
          real val = input->valuedata[i];
          if(val<minval)
            {
              minval = val;
              argmin = i;
            }
        }
      valuedata[0] = argmin;
    }
  else
    {
      int k = 0;
      int argmin_i = 0;
      int argmin_j = 0;
      for(int i=0; i<input->length(); i++)
        for(int j=0; j<input->width(); j++, k++)
          {
            real val = input->valuedata[k];
            if(val<minval)
              {
                minval = val;
                argmin_i = i;
                argmin_j = j;
              }
          }
      valuedata[0] = argmin_i;
      valuedata[1] = argmin_j;
    }
}

void ArgminVariable::bprop() {}
void ArgminVariable::symbolicBprop() {}

/** ArgmaxVariable **/

ArgmaxVariable::ArgmaxVariable(Variable* input)
  :UnaryVariable(input, input->isVec()?1:2, 1) {}

IMPLEMENT_NAME_AND_DEEPCOPY(ArgmaxVariable);
void ArgmaxVariable::recomputeSize(int& l, int& w) const
{ l=input->isVec()?1:2; w=1; }

void ArgmaxVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ArgmaxVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ArgmaxVariable");
}

void ArgmaxVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ArgmaxVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ArgmaxVariable");
}

void ArgmaxVariable::fprop()
{
  real maxval = input->valuedata[0];
  if (input->isVec())
    {
      int argmax = 0;
      for(int i=1; i<input->nelems(); i++)
        {
          real val = input->valuedata[i];
          if(val>maxval)
            {
              maxval = val;
              argmax = i;
            }
        }
      valuedata[0] = argmax;
    }
  else
    {
      int k = 0;
      int argmax_i = 0;
      int argmax_j = 0;
      for(int i=0; i<input->length(); i++)
        for(int j=0; j<input->width(); j++, k++)
          {
            real val = input->valuedata[k];
            if(val>maxval)
              {
                maxval = val;
                argmax_i = i;
                argmax_j = j;
              }
          }
      valuedata[0] = argmax_i;
      valuedata[1] = argmax_j;
    }
}

void ArgmaxVariable::bprop() {}
void ArgmaxVariable::symbolicBprop() {}

/** CutAboveThresholdVariable **/

CutAboveThresholdVariable::CutAboveThresholdVariable(Variable* input, real the_threshold)
  :UnaryVariable(input, input->length(), input->width()), threshold(the_threshold) 
{}

IMPLEMENT_NAME_AND_DEEPCOPY(CutAboveThresholdVariable);
void CutAboveThresholdVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void CutAboveThresholdVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "CutAboveThresholdVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, threshold);
  readFooter(in, "CutAboveThresholdVariable");
}

void CutAboveThresholdVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "CutAboveThresholdVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, threshold);
  writeFooter(out, "CutAboveThresholdVariable");
}

void CutAboveThresholdVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    {
      real val = input->valuedata[i];
      valuedata[i] = val>threshold ?threshold :val;
    }
}

void CutAboveThresholdVariable::bprop()
{
  for(int i=0; i<nelems(); i++)
    if(input->valuedata[i]<=threshold)        
      input->gradientdata[i] += gradientdata[i];
}

void CutAboveThresholdVariable::symbolicBprop()
{
  input->accg(g * (input<=threshold));
}

/** CutBelowThresholdVariable **/

CutBelowThresholdVariable::CutBelowThresholdVariable(Variable* input, real the_threshold)
  :UnaryVariable(input, input->length(), input->width()), threshold(the_threshold) 
{}

IMPLEMENT_NAME_AND_DEEPCOPY(CutBelowThresholdVariable);
void CutBelowThresholdVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void CutBelowThresholdVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "CutBelowThresholdVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, threshold);
  readFooter(in, "CutBelowThresholdVariable");
}

void CutBelowThresholdVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "CutBelowThresholdVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, threshold);
  writeFooter(out, "CutBelowThresholdVariable");
}

void CutBelowThresholdVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    {
      real val = input->valuedata[i];
      valuedata[i] = val<threshold ?threshold :val;
    }
}

void CutBelowThresholdVariable::bprop()
{
  for(int i=0; i<nelems(); i++)
    if(input->valuedata[i]>=threshold)        
      input->gradientdata[i] += gradientdata[i];
}

void CutBelowThresholdVariable::symbolicBprop()
{
  input->accg(g * (input>=threshold));
}

/** ExpVariable **/

ExpVariable::ExpVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(ExpVariable);
void ExpVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void ExpVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ExpVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ExpVariable");
}

void ExpVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ExpVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ExpVariable");
}

void ExpVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    valuedata[i] = safeexp(input->valuedata[i]);
}

void ExpVariable::bprop()
{
  for(int i=0; i<nelems(); i++)
    input->gradientdata[i] += gradientdata[i]*valuedata[i];
}

//! Incorrect!
// void ExpVariable::bbprop()
// {
//   if (input->diaghessian.length()==0)
//     input->resizeDiagHessian();
//   for(int i=0; i<nelems(); i++)
//     {
//       real yi = valuedata[i];
//       input->diaghessiandata[i] += diaghessiandata[i] * yi*yi + gradientdata[i] * yi;
//     }
// }

void ExpVariable::symbolicBprop()
{
  input->accg(g * Var(this));
}

// R{exp(x)} = exp(x) R(x)
void ExpVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  for(int i=0; i<nelems(); i++)
    rvaluedata[i] = input->rvaluedata[i] * valuedata[i];
}

/** LogVariable **/

LogVariable::LogVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(LogVariable);
void LogVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void LogVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "LogVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "LogVariable");
}

void LogVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "LogVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "LogVariable");
}

void LogVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
  {
    valuedata[i] = safeflog(input->valuedata[i]);
#ifdef BOUNDCHECK
    if (!finite(valuedata[i]))
    {
      //PLWARNING("LogVariable::fprop qqchose va pas");
      cout << "inputdata[i]= " << input->valuedata[i] << endl;
      cout << "valuedata[i]= " << valuedata[i] << endl;
      displayVarGraph(this, true, 250);
      PLERROR("LogVariable::fprop qqchose va pas");
    }
#endif
  }
}

void LogVariable::bprop()
{
  for(int i=0; i<nelems(); i++)
    input->gradientdata[i] += gradientdata[i]/input->valuedata[i];
}

void LogVariable::symbolicBprop()
{
  input->accg(g / input);
}

// R{log(x)} = 1/x R(x)
void LogVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  for(int i=0; i<nelems(); i++)
    rvaluedata[i] = input->rvaluedata[i] / input->valuedata[i];
}


/** LogSumVariable **/

LogSumVariable::LogSumVariable(Variable* input)
  :UnaryVariable(input,1,1),input_softmax(input->nelems()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(LogSumVariable);
void LogSumVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }

void LogSumVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "LogSumVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, input_softmax);
  readFooter(in, "LogSumVariable");
}

void LogSumVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "LogSumVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, input_softmax);
  writeFooter(out, "LogSumVariable");
}

void LogSumVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  UnaryVariable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(input_softmax, copies);
}

void LogSumVariable::fprop()
{
  valuedata[0] = logadd(input->value);
}

void LogSumVariable::bprop()
{
  softmax(input->value,input_softmax);
  multiplyAcc(input->gradient, input_softmax,gradientdata[0]);
}

void LogSumVariable::symbolicBprop()
{
  input->accg(g*softmax(input));
}

/** PLogPVariable **/

PLogPVariable::PLogPVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(PLogPVariable);
void PLogPVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void PLogPVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "PLogPVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "PLogPVariable");
}

void PLogPVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "PLogPVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "PLogPVariable");
}

void PLogPVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    {
      real p = input->valuedata[i];
      if(p<1e-10)
        p = 1e-10;
      valuedata[i] = p*safeflog(p);
    }
}

void PLogPVariable::bprop()
{
  for(int i=0; i<nelems(); i++)
    {
      real p = input->valuedata[i];
      if(p<1e-10)
        p = 1e-10;
      input->gradientdata[i] += gradientdata[i]*(1.0+safeflog(p));
    }
}

void PLogPVariable::symbolicBprop()
{
  input->accg(g * (1.+log(input)));
}

/** PowVariable **/

PowVariable::PowVariable(Variable* input, real the_power)
  :UnaryVariable(input, input->length(), input->width()), power(the_power) 
{}

IMPLEMENT_NAME_AND_DEEPCOPY(PowVariable);
void PowVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void PowVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "PowVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, power);
  readFooter(in, "PowVariable");
}

void PowVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "PowVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, power);
  writeFooter(out, "PowVariable");
}

void PowVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    valuedata[i] = mypow(input->valuedata[i],power);
}

void PowVariable::bprop()
{
  for(int i=0; i<nelems(); i++)
    input->gradientdata[i] += power*gradientdata[i]*mypow(input->valuedata[i],power-1.0);
}

void PowVariable::symbolicBprop()
{
  input->accg(g * (power*pow(input, power-1.)));
}


/** DeterminantVariable **/

DeterminantVariable::DeterminantVariable(Var m)
  :UnaryVariable(m,1,1) 
{
  if (m->width()!=m->length())
    PLERROR("Max2Variable: parent(%d,%d) must be a square matrix",
          m->length()!=m->width());
}

IMPLEMENT_NAME_AND_DEEPCOPY(DeterminantVariable);
void DeterminantVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }

void DeterminantVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "DeterminantVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "DeterminantVariable");
}

void DeterminantVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "DeterminantVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "DeterminantVariable");
}


void DeterminantVariable::fprop()
{
  valuedata[0] = det(input->matValue);
}

void DeterminantVariable::bprop()
{
  PLERROR("DeterminantVariable::bprop not implemented yet");
}

void DeterminantVariable::symbolicBprop()
{
  PLERROR("DeterminantVariable::symbolicBprop not implemented yet");
}

/** InterValuesVariable **/

// if values = [x1,x2,...,x10], the resulting variable is 
// [(x1+x2)/2,(x2+x3)/2, ... (x9+x10)/2]
InterValuesVariable::InterValuesVariable(Variable* values) 
  :UnaryVariable(values,values->length()-1,1) 
{
  if(!values->isColumnVec())
    PLERROR("In InterValuesVariable: input must be a column vector (single column matrix)");
}

IMPLEMENT_NAME_AND_DEEPCOPY(InterValuesVariable);
void InterValuesVariable::recomputeSize(int& l, int& w) const
{ l=input->length()-1; w=1; }

void InterValuesVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "InterValuesVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "InterValuesVariable");
}

void InterValuesVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "InterValuesVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "InterValuesVariable");
}


void InterValuesVariable::fprop()
{
  real prev_x = input->valuedata[0];
  for (int i=0;i<nelems();i++)
    {
      real next_x = input->valuedata[i+1];
      valuedata[i] = 0.5 * (prev_x + next_x);
      prev_x = next_x;
    }
}

void InterValuesVariable::bprop()
{
  real* prev_dx = &input->gradientdata[0];
  for (int i=0;i<nelems();i++)
    {
      real* next_dx = &input->gradientdata[i+1];
      *prev_dx += 0.5 * gradientdata[i];
      *next_dx += 0.5 * gradientdata[i];
      prev_dx = next_dx;
    }
}

void InterValuesVariable::symbolicBprop()
{
  Var zero(1);
  Var g1 = new InterValuesVariable(vconcat(zero & g & (VarArray)zero));

  input->accg(g1);
}

/** IsAboveThresholdVariable **/

IsAboveThresholdVariable::
IsAboveThresholdVariable(Variable* input, real the_threshold, real the_truevalue, real the_falsevalue)
  :UnaryVariable(input, input->length(), input->width()),
   threshold(the_threshold), truevalue(the_truevalue), falsevalue(the_falsevalue)
{}

IMPLEMENT_NAME_AND_DEEPCOPY(IsAboveThresholdVariable);
void IsAboveThresholdVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void IsAboveThresholdVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "IsAboveThresholdVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, threshold);
  PLearn::deepRead(in, old2new, truevalue);
  PLearn::deepRead(in, old2new, falsevalue);
  readFooter(in, "IsAboveThresholdVariable");
}

void IsAboveThresholdVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "IsAboveThresholdVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, threshold);
  PLearn::deepWrite(out, already_saved, truevalue);
  PLearn::deepWrite(out, already_saved, falsevalue);
  writeFooter(out, "IsAboveThresholdVariable");
}


void IsAboveThresholdVariable::fprop()
{
  for(int i=0; i<input->nelems(); i++)
    if(input->valuedata[i]>=threshold)
      valuedata[i] = truevalue;
    else
      valuedata[i] = falsevalue;
}

void IsAboveThresholdVariable::bprop() {}
void IsAboveThresholdVariable::symbolicBprop() {}

void IsAboveThresholdVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
}

/** OneHotVariable **/

OneHotVariable::OneHotVariable(int thelength, Variable* hotindex, real the_coldvalue, real the_hotvalue)
  :UnaryVariable(hotindex,thelength,1), hotvalue(the_hotvalue), coldvalue(the_coldvalue), length_(thelength)
{
  if(!hotindex->isScalar())
    PLERROR("InterValuesVariable OneHotVariable(int thelength, Variable* hotindex, real the_coldvalue, real the_hotvalue) hotindex must be scalar as it is supposed to be an integer index");
}

IMPLEMENT_NAME_AND_DEEPCOPY(OneHotVariable);
void OneHotVariable::recomputeSize(int& l, int& w) const
{ l=length_; w=1; }

void OneHotVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "OneHotVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, hotvalue);
  PLearn::deepRead(in, old2new, coldvalue);
  readFooter(in, "OneHotVariable");
}

void OneHotVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "OneHotVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, hotvalue);
  PLearn::deepWrite(out, already_saved, coldvalue);
  writeFooter(out, "OneHotVariable");
}

void OneHotVariable::fprop()
{
  int index = int(input->valuedata[0]);
  if (nelems()==1)
    value[0] = index==0 ? coldvalue : hotvalue;
  else
  {
    for(int i=0; i<nelems(); i++)
      valuedata[i] = coldvalue;
    value[index] = hotvalue;
  }
}

void OneHotVariable::bprop() {}

void OneHotVariable::symbolicBprop() {}

void OneHotVariable::rfprop() {
  if (rValue.length()==0) resizeRValue();
}

/** EqualConstantVariable **/

string EqualConstantVariable::info() const
{ return string("EqualConstant (== ")+tostring(eqv)+")"; }

EqualConstantVariable::EqualConstantVariable(Variable* input1, real input2)
  : UnaryVariable(input1, input1->length(), input1->width()), eqv(input2)
{}
  
IMPLEMENT_NAME_AND_DEEPCOPY(EqualConstantVariable);
void EqualConstantVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void EqualConstantVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "EqualConstantVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, eqv);
  readFooter(in, "EqualConstantVariable");
}

void EqualConstantVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "EqualConstantVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, eqv);
  writeFooter(out, "EqualConstantVariable");
}


void EqualConstantVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    valuedata[i] = (input->valuedata[i] == eqv);
}

// not really differentiable (zero gradient almost everywhere)
void EqualConstantVariable::bprop() {}
void EqualConstantVariable::symbolicBprop() {}

/** UnequalConstantVariable **/

string UnequalConstantVariable::info() const
{ return string("EqualConstantVariable (!= ")+tostring(c)+")"; }

UnequalConstantVariable::UnequalConstantVariable(Variable* input1, real c_)
  : UnaryVariable(input1, input1->length(), input1->width()), c(c_)
{}
  
IMPLEMENT_NAME_AND_DEEPCOPY(UnequalConstantVariable);
void UnequalConstantVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void UnequalConstantVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "UnequalConstantVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, c);
  readFooter(in, "UnequalConstantVariable");
}

void UnequalConstantVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "UnequalConstantVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, c);
  writeFooter(out, "UnequalConstantVariable");
}


void UnequalConstantVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    valuedata[i] = (input->valuedata[i] != c);
}

// not really differentiable (zero gradient almost everywhere)
void UnequalConstantVariable::bprop() {}
void UnequalConstantVariable::symbolicBprop() {}

/** SubsampleVariable **/

SubsampleVariable::SubsampleVariable(Variable* input, int the_subsamplefactor) 
  :UnaryVariable(input, input->length()/the_subsamplefactor, input->width()/the_subsamplefactor), 
   subsamplefactor(the_subsamplefactor) 
{
  if(input->length()%the_subsamplefactor!=0 || input->width()%the_subsamplefactor!=0)
    PLERROR("In SubsampleVariable constructor: Dimensions of input are not dividable by subsamplefactor");
}

IMPLEMENT_NAME_AND_DEEPCOPY(SubsampleVariable);
void SubsampleVariable::recomputeSize(int& l, int& w) const
{ l=input->length()/subsamplefactor; w=input->width()/subsamplefactor; }

void SubsampleVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "SubsampleVariable");
  inherited::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, subsamplefactor);
  readFooter(in, "SubsampleVariable");
}

void SubsampleVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "SubsampleVariable");
  inherited::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, subsamplefactor);
  writeFooter(out, "SubsampleVariable");
}


void SubsampleVariable::fprop()
{
  subsample(input->matValue, subsamplefactor, matValue);
}

void SubsampleVariable::bprop()
{
  int norm = subsamplefactor * subsamplefactor;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++)
      {
        real* inputgradientptr = input->matGradient[subsamplefactor*i]+subsamplefactor*j;
        real thisgradient = matGradient(i,j);
        for(int l=0; l<subsamplefactor; l++, inputgradientptr += input->matGradient.mod())
          for(int c=0; c<subsamplefactor; c++)
            {
              inputgradientptr[c] = thisgradient/norm;
            }
      }
}

void SubsampleVariable::symbolicBprop()
{ PLERROR("SubsampleVariable::symbolicBprop() not yet implemented"); }

/** ErfVariable **/

ErfVariable::ErfVariable(Variable* input) 
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(ErfVariable);
void ErfVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void ErfVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ErfVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ErfVariable");
}

void ErfVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ErfVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ErfVariable");
}

void ErfVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    valuedata[i] = pl_erf(input->valuedata[i]);
}

void ErfVariable::bprop()
{
  real cst = 2.0/sqrt(Pi);
  for(int i=0; i<nelems(); i++)
    input->gradientdata[i] += gradientdata[i] * cst*exp(input->valuedata[i]*input->valuedata[i]);
}

void ErfVariable::symbolicBprop()
{
  Var v(this);
  input->accg(g * 2.0/sqrt(Pi)*exp(square(input)));
}

/** SignVariable **/

SignVariable::SignVariable(Variable* input) 
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(SignVariable);
void SignVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void SignVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "SignVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "SignVariable");
}

void SignVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "SignVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "SignVariable");
}

void SignVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    valuedata[i] = sign(input->valuedata[i]);
}

// no gradients almost everywhere
void SignVariable::bprop()
{}

// no gradients almost everywhere
void SignVariable::symbolicBprop()
{}

/** SoftmaxVariable **/

SoftmaxVariable::SoftmaxVariable(Variable* input) 
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(SoftmaxVariable);
void SoftmaxVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void SoftmaxVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "SoftmaxVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "SoftmaxVariable");
}

void SoftmaxVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "SoftmaxVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "SoftmaxVariable");
}

void SoftmaxVariable::fprop()
{
  softmax(input->value,value);
}

void SoftmaxVariable::bprop()
{
  for(int i=0; i<input->nelems(); i++)
  {
    real vali = valuedata[i];
    for(int k=0; k<nelems(); k++)
    {
      if(k!=i)
        input->gradientdata[i] -= gradientdata[k]*vali*valuedata[k];
      else
        input->gradientdata[i] += gradientdata[i]*vali*(1.-vali);        
    }
  }
}

void SoftmaxVariable::bbprop()
{
  PLERROR("SofmaxVariable::bbprop() not implemented");
}

void SoftmaxVariable::symbolicBprop()
{
  PLERROR("SofmaxVariable::symbolicBprop() not implemented");
}

// R{ s_i = exp(x_i) / sum_j exp(x_j) }   = (s_i(1-s_i) - sum_{k!=i} s_i s_k) R(s_i) = s_i ((1-s_i) - sum_{k!=i} s_k) R(s_i)
void SoftmaxVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  for(int i=0; i<input->nelems(); i++)
  {
    real vali = valuedata[i];
    rvaluedata[i] = vali * (1 - vali) * input->rvaluedata[i];

/*    real vali = valuedata[i];
    rvaluedata[i] = 0.;
    for(int k=0; k<nelems(); k++)
    {
      if(k!=i)
        rvaluedata[i] =  - vali * valuedata[k] * input->rvaluedata[i];
      else
        rvaluedata[i] = vali * (1 - vali) * input->rvaluedata[i];
    }*/
  }
}

/** MatrixSoftmaxVariable **/

MatrixSoftmaxVariable::MatrixSoftmaxVariable(Variable* input) 
  :UnaryVariable(input, input->length(), input->width()) {}

IMPLEMENT_NAME_AND_DEEPCOPY(MatrixSoftmaxVariable);
void MatrixSoftmaxVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

void MatrixSoftmaxVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MatrixSoftmaxVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "MatrixSoftmaxVariable");
}

void MatrixSoftmaxVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MatrixSoftmaxVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MatrixSoftmaxVariable");
}

void MatrixSoftmaxVariable::fprop()
{
  for(int i=0; i<input->length(); i++)
    softmax(input->matValue(i),matValue(i));
}

void MatrixSoftmaxVariable::bprop()
{
  for(int i=0; i<input->length(); i++)
    for(int j=0; j<input->width(); j++)
      {
       real vali = matValue[i][j];
       for(int k=0; k<width(); k++)
         {
          if(k!=j)
            input->matGradient[i][j] -= matGradient[i][k]*vali*matValue[i][k];
          else
            input->matGradient[i][j] += matGradient[i][j]*vali*(1.-vali);
         }
       }
}

void MatrixSoftmaxVariable::bbprop()
{
  PLERROR("MatrixSofmaxVariable::bbprop() not implemented");
}

void MatrixSoftmaxVariable::symbolicBprop()
{
  PLERROR("SofmaxVariable::symbolicBprop() not implemented");
}

// R{ s_i = exp(x_i) / sum_j exp(x_j) }   = (s_i(1-s_i) - sum_{k!=i} s_i s_k) R(s_i) = s_i ((1-s_i) - sum_{k!=i} s_k) R(s_i)
void MatrixSoftmaxVariable::rfprop()
{
  PLERROR("SofmaxVariable::rfprop() not implemented");
}

/** LogSoftmaxVariable **/

IMPLEMENT_NAME_AND_DEEPCOPY(LogSoftmaxVariable);

void LogSoftmaxVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }

LogSoftmaxVariable::LogSoftmaxVariable(Variable* input) 
    : UnaryVariable(input, input->length(), input->width())
{
}

void
LogSoftmaxVariable::fprop()
{
    log_softmax(input->value, value);
}

void
LogSoftmaxVariable::bprop()
{
    for (int i = 0; i < input->nelems(); ++i) {
        real sum = 0.;
        for (int k = 0; k < nelems(); ++k)
            sum += gradientdata[k];
        input->gradientdata[i] += gradientdata[i] - sum * safeexp(valuedata[i]);
    }
}

void
LogSoftmaxVariable::bbprop()
{
    PLERROR("LogSofmaxVariable::bbprop() not implemented");
}

void
LogSoftmaxVariable::symbolicBprop()
{
    PLERROR("LogSofmaxVariable::symbolicBprop() not implemented");
}


void AffineTransformWeightPenalty::fprop()
{
  valuedata[0] = weight_decay_*sumsquare(input->matValue.subMatRows(1,input->length()-1));
  if(bias_decay_!=0)
    valuedata[0] += bias_decay_*sumsquare(input->matValue(1));
  //  cerr<<"$$ w:" << weight_decay_ << " b:" << bias_decay_ << " " << value[0]<<" ## ";
}
    
void AffineTransformWeightPenalty::bprop()
{
  int l = input->length() - 1;
  multiplyAcc(input->matGradient.subMatRows(1,l), input->matValue.subMatRows(1,l), two(weight_decay_)*gradientdata[0]);
  if(bias_decay_!=0)
    multiplyAcc(input->matGradient(1), input->matValue(1), two(bias_decay_)*gradientdata[0]);
}

%> // end of namespace PLearn




