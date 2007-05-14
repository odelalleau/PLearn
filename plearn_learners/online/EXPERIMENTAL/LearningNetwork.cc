// -*- C++ -*-

// LearningNetwork.cc
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

/*! \file LearningNetwork.cc */


#define PL_LOG_MODULE_NAME "LearningNetwork"
#include <plearn/io/pl_log.h>

#include "LearningNetwork.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    LearningNetwork,
    "Flexible network structure that can be optimized to learn some task(s).",
    "This network is made of several blocks, called 'modules' (deriving from\n"
    "the OnlineLearningModule class), connected together so as to be able to\n"
    "propagate information through the network.\n"
    "Typically, during training, (input, target) pairs are fed to the\n"
    "network and a cost is optimized. The trained network can later be used\n"
    "to make predictions on new test points.\n"
);

/////////////////////
// LearningNetwork //
/////////////////////
LearningNetwork::LearningNetwork():
    batch_size(1),
    mbatch_size(-1)
{
    random_gen = new PRandom();
}

////////////////////
// declareOptions //
////////////////////
void LearningNetwork::declareOptions(OptionList& ol)
{
    declareOption(ol, "modules", &LearningNetwork::modules,
                  OptionBase::buildoption,
       "List of modules contained in the network.");

    declareOption(ol, "connections", &LearningNetwork::connections,
                  OptionBase::buildoption,
       "List of connections between modules.");

    declareOption(ol, "input_module", &LearningNetwork::input_module,
                  OptionBase::buildoption,
        "Module that uses the samples' inputs.");

    declareOption(ol, "input_port", &LearningNetwork::input_port,
                  OptionBase::buildoption,
       "Port of 'input_module' that is filled with the samples' inputs.");

    declareOption(ol, "target_module", &LearningNetwork::target_module,
                  OptionBase::buildoption,
        "Module that uses the samples' targets.");

    declareOption(ol, "target_port", &LearningNetwork::target_port,
                  OptionBase::buildoption,
       "Port of 'target_module' that is filled with the samples' targets.");

    declareOption(ol, "weight_module", &LearningNetwork::weight_module,
                  OptionBase::buildoption,
        "Module that uses the samples' weights.");

    declareOption(ol, "weight_port", &LearningNetwork::weight_port,
                  OptionBase::buildoption,
       "Port of 'weight_module' that is filled with the samples' weights.");

    declareOption(ol, "output_module", &LearningNetwork::output_module,
                  OptionBase::buildoption,
        "Module that computes the network's output.");

    declareOption(ol, "output_port", &LearningNetwork::output_port,
                  OptionBase::buildoption,
       "Port of 'output_module' that contains the network's output");

    declareOption(ol, "cost_module", &LearningNetwork::cost_module,
                  OptionBase::buildoption,
        "Module that computes the network's cost.");

    declareOption(ol, "cost_port", &LearningNetwork::cost_port,
                  OptionBase::buildoption,
       "Port of 'cost_module' that contains the network's cost");

    declareOption(ol, "batch_size", &LearningNetwork::batch_size,
                  OptionBase::buildoption,
       "Number of samples fed to the network at each iteration of learning.\n"
       "Use '0' for full batch learning.");

    declareOption(ol, "mbatch_size", &LearningNetwork::mbatch_size,
                  OptionBase::learntoption,
       "Same as 'batch_size', except when 'batch_size' is set to 0, this\n"
       "option takes the value of the size of the training set.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void LearningNetwork::build_()
{
    // Forward random number generator to underlying modules.
    for (int i = 0; i < modules.length(); i++)
        if (!modules[i]->random_gen) {
            modules[i]->random_gen = random_gen;
            // Currently we call forget, since it ensures the module is
            // correctly initialized and also will propagate the random number
            // generator to its own sub-modules. However, this is not very
            // intuitive, and a better solution may be found.
            modules[i]->forget();
        }

    // Initialize 'all_modules' and 'all_connections' with copies of
    // respectively 'modules' and 'connections'.
    all_modules.resize(modules.length());
    all_modules << modules;
    all_connections.resize(connections.length());
    all_connections << connections;
    
    // Add connections corresponding to the input, target, weight, output and
    // cost data.
    if (input_module) {
        store_inputs = new MatrixModule(true);
        all_modules.append(get_pointer(store_inputs));
        all_connections.append(new NetworkConnection(
                    get_pointer(store_inputs), "data",
                    input_module, input_port, false));
    }
    if (target_module) {
        store_targets = new MatrixModule(true);
        all_modules.append(get_pointer(store_targets));
        all_connections.append(new NetworkConnection(
                    get_pointer(store_targets), "data",
                    target_module, target_port, false));
    }
    if (weight_module) {
        store_weights = new MatrixModule(true);
        all_modules.append(get_pointer(store_weights));
        all_connections.append(new NetworkConnection(
                    get_pointer(store_weights), "data",
                    weight_module, weight_port, false));
    }
    if (output_module) {
        store_outputs = new MatrixModule(true);
        all_modules.append(get_pointer(store_outputs));
        all_connections.append(new NetworkConnection(
                    output_module, output_port,
                    get_pointer(store_outputs), "data", false));
    }
    if (cost_module) {
        store_costs = new MatrixModule(true);
        all_modules.append(get_pointer(store_costs));
        all_connections.append(new NetworkConnection(
                    cost_module, cost_port,
                    get_pointer(store_costs), "data", true));
    }

    // Construct fprop and bprop paths from the list of modules and
    // connections.
    // First preprocess some convenience data structures from the list of
    // connections.
    // 'module_to_index' maps module pointers to their corresponding index in
    // the 'all_modules' list.
    map<const OnlineLearningModule*, int> module_to_index;
    for (int i = 0; i < all_modules.length(); i++)
        module_to_index[all_modules[i]] = i;
    // The i-th element of 'in_connections' maps each port in the i-th module
    // to the connection that has it as destination (there may be only one).
    TVec< map<string, PP<NetworkConnection> > > in_connections;
    in_connections.resize(all_modules.length());
    // The i-th element of 'out_connections' maps each port in the i-th module
    // to the connections that have it as source (there may be many).
    TVec< map<string, TVec< PP<NetworkConnection> > > > out_connections;
    out_connections.resize(all_modules.length());
    // The 'inputs_needed' vector contains the number of inputs that must be
    // fed to a module before it can compute a fprop.
    TVec<int> inputs_needed(all_modules.length(), 0);
    // The 'compute_input_of' list gives, for each module M, the indices of
    // other modules that take an output of M as input.
    TVec< TVec<int> > compute_input_of(all_modules.length());
    for (int i = 0; i < all_connections.length(); i++) {
        PP<NetworkConnection> connection = all_connections[i];
        int src = module_to_index[connection->src_module];
        int dest = module_to_index[connection->dest_module];
        inputs_needed[dest]++;
        compute_input_of[src].append(dest);
        map<string, PP<NetworkConnection> >& in_conn = in_connections[dest];
        if (in_conn.find(connection->dest_port) != in_conn.end())
            PLERROR("In LearningNetwork::build_ - A port may have only one "
                    "incoming connection");
        in_conn[connection->dest_port] = connection;
        out_connections[src][connection->src_port].append(connection);
    }

    // The fprop and bprop paths can now be computed.
    fprop_path.resize(0);
    bprop_path.resize(all_modules.length());
    bprop_path.fill(-1);
    TVec<bool> is_done(all_modules.length(), false);
    fprop_data.resize(0);
    bprop_data.resize(all_modules.length());
    all_mats.resize(0);
    fprop_toresize.resize(0);
    bprop_toresize.resize(all_modules.length());
    // A vector that stores the index of a module in the fprop path.
    TVec<int> module_index_to_path_index(all_modules.length(), -1);
    while (is_done.find(false) >= 0) {
        for (int i = 0; i < all_modules.length(); i++) {
            if (!is_done[i] && inputs_needed[i] == 0) {
                for (int j = 0; j < compute_input_of[i].length(); j++)
                    inputs_needed[compute_input_of[i][j]]--;
                // Compute the list of matrices that must be provided to this
                // module when doing a fprop and bprop.
                TVec<string> ports = all_modules[i]->getPorts();
                map<string, PP<NetworkConnection> >& in_conn =
                    in_connections[i];
                map<string, TVec< PP<NetworkConnection> > >& out_conn =
                    out_connections[i];
                TVec<int> fprop_tores;
                TVec<int> bprop_tores;
                TVec<Mat*> fprop_mats;
                TVec<Mat*> bprop_mats;
                for (int j = 0; j < ports.length(); j++) {
                    if (in_conn.find(ports[j]) != in_conn.end()) {
                        // This port has an incoming connection: it is thus an
                        // input, and the corresponding matrices for storing
                        // its value and gradient are found by looking at the
                        // source port of the connection.
                        PP<NetworkConnection> conn = in_conn[ports[j]];
                        int src_mod = module_to_index[conn->src_module];
                        int path_index = module_index_to_path_index[src_mod];
                        int port_index = conn->src_module->getPortIndex(
                                conn->src_port);
                        fprop_mats.append(fprop_data[path_index][port_index]);
                        if (!conn->propagate_gradient)
                            // This connection does not propagate the gradient,
                            // and thus we do not want to accumulate it.
                            bprop_mats.append(NULL);
                        else {
                            int b_idx = all_modules.length() - 1 - path_index;
                            bprop_mats.append(bprop_data[b_idx][port_index]);
                            bprop_tores.append(j);
                        }
                        PLASSERT( out_conn.find(ports[j]) == out_conn.end() );
                    } else if (out_conn.find(ports[j]) != out_conn.end()) {
                        // This port has (at least) one outgoing connection: it
                        // is thus an output, and it must be provided with
                        // matrices to store its value (and gradient if the
                        // connection propagates it).
                        all_mats.append(Mat());
                        Mat* new_mat = &all_mats.lastElement();
                        fprop_mats.append(new_mat);
                        fprop_tores.append(j);
                        // Ensure there exists a connection propagating the
                        // gradient to this port.
                        bool must_store_grad = false;
                        const TVec< PP<NetworkConnection> >& out_j =
                            out_conn[ports[j]];
                        for (int k = 0; k < out_j.length(); k++)
                            if (out_j[k]->propagate_gradient) {
                                must_store_grad = true;
                                break;
                            }
                        if (must_store_grad) {
                            all_mats.append(Mat());
                            new_mat = &all_mats.lastElement();
                            bprop_mats.append(new_mat);
                        } else
                            // No connection propagating gradient to this port.
                            bprop_mats.append(NULL);
                    } else {
                        // This port is not used (we do not provide its value,
                        // and we do not care about obtaining it).
                        fprop_mats.append(NULL);
                        bprop_mats.append(NULL);
                    }
                }
                module_index_to_path_index[i] = fprop_path.length();
                // Update fprop path.
                fprop_data.append(fprop_mats);
                fprop_toresize.append(fprop_tores);
                fprop_path.append(i);
                // Update bprop path.
                int bprop_idx = bprop_path.length() - fprop_path.length();
                bprop_data[bprop_idx] = bprop_mats;
                bprop_toresize[bprop_idx] = bprop_tores;
                bprop_path[bprop_idx] = i;

                is_done[i] = true;
            }
        }
    }
    PLASSERT( module_index_to_path_index.find(-1) == -1 );
}

///////////
// build //
///////////
void LearningNetwork::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LearningNetwork::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(output_module, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("LearningNetwork::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////
// outputsize //
////////////////
int LearningNetwork::outputsize() const
{
    PLASSERT( output_module );
    return output_module->getPortWidth(output_port);
}

////////////
// forget //
////////////
void LearningNetwork::forget()
{
    inherited::forget();

    // All modules should forget too.
    for (int i = 0; i < modules.length(); i++)
        modules[i]->forget();

    mbatch_size = -1;
}

///////////
// train //
///////////
void LearningNetwork::train()
{
    if (!initTrain())
        return;

    if (stage == 0) {
        // Perform training set-dependent initialization here.
        if (batch_size == 0)
            mbatch_size = train_set->length();
        else
            mbatch_size = batch_size;
        if (train_set->weightsize() >= 1 && !weight_module)
            PLWARNING("In LearningNetwork::train - The training set contains "
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
        PLWARNING("In LearningNetwork::train - The network was trained for "
                "only %d stages (instead of nstages = %d, which is not a "
                "multiple of batch_size = %d", stage, nstages, batch_size);
}

//////////////////
// trainingStep //
//////////////////
void LearningNetwork::trainingStep(const Mat& inputs, const Mat& targets,
                      const Vec& weights)
{
    // Fill in the provided batch values (only if they are actually used by the
    // network).
    if (input_module)
        store_inputs->setData(inputs);
    if (target_module)
        store_targets->setData(targets);
    if (weight_module)
        store_weights->setData(weights.toMat(weights.length(), 1));

    // Propagate up.
    for (int i = 0; i < fprop_path.length(); i++) {
        PP<OnlineLearningModule> module = all_modules[fprop_path[i]];
        DBG_MODULE_LOG << "FPROP: " << module->classname() << endl;
        // First resize some data matrices, so that the outputs are properly
        // computed.
        const TVec<int>& toresize = fprop_toresize[i];
        for (int j = 0; j < toresize.length(); j++) {
            DBG_MODULE_LOG << "  out = " << module->getPortName(toresize[j])
                           << endl;
            fprop_data[i][toresize[j]]->resize(0, 0);
        }
        module->fprop(fprop_data[i]);
    }

    // Clear gradients.
    PLASSERT( fprop_path.length() == bprop_path.length() );
    for (int i = 0; i < bprop_path.length(); i++) {
        const TVec<int>& toresize = bprop_toresize[i];
        const TVec<Mat*>& f_data = fprop_data[fprop_data.length() - 1 - i];
        for (int j = 0; j < toresize.length(); j++) {
            if (j == 0) {
                DBG_MODULE_LOG << "CLEAR: " <<
                    all_modules[bprop_path[i]]->classname() << endl;
            }
            int mat_idx = toresize[j];
            DBG_MODULE_LOG << "  grad = " <<
                all_modules[bprop_path[i]]->getPortName(mat_idx) << endl;
            Mat* mat_toresize = bprop_data[i][mat_idx];
            Mat* mat_tpl = f_data[mat_idx];
            mat_toresize->resize(mat_tpl->length(), mat_tpl->width());
            mat_toresize->fill(0);
        }
    }

    // Backpropagate gradient to optimize parameters.
    for (int i = 0; i < bprop_path.length(); i++) {
        PP<OnlineLearningModule> module = all_modules[bprop_path[i]];
        DBG_MODULE_LOG << "BPROP: " << module->classname() << endl;
        // First resize some gradient matrices, so that the gradient is
        // properly computed.
        const TVec<int>& toresize = bprop_toresize[i];
        for (int j = 0; j < toresize.length(); j++) {
            int mat_idx = toresize[j];
            DBG_MODULE_LOG << "  grad = " << module->getPortName(mat_idx)
                           << endl;
            Mat* mat_toresize = bprop_data[i][mat_idx];
            PLASSERT( mat_toresize->width() > 0 );
            mat_toresize->resize(0, mat_toresize->width());
        }
        // Then perform the bpropUpdate step.
        module->bpropAccUpdate(
                fprop_data[fprop_data.length() - 1 - i], bprop_data[i]);
    }
}

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void LearningNetwork::computeOutputAndCosts(const Vec& input, const Vec& target,
                                            Vec& output, Vec& costs) const
{
    if (input_module)
        store_inputs->setData(input.toMat(1, input.length()));
    if (target_module)
        store_targets->setData(target.toMat(1, target.length()));
    if (weight_module)
        // Should fill store_weights with 1, but this is not a priority.
        PLERROR("In LearningNetwork::computeOutputAndCosts - Not implemented "
                "with 'weight_module'");

    // Propagate up.
    // TODO Code duplicated with code in train. This is bad!
    for (int i = 0; i < fprop_path.length(); i++) {
        PP<OnlineLearningModule> module = all_modules[fprop_path[i]];
        DBG_MODULE_LOG << "FPROP: " << module->classname() << endl;
        // First resize some data matrices, so that the outputs are properly
        // computed.
        const TVec<int>& toresize = fprop_toresize[i];
        for (int j = 0; j < toresize.length(); j++) {
            DBG_MODULE_LOG << "  out = " << module->getPortName(toresize[j])
                           << endl;
            fprop_data[i][toresize[j]]->resize(0, 0);
        }
        module->fprop(fprop_data[i]);
    }

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
void LearningNetwork::computeOutput(const Vec& input, Vec& output) const
{
    // Unefficient implementation.
    Vec target(targetsize(), MISSING_VALUE);
    Vec costs;
    computeOutputAndCosts(input, target, output, costs);
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void LearningNetwork::computeCostsFromOutputs(const Vec& input, const Vec& output,
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
TVec<string> LearningNetwork::getTestCostNames() const
{
    if (!cost_module)
        return TVec<string>();
    else
        return cost_module->getPortDescription(cost_port);
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> LearningNetwork::getTrainCostNames() const
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
