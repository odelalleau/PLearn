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

#ifndef AffineTransformVariable_INC
#define AffineTransformVariable_INC

#include "BinaryVariable.h"
// For inline function that can return either an AffineTransformVariable or a MatrixAffineTransformVariable
#include "MatrixAffineTransformVariable.h"

namespace PLearn {
using namespace std;


class AffineTransformVariable: public BinaryVariable
{
    typedef BinaryVariable inherited;

public:

    // Public options.

    bool force_row_vec;

    //!  Default constructor for persistence
    AffineTransformVariable() {}
    AffineTransformVariable(Variable* vec, Variable* transformation,
                            bool call_build_ = true);


    PLEARN_DECLARE_OBJECT(AffineTransformVariable);

    virtual void build();

    virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();
    virtual void symbolicBprop();

private:

    void build_();

protected:

    static void declareOptions(OptionList& ol);

};

DECLARE_OBJECT_PTR(AffineTransformVariable);

//! First row of transformation is the bias.
//! The boolean 'force_row_vec' may be used when the first input is a
//! a vector, to force the resulting variable to be a row vector, even
//! when the input is a column vector or a scalar.
//! If the first input is a matrix, this parameter is ignored.
inline Var affine_transform(Var vec, Var transformation,
                            bool force_row_vec = false)
{ 
    if (vec->isVec()) {
        PP<AffineTransformVariable> res =
            new AffineTransformVariable(vec, transformation, false);
        res->force_row_vec = force_row_vec;
        res->build();
        return get_pointer(res);
    }
    else return new MatrixAffineTransformVariable(vec, transformation);
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
