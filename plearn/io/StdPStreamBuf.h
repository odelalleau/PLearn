// -*- C++ -*-

// StdPStreamBuf.h
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

/*! \file StdPStreamBuf.h */

//! StdPStreamBuf is is a PStreamBuf that wraps an underlying std::istream and std::ostream

#ifndef StdPStreamBuf_INC
#define StdPStreamBuf_INC

#include "PStreamBuf.h"
#include <iostream>

namespace PLearn {
using namespace std;

class StdPStreamBuf: public PStreamBuf
{

private:
  
    typedef PStreamBuf inherited;

protected:
    // *********************
    // * protected options *
    // *********************
    istream* pin;  //!< underlying input stream
    ostream* pout; //!< underlying output stream
    bool own_pin, own_pout; //!< true if {pin|pout} was created internally

public:

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    StdPStreamBuf();

    //! ctor. from an istream (I)
    StdPStreamBuf(istream* pin_, bool own_pin_=false);

    //! ctor. from an ostream (O)
    StdPStreamBuf(ostream* pout_, bool own_pout_=false);

    //! ctor. from an iostream (IO)
    StdPStreamBuf(iostream* pios_, bool own_pios_=false);

    //! ctor. from an istream and an ostream (IO)
    StdPStreamBuf(istream* pin_, ostream* pout_, bool own_pin_=false, bool own_pout_=false);

    virtual ~StdPStreamBuf();


    void setIn(istream* pin_, bool own_pin_=false);

    void setOut(ostream* pout_, bool own_pout_=false);


    // ******************
    // * Object methods *
    // ******************

protected: 

    virtual streamsize read_(char* p, streamsize n);

    //! writes exactly n characters from p (unbuffered, must flush)
    virtual void write_(const char* p, streamsize n);

public:

    inline istream* rawin() { return pin; }   //!< access to underlying istream
    inline ostream* rawout() { return pout; }

    virtual bool good() const;
};

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
