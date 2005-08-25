// -*- C++ -*-

// load_and_save.h
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

// Authors: Pascal Vincent

/*! \file load_and_save.h */


#ifndef load_and_save_INC
#define load_and_save_INC

// Put includes here

#include "PStream.h"
#include "fileutils.h" //!< For force_mkdir_for_file();
#include "openFile.h"

namespace PLearn {
using namespace std;

// ****************************************
// *** Generic save and load operations ***
// ****************************************

template <class T> 
inline void load(const PPath& filepath, T &x)
{
#if STREAMBUFVER == 1
    PStream in = openFile( filepath, PStream::plearn_ascii, "r" );
    in >> x;
#else
    ifstream in_( filepath.absolute().c_str() );
    if (!in_)
        PLERROR("Could not open file \"%s\" for reading", filepath.c_str());
    PStream in(&in_);
    in >> x;
#endif
}

//! If necessary, missing directories along the filepath will be created
template<class T> 
inline void save(const PPath& filepath, const T& x, PStream::mode_t io_formatting=PStream::plearn_ascii)
{ 
    force_mkdir_for_file(filepath);
#if STREAMBUF == 1
    PStream out = openFile( filepath, io_formatting, "w" );
    out << x;
#else
    ofstream out_( filepath.absolute().c_str() ); 
    if(!out_)
        PLERROR( "Could not open file %s for writing", filepath.c_str() );  
    PStream out(&out_);
    out.setMode(io_formatting);
    out << x;
#endif
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
