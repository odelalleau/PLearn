
// -*- C++ -*-

// StaticInitializer.h
//
// Copyright (C) 2003  Pascal Vincent 
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
   * $Id: StaticInitializer.h,v 1.3 2004/02/26 06:30:25 nova77 Exp $ 
   ******************************************************* */

/*! \file StaticInitializer.h */
#ifndef StaticInitializer_INC
#define StaticInitializer_INC

// norman:
// If I don't add a definition of the namespace std it does not compile!
#ifdef WIN32
#include <string>
#endif

namespace PLearn {
using namespace std;

typedef void (*VOIDFUNC)();

//! A StaticInitializer is typically declared as a static member of a class,
//! and given a parameter that is a static initialization function for said class.
//! This will ensure that said function will be called upon program start.
//! In Objects, this mechanism is defined automatically by the 
//! PLEARN_DECLARE_OBJECT_METHODS macro and used to automatically 
//! call TypeFactory::register_type

class StaticInitializer
{
public:
  // Simply calls the given initialize function upon construction
  StaticInitializer(VOIDFUNC initialize);
};


} // end of namespace PLearn

#endif
