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

#ifndef LogAddVariable_INC
#define LogAddVariable_INC

#include "BinaryVariable.h"

namespace PLearn {
using namespace std;

#ifdef __INTEL_COMPILER
//! ICC has trouble finding the right logadd function, thus we give it a hint.
inline real logadd_for_icc(real a, real b) {
    return logadd(a, b);
}
#endif

//!  output = log(exp(input1)+exp(input2)) but it is
//!  computed in such a way as to preserve precision
class LogAddVariable: public BinaryVariable
{
    typedef BinaryVariable inherited;

public:

    string vector_logadd;

    //! Default constructor.
    LogAddVariable() {}

    //! Convenience constructor.
    LogAddVariable(Variable* input1, Variable* input2,
                   const string& the_vector_logadd = "none",
                   bool call_build_ = true);

    PLEARN_DECLARE_OBJECT(LogAddVariable);

    virtual void build();

    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void recomputeSize(int& l, int& w) const;  
    virtual void fprop();
    virtual void bprop();
    virtual void symbolicBprop();

protected:

    //! Integer coding for 'vector_logadd':
    //!     0 <-> 'none'
    //!    -1 <-> 'per_column'
    //!    +1 <-> 'per_row'
    int vector_logadd_id;

    //! Temporary work vector.
    Vec work;

    //! Temporary work vector whose content must not be modified: it can only
    //! be used to point to other data in memory.
    Vec work_ptr;

    static void declareOptions(OptionList & ol);

private:
    void build_();
};

DECLARE_OBJECT_PTR(LogAddVariable);

inline Var logadd(Var& input1, Var& input2)
{ return new LogAddVariable(input1, input2); }

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
