// -*- C++ -*-

// ModuleLearner.cc
//
// Copyright (C) 2007 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file ModuleLearner.cc */


#define PL_LOG_MODULE_NAME "ModuleLearner"
#include <plearn/io/pl_log.h>

#include "ModuleLearner.h"
#include <plearn_learners/online/NullModule.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ModuleLearner,
    "A PLearner that contains a single OnlineLearningModule.\n",
    "That module should have ports named 'input', 'target', 'weight', 'output' and 'cost'.\n"
    "For example one can use a NetworkModule, which has such ports.\n"
    "The input and target from the training VMatrix are plugged on the 'input' and 'target'\n"
    "ports, and the output (for ComputeOutput) and cost (for ComputeOutputAndCost and\n"
    "for training) are obtained from the 'output' and 'cost' ports.\n"
    "During training gradient is propagated from the cost and the bpropUpdate()\n"
    "method of the module is called (possibly one mini-batch of examples at a time)\n"
    "so as to update the internal parameters of the module. During ComputeOutput,\n"
    "it is not necessary to provide a target in order to obtain an output.\n"
);

/////////////////////
// ModuleLearner //
/////////////////////
ModuleLearner::ModuleLearner():
    batch_size(1),
    mbatch_size(-1)
{
    random_gen = new PRandom();
}

////////////////////
// declareOptions //
////////////////////
void ModuleLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "module", &ModuleLearner::module,
                  OptionBase::buildoption,
       "The module being optimized. This module should typically have some\n"
       "ports named 'input', 'target', 'weight', 'output' and 'cost'.");

    declareOption(ol, "batch_size", &ModuleLearner::batch_size,
                  OptionBase::buildoption,
       "User-specified number of samples fed to the network at each iteration of learning.\n"
       "Use '0' for full batch learning.");

    declareOption(ol, "mbatch_size", &ModuleLearner::mbatch_size,
                  OptionBase::learntoption,
       "Effective 'batch_size': it takes the same value as 'batch_size'\n"
       "except when 'batch_size' is set to 0, and this\n"
       "option takes the value of the size of the training set.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void ModuleLearner::build_()
{
    if (!module)
        // Cannot do anything without an underlying module.
        return;

    // Forward random number generator to underlying module.
    if (!module->random_gen) {
        module->random_gen = random_gen;
        // Currently we call forget, since it ensures the module is
        // correctly initialized and also will propagate the random number
        // generator to its own sub-modules. However, this is not very
        // intuitive, and a better solution may be found.
        module->forget();
    }

    // Create a new NetworkModule that connects the ports of the underlying
    // module to simple MatrixModules that will provide/store data.
    const TVec<string>& ports = module->getPorts();
    TVec< PP<OnlineLearningModule> > all_modules;
    all_modules.append(module);
    TVec< PP<NetworkConnection> > all_connections;

    if (ports.find("input") >= 0) {
        store_inputs = new MatrixModule("store_inputs", true);
        all_modules.append(get_pointer(store_inputs));
        all_connections.append(new NetworkConnection(
                    get_pointer(store_inputs), "data",
                    module, "input", false));
    } else
        store_inputs = NULL;

    if (ports.find("target") >= 0) {
        store_targets = new MatrixModule("store_targets", true);
        all_modules.append(get_pointer(store_targets));
        all_connections.append(new NetworkConnection(
                    get_pointer(store_targets), "data",
                    module, "target", false));
    } else
        store_targets = NULL;

    if (ports.find("weight") >= 0) {
        store_weights = new MatrixModule("store_weights", true);
        all_modules.append(get_pointer(store_weights));
        all_connections.append(new NetworkConnection(
                    get_pointer(store_weights), "data",
                    module, "weight", false));
    } else
        store_weights = NULL;

    if (ports.find("output") >= 0) {
        store_outputs = new MatrixModule("store_outputs", true);
        all_modules.append(get_pointer(store_outputs));
        all_connections.append(new NetworkConnection(
                    module, "output",
                    get_pointer(store_outputs), "data", false));
    } else
        store_outputs = NULL;

    if (ports.find("cost") >= 0) {
        store_costs = new MatrixModule("store_costs", true);
        all_modules.append(get_pointer(store_costs));
        // Note that this is the only connection that propagates the gradient.
        all_connections.append(new NetworkConnection(
                    module, "cost",
                    get_pointer(store_costs), "data", true));
    } else
        store_costs = NULL;

    network = new NetworkModule();
    network->modules = all_modules;
    network->connections = all_connections;
    network->build();

    // Initialize the list of null pointers to provided for forward and
    // backward propagation.
    null_pointers.resize(module->nPorts());
    null_pointers.fill(NULL);
}

///////////
// build //
///////////
void ModuleLearner::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ModuleLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(module,             copies);
    deepCopyField(store_inputs,       copies);
    deepCopyField(store_targets,      copies);
    deepCopyField(store_weights,      copies);
    deepCopyField(store_outputs,      copies);
    deepCopyField(store_costs,        copies);
    deepCopyField(network,            copies);
    // does not compile right now
    // deepCopyField(null_pointers,      copies);
}

////////////////
// outputsize //
////////////////
int ModuleLearner::outputsize() const
{
    PLASSERT( module && store_outputs );
    return module->getPortWidth("output");
}

////////////
// forget //
////////////
void ModuleLearner::forget()
{
    inherited::forget();

    if (module)
        module->forget();

    mbatch_size = -1;
}

///////////
// train //
///////////
void ModuleLearner::train()
{
    if (!initTrain())
        return;

    if (stage == 0) {
        // Perform training set-dependent initialization here.
        if (batch_size == 0)
            mbatch_size = train_set->length();
        else
            mbatch_size = batch_size;
        if (train_set->weightsize() >= 1 && !store_weights)
            PLWARNING("In ModuleLearner::train - The training set contains "
                    "weights, but the network is not using them");
    }

    Mat inputs, targets;
    Vec weights;
    PP<ProgressBar> pb = NULL;

    int stage_init = stage;
    if (report_progress)
        pb = new ProgressBar( "Training " + classname(), nstages - stage);

    while (stage + mbatch_size <= nstages) {
        // Obtain training samples.
        int sample_start = stage % train_set->length();
        train_set->getExamples(sample_start, mbatch_size, inputs, targets,
                weights, NULL, true);
        // Perform a training step.
        trainingStep(inputs, targets, weights);
        // Handle training progress.
        stage += mbatch_size;
        if (report_progress)
            pb->update(stage - stage_init);
    }
    if (stage != nstages)
        PLWARNING("In ModuleLearner::train - The network was trained for "
                "only %d stages (instead of nstages = %d, which is not a "
                "multiple of batch_size = %d", stage, nstages, batch_size);
}

//////////////////
// trainingStep //
//////////////////
void ModuleLearner::trainingStep(const Mat& inputs, const Mat& targets,
                      const Vec& weights)
{
    // Fill in the provided batch values (only if they are actually used by the
    // network).
    if (store_inputs)
        store_inputs->setData(inputs);
    if (store_targets)
        store_targets->setData(targets);
    if (store_weights)
        store_weights->setData(weights.toMat(weights.length(), 1));

    // Forward propagation.
    network->fprop(null_pointers);

    // Initialize cost gradients to 1.
    // Note that we may not need to re-do it at every iteration, but this is so
    // cheap it should not impact performance.
    if (store_costs)
        store_costs->setGradientTo(1);

    // Backpropagation.
    network->bpropAccUpdate(null_pointers, null_pointers);
}

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void ModuleLearner::computeOutputAndCosts(const Vec& input, const Vec& target,
                                            Vec& output, Vec& costs) const
{
    static Mat one;
    if (store_inputs)
        store_inputs->setData(input.toMat(1, input.length()));
    if (store_targets)
        store_targets->setData(target.toMat(1, target.length()));
    if (store_weights) {
        if (one.isEmpty()) {
            one.resize(1, 1);
            one(0, 0) = 1;
        }
        store_weights->setData(one);
    }

    // Forward propagation.
    network->fprop(null_pointers);

    // Store output.
    PLASSERT( store_outputs );
    const Mat& net_out = store_outputs->getData();
    PLASSERT( net_out.length() == 1 );
    output.resize(net_out.width());
    output << net_out;

    // Store costs.
    PLASSERT( store_costs );
    const Mat& net_cost = store_costs->getData();
    PLASSERT( net_cost.length() == 1 );
    costs.resize(net_cost.width());
    costs << net_cost;
}

///////////////////
// computeOutput //
///////////////////
void ModuleLearner::computeOutput(const Vec& input, Vec& output) const
{
    // Unefficient implementation.
    Vec target(targetsize(), MISSING_VALUE);
    Vec costs;
    computeOutputAndCosts(input, target, output, costs);
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void ModuleLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    // Unefficient implementation (recompute the output too).
    Vec the_output;
    computeOutputAndCosts(input, target, the_output, costs);
#ifdef BOUNDCHECK
    // Ensure the computed output is the same as the one provided in this
    // method.
    PLASSERT( output.length() == the_output.length() );
    for (int i = 0; i < output.length(); i++) {
        PLASSERT( fast_exact_is_equal(output[i], the_output[i]) );
    }
#endif
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> ModuleLearner::getTestCostNames() const
{
    if (!store_costs)
        return TVec<string>();
    else
        return module->getPortDescription("cost");
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> ModuleLearner::getTrainCostNames() const
{
    // No training cost computed.
    return TVec<string>();
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
