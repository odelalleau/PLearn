// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: GradientOptimizer.h,v 1.8 2003/04/29 19:44:49 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/GradientOptimizer.h */

#ifndef GRADIENTOPTIMIZER_INC
#define GRADIENTOPTIMIZER_INC

#include "Optimizer.h"
#include "SumOfVariable.h"

namespace PLearn <%
using namespace std;


  class GradientOptimizer : public Optimizer
  {
      typedef Optimizer inherited;
      
//    protected:
      //!  Default constructor for persistence
      //GradientOptimizer () : start_learning_rate(), decrease_constant() {}
  
    public:
      //!  gradient descent specific parameters
      //!  (directly modifiable by the user)
      real learning_rate; // current learning rate

      // Options (also available through setOption)
      real start_learning_rate;
      real min_learning_rate;  //!< min value for learning_rate when adapting
      real max_learning_rate;  //!< max value for learning_rate when adapting
      //! Learning rate adaptation kind :
      //! 0  = none
      //! 1  = basic
      //! 2  = ALAP1
      int learning_rate_adaptation;
      real adapt_coeff1;  //!< a coefficient for learning rate adaptation
      real adapt_coeff2;  //!< a coefficient for learning rate adaptation
      real decrease_constant;

    private:
      bool stochastic_hack; // true when we're computing a stochastic gradient
      Vec learning_rates;   // used to store the individual learning rates
      Vec gradient;         // used to store the gradient
      Vec tmp_storage;      // used to store various stuff
      // used to store the previous learning rates evolution
      Vec old_evol;
      Array<Mat> oldgradientlocations; // used for the stochastic hack

    public: 

      GradientOptimizer(real the_start_learning_rate=0.01, 
                        real the_decrease_constant=0,
                        real the_min_learning_rate=0.001,
                        real the_max_learning_rate=0.02,
                        int the_learning_rate_adaptation=0,
                        real the_adapt_coeff1=0,
                        real the_adapt_coeff2=0,
                        int n_updates=1, const string& filename="", 
                        int every_iterations=1);
      GradientOptimizer(VarArray the_params, Var the_cost,
                        real the_start_learning_rate=0.01, 
                        real the_decrease_constant=0,
                        real the_min_learning_rate=0.001,
                        real the_max_learning_rate=0.02,
                        int the_learning_rate_adaptation=0,
                        real the_adapt_coeff1=0,
                        real the_adapt_coeff2=0,
                        int n_updates=1, const string& filename="", 
                        int every_iterations=1);
      GradientOptimizer(VarArray the_params, Var the_cost, 
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

      
    DECLARE_NAME_AND_DEEPCOPY(GradientOptimizer);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies) { inherited::makeDeepCopyFromShallowCopy(copies); }

    virtual void build()
    {
      inherited::build();
      build_();
    }

  private:

    void build_() {
      stage = 0;
      early_stop = false;
      SumOfVariable* sumofvar = dynamic_cast<SumOfVariable*>((Variable*)cost);
      stochastic_hack = sumofvar!=0 && sumofvar->nsamples==1;
      params.clearGradient();
      int n = params.nelems();
      if (n > 0) {
        learning_rates.resize(n);
        gradient.resize(n);
        tmp_storage.resize(n);
        old_evol.resize(n);
        oldgradientlocations.resize(params.size());
        meancost.resize(cost->size());
        learning_rates.fill(start_learning_rate);
        switch (learning_rate_adaptation) {
          case 0:
            break;
          case 1:
            // tmp_storage is used to store the old parameters
            params.copyTo(tmp_storage);
            old_evol.fill(0);
            break;
          case 2:
            // tmp_storage is used to store the initial gradient
            Optimizer::computeGradient(this, tmp_storage);
            break;
        }
      }
    }
    
  public:

    virtual void oldwrite(ostream& out) const;
    virtual void oldread(istream& in);
    virtual real optimize();
    virtual bool optimizeN(VecStatsCollector& stats_coll);

  private:

    // Basic learning rate adaptation
    // If grad(i) > 0 : lr(i) = lr(i) * adapt_coeff1
    // else           : lr(i) = lr(i) * adapt_coeff2
    void adaptLearningRateBasic(
        Vec learning_rates,
        Vec old_params,
        Vec new_evol);

    // ALAP1 formula learning rate adaptation
    // lr = lr + adapt_coeff1 * dot(grad(k-1), grad(k))
    void adaptLearningRateALAP1(
        Vec old_gradient,
        Vec new_gradient);

  protected:

    static void declareOptions(OptionList& ol);

  };

DECLARE_OBJECT_PTR(GradientOptimizer);

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

      // Not implemented
      virtual bool optimizeN(VecStatsCollector& stat_coll);
  };



%> // end of namespace PLearn

#endif

