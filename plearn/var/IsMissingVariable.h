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

#ifndef IsMissingVariable_INC
#define IsMissingVariable_INC

#include "UnaryVariable.h"

namespace PLearn {
using namespace std;


//!  A scalar var;  equal 1 if input1!=c, 0 otherwise
class IsMissingVariable: public UnaryVariable
{
    typedef UnaryVariable inherited;

protected:
    //! if true then output 1 value per input, otherwise output 1 if ANY of the inputs is missing
    bool parallel; 
    //! Indication that, instead of outputing 0 if the input value is NaN, 1 otherwise,
    //! this variable will output parallel_missing_output if the input value is NaN, 
    //! or the actual input value otherwise
    bool set_parallel_missing_output;
    //! Output value when input is missing (NaN)
    Vec parallel_missing_outputs;

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

public:
    //!  Default constructor for persistence
    IsMissingVariable() {}
    IsMissingVariable(Variable* input1, bool parallel=0, bool set_parallel_missing_output=0, Vec parallel_missing_outputs = Vec(0));

    PLEARN_DECLARE_OBJECT(IsMissingVariable);
    static void declareOptions(OptionList &ol);

    // Simply calls inherited::build() then build_()
    virtual void build();

    virtual string info() const
    { return string("IsMissingVariable ()"); }

    virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();
    virtual void symbolicBprop();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

DECLARE_OBJECT_PTR(IsMissingVariable);

inline Var isMissing(Var x, bool parallel, bool set_parallel_missing_output, real parallel_missing_output) { 
    Vec parallel_missing_outputs(x->size()); 
    parallel_missing_outputs.fill(parallel_missing_output); 
    return new IsMissingVariable(x, parallel,set_parallel_missing_output,parallel_missing_outputs); }
inline Var isMissing(Var x, bool parallel=0, bool set_parallel_missing_output=0, Vec parallel_missing_outputs = Vec(0)) { return new IsMissingVariable(x, parallel,set_parallel_missing_output,parallel_missing_outputs); }

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
