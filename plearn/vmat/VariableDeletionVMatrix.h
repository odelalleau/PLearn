// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
// Copyright (C) 2003, 2006 Olivier Delalleau
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


/* ********************************************************************
 * $Id: VariableDeletionVMatrix.h 3658 2005-07-06 20:30:15  Godbout $
 ******************************************************************** */



#ifndef VariableDeletionVMatrix_INC
#define VariableDeletionVMatrix_INC

#include "SelectColumnsVMatrix.h"

namespace PLearn {
using namespace std;

class VariableDeletionVMatrix: public SelectColumnsVMatrix
{
    typedef SelectColumnsVMatrix inherited;

public:

    real    min_non_missing_threshold;
    real    max_constant_threshold;
    int     number_of_train_samples;
    VMat    train_set;

    // Deprecated.
    VMat       complete_dataset;
    real       deletion_threshold;
    int        remove_columns_with_constant_value;

public:

    VariableDeletionVMatrix();

    VariableDeletionVMatrix(VMat the_source,
                            real the_min_non_missing_threshold,
                            bool the_remove_columns_with_constant_value,
                            int  the_number_of_train_samples,
                            bool call_build_ = true);

    virtual void build();
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    static void declareOptions(OptionList &ol);

private:

    void build_();
    void buildIndices();

    PLEARN_DECLARE_OBJECT(VariableDeletionVMatrix);
};

DECLARE_OBJECT_PTR(VariableDeletionVMatrix);

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
