// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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
   * $Id: ShiftAndRescaleVMatrix.cc,v 1.7 2004/02/20 21:14:44 chrish42 Exp $
   ******************************************************* */

#include "ShiftAndRescaleVMatrix.h"
#include "VMat_maths.h"

namespace PLearn {
using namespace std;

/** ShiftAndRescaleVMatrix **/

PLEARN_IMPLEMENT_OBJECT(ShiftAndRescaleVMatrix, "ONE LINE DESCR", "NO HELP");

ShiftAndRescaleVMatrix::
ShiftAndRescaleVMatrix(VMat underlying_vm, Vec the_shift, Vec the_scale)
  : shift(the_shift), scale(the_scale), automatic(0), n_train(0), n_inputs(-1)
{
  vm = underlying_vm;
  build_();
}


ShiftAndRescaleVMatrix::ShiftAndRescaleVMatrix() :automatic(1),n_train(0), n_inputs(-1) {}

ShiftAndRescaleVMatrix::
ShiftAndRescaleVMatrix(VMat underlying_vm, int n_inputs_)
  :shift(underlying_vm->width()),
   scale(underlying_vm->width()), automatic(1),n_train(0), n_inputs(n_inputs_)
{
  vm = underlying_vm;
  build_();
}

ShiftAndRescaleVMatrix::
ShiftAndRescaleVMatrix(VMat underlying_vm, int n_inputs_, int n_train_)
  : shift(underlying_vm->width()), scale(underlying_vm->width()), automatic(1), n_train(n_train_), n_inputs(n_inputs_)
{
  vm = underlying_vm;
  build_();  
}

void ShiftAndRescaleVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "underlying_vmat", &ShiftAndRescaleVMatrix::vm, OptionBase::buildoption,
                "This is the source vmatrix that will be shifted and rescaled");
  declareOption(ol, "shift", &ShiftAndRescaleVMatrix::shift, OptionBase::buildoption,
                "This is the quantity added to each INPUT element of a row of the source vmatrix.");
  declareOption(ol, "scale", &ShiftAndRescaleVMatrix::scale, OptionBase::buildoption,
                "This is the quantity multiplied to each shifted element of a row of the source vmatrix.");
  declareOption(ol, "automatic", &ShiftAndRescaleVMatrix::automatic, OptionBase::buildoption,
                "Whether shift and scale are determined from the mean and stdev of the source vmatrix, or user-provided.");
  declareOption(ol, "n_train", &ShiftAndRescaleVMatrix::n_train, OptionBase::buildoption,
                "when automatic, use only the n_train first examples to estimate shift and scale, if n_train>0.");
  declareOption(ol, "n_inputs", &ShiftAndRescaleVMatrix::n_inputs, OptionBase::buildoption,
                "when automatic, shift and scale only the first n_inputs columns (If n_inputs<0, set n_inputs from underlying_vmat->inputsize()).");
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void ShiftAndRescaleVMatrix::build_()
{
  length_ = vm->length();
  width_ = vm->width();
  writable = vm->isWritable();
  if(inputsize_<0)
    inputsize_ = vm->inputsize();
  if(targetsize_<0)
    targetsize_ = vm->targetsize();
  if(weightsize_<0)
    weightsize_ = vm->weightsize();
  setMtime(vm->getMtime());
  if (vm->getMetaDataDir() != "") {
    setMetaDataDir(vm->getMetaDataDir());
  }
  setAlias(vm->getAlias());
  fieldinfos = vm->getFieldInfos();
  if (automatic)
    {
      if (n_inputs<0)
        {
          n_inputs = vm->inputsize();
          if (n_inputs<0)
            PLERROR("ShiftAndRescaleVMatrix: either n_inputs should be provided explicitly or the underlying VMatrix should have a set value of inputsize");
        }
      if (n_train>0)
        computeMeanAndStddev(vm.subMatRows(0,n_train), shift, scale);
      else
        computeMeanAndStddev(vm, shift, scale);
      negateElements(shift);
      for (int i=0;i<scale.length();i++) 
        if (scale[i]==0)
          {
            PLWARNING("ShiftAndRescale: data column number %d is constant",i);
            scale[i]=1;
          }
      invertElements(scale);
      shift.subVec(n_inputs,shift.length()-n_inputs).fill(0);
      scale.subVec(n_inputs,shift.length()-n_inputs).fill(1);
    }
  reset_dimensions();
}

                                         
real ShiftAndRescaleVMatrix::get(int i, int j) const
{
  return (vm->get(i,j) + shift[j]) * scale[j];
}

void ShiftAndRescaleVMatrix::getSubRow(int i, int j, Vec v) const
{
  vm->getSubRow(i,j,v);
  for(int jj=0; jj<v.length(); jj++)
    v[jj] = (v[jj] + shift[j+jj]) * scale[j+jj];
}

string ShiftAndRescaleVMatrix::help()
{
  return 
    "ShiftAndRescaleVMatrix allows to shift and scale the first n_inputs columns of an underlying_vm.\n"
    + optionHelp();
}

void ShiftAndRescaleVMatrix::build()
{
  inherited::build();
  build_();
}

} // end of namespcae PLearn
