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

#ifndef SubMatVariable_INC
#define SubMatVariable_INC

#include "UnaryVariable.h"

namespace PLearn {
using namespace std;

/**
 *  Takes a submatrix of an input variable.
 *
 *  This Variable performs creates a view of a subset of an input variable.
 *  The starting row and column in the input variable must be specified, as
 *  well as the new number of rows and columns.
 *
 *  Variables of this kind can also be created from C++ through the subMat
 *  function.
 */
class SubMatVariable: public UnaryVariable
{
    typedef UnaryVariable inherited;

protected:
    //! Should not be set directly: this is equal to the starting element
    //! in the INPUT MATRIX corresponding to the settings of i_ and j_.
    int startk;

    //! New length (number of rows) of the SubVMatrix variable
    int length_;

    //! New width (number of columns) of the SubVMatrix variable
    int width_;

    //! Starting ROW in the input variable
    int i_;

    //! Starting COLUMN in the input variable
    int j_;

public:

    //!  Default constructor for persistence
    SubMatVariable() : startk(0), length_(0), width_(0), i_(0), j_(0) {}
    SubMatVariable(Variable* v, int i, int j, int the_length, int the_width);
    PLEARN_DECLARE_OBJECT(SubMatVariable);
    static void declareOptions(OptionList &ol);

    virtual void build();

    virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();
    virtual void bbprop();
    virtual void symbolicBprop();
    virtual void rfprop();

private:
    void build_();

};

DECLARE_OBJECT_PTR(SubMatVariable);

inline Var subMat(Var v, int i, int j, int l, int w) {
    return new SubMatVariable(v, i, j, l, w);
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
