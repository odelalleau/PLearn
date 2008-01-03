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
 * $Id: AutoScaledGradientOptimizer.cc 5852 2006-06-14 14:40:03Z larocheh $
 * This file is part of the PLearn library.
 ******************************************************* */

#define PL_LOG_MODULE_NAME "AutoScaledGradientOptimizer"

#include "AutoScaledGradientOptimizer.h"
#include <plearn/io/pl_log.h>
#include <plearn/math/TMat_maths.h>
#include <plearn/display/DisplayUtils.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    AutoScaledGradientOptimizer,
    "Optimization by gradient descent with adapted scaling for each parameter.", 
    "This is a simple variation on the basic GradientOptimizer \n"
    "in which the gradient is scaled elementwise (for each parameter) \n"
    "by a scaling factor that is 1 over an average of the \n"
    "absolute value of the gradient plus some small epsilon. \n"
    "\n"
);

AutoScaledGradientOptimizer::AutoScaledGradientOptimizer():
    learning_rate(0.),   
    start_learning_rate(1e-2),
    decrease_constant(0),
    verbosity(0),
    evaluate_scaling_every(1000),
    evaluate_scaling_during(1000),
    epsilon(1e-6),
    nsteps_remaining_for_evaluation(-1)
{}


void AutoScaledGradientOptimizer::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "start_learning_rate", &AutoScaledGradientOptimizer::start_learning_rate,
        OptionBase::buildoption, 
        "The initial learning rate\n");

    declareOption(
        ol, "learning_rate", &AutoScaledGradientOptimizer::learning_rate,
        OptionBase::learntoption, 
        "The current learning rate\n");

    declareOption(
        ol, "decrease_constant", &AutoScaledGradientOptimizer::decrease_constant,
        OptionBase::buildoption, 
        "The learning rate decrease constant \n");

    declareOption(
        ol, "lr_schedule", &AutoScaledGradientOptimizer::lr_schedule,
        OptionBase::buildoption, 
        "Fixed schedule instead of decrease_constant. This matrix has 2 columns: iteration_threshold \n"
        "and learning_rate_factor. As soon as the iteration number goes above the iteration_threshold,\n"
        "the corresponding learning_rate_factor is applied (multiplied) to the start_learning_rate to\n"
        "obtain the learning_rate.\n");

    declareOption(
        ol, "verbosity", &AutoScaledGradientOptimizer::verbosity,
        OptionBase::buildoption, 
        "Controls the amount of output.  If zero, does not print anything.\n"
        "If 'verbosity'=V, print the current cost and learning rate if\n"
        "\n"
        "    stage % V == 0\n"
        "\n"
        "i.e. every V stages.  (Default=0)\n");

    declareOption(
        ol, "evaluate_scaling_every", &AutoScaledGradientOptimizer::evaluate_scaling_every,
        OptionBase::buildoption, 
        "every how-many steps should the mean and scaling be reevaluated\n");

    declareOption(
        ol, "evaluate_scaling_during", &AutoScaledGradientOptimizer::evaluate_scaling_during,
        OptionBase::buildoption, 
        "how many steps should be used to re-evaluate the mean and scaling\n");

    declareOption(
        ol, "epsilon", &AutoScaledGradientOptimizer::epsilon,
        OptionBase::buildoption, 
        "scaling will be 1/(mean_abs_grad + epsilon)\n");

    inherited::declareOptions(ol);
}


void AutoScaledGradientOptimizer::setToOptimize(const VarArray& the_params, Var the_cost, VarArray the_other_costs, TVec<VarArray> the_other_params, real the_other_weight)
{
    inherited::setToOptimize(the_params, the_cost, the_other_costs, the_other_params, the_other_weight);
    int n = params.nelems();
    param_values = Vec(n);
    param_gradients = Vec(n);
    params.makeSharedValue(param_values);
    params.makeSharedGradient(param_gradients);
    scaling.resize(n);
    scaling.clear();
    if(epsilon<0)
        scaling.fill(1.0);
    meanabsgrad.resize(n);
    meanabsgrad.clear();
}


// static bool displayvg=false;

bool AutoScaledGradientOptimizer::optimizeN(VecStatsCollector& stats_coll) 
{
    PLASSERT_MSG(other_costs.length()==0, "gradient on other costs not currently supported");

    param_gradients.clear();

    int stage_max = stage + nstages; // the stage to reach

    int current_schedule = 0;
    int n_schedules = lr_schedule.length();
    if (n_schedules>0)
        while (current_schedule+1 < n_schedules && stage > lr_schedule(current_schedule,0))
            current_schedule++;
    
    while (stage < stage_max) 
    {        
        if (n_schedules>0)
        {
            while (current_schedule+1 < n_schedules && stage > lr_schedule(current_schedule,0))
                current_schedule++;
            learning_rate = start_learning_rate * lr_schedule(current_schedule,1);
        }
        else
            learning_rate = start_learning_rate/(1.0+decrease_constant*stage);

        proppath.clearGradient();
        cost->gradient[0] = 1.0;

        static bool display_var_graph_before_fbprop=false;
        if (display_var_graph_before_fbprop)
            displayVarGraph(proppath, true, 333);
        proppath.fbprop(); 
#ifdef BOUNDCHECK
        int np = params.size();
        for(int i=0; i<np; i++)
            if (params[i]->value.hasMissing())
                PLERROR("parameter updated with NaN");
#endif
        static bool display_var_graph=false;
        if (display_var_graph)
            displayVarGraph(proppath, true, 333);

//       // Debugging of negative NLL bug...
//       if (cost->value[0] <= 0) {
//         displayVarGraph(proppath, true, 333);
//         cerr << "Negative NLL cost vector = " << cost << endl;
//         PLERROR("Negative NLL encountered in optimization");
//       }

        // set params += -learning_rate * params.gradient * scaling
        {
        real* p_val = param_values.data();
        real* p_grad = param_gradients.data();
        real* p_scale = scaling.data();
        real neg_learning_rate = -learning_rate;

        int n = param_values.length();
        while(n--)
            *p_val++ += neg_learning_rate*(*p_grad++)*(*p_scale++);
        }

        if(stage%evaluate_scaling_every==0)
        {
            nsteps_remaining_for_evaluation = evaluate_scaling_during;
            meanabsgrad.clear();
            if(verbosity>=4)
                perr << "At stage " << stage << " beginning evaluating meanabsgrad during " << evaluate_scaling_during << " stages" << endl;
        }

        if(nsteps_remaining_for_evaluation>0)
        {
            real* p_grad = param_gradients.data();
            real* p_mean = meanabsgrad.data();
            int n = param_gradients.length();
            while(n--)
                *p_mean++ += fabs(*p_grad++);
            --nsteps_remaining_for_evaluation;

            if(nsteps_remaining_for_evaluation==0) // finalize evaluation
            {
                int n = param_gradients.length();                
                for(int i=0; i<n; i++)
                {
                    meanabsgrad[i] /= evaluate_scaling_during;
                    scaling[i] = 1.0/(meanabsgrad[i]+epsilon);
                }
                if(verbosity>=4)
                    perr << "At stage " << stage 
                         << " finished evaluating meanabsgrad. It's in range: ( " 
                         << min(meanabsgrad) << ",  " << max(meanabsgrad) << " )" << endl;
                if(verbosity>=5)
                    perr << meanabsgrad << endl;

                if(epsilon<0)
                    scaling.fill(1.0);
            }
        }
        param_gradients.clear();

        if (verbosity > 0 && stage % verbosity == 0) {
            MODULE_LOG << "Stage " << stage << ": " << cost->value
                       << "\tlr=" << learning_rate
                       << endl;
        }
        stats_coll.update(cost->value);
        ++stage;
    }

    return false;
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
