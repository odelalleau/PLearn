// -*- C++ -*-

// openFile.cc
//
// Copyright (C) 2004 Pascal Vincent 
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

// Authors: Pascal Vincent, Christian Hudon

/*! \file openFile.cc */


#include "openFile.h"
#include <nspr/prio.h>
#include <plearn/io/fileutils.h>
#include <plearn/io/PrPStreamBuf.h>

namespace PLearn {
using namespace std;

/** Given a filename, opens the file and returns a PStream that can be
 *  used to read and/or write to the file.
 *
 *  @param filepath_ The filename to be opened. Slashes will automatically
 *  be converted to the path seperator of the underlying OS before
 *  opening the file.
 *
 *  @param io_formatting The PStream formatting that will be used when
 *  reading/writing to the file. Common modes include PStream::raw_ascii
 *  (for a normal ascii text file) and PStream::plearn_ascii (for files
 *  in the PLearn serialization format).
 *
 *  @param openmode The mode (read/write/append) to open the file in. Use
 *  "r" for opening the file for reading, "w" for writing (overwrites the
 *  file if it exists), or "a" for appending to the file (creating it if
 *  it doesn't exist). The default is to open the file for reading ("r").
 */
PStream openFile(const PPath& filepath_, PStream::mode_t io_formatting,
                 const string& openmode)
{
    const PPath filepath = filepath_.absolute();
    
    PStream st;
    PRFileDesc* fd;
    if ((openmode == "r" || openmode == "a") && isdir(filepath))
        PLERROR("In openFile(..) - Cannot open this directory for reading or "
                "append (%s), it should be a file!", filepath.c_str());

    if (openmode == "r")
    {
        fd = PR_Open(filepath.c_str(), PR_RDONLY, 0666);
        if (!fd)
            PLERROR("openFile(\"%s\",\"%s\") failed.",filepath.c_str(), openmode.c_str());
        st = new PrPStreamBuf(fd, 0, true, false);
    }
    else if (openmode == "w")
    {
        fd = PR_Open(filepath.c_str(), PR_WRONLY | PR_CREATE_FILE | PR_TRUNCATE, 0666);
        if (!fd)
            PLERROR("openFile(\"%s\",\"%s\") failed.",filepath.c_str(), openmode.c_str());
        st = new PrPStreamBuf(0, fd, false, true);
    }
    else if (openmode == "a")
    {
        fd = PR_Open(filepath.c_str(), PR_WRONLY | PR_CREATE_FILE | PR_APPEND, 0666);
        if (!fd)
            PLERROR("openFile(\"%s\",\"%s\") failed.",filepath.c_str(), openmode.c_str());
        st = new PrPStreamBuf(0, fd, false, true);
    }
    else
        PLERROR("In openFile, invalid openmode=\"%s\" ",openmode.c_str());    
    
    st.setMode(io_formatting);
    return st;
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
