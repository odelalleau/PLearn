

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
   * $Id: NaryVariable.cc,v 1.3 2002/09/23 20:31:11 wangxian Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "NaryVariable.h"
#include "Var.h"
#include "TMat_maths.h"
#include "PLMPI.h"
#include "DisplayUtils.h"
#include "pl_erf.h"

namespace PLearn <%
using namespace std;

/** NaryVariable **/

NaryVariable::NaryVariable(const VarArray& the_varray, int thelength, int thewidth)
  :Variable(thelength,thewidth), varray(the_varray) {}

IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY(NaryVariable);

void NaryVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Variable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(varray, copies);
  //for(int i=0; i<varray.size(); i++)
  //  deepCopyField(varray[i], copies);
}

void NaryVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "NaryVariable");
  Variable::deepRead(in, old2new);
  PLearn::deepRead(in, old2new, varray);
  readFooter(in, "NaryVariable");
}

void NaryVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "NaryVariable");
  Variable::deepWrite(out, already_saved);
  PLearn::deepWrite(out, already_saved, varray);
  writeFooter(out, "NaryVariable");
}

bool NaryVariable::markPath()
{
  if(!marked)
    {
      for(int i=0; i<varray.size(); i++)
        if (!varray[i].isNull())
          marked |= varray[i]->markPath();
    }
  return marked;
}

void NaryVariable::buildPath(VarArray& proppath)
{
  if(marked)
    {
      for(int i=0; i<varray.size(); i++)
        if (!varray[i].isNull())
          varray[i]->buildPath(proppath);
      proppath &= Var(this);
      clearMark();
    }
}

VarArray NaryVariable::sources()
{
  VarArray a(0,0);
  if (!marked)
    {
      marked = true;
      for(int i=0; i<varray.size(); i++)
        if (!varray[i].isNull())
          a &= varray[i]->sources();
      if (a.size()==0)
        a &= Var(this);
    }
  return a;
}

VarArray NaryVariable::random_sources()
{
  VarArray a(0,0);
  if (!marked)
    {
      marked = true;
      for(int i=0; i<varray.size(); i++)
        if (!varray[i].isNull())
          a &= varray[i]->random_sources();
    }
  return a;
}

VarArray NaryVariable::ancestors() 
{ 
  VarArray a(0,0);
  if (marked)
    return a;
  marked = true;
  for(int i=0; i<varray.size(); i++)
    if (!varray[i].isNull())
      a &= varray[i]->ancestors();
  a &= Var(this);
  return a;
}

void NaryVariable::unmarkAncestors()
{
  if (marked)
    {
      marked = false;
      for(int i=0; i<varray.size(); i++)
        if (!varray[i].isNull())
          varray[i]->unmarkAncestors();
    }
}

VarArray NaryVariable::parents()
{
  VarArray unmarked_parents;
  for(int i=0; i<varray.size(); i++)
    if (!varray[i].isNull() && !varray[i]->marked)
      unmarked_parents.append(varray[i]);
  return unmarked_parents;
}

void NaryVariable::resizeRValue()
{
  inherited::resizeRValue();
  for (int i=0; i<varray.size(); i++)
    if (!varray[i]->rvaluedata) varray[i]->resizeRValue();
}


/** ConcatRowsVariable **/
ConcatRowsVariable::ConcatRowsVariable(const VarArray& vararray)
    :NaryVariable(vararray.nonNull(), vararray.sumOfLengths(), vararray.maxWidth())
{
  // all the variables must have the same width
  int w = varray[0]->width();
  for (int i=1; i<varray.size(); i++)
    if (w!=varray[i]->width())
      PLERROR("ConcatRowsVariable: all non-null variables must have the same width");
}

IMPLEMENT_NAME_AND_DEEPCOPY(ConcatRowsVariable);
void ConcatRowsVariable::recomputeSize(int& l, int& w) const
{ l=varray.sumOfLengths(); w=varray.maxWidth(); }

void ConcatRowsVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ConcatRowsVariable");
  NaryVariable::deepRead(in, old2new);
  readFooter(in, "ConcatRowsVariable");
}

void ConcatRowsVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ConcatRowsVariable");
  NaryVariable::deepWrite(out, already_saved);
  writeFooter(out, "ConcatRowsVariable");
}

void ConcatRowsVariable::fprop()
{
  int k=0;
  for (int n=0; n<varray.size(); n++) {
    Var vn = varray[n];
    for (int i=0; i<vn->nelems(); i++, k++)
      valuedata[k] = vn->valuedata[i];
  }
}

void ConcatRowsVariable::bprop()
{
  int k=0;
  for (int n=0; n<varray.size(); n++) {
    Var vn = varray[n];
    for (int i=0; i<vn->nelems(); i++, k++)
      vn->gradientdata[i] += gradientdata[k];
  }
}

void ConcatRowsVariable::symbolicBprop()
{
  int k=0;
  for (int n=0; n<varray.size(); n++) {
    Var vn = varray[n];
    vn->accg(new SubMatVariable(g, k, 0, vn->length(), width()));
    k += vn->length();
  }
}

void ConcatRowsVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  int k=0;
  for (int n=0; n<varray.size(); n++) {
    Var vn = varray[n];
    for (int i=0; i<vn->nelems(); i++, k++)
      rvaluedata[k] = vn->rvaluedata[i];
  }
}

/** ConcatColumnsVariable **/
ConcatColumnsVariable::ConcatColumnsVariable(const VarArray& vararray)
    :NaryVariable(vararray.nonNull(), vararray.maxLength(), vararray.sumOfWidths())
{
  int l=varray[0]->length();
  for (int i=1; i<varray.size(); i++)
    if (l!=varray[i]->length())
      PLERROR("ConcatColumnsVariable: all non-null variables must have the same length");
}

IMPLEMENT_NAME_AND_DEEPCOPY(ConcatColumnsVariable);
void ConcatColumnsVariable::recomputeSize(int& l, int& w) const
{ l=varray.maxLength(); w=varray.sumOfWidths(); }

void ConcatColumnsVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ConcatColumnsVariable");
  NaryVariable::deepRead(in, old2new);
  readFooter(in, "ConcatColumnsVariable");
}

void ConcatColumnsVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ConcatColumnsVariable");
  NaryVariable::deepWrite(out, already_saved);
  writeFooter(out, "ConcatColumnsVariable");
}

void ConcatColumnsVariable::fprop()
{
  int n_rows = matValue.length();
  int m_start = 0;
  int mod = matValue.mod();
  for (int m=0;m<varray.size();m++)
    {
      real* mp = varray[m]->valuedata;
      int n_cols = varray[m]->matValue.width();
      real* p = &valuedata[m_start];
      for (int i=0;i<n_rows;i++,p+=mod)
        for (int j=0;j<n_cols;j++,mp++)
          p[j] = *mp;
      m_start+=n_cols;
    }
}

void ConcatColumnsVariable::bprop()
{
  int n_rows = matValue.length();
  int m_start = 0;
  int mod = matValue.mod();
  for (int m=0;m<varray.size();m++)
    {
      real* mp = varray[m]->gradientdata;
      int n_cols = varray[m]->matGradient.width();
      real* p = &gradientdata[m_start];
      for (int i=0;i<n_rows;i++,p+=mod)
        for (int j=0;j<n_cols;j++,mp++)
          *mp += p[j];
      m_start+=n_cols;
    }
}

void ConcatColumnsVariable::symbolicBprop()
{
  int k=0;
  for (int n=0; n<varray.size(); n++) {
    Var vn = varray[n];
    vn->accg(new SubMatVariable(g, 0, k, length(), vn->width()));
    k += vn->width();
  }
}


/** SumOfVariable **/

SumOfVariable::SumOfVariable(VMat the_distr, Func the_f, int the_nsamples)
  :NaryVariable(nonInputParentsOfPath(the_f->inputs,the_f->outputs), 
                the_f->outputs[0]->length(), 
                the_f->outputs[0]->width()),
  distr(the_distr), f(the_f), nsamples(the_nsamples), curpos(0),
  input_value(the_distr->width()), input_gradient(the_distr->width()),
  output_value(the_f->outputs[0]->size())
{
  if(f->outputs.size()!=1)
    PLERROR("In SumOfVariable: function must have a single variable output (maybe you can vconcat the vars into a single one prior to calling sumOf, if this is really what you want)");

  if(nsamples==-1)
    nsamples = distr->length();
  f->inputs.setDontBpropHere(true);
}

IMPLEMENT_NAME_AND_DEEPCOPY(SumOfVariable);
void SumOfVariable::recomputeSize(int& l, int& w) const
{ l=f->outputs[0]->length(); w=f->outputs[0]->width(); }

void SumOfVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  NaryVariable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(distr, copies);
  deepCopyField(f, copies);
}

void SumOfVariable::fprop()
{
  f->recomputeParents();

  if(nsamples==1)
  {
    distr->getRow(curpos, input_value);
    f->fprop(input_value, value);
    if(++curpos == distr->length())
      curpos = 0;
  }
  else
  {
    value.clear();
#if USING_MPI
    if (nsamples > distr->length())
      PLERROR("In SumOfVariable::fprop, the case where nsamples is greater than distr->length is not supported in parallel computation");
    int nb_sample = nsamples/PLMPI::size;
    int start_pos = PLMPI::rank * nb_sample;
    int end_pos = (PLMPI::rank==PLMPI::size-1) ? nsamples : start_pos + nb_sample;
    Vec dummy_value(value.length());
    for(int i=start_pos; i<end_pos; i++)
    {
      distr->getRow(i, input_value);
      f->fprop(input_value, output_value);
      dummy_value += output_value;
    }
    MPI_Allreduce(dummy_value.data(), value.data(), value.length(), PLMPI_REAL, MPI_SUM, MPI_COMM_WORLD);
#else
    for(int i=0; i<nsamples; i++)
    {
      distr->getRow(curpos, input_value);
      f->fprop(input_value, output_value);
      value += output_value;
      if(++curpos == distr->length())
        curpos = 0;
    }
#endif
  }
}

void SumOfVariable::bprop()
{ fbprop(); }

void SumOfVariable::fbprop()
{
  f->recomputeParents();
  
  if(nsamples==1)
  {
    distr->getRow(curpos, input_value);
    //displayFunction(f, true, false, 250);
    f->fbprop(input_value, value, input_gradient, gradient);
    //displayFunction(f, true, false, 250);
    if(++curpos == distr->length()) 
      curpos = 0;
  }
  else
  {
    value.clear();
#if USING_MPI
    if (nsamples > distr->length())
      PLERROR("In SumOfVariable::fbprop, the case where nsamples is greater than distr->length is not supported in parallel computation");
    int nb_sample = nsamples/PLMPI::size;
    int start_pos = PLMPI::rank * nb_sample;
    int end_pos = (PLMPI::rank==PLMPI::size-1) ? nsamples : start_pos + nb_sample;
    Vec dummy_value(value.length());
    for(int i=start_pos; i<end_pos; i++)
    {
      distr->getRow(i, input_value);
      f->fbprop(input_value, output_value, input_gradient, gradient);
      dummy_value += output_value;
    }
    MPI_Allreduce(dummy_value.data(), value.data(), value.length(), PLMPI_REAL, MPI_SUM, MPI_COMM_WORLD);
    VarArray params = f->parameters;
    for (int i=0; i<params->length(); i++)
    {
      Vec buffer(params[i]->size());
      MPI_Reduce(params[i]->gradientdata, buffer.data(), buffer.length(), PLMPI_REAL, MPI_SUM, 0, MPI_COMM_WORLD);
      buffer >> params[i]->gradient;
      MPI_Bcast(params[i]->gradientdata, buffer.length(), PLMPI_REAL, 0, MPI_COMM_WORLD);
    }
#else
    for(int i=0; i<nsamples; i++)
    {
      distr->getRow(curpos, input_value);
      //displayFunction(f, true, false, 250);
      f->fbprop(input_value, output_value, input_gradient, gradient);
      value += output_value;
      if(++curpos == distr->length()) 
        curpos = 0;
    }
#endif
  }
}

void SumOfVariable::symbolicBprop()
{
  /*
  // f is a function of its inputs, what we want is a function of the parameters of f (which are in the inputs field of this SumOfVariable)
  VarArray& params = varray; 
  int nparams = params.size();
  f->bproppath.symbolicBprop();

  VarArray dparams(nparams);    
  for(int i=0; i<nparams; i++)
    dparams[i] = params[i]->g;

  Var dparams_concat = new ConcatElementsVariable(dparams);
  Var dparams_sum = new SumOfVariable(distr, Func(params,dparams_concat), nsamples);

  for(int i=0; i<nparams; i++)
    params[i]->g += dparams_sum.sub(...)
  */
}

void SumOfVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  // TODO... (we will need a rfprop() in Func)
  
//    f->recomputeParents();
  
//    if(nsamples==1)
//    {
//      distr->getRow(curpos, input_value);
//      f->fprop(input_value, value);
//      if(++curpos == distr->length())
//        curpos = 0;
//    }
//    else
//    {
//      value.clear();
//  #if USING_MPI
//      if (nsamples > distr->length())
//        PLERROR("In SumOfVariable::fprop, the case where nsamples is greater than distr->length is not supported in parallel computation");
//      int nb_sample = nsamples/PLMPI::size;
//      int start_pos = PLMPI::rank * nb_sample;
//      int end_pos = (PLMPI::rank==PLMPI::size-1) ? nsamples : start_pos + nb_sample;
//      Vec dummy_value(value.length());
//      for(int i=start_pos; i<end_pos; i++)
//      {
//        distr->getRow(i, input_value);
//        f->fprop(input_value, output_value);
//        dummy_value += output_value;
//      }
//      MPI_Allreduce(dummy_value.data(), value.data(), value.length(), PLMPI_REAL, MPI_SUM, MPI_COMM_WORLD);
//  #else
//      for(int i=0; i<nsamples; i++)
//      {
//        distr->getRow(curpos, input_value);
//        f->fprop(input_value, output_value);
//        value += output_value;
//        if(++curpos == distr->length())
//          curpos = 0;
//      }
//  #endif
//    }
}

void SumOfVariable::printInfo(bool print_gradient)
{
  Vec input_value(distr->width());
  Vec input_gradient(distr->width());
  Vec output_value(nelems());

  f->recomputeParents();
  value.clear();

  for(int i=0; i<nsamples; i++)
  {
    distr->getRow(curpos++,input_value);
    if (print_gradient)
      f->fbprop(input_value, output_value, input_gradient, gradient);
    else
      f->fprop(input_value, output_value);
    value += output_value;
    if(curpos>=distr->length())
      curpos = 0;
    f->fproppath.printInfo(print_gradient);
  }
  cout << info() << " : " << getName() << " = " << value;
  if (print_gradient) cout << " gradient=" << gradient;
  cout << endl; 
}

/** MatrixSumOfVariable **/

MatrixSumOfVariable::MatrixSumOfVariable(VMat the_distr, Func the_f, int the_nsamples, int the_input_size)
  :NaryVariable(nonInputParentsOfPath(the_f->inputs,the_f->outputs), 
                the_f->outputs[0]->length(), 
                the_f->outputs[0]->width()),
  distr(the_distr), f(the_f), nsamples(the_nsamples), input_size(the_input_size), curpos(0),
  input_value(the_distr->width()*nsamples), input_gradient(the_distr->width()*nsamples),
  output_value(the_f->outputs[0]->size())
  
{
  if(f->outputs.size()!=1)
    PLERROR("In MatrixSumOfVariable: function must have a single variable output (maybe you can vconcat the vars into a single one prior to calling sumOf, if this is really what you want)");

  if(nsamples==-1)
    nsamples = distr->length();
  f->inputs.setDontBpropHere(true);
}

IMPLEMENT_NAME_AND_DEEPCOPY(MatrixSumOfVariable);
void MatrixSumOfVariable::recomputeSize(int& l, int& w) const
{ l=f->outputs[0]->length(); w=f->outputs[0]->width(); }

void MatrixSumOfVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  NaryVariable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(distr, copies);
  deepCopyField(f, copies);
}

void MatrixSumOfVariable::fprop()
{
  Vec oneInput_value(distr->width());
  f->recomputeParents();

  int inputpos=0;
  int targetpos=nsamples*input_size;
  for (int i=0; i<nsamples; i++)
    {
    distr->getRow(curpos, oneInput_value);
    for (int j=0; j<input_size; j++,inputpos++)
      input_value[inputpos]=oneInput_value[j];
    for (int j=input_size; j<distr.width(); j++,targetpos++)
      input_value[targetpos] = oneInput_value[j];
    if(++curpos==distr.length())
      curpos=0;
    }
  f->fprop(input_value, value);
}

void MatrixSumOfVariable::bprop()
{ fbprop(); }

void MatrixSumOfVariable::fbprop()
{
  Vec oneInput_value(distr->width());
  f->recomputeParents();
  
  int inputpos=0;
  int targetpos=nsamples*input_size;
  for (int i=0; i<nsamples; i++)
    {
    distr->getRow(curpos, oneInput_value);
    for (int j=0; j<input_size; j++,inputpos++)
      input_value[inputpos]=oneInput_value[j];
    for (int j=input_size; j<distr.width(); j++,targetpos++)
      input_value[targetpos] = oneInput_value[j];
    if(++curpos==distr.length())
      curpos=0;
    }
  f->fbprop(input_value, value, input_gradient, gradient);
}

void MatrixSumOfVariable::symbolicBprop()
{
    PLERROR("MatrixSumOfVariable::symbolicBprop not implemented.");
}

void MatrixSumOfVariable::rfprop()
{
    PLERROR("MatrixSumOfVariable::rfprop not implemented.");
}

void MatrixSumOfVariable::printInfo(bool print_gradient)
{
  PLERROR("MatrixSumOfVariable::printInfo not implemented.");
  Vec input_value(distr->width());
  Vec input_gradient(distr->width());
  Vec output_value(nelems());

  f->recomputeParents();
  value.clear();

  for(int i=0; i<nsamples; i++)
  {
    distr->getRow(curpos++,input_value);
    if (print_gradient)
      f->fbprop(input_value, output_value, input_gradient, gradient);
    else
      f->fprop(input_value, output_value);
    value += output_value;
    if(++curpos>=distr->length())
      curpos = 0;
    f->fproppath.printInfo(print_gradient);
  }
  cout << info() << " : " << getName() << " = " << value;
  if (print_gradient) cout << " gradient=" << gradient;
  cout << endl; 
}


/** ConcatOfVariable **/
/* concatenates the results of each operation in the loop into the resulting variable */

ConcatOfVariable::ConcatOfVariable(VMat the_distr, Func the_f)
  :NaryVariable(nonInputParentsOfPath(the_f->inputs,the_f->outputs), 
                the_f->outputs[0]->length()*the_distr->length(), 
                the_f->outputs[0]->width()),
  distr(the_distr), f(the_f)
{
  if(f->outputs.size()!=1)
    PLERROR("In ConcatOfVariable constructor: function must have a single output variable");

  input_value.resize(distr->width());
  input_gradient.resize(distr->width());
}

/* Old constructor
ConcatOfVariable::ConcatOfVariable(Variable* the_output, const VarArray& the_inputs, VMat the_distr, const VarArray& the_parameters)
  :NaryVariable(nonInputParentsOfPath(the_inputs,the_output), the_output->length()*the_distr->length(), the_output->width()), inputs(the_inputs), distr(the_distr), output(the_output), parameters(the_parameters)
{
  full_fproppath = propagationPath(inputs&parameters, output);
  fproppath = propagationPath(inputs, output);
  bproppath = propagationPath(parameters, output);
}
*/

IMPLEMENT_NAME_AND_DEEPCOPY(ConcatOfVariable);
void ConcatOfVariable::recomputeSize(int& l, int& w) const
{ l=f->outputs[0]->length()*distr->length(); w=f->outputs[0]->width(); }

void ConcatOfVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  NaryVariable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(distr, copies);
  deepCopyField(f, copies);
  deepCopyField(input_value, copies);
  deepCopyField(input_gradient, copies);
}


void ConcatOfVariable::fprop()
{
  f->recomputeParents();

  int pos = 0;
  int singleoutputsize = f->outputs[0]->nelems();
  for(int i=0; i<distr->length(); i++, pos+=singleoutputsize)
    {
      distr->getRow(i,input_value);
      f->fprop(input_value, value.subVec(pos,singleoutputsize));
    }
}

void ConcatOfVariable::bprop()
{
  fbprop();
}

void ConcatOfVariable::fbprop()
{
  f->recomputeParents();

  int pos = 0;
  int singleoutputsize = f->outputs[0]->nelems();
  for(int i=0; i<distr->length(); i++, pos+=singleoutputsize)
    {
      distr->getRow(i, input_value);
      f->fbprop(input_value, value.subVec(pos,singleoutputsize), 
                input_gradient, gradient.subVec(pos,singleoutputsize));
      // We don't use the computed input_gradients, as the input is a dummy variable.
      // The gradients on other (non-input) variables which we are interested in, 
      // have been accumulated by the call as a side effect.
    }
}

/** ArgminOfVariable **/
/* returns the value of v within the_values_of_v that gives the lowest
   value of a scalar expression (which may depend on inputs). */
ArgminOfVariable::ArgminOfVariable(Variable* the_v,
                                   Variable* the_expression,
                                   Variable* the_values_of_v,
                                   const VarArray& the_inputs)
  :NaryVariable(the_inputs,1,1), inputs(the_inputs), expression(the_expression),
   values_of_v(the_values_of_v), v(the_v)
{
  if (!v->isScalar() || !values_of_v->isVec())
    PLERROR("ArgminOfVariable currently implemented only for a scalar v and a vector values_of_v");
  vv_path = propagationPath(inputs,values_of_v);
  e_path = propagationPath(inputs& (VarArray)v, expression);
  v_path = propagationPath(v, expression);
}

IMPLEMENT_NAME_AND_DEEPCOPY(ArgminOfVariable);
void ArgminOfVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }

void ArgminOfVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  NaryVariable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(inputs, copies);
  deepCopyField(expression, copies);
  deepCopyField(values_of_v, copies);
  deepCopyField(v, copies);
  deepCopyField(vv_path, copies);
  deepCopyField(e_path, copies);
  deepCopyField(v_path, copies);
}


void ArgminOfVariable::fprop()
{
  vv_path.fprop(); // compute influence of inputs on values_of_v
  real min_value_of_expression = FLT_MAX;
  real argmin_value_of_v = values_of_v->value[0];
  for (int i=0;i<values_of_v->nelems();i++)
    {
      v->value[0] = values_of_v->value[i];
      if (i==0)
        e_path.fprop(); // compute influence of v and inputs on expression
      else
        v_path.fprop(); // otherwise, keep influence of inputs fixed
      real e = expression->value[0];
      if (e<min_value_of_expression)
        {
          min_value_of_expression = e;
          argmin_value_of_v = v->value[0];
          index_of_argmin = i;
        }
    }
  value[0] = argmin_value_of_v;
}

void ArgminOfVariable::bprop()
{
  vv_path.clearGradient();
  values_of_v->gradientdata[index_of_argmin] = gradientdata[0];
  vv_path.bprop();
}


/** MatrixElementsVariable **/
/* fills the elements of a matrix using the given scalar Variable expression,
   that depends on index variables i and j, that loop from
   0 to ni-1 and 0 to nj-1 respectively. */

MatrixElementsVariable::MatrixElementsVariable
(Variable* the_expression, const Var& i_index, const Var& j_index,
 int number_of_i_values, int number_of_j_values,
 const VarArray& the_parameters)
  :NaryVariable(the_parameters, number_of_i_values, number_of_j_values), 
   i(i_index), j(j_index), ni(number_of_i_values), 
   nj(number_of_j_values), expression(the_expression), 
   parameters(the_parameters)
{
  if (!i->isScalar())
    PLERROR("MatrixElementsVariable expect a scalar index Var i_index");
  if (!j->isScalar())
    PLERROR("MatrixElementsVariable expect a scalar index Var j_index");

  full_fproppath = propagationPath(parameters&(VarArray)i&(VarArray)j, expression);
  fproppath = propagationPath(i&j, expression);
  bproppath = propagationPath(parameters, expression);
}

IMPLEMENT_NAME_AND_DEEPCOPY(MatrixElementsVariable);
void MatrixElementsVariable::recomputeSize(int& l, int& w) const
{ l=ni; w=nj; }

void MatrixElementsVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  NaryVariable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(i, copies);
  deepCopyField(j, copies);
  deepCopyField(expression, copies);
  deepCopyField(parameters, copies);
  deepCopyField(full_fproppath, copies);
  deepCopyField(fproppath, copies);
  deepCopyField(bproppath, copies);
}


void MatrixElementsVariable::fprop()
{
  int ii,jj;

  for (ii=0;ii<ni;ii++)
    {
      i->value[0]=ii;
      for (jj=0;jj<nj;jj++)
        {
          j->value[0]=jj;
          if (ii==0 && jj==0)
            // effect of parameters is computed only once
            full_fproppath.fprop();
          else
            fproppath.fprop();
          matValue(ii,jj) = expression->value[0];
        }
    }
}

void MatrixElementsVariable::bprop()
{
  int ii,jj;

  for (ii=0;ii<ni;ii++)
    {
      i->value[0]=ii;
      for (jj=0;jj<nj;jj++)
        {
          j->value[0]=jj;
          if (ii==0 && jj==0)
            // effect of parameters is computed only once
            full_fproppath.fprop();
          else
            fproppath.fprop();
          bproppath.clearGradient();
          // PASCAL: peut-etre qu'ici on devrait avoir += ?
          // (si l'expression est une Var utilisee ailleurs!)
          // REPONSE: Non! (expression ne devrait etre utilisee 
          // nulle part ailleurs. To do: utilisation de Func 
          // pour le garantir...)
          expression->gradient[0] = matGradient(ii,jj);
          bproppath.bprop();
        }
    }
}

void MatrixElementsVariable::fbprop()
{
  int ii,jj;

  for (ii=0;ii<ni;ii++)
    {
      i->value[0]=ii;
      for (jj=0;jj<nj;jj++)
        {
          j->value[0]=jj;
          if (ii==0 && jj==0)
            // effect of parameters is computed only once
            full_fproppath.fprop();
          else
            fproppath.fprop();
          matValue(ii,jj) = expression->value[0];
          bproppath.clearGradient();
          // PASCAL: peut-etre qu'ici on devrait avoir += ?
          // (si l'expression est une Var utilisee ailleurs!)
          // REPONSE: Non! (expression ne devrait etre utilisee 
          // nulle part ailleurs. To do: utilisation de Func 
          // pour le garantir...)
          expression->gradient[0] = matGradient(ii,jj);
          bproppath.bprop();
        }
    }
}

/** VarArrayElementVariable **/
/* selects one element of a VarArray according to a Var index */

VarArrayElementVariable::
VarArrayElementVariable(VarArray& input1, const Var& input2)
  :NaryVariable(input1 & (VarArray)input2, input1[0]->length(), input1[0]->width())
{
  int l=input1[0]->length();
  int w=input1[0]->width();
  for (int i=1;i<input1.size();i++)
    if (input1[i]->length()!=l || input1[i]->width()!=w)
      PLERROR("VarArrayElementVairables expect all the elements of input1 array to have the same size");
  if (!input2->isScalar())
    PLERROR("VarArrayElementVariable expect an index Var (input2) of length 1");
}

IMPLEMENT_NAME_AND_DEEPCOPY(VarArrayElementVariable);

void VarArrayElementVariable::recomputeSize(int& l, int& w) const
{ l=varray[0]->length(); w=varray[0]->width(); }

void VarArrayElementVariable::fprop()
{
  int index = (int)(varray.last()->valuedata[0]);
#ifdef BOUNDCHECK
  if (index<0 || index>=varray.size()-1)
    PLERROR("VarArrayElementVariable::fprop, out of bound access of array(%d) at %d",
          varray.size()-1,index);
#endif
  for (int i=0;i<nelems();i++)
    valuedata[i] = varray[index]->valuedata[i];
}

void VarArrayElementVariable::bprop()
{
  int index = (int)(varray.last()->valuedata[0]);
  for (int i=0;i<nelems();i++)
    varray[index]->gradientdata[i] += gradientdata[i];
}

void VarArrayElementVariable::symbolicBprop()
{
  int index = (int)(varray.last()->valuedata[0]);
  varray[index]->accg(g);
}

/** IfThenElseVariable **/

IfThenElseVariable::IfThenElseVariable(Var IfVar, Var ThenVar, Var ElseVar)
    :NaryVariable(IfVar & ThenVar & (VarArray)ElseVar,ThenVar->length(), ThenVar->width())
{
  if (ThenVar->length() != ElseVar->length() || ThenVar->width() != ElseVar->width())
    PLERROR("In IfThenElseVariable: ElseVar and ThenVar must have the same size");
  if (!IfVar->isScalar() && (IfVar->length()!=ThenVar->length() || IfVar->width()!=ThenVar->width()))
    PLERROR("In IfThenElseVariable: IfVar must either be a scalar or have the same size as ThenVar and ElseVar");
}

IMPLEMENT_NAME_AND_DEEPCOPY(IfThenElseVariable);
void IfThenElseVariable::recomputeSize(int& l, int& w) const
{ l=varray[1]->length(); w=varray[1]->width(); }

void IfThenElseVariable::fprop()
{
  if(If()->isScalar())
    {
      bool test = (bool)If()->valuedata[0];
      if (test)
        value << Then()->value;
      else
        value << Else()->value;
    }
  else
    {
      real* ifv = If()->valuedata;
      real* thenv = Then()->valuedata;
      real* elsev = Else()->valuedata;
      for (int k=0;k<nelems();k++)
        {
          if ((bool)ifv[k])
            valuedata[k]=thenv[k];
          else
            valuedata[k]=elsev[k];
        }
    }
}

void IfThenElseVariable::bprop()
{
  if(If()->isScalar())
    {
      bool test = (bool)If()->valuedata[0];
      if (test)
        Then()->gradient += gradient;
      else
        Else()->gradient += gradient;
    }
  else
    {
      real* ifv = If()->valuedata;
      real* theng = Then()->gradientdata;
      real* elseg = Else()->gradientdata;
      for (int k=0;k<nelems();k++)
        {
          if ((bool)ifv[k])
            theng[k] += gradientdata[k];
          else
            elseg[k] += gradientdata[k];
        }
    }
}

void IfThenElseVariable::symbolicBprop()
{
  Var zero(length(), width());
  Then()->accg(ifThenElse(If(), g, zero));
  Else()->accg(ifThenElse(If(), zero, g));
}

void IfThenElseVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  if(If()->isScalar())
    {
      bool test = (bool)If()->valuedata[0];
      if (test)
        rValue << Then()->rValue;
      else
        rValue << Else()->rValue;
    }
  else
    {
      real* ifv = If()->valuedata;
      real* rthenv = Then()->rvaluedata;
      real* relsev = Else()->rvaluedata;
      for (int k=0;k<nelems();k++)
        {
          if ((bool)ifv[k])
            rvaluedata[k]=rthenv[k];
          else
            rvaluedata[k]=relsev[k];
        }
    }
}






%> // end of namespace PLearn




