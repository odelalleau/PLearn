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
 * $Id: NegLogPoissonVariable.h 3994 2005-08-25 13:35:03Z chapados $
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef NegLogPoissonVariable_INC
#define NegLogPoissonVariable_INC

#include "BinaryVariable.h"
#include <plearn/math/pl_erf.h>

namespace PLearn {
using namespace std;

 
//! cost =  sum_i { exp(output_i) - output_i * target_i + log(target_i!) }
//
// We can link the notation used above to the usual
// statistical notation in the following way:
// lambda = exp(output_i) 
// where exponentiation ensures a positive value for lambda
// and k = target_i, the number of observed events
// Thus, the cost corresponds to the negative log likelihood
// of the Poisson density

class NegLogPoissonVariable: public BinaryVariable
{
    typedef BinaryVariable inherited;

public:
    //!  Default constructor for persistence
    NegLogPoissonVariable() {}
    NegLogPoissonVariable(Variable* netout, Variable* target);
    NegLogPoissonVariable(Variable* netout, Variable* target, bool scaled_targets_);
    
    PLEARN_DECLARE_OBJECT(NegLogPoissonVariable);

    virtual void build();

    virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();

    // This flag is set to *true* in cases of weighted
    // datasets where the target represents the proportion of
    // events among a certain number of observations. The number
    // of observations is assumed to be fed as the weight.
    // Default value is *false*.
    bool scaled_targets;
    
protected:
    void build_();
};

DECLARE_OBJECT_PTR(NegLogPoissonVariable);

inline Var neglogpoissonvariable(Var network_output, Var targets, bool scaled_targets_=false)
{ return new NegLogPoissonVariable(network_output, targets, scaled_targets_); }

} // end of namespace PLearn

#endif 


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
