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


/*! \file FileVMatrix.h */

#ifndef FileVMatrix_INC
#define FileVMatrix_INC

#include "RowBufferedVMatrix.h"
#include <nspr/prlong.h>
// While under development, we use this define to control
// whether to use the NSPR 64 bit file access or the old std C FILE*
#define USE_NSPR_FILE
struct PRFileDesc;

namespace PLearn {
using namespace std;

//! A VMatrix that exists in a .pmat file (native PLearn matrix format,
//! same as for Mat).
class FileVMatrix: public RowBufferedVMatrix
{

private:

    typedef RowBufferedVMatrix inherited;

protected:

    PPath filename_;

#ifdef USE_NSPR_FILE
    PRFileDesc* f;
#else
    FILE* f;
#endif
    bool file_is_bigendian;
    bool file_is_float;
    bool force_float;

private:

    bool build_new_file;
    void openfile(const PPath& path, const char *mode, PRIntn flags,
                  PRIntn mode2);

public:

    FileVMatrix();
    FileVMatrix(const PPath& filename, bool writable_=false); //!<  opens an existing file
    FileVMatrix(const PPath& filename, int the_length, int the_width,
                bool force_float=false, bool call_build_ = true); //!<  create a new matrix file
    FileVMatrix(const PPath& filename, int the_length, const TVec<string>& fieldnames); //!<  create a new matrix file

protected:

    static void declareOptions(OptionList & ol);
    virtual void getNewRow(int i, const Vec& v) const;

    //! Close the current '.pmat' file.
    virtual void closeCurrentFile();

public:

     int remove_when_done; //!< Deprecated!
     int track_ref;        //!< Deprecated!

    //! Re-write the header with all current field values.
    virtual void updateHeader();

    virtual void put(int i, int j, real value);
    virtual void putSubRow(int i, int j, Vec v);
    virtual void appendRow(Vec v);
    virtual void flush();

    virtual void build();

    PLEARN_DECLARE_OBJECT(FileVMatrix);


    //! Transform a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Destructor.
    virtual ~FileVMatrix();

    virtual int64_t getSizeOnDisk();
private:

    void build_();
    // seek to element i,j in file
    void moveto(int i, int j=0) const;

};

DECLARE_OBJECT_PTR(FileVMatrix);

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
