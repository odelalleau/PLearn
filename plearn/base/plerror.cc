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


 

/* *******************************************************      
   * $Id: plerror.cc,v 1.1 2002/07/30 09:01:26 plearner Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */
#include <cstdlib>

#include "plerror.h"
#include "general.h"

#if USING_MPI
#include "PLMPI.h"
#endif //USING_MPI


//extern ofstream debug_stream;

namespace PLearn <%
using namespace std;

#ifdef USE_EXCEPTIONS
// We put the following variables as global to simplify things somewhat.
char error_msg[ERROR_MSG_SIZE];
int error_status = ERROR_STATUS_NO_ERROR;
bool may_continue = false;

void clear_warning()
{ error_status = ERROR_STATUS_NO_ERROR; }

void throw_exception(char *msg, int exception_type)
{
  switch (exception_type) {
  case FATAL_ERROR_EXCEPTION:
  case RECUPERABLE_ERROR_EXCEPTION:
    // The difference between these two cases is that in the first case
    // the caller will call exit() upon return of this function while in the
    // second case it won't.
    // NOTE: The RECUPERABLE_ERROR_EXCEPTION is not yet used. However, when
    //       (if ever) it is, we should be able to detect if the exception has
    //       been handled in some way by the user. If we detect that it has not
    //       been, we should call exit() at some point.
    //       The RECUPERABLE_ERROR_EXCEPTION is meant to deal with situations
    //       where in-place fixes can be made between resuming executions.
    //       Pratically, this implies user-intervention (human-interface related)
    //       to provide some input has to what to do to able the program to
    //       continue.
    error_status = ERROR_STATUS_FATAL_ERROR;
    strncpy(error_msg, msg, ERROR_MSG_SIZE);
    throw PLearnError();
    break;
  case WARNING_EXCEPTION:
    // We don't throw anything since we don't want
    // to interrupt the program, it will be for the
    // user to watch JULIEN KEABLE from time to time the value the 
    // error_status variable and retrieve the error_msg
    error_status = ERROR_STATUS_WARNING;
    strncpy(error_msg, msg, ERROR_MSG_SIZE);
    break;
  case IMMINENT_EXIT_EXCEPTION:
    error_status = ERROR_STATUS_FATAL_ERROR;
    strncpy(error_msg, msg, ERROR_MSG_SIZE);
    throw PLearnError();
    break;

  default:
    strcpy(error_msg, "Unexpected error-handling behavior!, BAD BAD");
    exit(1);
  }
}

#else // !defined(USE_EXCEPTIONS)

ostream* error_stream = &cerr;

#endif // USE_EXCEPTIONS

#ifndef USER_SUPPLIED_ERROR
void  errormsg(const char* msg, ...)
{
  va_list args;
  va_start(args,msg);
  char message[ERROR_MSG_SIZE];

#if !defined(ULTRIX) && !defined(_MINGW_)
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
  throw_exception(message, FATAL_ERROR_EXCEPTION);
#endif
}
#endif

#ifdef USE_EXCEPTIONS
// Why "recuperable" == "friendly" because it implies human
// intervention from the user.
void friendlyerrormsg(const char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  char message[ERROR_MSG_SIZE];

#if !defined(ULTRIX) && !defined(_MINGW_)
  vsnprintf(message, ERROR_MSG_SIZE, msg, args);
#else
  vsprintf(message, msg, args);
#endif

  va_end(args);

  throw_exception(message, RECUPERABLE_ERROR_EXCEPTION);
  if (!may_continue)
    exit(1);
}
#endif // USE_EXCEPTIONS

void  warningmsg(const char* msg, ...)
{
  va_list args;
  va_start(args,msg);
  char message[ERROR_MSG_SIZE];

#if !defined(ULTRIX) && !defined(_MINGW_)
  vsnprintf(message,ERROR_MSG_SIZE,msg,args);
#else
  vsprintf(message,msg,args);
#endif

  va_end(args);

#ifndef USE_EXCEPTIONS
  *error_stream <<" WARNING: "<<message<<endl;
#else
  throw_exception(message, WARNING_EXCEPTION);
#endif
}

void exitmsg(const char* msg, ...)
{
  va_list args;
  va_start(args,msg);
  char message[ERROR_MSG_SIZE];

#if !defined(ULTRIX) && !defined(_MINGW_)
  vsnprintf(message,ERROR_MSG_SIZE,msg,args);
#else
  vsprintf(message,msg,args);
#endif

  va_end(args);

#ifndef USE_EXCEPTIONS
  *error_stream <<message<<endl;
  exit(1);
#else
  throw_exception(message, IMMINENT_EXIT_EXCEPTION);
  // Shouldn't there be an exit() call HERE???
#endif
}


%> // end of namespace PLearn
