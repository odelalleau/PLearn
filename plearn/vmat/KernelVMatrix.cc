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

#include "KernelVMatrix.h"

// From Old Kernel.cc: all includes are putted in every file.
// To be revised manually
/*#include <cmath>
  #include <plearn/base/stringutils.h>
  #include <plearn/ker/Kernel.h>
  #include <plearn/math/TMat_maths.h>
  #include <plearn/sys/PLMPI.h>*/
//////////////////////////
namespace PLearn {
using namespace std;


// *****************
// * KernelVMatrix *
// *****************

PLEARN_IMPLEMENT_OBJECT(KernelVMatrix, "ONE LINE DESC", "NO HELP");

KernelVMatrix::KernelVMatrix()
{
}

KernelVMatrix::KernelVMatrix(VMat the_source1, VMat the_source2, Ker the_ker)
    : VMatrix(the_source1->length(), the_source2->length()),
      source1(the_source1), source2(the_source2), ker(the_ker),
      input1(the_source1->width()), input2(the_source2->width())
{}

void
KernelVMatrix::build()
{
    inherited::build();
    build_();
}

void
KernelVMatrix::build_()
{
    if (source1)
        input1.resize(source1->width());
    if (source2)
        input2.resize(source2->width());
    updateMtime(source1);
    updateMtime(source2);
    updateMtime(ker.getData());
}

void
KernelVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "source1", &KernelVMatrix::source1,
                  OptionBase::buildoption, "");

    declareOption(ol, "source2", &KernelVMatrix::source2,
                  OptionBase::buildoption, "");

    declareOption(ol, "ker", &KernelVMatrix::ker,
                  OptionBase::buildoption, "");

    inherited::declareOptions(ol);
}


//  IMPLEMENT_NAME_AND_DEEPCOPY(KernelVMatrix);
void KernelVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(source1, copies);
    deepCopyField(source2, copies);
    deepCopyField(ker, copies);
    deepCopyField(input1, copies);
    deepCopyField(input2, copies);
}


real KernelVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length() || j<0 || j>=width())
        PLERROR("In KernelVMatrix::get OUT OF BOUNDS");
#endif

    source1->getRow(i,input1);
    source2->getRow(j,input2);
    return ker(input1,input2);
}


void KernelVMatrix::getSubRow(int i, int j, Vec v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length() || j<0 || j+v.length()>width())
        PLERROR("In KernelVMatrix::getRow OUT OF BOUNDS");
#endif

    source1->getRow(i,input1);
    for(int jj=0; jj<v.length(); jj++)
    {
        source2->getRow(j+jj,input2);
        v[jj] = ker(input1,input2);
    }
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
