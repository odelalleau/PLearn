// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion
// Copyright (C) 2004 Olivier Delalleau

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

#include "BinaryClassificationLossVariable.h"
namespace PLearn {
using namespace std;

/** BinaryClassificationLossVariable **/

PLEARN_IMPLEMENT_OBJECT(
    BinaryClassificationLossVariable,
    "Variable outputting the class for thresholded binary classification",
    "For one-dimensional output, \n"
    "Class is 'class_1' if output < 'threshold', and 'class_2' if >= 'threshold'. \n"
    );


BinaryClassificationLossVariable::BinaryClassificationLossVariable()
    : threshold(0.0)
{ }
  
////////////////////
// declareOptions //
////////////////////
void BinaryClassificationLossVariable::declareOptions(OptionList& ol) {
    declareOption(ol, "threshold", &BinaryClassificationLossVariable::threshold, OptionBase::buildoption,
                  "The threshold under which the class is 'class_1', and above which the class is 'class_2'.");
    inherited::declareOptions(ol);
}

BinaryClassificationLossVariable::BinaryClassificationLossVariable(Variable* netout, Variable* classnum)
    : inherited(netout,classnum,1,1),
      class_1(0),
      class_2(1),
      threshold(0.5)
{
    build_();
}

void
BinaryClassificationLossVariable::build()
{
    inherited::build();
    build_();
}

void
BinaryClassificationLossVariable::build_()
{
    // input2 == classnum from constructor
    if (input2 && !input2->isScalar())
        PLERROR("In BinaryClassificationLossVariable: classnum must be a scalar variable representing an index of netout (typically a class number)");
}

void BinaryClassificationLossVariable::recomputeSize(int& l, int& w) const
{ l=1, w=1; }


void BinaryClassificationLossVariable::fprop()
{
    int classnum = int(input2->valuedata[0]);
    int outputclass;
    if (input1->valuedata[0] < threshold) {
        outputclass = class_1;
    } else {
        outputclass = class_2;
    }
  
    valuedata[0] = (outputclass == classnum ?0 :1);
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
