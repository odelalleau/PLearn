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

#ifndef NegCrossEntropySigmoidVariable_INC
#define NegCrossEntropySigmoidVariable_INC

#include "BinaryVariable.h"

namespace PLearn {
using namespace std;


class NegCrossEntropySigmoidVariable: public BinaryVariable
{
    typedef BinaryVariable inherited;
protected:
  
    //! If > 0, will modify the cost function to:
    //! (1-t)(r*log(o)+(1-r)*log(1-o)) + t*(r*log(1-o)+(1-r)*log(o))
    //! (t = target, o = output, r = regularizer = a small value)
    real regularizer;
    //! Indication that missing targets should be ignored
    bool ignore_missing;
  
public:
    //!  Default constructor for persistence
    NegCrossEntropySigmoidVariable() 
        : regularizer(0.0), ignore_missing(false) {}

    NegCrossEntropySigmoidVariable(Variable* netout, Variable* target, real regularizer_ = 0, bool ignore_missing_ = false);

    PLEARN_DECLARE_OBJECT(NegCrossEntropySigmoidVariable);

    virtual void build();

    virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();
    //!  Deprecated
    void setRegularizer(real r);

protected:
    static void declareOptions(OptionList & ol);

    void build_();
};

DECLARE_OBJECT_PTR(NegCrossEntropySigmoidVariable);

inline Var stable_cross_entropy(Var linear_output, Var target, bool ignore_missing=false)
{
    return new NegCrossEntropySigmoidVariable(linear_output, target, 0, ignore_missing);
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
