// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003,2006 Olivier Delalleau
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


#ifndef CONJGRADIENTOPTIMIZER_INC
#define CONJGRADIENTOPTIMIZER_INC

#include "Optimizer.h"

namespace PLearn {
using namespace std;


class ConjGradientOptimizer : public Optimizer {

    typedef Optimizer inherited;

public:

    // Public options.

    real constrain_limit;
    real expected_red;
    int max_eval_per_line_search;
    real max_extrapolate;
    real rho;
    real sigma;
    real slope_ratio;
    int verbosity;

protected:
  
    /*
    Vec meancost;              // used to store the cost, for display purpose
    */

    // RAS stuff.

    real cubic_a, cubic_b, step2, fun_val1, step1, fun_val2, fun_deriv2, fun_deriv1;
    real bracket_limit;

    int fun_eval_count;
    bool line_search_failed, line_search_succeeded;
    

private:

    // Internal data
    // TODO See what we can get rid of.
    Vec current_opp_gradient;  // current opposite gradient value
    Vec search_direction;      // current search direction for the line search
    Vec tmp_storage;           // used for temporary storage of data
    Vec delta;                 // temporary storage of the gradient
    real last_improvement;     // cost improvement during the last iteration
    real last_cost;            // last cost computed
    real current_step_size;    // current step size for line search
  
public:

    ConjGradientOptimizer();
  
    PLEARN_DECLARE_OBJECT(ConjGradientOptimizer);

    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void build()
    {
        inherited::build();
        build_();
    }
  
private:

    void build_();
    
public:

    virtual bool optimizeN(VecStatsCollector& stat_coll);

    virtual void reset();

protected:

    static void declareOptions(OptionList& ol);
  
    //! Find the new search direction for the line search algorithm
    bool findDirection();

    //! Search the minimum in the current search direction
    //! Return true iif no improvement was possible (and we can stop here)
    bool lineSearch();

    //! Update the search_direction by
    //! search_direction = delta + gamma * search_direction
    //! Delta is supposed to be the current opposite gradient
    //! Also update current_opp_gradient to be equal to delta
    void updateSearchDirection(real gamma);

    //----------------------- CONJUGATE GRADIENT FORMULAS ---------------------
    //
    // A Conjugate Gradient formula finds the new search direction, given
    // the current gradient, the previous one, and the current search direction.
    // It returns a constant gamma, which will be used in :
    // h(n) = -g(n) + gamma * h(n-1)

    // The Polak-Ribiere formula used to find the new direction
    // h(n) = -g(n) + dot(g(n), g(n)-g(n-1)) / norm2(g(n-1)) * h(n-1)
    real polakRibiere();

    //------------------------- LINE SEARCH ALGORITHMS -------------------------
    //
    // A line search algorithm moves "params" to the value minimizing "cost",
    // when moving in the direction "search_direction".
    // It must not update "current_opp_gradient" (that is done in the Conjugate
    // Gradient formulas).
    // It must return the optimal step found to minimize the gradient.

    // TODO Comment Rasmussen's algorithm.
    real rasmussenSearch();
  
    //--------------------------- UTILITY FUNCTIONS ----------------------------
  
protected:

    // Return cost->value() after an update of params with step size alpha
    // in the current search direction
    // ie : f(x) = cost(params + x*search_direction) in x = alpha
    real computeCostValue(real alpha);

    // Return the derivative of the function
    // f(x) = cost(params + x*search_direction)
    // in x = alpha
    real computeDerivative(real alpha);

    // Same as the two functions above combined.
    // The result is returned in the cost and derivative parameters.
    void computeCostAndDerivative(real alpha, real& cost, real& derivative);

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
