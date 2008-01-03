// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent and Yoshua Bengio
// Copyright (C) 1999-2002, 2006 University of Montreal
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
 * $Id: AutoScaledGradientOptimizer.h 8247 2007-11-12 20:22:12Z nouiz $
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file AutoScaledGradientOptimizer.h */

#ifndef AutoScaledGradientOptimizer_INC
#define AutoScaledGradientOptimizer_INC

#include <plearn/opt/Optimizer.h>

namespace PLearn {
using namespace std;


class AutoScaledGradientOptimizer : public Optimizer
{
    typedef Optimizer inherited;
      
public:

    //!  gradient descent specific parameters
    //!  (directly modifiable by the user)
    real learning_rate; // current learning rate

    // Options (also available through setOption)
    real start_learning_rate;
    real decrease_constant;

    // optionally the user can instead of using the decrease_constant
    // use a fixed schedule. This matrix has 2 columns: iteration_threshold and learning_rate_factor
    // As soon as the iteration number goes above the iteration_threshold, the corresponding learning_rate_factor
    // is applied (multiplied) to the start_learning_rate to obtain the learning_rate.
    Mat lr_schedule;

    int verbosity;

    // every how-many steps should the mean and scaling be reevaluated
    int evaluate_scaling_every;
    // how many steps should be used to re-evaluate the mean and scaling
    int evaluate_scaling_during; 
    // scaling will be 1/(mean_abs_grad + epsilon)
    real epsilon;

    AutoScaledGradientOptimizer();

    PLEARN_DECLARE_OBJECT(AutoScaledGradientOptimizer);

    virtual void setToOptimize(const VarArray& the_params, Var the_cost, VarArray the_other_costs = VarArray(0), TVec<VarArray> the_other_params = TVec<VarArray>(0), real the_other_weight = 1);

    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies)
    { inherited::makeDeepCopyFromShallowCopy(copies); }

    virtual void build()
    {
        inherited::build();
        build_();
    }

protected:
    Vec scaling; // by how much to multiply the gradient before performing an update
    Vec meanabsgrad; // the mean absolute value of the gradient computed for 
    int nsteps_remaining_for_evaluation;

    // Vecs pointing to the value and graident of parameters (setup with the makeSharedValue and makeShared Gradient hack)
    Vec param_values;
    Vec param_gradients;

private:
    void build_()
    {}
    
public:

    // virtual void oldwrite(ostream& out) const;
    // virtual void oldread(istream& in);
    //virtual real optimize();
    virtual bool optimizeN(VecStatsCollector& stats_coll);

protected:

    static void declareOptions(OptionList& ol);
};

DECLARE_OBJECT_PTR(AutoScaledGradientOptimizer);

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
