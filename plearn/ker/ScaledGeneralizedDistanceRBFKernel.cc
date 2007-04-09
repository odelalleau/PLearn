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

#include "ScaledGeneralizedDistanceRBFKernel.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(ScaledGeneralizedDistanceRBFKernel, "ONE LINE DESCR", "NO HELP");

void ScaledGeneralizedDistanceRBFKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(phi, copies);
    deepCopyField(a, copies);
}

real ScaledGeneralizedDistanceRBFKernel::evaluate(const Vec& x1, const Vec& x2) const
{ 
#ifdef BOUNDCHECK
    if(x1.length()!=x2.length())
        PLERROR("IN ScaledGeneralizedDistanceRBFKernel::evaluate x1 and x2 must have the same length");
#endif

    real summ = 0.0;
    real* ph=phi.data();
    real* aa=a.data();
    for(int i=0; i<x1.length(); i++)
        summ += ph[i]*pow(fabs(pow(x1[i],aa[i])-pow(x2[i],aa[i])), (real)b);
    return exp(-pow(summ,c));
}

void ScaledGeneralizedDistanceRBFKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "phi", &ScaledGeneralizedDistanceRBFKernel::phi, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "a", &ScaledGeneralizedDistanceRBFKernel::phi, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "b", &ScaledGeneralizedDistanceRBFKernel::phi, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "c", &ScaledGeneralizedDistanceRBFKernel::phi, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
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
