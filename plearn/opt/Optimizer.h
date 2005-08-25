// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
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


/*! \file PLearnLibrary/PLearnCore/Optimizer.h */

#ifndef OPTIMIZER_INC
#define OPTIMIZER_INC

#include <plearn/var/Func.h>
#include <plearn/math/Mat.h>
#include <plearn/measure/Measurer.h>
#include <plearn/base/Object.h>
#include <plearn/math/VecStatsCollector.h>
#include <plearn/vmat/VMat.h>

namespace PLearn {
using namespace std;


#define ALL_SAMPLES (-1)
#define DEFAULT_SAMPLES (-2)

//typedef void (*OptimizerCallback)(int t);

class Optimizer : public Object
{
    typedef Object inherited;
      
public:
    VarArray params;
    Var cost;
    VarArray proppath; //forward and/or backward
    int nupdates; // deprecated  TODO Remove ?
    int nstages; //!< number of steps to perform when calling optimizeN
    int stage;   //!< current number of steps performed

    bool early_stop;
    int early_stop_i;// number of epoch before early stopping

    VarArray update_for_measure; // not used if length()==0
    //OptimizerCallback callback; //!<  callback function

    //oassignstream vlog;
    PStream vlog;
    // TODO Looks like this PStream is never used!
      
private:
    Vec temp_grad;  //!< used to store temp stuff for gradient stats
/*      Vec same_sign;  //!< number of consecutive updates in same direction */

protected:

    Array<Measurer*> measurers;

    //!  call measure <every> <nupdates> iterations
    //!  saving the results in the <filename>.
    string filename; // JS - that was const...
      
public:
    int every; //!<  if = 0 don't print or measure anything

public:
    Optimizer(int n_updates=1, const string& file_name="",
              int every_iterations=1);
    Optimizer(VarArray the_params, Var the_cost,
              int n_updates=1, const string& file_name="",
              int every_iterations=1);
    Optimizer(VarArray the_params, Var the_cost, 
              VarArray the_update_for_measure,
              int n_updates=1, const string& file_name="",
              int every_iterations=1);
      
    virtual void init() { build(); } // DEPRECATED : use build() instead
    virtual void build();

private:
    void build_();

public:

    virtual void reset(); 

    virtual void setToOptimize(VarArray the_params, Var the_cost);
      
    virtual void setVarArrayOption(const string& optionname, VarArray value);
    virtual void setVarOption(const string& optionname, Var value);
    virtual void setVMatOption(const string& optionname, VMat value);
      
    PLEARN_DECLARE_ABSTRACT_OBJECT(Optimizer);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
      
    void addMeasurer(Measurer& measurer);
      
    virtual bool measure(int t, const Vec& costs);
      
    //!  sub-classes should define this, which is the main method
    virtual real optimize() = 0;

    //!  sub-classes should define this, which is the new main method
    virtual bool optimizeN(VecStatsCollector& stats_coll) =0;
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

    virtual void oldwrite(ostream& out) const;
    /* TODO Remove (deprecated)
       virtual void oldread(istream& in);
    */
      
    virtual ~Optimizer();

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
    real collectGradientStats(Vec gradient);

    //! Given an optimizer, compute the gradient of the cost function and
    //! store it in the "gradient" Vec
    static void computeGradient(
        Optimizer* opt,
        const Vec& gradient);
      
    //! Given an optimizer, compute the opposite of the gradient of the cost
    //! function and store it in the "gradient" Vec
    static void computeOppositeGradient(
        Optimizer* opt,
        const Vec& gradient);

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
