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
   * $Id: SubMatTransposeVariable.cc,v 1.5 2004/02/20 21:11:53 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ExtendedVariable.h"
#include "SubMatTransposeVariable.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** SubMatTransposeVariable **/

SubMatTransposeVariable::SubMatTransposeVariable(Variable* v, int i, int j, int the_length, int the_width)
  :UnaryVariable(v, the_width, the_length), startk(i*v->length()+j), length_(the_length), width_(the_width)
{
  if(i<0 || i+the_length>v->length() || j<0 || j+the_width>v->width())
    PLERROR("In SubMatTransposeVariable: requested sub-matrix is out of matrix bounds");
}


PLEARN_IMPLEMENT_OBJECT(SubMatTransposeVariable, "ONE LINE DESCR", "NO HELP");


void SubMatTransposeVariable::recomputeSize(int& l, int& w) const
{ l=width_; w=length_; }








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



} // end of namespace PLearn


