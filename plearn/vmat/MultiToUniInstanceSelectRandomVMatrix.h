// -*- C++ -*-

// MultiToUniInstanceSelectRandomVMatrix.h
//
// Copyright (C) 2005 Benoit Cromp
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

// Authors: Benoit Cromp

/*! \file MultiToUniInstanceSelectRandomVMatrix.h */


#ifndef MultiToUniInstanceSelectRandomVMatrix_INC
#define MultiToUniInstanceSelectRandomVMatrix_INC

#include <plearn/vmat/SelectRowsVMatrix.h>

namespace PLearn {
using namespace std;

//! selects randomly one row per bags from a multi instances conforming VMatrix
//! and discard the multi instances bag information column.
class MultiToUniInstanceSelectRandomVMatrix: public SelectRowsVMatrix
{

private:

    typedef SelectRowsVMatrix inherited;

public:

    VMat source_;

    // ************************
    // * Public build options *
    // ************************

    //! Random number generator seed
    long seed;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor
    MultiToUniInstanceSelectRandomVMatrix();

    PLEARN_DECLARE_OBJECT(MultiToUniInstanceSelectRandomVMatrix);

    static void declareOptions(OptionList &ol);
    virtual void build();

protected:

    // ******************
    // * Object methods *
    // ******************
public:

private:

    void build_();
};

DECLARE_OBJECT_PTR(MultiToUniInstanceSelectRandomVMatrix);

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
