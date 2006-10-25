// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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


/*! \file ConcatRowsVMatrix.h */

#ifndef ConcatRowsVMatrix_INC
#define ConcatRowsVMatrix_INC

#include "VMat.h"

namespace PLearn {
using namespace std;

class ConcatRowsVMatrix: public VMatrix
{
    typedef VMatrix inherited;

protected:
    //! The (original) VMats to concatenate
    TVec<VMat> sources;

    //! A vector containing the final VMats to concatenate.
    //! These are either the same as the ones in 'sources', or a selection
    //! of their fields when the 'only_common_fields' option is true.
    TVec<VMat> to_concat;

    //! Will be set to 'true' iff two VMats concatenated have not the same
    //! mapping for a given string. This means the output must systematically
    //! be checked to ensure consistency.
    bool need_fix_mappings;

    //! This is a matrix of size (number of matrices to concatenate, number of
    //! columns). The element (i, j) is a mapping that says which value needs
    //! to be replaced with  what in the j-th column of the i-th matrix. This
    //! is to fix the mappings when 'need_fix_mappings' is true.
    TMat< map<real, real> > fixed_mappings;

public:

    bool fill_missing;
    bool fully_check_mappings;
    bool only_common_fields;

    //! The fields names are copied from the FIRST VMat, unless the
    //! 'only_common_fields' option is set to 'true'.
    ConcatRowsVMatrix(TVec<VMat> the_sources = TVec<VMat>());
    ConcatRowsVMatrix(VMat d1, VMat d2, bool fully_check_mappings = false);

    virtual real get(int i, int j) const;
    virtual void getSubRow(int i, int j, Vec v) const;

    virtual void reset_dimensions();

    virtual real dot(int i1, int i2, int inputsize) const;
    virtual real dot(int i, const Vec& v) const;
    virtual void putMat(int i, int j, Mat m);

    PLEARN_DECLARE_OBJECT(ConcatRowsVMatrix);
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    /// Return the index of the correct VMat in the sources and the row number
    /// in this VMat that correspond to row i in the ConcatRowsVMat.  The
    /// inline version performs a cache lookup, or calls the out-of-line
    /// version getPositionsAux, which performs a linear search
    inline void getpositions(int i, int& whichvm, int& rowofvm) const;

    static void declareOptions(OptionList &ol);

private:

    void build_();

    //! Build the string <-> real mappings so that they are consistent with the
    //! different mappings from the concatenated VMats (the same string must
    //! be mapped to the same value).
    void ensureMappingsConsistency();

    //! Browse through all data in the VMats to make sure there is no numerical
    //! value conflicting with a string mapping. An error occurs if it is the case.
    void fullyCheckMappings(bool report_progress = true);

    //! Selects all the fields that appear in any of the concatenated VMat.
    void findAllFields();

    //! Selects the fields common to all VMats to concatenate (called at build
    //! time if 'only_common_fields' is true).
    void findCommonFields();

    //! Recompute length and width (same as reset_dimensions(), except it does not
    //! forward to the underlying VMats).
    void recomputeDimensions();

    /// Used in the implementation of getpositions (out-of-line version)
    void getPositionsAux(int i, int& whichvm, int& rowofvm) const;

private:
    /// Cache of the index of the last-recently used VMat; -1 if invalid
    mutable int m_last_vmat_index;

    /// Cache of the starting row in the ConcatRowsVMatrix corresponding to
    /// to the first row of m_last_vmat_index; -1 if invalid
    mutable int m_last_vmat_startrow;

    /// Cache of the last row (inclusive) in the ConcatRowsVMatrix
    /// corresponding to m_last_vmat_index; -1 if invalid
    mutable int m_last_vmat_lastrow;
};

DECLARE_OBJECT_PTR(ConcatRowsVMatrix);

inline VMat vconcat(VMat d1, VMat d2)
{ return new ConcatRowsVMatrix(d1,d2); }

inline VMat vconcat(TVec<VMat> ds) {
    if (ds.size() == 1) {
        // Only one matrix: no need to use a ConcatRowsVMatrix.
        return ds[0];
    } else {
        return new ConcatRowsVMatrix(ds);
    }
}

//////////////////
// getpositions //
//////////////////
inline void ConcatRowsVMatrix::getpositions(int i, int& whichvm, int& rowofvm) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length())
        PLERROR("In ConcatRowsVMatrix::getpositions OUT OF BOUNDS");
#endif

    // Start by looking if i belongs to the most-recently-used VMat. If so, no
    // need to search for it
    if (i >= m_last_vmat_startrow && i <= m_last_vmat_lastrow) {
        PLASSERT( m_last_vmat_index >= 0 );
        whichvm = m_last_vmat_index;
        rowofvm = i - m_last_vmat_startrow;
        return;
    }

    // Now in cache: perform linear search out-of-line
    getPositionsAux(i, whichvm, rowofvm);
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
