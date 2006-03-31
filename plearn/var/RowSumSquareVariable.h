// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
//
// Copyright (C) 2006 University of Montreal

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
 * $Id: RowSumSquareVariable.h 3994 2005-08-25 13:35:03Z chapados $
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef RowSumSquareVariable_INC
#define RowSumSquareVariable_INC

#include "UnaryVariable.h"

namespace PLearn {
using namespace std;


class RowSumSquareVariable: public UnaryVariable
{
    typedef UnaryVariable inherited;

public:
    //!  Default constructor for persistence
    RowSumSquareVariable() {}
    RowSumSquareVariable(Variable* input);

    PLEARN_DECLARE_OBJECT(RowSumSquareVariable);

    virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();

};

DECLARE_OBJECT_PTR(RowSumSquareVariable);

inline Var rowSumSquare(Var v)
{ 
    return new RowSumSquareVariable(v); 
}

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
