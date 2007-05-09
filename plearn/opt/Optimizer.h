// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999, 2000 Pascal Vincent and Yoshua Bengio
// Copyright (C) 2000, 2006 University of Montreal
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

#ifndef OPTIMIZER_INC
#define OPTIMIZER_INC

#include <plearn/base/Object.h>
#include <plearn/var/Func.h>
#include <plearn/math/Mat.h>
#include <plearn/measure/Measurer.h>
#include <plearn/math/VecStatsCollector.h>
#include <plearn/vmat/VMat.h>

namespace PLearn {
using namespace std;


#define ALL_SAMPLES (-1)
#define DEFAULT_SAMPLES (-2)

class Optimizer : public Object
{
    typedef Object inherited;
      
public:

    VarArray params;
    Var cost;
    //! Vars that are partially updated. 
    // This lets the optimizer avoid the growth of rows_to_update for
    // those vars, when bprop is called but not updateAndClear()
    // (for instance when using the stochastic_hack of GradientOptimizer).
    VarArray partial_update_vars;
    VarArray proppath; //forward and/or backward

    //! Boolean used in subclasses to notify of early stopping.
    // TODO Might just be better to move it into subclasses?
    bool early_stop;
    int nstages; //!< number of steps to perform when calling optimizeN
    int stage;   //!< current number of steps performed

    //! Other costs (for regularisation for example)
    VarArray other_costs;
    //! Parameters of other costs to update (usually a subset of params)
    TVec<VarArray> other_params;
    //! Propagation paths of other_costs
    TVec<VarArray> other_proppaths;
    //! Weight for all the other costs
    real other_weight;

public:

    //! Default constructor.
    Optimizer();
      
    virtual void build();

private:

    void build_();

public:

    virtual void reset(); 

    virtual void setToOptimize(const VarArray& the_params, Var the_cost, VarArray the_other_costs = VarArray(0), TVec<VarArray> the_other_params = TVec<VarArray>(0), real the_other_weight = 1);

    /*
    virtual void setVarArrayOption(const string& optionname,
                                   const VarArray& value);
    virtual void setVarOption(const string& optionname, Var value);
    virtual void setVMatOption(const string& optionname, VMat value);
    */
      
    PLEARN_DECLARE_ABSTRACT_OBJECT(Optimizer);

    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
      
    //! Main optimization method, to be defined in subclasses.
    //! Return true iff no further optimization is possible.
    virtual bool optimizeN(VecStatsCollector& stats_coll) = 0;
    /* while (stage < stage_init + nstages) {
     *   params.update(..)
     *   stats_coll.update(cost)
     *   stage++
     *   if finished return is_finished
     * }
     * return false
     */
      
    //!  verify gradient with uniform random initialization of parameters
    //!  using step for the finite difference approximation of the gradient
    void verifyGradient(real minval, real maxval, real step);
      
    //!  verify gradient at the current value of the parameters
    //!  using step for the finite difference approximation of the gradient
    void verifyGradient(real step);

    virtual void setPartialUpdateVars(const VarArray& the_partial_update_vars)
    {
        partial_update_vars = the_partial_update_vars;
    }
      
protected:

    static void declareOptions(OptionList& ol);

public:

    //--------------------------- UTILITY FUNCTIONS ----------------------------

    //! Compute the repartition of v by splitting the interval [mini,maxi] into
    //! n intervals. The result is stored into res.
    void computeRepartition(
        Vec v, int n, real mini, real maxi, 
        Vec res, int& noutliers);

    //! Collect various statistics on the gradient.
    real collectGradientStats(const Vec& gradient);

    //! Given an optimizer, compute the gradient of the cost function and
    //! store it in the "gradient" Vec
    void computeGradient(const Vec& gradient);

    //! Given an optimizer, compute the opposite of the gradient of the cost
    //! function and store it in the "gradient" Vec
    void computeOppositeGradient(const Vec& gradient);

};

DECLARE_OBJECT_PTR(Optimizer);

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
