// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
   * $Id: UnfoldedSumOfVariable.cc,v 1.1 2004/02/16 14:38:11 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "UnfoldedSumOfVariable.h"
#include "PLMPI.h"
#include "DisplayUtils.h"

namespace PLearn <%
using namespace std;



/** UnfoldedSumOfVariable **/

PLEARN_IMPLEMENT_OBJECT(UnfoldedSumOfVariable, "Variable that sums the value of a Func evaluated on each row of a VMat\n", 
                        "However, unlike the SumOfVariable, it does so by unfolding the Func (up to given maximum number\n"
                        "of times 'max_bag_size'), and it allows that number to be variable. Each of the unfolded Func\n"
                        "is applied on a different row of the input VMat. The number of rows to sum is specified on the\n"
                        "fly by the target: all the rows in a bag except the last one have missing values in their\n"
                        "target sub-field.");

UnfoldedSumOfVariable::UnfoldedSumOfVariable(VMat the_distr, Func the_f, int max_bagsize)
  :NaryVariable(nonInputParentsOfPath(the_f->inputs,the_f->outputs), 
                the_f->outputs[0]->length(), 
                the_f->outputs[0]->width()),
   distr(the_distr), f(the_f), max_bag_size(max_bagsize), curpos(0)
{
  build();
}

void UnfoldedSumOfVariable::build()
{
  inherited::build();
  build_();
}

void UnfoldedSumOfVariable::build_()
{
  input_value.resize(distr->width());
  input_value.resize(distr->inputsize()+distr->targetsize()+distr->weightsize());
  target = input_value.subVec(distr->inputsize(),distr->targetsize());
  input_values.resize(max_n_samples,distr->inputsize()+distr->targetsize()+distr->weightsize());
  input_gradients.resize(max_n_samples,distr->inputsize()+distr->targetsize()+distr->weightsize());
  output_values.resize(max_n_samples,f->outputs[0]->size());
  if(f->outputs.size()!=1)
    PLERROR("In UnfoldedSumOfVariable: function must have a single variable output (maybe you can vconcat the vars into a single one prior to calling sumOf, if this is really what you want)");
  f->inputs.setDontBpropHere(true);
  inputs.resize(max_n_samples);
  outputs.resize(max_n_samples);
  unfolded_f.resize(max_n_samples);
  for (int i=0;i<max_n_samples;i++)
  {
    inputs[i].resize(f->inputs.size());
    outputs[i] = f(inputs[i]);
    unfolded_f[i] = Func(inputs[i],outputs[i]);
  }
}

void UnfoldedSumOfVariable::declareOptions(OptionList& ol)
{
  declareOption(ol, "f", &UnfoldedSumOfVariable::f, OptionBase::buildoption, 
                "    Func that is replicated for each element of the 'bag' taken from the VMat.");

  declareOption(ol, "distr", &UnfoldedSumOfVariable::distr, OptionBase::buildoption, 
                "    VMatrix that contains the data, with multiple consecutive rows forming one bag.\n"
                "    The last row of a bag has a non-missing target value.\n");

  declareOption(ol, "max_bag_size", &UnfoldedSumOfVariable::max_bag_size, OptionBase::buildoption, 
                "    maximum number of examples in a bag (more than that in distr will trigger a run-time error).\n");

  inherited::declareOptions(ol);
}

void UnfoldedSumOfVariable::recomputeSize(int& l, int& w) const
{ l=f->outputs[0]->length(); w=f->outputs[0]->width(); }


void UnfoldedSumOfVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  NaryVariable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(distr, copies);
  deepCopyField(f, copies);
  deepCopyField(inputs, copies);
  deepCopyField(outputs, copies);
  deepCopyField(unfolded_f, copies);
}


void UnfoldedSumOfVariable::fprop()
{
  value.clear();
  bool not_reached_end_of_bag=true;
  
  for (int i=0;not_reached_end_of_bag;i++)
    {
      input_value.resize(distr->width());
      distr->getRow(curpos, input_value);
      input_value.resize(distr->inputsize()+distr->targetsize()+distr->weightsize());
      not_reached_end_of_bag = target.hasMissing();
      unfolded_f[i]->fprop(input_value, output_value);
      value += output_value;
      if(++curpos == distr->length())
        curpos = 0;
    }
  }
}


void UnfoldedSumOfVariable::bprop()
{ 
}


void UnfoldedSumOfVariable::fbprop()
{
  f->recomputeParents();
  
  if(nsamples==1)
  {
    input_value.resize(distr->width());
    distr->getRow(curpos, input_value);
    input_value.resize(distr->inputsize()+distr->targetsize()+distr->weightsize());
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
      PLERROR("In UnfoldedSumOfVariable::fbprop, the case where nsamples is greater than distr->length is not supported in parallel computation");
    int nb_sample = nsamples/PLMPI::size;
    int start_pos = PLMPI::rank * nb_sample;
    int end_pos = (PLMPI::rank==PLMPI::size-1) ? nsamples : start_pos + nb_sample;
    Vec dummy_value(value.length());
    for(int i=start_pos; i<end_pos; i++)
    {
      input_value.resize(distr->width());
      distr->getRow(i, input_value);
      input_value.resize(distr->inputsize()+distr->targetsize()+distr->weightsize());
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
      input_value.resize(distr->width());
      distr->getRow(curpos, input_value);
      input_value.resize(distr->inputsize()+distr->targetsize()+distr->weightsize());
      static bool display_fn=false;
      if (display_fn)
        displayFunction(f, true, false, 250);
      f->fbprop(input_value, output_value, input_gradient, gradient);
      value += output_value;
      if(++curpos == distr->length()) 
        curpos = 0;
    }
#endif
  }
}


void UnfoldedSumOfVariable::symbolicBprop()
{
  /*
  // f is a function of its inputs, what we want is a function of the parameters of f (which are in the inputs field of this UnfoldedSumOfVariable)
  VarArray& params = varray; 
  int nparams = params.size();
  f->bproppath.symbolicBprop();

  VarArray dparams(nparams);    
  for(int i=0; i<nparams; i++)
    dparams[i] = params[i]->g;

  Var dparams_concat = new ConcatElementsVariable(dparams);
  Var dparams_sum = new UnfoldedSumOfVariable(distr, Func(params,dparams_concat), nsamples);

  for(int i=0; i<nparams; i++)
    params[i]->g += dparams_sum.sub(...)
  */
}


void UnfoldedSumOfVariable::rfprop()
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
//        PLERROR("In UnfoldedSumOfVariable::fprop, the case where nsamples is greater than distr->length is not supported in parallel computation");
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


void UnfoldedSumOfVariable::printInfo(bool print_gradient)
{
  Vec input_value(distr->width());
  Vec input_gradient(distr->width());
  Vec output_value(nelems());

  f->recomputeParents();
  value.clear();

  for(int i=0; i<nsamples; i++)
  {
    input_value.resize(distr->width());
    distr->getRow(curpos++,input_value);
    input_value.resize(distr->inputsize()+distr->targetsize()+distr->weightsize());
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



%> // end of namespace PLearn


