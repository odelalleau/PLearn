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
   * $Id: SumOverBagsVariable.cc,v 1.2 2004/02/19 15:25:31 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SumOverBagsVariable.h"
#include "PLMPI.h"
#include "DisplayUtils.h"

namespace PLearn <%
using namespace std;



/** SumOverBagsVariable **/

PLEARN_IMPLEMENT_OBJECT(SumOverBagsVariable, "Variable that sums the value of a Func evaluated on each row of a VMat\n", 
                        "However, unlike the SumOfVariable, it does so by unfolding the Func (up to given maximum number\n"
                        "of times 'max_bag_size'), and it allows that number to be variable. Each of the unfolded Func\n"
                        "is applied on a different row of the input VMat. The number of rows to sum is specified on the\n"
                        "fly by the target: all the rows in a bag except the last one have missing values in their\n"
                        "target sub-field.");

SumOverBagsVariable::SumOverBagsVariable(VMat the_vmat, Func the_f, int max_bagsize, int nsamples)
  :NaryVariable(nonInputParentsOfPath(the_f->inputs,the_f->outputs), 
                the_f->outputs[0]->length(), 
                the_f->outputs[0]->width()),
   vmat(the_vmat), f(the_f), max_bag_size(max_bagsize), n_samples(nsamples),
   curpos(0), bag_size(0)
{
  build();
}

void SumOverBagsVariable::build()
{
  inherited::build();
  build_();
}

void SumOverBagsVariable::build_()
{
  if (vmat)
  {
    if (vmat->weightsize()!=0 && vmat->weightsize()!=1)
      PLERROR("SumOverBagsVariable expected vmat->weightsize to be 0 or 1");
    
    input_values.resize(max_bag_size,vmat->inputsize());
    output_value.resize(f->outputs[0]->nelems());
    output_av = Array<Vec>(output_av);
    gradient_av = Array<Vec>(gradient);
    f->inputs.setDontBpropHere(true);

    bag_size_vec.resize(1);
    bag_target.resize(vmat->targetsize());
    bag_weight.resize(vmat->weightsize());
    f_inputs.resize(4);
    f_inputs[0] = input_values.toVec();
    f_inputs[1] = bag_size_vec;
    f_inputs[2] = bag_target;
    f_inputs[3] = bag_weight;
    unused_gradients.resize(4);
    for (int i=0;i<4;i++) unused_gradients[i] = f_inputs[i].copy();
  }
}

void SumOverBagsVariable::declareOptions(OptionList& ol)
{
  declareOption(ol, "f", &SumOverBagsVariable::f, OptionBase::buildoption, 
                "    Func that is applied on each bag, whose input is a matrix that is the\n"
                "    concatenation of the vmat VMatrix rows of the bag. Its output is a vector\n"
                "    that is sum over 1 or more (n_samples) bags in the vmat.");

  declareOption(ol, "vmat", &SumOverBagsVariable::vmat, OptionBase::buildoption, 
                "    VMatrix that contains the data, with multiple consecutive rows forming one bag.\n"
                "    The last row of a bag has a non-missing target value.\n");

  declareOption(ol, "max_bag_size", &SumOverBagsVariable::max_bag_size, OptionBase::buildoption, 
                "    maximum number of examples in a bag (more than that in vmat will trigger a run-time error).\n");

  declareOption(ol, "n_samples", &SumOverBagsVariable::n_samples, OptionBase::buildoption, 
                "    number of samples to iterate over (1 for online gradient, <=0 for batch).");

  inherited::declareOptions(ol);
}

void SumOverBagsVariable::recomputeSize(int& l, int& w) const
{ l=f->outputs[0]->length(); w=f->outputs[0]->width(); }


void SumOverBagsVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  NaryVariable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(vmat, copies);
  deepCopyField(f, copies);
  deepCopyField(output_value, copies);
  deepCopyField(input_values, copies);
  deepCopyField(bag_size_vec, copies);
  deepCopyField(bag_target, copies);
  deepCopyField(bag_weight, copies);
  deepCopyField(f_inputs, copies);
  deepCopyField(unused_gradients, copies);
  deepCopyField(output_av, copies);
  deepCopyField(gradient_av, copies);
}


void SumOverBagsVariable::fpropOneBag(bool do_bprop)
{
  static real dummy_weight=0;
  bool not_reached_end_of_bag=true;
  input_values.resize(max_bag_size,input_values.width());
  for (bag_size=0;not_reached_end_of_bag;bag_size++)
  {
    if (bag_size>=max_bag_size)
      PLERROR("SumOverBagsVariable: bag size=%d > expected max. bag size(%d)",
              bag_size,max_bag_size);
    Vec input_value = input_values(bag_size);
    if (vmat->weightsize()>0)
      {
        real& weight = bag_weight[0];
        vmat->getExample(curpos,input_value,bag_target,weight);
      }
    else
      vmat->getExample(curpos,input_value,bag_target,dummy_weight);
    not_reached_end_of_bag = bag_target.hasMissing();
    if(++curpos == vmat->length())
    {
      curpos = 0;
      if (not_reached_end_of_bag)
        {
          PLWARNING("SumOverBagsVariable: last bag of VMatrix is not complete");
          return;
        }
        break;
    }
  }
  bag_size_vec[0]=bag_size;
  if (do_bprop)
    f->fbprop(f_inputs,output_av,unused_gradients,gradient_av);
  else
    f->fprop(f_inputs,output_av);
  value += output_value;
}

void SumOverBagsVariable::fprop()
{
  value.clear();
  f->recomputeParents();
  if (n_samples==1)
    fpropOneBag();
  else if (n_samples<=0) // one pass through the whole data set
    {
      curpos=0;
      do 
        fpropOneBag();
      while (curpos>0);
    }
  else 
    for (int i=0;i<n_samples;i++)
      fpropOneBag();
}


void SumOverBagsVariable::fbprop()
{
  value.clear();
  f->recomputeParents();
  if (n_samples==1)
    fpropOneBag(true);
  else if (n_samples<=0) // one pass through the whole data set
    {
      curpos=0;
      do 
        fpropOneBag();
      while (curpos>0);
    }
  else 
    for (int i=0;i<n_samples;i++)
      fpropOneBag(true);
}

void SumOverBagsVariable::bprop()
{ 
  fbprop();
}

void SumOverBagsVariable::printInfo(bool print_gradient)
{
  f->fproppath.printInfo(print_gradient);
  cout << info() << " : " << getName() << "(max_bag_size=" << max_bag_size << ", ";
  cout << ", n_samples=" << n_samples << ") = " << value;
  if (print_gradient) cout << " gradient=" << gradient;
  cout << endl; 
}



%> // end of namespace PLearn


