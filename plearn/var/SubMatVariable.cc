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
   * $Id: SubMatVariable.cc,v 1.2 2003/01/08 21:33:02 ducharme Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SubMatVariable.h"
#include "Var_utils.h"

namespace PLearn <%
using namespace std;


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



%> // end of namespace PLearn


