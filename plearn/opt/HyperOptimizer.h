// -*- C++ -*-4 2002/09/08 21:59:21 morinf

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
   * $Id: HyperOptimizer.h,v 1.1 2002/09/09 14:33:01 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearn/plearn/opt/HyperOptimizer.h */

#ifndef HyperOptimizer_INC
#define HyperOptimizer_INC

#include "TestMethod.h"


namespace PLearn <%
using namespace std;

// ###### HyperOptimizer ######################################################
//
// This is the generic definition of an HyperOptimizer.
// NOTE: This is _BETA_ stuff, things may change...

class HyperOptimizer;
typedef Array<PP<HyperOptimizer> > HStrategy;

class HyperOptimizer: public Object
{
    typedef Object inherited;
 public:
    HyperOptimizer()
        {};

    //! Evaluate learner's performance through the given objective
    real evaluate(PP<Learner> learner, const VMat dataset) const
        { return 0; }; //objective->test(learner, dataset); };

    //! Optimize the learner with respect to the given 
    //! hyperparameters and dataset.
    virtual void optimize(PP<Learner> learner, const VMat dataset) = 0;

    PP<TestMethod> objective;
    HStrategy substrategy;

    DECLARE_ABSTRACT_NAME_AND_DEEPCOPY(HyperOptimizer);
    static void declareOptions(OptionList &ol);
 protected:
 private:
}; // class HyperOptimizer

DECLARE_OBJECT_PTR(HyperOptimizer);
%>; // end of namespace PLearn

#endif // HyperOptimizer_INC
