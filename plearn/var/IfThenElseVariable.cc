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
   * $Id: IfThenElseVariable.cc,v 1.1 2002/10/23 23:32:34 dorionc Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "IfThenElseVariable.h"

// From Old NaryVariable.cc: all includes are putted in every file.
// To be revised manually 
#include "NaryVariable.h"
#include "Var.h"
#include "TMat_maths.h"
#include "PLMPI.h"
#include "DisplayUtils.h"
#include "pl_erf.h"
#include "Var_utils.h"
namespace PLearn <%
using namespace std;


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


