// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Pascal Vincent, Olivier Delalleau
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
 * $Id$
 ******************************************************* */


#ifndef IndexedVMatrix_INC
#define IndexedVMatrix_INC

#include "SourceVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;

//! VMat class that sees a matrix as a collection of triplets
//! (row, column, value)
//! Thus it is a N x 3 matrix, with N = the number of elements in the original
//! matrix.
class IndexedVMatrix: public SourceVMatrix
{
    typedef SourceVMatrix inherited;

protected:
    //! Will be set to 'true' iff two columns of source have not the same
    //! mapping for a given string. This means the output must systematically
    //! be checked to ensure consistency.
    bool need_fix_mappings;

    //! This is a vector with as many elements as columns in source.
    //! The element (i) is a mapping that says which value needs to be
    //! replaced with what in the i-th column of the source matrix. This
    //! is to fix the mappings when 'need_fix_mappings' is true.
    TVec< map<real, real> > fixed_mappings;


public:
    //! Public build options
// - DEPRECATED    VMat m;
    bool fully_check_mappings;

    IndexedVMatrix(bool call_build_ = false);

    IndexedVMatrix(VMat the_source,
                   bool the_fully_check_mappings = false,
                   bool call_build_ = false);

    PLEARN_DECLARE_OBJECT(IndexedVMatrix);

    static void declareOptions(OptionList& ol);

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
    virtual void build();
    virtual void getNewRow(int i, const Vec&v) const;
//    virtual real get(int i, int j) const;
    virtual void put(int i, int j, real value);

private:

    void build_();

    //! Build the string <-> real mappings so that they are consistent
    //! with the different mappings from the concatenated VMats (the
    //! same string must be mapped to the same value).
    void ensureMappingsConsistency();

    //! Browse through all data in the VMats to make sure there is no
    //! numerical value conflicting with a string mapping.  An error
    //! occurs if it is the case.
    void fullyCheckMappings();

};

DECLARE_OBJECT_PTR(IndexedVMatrix);

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
