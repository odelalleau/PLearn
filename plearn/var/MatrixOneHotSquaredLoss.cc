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
   * $Id: MatrixOneHotSquaredLoss.cc,v 1.2 2003/01/08 21:32:42 ducharme Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "MatrixOneHotSquaredLoss.h"

namespace PLearn <%
using namespace std;


/** MatrixOneHotSquaredLoss **/

IMPLEMENT_NAME_AND_DEEPCOPY(MatrixOneHotSquaredLoss);

MatrixOneHotSquaredLoss::MatrixOneHotSquaredLoss(Variable* input1, Variable* input2, real coldval, real hotval)
    :BinaryVariable(input1,input2,input2->length(),input2->width()), coldval_(coldval), hotval_(hotval)
{
  if(!input2->isVec())
    PLERROR("In MatrixOneHotSquaredLoss: classnum must be a vector variable representing the indexs of netouts (typically some classnums)");
}


void MatrixOneHotSquaredLoss::recomputeSize(int& l, int& w) const
{ l=input2->length(), w=input2->width(); }


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


void MatrixOneHotSquaredLoss::symbolicBprop()
{
  PLERROR("MatrixOneHotSquaredLoss::symbolicBprop not implemented.");
}



%> // end of namespace PLearn


