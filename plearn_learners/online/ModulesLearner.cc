// -*- C++ -*-

// ModulesLearner.cc
//
// Copyright (C) 2006 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file ModulesLearner.cc */


#include "ModulesLearner.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ModulesLearner,
    "Trains an OnlineLearningModule wrt the cost of a CostModule.",
    "The CostModule provides the output gradient to train the\n"
    "OnlineLearningModule.\n"
    "In order to stack layers, you can use StackedModulesModule,\n"
    "and in order to compute several costs, you can use CombinedCostsModule.\n"
    );

ModulesLearner::ModulesLearner()
    : hessian_estimation( "none" )
{
    random_gen = new PRandom();
}

void ModulesLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "module", &ModulesLearner::module,
                  OptionBase::buildoption,
                  "The module to train");

    declareOption(ol, "cost", &ModulesLearner::cost,
                  OptionBase::buildoption,
                  "The cost module");

    declareOption(ol, "hessian_estimation",
                  &ModulesLearner::hessian_estimation,
                  OptionBase::buildoption,
                  "Estimation of the second-order terms. One of:\n"
                  "  - \"none\": using only first-order derivative for"
                  " update,\n"
                  "  - \"diag\": estimating the diagonal of the hessian,\n"
                  "  - \"simpler_diag\": positive estimation of the diagonal\n"
                 );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ModulesLearner::build_()
{
    // hessian estimation
    string h_est = lowerstring( hessian_estimation );
    if( h_est == "none" || h_est == "" )
        hessian_estimation = "none";
    else if( h_est == "diag" )
        hessian_estimation = h_est;
    else if( h_est == "simpler_diag" )
        hessian_estimation = h_est;
    else
        PLERROR( "ModulesLearner::buildOptions(): hessian_estimation\n"
                 "value '%s' is unknown.\n", hessian_estimation.c_str() );

    if( hessian_estimation == "diag" )
        cost->estimate_simpler_diag_hessian = false;
    else
        cost->estimate_simpler_diag_hessian = true;

    // initialize random generator from seed
    random_gen->manual_seed( seed_ );

    module->random_gen = random_gen;
    cost->random_gen = random_gen;

    // if train_set is not set, we don't know inputsize nor targetsize
    if( inputsize_ >= 0 ) // we don't use inputsize() because it crashes if <0
    {
        module->input_size = inputsize();
        module->build();

        output.resize( outputsize() );
        d_output.resize( outputsize() );
        if( hessian_estimation != "none" )
            d2_output.resize( outputsize() );

        cost->input_size = outputsize();
        cost->target_size = targetsize();
        cost->build();

        costs->resize( cost->output_size );
    }
}


// ### Nothing to add here, simply calls build_
void ModulesLearner::build()
{
    inherited::build();
    build_();
}


void ModulesLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(trainvec, copies);

    deepCopyField(module, copies);
    deepCopyField(cost, copies);
    deepCopyField(output, copies);
    deepCopyField(d_output, copies);
    deepCopyField(d2_output, copies);
    deepCopyField(costs, copies);
}


int ModulesLearner::outputsize() const
{
    // Compute and return the size of this learner's output (which typically
    // may depend on its inputsize(), targetsize() and set options).
    if( module )
        return module->output_size;
    else
        return -1;
}

void ModulesLearner::forget()
{
    //! (Re-)initialize the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!)
    /*!
      A typical forget() method should do the following:
      - initialize a random number generator with the seed option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */

    random_gen->manual_seed( seed_ );

    // reset temporary vectors
    output.clear();
    d_output.clear();
    if( d2_output )
        d2_output.clear();
    costs.clear();

    // reset module and cost
    module->forget();
    cost->forget();

    stage = 0;
}

void ModulesLearner::train()
{
    // The role of the train method is to bring the learner up to
    // stage==nstages, updating train_stats with training costs measured
    // on-line in the process.

    /* TYPICAL CODE:

    static Vec input;  // static so we don't reallocate memory each time...
    static Vec target; // (but be careful that static means shared!)
    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()
    real weight;

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    while(stage<nstages)
    {
        // clear statistics of previous epoch
        train_stats->forget();

        //... train for 1 stage, and update train_stats,
        // using train_set->getExample(input, target, weight)
        // and train_stats->update(train_costs)

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }
    */

    Vec input( inputsize() );
    Vec target( targetsize() );
    real weight;
    int nsamples = train_set->length();

    if( !initTrain() )
        return;

    ProgressBar* pb = 0;
    if( report_progress )
        pb = new ProgressBar( "Training " + classname() + " from stage "
                              + tostring(stage) + " to " + tostring(nstages),
                              nstages - stage );

    int initial_stage = stage;
    for( ; stage < nstages ; stage++ )
    {
        // clear stats of previous epoch
        train_stats->forget();
        for( int sample=0 ; sample < nsamples ; sample++ )
        {
            train_set->getExample( sample, input, target, weight );

            // fprop
            module->fprop( input, output );
            cost->fprop( output, target, costs );

            // bprop
            if( hessian_estimation != "none" ) // bbpropUpdate
            {
                cost->bbpropUpdate( output, target, costs[0],
                                    d_output, d2_output );

                module->bbpropUpdate( input, output, d_output, d2_output );
            }
            else // bpropUpdate
            {
                cost->bpropUpdate( output, target, costs[0], d_output );

                module->bpropUpdate( input, output, d_output );
            }

            train_stats->update( costs );
        }
        train_stats->finalize(); // finalize statistics for this epoch

        if(pb)
            pb->update( stage+1 - initial_stage );
    }
    if( pb )
    {
        delete pb;
        pb = 0;
    }
}


void ModulesLearner::computeOutput(const Vec& input, Vec& output) const
{
    module->fprop( input, output );
}

void ModulesLearner::computeCostsFromOutputs(const Vec& input,
                                             const Vec& output,
                                             const Vec& target,
                                             Vec& costs) const
{
    cost->fprop( output, target, costs );
}

TVec<string> ModulesLearner::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    return cost->name();
}

TVec<string> ModulesLearner::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes
    // and for which it updates the VecStatsCollector train_stats
    return cost->name();
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
