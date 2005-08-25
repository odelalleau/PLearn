// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2003 Pascal Vincent, Yoshua Bengio,
//                         Olivier Delalleau and University of Montreal
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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearn/plearn/opt/AdaptGradientOptimizer.h */

#ifndef AdaptGradientOptimizer_INC
#define AdaptGradientOptimizer_INC

#include "Optimizer.h"

namespace PLearn {
using namespace std;


/*!
 * CLASS ADAPTGRADIENTOPTIMIZER
 *
 * A (possibly stochastic) gradient optimizer using various learning rate
 * adaptation methods.
 *
 */
class AdaptGradientOptimizer : public Optimizer
{
    typedef Optimizer inherited;
      
public:

    //!  gradient descent specific parameters
    //!  (directly modifiable by the user)
    real learning_rate; // current learning rate

    // Options (also available through setOption)
    real start_learning_rate; //!< initial learning rate
    real min_learning_rate;  //!< min value for learning_rate when adapting
    real max_learning_rate;  //!< max value for learning_rate when adapting
    //! Learning rate adaptation kind :
    //! 0  : none
    //! 1  : basic
    //! 2  : ALAP1
    //! 3  : variance
    int learning_rate_adaptation;
    real adapt_coeff1;  //!< a coefficient for learning rate adaptation
    real adapt_coeff2;  //!< a coefficient for learning rate adaptation
    real decrease_constant;
    int mini_batch;
    int adapt_every;    //!< after how many updates we adapt learning rate

private:

    bool stochastic_hack; // true when we're computing a stochastic gradient
    Vec learning_rates;   // used to store the individual learning rates
    Vec gradient;         // used to store the gradient
    Vec tmp_storage;      // used to store various stuff
    // used to store the previous weights evolution, it can be used to
    // see how many times a weight has increased / decreased consecutively
    Vec old_evol;
    Array<Mat> oldgradientlocations; // used for the stochastic hack
    Vec store_var_grad;     // used to store the gradient variance
    Vec store_grad;         // used to store the gradient
    Vec store_quad_grad;    // used to store the gradient^2
    int count_updates;      // used to count how many examples we went through

public: 

    AdaptGradientOptimizer(real the_start_learning_rate=0.01, 
                           real the_decrease_constant=0,
                           real the_min_learning_rate=0.001,
                           real the_max_learning_rate=0.02,
                           int the_learning_rate_adaptation=0,
                           real the_adapt_coeff1=0,
                           real the_adapt_coeff2=0,
                           int n_updates=1, const string& filename="", 
                           int every_iterations=1);
    AdaptGradientOptimizer(VarArray the_params, Var the_cost,
                           real the_start_learning_rate=0.01, 
                           real the_decrease_constant=0,
                           real the_min_learning_rate=0.001,
                           real the_max_learning_rate=0.02,
                           int the_learning_rate_adaptation=0,
                           real the_adapt_coeff1=0,
                           real the_adapt_coeff2=0,
                           int n_updates=1, const string& filename="", 
                           int every_iterations=1);
    AdaptGradientOptimizer(VarArray the_params, Var the_cost, 
                           VarArray update_for_measure,
                           real the_start_learning_rate=0.01, 
                           real the_decrease_constant=0,
                           real the_min_learning_rate=0.001,
                           real the_max_learning_rate=0.02,
                           int the_learning_rate_adaptation=0,
                           real the_adapt_coeff1=0,
                           real the_adapt_coeff2=0,
                           int n_updates=1, const string& filename="", 
                           int every_iterations=1);

      
    PLEARN_DECLARE_OBJECT(AdaptGradientOptimizer);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies) { inherited::makeDeepCopyFromShallowCopy(copies); }

    virtual void build()
    {
        inherited::build();
        build_();
    }

private:

    void build_();
    
public:

    virtual real optimize();
    virtual bool optimizeN(VecStatsCollector& stats_coll);

private:

    // Basic learning rate adaptation
    // If grad(i) > 0 : lr(i) = lr(i) + lr(i) * adapt_coeff1
    // else           : lr(i) = lr(i) - lr(i) * adapt_coeff2
    void adaptLearningRateBasic(
        Vec old_params,
        Vec new_evol);

    // ALAP1 formula learning rate adaptation
    // lr = lr + adapt_coeff1 * dot(grad(k-1), grad(k))
    // NB: has not been tested
    void adaptLearningRateALAP1(
        Vec old_gradient,
        Vec new_gradient);

    // Learning rate adaptation depending on the variance :
    // If var(i) is low, lr(i) = max_learning_rate
    // else              lr(i) = min_learning_rate
    void adaptLearningRateVariance();

protected:

    static void declareOptions(OptionList& ol);

};

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
