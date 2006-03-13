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

#ifndef KernelVMatrix_INC
#define KernelVMatrix_INC

#include <plearn/ker/Kernel.h>
#include "VMatrix.h"

namespace PLearn {
using namespace std;


/*!   *****************
 * KernelVMatrix *
 *****************
 */

/*!   This VMat is built from 2 other (possibly identical)
  VMats and a Kernel K.
  The resulting VMat takes every sample (x_i,y_i) 
*/

class KernelVMatrix: public VMatrix
{
    typedef VMatrix inherited;
    //protected:
    //!   KernelVMatrix() : source1(), source2(), ker(), input1(), input2() {}
protected:
    VMat source1;
    VMat source2;
    Ker ker;

    Vec input1;
    Vec input2;

public:
    KernelVMatrix();
    KernelVMatrix(VMat the_source1, VMat the_source2, Ker the_ker);

    PLEARN_DECLARE_OBJECT(KernelVMatrix);
    static void declareOptions(OptionList &ol);

    virtual void build();

    //DECLARE_NAME_AND_DEEPCOPY(KernelVMatrix);
    //virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
    virtual real get(int i, int j) const;
    virtual void getSubRow(int i, int j, Vec v) const;
private:
    void build_();
};

DECLARE_OBJECT_PTR(KernelVMatrix);

} // end of namespace PLearn

#endif


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
