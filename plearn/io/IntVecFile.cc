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
 * AUTHORS: Pascal Vincent
 * This file is part of the PLearn library.
 ******************************************************* */

#include "IntVecFile.h"
//#include "fileutils.h"
#include <plearn/base/byte_order.h>

namespace PLearn {
using namespace std;

const char IntVecFile::signature[] = {
    '\xDE', '\xAD', '\xBE', '\xEF', '\x00'     //!< deadbeef, really...
};

const int IntVecFile::header_size[] = {      //!< in number of ints
    0,                                         //!< version 0
    2                                          //!< version 1
};


IntVecFile::IntVecFile(const IntVecFile& other)
    : filename(other.filename), f(0), length_(other.length_),
      version_number_(other.version_number_), endianness_(other.endianness_)
{
    open(filename, false /* readonly */);
}

void IntVecFile::open(const string& the_filename, bool readwrite)
{
    if(f)
        close();

    filename = the_filename;
    bool file_exists = isfile(filename);
    if(readwrite)
    {
        f = fopen(filename.c_str(),"a+b");
        if(!f)
            PLERROR("Couldn't open file %s for read/write",filename.c_str());
    }
    else
    {
        f = fopen(filename.c_str(),"rb");
        if(!f)
            PLERROR("Couldn't open file %s for reading",filename.c_str());
    }
    if (file_exists)
        getVersionAndSize();
    else
        writeFileSignature();
}

int IntVecFile::get(int i) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length())
        PLERROR("Out Of Bounds in IntVecFile::get");
#endif
    seek_to_index(i);
    return fread_int(f, endianness_ == BIG_ENDIAN_ORDER);
}

void IntVecFile::put(int i, int value)
{
    seek_to_index(i);
    fwrite_int(f, value, endianness_ == BIG_ENDIAN_ORDER);
    if(i>=length_)
        length_ = i+1;
}

void IntVecFile::close()
{
    if(f)
        fclose(f);
    f=0;
}

IntVecFile::~IntVecFile()
{
    close();
}

#ifdef __INTEL_COMPILER
#pragma warning(disable:593)  // Get rid of compiler warning.
#endif
TVec<int> IntVecFile::getVec() const
{
    size_t tt;
    TVec<int> res(length());
    seek_to_index(0);
    if((tt=fread(res.data(), sizeof(int), length(), f)) != size_t(length()))
        PLERROR("fread error in IntVecFile::getVec()");
    // Switch byte order if necessary
    if (byte_order() != endianness_)
        endianswap(res.data(), length());

    return res;
}
#ifdef __INTEL_COMPILER
#pragma warning(default:593)
#endif

void IntVecFile::append(const TVec<int>& vec)
{
    seek_to_index(length());

    if (byte_order() != endianness_) {
        TVec<int> new_vec(vec.length());
        new_vec << vec;
        endianswap(new_vec.data(), new_vec.length());
        fwrite(new_vec.data(), sizeof(int), new_vec.length(), f);
    }
    else {
        fwrite(vec.data(), sizeof(int), vec.length(), f);
    }

    length_ += vec.length();
}

void IntVecFile::writeFileSignature()
{
    // This is for a new file.  Assume length 0, version number 1,
    // file endianness is current-platform endianness
    length_ = 0;
    version_number_ = 1;
    endianness_ = byte_order();

    fseek(f, 0, SEEK_SET);
    fputs(signature, f);                       //!< write without \0
    fputc(endianness_, f);
    fputc(0x00, f);
    fputc(0x00, f);
    fputc(char(version_number_), f);
}

void IntVecFile::getVersionAndSize()
{
    if (sizeof(int) != 4)
        PLERROR("IntVecFile::getVersionAndSize: "
                "IntVecFile not yet designed to handle sizeof(int) != 4");
  
    long the_filesize = filesize(filename);
    if (the_filesize < long(2*sizeof(int)) /* assume 4 */)
        goto version0;                           // unbelievable but true!

    fseek(f, 0, SEEK_SET);
    for (int i=0; i<4; ++i)
        if (char(fgetc(f)) != signature[i])
            goto version0;                         //!< unbelievable (bis)

    // Assume new-world file format
    endianness_ = char(fgetc(f));
    if (endianness_ != LITTLE_ENDIAN_ORDER &&
        endianness_ != BIG_ENDIAN_ORDER)
        PLERROR("IntVecFile::getVersionAndSize: "
                "File format error in file %s Only supported endianness is 'L' or 'B'\n"
                "Read %c", filename.c_str(),endianness_);

    if(fgetc(f) != 0x00 || fgetc(f) != 0x00)
        PLERROR("IntVecFile::getVersionAndSize: "
                "File format error in file %s", filename.c_str());

    version_number_ = (unsigned char)fgetc(f);

    if (version_number_ > 1)
        PLERROR("IntVecFile::getVersionAndSize: "
                "File version (%d) is not supported", version_number_);

    length_ = int(the_filesize / sizeof(int) - header_size[version_number_]);
    return;

 version0:
    length_ = int(the_filesize / sizeof(int));
    version_number_ = 0;
    endianness_ = 'L';
}

void IntVecFile::seek_to_index(int i) const
{
    fseek(f, (i+header_size[version_number_]) * sizeof(int), SEEK_SET);
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
