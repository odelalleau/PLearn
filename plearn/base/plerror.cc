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

// Authors: Pascal Vincent & Yoshua Bengio

#include <cstdarg>
//#include <cstdlib>
#include <iostream>

#include "plerror.h"
#include <plearn/io/pl_log.h>

#if USING_MPI
#include <plearn/sys/PLMPI.h>

#endif //USING_MPI


//extern ofstream debug_stream;

namespace PLearn {
using namespace std;

ostream* error_stream = &cerr;

#define ERROR_MSG_SIZE 1024

#ifndef USER_SUPPLIED_ERROR
void errormsg(const char* msg, ...)
{
    va_list args;
    va_start(args,msg);
    char message[ERROR_MSG_SIZE];

#if !defined(ULTRIX) && !defined(_MINGW_) && !defined(WIN32)
    vsnprintf(message, ERROR_MSG_SIZE,msg,args);
#else
    vsprintf(message,msg,args);
#endif

    va_end(args);

#ifndef USE_EXCEPTIONS
#if USING_MPI
    *error_stream <<" ERROR from rank=" << PLMPI::rank << ": " <<message<<endl;
#else //USING_MPI
    *error_stream <<" ERROR: "<<message<<endl;
#endif //USING_MPI
    exit(1);
#else
// Commented out as one error message seems to be enough.
//  IMP_LOG << "Throwing PLearnError exception: " << message << endl;
    throw PLearnError(message);                
#endif
}
#endif


void  warningmsg(const char* msg, ...)
{
    va_list args;
    va_start(args,msg);
    char message[ERROR_MSG_SIZE];

#if !defined(ULTRIX) && !defined(_MINGW_) && !defined(WIN32)
    vsnprintf(message,ERROR_MSG_SIZE,msg,args);
#else
    vsprintf(message,msg,args);
#endif

    va_end(args);

    // *error_stream <<" WARNING: "<<message<<endl;
    NORMAL_LOG << " WARNING: " << message << endl;
}

void  deprecationmsg(const char* msg, ...)
{
    va_list args;
    va_start(args,msg);
    char message[ERROR_MSG_SIZE];

#if !defined(ULTRIX) && !defined(_MINGW_) && !defined(WIN32)
    vsnprintf(message,ERROR_MSG_SIZE,msg,args);
#else
    vsprintf(message,msg,args);
#endif

    va_end(args);

    // *error_stream <<" DEPRECATION_WARNING: "<<message<<endl;
    NORMAL_LOG << " DEPRECATION_WARNING: " << message << endl;
}

void exitmsg(const char* msg, ...)
{
    va_list args;
    va_start(args,msg);
    char message[ERROR_MSG_SIZE];

#if !defined(ULTRIX) && !defined(_MINGW_) && !defined(WIN32)
    vsnprintf(message,ERROR_MSG_SIZE,msg,args);
#else
    vsprintf(message,msg,args);
#endif

    va_end(args);

    *error_stream << message << endl;
    exit(1);
}


void pl_assert_fail(const char* expr, const char* file, unsigned line,
                    const char* function, const char* message)
{
    PLERROR("Assertion failed: %s\n"
            "Function: %s\n"
            "    File: %s\n"
            "    Line: %d"
            "%s%s",
            expr, function, file, line,
            (message && *message? "\n Message: " : ""),
            message);
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
