// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: plexceptions.h,v 1.1 2002/11/22 16:59:37 ducharme Exp $
   * AUTHOR: Frederic Morin
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/plexceptions.h */

#ifndef pexceptions_INC
#define pexceptions_INC

#define ERROR_MSG_SIZE               2048

#ifdef USE_EXCEPTIONS

#include <string>
#include "plerror.h"

namespace PLearn <%
using namespace std;

// externally declared variable, please use PLearnError()'s interface
// instead of using them directly. Direct access can be justified but
// not in YOUR case.
extern char error_msg[ERROR_MSG_SIZE];
extern int error_status;
extern bool may_continue;
extern char *error_file;
extern int error_line;

void clear_warning();	//!<  Reset error_status to ERROR_STATUS_NO_ERROR
void throw_exception(char *msg, int exception_type); //!<  Throw an exception


//!  Error status
#define ERROR_STATUS_NO_ERROR           0
#define ERROR_STATUS_WARNING            1
#define ERROR_STATUS_FATAL_ERROR        100

//!  Exception types (these are bit testable)
#define FATAL_ERROR_EXCEPTION           (1)
#define WARNING_EXCEPTION               (2)
#define IMMINENT_EXIT_EXCEPTION         (4)
#define RECUPERABLE_ERROR_EXCEPTION     (8)
#define DEPRECATED_EXCEPTION            (16)


class PLearnError {
public:
    PLearnError() {};
    virtual ~PLearnError() {};

    virtual string message() const { return string(error_msg); }; // Return error message
    virtual int status() const { return error_status; };          // Return status of error
    virtual string file() const { return string(error_file); };   // Return filename in which error occured
    virtual int line() const { return error_line; };              // Return line number of error (in file)

    bool isRecuperable() const { return (error_status & RECUPERABLE_ERROR_EXCEPTION); };
    bool clear() { error_status = ERROR_STATUS_NO_ERROR; return true; };

    // Use only with RECUPERABLE errors
    void dealtWith() { clear_warning(); may_continue = true; };
};

%> // end of namespace PLearn
//!<  USENAMESPACE

#endif //!<  USE_EXCEPTIONS

#endif //!<  pexceptions_INC
