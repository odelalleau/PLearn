// -*- C++ -*-

// plearn_tests_inc.h
//
// Copyright (C) 2005 Olivier Delalleau 
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
 * $Id: plearn_tests_inc.h 4275 2005-10-20 00:34:14Z tihocan $ 
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file plearn_inc.h */

/*! Include here all PLearn test classes */

#ifndef plearn_tests_inc_INC
#define plearn_tests_inc_INC

// Convenient vim macro to comment all tests (useful to debug a given test):
//      %s/\(\#include\(.\)\+Test.h>\)/\/\/\1/
// And another vim macro to restore all tests:
//      %s/\/\/\#include/#include/

/*********
 * PTest *
 *********/
#include <plearn/io/test/PLLogTest.h>
#include <plearn/io/test/PPathTest.h>
#include <plearn/io/test/PStreamBufTest.h>
#include <plearn/math/test/TMat/TMatTest.h>

// Some other minimal includes to be able to run tests.

/***********
 * Command *
 ***********/
#include <commands/PLearnCommands/DiffCommand.h>
#include <commands/PLearnCommands/ReadAndWriteCommand.h>
#include <commands/PLearnCommands/RunCommand.h>

/**********
 * Object *
 **********/
#include <plearn/misc/RunObject.h>

#endif

