// -*- C++ -*-

// TestDependenciesCommand.cc
// 
// Copyright (C) 2003 Pascal Vincent
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
   * $Id: TestDependenciesCommand.cc,v 1.1 2004/01/11 03:05:55 yoshua Exp $ 
   ******************************************************* */

/*! \file TestDependenciesCommand.cc */
#include "TestDependenciesCommand.h"
#include "getDataSet.h"
#include "stats_utils.h"

namespace PLearn <%
using namespace std;

//! This allows to register the 'TestDependenciesCommand' command in the command registry
PLearnCommandRegistry TestDependenciesCommand::reg_(new TestDependenciesCommand);

//! The actual implementation of the 'TestDependenciesCommand' command 
void TestDependenciesCommand::run(const vector<string>& args)
{
  if(args.size()<1 || args.size()>3)
    PLERROR("test-dependencies expects 1, 2 or 3 arguments, check the help");

  VMat data = getDataSet(args[0]);
  int inputsize = (args.size()>1)?toint(args[1]):data->inputsize();
  int targetsize = (args.size()>2)?toint(args[2]):data->targetsize();
  if (args.size()>1)
    data->defineSizes(inputsize,targetsize,data->weightsize());

  VMat x = data.subMatColumns(0,inputsize);
  VMat y = data.subMatColumns(inputsize,targetsize);
  int n=data->length();
  if (n*inputsize<50000000) // 400M in double precision
    x = VMat(x.toMat());
  Mat r(x.width(),y.width());
  testSpearmanRankCorrelation(x,y,r);
}

%> // end of namespace PLearn

