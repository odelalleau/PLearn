// -*- C++ -*-

// BinaryNumbersVMatrix.cc
//
// Copyright (C) 2007 Yoshua Bengio
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

// Authors: Yoshua Bengio

/*! \file BinaryNumbersVMatrix.cc */


#include "BinaryNumbersVMatrix.h"


namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    BinaryNumbersVMatrix,
    "VMatrix reading file with numbers in various possible binary formats",
    "VMatrix that can take its values from a possibly large file (greater than 2Gig)\n"
    "containing numbers in a user-given binary format, preceded by an arbitrary header whose\n"
    "length is user-given. The user must also specify the dimensions of the matrix\n"
    "(length and width).\n"
    );

BinaryNumbersVMatrix::BinaryNumbersVMatrix()
    : format("u1"), header_size(0), file_is_bigendian(false), f(0), buffer(0)
{
}

void BinaryNumbersVMatrix::getNewRow(int i, const Vec& v) const
{
    PLASSERT_MSG(v.length()==width_,"BinaryNumbersVMatrix::getNewRow(i,v) with v.length!= vmatrix width");
    PRInt64 offset = i;
    offset *= row_size;
    offset += header_size;
    PR_Seek64(f,offset,PR_SEEK_SET);
    PR_Read(f,buffer,row_size);
    bool swap_endian=false;
#ifdef LITTLEENDIAN
    if(file_is_bigendian)
        swap_endian=true;
#endif
#ifdef BIGENDIAN
    if(!file_is_bigendian)
        swap_endian=true;
#endif
    
    int l=v.length();
    if (format=="u1") 
        for (int i=0;i<l;i++)
            v[i] = (real)((unsigned char*)buffer)[i];
    else if (format=="u2") {
        if (swap_endian)
            endianswap2(buffer,width_);
        for (int i=0;i<l;i++)
            v[i] = (real)((unsigned short*)buffer)[i];
    }
    else if (format=="i4") {
        if (swap_endian)
            endianswap4(buffer,width_);
        for (int i=0;i<l;i++)
            v[i] = (real)((int*)buffer)[i];
    }
    else if (format=="f4") {
        if (swap_endian)
            endianswap4(buffer,width_);
        for (int i=0;i<l;i++)
            v[i] = (real)((float*)buffer)[i];
    }
    else if (format=="f8") {
        if (swap_endian)
            endianswap8(buffer,width_);
        for (int i=0;i<l;i++)
            v[i] = (real)((double*)buffer)[i];
    }
    else
        PLERROR("BinaryNumbersVMatrix: unknown format = %s\n",format.c_str());
}

void BinaryNumbersVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "filename", &BinaryNumbersVMatrix::filename,
                   OptionBase::buildoption,
                  "Name of file to be read.");

    declareOption(ol, "format", &BinaryNumbersVMatrix::format,
                   OptionBase::buildoption,
                  "2-character specification of binary format of the numbers in the file:\n"
                  "  u1 = 1-byte unsigned integers\n"
                  "  u2 = 1-byte unsigned integers\n"
                  "  i4 = 4-byte signed integers\n"
                  "  f4 = 4-byte floating point\n"
                  "  f8 = 8-byte floating point\n");

    declareOption(ol, "header_size", &BinaryNumbersVMatrix::header_size,
                   OptionBase::buildoption,
                  "Number of bytes of header at beginning of file.");

    declareOption(ol, "file_is_bigendian", &BinaryNumbersVMatrix::file_is_bigendian,
                   OptionBase::buildoption,
                  "Whether the byte order is 'BIG ENDIAN' or not.");

    declareOption(ol, "row_size", &BinaryNumbersVMatrix::row_size,
                   OptionBase::learntoption,
                  "Number of bytes in each row");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void BinaryNumbersVMatrix::build_()
{
    if (f)
        PR_Close(f);
    updateMtime(filename);
    f = PR_Open(filename.c_str(), PR_RDONLY, 0666);
    if (width_>0)
    {
        if (buffer) 
            delete[] (char*)buffer;
        row_size = width_*(format[1]-'0');
        buffer = (void*) (new char[row_size]);
    }
}

// ### Nothing to add here, simply calls build_
void BinaryNumbersVMatrix::build()
{
    inherited::build();
    build_();
}

void BinaryNumbersVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


BinaryNumbersVMatrix::~BinaryNumbersVMatrix()
{
    if (buffer) delete[] (char*)buffer;
    buffer=0;
    if (f) 
        PR_Close(f);
    f=0;
}


} // end of namespace PLearn


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
