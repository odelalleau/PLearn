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
    "That module should have ports that can be fed with the input, target\n"
    "and weight of an example (defined by the 'input_ports', 'target_ports'\n"
    "and 'weight_ports' options), ports that compute costs (defined by the\n"
    "'cost_ports' option), and a port named 'output' that computes the\n"
    "output of this learner.\n"
    "\n"
    "For example one can use a NetworkModule, which can define such ports.\n"
    "\n"
    "The input and target from the training VMatrix are plugged on their\n"
    "corresponding ports, and the output (for ComputeOutput) and cost (for\n"
    "ComputeOutputAndCost and for training) are obtained from the 'output'\n"
    "port and the ports defined by the 'cost_ports' option.\n"
    "\n"
    "During training gradient is propagated from the costs and the bpropUpdate()\n"
    "method of the module is called (possibly one mini-batch of examples at a time)\n"
    "so as to update the internal parameters of the module. During ComputeOutput,\n"
    "it is not necessary to provide a target in order to obtain an output.\n"
);

/////////////////////
// ModuleLearner //
/////////////////////
ModuleLearner::ModuleLearner():
    batch_size(1),
    cost_ports(TVec<string>(1, "cost")),
    input_ports(TVec<string>(1, "input")),
    target_ports(TVec<string>(1, "target")),
    // Note: many learners do not use weights, thus the default behavior is not
    // to have a 'weight' port in 'weight_ports'.
    mbatch_size(-1)
{
    random_gen = new PRandom();
    test_minibatch_size = 1000;
}

////////////////////
// declareOptions //
////////////////////
void ModuleLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "module", &ModuleLearner::module,
                  OptionBase::buildoption,
       "The module being optimized.");

    declareOption(ol, "batch_size", &ModuleLearner::batch_size,
                  OptionBase::buildoption,
       "User-specified number of samples fed to the network at each iteration of learning.\n"
       "Use '0' for full batch learning.");

    declareOption(ol, "cost_ports", &ModuleLearner::cost_ports,
                  OptionBase::buildoption,
       "List of ports that contain costs being optimized.");

    declareOption(ol, "input_ports", &ModuleLearner::input_ports,
                  OptionBase::buildoption,
       "List of ports that take the input part of a sample as input.");

    declareOption(ol, "target_ports", &ModuleLearner::target_ports,
                  OptionBase::buildoption,
       "List of ports that take the target part of a sample as input.");

    declareOption(ol, "weight_ports", &ModuleLearner::weight_ports,
                  OptionBase::buildoption,
       "List of ports that take the weight part of a sample as input.");

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
        module->build();
        module->forget();
    }

    // Create a new NetworkModule that connects the ports of the underlying
    // module to simple MatrixModules that will provide/store data.
    const TVec<string>& ports = module->getPorts();
    TVec< PP<OnlineLearningModule> > all_modules;
    all_modules.append(module);
    TVec< PP<NetworkConnection> > all_connections;
    store_inputs = store_targets = store_weights = NULL;

    for (int i = 0; i < input_ports.length(); i++) {
        if (!store_inputs) {
            store_inputs = new MatrixModule("store_inputs", true);
            all_modules.append(get_pointer(store_inputs));
        }
        all_connections.append(new NetworkConnection(
                    get_pointer(store_inputs), "data",
                    module, input_ports[i], false));
    }

    for (int i = 0; i < target_ports.length(); i++) {
        if (!store_targets) {
            store_targets = new MatrixModule("store_targets", true);
            all_modules.append(get_pointer(store_targets));
        }
        all_connections.append(new NetworkConnection(
                    get_pointer(store_targets), "data",
                    module, target_ports[i], false));
    }

    for (int i = 0; i < weight_ports.length(); i++) {
        if (!store_weights) {
            store_weights = new MatrixModule("store_weights", true);
            all_modules.append(get_pointer(store_weights));
        }
        all_connections.append(new NetworkConnection(
                    get_pointer(store_weights), "data",
                    module, weight_ports[i], false));
    }

    if (ports.find("output") >= 0) {
        store_outputs = new MatrixModule("store_outputs", true);
        all_modules.append(get_pointer(store_outputs));
        all_connections.append(new NetworkConnection(
                    module, "output",
                    get_pointer(store_outputs), "data", false));
    } else
        store_outputs = NULL;

    store_costs.resize(0);
    for (int i = 0; i < cost_ports.length(); i++) {
        const string& cost_port = cost_ports[i];
        PLCHECK( ports.find(cost_port) >= 0 );
        PP<MatrixModule> store = new MatrixModule("store_costs_" + tostring(i),
                                                  true);
        all_modules.append(get_pointer(store));
        // Note that these connections do propagate the gradient.
        all_connections.append(new NetworkConnection(
                    module, cost_port,
                    get_pointer(store), "data", true));
        store_costs.append(store);
    }

    network = new NetworkModule();
    network->modules = all_modules;
    network->connections = all_connections;
    network->build();

    // Initialize the list of null pointers used for forward and backward
    // propagation.
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
    deepCopyField(cost_ports,         copies);
    deepCopyField(input_ports,        copies);
    deepCopyField(target_ports,       copies);
    deepCopyField(weight_ports,       copies);
    deepCopyField(store_inputs,       copies);
    deepCopyField(store_targets,      copies);
    deepCopyField(store_weights,      copies);
    deepCopyField(store_outputs,      copies);
    deepCopyField(store_costs,        copies);
    deepCopyField(network,            copies);
    deepCopyField(null_pointers,      copies);
    deepCopyField(all_ones,           copies);
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
    for (int i = 0; i < store_costs.length(); i++)
        store_costs[i]->setGradientTo(1);

    // Backpropagation.
    network->bpropAccUpdate(null_pointers, null_pointers);
}

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void ModuleLearner::computeOutputAndCosts(const Vec& input, const Vec& target,
                                            Vec& output, Vec& costs) const
{
    if (store_inputs)
        store_inputs->setData(input.toMat(1, input.length()));
    if (store_targets)
        store_targets->setData(target.toMat(1, target.length()));
    if (store_weights) {
        all_ones.resize(1, 1);
        all_ones(0, 0) = 1;
        store_weights->setData(all_ones);
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
    costs.resize(0);
    for (int i = 0; i < store_costs.length(); i++) {
        const Mat& cost_i = store_costs[i]->getData();
        PLASSERT( cost_i.length() == 1 );
        costs.append(cost_i(0));
    }
}

///////////////////////////
// computeOutputsAndCosts //
///////////////////////////
void ModuleLearner::computeOutputsAndCosts(const Mat& input, const Mat& target,
                                           Mat& output, Mat& costs) const
{
    static Mat one;
    if (store_inputs)
        store_inputs->setData(input);
    if (store_targets)
        store_targets->setData(target);
    if (store_weights) {
        if (all_ones.width() != 1 || all_ones.length() != input.length()) {
            all_ones.resize(input.length(), 1);
            all_ones.fill(1.0);
        }
        store_weights->setData(all_ones);
    }
    PLASSERT( store_outputs );
    // make the store_output temporarily point to output
    Mat& net_out = store_outputs->getData();
    Mat old_net_out = net_out;
    output.resize(input.length(),outputsize());
    net_out = output;

    // Forward propagation.
    network->fprop(null_pointers);

    // Restore output_store.
    net_out = old_net_out;

    // Copy costs.
    // Note that a more efficient implementation may be done when only one cost
    // is computed (see code in previous version).
    // First compute total size.
    int cost_size = 0;
    for (int i = 0; i < store_costs.length(); i++)
        cost_size += store_costs[i]->getData().width();
    // Then resize the 'costs' matrix and fill it.
    costs.resize(input.length(), cost_size);
    int cost_idx = 0;
    for (int i = 0; i < store_costs.length(); i++) {
        const Mat& cost_i = store_costs[i]->getData();
        PLASSERT( cost_i.length() == costs.length() );
        costs.subMatColumns(cost_idx, cost_i.width()) << cost_i;
        cost_idx += cost_i.width();
    }
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
    TVec<string> cost_names;
    for (int i = 0; i < cost_ports.length(); i++)
        cost_names.append(module->getPortDescription(cost_ports[i]));
    return cost_names;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> ModuleLearner::getTrainCostNames() const
{
    // No training cost is currently computed.
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
