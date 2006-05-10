// -*- C++ -*-

// ReIndexedTargetVMatrix.h
//
// Copyright (C) 2005 Hugo Larochelle
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

/* *******************************************************
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file ReIndexedTargetVMatrix.h */


#ifndef ReIndexedTargetVMatrix_INC
#define ReIndexedTargetVMatrix_INC

#include <plearn/vmat/SourceVMatrix.h>

namespace PLearn {

/**
   VMatrix the reindexes the target fields of a source VMatrix,
   according to the getValues(row,target_col) function, where row
   contains the values of a row of the source VMatrix, and target_col
   is the column index of (one of ) the target field. This is useful
   when the possible target values change from one row to another.
   This way, target values will range from 0 to getValues(row,target_col).length()-1.
   The source VMatrix must be able to fournish the Dictionary objects for the
   target fields.
 */
class ReIndexedTargetVMatrix : public SourceVMatrix
{
    typedef SourceVMatrix inherited;

public:

    //! Indication that the OOV_TAG must be ignored in the possible values
    bool ignore_oov_tag;

public:

    //! Default constructor
    ReIndexedTargetVMatrix();

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(ReIndexedTargetVMatrix);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Fill the vector 'v' with the content of the i-th row.
    //! v is assumed to be the right size.
    virtual void getNewRow(int i, const Vec& v) const;

private:

    //! This does the actual building.
    void build_();

private:

    //! Vector of possible values
    Vec values;

    //! getNewRow temporary values
    int t,index,oov_index;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ReIndexedTargetVMatrix);

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
