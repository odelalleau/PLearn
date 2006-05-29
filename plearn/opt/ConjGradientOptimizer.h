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
    real max_extrapolate;
    real rho;
    real sigma;
    real slope_ratio;
    int max_eval_per_line_search;
    bool no_negative_gamma;
    int verbosity;

protected:
  
    //! Bracket limit.
    real bracket_limit;

    //! Cubic interpolation coefficients.
    real cubic_a, cubic_b;

    //! Current cost (=function) value.
    real current_cost;

    //! Function derivative (w.r.t. to the step along the search direction).
    real fun_deriv2, fun_deriv1;

    //! Function values.
    real fun_val1, fun_val2;

    //! Step values along the search direction, during line search.
    real step1, step2;

    //! Counter to make sure the number of function evaluations does not exceed
    //! the 'max_eval_per_line_search' option.
    int fun_eval_count;

    //! Booleans indicating the line search outcome.
    bool line_search_failed, line_search_succeeded;

    Vec current_opp_gradient;  //!< Current opposite gradient value.
    Vec search_direction;      //!< Current search direction for line search.
    Vec tmp_storage;           //!< Temporary data storage.
    Vec delta;                 //!< Temporary storage of the gradient.
  
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
  
    //! Find the new search direction for the line search algorithm.
    void findDirection();

    //! Search the minimum in the current search direction.
    //! Return false iff no improvement was possible (and we can stop here).
    bool lineSearch();

    //! Update the search_direction by
    //!     search_direction = delta + gamma * search_direction
    //! 'delta' is supposed to be the opposite gradient at the point we have
    //! reached during the line search: 'current_opp_gradient' is also updated
    //! in this function (set equal to 'delta').
    void updateSearchDirection(real gamma);

    //! A Conjugate Gradient formula finds the new search direction, given
    //! the current gradient, the previous one, and the current search direction.
    //! It returns a constant gamma, which will be used in :
    //!   search(t) = -gradient(t) + gamma * search(t-1)
    //! The Polak-Ribiere formula is:
    //!   gamma = dot(gradient(t), gradient(t)-gradient(t-1))
    //!             / ||gradient(t-1)||^2
    real polakRibiere();

    //! A line search algorithm moves 'params' to the value minimizing 'cost',
    //! when moving in the direction 'search_direction'.
    //! It must not update 'current_opp_gradient' (this is done later in
    //! updateSearchDirection(..)).
    //! It returns the optimal step found to minimize the gradient.
    //! The following line search algorithm is inspired by Carl Rasmussen's
    //! 'minimize' Matlab algorithm.
    real minimizeLineSearch();
  
protected:

    //! Return cost->value() after an update of params with step size alpha
    //! in the current search direction, i.e:
    //!     f(x) = cost(params + x*search_direction) in x = alpha.
    //! The parameters' values are not modified by this function.
    real computeCostValue(real alpha);

    //! Return the derivative of the function
    //!     f(x) = cost(params + x*search_direction)
    //! in x = alpha.
    //! The parameters' values are not modified by this function (however, the
    //! gradients are altered).
    real computeDerivative(real alpha);

    // Same as the two functions above combined. The result is returned through
    // the 'cost' and 'derivative' parameters.
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
