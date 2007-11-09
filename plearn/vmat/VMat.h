// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file VMat.h */

#ifndef VMat_INC
#define VMat_INC

#include "VMatrix.h"
//#include <cstdlib>
#include <plearn/base/PP.h>
#include <plearn/math/TMat.h>
//#include "VMField.h"
#include "VVec.h"

namespace PLearn {
using namespace std;

class VMat: public PP<VMatrix>
{
public:
    VMat();
    VMat(VMatrix* d);
    VMat(const VMat& d);
    VMat(const Mat& datamat); //!<  Convenience constructor: will use an MemoryVMatrix built from datamat

    //! Return the number of rows in the matrix.
    int length() const { return ptr->length(); }

    //! Return the number of columns in the matrix.
    int width() const { return ptr->width(); }

    string fieldName(int fieldindex) const
    { return ptr->fieldName(fieldindex); }

    int getFieldIndex(const string& fieldname_or_num) const
    { return ptr->getFieldIndex(fieldname_or_num); }


    real operator()(int i, int j) const { return ptr->get(i,j); }
    VVec operator()(int i) const { return VVec(*this, i); }
    Vec getColumn(int i) const { Vec v(ptr->length()); ptr->getColumn(i,v); return v; }
    Vec getSubRow(int i, int s) const { Vec v(s); ptr->getSubRow(i, 0, v); return v; }

    VMat subMat(int i, int j, int l, int w) const { return ptr->subMat(i,j,l,w); }
    VMat subMatRows(int i, int l) const;
    //VMat subMatRows(int i, int l) const { return ptr->subMat(i,0,l,width()); }
    VMat subMatColumns(int j, int w) const { return ptr->subMat(0,j,length(),w); }

    inline void getExample(int i, Vec& input, Vec& target, real& weight)
    { ptr->getExample(i, input, target, weight); }

    VMat row(int i) const { return subMatRows(i,1); }
    VMat firstRow() const { return row(0); }
    VMat lastRow() const { return row(length()-1); }
    VMat column(int j) const { return subMatColumns(j,1); }
    VMat firstColumn() const { return column(0); }
    VMat lastColumn() const { return column(width()-1); }
    Mat toMat() const { return ptr->toMat();}

    //!  Returns a VMatrix made of only the specified rows
    VMat rows(TVec<int> rows_indices) const;
    //!  Returns a VMatrix made of only the specified rows
    VMat rows(Vec rows_indices) const;
    //!  Returns a VMatrix made of only the rows specified in the indexfile (see IntVecFile)
    VMat rows(const PPath& indexfile) const;

    //!  Returns a VMatrix made of only the specified columns
    VMat columns(TVec<int> columns_indices) const;
    //!  Returns a VMatrix made of only the specified columns
    VMat columns(Vec columns_indices) const;


    operator Mat() const { return ptr->toMat(); }
    inline void save(const PPath& filename) const { ptr->save(filename); }

    //!  Will copy a precomputed version of the whole VMat into memory
    //!  and replace the current pointer to point to the corresponding MemoryVMatrix.
    //!  Note that some info will be lost (like fields infos): check the .cc
    //!  to understand why.
    void precompute();

/*!     will copy a precomputed version of the whole VMat to the given file
  and replace the current pointer to point to the corresponding FileVMatrix
  For fast access, make sure the file is on a local Disk rather than on
  a Network Mounted File System.
  If use_existing_file is true, it will use the existing file (from a
  previous precomputation for instance) rather than overwriting it (make
  sure the file indeed contains what you expect!)
*/
    void precompute(const PPath& pmatfile, bool use_existing_file=false);

//  inline void print(PStream& out) const { ptr->print(out); }

    ~VMat();
};

DECLARE_OBJECT_PP(VMat, VMatrix);

inline void operator<<(const Mat& dest, const VMatrix& src)
{
    if(dest.length()!=src.length() || dest.width()!=src.width())
        PLERROR("In operator<<(const Mat& dest, const VMatrix& src), incompatible dimensions");
    src.getMat(0,0,dest);
}

inline void operator>>(const VMatrix& src, const Mat& dest)
{ dest << src; }

inline void operator<<(const Mat& dest, const VMat& src)
{ dest << *(VMatrix*)src; }

inline void operator>>(const VMat& src, const Mat& dest)
{ dest << src; }



/*! ********************************
 * User-friendly VMat interface *
 ********************************
 */

inline Array<VMat> operator&(const VMat& d1, const VMat& d2)
{ return Array<VMat>(d1,d2); }

/* Already defined in the DECLARE_OBJECT_PP macro.
   inline PStream& operator<<(PStream& out, const VMat& m)
   { m.print(out); return out; }
*/

template <> void deepCopyField(VMat& field, CopiesMap& copies);

//! Load an ASCII file and return the corresponding VMat (this will be
//! a MemoryVMatrix, since the entire file is loaded in memory).
VMat loadAsciiAsVMat(const PPath& filename);

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
