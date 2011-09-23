// -*- C++ -*-

// PythonIncludes.h
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

/* *******************************************************      
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/**
 *  @file  PythonIncludes.h
 *
 *  From the desired python version defined in the compile-time define
 *  PL_PYTHON_VERSION, include a few useful python modules.  The Python Version
 *  numbers are interpreted as follows:
 *
 *  - 230 = Python 2.3.x
 *  - 240 = Python 2.4.x
 *  - 250 = Python 2.5.x
 *
 *  NOTE: This header file MUST BE INCLUDED FIRST, even before any standard
 *  library includes are carried out.  (Python.h plays hacks with some macros)
 */

#ifdef PL_PYTHON_VERSION

// Under Windows with Microsoft Visual Studio in debug mode, _DEBUG is defined
// resulting in a dependence on the python24_d.lib library file. To get rid of
// this, we undefine _DEBUG before including the python headers.
#if defined(WIN32) && defined(_DEBUG)
#define NEED_REDEFINE_DEBUG
#undef _DEBUG
#endif

/* allow to be undefined
#ifndef PL_PYTHON_VERSION
#  define PL_PYTHON_VERSION 230
#endif
*/

#if defined(PL_USE_NUMARRAY) && defined(PL_USE_NUMPY)
#  error "Symbols PL_USE_NUMARRAY and PL_USE_NUMPY are mutually exclusive; they should not both be defined"
#endif

#if PL_PYTHON_VERSION >= 270

#include <python2.7/Python.h>
#include <python2.7/compile.h>  // define PyCodeObject
#include <python2.7/eval.h>     // for accessing PyEval_EvalCode: not included by default
#ifdef PL_USE_NUMARRAY
#  include <python2.7/numarray/libnumarray.h>
#else
#  ifdef PL_USE_NUMPY
#    pragma GCC system_header //suppress all warnings/errors for numpy
#    include <libnumarray.h>
#  else
#    error "should use either NumPy (preferred) or NUMARRAY (deprecated)"
#  endif //def PL_USE_NUMPY
#endif //def PL_USE_NUMARRAY

#elif PL_PYTHON_VERSION >= 260

#include <python2.6/Python.h>
#include <python2.6/compile.h>  // define PyCodeObject
#include <python2.6/eval.h>     // for accessing PyEval_EvalCode: not included by default
#ifdef PL_USE_NUMARRAY
#  include <python2.6/numarray/libnumarray.h>
#else
#  ifdef PL_USE_NUMPY
#    pragma GCC system_header //suppress all warnings/errors for numpy
#    include <libnumarray.h>
#  else
#    error "should use either NumPy (preferred) or NUMARRAY (deprecated)"
#  endif //def PL_USE_NUMPY
#endif //def PL_USE_NUMARRAY

#elif PL_PYTHON_VERSION >= 250

#include <python2.5/Python.h>
#include <python2.5/compile.h>  // define PyCodeObject
#include <python2.5/eval.h>     // for accessing PyEval_EvalCode: not included by default
#ifdef PL_USE_NUMARRAY
#  include <python2.5/numarray/libnumarray.h>
#else
#  ifdef PL_USE_NUMPY
#    pragma GCC system_header //suppress all warnings/errors for numpy
#    include <libnumarray.h>
#  else
#    error "should use either NumPy (preferred) or NUMARRAY (deprecated)"
#  endif //def PL_USE_NUMPY
#endif //def PL_USE_NUMARRAY

#elif PL_PYTHON_VERSION >= 240

#include <python2.4/Python.h>
#include <python2.4/compile.h>  // define PyCodeObject
#include <python2.4/eval.h>     // for accessing PyEval_EvalCode: not included by default
#ifdef PL_USE_NUMARRAY
#  include <python2.4/numarray/libnumarray.h>
#else
#  ifdef PL_USE_NUMPY
#    pragma GCC system_header //suppress all warnings/errors for numpy
#    include <libnumarray.h>
#  else
#    error "should use either NumPy (preferred) or NUMARRAY (deprecated)"
#  endif //def PL_USE_NUMPY
#endif //def PL_USE_NUMARRAY

#elif PL_PYTHON_VERSION >= 230

#include <python2.3/Python.h>
#include <python2.3/compile.h>  // define PyCodeObject
#include <python2.3/eval.h>     // for accessing PyEval_EvalCode: not included by default
#include <python2.3/numarray/libnumarray.h>

#else
/*
// it is not an error not to include python
#  error "PL_PYTHON_VERSION should be defined to one of: 230, 240, 250"
*/
#endif

// Redefine _DEBUG if needed (see above).
#ifdef NEED_REDEFINE_DEBUG
#define _DEBUG
#undef NEED_REDEFINE_DEBUG
#endif

// Undefine HAVE_LONG_LONG so we do not get an annoying warning when using
// PythonIncludes.h together with NSPR or other libraries that also define
// this. Works only because PythonIncludes.h *must* be the very first include,
// so we know that if there is an HAVE_LONG_LONG defined, we are the one that
// defined it.
//
#undef HAVE_LONG_LONG

#endif //def PL_PYTHON_VERSION


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
