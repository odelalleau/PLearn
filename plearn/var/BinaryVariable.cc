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
   * $Id: BinaryVariable.cc,v 1.1 2002/07/30 09:01:28 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "BinaryVariable.h"
#include "Var.h"
#include "pl_erf.h"

namespace PLearn <%
using namespace std;

/** BinaryVariable **/

BinaryVariable::BinaryVariable(Variable* v1, Variable* v2, int thelength,int thewidth)
  :Variable(thelength,thewidth), input1(v1), input2(v2) 
{
  input1->disallowPartialUpdates();
  input2->disallowPartialUpdates();
}

IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY(BinaryVariable);


void BinaryVariable::sizeprop()
{
}

void BinaryVariable::setParents(const VarArray& parents)
{
  if(parents.length() != 2)
    PLERROR("In BinaryVariable::setParents  VarArray length must be 2;"
	    " you are trying to set %d parents for this BinaryVariable...", parents.length());

  input1= parents[0];
  input2= parents[1];
  
  int dummy_l, dummy_w;
  recomputeSize(dummy_l, dummy_w);
}

void BinaryVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "BinaryVariable");
  Variable::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, input1);
  PLearn::deepRead(in, old2new, input2);
  readFooter(in, "BinaryVariable");
}

void BinaryVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "BinaryVariable");
  Variable::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, input1);
  PLearn::deepWrite(out, already_saved, input2);
  writeFooter(out, "BinaryVariable");
}

void BinaryVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Variable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(input1, copies);
  deepCopyField(input2, copies);
}

bool BinaryVariable::markPath()
{
  if(!marked)
    marked = input1->markPath() | input2->markPath();
  return marked;
}

void BinaryVariable::buildPath(VarArray& proppath)
{
  if(marked)
    {
      input1->buildPath(proppath);
      input2->buildPath(proppath);
      proppath &= Var(this);
      //cout<<"proppath="<<this->getName()<<endl;
      clearMark();
    }
}

VarArray BinaryVariable::sources() 
{ 
  if (marked)
    return VarArray(0,0);
  marked = true;
  return input1->sources() & input2->sources(); 
}

VarArray BinaryVariable::random_sources() 
{ 
  if (marked)
    return VarArray(0,0);
  marked = true;
  return input1->random_sources() & input2->random_sources(); 
}

VarArray BinaryVariable::ancestors() 
{ 
  if (marked)
    return VarArray(0,0);
  marked = true;
  return input1->ancestors() & input2->ancestors() & Var(this) ;
}

void BinaryVariable::unmarkAncestors()
{ 
  if (marked)
    {
      marked = false;
      input1->unmarkAncestors();
      input2->unmarkAncestors();
    }
}

VarArray BinaryVariable::parents() 
{ 
  VarArray unmarked_parents;
  if (!input1->marked)
    unmarked_parents.append(input1);
  if (!input2->marked)
    unmarked_parents.append(input2);
  return unmarked_parents;
}

void BinaryVariable::resizeRValue()
{
  inherited::resizeRValue();
  if (!input1->rvaluedata) input1->resizeRValue();
  if (!input2->rvaluedata) input2->resizeRValue();
}



/** VarRowVariable **/

VarRowVariable::VarRowVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, 1, input1->width())
{
  if(!input2->isScalar())
    PLERROR("IN VarRowVariable(Variable* input1, Variable* input2) input2 must be scalar as it is supposed to be an integer index");
  input1->allowPartialUpdates();
}
  
IMPLEMENT_NAME_AND_DEEPCOPY(VarRowVariable);

void VarRowVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "VarRowVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "VarRowVariable");
}

void VarRowVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "VarRowVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "VarRowVariable");
}

void VarRowVariable::fprop()
{
  int i = int(input2->valuedata[0]);
  value << input1->matValue(i);
}

void VarRowVariable::bprop()
{
  int i = int(input2->valuedata[0]);
  Vec input1_gradient_i = input1->matGradient(i);
  input1_gradient_i += gradient;
  input1->updateRow(i);
}

void VarRowVariable::symbolicBprop()
{
  input1->accg(new RowAtPositionVariable(g,input2,input1->length()));
}

void VarRowVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue(); 
  int i = int(input2->valuedata[0]);
  rValue << input1->matRValue(i);
}


/** VarRowsVariable **/

VarRowsVariable::VarRowsVariable(Variable *input1, Variable *input2)
  : BinaryVariable(input1, input2, input2->length(), input1->width())
{
  input1->allowPartialUpdates();
}

IMPLEMENT_NAME_AND_DEEPCOPY(VarRowsVariable);

void VarRowsVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "VarRowsVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "VarRowsVariable");
}

void VarRowsVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "VarRowsVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "VarRowsVariable");
}

void VarRowsVariable::fprop()
{
  matValue << input1->matValue.rows(input2->value);
}

void VarRowsVariable::bprop()
{
  for (int i = 0; i < input2->length(); ++i)
  {
    int r=int(input2->value[i]);
    input1->matGradient.row(r) += matGradient.row(i);
    input1->updateRow(r);
  }
}

void VarRowsVariable::symbolicBprop()
{
  PLERROR("VarRowsVariable::symbolicBprop() not implemented");
}

/** VarColumnsVariable **/

VarColumnsVariable::VarColumnsVariable(Variable *input1, Variable *input2)
  //  : BinaryVariable(input1, input2, input1->length(), input2->width())
  : BinaryVariable(input1, input2, input1->length(), input2->length())
{
}

IMPLEMENT_NAME_AND_DEEPCOPY(VarColumnsVariable);

void VarColumnsVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "VarColumnsVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "VarColumnsVariable");
}

void VarColumnsVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "VarColumnsVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "VarColumnsVariable");
}

void VarColumnsVariable::fprop()
{
  matValue << input1->matValue.columns(input2->value);
}

void VarColumnsVariable::bprop()
{
  for (int i = 0; i < input2->length(); ++i)
    input1->matGradient.column(int(input2->value[i])) += matGradient.column(i);
}

void VarColumnsVariable::symbolicBprop()
{
  PLERROR("VarRowsVariable::symbolicBprop() not implemented");
}

/** VarElementVariable **/

VarElementVariable::VarElementVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, 1, 1)
{
  if(input2->nelems()>2)
    PLERROR("IN VarElementVariable(Variable* input1, Variable* input2) input2 must have 1 (a k position index) or 2 elements (an i,j position index)");
}
  
IMPLEMENT_NAME_AND_DEEPCOPY(VarElementVariable);

void VarElementVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "VarElementVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "VarElementVariable");
}

void VarElementVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "VarElementVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "VarElementVariable");
}

void VarElementVariable::fprop()
{
  if(input2->isScalar()) // scalar position index k
    {
      int k = int(input2->valuedata[0]);
      valuedata[0] = input1->valuedata[k];
    }
  else // (i,j) position index
    {
      int i = int(input2->valuedata[0]);
      int j = int(input2->valuedata[1]);
      valuedata[0] = input1->valuedata[i*input1->width()+j];
    }
}

void VarElementVariable::bprop()
{
  if(input2->isScalar()) // scalar position index k
    {
      int k = int(input2->valuedata[0]);
      input1->gradientdata[k] += gradientdata[0];
    }
  else // (i,j) position index
    {
      int i = int(input2->valuedata[0]);
      int j = int(input2->valuedata[1]);
      input1->gradientdata[i*input1->width()+j] += gradientdata[0];
    }
}

void VarElementVariable::symbolicBprop()
{
  input1->accg(new ElementAtPositionVariable(g,input2,input1->length(),input1->width()));
}

void VarElementVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  if(input2->isScalar()) // scalar position index k
    {
      int k = int(input2->valuedata[0]);
      rvaluedata[0] = input1->rvaluedata[k];
    }
  else // (i,j) position index
    {
      int i = int(input2->valuedata[0]);
      int j = int(input2->valuedata[1]);
      rvaluedata[0] = input1->rvaluedata[i*input1->width()+j];
    }
}

/** RowAtPositionVariable **/

RowAtPositionVariable::RowAtPositionVariable(Variable* input1, Variable* input2, int the_length)
  :BinaryVariable(input1, input2, the_length, input1->width())
{
  if(!input1->isRowVec())
    PLERROR("In RowAtPositionVariable: input1 must be a single row");
  if(!input2->isScalar())
    PLERROR("In RowAtPositionVariable: position must be a scalar");
}

IMPLEMENT_NAME_AND_DEEPCOPY(RowAtPositionVariable);

void RowAtPositionVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "RowAtPositionVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "RowAtPositionVariable");
}

void RowAtPositionVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "RowAtPositionVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "RowAtPositionVariable");
}


void RowAtPositionVariable::fprop()
{
  value.clear();
  int i = (int)input2->valuedata[0];
  matValue(i) << input1->value;
}

void RowAtPositionVariable::bprop()
{
  int i = (int)input2->valuedata[0];
  input1->gradient += matGradient(i);
}

void RowAtPositionVariable::symbolicBprop()
{
  input1->accg(new VarRowVariable(g,input2));
}

void RowAtPositionVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  //rValue.clear();
  int i = (int)input2->valuedata[0];
  matRValue(i) << input1->rValue;
}

/** ElementAtPositionVariable **/

ElementAtPositionVariable::ElementAtPositionVariable(Variable* input1, Variable* input2, int the_length, int the_width)
  :BinaryVariable(input1, input2, the_length, the_width)
{
  if(!input1->isScalar())
    PLERROR("In ElementAtPositionVariable: element must be a scalar var");
  if(input2->nelems()>2)
    PLERROR("In ElementAtPositionVariable: position must have 1 or 2 elements");
}

IMPLEMENT_NAME_AND_DEEPCOPY(ElementAtPositionVariable);

void ElementAtPositionVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ElementAtPositionVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ElementAtPositionVariable");
}

void ElementAtPositionVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ElementAtPositionVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ElementAtPositionVariable");
}

 
void ElementAtPositionVariable::fprop()
{
  value.clear();
  if (input2->isScalar()) // input2 is a scalar (interpreted as a k)
    {
      int k = (int)input2->valuedata[0];
      valuedata[k] = input1->valuedata[0];
    }
  else // input2 has 2 elements (interpreted as (i,j) coordinates)
    {
      int i = (int)input2->valuedata[0];
      int j = (int)input2->valuedata[1];
      valuedata[i*width()+j] = input1->valuedata[0];
    }
}

void ElementAtPositionVariable::bprop()
{
  if (input2->isScalar()) // input2 is a scalar (interpreted as a k)
    {
      int k = (int)input2->valuedata[0];
      input1->gradientdata[0] += gradientdata[k];
    }
  else // input2 has 2 elements (interpreted as (i,j) coordinates)
    {
      int i = (int)input2->valuedata[0];
      int j = (int)input2->valuedata[1];
      input1->gradientdata[0] += gradientdata[i*width()+j];
    }  
}

void ElementAtPositionVariable::symbolicBprop()
{
  input1->accg(new VarElementVariable(g,input2));
}


void ElementAtPositionVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  rValue.clear();
  if (input2->isScalar()) // input2 is a scalar (interpreted as a k)
    {
      int k = (int)input2->valuedata[0];
      rvaluedata[k] = input1->rvaluedata[0];
    }
  else // input2 has 2 elements (interpreted as (i,j) coordinates)
    {
      int i = (int)input2->valuedata[0];
      int j = (int)input2->valuedata[1];
      rvaluedata[i*width()+j] = input1->rvaluedata[0];
    }
}

/** PlusScalarVariable **/

PlusScalarVariable::PlusScalarVariable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isScalar())
    PLERROR("IN PlusScalarVariable: input2 is not a scalar");
}

IMPLEMENT_NAME_AND_DEEPCOPY(PlusScalarVariable);

void PlusScalarVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "PlusScalarVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "PlusScalarVariable");
}

void PlusScalarVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "PlusScalarVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "PlusScalarVariable");
}
void PlusScalarVariable::fprop()
{
  real scal = input2->valuedata[0];
  for(int k=0; k<nelems(); k++)
    valuedata[k] = input1->valuedata[k] + scal;
}

void PlusScalarVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    {
      input1->gradientdata[k] += gradientdata[k];
      input2->gradientdata[0] += gradientdata[k];
    }
}

void PlusScalarVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  for(int k=0; k<nelems(); k++)
    {
      input1->diaghessiandata[k] += diaghessiandata[k];
      input2->diaghessiandata[0] += diaghessiandata[k];
    }
}

void PlusScalarVariable::symbolicBprop()
{
  input1->accg(g);
  input2->accg(sum(g));
}


// R(x1+x2) = R(x1) + R(x2)
void PlusScalarVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  real rscal = input2->rvaluedata[0];
  for(int k=0; k<nelems(); k++)
    rvaluedata[k] = input1->rvaluedata[k] + rscal;
}

/** PlusRowVariable **/

PlusRowVariable::PlusRowVariable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isRowVec())
    PLERROR("IN PlusRowVariable: input2 is not a row");
  if(input2->width() != input1->width())
    PLERROR("IN PlusRowVariable: input1 and input2 have a different width()");
}

IMPLEMENT_NAME_AND_DEEPCOPY(PlusRowVariable);

void PlusRowVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "PlusRowVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "PlusRowVariable");
}

void PlusRowVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "PlusRowVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "PlusRowVariable");
}

void PlusRowVariable::fprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      valuedata[k] = input1->valuedata[k] + input2->valuedata[j];
}

void PlusRowVariable::bprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      {
        input1->gradientdata[k] += gradientdata[k];
        input2->gradientdata[j] += gradientdata[k];
      }
}

void PlusRowVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      {
        input1->diaghessiandata[k] += diaghessiandata[k];
        input2->diaghessiandata[j] += diaghessiandata[k];
      }
}

void PlusRowVariable::symbolicBprop()
{
  input1->accg(g);
  input2->accg(columnSum(g));
}

void PlusRowVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      rvaluedata[k] = input1->rvaluedata[k] + input2->rvaluedata[j];
}

/** PlusColumnVariable **/

PlusColumnVariable::PlusColumnVariable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isColumnVec())
    PLERROR("IN PlusColumnVariable: input2 is not a column");
  if(input2->length() != input1->length())
    PLERROR("IN PlusColumnVariable: input1 and input2 have a different length()");
}

IMPLEMENT_NAME_AND_DEEPCOPY(PlusColumnVariable);

void PlusColumnVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "PlusColumnVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "PlusColumnVariable");
}

void PlusColumnVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "PlusColumnVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "PlusColumnVariable");
}

void PlusColumnVariable::fprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      valuedata[k] = input1->valuedata[k] + input2->valuedata[i];
}

void PlusColumnVariable::bprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      {
        input1->gradientdata[k] += gradientdata[k];
        input2->gradientdata[i] += gradientdata[k];
      }
}

void PlusColumnVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      {
        input1->diaghessiandata[k] += diaghessiandata[k];
        input2->diaghessiandata[i] += diaghessiandata[k];
      }
}

void PlusColumnVariable::symbolicBprop()
{
  input1->accg(g);
  input2->accg(rowSum(g));
}

void PlusColumnVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      rvaluedata[k] = input1->rvaluedata[k] + input2->rvaluedata[i];
}

/** PlusVariable **/

PlusVariable::PlusVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(input1->length()!=input2->length() || input1->width()!=input2->width())
    PLERROR("In PlusVariable: input1 and input2 must have exactly the same size");
}

IMPLEMENT_NAME_AND_DEEPCOPY(PlusVariable);

void PlusVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "PlusVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "PlusVariable");
}

void PlusVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "PlusVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "PlusVariable");
}

void PlusVariable::fprop()
{
  for(int k=0; k<nelems(); k++)
    valuedata[k] = input1->valuedata[k]+input2->valuedata[k];
}

void PlusVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    {
      input1->gradientdata[k] += gradientdata[k];
      input2->gradientdata[k] += gradientdata[k];
    }
}

void PlusVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  for(int k=0; k<nelems(); k++)
    {
      input1->diaghessiandata[k] += diaghessiandata[k];
      input2->diaghessiandata[k] += diaghessiandata[k];
    }
}

void PlusVariable::symbolicBprop()
{
  input1->accg(g);
  input2->accg(g);
}

/** MinusScalarVariable **/

MinusScalarVariable::MinusScalarVariable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isScalar())
    PLERROR("IN MinusScalarVariable: input2 is not a scalar");
}

IMPLEMENT_NAME_AND_DEEPCOPY(MinusScalarVariable);

void MinusScalarVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MinusScalarVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "MinusScalarVariable");
}

void MinusScalarVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MinusScalarVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MinusScalarVariable");
}

void MinusScalarVariable::fprop()
{
  real scal = input2->valuedata[0];
  for(int k=0; k<nelems(); k++)
    valuedata[k] = input1->valuedata[k] - scal;
}

void MinusScalarVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    {
      input1->gradientdata[k] += gradientdata[k];
      input2->gradientdata[0] -= gradientdata[k];
    }
}

void MinusScalarVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  for(int k=0; k<nelems(); k++)
    {
      input1->diaghessiandata[k] += diaghessiandata[k];
      input2->diaghessiandata[0] -= diaghessiandata[k];
    }
}

void MinusScalarVariable::symbolicBprop()
{
  input1->accg(g);
  input2->accg(-sum(g));
}

// R(x1-x2) = R(x1)-R(x2)
void MinusScalarVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  real rscal = input2->rvaluedata[0];
  for(int k=0; k<nelems(); k++)
    rvaluedata[k] = input1->rvaluedata[k] - rscal;
}

/** MinusRowVariable **/

MinusRowVariable::MinusRowVariable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isRowVec())
    PLERROR("IN MinusRowVariable: input2 is not a row");
  if(input2->width() != input1->width())
    PLERROR("IN MinusRowVariable: input1 and input2 have a different width()");
}

IMPLEMENT_NAME_AND_DEEPCOPY(MinusRowVariable);

void MinusRowVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MinusRowVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "MinusRowVariable");
}

void MinusRowVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MinusRowVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MinusRowVariable");
}

void MinusRowVariable::fprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      valuedata[k] = input1->valuedata[k] - input2->valuedata[j];
}

void MinusRowVariable::bprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      {
        input1->gradientdata[k] += gradientdata[k];
        input2->gradientdata[j] -= gradientdata[k];
      }
}

void MinusRowVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      {
        input1->diaghessiandata[k] += diaghessiandata[k];
        input2->diaghessiandata[j] -= diaghessiandata[k];
      }
}

void MinusRowVariable::symbolicBprop()
{
  input1->accg(g);
  input2->accg(-columnSum(g));
}

/** MinusTransposedColumnVariable **/

MinusTransposedColumnVariable::MinusTransposedColumnVariable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isColumnVec())
    PLERROR("IN MinusTransposedColumnVariable: input2 is not a column");
  if(input2->length() != input1->width())
    PLERROR("IN MinusTransposedColumnVariable: the width() of input1 and the length() of input2 differ");
}

IMPLEMENT_NAME_AND_DEEPCOPY(MinusTransposedColumnVariable);

void MinusTransposedColumnVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MinusTransposedColumnVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "MinusTransposedColumnVariable");
}

void MinusTransposedColumnVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MinusTransposedColumnVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MinusTransposedColumnVariable");
}

void MinusTransposedColumnVariable::fprop()
{
  int k=0;
  real* i2 = input2->valuedata;
  real* i1 = input1->valuedata;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      valuedata[k] = i1[k] - i2[j];
}

void MinusTransposedColumnVariable::bprop()
{
  int k=0;
  real* i2g = input2->gradientdata;
  real* i1g = input1->gradientdata;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      {
        i1g[k] += gradientdata[k];
        i2g[j] -= gradientdata[k];
      }
}

void MinusTransposedColumnVariable::symbolicBprop()
{
  input1->accg(g);
  input2->accg(-columnSum(g));
}

/** MinusColumnVariable **/

MinusColumnVariable::MinusColumnVariable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isColumnVec())
    PLERROR("IN MinusColumnVariable: input2 is not a column");
  if(input2->length() != input1->length())
    PLERROR("IN MinusColumnVariable: input1 and input2 have a different length()");
}

IMPLEMENT_NAME_AND_DEEPCOPY(MinusColumnVariable);

void MinusColumnVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MinusColumnVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "MinusColumnVariable");
}

void MinusColumnVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MinusColumnVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MinusColumnVariable");
}

void MinusColumnVariable::fprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      valuedata[k] = input1->valuedata[k] - input2->valuedata[i];
}

void MinusColumnVariable::bprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      {
        input1->gradientdata[k] += gradientdata[k];
        input2->gradientdata[i] -= gradientdata[k];
      }
}

void MinusColumnVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      {
        input1->diaghessiandata[k] += diaghessiandata[k];
        input2->diaghessiandata[i] -= diaghessiandata[k];
      }
}

void MinusColumnVariable::symbolicBprop()
{
  input1->accg(g);
  input2->accg(-rowSum(g));
}

/** MinusVariable **/

MinusVariable::MinusVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(input1->length()!=input2->length() || input1->width()!=input2->width())
    PLERROR("In MinusVariable: input1 and input2 must have exactly the same size");
}

IMPLEMENT_NAME_AND_DEEPCOPY(MinusVariable);

void MinusVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MinusVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "MinusVariable");
}

void MinusVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MinusVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MinusVariable");
}

void MinusVariable::fprop()
{
  for(int k=0; k<nelems(); k++)
    valuedata[k] = input1->valuedata[k]-input2->valuedata[k];
}

void MinusVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    {
      input1->gradientdata[k] += gradientdata[k];
      input2->gradientdata[k] -= gradientdata[k];
    }
}

void MinusVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  for(int k=0; k<nelems(); k++)
    {
      input1->diaghessiandata[k] += diaghessiandata[k];
      input2->diaghessiandata[k] -= diaghessiandata[k];
    }
}

void MinusVariable::symbolicBprop()
{
  input1->accg(g);
  input2->accg(-g);
}


/** TimesScalarVariable **/

TimesScalarVariable::TimesScalarVariable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isScalar())
    PLERROR("IN TimesScalarVariable: input2 is not a scalar");
}

IMPLEMENT_NAME_AND_DEEPCOPY(TimesScalarVariable);

void TimesScalarVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "TimesScalarVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "TimesScalarVariable");
}

void TimesScalarVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "TimesScalarVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "TimesScalarVariable");
}

void TimesScalarVariable::fprop()
{
  real scal = input2->valuedata[0];
  for(int k=0; k<nelems(); k++)
    valuedata[k] = input1->valuedata[k] * scal;
}

void TimesScalarVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    {
      input1->gradientdata[k] += input2->valuedata[0]*gradientdata[k];
      input2->gradientdata[0] += input1->valuedata[k]*gradientdata[k];
    }
}

void TimesScalarVariable::symbolicBprop()
{
  input1->accg(g*input2);
  input2->accg(dot(g,input1));
}

//R(x1x2)=R(x1)x2+x1R(x2)
void TimesScalarVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  real scal = input2->valuedata[0];
  real rscal = input2->rvaluedata[0];
  for(int k=0; k<nelems(); k++)
    rvaluedata[k] = input1->rvaluedata[k] * scal + input1->valuedata[k] * rscal;
}

/** TimesRowVariable **/

TimesRowVariable::TimesRowVariable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isRowVec())
    PLERROR("IN TimesRowVariable: input2 is not a row");
  if(input2->width() != input1->width())
    PLERROR("IN TimesRowVariable: input1 and input2 have a different width()");
}

IMPLEMENT_NAME_AND_DEEPCOPY(TimesRowVariable);

void TimesRowVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "TimesRowVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "TimesRowVariable");
}

void TimesRowVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "TimesRowVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "TimesRowVariable");
}

void TimesRowVariable::fprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      valuedata[k] = input1->valuedata[k] * input2->valuedata[j];
}

void TimesRowVariable::bprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      {
        input1->gradientdata[k] += input2->valuedata[j]*gradientdata[k];
        input2->gradientdata[j] += input1->valuedata[k]*gradientdata[k];
      }
}

void TimesRowVariable::symbolicBprop()
{
  input1->accg(g*input2);
  input2->accg(columnSum(g*input1));
}

void TimesRowVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      rvaluedata[k] = input1->rvaluedata[k] * input2->valuedata[j] + input1->valuedata[k] * input2->rvaluedata[j];
}

/** TimesColumnVariable **/

TimesColumnVariable::TimesColumnVariable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isColumnVec())
    PLERROR("IN TimesColumnVariable: input2 is not a column");
  if(input2->length() != input1->length())
    PLERROR("IN TimesColumnVariable: input1 and input2 have a different length()");
}

IMPLEMENT_NAME_AND_DEEPCOPY(TimesColumnVariable);

void TimesColumnVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "TimesColumnVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "TimesColumnVariable");
}

void TimesColumnVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "TimesColumnVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "TimesColumnVariable");
}

void TimesColumnVariable::fprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      valuedata[k] = input1->valuedata[k] * input2->valuedata[i];
}

void TimesColumnVariable::bprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      {
        input1->gradientdata[k] += input2->valuedata[i]*gradientdata[k];
        input2->gradientdata[i] += input1->valuedata[k]*gradientdata[k];
      }
}

void TimesColumnVariable::symbolicBprop()
{
  input1->accg(g*input2);
  input2->accg(rowSum(g*input1));
}

//???
void TimesColumnVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      rvaluedata[k] = input1->valuedata[k]*input2->rvaluedata[i] + input1->rvaluedata[k]*input2->valuedata[i];
}

/** TimesVariable **/

TimesVariable::TimesVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(input1->length()!=input2->length() || input1->width()!=input2->width())
    PLERROR("In TimesVariable: input1 and input2 must have exactly the same size");
}

IMPLEMENT_NAME_AND_DEEPCOPY(TimesVariable);

void TimesVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "TimesVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "TimesVariable");
}

void TimesVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "TimesVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "TimesVariable");
}

void TimesVariable::fprop()
{
  for(int k=0; k<nelems(); k++)
    valuedata[k] = input1->valuedata[k]*input2->valuedata[k];
}

void TimesVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    {
      input1->gradientdata[k] += input2->valuedata[k]*gradientdata[k];
      input2->gradientdata[k] += input1->valuedata[k]*gradientdata[k];
    }
}

void TimesVariable::symbolicBprop()
{
  input1->accg(g*input2);
  input2->accg(g*input1);
}

/** DivVariable **/

DivVariable::DivVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(input1->length()!=input2->length() || input1->width()!=input2->width())
    PLERROR("In DivVariable: input1 and input2 must have exactly the same shape");
}

IMPLEMENT_NAME_AND_DEEPCOPY(DivVariable);

void DivVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "DivVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "DivVariable");
}

void DivVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "DivVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "DivVariable");
}

void DivVariable::fprop()
{
  for(int k=0; k<nelems(); k++)
    valuedata[k] = input1->valuedata[k]/input2->valuedata[k];
}

void DivVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    {
      real iv2k = 1.0/input2->valuedata[k];
      input1->gradientdata[k] += gradientdata[k] * iv2k;
      input2->gradientdata[k] -= gradientdata[k] * 
        input1->valuedata[k] * iv2k * iv2k;
    }
}

void DivVariable::symbolicBprop()
{
  Var iv2 = invertElements(input2);
  input1->accg(g * iv2);
  input2->accg(-g * input1 * square(iv2));
}

void DivVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  
  for(int k=0; k<nelems(); k++)
    {
      real iv2k = 1.0/input2->valuedata[k];
      rvaluedata[k] = input1->rvaluedata[k] * iv2k - input2->rvaluedata[k] * input1->valuedata[k] * iv2k * iv2k;
    }
}

/** PowVariableVariable **/

/* PowVariableVariable: x^y where x and y are variables but y is scalar 
   or it has the same size as x */

PowVariableVariable::PowVariableVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isScalar() && (input1->length()!=input2->length() || input1->width()!=input2->width()))
    PLERROR("IN FunctionPowVariableVariable(Variable* input1, Variable* input2) input1 and input2 must have the same size or input2 must be scalar");
}
  
IMPLEMENT_NAME_AND_DEEPCOPY(PowVariableVariable);

void PowVariableVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "PowVariableVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "PowVariableVariable");
}

void PowVariableVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "PowVariableVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "PowVariableVariable");
}

void PowVariableVariable::fprop()
{
  if (input2->isScalar())
    {
      real p = input2->valuedata[0];
      for(int i=0; i<nelems(); i++)
        if (input1->valuedata[i]>0)
          valuedata[i] = pow(input1->valuedata[i],p);
        else
          valuedata[i] = 0;
    }
  else
    for(int i=0; i<nelems(); i++)
      if (input1->valuedata[i]>0)
        valuedata[i] = pow(input1->valuedata[i],input2->valuedata[i]);
      else
        valuedata[i] = 0;
}

void PowVariableVariable::bprop()
{
  if (input2->isScalar())
    {
      real p = input2->valuedata[0];
      real& dp = input2->gradientdata[0];
      for(int i=0; i<nelems(); i++)
        {
          if (input1->valuedata[i]>0)
            {
              input1->gradientdata[i] += 
                gradientdata[i] * valuedata[i] * p / input1->valuedata[i];
              dp += gradientdata[i] * valuedata[i] * safeflog(input1->valuedata[i]);
            }
        }
    }
  else
    {
      for(int i=0; i<nelems(); i++)
        {
          if (input1->valuedata[i]>0)
            {
              input1->gradientdata[i] += 
                gradientdata[i] * valuedata[i] * input2->valuedata[i] 
                / input1->valuedata[i];
              input2->gradientdata[i] += 
                gradientdata[i] * valuedata[i] * safeflog(input1->valuedata[i]);
            }
        }
    }
}

void PowVariableVariable::symbolicBprop()
{
  Var gv = g * Var(this);
  Var input1zero = (input1<=0.0);
  Var zero(length(), width());
  input1->accg(ifThenElse(input1zero, zero, gv * input2 / input1));
  if (input2->isScalar())
    input2->accg(dot(gv,ifThenElse(input1zero, zero, log(input1))));
  else
    input2->accg(ifThenElse(input1zero, zero, gv * log(input1)));
}

/** DotProductVariable **/

// Dot product between 2 matrices (or vectors) with same number of elements

DotProductVariable::DotProductVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, 1, 1)
{
  if(input1->nelems() != input2->nelems())
    PLERROR("IN DotProductVariable input1 and input2 must have the same number of elements");
}

IMPLEMENT_NAME_AND_DEEPCOPY(DotProductVariable);

void DotProductVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "DotProductVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "DotProductVariable");
}

void DotProductVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "DotProductVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "DotProductVariable");
}

void DotProductVariable::fprop()
{
  real sum = 0.0;
  for (int k=0; k<input1->nelems(); k++)
    sum += input1->valuedata[k] * input2->valuedata[k];
  valuedata[0] = sum;
}

void DotProductVariable::bprop()
{
  real grad = gradientdata[0];
  for (int k=0; k<input1->nelems(); k++)
    {
      input1->gradientdata[k] += input2->valuedata[k] * grad;
      input2->gradientdata[k] += input1->valuedata[k] * grad;
    }
}

void DotProductVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  real h = diaghessiandata[0];
  for (int k=0; k<input1->nelems(); k++)
    {
      real in2v=input2->valuedata[k];
      input1->diaghessiandata[k] += in2v * in2v * h;
      real in1v=input1->valuedata[k];
      input2->diaghessiandata[k] += in1v * in1v * h;
    }
}

void DotProductVariable::symbolicBprop()
{
  input1->accg(input2*g);
  input2->accg(input1*g);
}

void DotProductVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue(); 
  real sum = 0.0;
  for (int k=0; k<input1->nelems(); k++)
    sum += input1->rvaluedata[k] * input2->valuedata[k] + input1->valuedata[k] * input2->rvaluedata[k];
  rvaluedata[0] = sum;
}

/** ProductVariable **/

// Matrix product
ProductVariable::ProductVariable(Variable* m1, Variable* m2)
  : BinaryVariable(m1, m2, m1->length(), m2->width())
{
  if (m1->width() != m2->length())
    PLERROR("In ProductVariable: the size of m1 and m2 are not compatible for a matrix product");
}

IMPLEMENT_NAME_AND_DEEPCOPY(ProductVariable);

void ProductVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ProductVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ProductVariable");
}

void ProductVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ProductVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ProductVariable");
}

void ProductVariable::fprop()
{
  // m[i,j] = sum_k input1[i,k] * input2[k,j]
  product(matValue, input1->matValue, input2->matValue);
}

void ProductVariable::bprop()
{
  // dC/dinput1[i,k] = sum_j dC/dm[i,j] input2[k,j]
  productTransposeAcc(input1->matGradient, matGradient, input2->matValue);
  // dC/dinput2[k,j] += sum_i dC/dm[i,j] input1[i,k]
  transposeProductAcc(input2->matGradient, input1->matValue, matGradient);
}

void ProductVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  // d^2C/dinput1[i,k]^2 = sum_j d^2C/dm[i,j]^2 input2[k,j]*input2[k,j]
  product2TransposeAcc(input1->matGradient, matGradient, input2->matValue);
  // dC/dinput2[k,j] += sum_i d^2C/dm[i,j]^2 input1[i,k]*input1[i,k]
  transposeProduct2Acc(input2->matGradient, input1->matValue, matGradient);
}

void ProductVariable::symbolicBprop()
{
  // dC/dinput1[i,k] = sum_j dC/dm[i,j] input2[k,j]
  input1->accg(productTranspose(g, input2));
  // dC/dinput2[k,j] += sum_i dC/dm[i,j] input1[i,k]
  input2->accg(transposeProduct(input1, g));
}

//R(x1x2)=R(x1)x2+x1R(x2)
void ProductVariable::rfprop()
{
  if (rValue.length()==0)
    resizeRValue();
  product(matRValue, input1->matValue, input2->matRValue);
  productAcc(matRValue,input1->matRValue, input2->matValue);
}

/** ProductTransposeVariable **/

// Matrix product between matrix1 and transpose of matrix2

ProductTransposeVariable::ProductTransposeVariable(Variable* m1, Variable* m2)
  : BinaryVariable(m1, m2, m1->length(), m2->length())
{
  if (m1->width() != m2->width())
    PLERROR("In ProductVariable: the size of m1 and m2 are not compatible for a matrix product");
}

IMPLEMENT_NAME_AND_DEEPCOPY(ProductTransposeVariable);

void ProductTransposeVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ProductTransposeVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ProductTransposeVariable");
}

void ProductTransposeVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ProductTransposeVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ProductTransposeVariable");
}

void ProductTransposeVariable::fprop()
{
  // m[i,j] = sum_k input1[i,k] * input2[j,k]
  productTranspose(matValue, input1->matValue,input2->matValue);
}

void ProductTransposeVariable::bprop()
{
  // dC/dinput1[i,k] += sum_j dC/dm[i,j] input2[j,k]
  productAcc(input1->matGradient, matGradient,input2->matValue);
  // dC/dinput2[j,k] += sum_i dC/dm[i,j] itnput1[i,k]
  transposeProductAcc(input2->matGradient, matGradient,input1->matValue);
}

void ProductTransposeVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  // d^2C/dinput1[i,k]^2 += sum_j d^2C/dm[i,j]^2 input2[j,k]^2
  product2Acc(input1->matGradient, matGradient,input2->matValue);
  // d^2C/dinput2[j,k]^2 += sum_i d^C/dm[i,j]^2 input1[i,k]^2
  transposeProduct2Acc(input2->matGradient, matGradient,input1->matValue);
}

void ProductTransposeVariable::symbolicBprop()
{
  // dC/dinput1[i,k] += sum_j dC/dm[i,j] input2[j,k]
  input1->accg(product(g, input2));
  // dC/dinput2[j,k] += sum_i dC/dm[i,j] itnput1[i,k]
  input2->accg(transposeProduct(g,input1));
}

void ProductTransposeVariable::rfprop()
{
  if (rValue.length()==0)
    resizeRValue();
  productTranspose(matRValue, input1->matRValue,input2->matValue);
  productTransposeAcc(matRValue, input1->matValue,input2->matRValue);
}

/** TransposeProductVariable **/

// Matrix product between transpose of matrix1 and matrix2
TransposeProductVariable::TransposeProductVariable(Variable* m1, Variable* m2)
  : BinaryVariable(m1, m2, m1->width(), m2->width())
{
  if (m1->length() != m2->length())
    PLERROR("In ProductVariable: the size of m1 and m2 are not compatible for a matrix product");
}

IMPLEMENT_NAME_AND_DEEPCOPY(TransposeProductVariable);

void TransposeProductVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "TransposeProductVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "TransposeProductVariable");
}

void TransposeProductVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "TransposeProductVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "TransposeProductVariable");
}

void TransposeProductVariable::fprop()
{
  // m[i,j] = sum_k input1[k,i] * input2[k,j]
  transposeProduct(matValue, input1->matValue,input2->matValue);
}

void TransposeProductVariable::bprop()
{
  // dC/dinput1[k,i] += sum_j input2[k,j] dC/dm[i,j] 
  productTransposeAcc(input1->matGradient, input2->matValue,matGradient);
  // dC/dinput2[k,j] += sum_i input1[k,i] dC/dm[i,j] 
  productAcc(input2->matGradient, input1->matValue,matGradient);
}

void TransposeProductVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  // d^2C/dinput1[k,i]^2 += sum_j input2[k,j]^2 dC/dm[i,j] 
  squareProductTransposeAcc(input1->matGradient, input2->matValue,matGradient);
  // d^2C/dinput2[k,j]^2 += sum_i input1[k,i]^2 dC/dm[i,j] 
  squareProductAcc(input2->matGradient, input1->matValue,matGradient);
}

void TransposeProductVariable::symbolicBprop()
{
  // dC/dinput1[k,i] += sum_j input2[k,j] dC/dm[i,j] 
  input1->accg(productTranspose(input2,g));
  // dC/dinput2[k,j] += sum_i input1[k,i] dC/dm[i,j] 
  input2->accg(product(input1, g));
}

void TransposeProductVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  // m[i,j] = sum_k input1[k,i] * input2[k,j]
  transposeProduct(matRValue, input1->matRValue,input2->matValue);
  transposeProductAcc(matRValue, input1->matValue,input2->matRValue);
}


/** LogAddVariable **/

LogAddVariable::LogAddVariable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if (input1->length() != input2->length()  ||  input1->width() != input2->width())
    PLERROR("PLogPVariable LogAddVariable input1 and input2 must have the same size");
}

IMPLEMENT_NAME_AND_DEEPCOPY(LogAddVariable);

void LogAddVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "LogAddVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "LogAddVariable");
}

void LogAddVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "LogAddVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "LogAddVariable");
}

void LogAddVariable::fprop()
{
  apply(input1->value,input2->value,value,logadd);
}

void LogAddVariable::bprop()
{
  Vec grad1(nelems());
  grad1 = input1->value - value;
  apply(grad1, grad1, safeexp);
  input1->gradient += grad1%gradient;

  Vec grad2(nelems());
  grad2 = input2->value - value;
  apply(grad2, grad2, safeexp);
  input2->gradient += grad2%gradient;
}

void LogAddVariable::symbolicBprop()
{
  input1->accg(g * (exp(input1)/(exp(input1)+exp(input2))));
  input2->accg(g * (exp(input2)/(exp(input1)+exp(input2))));
}

/** Max2Variable **/

Max2Variable::Max2Variable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if (input1->length() != input2->length()  ||  input1->width() != input2->width())
    PLERROR("IN Max2Variable input1 and input2 must have the same size");
}

IMPLEMENT_NAME_AND_DEEPCOPY(Max2Variable);

void Max2Variable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "Max2Variable");
  inherited::deepRead(in, old2new);
  readFooter(in, "Max2Variable");
}

void Max2Variable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "Max2Variable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "Max2Variable");
}


void Max2Variable::fprop()
{
  int n=input1->value.length();
  real* v1=input1->value.data();
  real* v2=input2->value.data();
  real* v=value.data();
  for (int i=0;i<n;i++)
    v[i] = std::max(v1[i],v2[i]);
}

void Max2Variable::bprop()
{
  int n=input1->value.length();
  real* v1=input1->value.data();
  real* v2=input2->value.data();
  real* grad1=input1->gradient.data();
  real* grad2=input2->gradient.data();
  real* grad=gradient.data();
  for (int i=0;i<n;i++)
  {
    if (v2[i]<v1[i])
      grad1[i] += grad[i];
    if (v1[i]<v2[i])
      grad2[i] += grad[i];
  }
}

void Max2Variable::symbolicBprop()
{
  input1->accg((input2<input1)*g);
  input2->accg((input1<input2)*g);
}



/** EqualVariable **/

EqualVariable::EqualVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, 1,1)
{
  if(input1->length() != input2->length()  ||  input1->width() != input2->width())
    PLERROR("IN EqualVariable(Variable* input1, Variable* input2) input1 and input2 must have the same size");
}
  
IMPLEMENT_NAME_AND_DEEPCOPY(EqualVariable);

void EqualVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "EqualVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "EqualVariable");
}

void EqualVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "EqualVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "EqualVariable");
}


void EqualVariable::fprop()
{
  bool equal=true;
  for (int i=0; i<nelems(); i++)
    equal = equal && (input1->valuedata[i] == input2->valuedata[i]);
  valuedata[0]=equal;
}

// not really differentiable (zero gradient almost everywhere)
void EqualVariable::bprop() {}
void EqualVariable::symbolicBprop() {}

/** IsLargerVariable **/

IsLargerVariable::IsLargerVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(input1->length() != input2->length() || input1->width() != input2->width())
    PLERROR("EqualVariable IsLargerVariable(Variable* input1, Variable* input2), the size of input1 and input2 are inconsistent");
}

IMPLEMENT_NAME_AND_DEEPCOPY(IsLargerVariable);

void IsLargerVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "IsLargerVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "IsLargerVariable");
}

void IsLargerVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "IsLargerVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "IsLargerVariable");
}

void IsLargerVariable::fprop()
{
  for(int i = 0; i <nelems(); i++)
    valuedata[i] = input1->valuedata[i] > input2->valuedata[i];
}

// not really differentiable (zero gradient almost everywhere)
void IsLargerVariable::bprop() {}
void IsLargerVariable::symbolicBprop() {}

/** IsSmallerVariable **/

IsSmallerVariable::IsSmallerVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, input1->length(), input2->width())
{
  if(input1->length() != input2->length() || input1->width() != input2->width())
    PLERROR("In IsSmallerVariable(Variable* input1, Variable* input2), the size of input1 and input2 are inconsistent");
}

IMPLEMENT_NAME_AND_DEEPCOPY(IsSmallerVariable);

void IsSmallerVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "IsSmallerVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "IsSmallerVariable");
}

void IsSmallerVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "IsSmallerVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "IsSmallerVariable");
}

void IsSmallerVariable::fprop()
{
  for(int i = 0; i<nelems(); i++)
    valuedata[i] = input1->valuedata[i] < input2->valuedata[i];
}

// not really differentiable (zero gradient almost everywhere)
void IsSmallerVariable::bprop() {}
void IsSmallerVariable::symbolicBprop() {}

/** EqualScalarVariable **/

EqualScalarVariable::EqualScalarVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(!input2->isScalar())
    PLERROR("IsSmallerVariable EqualScalarVariable(Variable* input1, Variable* input2) input2 must be a scalar");
}
  
IMPLEMENT_NAME_AND_DEEPCOPY(EqualScalarVariable);

void EqualScalarVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "EqualScalarVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "EqualScalarVariable");
}

void EqualScalarVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "EqualScalarVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "EqualScalarVariable");
}


void EqualScalarVariable::fprop()
{
  real eqv = input2->valuedata[0];
  for(int i=0; i<nelems(); i++)
    valuedata[i] = (input1->valuedata[i] == eqv);
}

// not really differentiable (zero gradient almost everywhere)
void EqualScalarVariable::bprop() {}
void EqualScalarVariable::symbolicBprop() {}



/** ConvolveVariable **/

ConvolveVariable::ConvolveVariable(Variable* input, Variable* mask)
  : BinaryVariable(input, mask, input->length()-mask->length()+1, input->width()-mask->width()+1)
{}

IMPLEMENT_NAME_AND_DEEPCOPY(ConvolveVariable);

void ConvolveVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ConvolveVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ConvolveVariable");
}

void ConvolveVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ConvolveVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ConvolveVariable");
}


void ConvolveVariable::fprop()
{
  convolve(input1->matValue, input2->matValue, matValue);
}

void ConvolveVariable::bprop()
{
  for(int i=0; i<length(); i++) // size of matGradient
    for(int j=0; j<width(); j++)
      {
        real* input1valueptr = input1->matValue[i]+j;
        real* input2valueptr = input2->matValue.data();
        
        real thisgradient = matGradient(i,j);
        real* input1gradientptr = input1->matGradient[i]+j;
        real* input2gradientptr = input2->matGradient.data();
        
        for(int l=0; l<input2->length(); l++,
              input1valueptr += input1->matValue.mod(), input2valueptr += input2->matValue.mod(),
              input1gradientptr += input1->matGradient.mod(), input2gradientptr += input2->matGradient.mod())
          for(int c=0; c<input2->width(); c++)
            {
              input1gradientptr[c] += thisgradient * input2valueptr[c];
              input2gradientptr[c] += thisgradient * input1valueptr[c];
            }
      }
}

void ConvolveVariable::symbolicBprop()
{ PLERROR("ConvolveVariable::symbolicBprop() not yet implemented"); }

IMPLEMENT_NAME_AND_DEEPCOPY(AffineTransformVariable);
void AffineTransformVariable::fprop()
  {
    value << input2->matValue.firstRow();
    Mat lintransform = input2->matValue.subMatRows(1,input2->length()-1);
    transposeProductAcc(value, lintransform, input1->value);
  }

void AffineTransformVariable::bprop()
  {
    Mat&  afftr = input2->matValue;
    int l = afftr.length();
    // Vec bias = afftr.firstRow();
    Mat lintr = afftr.subMatRows(1,l-1);

    Mat& afftr_g = input2->matGradient;
    Vec bias_g = afftr_g.firstRow();
    Mat lintr_g = afftr_g.subMatRows(1,l-1);

    bias_g += gradient;    
    if(!input1->dont_bprop_here)      
      productAcc(input1->gradient, lintr, gradient);
    externalProductAcc(lintr_g, input1->value, gradient);
  }

void AffineTransformVariable::symbolicBprop()
  {
   PLERROR("AffineTransformVariable::symbolicBprop() not implemented");
  }

IMPLEMENT_NAME_AND_DEEPCOPY(MatrixAffineTransformVariable);
void MatrixAffineTransformVariable::fprop()
  {
    Mat lintransform = input2->matValue.subMatRows(1,input2->length()-1);
    // matValue << input2->matValue.firstRow();
    for (int i = 0; i < length(); i++)
        for (int j = 0; j < width(); j++)
             matValue[i][j] = input2->matValue[0][i];
    //matValue += transposeProduct(lintransform, input1->matValue);
    transposeProductAcc(matValue,lintransform, input1->matValue);
  }

void MatrixAffineTransformVariable::bprop()
  {
    Mat&  afftr = input2->matValue;
    int l = afftr.length();
    //Vec bias = afftr.firstRow();
    Mat lintr = afftr.subMatRows(1,l-1);

    Mat& afftr_g = input2->matGradient;
    Vec bias_g = afftr_g.firstRow();
    Mat lintr_g = afftr_g.subMatRows(1,l-1);

    for (int i = 0; i < length(); i++)
        for (int j = 0; j < width(); j++)
            {
            bias_g[i] += matGradient[i][j];
            }
    if(!input1->dont_bprop_here)      
      productAcc(input1->matGradient, lintr, matGradient);
    productTransposeAcc(lintr_g, input1->matValue, matGradient);
  }

void MatrixAffineTransformVariable::symbolicBprop()
  {
   PLERROR("MatrixAffineTransformVariable::symbolicBprop() not implemented");
  }

/** OneHotSquaredLoss **/

IMPLEMENT_NAME_AND_DEEPCOPY(OneHotSquaredLoss);
OneHotSquaredLoss::OneHotSquaredLoss(Variable* netout, Variable* classnum, real coldval, real hotval)
    :BinaryVariable(netout,classnum,1,1), coldval_(coldval), hotval_(hotval)
{
  if(!classnum->isScalar())
    PLERROR("In OneHotSquaredLoss: classnum must be a scalar variable representing an index of netout (typically a classnum)");
}
  
void OneHotSquaredLoss::fprop()
{
  real* netout = input1->valuedata;
  int n = input1->value.size();
  int classnum = (int) input2->valuedata[0];
  real res = 0.;
  for(int i=0; i<n; i++)
    res += square(*netout++ - (i==classnum ? hotval_ : coldval_));
  *valuedata = res;
}

void OneHotSquaredLoss::bprop()
{
  real gr = *gradientdata;
  real* netout = input1->valuedata;
  int n = input1->value.size();
  int classnum = (int) input2->valuedata[0];
  real* input1grptr = input1->gradientdata;
  if(gr!=1.)
  {
    gr = gr+gr;
    for(int i=0; i<n; i++)
      *input1grptr++ += gr*(*netout++ - (i==classnum ? hotval_ : coldval_));
  }
  else // specialised version for gr==1
  {
    for(int i=0; i<n; i++)
      input1->gradientdata[i] += two(netout[i] - (i==classnum ? hotval_ : coldval_));        
  }
}

void OneHotSquaredLoss::symbolicBprop()
{
  Var gi =  g*(input1 - coldval_);
  Var gindex = new RowAtPositionVariable(g*(coldval_-hotval_), input2, input1->length());
  Var ginput = gi + gindex;
  input1->accg(ginput+ginput); //2*gi
}

void OneHotSquaredLoss::rfprop()
{
  int n = input1->value.size();
  int classnum = (int) input2->valuedata[0];
  real sum = 0;
  for (int i=0; i<n; i++)
       sum += 2 * input1->rvaluedata[i] * (input1->valuedata[i] - (i==classnum ? hotval_ : coldval_));
  rvaluedata[0] = sum;
}

/** MatrixOneHotSquaredLoss **/

IMPLEMENT_NAME_AND_DEEPCOPY(MatrixOneHotSquaredLoss);
MatrixOneHotSquaredLoss::MatrixOneHotSquaredLoss(Variable* input1, Variable* input2, real coldval, real hotval)
    :BinaryVariable(input1,input2,input2->length(),input2->width()), coldval_(coldval), hotval_(hotval)
{
  if(!input2->isVec())
    PLERROR("In MatrixOneHotSquaredLoss: classnum must be a vector variable representing the indexs of netouts (typically some classnums)");
}

void MatrixOneHotSquaredLoss::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MatrixOneHotSquaredLoss");
  inherited::deepRead(in, old2new);
  readFooter(in, "MatrixOneHotSquaredLoss");
}

void MatrixOneHotSquaredLoss::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MatrixOneHotSquaredLoss");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MatrixOneHotSquaredLoss");
}
  
void MatrixOneHotSquaredLoss::fprop()
{
  int n = input1->length();
  for (int k=0; k<length(); k++)
   {
    int classnum = (int) input2->valuedata[k];
    real res = 0.;
    for(int i=0; i<n; i++)
        res += square(input1->matValue[i][k] - (i==classnum ? hotval_ : coldval_));
    valuedata[k] = res;
    }
}

void MatrixOneHotSquaredLoss::bprop()
{
  int n = input1->length();
  for(int k=0; k<length(); k++)
     {
     real gr = gradientdata[k];
     int classnum = (int) input2->valuedata[k];
     if (gr!=1.)
        {
        gr = gr+gr;
        for (int i=0; i<n; i++)
           input1->matGradient[i][k] += gr*(input1->matValue[i][k] - (i==classnum ? hotval_ : coldval_));
        }
        else // specialised version for gr==1
           {
           for (int i=0; i<n; i++)
               input1->matGradient[i][k] += two(input1->matValue[i][k] - (i==classnum ? hotval_ : coldval_));
           }
     }
}

/** ClassificationLossVariable **/

IMPLEMENT_NAME_AND_DEEPCOPY(ClassificationLossVariable);
ClassificationLossVariable::ClassificationLossVariable(Variable* netout, Variable* classnum)
  :BinaryVariable(netout,classnum,1,1)
{
  if(!classnum->isScalar())
    PLERROR("In ClassificationLossVariable: classnum must be a scalar variable representing an index of netout (typically a class number)");
}

void ClassificationLossVariable::fprop()
{
  int topscorepos = argmax(input1->value);
  int classnum = int(input2->valuedata[0]);
  valuedata[0] = (topscorepos==classnum ?0 :1);
}


/** MulticlassLossVariable **/

IMPLEMENT_NAME_AND_DEEPCOPY(MulticlassLossVariable);
MulticlassLossVariable::MulticlassLossVariable(Variable* netout, Variable* target)
  :BinaryVariable(netout,target,1,1)
{
  if(netout->size() != target->size())
    PLERROR("In MulticlassLossVariable: netout and target must the same size");
}

void MulticlassLossVariable::fprop()
{
  real cost = 0.0;
  for (int i=0; i<input1->size(); i++)
  {
    real output = input1->valuedata[i];
    real target = input2->valuedata[i];
    cost += (target==1) ? output<0.5 : output>0.5;
  }
  valuedata[0] = cost;
}


/** CrossEntropyVariable **/

IMPLEMENT_NAME_AND_DEEPCOPY(CrossEntropyVariable);
CrossEntropyVariable::CrossEntropyVariable(Variable* netout, Variable* target)
  :BinaryVariable(netout,target,1,1)
{
  if(netout->size() != target->size())
    PLERROR("In CrossEntropyVariable: netout and target must the same size");
}

void CrossEntropyVariable::fprop()
{
  real cost = 0.0;
  for (int i=0; i<input1->size(); i++)
  {
    real output = input1->valuedata[i];
    real target = input2->valuedata[i];
    cost += target*log(output) + (1.0-target)*log(1.0-output);
  }
  valuedata[0] = -cost;
}

void CrossEntropyVariable::bprop()
{
  real gr = *gradientdata;
  for (int i=0; i<input1->size(); i++)
  {
    real output = input1->valuedata[i];
    real target = input2->valuedata[i];
    input1->gradientdata[i] += gr*(-target/output + (1.0-target)/(1.0-output));
  }
}

/** SoftmaxLossVariable **/
IMPLEMENT_NAME_AND_DEEPCOPY(SoftmaxLossVariable);
SoftmaxLossVariable::SoftmaxLossVariable(Variable* input1, Variable* input2) 
:BinaryVariable(input1, input2, 1, 1)
{
  if(!input2->isScalar())
    PLERROR("In RowAtPositionVariable: position must be a scalar");
}

void SoftmaxLossVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "SoftmaxLossVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "SoftmaxLossVariable");
}

void SoftmaxLossVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "SoftmaxLossVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "SoftmaxLossVariable");
}

void SoftmaxLossVariable::fprop()
{
  int classnum = (int)input2->valuedata[0];
  real input_index = input1->valuedata[classnum];
  real sum=0;
  for(int i=0; i<input1->nelems(); i++)
    sum += safeexp(input1->valuedata[i]-input_index);
  valuedata[0] = 1.0/sum;
}

void SoftmaxLossVariable::bprop()
{
  int classnum = (int)input2->valuedata[0];
  real input_index = input1->valuedata[classnum];
  real vali = valuedata[0];
  for(int i=0; i<input1->nelems(); i++)
  {
    if (i!=classnum)
       input1->gradientdata[i] = -gradientdata[i]/*?*/*vali*vali*safeexp(input1->valuedata[i]-input_index);
    else
       input1->gradientdata[i] = gradientdata[i]*vali*(1.-vali);
  }
}

void SoftmaxLossVariable::bbprop()
{
  PLERROR("SofmaxVariable::bbprop() not implemented");
}

void SoftmaxLossVariable::symbolicBprop()
{
  Var gi = -g * Var(this) * Var(this) * exp(input1-input1(input2));
  Var gindex = new RowAtPositionVariable(g * Var(this), input2, input1->length());
  input1->accg(gi+gindex);
}

// R{ s_i = exp(x_i) / sum_j exp(x_j) }   = (s_i(1-s_i) - sum_{k!=i} s_i s_k) R(s_i) = s_i ((1-s_i) - sum_{k!=i} s_k) R(s_i)
void SoftmaxLossVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();

  int classnum = (int)input2->valuedata[0];
  real input_index = input1->valuedata[classnum];
  real vali = valuedata[0];
  real sum = 0;
  for(int i=0; i<input1->nelems(); i++)
  {
    real res =vali * input1->rvaluedata[i];
    if (i != classnum)
       sum -= res * vali* safeexp(input1->valuedata[i]-input_index);
    else sum += res * (1 - vali);
  }
  rvaluedata[0] = sum;
}

/** MatrixSoftmaxLossVariable **/
IMPLEMENT_NAME_AND_DEEPCOPY(MatrixSoftmaxVariable);
MatrixSoftmaxVariable::MatrixSoftmaxVariable(Variable* input1, Variable* input2) 
:BinaryVariable(input1, input2, input2->length(), input2->width())
{
  if(!input2->isVec())
    PLERROR("In MatrixSoftmaxVariable: position must be a vector");
}

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
  for (int i=0; i<input2->length(); i++)
  {
    int classnum = (int)input2->valuedata[i];
    real input_index = input1->matValue[classnum][i];
    real sum=0;
    for(int j=0; j<input1->length(); j++)
        sum += safeexp(input1->matValue[j][i]-input_index);
    valuedata[i] = 1.0/sum;
  }
}

void MatrixSoftmaxVariable::bprop()
{  
  for (int i=0; i<input2->length(); i++)
  {
    int classnum = (int)input2->valuedata[i];
    real input_index = input1->matValue[classnum][i];
    real vali = valuedata[i];
    for(int j=0; j<input1->length(); j++)
      {
       if (j!=classnum){
          input1->matGradient[j][i] = -gradientdata[i]*vali*vali*safeexp(input1->matValue[j][i]-input_index);}
          else
          input1->matGradient[j][i] = gradientdata[i]*vali*(1.-vali);
      }
  }
}

void MatrixSoftmaxVariable::bbprop()
{
  PLERROR("MatrixSoftmaxVariable::bbprop() not implemented");
}

void MatrixSoftmaxVariable::symbolicBprop()
{
  PLERROR("MatrixSoftmaxVariable::symbolicBprop() not implemented");
}

void MatrixSoftmaxVariable::rfprop()
{
  PLERROR("MatrixSoftmaxVariable::rfprop() not implemented");
}



/** WeightedSumSquareVariable **/


IMPLEMENT_NAME_AND_DEEPCOPY(WeightedSumSquareVariable);
WeightedSumSquareVariable::WeightedSumSquareVariable(Variable* input, Variable* weights)
  :BinaryVariable(input,weights,1,1)
{
  if(input->nelems() != weights->nelems())
    PLERROR("In WeightedSumSquareVariable: input and weights must be the same size;"
	    " input->nelems()=%d weights->nelems()=&d.",
	    input->nelems(), weights->nelems());
}


void WeightedSumSquareVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "WeightedSumSquareVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "WeightedSumSquareVariable");
}

void WeightedSumSquareVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "WeightedSumSquareVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "WeightedSumSquareVariable");
}

void WeightedSumSquareVariable::fprop()
{
  int n=input1->nelems();
  *valuedata= 0;
  for(int i=0; i<n; i++)
    *valuedata+= input1->valuedata[i]*input1->valuedata[i] * input2->valuedata[i];
}

void WeightedSumSquareVariable::bprop()
{
  int n=input1->nelems();
  for(int i=0; i<n; i++)
    {
      input1->gradientdata[i]+= 2.0 * input1->valuedata[i] * input2->valuedata[i] * *gradientdata;
      input2->gradientdata[i]+= input1->valuedata[i] * input1->valuedata[i] * *gradientdata;
    }
}

void WeightedSumSquareVariable::symbolicBprop()
{
  input1->accg(2.0 * (g*input1*input2));
  input2->accg(g*input1*input1);
}


%> // end of namespace PLearn




