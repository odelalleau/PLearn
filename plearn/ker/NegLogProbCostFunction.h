// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
   * $Id: NegLogProbCostFunction.h,v 1.4 2004/04/07 23:15:17 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef NegLogProbCostFunction_INC
#define NegLogProbCostFunction_INC

#include "Kernel.h"

namespace PLearn {
using namespace std;



/*!   The target is an integer between 0 and n_classes-1 corresponding
  to a class Y, and the output is a vector of length n_classes 
  of non-negative estimated
  conditional probabilities P(Y=i|X) for class i (i=0 to n_classes-1),
  or if n_classes=2, the output could be a vector of length 1
  containing P(Y=1|X). The cost is
     - log P(Y=target|X).
*/

class NegLogProbCostFunction: public Kernel
{
    typedef Kernel inherited;
		
public:
    bool normalize;
    bool smooth_map_outputs;
#if USING_MPI
    int out_start;
    int out_end;
#endif
public:
#if USING_MPI
    NegLogProbCostFunction(bool do_normalize=false, bool do_smooth_map_outputs=false,
                           int outstart=-1, int outend=-1)
        :normalize(do_normalize), smooth_map_outputs(do_smooth_map_outputs),
         out_start(outstart), out_end(outend) {}
#else
    NegLogProbCostFunction(bool do_normalize=false, bool do_smooth_map_outputs=false)
        : normalize(do_normalize), smooth_map_outputs(do_smooth_map_outputs) {}
#endif
    PLEARN_DECLARE_OBJECT(NegLogProbCostFunction);

    virtual string info() const
        { return "negative_log_probability"; }

    virtual real evaluate(const Vec& output, const Vec& target) const;

protected:
    //!  recognized options are "normalize" and "smooth_map_outputs"  
    static void declareOptions(OptionList &ol);
};

DECLARE_OBJECT_PTR(NegLogProbCostFunction);

//!  negative log conditional probability 
#if USING_MPI
inline CostFunc condprob_cost(bool normalize=false, bool smooth_map_outputs=false,
                              int outstart=-1, int outend=-1)
{ return new NegLogProbCostFunction(normalize,smooth_map_outputs,outstart,outend); }
#else
inline CostFunc condprob_cost(bool normalize=false, bool smooth_map_outputs=false)
{ return new NegLogProbCostFunction(normalize,smooth_map_outputs); }
#endif

} // end of namespace PLearn

#endif

