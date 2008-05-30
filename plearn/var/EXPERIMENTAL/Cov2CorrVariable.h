// -*- C++ -*-

// Cov2CorrVariable.h
//
// Copyright (C) 2008 Pascal Vincent
//
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

// Authors: Pascal Vincent

/*! \file Cov2CorrVariable.h */

#ifndef Cov2CorrVariable_INC
#define Cov2CorrVariable_INC

#include <plearn/var/UnaryVariable.h>

namespace PLearn {
using namespace std;


class Cov2CorrVariable: public UnaryVariable
{
    typedef UnaryVariable inherited;

public:
    int diagonal_choice;
    double epsilon;

    //! Default constructor.
    Cov2CorrVariable();

    Cov2CorrVariable(Variable* input, int diagonal_choice_=1,
                     double epsilon_=0, bool call_build_ = true);

    PLEARN_DECLARE_OBJECT(Cov2CorrVariable);

    static void declareOptions(OptionList& ol);

    virtual void build();

    virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();
    //!  here don't approximate, do d2C/dx^2 = 4 x^2 d2C/dy^2 + 2 dC/dy 
    virtual void bbprop();
    virtual void symbolicBprop();
    virtual void rfprop();

private:

    void build_();

};

DECLARE_OBJECT_PTR(Cov2CorrVariable);

inline Var cov2corr(Var v, int diagonal_choice=1, double epsilon=0.)
{ return new Cov2CorrVariable(v,diagonal_choice,epsilon); }

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
