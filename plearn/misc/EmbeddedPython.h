// -*- C++ -*-

// EmbeddedPython.h
//
// Copyright (C) 2004 Nicolas Chapados 
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
   * $Id: EmbeddedPython.h,v 1.1 2004/11/30 09:38:21 chapados Exp $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file EmbeddedPython.h */


#ifndef EmbeddedPython_INC
#define EmbeddedPython_INC

// Put includes here

namespace PLearn {

/**
 *  Include this file when you want to embed the Python interpreter.  It
 *  ensures that Py_Initialize and Py_Finalize are called only once in the
 *  entire program.  NOTE that it does not include the Python.h header file
 *  for you; you must still include those files yourself.
 *
 *  Create one such object that is called early enough in your Python-using
 *  code (e.g. a build function before any Python calls are made) ::
 *
 *    static PythonEmbedder python;
 */

class PythonEmbedder
{
public:
  PythonEmbedder();
  ~PythonEmbedder();
};


} // end of namespace PLearn

#endif
