// -*- C++ -*-

// StackedModulesLearner.cc
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

/*! \file StackedModulesLearner.cc */


#include "StackedModulesLearner.h"
#include <plearn/math/PRandom.h>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/SquaredErrModule.h>
#include <plearn_learners/online/NLLErrModule.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    StackedModulesLearner,
    "Trains a stack of OnlineLearningModule, which are layers.",
    "The OnlineLearningModule's are disposed like superposed layers:\n"
    "outputs of module i are the inputs of module (i+1), the last layer is\n"
    "the output layer.\n"
    "Another TVec of modules contains the cost modules. The first one is"
    " used\n"
    "during the training phase as the cost to minimize, the other ones are"
    " only\n"
    "measured.\n");

StackedModulesLearner::StackedModulesLearner()
    : cost_funcs( 1, "mse" ),
      hessian_estimation( "none" ),
      nmodules( 0 ),
      ncosts( 1 )
{
    random_gen = new PRandom();
}

void StackedModulesLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "modules", &StackedModulesLearner::modules,
                  OptionBase::buildoption,
                  "Layers of the learner");

    declareOption(ol, "cost_funcs", &StackedModulesLearner::cost_funcs,
                  OptionBase::buildoption,
                  "Names of the cost functions to apply on output. First one\n"
                  "will be used ass the cost function to optimize. For the"
                  " moment,\n"
                  "supported value are:\n"
                  "   - \"mse\" (default)\n"
                  "   - \"NLL\"\n");

    declareOption(ol, "hessian_estimation",
                  &StackedModulesLearner::hessian_estimation,
                  OptionBase::buildoption,
                  "Estimation of the second-order terms. One of:\n"
                  "  - \"none\": using only first-order derivative for"
                  " update,\n"
                  "  - \"diag\": estimating the diagonal of the hessian,\n"
                  "  - \"simpler_diag\": positive estimation of the diagonal\n"
                 );

    declareOption(ol, "random_gen", &StackedModulesLearner::random_gen,
                  OptionBase::buildoption,
                  "Random numbers generator.");

    declareOption(ol, "nmodules", &StackedModulesLearner::nmodules,
                  OptionBase::learntoption,
                  "Number of module layers");

    declareOption(ol, "cost_modules", &StackedModulesLearner::cost_modules,
                  OptionBase::learntoption,
                  "Modules that will compute the costs");

    declareOption(ol, "ncosts", &StackedModulesLearner::ncosts,
                  OptionBase::learntoption,
                  "Number of cost modules");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void StackedModulesLearner::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.

    // build the layers
    buildOptions();

    // initialize random generator from seed
    random_gen->manual_seed( seed_ );

    // if train_set is not set, we don't know inputsize nor targetsize
    if( inputsize_ >= 0 ) // we don't use inputsize() because it crashes if <0
        buildLayers();

    // build cost functions
    buildCostFunctions();
}

void StackedModulesLearner::buildOptions()
{
    nmodules = modules.length();
    ncosts = cost_funcs.length();

    // check length of cost_modules
    int ncm = cost_modules.length();
    if( ncm != 0 && ncm != ncosts )
        PLWARNING( "StackedModulesLearner::buildOptions(): 'cost_modules' is"
                   " set,\n"
                   "but its length differ from 'cost_funcs.length()'"
                   " (%d != %d).\n",
                   ncm, ncosts);
    cost_modules.resize( ncosts );

    // check string's values
    for( int i=0 ; i<ncosts ; i++ )
    {
        string cf = lowerstring( cost_funcs[i] );
        if( cf == "nll" )
            cost_funcs[i] = "NLL";
        else if( cf == "mse" || cf == "" )
            cost_funcs[i] = "mse";
        else
            PLERROR( "StackedModulesLearner::buildOptions(): cost function\n"
                     "'%s' is unknown.\n", cost_funcs[i].c_str() );
    }

    // hessian estimation
    string h_est = lowerstring( hessian_estimation );
    if( h_est == "none" || h_est == "" )
        hessian_estimation = "none";
    else if( h_est == "diag" )
        hessian_estimation = h_est;
    else if( h_est == "simpler_diag" )
        hessian_estimation = h_est;
    else
        PLERROR( "StackedModulesLearner::buildOptions(): hessian_estimation\n"
                 "value '%s' is unknown.\n", hessian_estimation.c_str() );
}

void StackedModulesLearner::buildCostFunctions()
{
    // build cost functions
    for( int i=0 ; i<ncosts ; i++ )
    {
        string cf = cost_funcs[i];
        if( cf == "mse" )
        {
            PP<SquaredErrModule> p_mse;
            // if the first module is not already a SquaredErrModule,
            // allocate a new one
            if( !(p_mse = dynamic_cast<SquaredErrModule*>(
                    (OnlineLearningModule*) cost_modules[i] )) )
            {
                p_mse = new SquaredErrModule();
                cost_modules[i] = p_mse;
            }
        }
        else if( cf == "NLL" )
        {
            PP<NLLErrModule> p_nll;
            // if the first module is not already a NLLErrModule,
            // allocate a new one
            if( !(p_nll = dynamic_cast<NLLErrModule*>(
                    (OnlineLearningModule*) cost_modules[i] )) )
            {
                p_nll = new NLLErrModule();
                cost_modules[i] = p_nll;
            }
        }

        cost_modules[i]->input_size = outputsize();
        if( hessian_estimation == "diag" )
            cost_modules[i]->estimate_simpler_diag_hessian = false;
        else
            cost_modules[i]->estimate_simpler_diag_hessian = true;
        cost_modules[i]->build();
    }
}

void StackedModulesLearner::buildLayers()
{
    // first values will be "input" values
    int size = inputsize();
    values.resize( nmodules+1 );
    values[0].resize( size );
    gradients.resize( nmodules+1 );
    gradients[0].resize( size );
    if( hessian_estimation != "none" )
    {
        diag_hessians.resize( nmodules+1 );
        diag_hessians[0].resize( size );
    }

    for( int i=0 ; i<nmodules ; i++ )
    {
        PP<OnlineLearningModule> p_module = modules[i];
        if( p_module->input_size != size )
        {
            PLWARNING( "StackedModulesLearner::buildLayers(): module '%d'\n"
                       "has an input size of '%d', but previous layer's output"
                       " size\n"
                       "is '%d'. Resizing module '%d'.\n",
                       i, p_module->input_size, size, i);
            p_module->input_size = size;
        }

        if( hessian_estimation == "diag" )
            p_module->estimate_simpler_diag_hessian = false;
        else
            p_module->estimate_simpler_diag_hessian = true;

        p_module->build();

        size = p_module->output_size;
        values[i+1].resize( size );
        gradients[i+1].resize( size );
        if( hessian_estimation != "none" )
            diag_hessians[i+1].resize( size );
    }
}

// ### Nothing to add here, simply calls build_
void StackedModulesLearner::build()
{
    inherited::build();
    build_();
}


void StackedModulesLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    deepCopyField(modules, copies);
    deepCopyField(cost_funcs, copies);
    deepCopyField(random_gen, copies);
    deepCopyField(cost_modules, copies);
    deepCopyField(values, copies);
    deepCopyField(gradients, copies);
    deepCopyField(diag_hessians, copies);
}


int StackedModulesLearner::outputsize() const
{
    // Compute and return the size of this learner's output (which typically
    // may depend on its inputsize(), targetsize() and set options).
    if( nmodules < 0 || values.length() <= nmodules )
        return -1;
    else
        return values[ nmodules ].length();
}

void StackedModulesLearner::forget()
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

    // reset inputs
    values[0].clear();
    gradients[0].clear();
    if( hessian_estimation != "none" )
        diag_hessians[0].clear();

    // reset modules and outputs
    for( int i=0 ; i<nmodules ; i++ )
    {
        modules[i]->forget();
        values[i+1].clear();
        gradients[i+1].clear();
        if( hessian_estimation != "none" )
            diag_hessians[i+1].clear();
    }

    stage = 0;
}

void StackedModulesLearner::train()
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
    Vec train_costs( ncosts );
    Vec output( outputsize() );
    int nsamples = train_set->length();

    if( !initTrain() )
        return;

    for( ; stage < nstages ; stage++ )
    {
        // clear stats of previous epoch
        train_stats->forget();
        for( int sample=0 ; sample < nsamples ; sample++ )
        {
            train_set->getExample( sample, input, target, weight );

            // fprop
            computeOutputAndCosts(input, target, output, train_costs);
            output.append( target );

            // bprop
            Vec out_gradient(1,1); // the gradient wrt the cost is '1'
            Vec out_dh(1); // the hessian wrt the cost is '0'

            if( hessian_estimation != "none" ) // bbpropUpdate
            {
                cost_modules[0]->bbpropUpdate( output,
                                               train_costs.subVec(0,1),
                                               gradients[ nmodules ],
                                               out_gradient,
                                               diag_hessians[ nmodules ],
                                               out_dh );

                for( int i=nmodules-1 ; i>=0 ; i-- )
                    modules[i]->bbpropUpdate( values[i], values[i+1],
                                              gradients[i], gradients[i+1],
                                              diag_hessians[i],
                                              diag_hessians[i+1] );
            }
            else // bpropUpdate
            {
                cost_modules[0]->bpropUpdate( output,
                                              train_costs.subVec(0,1),
                                              gradients[ nmodules ],
                                              out_gradient );

                for( int i=nmodules-1 ; i>=0 ; i-- )
                    modules[i]->bpropUpdate( values[i], values[i+1],
                                             gradients[i], gradients[i+1] );
            }

            train_stats->update( train_costs );
        }
        train_stats->finalize(); // finalize statistics for this epoch
    }
}


void StackedModulesLearner::computeOutput(const Vec& input, Vec& output) const
{
    values[0] << input;

    // fprop
    for( int i=0 ; i<nmodules ; i++ )
        modules[i]->fprop( values[i], values[i+1] );

    output.resize( outputsize() );
    output << values[ nmodules ];
}

void StackedModulesLearner::computeCostsFromOutputs(const Vec& input,
                                                    const Vec& output,
                                                    const Vec& target,
                                                    Vec& costs) const
{
    Vec out_tgt = output.copy();
    out_tgt.append( target );
    for( int i=0 ; i<ncosts ; i++ )
    {
        Vec cost(1);
        cost_modules[i]->fprop( out_tgt, cost );
        costs[i] = cost[0];
    }
}

TVec<string> StackedModulesLearner::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    return cost_funcs;
}

TVec<string> StackedModulesLearner::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes
    // and for which it updates the VecStatsCollector train_stats
    return cost_funcs;
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
