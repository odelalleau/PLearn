// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2006 Xavier Saint-Mleux
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


#include "BufferedIntVecFile.h"

namespace PLearn {
using namespace std;

/*
BufferedIntVecFile::BufferedIntVecFile(const IntVecFile& other)
    : filename(other.filename), f(0), length_(other.length_),
      version_number_(other.version_number_), endianness_(other.endianness_)
{
    open(filename, false / * readonly * /);
}
*/

void BufferedIntVecFile::open(const string& the_filename, bool readwrite)
{
    flush();
    inherited::open(the_filename, readwrite);
    bufstart= -1;
    bufmod= false;
}

void BufferedIntVecFile::close()
{
    flush();
    inherited::close();
}

int BufferedIntVecFile::get(int i) const
{
    if(bufstart < 0 || bufstart > i || i >= bufstart+buflen)
        const_cast<BufferedIntVecFile*>(this)->getBuf(i);
    return buf[i-bufstart];
}

void BufferedIntVecFile::put(int i, int value)
{
    if(bufstart < 0 || bufstart > i || i >= bufstart+buflen)
        getBuf(i);
    bufmod= true;
    buf[i-bufstart]= value;
    if(i>=length_)
        length_ = i+1;
}


BufferedIntVecFile::~BufferedIntVecFile()
{
    close();
    if(buf) delete[] buf;
}

void BufferedIntVecFile::getBuf(int bufstart_)
{
    flush();
    bufstart= bufstart_;
    for(int i= 0; i < buflen && i+bufstart < length(); ++i)
        buf[i]= inherited::get(i+bufstart);
}

void BufferedIntVecFile::flush()
{
    if(bufmod)
        for(int i= 0; i < buflen && i+bufstart < length(); ++i)
            inherited::put(i+bufstart, buf[i]);
    bufmod= false;
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
