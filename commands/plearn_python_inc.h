// -*- C++ -*-

// plearn_python_inc.h
//
// Copyright (C) 2006 Pascal Vincent
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
 * $Id: plearn_python_inc.h 6346 2006-10-24 17:02:02Z lamblin $ 
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file plearn_python_inc.h */

/*! Include here all classes available in the PLearn repository 
  that imply linking with the python runtime.
*/

#ifndef plearn_python_inc_INC
#define plearn_python_inc_INC

// Do not include if PL_PYTHON_VERSION is undefined (compiling with -nopython)
#ifdef PL_PYTHON_VERSION

/******************************************************
 * Python includes must come FIRST, as per Python doc *
 ******************************************************/
#include <plearn/python/PythonIncludes.h>

#include <plearn/python/PythonCodeSnippet.h>
#include <plearn/python/PythonProcessedVMatrix.h>
#include <plearn/vmat/DictionaryVMatrix.h>
#include <commands/PLearnCommands/VMatDictionaryCommand.h>

#endif // PL_PYTHON_VERSION

#endif // plearn_python_inc_INC


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