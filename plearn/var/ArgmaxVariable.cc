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
   * $Id: ArgmaxVariable.cc,v 1.1 2002/10/23 23:32:34 dorionc Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ArgmaxVariable.h"

// From Old UnaryVariable.cc: all includes are putted in every file.
// To be revised manually 
#include "DisplayUtils.h"
#include "UnaryVariable.h"
#include "Var.h"
#include "TMat_maths.h"
#include "pl_erf.h"
#include "Var_utils.h"
namespace PLearn <%
using namespace std;


/** ArgmaxVariable **/

ArgmaxVariable::ArgmaxVariable(Variable* input)
  :UnaryVariable(input, input->isVec()?1:2, 1) {}


IMPLEMENT_NAME_AND_DEEPCOPY(ArgmaxVariable);

void ArgmaxVariable::recomputeSize(int& l, int& w) const
{ l=input->isVec()?1:2; w=1; }


void ArgmaxVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ArgmaxVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ArgmaxVariable");
}


void ArgmaxVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ArgmaxVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ArgmaxVariable");
}


void ArgmaxVariable::fprop()
{
  real maxval = input->valuedata[0];
  if (input->isVec())
    {
      int argmax = 0;
      for(int i=1; i<input->nelems(); i++)
        {
          real val = input->valuedata[i];
          if(val>maxval)
            {
              maxval = val;
              argmax = i;
            }
        }
      valuedata[0] = argmax;
    }
  else
    {
      int k = 0;
      int argmax_i = 0;
      int argmax_j = 0;
      for(int i=0; i<input->length(); i++)
        for(int j=0; j<input->width(); j++, k++)
          {
            real val = input->valuedata[k];
            if(val>maxval)
              {
                maxval = val;
                argmax_i = i;
                argmax_j = j;
              }
          }
      valuedata[0] = argmax_i;
      valuedata[1] = argmax_j;
    }
}


void ArgmaxVariable::bprop() {}

void ArgmaxVariable::symbolicBprop() {}



%> // end of namespace PLearn


