// -*- C++ -*-

// KolmogorovSmirnovCommand.cc
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
   * $Id: KolmogorovSmirnovCommand.cc,v 1.2 2004/02/20 21:11:40 chrish42 Exp $ 
   ******************************************************* */

/*! \file KolmogorovSmirnovCommand.cc */
#include "KolmogorovSmirnovCommand.h"
#include "getDataSet.h"
#include "stats_utils.h"

namespace PLearn {
using namespace std;

//! This allows to register the 'KolmogorovSmirnovCommand' command in the command registry
PLearnCommandRegistry KolmogorovSmirnovCommand::reg_(new KolmogorovSmirnovCommand);

//! The actual implementation of the 'KolmogorovSmirnovCommand' command 
void KolmogorovSmirnovCommand::run(const vector<string>& args)
{
  if(args.size()!=4 && args.size()!=5)
    PLERROR("ks-stat expects 4 or 5 arguments, check the help");

  VMat m1 = getDataSet(args[0]);
  int c1 = toint(args[1]);
  VMat m2 = getDataSet(args[2]);
  int c2 = toint(args[3]);
  int conv = 10;
  if(args.size()==5)
     conv = toint(args[4]);

  Vec v1 = m1.getColumn(c1);
  Vec v2 = m2.getColumn(c2);

  real D, ks_stat;
  KS_test(v1, v2, conv, D, ks_stat);

  cout << "Maximum absolute difference between the 2 cdfs is: " << D << endl;
  cout << "Result of Kolmogorov-Smirnov test is: " << ks_stat << endl;
}

} // end of namespace PLearn

