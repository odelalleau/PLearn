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

#ifndef IfThenElseVariable_INC
#define IfThenElseVariable_INC

#include "NaryVariable.h"

namespace PLearn {
using namespace std;


/**
 *  Variable that represents the element-wise IF-THEN-ELSE:
 *  - The first parent is the test (0 or different from 0)
 *  - The second parent is the value returned when the test is !=0,
 *  - The third parent is the value returned when the test is ==0,
 */
class IfThenElseVariable: public NaryVariable
{
    typedef NaryVariable inherited;

public:
    //!  Default constructor for persistence
    IfThenElseVariable() {}
    IfThenElseVariable(Var IfVar, Var ThenVar, Var ElseVar);

    PLEARN_DECLARE_OBJECT(IfThenElseVariable);

    virtual void build();

    virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();
    virtual void symbolicBprop();
    virtual void rfprop();
    Var& If() { return varray[0]; }
    Var& Then() { return varray[1]; }
    Var& Else() { return varray[2]; }

protected:
    void build_();
};

DECLARE_OBJECT_PTR(IfThenElseVariable);

//!  IT WOULD BE NICE IF WE COULD REDEFINE (:?)
inline Var ifThenElse(Var If, Var Then, Var Else)
{ return new IfThenElseVariable(If,Then,Else); }

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
