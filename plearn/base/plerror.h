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
   * $Id: plerror.h,v 1.2 2002/08/07 16:54:21 morinf Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/plerror.h */

#ifndef perror_INC
#define perror_INC

#include <cstdio>
#include <cstdarg>
#include <iostream>
#include "plexceptions.h"

namespace PLearn <%
using namespace std;

#ifndef USE_EXCEPTIONS
extern ostream* error_stream;

inline void send_file_line(char* file,int line) 
{ *error_stream<<"At "<<file<<":"<<line; }

#define PLERROR send_file_line(__FILE__,__LINE__),errormsg
//#define PLFRIENDLYERROR - Doesn't make sense to define this if exceptions
//                          are not used!
#define PLWARNING send_file_line(__FILE__,__LINE__),warningmsg
#define PLDEPRECATED send_file_line(__FILE__, __LINE__), deprecatedmsg

#else

void send_file_line(char *file, int line);

#define PLERROR         send_file_line(__FILE__, __LINE__), errormsg
#define PLFRIENDLYERROR send_file_line(__FILE__, __LINE__), friendlyerrormsg
#define PLWARNING       send_file_line(__FILE__, __LINE__), warningmsg
#define PLDEPRECATED    send_file_line(__FILE__, __LINE__), deprecatedmsg

#endif //!<  USE_EXCEPTIONS

void errormsg(const char* msg, ...);
void warningmsg(const char* msg, ...);
void exitmsg(const char* msg, ...);
void deprecatedmsg();

%> // end of namespace PLearn

#endif
