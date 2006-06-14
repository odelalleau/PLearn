// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent and Yoshua Bengio
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

#include "Optimizer.h"
//#define DEBUGCG
#ifdef DEBUGCG
#include <plearn/display/GhostScript.h>
#endif

namespace PLearn {
using namespace std;


Optimizer::Optimizer():
    early_stop(false),
    nstages(1),
    stage(0)
{}

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    Optimizer,
    "Base class for Optimization algorithms.",
    "In the PLearn context, optimizers operate on graph of Variable objects,\n"
    "mostly expressed in VarArray form.\n"
);

void Optimizer::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void Optimizer::build_()
{
    if (cost)
        setToOptimize(params, cost, other_costs, other_params, other_weight);
}

///////////
// reset //
///////////
void Optimizer::reset()
{
    stage = 0;
    early_stop = false;
}

////////////////////
// declareOptions //
////////////////////
void Optimizer::declareOptions(OptionList& ol)
{
    declareOption(ol, "nstages", &Optimizer::nstages, OptionBase::buildoption, 
        "Number of iterations to perform on the next call to optimizeN(..).");
    inherited::declareOptions(ol);
}

///////////////////
// setToOptimize //
///////////////////
void Optimizer::setToOptimize(const VarArray& the_params, Var the_cost, VarArray the_other_costs, TVec<VarArray> the_other_params, real the_other_weight)
{
    params = the_params;//displayVarGraph(params, true, 333, "p1", false);
    cost = the_cost;//displayVarGraph(cost[0], true, 333, "c1", false);
    proppath = propagationPath(params,cost);//displayVarGraph(proppath, true, 333, "x1", false);
    VarArray path_from_all_sources_to_direct_parents = propagationPathToParentsOfPath(params, cost);
    path_from_all_sources_to_direct_parents.fprop();//displayVarGraph(path_from_all_sources_to_direct_parents, true, 333, "x1", false);

    // This is probably not complete. Maybe a 
    // path_from_all_sources_to_direct_parents should also be computed and fproped
    other_costs = the_other_costs;
    other_params = the_other_params;
    other_proppaths.resize(other_costs.length());
    for(int i=0; i<other_proppaths.length(); i++)
        other_proppaths[i] = propagationPath(other_params[i],other_costs[i]);
    other_weight = the_other_weight;
}

/*
void Optimizer::setVarArrayOption(const string& optionname, VarArray value)
{
    if (optionname=="params") setToOptimize(value, cost);
    else if (optionname=="update_for_measure") update_for_measure = value;
    else PLERROR("In Optimizer::setVarArrayOption(const string& optionname, VarArray value): option not recognized (%s).",optionname.c_str());
}

void Optimizer::setVarOption(const string& optionname, Var value)
{
    if (optionname=="cost") setToOptimize(params, value);
    else PLERROR("In Optimizer::setVarOption(const string& optionname, VarArray value): option not recognized (%s).",optionname.c_str());
}

void Optimizer::setVMatOption(const string& optionname, VMat value)
{
    PLERROR("In Optimizer::setVMatOption(const string& optionname, VarArray value): option not recognized (%s).",optionname.c_str());
}
*/


//! To use varDeepCopyField.
#ifdef __INTEL_COMPILER
#pragma warning(disable:1419)  // Get rid of compiler warning.
#endif
extern void varDeepCopyField(Var& field, CopiesMap& copies);
#ifdef __INTEL_COMPILER
#pragma warning(default:1419)
#endif



/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void Optimizer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(params, copies);
    varDeepCopyField(cost, copies);
    deepCopyField(partial_update_vars, copies);
    deepCopyField(proppath, copies);
    deepCopyField(other_costs, copies);
    deepCopyField(other_params, copies);
    deepCopyField(other_proppaths, copies);
}

void Optimizer::verifyGradient(real minval, real maxval, real step)
{
    Func f(params,cost);
    f->verifyGradient(minval, maxval, step);
}

void Optimizer::verifyGradient(real step)
{
    Func f(params,cost);
    Vec p(params.nelems());
    params >> p;
    f->verifyGradient(p, step);
}

////////////////////////
// computeRepartition //
////////////////////////
void Optimizer::computeRepartition(
    Vec v, int n, real mini, real maxi, 
    Vec res, int& noutliers) {
    res.clear();
    noutliers = 0;
    for (int i=0; i<v.length(); i++) {
        real k = (v[i] - mini) / (maxi - mini);
        int j = int(k*n);
        if (j >= n) {
            noutliers++;
            j = n-1;
        }
        if (j < 0) {
            noutliers++;
            j = 0;
        }
        res[j]++;
    }
    for (int i = 0; i<n; i++) {
        res[i] /= v.length();
    }
}

/////////////////////
// computeGradient //
/////////////////////
void Optimizer::computeGradient(const Vec& gradient) {
    // Clear all what's left from previous computations
    this->proppath.clearGradient();
    this->params.clearGradient();
    this->cost->gradient[0] = 1;
    this->proppath.fbprop();
    this->params.copyGradientTo(gradient);
}

#ifdef DEBUGCG
extern GhostScript* gs;
#endif

/////////////////////////////
// computeOppositeGradient //
/////////////////////////////
void Optimizer::computeOppositeGradient(const Vec& gradient) {
    // Clear all what's left from previous computations
    this->proppath.clearGradient();
    this->params.clearGradient();
    // We want the opposite of the gradient, thus the -1
    this->cost->gradient[0] = -1;
    this->proppath.fbprop();
    this->params.copyGradientTo(gradient);
#ifdef DEBUGCG
    gs->setcolor("blue");
    gs->drawCircle(this->params[0]->value[0],this->params[0]->value[1],0.02);
#endif

}
  
} // end of namespace PLearn


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
