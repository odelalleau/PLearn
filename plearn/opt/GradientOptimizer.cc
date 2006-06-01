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

#define PL_LOG_MODULE_NAME "GradientOptimizer"

#include "GradientOptimizer.h"
#include <plearn/io/pl_log.h>
#include <plearn/math/TMat_maths.h>
#include <plearn/display/DisplayUtils.h>
#include <plearn/var/SumOfVariable.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    GradientOptimizer,
    "Optimization by gradient descent.", 
    "GradientOptimizer is the simple usual gradient descent algorithm \n"
    "(the number of samples on which to estimate gradients before an \n"
    "update, which determines whether we are performing 'batch' \n"
    "'stochastic' or even 'minibatch', is currently specified outside \n"
    "this class, typically in the numer of s/amples of the meanOf function \n"
    "to be optimized, as its 'nsamples' parameter). \n"
    "\n"
    "Options for GradientOptimizer are [ option_name: <type> (default) ]: \n"
    "  - start_learning_rate: <real> (0.01) \n"
    "    the initial learning rate \n"
    "  - decrease_constant: <real> (0) \n"
    "    the learning rate decrease constant \n"
    "\n"
);

GradientOptimizer::GradientOptimizer():
    learning_rate(0.),   
    start_learning_rate(1e-2),
    decrease_constant(0),
    use_stochastic_hack(false),
    verbosity(0)
{}


void GradientOptimizer::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "start_learning_rate", &GradientOptimizer::start_learning_rate,
        OptionBase::buildoption, 
        "The initial learning rate\n");

    declareOption(
        ol, "learning_rate", &GradientOptimizer::learning_rate,
        OptionBase::learntoption, 
        "The current learning rate\n");

    declareOption(
        ol, "decrease_constant", &GradientOptimizer::decrease_constant,
        OptionBase::buildoption, 
        "The learning rate decrease constant \n");

    declareOption(
        ol, "lr_schedule", &GradientOptimizer::lr_schedule,
        OptionBase::buildoption, 
        "Fixed schedule instead of decrease_constant. This matrix has 2 columns: iteration_threshold \n"
        "and learning_rate_factor. As soon as the iteration number goes above the iteration_threshold,\n"
        "the corresponding learning_rate_factor is applied (multiplied) to the start_learning_rate to\n"
        "obtain the learning_rate.\n");

    declareOption(
        ol, "use_stochastic_hack", &GradientOptimizer::use_stochastic_hack,
        OptionBase::buildoption, 
        "Indication that a stochastic hack to accelerate stochastic gradient descent should be used.\n"
        "Be aware that it will not take into account minimum and maximum values in variables.\n"
        );

    declareOption(
        ol, "verbosity", &GradientOptimizer::verbosity,
        OptionBase::buildoption, 
        "Controls the amount of output.  If zero, does not print anything.\n"
        "If 'verbosity'=V, print the current cost and learning rate if\n"
        "\n"
        "    stage % V == 0\n"
        "\n"
        "i.e. every V stages.  (Default=0)\n");

    inherited::declareOptions(ol);
}


// static bool displayvg=false;

bool GradientOptimizer::optimizeN(VecStatsCollector& stats_coll) 
{
    // Big hack for the special case of stochastic gradient, to avoid doing an
    // explicit update (temporarily change the gradient fields of the
    // parameters to point to the parameters themselves, so that gradients are
    // "accumulated" directly in the parameters, thus updating them!
    SumOfVariable* sumofvar = dynamic_cast<SumOfVariable*>((Variable*)cost);
    Array<Mat> oldgradientlocations;
    bool stochastic_hack = use_stochastic_hack && sumofvar!=0 && sumofvar->nsamples==1;
    //stochastic_hack=false;
    if(stochastic_hack)
    {
        // make the gradient and values fields of parameters point to the same
        // place, so that when the descendants of the parameter Var's do a
        // bprop this automatically increments the parameters (by the right
        // amount since we set the cost->gradient to -learning_rate).
        int n = params.size();
        oldgradientlocations.resize(n);
        for(int i=0; i<n; i++)
            oldgradientlocations[i] = params[i]->defineGradientLocation(params[i]->matValue);
    }
    else
        params.clearGradient();

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
        cost->gradient[0] = -learning_rate;

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

        // set params += -learning_rate * params.gradient
        if(!stochastic_hack)
            params.updateAndClear();
        else
            if(partial_update_vars.length() != 0) 
                for(int i=0; i<partial_update_vars.length(); i++)
                    partial_update_vars[i]->clearRowsToUpdate();
        if (verbosity > 0 && stage % verbosity == 0) {
            MODULE_LOG << "Stage " << stage << ": " << cost->value
                       << "\tlr=" << learning_rate
                       << endl;
        }
        stats_coll.update(cost->value);
        ++stage;
    }

    if(stochastic_hack) // restore the gradients as they previously were...
    {
        int n = params.size();
        for(int i=0; i<n; i++)
            params[i]->defineGradientLocation(oldgradientlocations[i]);
    }
    return false;
}

// Very old code.  TO BE DEPRECATED
#if 0
/*
real ScaledGradientOptimizer::optimize()
{
    ofstream out;
    if (!filename.empty())
        out.open(filename.c_str());

    eps_scale.fill(1.0);
    Vec first_long_time_mv; 
    real best_cost = 1e30;
    Vec prev_params(gradient.length());
    Vec prev_gradient(gradient.length());
    Vec best_params(gradient.length());
    Vec best_gradient(gradient.length());
    params >> prev_params;
    params >> best_params;
    params.copyGradientTo(prev_gradient);
    params.copyGradientTo(best_gradient);
    int n_long = (int)(1.0/(short_time_mac*long_time_mac));
    cout << "start learning rate = " << start_learning_rate << endl;
    learning_rate = 0;
    Vec meancost(cost->size());
    Vec lastmeancost(cost->size());
    early_stop = false;
    for (int t=0; !early_stop && t<nupdates; t++)
    {
        params.clearGradient();
        proppath.clearGradient();
        cost->gradient[0] = 1.0;
        proppath.fbprop();
        if (every!=0) 
        {
            if ((t%every==0) && (t>0)) 
            {
                meancost /= real(every);      
                if (meancost[0] > best_cost)
                {
                    start_learning_rate *= 0.5;
                    params << best_params;
                    params.copyGradientFrom(best_gradient);
                }
                else
                {
                    best_cost = meancost[0];
                    best_params << prev_params;
                    best_gradient << prev_gradient;
                    params >> prev_params;
                    params.copyGradientTo(prev_gradient);
                    start_learning_rate *= 1.1;
                }
                learning_rate = start_learning_rate/(1.0+decrease_constant*t);
                cout << t << ' ' << meancost << ' ' << learning_rate << endl;
                if (out)
                    out << t << ' ' << meancost << ' ' << learning_rate << endl;
                early_stop = measure(t,meancost);
                lastmeancost << meancost;
                meancost.clear();
            }
            else
            {
                learning_rate = start_learning_rate/(1.0+decrease_constant*t);
            }
        } 
        params.copyGradientTo(gradient);
        if (t<n_long-1)
            // prepare to initialize the moving average
            // (by doing initially a batch average)
        {
            long_time_ma += gradient;
            squareAcc(long_time_mv, gradient);
        }
        else if (t==n_long-1) 
            // prepare to initialize the moving averages
        {
            long_time_ma *= real(1.0)/ (real)n_long;
            long_time_mv *= real(1.0)/ (real)n_long;
            squareMultiplyAcc(long_time_mv, long_time_ma,(real)-1);
            first_long_time_mv << long_time_mv;
            short_time_ma << long_time_ma;
        }
        else 
            // steady-state mode
        {
            exponentialMovingAverageUpdate(short_time_ma, gradient,short_time_mac);
            exponentialMovingAverageUpdate(long_time_ma, short_time_ma,long_time_mac);
            exponentialMovingSquareUpdate(long_time_mv, gradient,long_time_mac);
            if (t%n_long==0)
            {
                real prev_eps = 0.5*(max(eps_scale)+mean(eps_scale));
                //apply(long_time_mv,long_time_md,sqrt);
                cout << "******* AT T= " << t << " *******" << endl;
                cout << "average gradient norm = " 
                     << norm(long_time_ma) << endl;
                cout << "average gradient = " << long_time_ma << endl;
                //cout << "short time average gradient = " << short_time_ma << endl;
                Vec long_time_md = sqrt(long_time_mv);
                cout << "sdev(gradient) = " << long_time_md << endl;
                cout << "mean(sdev(gradient)) = " << mean(long_time_md) << endl;
                add(long_time_mv,regularizer,eps_scale);
                //divide(1.0,long_time_mv,eps_scale);
                //divide(first_long_time_mv,long_time_mv,eps_scale);
                cout << "eps_scale = " << eps_scale << endl;
                real new_eps = 0.5*(max(eps_scale)+mean(eps_scale));
                start_learning_rate *= prev_eps / new_eps;
                learning_rate = start_learning_rate / (1 + decrease_constant*t);
                cout << "scale learning rate by " << prev_eps / new_eps << " to " << learning_rate << endl;

                //real *e=eps_scale.data();
                //for (int i=0;i<eps_scale.length();i++)
                //  if (e[i]>regularizer) e[i]=regularizer;
                //cout << "regularized  eps_scale = " << eps_scale << endl;
                //cout << "avg/sdev) = " << long_time_md  << endl;
                //eps_scale *= learning_rate;
                //cout << "regularized eps_scale * learning_rate = " << eps_scale << endl;
            }
        }
        // set params += -learning_rate * params.gradient
        meancost += cost->value;
        gradient *= eps_scale;
        params.update(-learning_rate,gradient);
    }
    return meancost[0];
}
*/
#endif // #if 0

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
