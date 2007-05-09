// -*- C++ -*-

// PythonGlobalInterpreterLock.h
//
// Copyright (C) 2005 Nicolas Chapados 
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


#include <plearn/python/PythonIncludes.h>

//#####  PythonGlobalInterpreterLock  #########################################

/**
 *  @class  PythonGlobalInterpreterLock
 *  @brief  Ensure thread safety by managing the Python Global Interpreter Lock
 *
 *  The Python interpreter is not fully reentrant and must be handled carefully
 *  in the presence of multi-threading.  While this does not affect
 *  multi-threaded pure Python code (for which reentrancy is properly managed),
 *  it does affect extensions that are written in other languages.  To this
 *  end, Python provides a Global Interpreter Lock (GIL) which must be acquired
 *  before calling any of its API within extension code and released
 *  afterwards.
 *
 *  This class provides a simple Resource-Acquisition-is-Initialization (RAII)
 *  idiom to manage the GIL.  The idea is to construct a local variable of this
 *  class at the beginning of a scope which uses Python.  The constructor
 *  acquires the lock, and the destructor automatically releases it.  For
 *  example:
 *
 *  @code
 *  void foo()
 *  {
 *      // Acquire the Python lock.  This blocks if another thread
 *      // already has the lock.
 *      PythonGlobalInterpreterLock gil;
 *
 *      // Code which uses the Python C API comes here
 *      // ...
 *  }   // Destructor releases the lock, so nothing to do.
 *  @endcode
 */
class PythonGlobalInterpreterLock
{
public:
    PythonGlobalInterpreterLock()
        : m_gilstate(PyGILState_Ensure())
    { }
       
    ~PythonGlobalInterpreterLock()
    {
        PyGILState_Release(m_gilstate);
    }

    PyGILState_STATE m_gilstate;
};

