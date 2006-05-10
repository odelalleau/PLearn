// -*- C++ -*-

// AsciiVMatrix.h
//
// Copyright (C) 2003 Rejean Ducharme, Pascal Vincent
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

/*! \file AsciiVMatrix.h */
#ifndef AsciiVMatrix_INC
#define AsciiVMatrix_INC

#include "RowBufferedVMatrix.h"
#include <fstream>

namespace PLearn {
using namespace std;

class AsciiVMatrix: public RowBufferedVMatrix
{
    typedef RowBufferedVMatrix inherited;

protected:
    string filename;
    mutable PStream file;
    vector<streampos> pos_rows;
    streampos vmatlength_pos;
    bool readwritemode;
    bool newfile;
    bool rewrite_length;
    int length_max;

public:
    // ******************
    // *  Constructors  *
    // ******************
    AsciiVMatrix(); //!<  default constructor (for automatic deserialization)

    // Open an existing file.
    // If readwrite==false, the appendRow method sends an error.
    // If readwrite==true (the default), the file is open in read/write mode.
    AsciiVMatrix(const string& fname, bool readwrite=true);

    // Open a new file (in read/write mode).
    // If a file of the same name already exist, an error is sent!
    // The width of the matrix to be writen must be given.
    // Optional fieldnames may be given (as many as width, blanks will be converted to underscore)
    // Optional comment string (must start with a #, may be multiline with \n provided each line starts with a #).
    AsciiVMatrix(const string& fname, int the_width,
                 const TVec<string>& fieldnames = TVec<string>(),
                 const string& the_comment="");

private:
    //! This does the actual building.
    void build_();

protected:
    //! Declares this class' options
    static void declareOptions(OptionList& ol);

    //!  This is the only method requiring implementation
    virtual void getNewRow(int i, const Vec& v) const;

public:
    //! Append a row at the end of the file
    virtual void appendRow(Vec v);

    //! These methods send an error message since, by choice,
    //! we accept only to append row at the end of the file
    virtual void put(int i, int j, real value);
    virtual void putSubRow(int i, int j, Vec v);
    virtual void putRow(int i, Vec v);

    // simply calls inherited::build() then build_()
    virtual void build();

    virtual ~AsciiVMatrix();

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(AsciiVMatrix);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(AsciiVMatrix);

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
