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

#ifndef ArgminOfVariable_INC
#define ArgminOfVariable_INC

#include "NaryVariable.h"

namespace PLearn {
using namespace std;


/*! returns the value of v within the_values_of_v that gives the lowest
  value of expression (which may depend on inputs). */
class ArgminOfVariable: public NaryVariable
{

    typedef NaryVariable inherited;
    
protected:
    //!  protected default constructor for persistence
    ArgminOfVariable() : inputs(), expression(), values_of_v(), v(),
                         index_of_argmin(), vv_path(), e_path(), v_path() {}

protected:
    VarArray inputs;
    Var expression;
    Var values_of_v;
    Var v;
    int index_of_argmin;

    VarArray vv_path; //!<  values_of_v(inputs)
    VarArray e_path; //!<  expression(v&inputs)
    VarArray v_path; //!<  expression(v) 

public:
    ArgminOfVariable(Variable* the_v,
                     Variable* the_expression,
                     Variable* the_values_of_v,
                     const VarArray& the_inputs);
    PLEARN_DECLARE_OBJECT(ArgminOfVariable);
    virtual void recomputeSize(int& l, int& w) const;
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
    virtual void fprop();
    virtual void bprop();
};

/*! returns the value of v within the_values_of_v that gives the lowest
  value of expression (which may depend on inputs). */
inline Var argminOf(Var v, Var expression, Var values_of_v, VarArray inputs)
{ return new ArgminOfVariable(v, expression, values_of_v, inputs); }

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
