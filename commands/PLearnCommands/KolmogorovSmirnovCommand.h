// -*- C++ -*-

// KolmogorovSmirnovCommand.h
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
   * $Id: KolmogorovSmirnovCommand.h,v 1.2 2004/02/20 21:11:40 chrish42 Exp $ 
   ******************************************************* */

/*! \file KolmogorovSmirnovCommand.h */
#ifndef KolmogorovSmirnovCommand_INC
#define KolmogorovSmirnovCommand_INC

#include "PLearnCommand.h"
#include "PLearnCommandRegistry.h"

namespace PLearn {
using namespace std;

class KolmogorovSmirnovCommand: public PLearnCommand
{
public:
  KolmogorovSmirnovCommand():
    PLearnCommand("ks-stat",
                  "Computes the Kolmogorov-Smirnov statistic between 2 matrix columns",
                  "ks-stat <matA> <colA> <matB> <colB> [conv] \n"
                  "Will compute the ks-statistic between column colA of matrix matA \n"
                  "and column colB of matrix matB, with presion conv (defaults to 10). \n"
                  "You can use any matrix files recognized by PLearn \n"
                  ) 
  {}
                    
  virtual void run(const vector<string>& args);

protected:
  static PLearnCommandRegistry reg_;
};

  
} // end of namespace PLearn

#endif
