// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Pascal Vincent, Frederic Morin

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
   * $Id: HyperOptimizer.cc,v 1.1 2002/09/09 14:33:01 morinf Exp $
   * AUTHORS: Pascal Vincent & Frederic Morin
   * This file is part of the PLearn library.
   ******************************************************* */

#include "HyperOptimizer.h"

namespace PLearn <%

// ###### HyperOptimizer ######################################################

IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY(HyperOptimizer);

void
HyperOptimizer::declareOptions(OptionList &ol)
{
    declareOption(ol, "objective", &HyperOptimizer::objective, OptionBase::buildoption,
                  "Objective function to be optimized");
    declareOption(ol, "substrategy", &HyperOptimizer::substrategy, OptionBase::buildoption,
                  "Embedded strategies performed during optimization");
    inherited::declareOptions(ol);
}

%>; // end of namespace PLearn
