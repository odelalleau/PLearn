// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
// Copyright (C) 2008 Xavier Saint-Mleux, ApSTAT Technologies inc.
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


/*! \file DiskVMatrix.h */

#ifndef DiskVMatrix_INC
#define DiskVMatrix_INC

#include "RowBufferedVMatrix.h"
#include <cstdio>

namespace PLearn {
using namespace std;


//!  A VMatrix whose (compressed) data resides in a directory and can span several files.
//!  Each row is compressed/decompressed through the methods of VecCompressor
class DiskVMatrix: public RowBufferedVMatrix
{
    typedef RowBufferedVMatrix inherited;

protected:
    mutable FILE* indexf;
    mutable TVec<FILE*> dataf;
    bool freshnewfile;
    bool old_format; // is thisfile in the old deprecated format?
    bool swap_endians; // was this file written in the opposite endian order
    mutable bool last_op_was_append; //<! true if the last I/O operation performed was an append; if so, we don't need to seek to the end of file before writing again.
public:
    static const int NO_LENGTH_SENTINEL= -42; // written in header instead of actual length
    static const int HEADER_OFFSET_LENGTH= 4; // offset to length in header

    // Build options
    PPath dirname;
    //bool readwritemode;
    double tolerance;    // the error tolerance for storing doubles as floats

    DiskVMatrix();

/*!     Opens an existing one. If directory does not exist or has missing files, an error is issued.
  If readwrite is true, then the files are opened in read/write mode and appendRow can be called.
  If readwrite is false (the default), then the files are opened in read only mode, and calling appendRow
  will issue an error.
*/
    DiskVMatrix(const PPath& the_dirname, bool readwrite=false,
                bool call_build_ = true);

/*!     Create a new one.
  If directory already exist an error is issued
  (you may consider calling force_rmdir prior to this.)
  Howver if it is a file then the file is erased and replaced by a new directory
  (this was to allow TmpFilenames to be used with this class).
  Files are opened in read/write mode so appendRow can be called.
*/
    DiskVMatrix(const PPath& the_dirname, int the_width, bool write_double_as_float=false);

    virtual void putRow(int i, Vec v);
    virtual void appendRow(Vec v);
    virtual void flush();

    virtual void build();

    //! Transform a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    virtual void getNewRow(int i, const Vec& v) const;
    static void declareOptions(OptionList & ol);

    //! Close all current files opened by this DiskVMatrix.
    //! As a side effect, empty the 'dataf' vector and set 'indexf' to 0.
    virtual void closeCurrentFiles();

public:

    PLEARN_DECLARE_OBJECT(DiskVMatrix);


    virtual ~DiskVMatrix();
private:
    void build_();
};

DECLARE_OBJECT_PTR(DiskVMatrix);

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
