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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "OneHotSquaredLoss.h"
#include "RowAtPositionVariable.h"
#include "Var_operators.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** OneHotSquaredLoss **/

PLEARN_IMPLEMENT_OBJECT(OneHotSquaredLoss,
                        "Computes sum(square_i(netout[i]-(i==classnum ?hotval :coldval))",
                        "NO HELP");

OneHotSquaredLoss::OneHotSquaredLoss()
    : coldval_(0.), hotval_(0.)
{ }
  
OneHotSquaredLoss::OneHotSquaredLoss(Variable* netout, Variable* classnum, real coldval, real hotval)
    : inherited(netout,classnum,1,1), coldval_(coldval), hotval_(hotval)
{
    build_();
}

void
OneHotSquaredLoss::build()
{
    inherited::build();
    build_();
}

void
OneHotSquaredLoss::build_()
{
    // input2 is classnum from constructor
    if(input2 && !input2->isScalar())
        PLERROR("In OneHotSquaredLoss: classnum must be a scalar variable representing an index of netout (typically a classnum)");
}

void
OneHotSquaredLoss::declareOptions(OptionList &ol)
{
    declareOption(ol, "coldval_", &OneHotSquaredLoss::coldval_, OptionBase::buildoption, "");
    declareOption(ol, "hotval_", &OneHotSquaredLoss::hotval_, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void OneHotSquaredLoss::recomputeSize(int& l, int& w) const
{ l=1, w=1; }
  
void OneHotSquaredLoss::fprop()
{
    real* netout = input1->valuedata;
    int n = input1->value.size();
    int classnum = (int) input2->valuedata[0];
    real res = 0.;
    for(int i=0; i<n; i++)
        res += square(*netout++ - (i==classnum ? hotval_ : coldval_));
    *valuedata = res;
}


void OneHotSquaredLoss::bprop()
{
    real gr = *gradientdata;
    real* netout = input1->valuedata;
    int n = input1->value.size();
    int classnum = (int) input2->valuedata[0];
    real* input1grptr = input1->gradientdata;
    if(gr!=1.)
    {
        gr = gr+gr;
        for(int i=0; i<n; i++)
            *input1grptr++ += gr*(*netout++ - (i==classnum ? hotval_ : coldval_));
    }
    else // specialised version for gr==1
    {
        for(int i=0; i<n; i++)
            input1->gradientdata[i] += two(netout[i] - (i==classnum ? hotval_ : coldval_));        
    }
}


void OneHotSquaredLoss::symbolicBprop()
{
    Var gi =  g*(input1 - coldval_);
    Var gindex = new RowAtPositionVariable(g*(coldval_-hotval_), input2, input1->length());
    Var ginput = gi + gindex;
    input1->accg(ginput+ginput); //2*gi
}


void OneHotSquaredLoss::rfprop()
{
    int n = input1->value.size();
    int classnum = (int) input2->valuedata[0];
    real sum = 0;
    for (int i=0; i<n; i++)
        sum += 2 * input1->rvaluedata[i] * (input1->valuedata[i] - (i==classnum ? hotval_ : coldval_));
    rvaluedata[0] = sum;
}



} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
