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
   * $Id: MiniBatchClassificationLossVariable.cc,v 1.2 2003/01/08 21:32:47 ducharme Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "MiniBatchClassificationLossVariable.h"

namespace PLearn <%
using namespace std;


/** MiniBatchClassificationLossVariable **/
IMPLEMENT_NAME_AND_DEEPCOPY(MiniBatchClassificationLossVariable);

MiniBatchClassificationLossVariable::MiniBatchClassificationLossVariable(Variable* netout, Variable* classnum)
  :BinaryVariable(netout,classnum,classnum->length(),classnum->width())
{
  if(!classnum->isVec())
    PLERROR("In MiniBatchClassificationLossVariable: classnum must be a vector variable representing the indexs of netout (typically class numbers)");
}


void MiniBatchClassificationLossVariable::recomputeSize(int& l, int& w) const
{ l=input2->length(), w=input2->width(); }


void MiniBatchClassificationLossVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MiniBatchClassificationLossVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "MiniBatchClassificationLossVariable");
}


void MiniBatchClassificationLossVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MiniBatchClassificationLossVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MiniBatchClassificationLossVariable");
}


void MiniBatchClassificationLossVariable::fprop()
{
  int n = input2->size();
  if(input1->length()==n)
    for (int i=0; i<n; i++)
      {
      int topscorepos = argmax(input1->matValue.row(i));
      int num = int(input2->valuedata[i]);
      valuedata[i] = (topscorepos==num ?0 :1);
      }
  else if(input1->width()==n)
    for (int i=0; i<n; i++)
      {
      int topscorepos = argmax(input1->matValue.column(i));
      int num = int(input2->valuedata[i]);
      valuedata[i] = (topscorepos==num ?0 :1);
      }
  else PLERROR("In MiniBatchClassificationLossVariable: The length or width of netout doesn't equal to the size of classnum");
}


void MiniBatchClassificationLossVariable::symbolicBprop()
{
  PLERROR("MiniBatchClassificationLossVariable::symbolicBprop not implemented.");
}



%> // end of namespace PLearn


