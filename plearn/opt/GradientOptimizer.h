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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file GradientOptimizer.h */

#ifndef GRADIENTOPTIMIZER_INC
#define GRADIENTOPTIMIZER_INC

#include "Optimizer.h"

namespace PLearn {
using namespace std;


class GradientOptimizer : public Optimizer
{
    typedef Optimizer inherited;
      
public:

    //!  gradient descent specific parameters
    //!  (directly modifiable by the user)
    real learning_rate; // current learning rate

    // Options (also available through setOption)
    real start_learning_rate;
    real decrease_constant;
    //! Indication that a stochastic hack to accelerate
    //! stochastic gradient descent should be used 
    bool use_stochastic_hack;

    // optionally the user can instead of using the decrease_constant
    // use a fixed schedule. This matrix has 2 columns: iteration_threshold and learning_rate_factor
    // As soon as the iteration number goes above the iteration_threshold, the corresponding learning_rate_factor
    // is applied (multiplied) to the start_learning_rate to obtain the learning_rate.
    Mat lr_schedule;

    int verbosity;
    
    GradientOptimizer();
    /*(
    real the_start_learning_rate=0.01, 
                      real the_decrease_constant=0);
    */

    /*
    GradientOptimizer(VarArray the_params, Var the_cost,
                      real the_start_learning_rate=0.01, 
                      real the_decrease_constant=0,
                      int n_updates=1, const string& filename="", 
                      int every_iterations=1);
    GradientOptimizer(VarArray the_params, Var the_cost, 
                      VarArray update_for_measure,
                      real the_start_learning_rate=0.01, 
                      real the_decrease_constant=0,
                      int n_updates=1, const string& filename="", 
                      int every_iterations=1);
                      */

    PLEARN_DECLARE_OBJECT(GradientOptimizer);

    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies)
    { inherited::makeDeepCopyFromShallowCopy(copies); }

    virtual void build()
    {
        inherited::build();
        build_();
    }
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

DECLARE_OBJECT_PTR(GradientOptimizer);

/* Commented out the whole class: if one uses it, better put it in its own
 * separate file!
class ScaledGradientOptimizer : public Optimizer
{
protected:
    Vec gradient;
    //!  exponential moving average coefficient for short time scale
    real short_time_mac; 
    //!  exponential moving average coefficient for long time scale
    real long_time_mac;
    //!  short time moving average
    Vec short_time_ma;
    //!  long time moving average
    Vec long_time_ma;
    //!  long time moving variance
    Vec long_time_mv;
    //!  long time moving sdev
    Vec long_time_md;

public:
    //!  gradient descent specific parameters
    //!  (directly modifiable by the user)
    real start_learning_rate;
    real decrease_constant;
    real init_learning_rate;
    real learning_rate;
    Vec eps_scale; //!<  scaling parameter for the learning rate of each parameter
  
protected:
    //!  regularizer for update of local learning rates (eps_scale)
    real regularizer;
  
public:
    //////////// JS - faut faire de quoi pour c'te pauv' ptite bebete!
    ScaledGradientOptimizer(VarArray the_params, Var the_cost,
                            real the_start_learning_rate=0.01, 
                            real the_decrease_constant=0.01,
                            real the_init_learning_rate=0.003,
                            int n_updates=1, 
                            real short_time_moving_avg_coef=0.01,
                            real long_time_moving_avg_coef=0.001,
                            real the_regularizer=1.0,
                            const string& filename="", 
                            int every_iterations=1)
        :Optimizer(the_params,the_cost, n_updates, filename, every_iterations),
         gradient(the_params.nelems()),
         short_time_mac(short_time_moving_avg_coef),
         long_time_mac(long_time_moving_avg_coef),
         short_time_ma(the_params.nelems()),
         long_time_ma(the_params.nelems()),
         long_time_mv(the_params.nelems()),
         long_time_md(the_params.nelems()),
         start_learning_rate(the_start_learning_rate),
         decrease_constant(the_decrease_constant),
         init_learning_rate(the_init_learning_rate),
         eps_scale(the_params.nelems()),
         regularizer(the_regularizer) {}

    
    ScaledGradientOptimizer(VarArray the_params, Var the_cost, 
                            real the_start_learning_rate=0.01, 
                            real the_decrease_constant=0.01,
                            int n_updates=1,
                            real short_time_moving_avg_coef=0.01,
                            real long_time_moving_avg_coef=0.001,
                            real the_regularizer=1.0,
                            const string& filename="", 
                            int every_iterations=1)
        :Optimizer(the_params,the_cost,
                   n_updates, filename, every_iterations),
         gradient(the_params.nelems()),
         short_time_mac(short_time_moving_avg_coef),
         long_time_mac(long_time_moving_avg_coef),
         short_time_ma(the_params.nelems()),
         long_time_ma(the_params.nelems()),
         long_time_mv(the_params.nelems()),
         long_time_md(the_params.nelems()),
         start_learning_rate(the_start_learning_rate),
         decrease_constant(the_decrease_constant),
         eps_scale(the_params.nelems()),
         regularizer(the_regularizer)
    {
        eps_scale.fill(1.0);
    }
                    
    virtual real optimize();
};

*/


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
