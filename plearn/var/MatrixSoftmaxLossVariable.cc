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
   * $Id: MatrixSoftmaxLossVariable.cc,v 1.1 2002/10/23 23:32:34 dorionc Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "MatrixSoftmaxLossVariable.h"

// From Old BinaryVariable.cc: all includes are putted in every file.
// To be revised manually 
#include "BinaryVariable.h"
#include "Var.h"
#include "pl_erf.h"
#include "Var_utils.h"
namespace PLearn <%
using namespace std;


/** MatrixSoftmaxLossVariable **/
IMPLEMENT_NAME_AND_DEEPCOPY(MatrixSoftmaxLossVariable);

MatrixSoftmaxLossVariable::MatrixSoftmaxLossVariable(Variable* input1, Variable* input2) 
:BinaryVariable(input1, input2, input2->length(), input2->width())
{
  if(!input2->isVec())
    PLERROR("In MatrixSoftmaxLossVariable: position must be a vector");
}


void MatrixSoftmaxLossVariable::recomputeSize(int& l, int& w) const
{ l=input2->length(); w=input2->width(); }


void MatrixSoftmaxLossVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MatrixSoftmaxLossVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "MatrixSoftmaxLossVariable");
}


void MatrixSoftmaxLossVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MatrixSoftmaxLossVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MatrixSoftmaxLossVariable");
}


void MatrixSoftmaxLossVariable::fprop()
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


void MatrixSoftmaxLossVariable::bprop()
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


void MatrixSoftmaxLossVariable::bbprop()
{
  PLERROR("MatrixSoftmaxLossVariable::bbprop() not implemented");
}


void MatrixSoftmaxLossVariable::symbolicBprop()
{
  PLERROR("MatrixSoftmaxLossVariable::symbolicBprop() not implemented");
}


void MatrixSoftmaxLossVariable::rfprop()
{
  PLERROR("MatrixSoftmaxLossVariable::rfprop() not implemented");
}



%> // end of namespace PLearn


