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
 * $Id: ReIndexedTargetVariable.h 3994 2005-08-25 13:35:03Z chapados $
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef ReIndexedTargetVariable_INC
#define ReIndexedTargetVariable_INC

#include "BinaryVariable.h"
#include <plearn/vmat/VMat.h>

namespace PLearn {
using namespace std;

/*
  Inspired from ReIndexedTargetVMatrix, this variable reindexes a
  variable (input1) according to the getValues(row,target_col) function of
  a given VMatrix. The user must specify the input part of the VMatrix
  (input2), and the columns numbers corresponding to input1's fields. 
  In order to construct row when calling getValues(row,target_col), 
  input2 is used for the input part, and the rest of row is filled
  with NaN.
  The user can, alternatively, directly give a Dictionary object
  that will give the possible values. Then, input2 is ignored.
*/
class ReIndexedTargetVariable: public BinaryVariable
{
    typedef BinaryVariable inherited;

protected:
    Vec row;

public:
    //! VMatrix that gives the possible values for the target columns
    VMat source;
    //! Target columns
    TVec<int> target_cols;
    //! Alternatively, the user can give a Dictionary to reindexe
    //! If source is not null, then this field is ignored
    PP<Dictionary> dict;

public:
    //!  Default constructor for persistence
    ReIndexedTargetVariable();
    ReIndexedTargetVariable(Variable* input1, Variable* input2, VMat the_source, TVec<int> the_target_cols);
    ReIndexedTargetVariable(Variable* input1, Variable* input2, PP<Dictionary> the_dict, TVec<int> the_target_cols);

    PLEARN_DECLARE_OBJECT(ReIndexedTargetVariable);
    static void declareOptions(OptionList &ol);

    virtual void build();

    virtual void recomputeSize(int& l, int& w) const;  
    virtual void fprop();
    virtual void bprop();
    virtual void symbolicBprop();
    virtual void rfprop();
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    void build_();
};

DECLARE_OBJECT_PTR(ReIndexedTargetVariable);

inline Var reindexed_target(Var target, Var input, VMat source, TVec<int> target_cols)
{ return new ReIndexedTargetVariable(target,input,source,target_cols); }

inline Var reindexed_target(Var target, Var input, PP<Dictionary> dict, TVec<int> target_cols)
{ return new ReIndexedTargetVariable(target,input,dict,target_cols); }


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
