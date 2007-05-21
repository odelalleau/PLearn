// -*- C++ -*-

// NetworkModule.cc
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

/*! \file NetworkModule.cc */



#include "NetworkModule.h"
#include <plearn_learners/online/NullModule.h>

#define PL_LOG_MODULE_NAME "NetworkModule"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NetworkModule,
    "A module that encapsulates a whole network of underlying modules.",
    "The network is defined by the list of modules, and the list of\n"
    "connections between these modules.\n"
    "The network's ports are given through the 'ports' option. A typical\n"
    "value for 'ports' would be for instance something like:\n"
    "   [ \"input\"  \"rbm.visible\"\n"
    "     \"target\" \"nll.target\"\n"
    "     \"output\" \"rbm.hidden\"\n"
    "     \"cost\"   \"nll.cost\"\n"
    "   ]\n"
    "which means this module has four ports (input, target, output and cost)\n"
    "which redirect to specific ports of modules in the network.\n"
);

///////////////////
// NetworkModule //
///////////////////
NetworkModule::NetworkModule():
    save_states(true)
{}

////////////////////
// declareOptions //
////////////////////
void NetworkModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "modules", &NetworkModule::modules,
                  OptionBase::buildoption,
       "List of modules contained in the network.");

    declareOption(ol, "connections", &NetworkModule::connections,
                  OptionBase::buildoption,
       "List of connections between modules.");

    declareOption(ol, "ports", &NetworkModule::ports,
                  OptionBase::buildoption,
       "A sequence of pairs of strings, where each pair is of the form\n"
       "('P', 'M.N') with 'M' the name of an underlying module, 'N' one of\n"
       "its ports, and 'P' the name under which the NetworkModule sees this\n"
       "port. See the class help for an example with the correct syntax.");

    declareOption(ol, "save_states", &NetworkModule::save_states,
                  OptionBase::buildoption,
       "If set to 1, then any port of an underlying module whose name ends\n"
       "with 'state' will be automatically saved, even if it has no incoming\n"
       "or outgoing connection.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Hide unused parent options.

    redeclareOption(ol, "input_size", &NetworkModule::input_size,
                    OptionBase::nosave,
                    "Not used.");

    redeclareOption(ol, "output_size", &NetworkModule::output_size,
                    OptionBase::nosave,
                    "Not used.");

}

////////////////////
// bpropAccUpdate //
////////////////////
void NetworkModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                   const TVec<Mat*>& ports_gradient)
{

    // Reset internal gradients, either:
    // - to zero if no additional gradient is provided in 'ports_gradient'
    // - to the gradient provided in 'ports_gradient' otherwise
    PLASSERT( fprop_path.length() == bprop_path.length() );
    for (int i = 0; i < bprop_path.length(); i++) {
        const TVec<int>& toresize = bprop_toresize[i];
        int fprop_idx = fprop_data.length() - 1 - i;
        const TVec<Mat*>& f_data = fprop_data[fprop_idx];
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
        const TMat<int>& f_toplug = fprop_toplug[fprop_idx];
        for (int j = 0; j < f_toplug.length(); j++) {
            int mod_idx = f_toplug(j, 1);
            Mat* current = bprop_data[i][mod_idx];
            if (!current)
                // There is no gradient currently computed for this port, thus
                // nothing to initialize.
                continue;
            int this_idx = f_toplug(j, 0);
            Mat* provided = ports_gradient[this_idx];
            if (provided) {
                // We are provided with some additional gradient, either as
                // input or output. If it is an output, there is nothing more
                // to do. However, if it is an input, we must use it to
                // initialize the gradient.
                if (!provided->isEmpty()) {
                    PLASSERT( current->length() == provided->length() &&
                            current->width() == provided->width() );
                    PLASSERT( (*current)(0, 0) == 0 );
                    *current << *provided;
                } else {
                    // Just for safety reason, crash here. It seems weird that
                    // we may want to accumulate the gradient w.r.t. a port for
                    // which a gradient is already computed internally (i.e. is
                    // used in the bprop). If we want to allow this, care
                    // should be taken as to what to do with the current value
                    // of the gradient in 'provided' (in particular, do we use
                    // it to update the internal paremeters?)
                    PLASSERT( false );
                }
            }
        }
    }

    // Backpropagate gradient and update parameters.
    for (int i = 0; i < bprop_path.length(); i++) {
        PP<OnlineLearningModule> module = all_modules[bprop_path[i]];
        DBG_MODULE_LOG << "BPROP: " << module->name << endl;
        // First resize some gradient matrices, so that the gradient is
        // properly computed.
        const TVec<int>& toresize = bprop_toresize[i];
        const TVec<Mat*> b_data = bprop_data[i];
        for (int j = 0; j < toresize.length(); j++) {
            int mat_idx = toresize[j];
            DBG_MODULE_LOG << "  grad = " << module->getPortName(mat_idx)
                           << endl;
            Mat* mat_toresize = b_data[mat_idx];
            PLASSERT( mat_toresize->width() > 0 );
            mat_toresize->resize(0, mat_toresize->width());
        }

        // Plug in the matrices provided as parameters of this method.
        int fprop_idx = fprop_data.length() - 1 - i;
        const TVec<Mat*> f_data = fprop_data[fprop_idx];
        const TMat<int>& f_toplug = fprop_toplug[fprop_idx];
        for (int j = 0; j < f_toplug.length(); j++) {
            int this_idx = f_toplug(j, 0);
            int mod_idx = f_toplug(j, 1);
            Mat* provided = ports_gradient[this_idx];
            if (provided) {
                Mat* current = b_data[mod_idx];
                if (!current) {
                    // We can directly plug in the provided matrix.
                    b_data[mod_idx] = provided;
                } else {
                    if (!provided->isEmpty()) {
                        PLASSERT( !current->isEmpty() );
                        // This gradient is some external gradient we are
                        // provided with. It has already been added in the step
                        // where we reset (see above), thus there is nothing
                        // more to do.  TODO Note that it may be cleaner to
                        // actually add it here to simplify the code above.
                    } else {
                        PLASSERT( current->isEmpty() );
                        // This gradient must be computed, but is already
                        // computed somewhere here. We will copy it after the
                        // update step.
                        PLASSERT( false); // This should not happen (cf. above)
                    }
                }
            }
            // Same for the fprop data since it is also given in argument.
            Mat* f_provided = ports_value[this_idx];
            if (f_provided) {
                Mat* f_current = f_data[mod_idx];
                if (!f_current)
                    f_data[mod_idx] = f_provided;
                else {
                    // 'f_provided' is thus the result of the computation, that
                    // was copied from the value of 'f_current' at fprop time.
                    // We must fill in 'f_current' with this value (once again,
                    // this is not best for efficiency).
                    f_current->resize(f_provided->length(),
                                      f_provided->width());
                    *f_current << *f_provided;
                }
            }
        }

        // Then perform the bpropUpdate step.
        module->bpropAccUpdate(
                fprop_data[fprop_data.length() - 1 - i], bprop_data[i]);

        // Restore the 'bprop_data' and 'fprop_data' parameters.
        for (int j = 0; j < f_toplug.length(); j++) {
            int this_idx = f_toplug(j, 0);
            int mod_idx = f_toplug(j, 1);
            Mat* provided = ports_gradient[this_idx];
            if (provided) {
                Mat* current = b_data[mod_idx];
                PLASSERT_MSG(current, "This should not happen: if we are "
                        "provided with a matrix, it should have been plugged "
                        "in here");
                if (provided == current)
                    // They are the same matrices: this means there used to be
                    // a NULL matrix before.
                    b_data[mod_idx] = NULL;
                else {
                    // This should never happen.
                    PLASSERT( false );
                }
            }
            Mat* f_provided = ports_value[this_idx];
            if (f_provided) {
                Mat* f_current = f_data[mod_idx];
                if (f_current == f_provided)
                    // This used to be a null pointer, that must be restored.
                    f_data[mod_idx] = NULL;
            }
        }
    }
}


///////////
// build //
///////////
void NetworkModule::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void NetworkModule::build_()
{
    // Initialize the 'all_modules' and 'all_connections' lists.
    all_modules.resize(modules.length());
    all_modules << modules;
    all_connections.resize(connections.length());
    all_connections << connections;
    if (save_states) {
        // Ensure all ports which are states are correctly saved.
        // This is done by adding connections to a NullModule in order to force
        // them being saved.
        for (int i = 0; i < modules.length(); i++) {
            const TVec<string>& mod_ports = modules[i]->getPorts();
            for (int j = 0; j < mod_ports.length(); j++) {
                const string& port = mod_ports[j];
                if (port.size() >= 5 &&
                    port.substr(port.size() - 5) == "state")
                {
                    // Look for either an incoming or outgoing connection.
                    string port_name = modules[i]->name + "." + port;
                    bool has_a_connection = false;
                    for (int k = 0; k < connections.length(); k++)
                        if (connections[k]->destination == port_name ||
                            connections[k]->source == port_name)
                        {
                            has_a_connection = true;
                            break;
                        }
                    if (!has_a_connection) {
                        PP<OnlineLearningModule> null;
                        if (all_modules.length() == modules.length()) {
                            // No null module added yet.
                            null = new NullModule("_null_");
                            all_modules.append(null);
                        } else {
                            PLASSERT( all_modules.length() == 
                                        modules.length() + 1 );
                            null = all_modules.lastElement();
                        }
                        all_connections.append( new NetworkConnection(
                                    modules[i], port, null, "_null_", false));
                    }
                }
            }
        }
    }
    
    // Construct fprop and bprop paths from the list of modules and
    // connections.

    // First create a dictionary of modules.
    map<string, PP<OnlineLearningModule> > name_to_module;
    for (int i = 0; i < all_modules.length(); i++) {
        string module = all_modules[i]->name;
        if (name_to_module.find(module) != name_to_module.end()) {
            // There is already a module with the same name. For safety
            // reasons, we make it point to a NULL pointer to ensure we do not
            // accidentally use the wrong module.
            name_to_module[module] = NULL;
        } else
            name_to_module[module] = all_modules[i];
    }

    // Initialize connections.
    for (int i = 0; i < all_connections.length(); i++)
        all_connections[i]->initialize(name_to_module);

    // Preprocess some convenience data structures from the list of
    // connections.

    // 'module_to_index' maps module pointers to their corresponding index in
    // the 'all_modules' list.
    map<const OnlineLearningModule*, int> module_to_index;
    for (int i = 0; i < all_modules.length(); i++)
        module_to_index[all_modules[i]] = i;
    
    // Analyze the list of ports.
    PLASSERT( ports.length() % 2 == 0 );
    // The 'port_correspondances' lists, for each module, the correspondances
    // between the modules' ports and the ports of the NetworkModule.
    TVec< TMat<int> > port_correspondances(all_modules.length());
    for (int i = 0; i < all_modules.length(); i++)
        port_correspondances[i].resize(0, 2);
    TVec<int> new_row(2);
    all_ports.resize(0);
    port_sizes.resize(0, 2);
    int n_ports = ports.length() / 2;
    port_descriptions.resize(n_ports);
    for (int i = 0; i < n_ports; i++) {
        const string& new_name = ports[2*i];
        all_ports.append(new_name);
        const string& old_name = ports[2*i + 1];
        size_t dot_pos = old_name.find('.');
        PLASSERT( dot_pos != string::npos );
        string old_module_name = old_name.substr(0, dot_pos);
        string old_module_port = old_name.substr(dot_pos + 1);
        PLASSERT( name_to_module.find(old_module_name) !=
                  name_to_module.end() );
        new_row[0] = i;
        PP<OnlineLearningModule> old_module = name_to_module[old_module_name];
        new_row[1] = old_module->getPortIndex(old_module_port);
        port_correspondances[module_to_index[old_module]].appendRow(new_row);
        new_row[0] = old_module->getPortLength(old_module_port);
        new_row[1] = old_module->getPortWidth(old_module_port);
        port_sizes.appendRow(new_row);
        port_descriptions[i] = old_module->getPortDescription(old_module_port);
    }

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
        int src = module_to_index[connection->getSourceModule()];
        int dest = module_to_index[connection->getDestinationModule()];
        inputs_needed[dest]++;
        compute_input_of[src].append(dest);
        map<string, PP<NetworkConnection> >& in_conn = in_connections[dest];
        if (in_conn.find(connection->getDestinationPort()) != in_conn.end()) {
            // The port has more than one incoming connection. Currently, we
            // only allow this to happen if the module is a NullModule (this is
            // for safety). If in the future we want more modules to allow
            // multiple incoming connections, we may get rid of this error.
            NullModule* test = dynamic_cast<NullModule*>(
                    get_pointer(connection->getDestinationModule()));
            if (!test)
                PLERROR("In LearningNetwork::build_ - A port may have only one "
                        "incoming connection");
        }
        in_conn[connection->getDestinationPort()] = connection;
        out_connections[src][connection->getSourcePort()].append(connection);
    }

    // The fprop and bprop paths can now be computed.
    fprop_path.resize(0);
    bprop_path.resize(all_modules.length());
    bprop_path.fill(-1);
    TVec<bool> is_done(all_modules.length(), false);
    fprop_data.resize(0);
    fprop_data_idx.resize(0);
    bprop_data.resize(all_modules.length());
    bprop_data_idx.resize(all_modules.length());
    fprop_toresize.resize(0);
    bprop_toresize.resize(all_modules.length());
    fprop_toplug.resize(0);
    // This is getting a little hackish here... If not enough memory is
    // allocated to store the work matrices, then when appending a new Mat to
    // 'all_mats', we will have to create a new Storage and copy previous data.
    // The problem is then that the pointers in 'fprop_data' and 'bprop_data'
    // will become invalid. So an easy fix is to allocate enough memory to
    // ensure we never have to resize the Storage later.
    int max_n_mats = 1000;
    all_mats.resize(max_n_mats);
    all_mats.resize(0);
    // A vector that stores the index of a module in the fprop path.
    TVec<int> module_index_to_path_index(all_modules.length(), -1);
    while (is_done.find(false) >= 0) {
        for (int i = 0; i < all_modules.length(); i++) {
            if (!is_done[i] && inputs_needed[i] == 0) {
                for (int j = 0; j < compute_input_of[i].length(); j++)
                    inputs_needed[compute_input_of[i][j]]--;
                // Save any correspondance between ports of this module and
                // ports of the encapsulating NetworkModule.
                fprop_toplug.append(port_correspondances[i]);
                // Compute the list of matrices that must be provided to this
                // module when doing a fprop and bprop.
                TVec<string> mod_ports = all_modules[i]->getPorts();
                map<string, PP<NetworkConnection> >& in_conn =
                    in_connections[i];
                map<string, TVec< PP<NetworkConnection> > >& out_conn =
                    out_connections[i];
                TVec<int> fprop_tores;
                TVec<int> bprop_tores;
                TVec<Mat*> fprop_mats;
                TVec<Mat*> bprop_mats;
                TVec<int> fprop_mats_idx;
                TVec<int> bprop_mats_idx;
                for (int j = 0; j < mod_ports.length(); j++) {
                    if (in_conn.find(mod_ports[j]) != in_conn.end()) {
                        // This port has an incoming connection: it is thus an
                        // input, and the corresponding matrices for storing
                        // its value and gradient are found by looking at the
                        // source port of the connection.
                        PP<NetworkConnection> conn = in_conn[mod_ports[j]];
                        int src_mod = module_to_index[conn->getSourceModule()];
                        int path_index = module_index_to_path_index[src_mod];
                        int port_index = conn->getSourceModule()->getPortIndex(
                                conn->getSourcePort());
                        fprop_mats.append(fprop_data[path_index][port_index]);
                        fprop_mats_idx.append(
                                fprop_data_idx[path_index][port_index]);
                        if (!conn->propagate_gradient) {
                            // This connection does not propagate the gradient,
                            // and thus we do not want to accumulate it.
                            bprop_mats.append(NULL);
                            bprop_mats_idx.append(-1);
                        } else {
                            int b_idx = all_modules.length() - 1 - path_index;
                            bprop_mats.append(bprop_data[b_idx][port_index]);
                            bprop_mats_idx.append(
                                    bprop_data_idx[b_idx][port_index]);
                            bprop_tores.append(j);
                        }
                        PLASSERT( out_conn.find(mod_ports[j]) == out_conn.end() );
                    } else if (out_conn.find(mod_ports[j]) != out_conn.end()) {
                        // This port has (at least) one outgoing connection: it
                        // is thus an output, and it must be provided with
                        // matrices to store its value (and gradient if the
                        // connection propagates it).
                        all_mats.append(Mat());
                        if (all_mats.length() > max_n_mats)
                            PLERROR("In NetworkModule::build_ - Will need to "
                                    "increase 'max_n_mats'");
                        Mat* new_mat = &all_mats.lastElement();
                        fprop_mats.append(new_mat);
                        fprop_mats_idx.append(all_mats.length() - 1);
                        fprop_tores.append(j);
                        // Ensure there exists a connection propagating the
                        // gradient to this port.
                        bool must_store_grad = false;
                        const TVec< PP<NetworkConnection> >& out_j =
                            out_conn[mod_ports[j]];
                        for (int k = 0; k < out_j.length(); k++)
                            if (out_j[k]->propagate_gradient) {
                                must_store_grad = true;
                                break;
                            }
                        if (must_store_grad) {
                            all_mats.append(Mat());
                            if (all_mats.length() > max_n_mats)
                                PLERROR("In NetworkModule::build_ - Will need "
                                        "to increase 'max_n_mats'");
                            new_mat = &all_mats.lastElement();
                            bprop_mats.append(new_mat);
                            bprop_mats_idx.append(all_mats.length() - 1);
                        } else {
                            // No connection propagating gradient to this port.
                            bprop_mats.append(NULL);
                            bprop_mats_idx.append(-1);
                        }
                    } else {
                        // This port is not used (we do not provide its value,
                        // and we do not care about obtaining it).
                        fprop_mats.append(NULL);
                        bprop_mats.append(NULL);
                        fprop_mats_idx.append(-1);
                        bprop_mats_idx.append(-1);
                    }
                }
                module_index_to_path_index[i] = fprop_path.length();
                // Update fprop path.
                PLASSERT( fprop_mats.length() == fprop_mats_idx.length() );
                fprop_data.append(fprop_mats);
                fprop_data_idx.append(fprop_mats_idx);
                fprop_toresize.append(fprop_tores);
                fprop_path.append(i);
                // Update bprop path.
                PLASSERT( bprop_mats_idx.length() == bprop_mats.length() );
                int bprop_idx = bprop_path.length() - fprop_path.length();
                bprop_data[bprop_idx] = bprop_mats;
                bprop_data_idx[bprop_idx] = bprop_mats_idx;
                bprop_toresize[bprop_idx] = bprop_tores;
                bprop_path[bprop_idx] = i;

                is_done[i] = true;
            }
        }
    }
    PLASSERT( module_index_to_path_index.find(-1) == -1 );
}

////////////
// forget //
////////////
void NetworkModule::forget()
{
    // Forward forget to the underlying modules, and provide them with a random
    // number generator if needed.
    for (int i = 0; i < all_modules.length(); i++) {
        if (!all_modules[i]->random_gen) {
            all_modules[i]->random_gen = random_gen;
            all_modules[i]->build();
        }
        all_modules[i]->forget();
    }
}

///////////
// fprop //
///////////
void NetworkModule::fprop(const Vec& input, Vec& output) const
{
    PLERROR("In NetworkModule::fprop - Not implemented for single vector "
            "operations");
}

void NetworkModule::fprop(const TVec<Mat*>& ports_value) {
    for (int i = 0; i < fprop_path.length(); i++) {
        PP<OnlineLearningModule> module = all_modules[fprop_path[i]];
        DBG_MODULE_LOG << "FPROP: " << module->name << endl;

        // First resize some data matrices, so that the outputs are properly
        // computed.
        const TVec<int>& toresize = fprop_toresize[i];
        const TVec<Mat*>& data = fprop_data[i];
        for (int j = 0; j < toresize.length(); j++) {
            DBG_MODULE_LOG << "  out = " << module->getPortName(toresize[j])
                           << endl;
            data[toresize[j]]->resize(0, 0);
        }

        // Then plug in the extra data provided in this method's fprop.
        const TMat<int>& toplug = fprop_toplug[i];
        for (int j = 0; j < toplug.length(); j++) {
            int swap_1 = toplug(j, 0);
            Mat* provided = ports_value[swap_1];
            if (!provided)
                // There is no provided matrix for this port, thus no extra
                // information to use or compute.
                continue;
            int swap_2 = toplug(j, 1);
            Mat* current = data[swap_2];
            if (!provided->isEmpty()) {
                // A full matrix is provided. This means the data is readily
                // available (= input), and thus should not have been computed
                // anywhere else.
                PLASSERT( !current );
                data[swap_2] = provided;
            } else {
                // An empty matrix is provided. This means we want to compute
                // this data. If it is already computed in the network, we do a
                // copy (may not be the most efficient way to do it though). If
                // it is not already computed, we can directly plug the
                // provided matrix.
                if (!current) {
                    data[swap_2] = provided;
                } else {
                    // This data should be something we compute.
                    PLASSERT( current->isEmpty() );
                }
            }
        }

        // Now perform fprop of the i-th module.
        module->fprop(data);

        // And restore 'data', in addition to copy any computed value that
        // needs to be retrieved.
        for (int j = 0; j < toplug.length(); j++) {
            int swap_1 = toplug(j, 0);
            Mat* provided = ports_value[swap_1];
            if (!provided)
                continue;
            int swap_2 = toplug(j, 1);
            Mat* current = data[swap_2];
            if (!provided->isEmpty()) {
                // This is either a matrix given as input, or directly plugged
                // in the network. In both cases, the old value in 'data' was a
                // null pointer.
                PLASSERT( current == provided );
                data[swap_2] = NULL;
            } else {
                // This is an output we need to copy.
                PLASSERT( current && !current->isEmpty() );
                provided->resize(current->length(), current->width());
                *provided << *current;
            }
        }
    }
}

////////////////////////
// getPortDescription //
////////////////////////
TVec<string> NetworkModule::getPortDescription(const string& port)
{
    return port_descriptions[getPortIndex(port)];
}

//////////////
// getPorts //
//////////////
const TVec<string>& NetworkModule::getPorts()
{
    return all_ports;
}

//////////////////
// getPortSizes //
//////////////////
const TMat<int>& NetworkModule::getPortSizes()
{
    return port_sizes;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void NetworkModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(modules, copies);
    deepCopyField(connections, copies);
    deepCopyField(ports, copies);
    deepCopyField(all_modules, copies);
    deepCopyField(all_connections, copies);
    deepCopyField(fprop_path, copies);
    deepCopyField(bprop_path, copies);
    deepCopyField(fprop_data_idx, copies);
    deepCopyField(fprop_toresize, copies);
    deepCopyField(fprop_toplug, copies);
    deepCopyField(bprop_data_idx, copies);
    deepCopyField(bprop_toresize, copies);
    deepCopyField(all_mats, copies);
    deepCopyField(all_ports, copies);
    deepCopyField(port_descriptions, copies);

    // Special code to handle the deep copy of 'fprop_data' and 'bprop_data'.
    // A better way may exist to do this!
    fprop_data = TVec< TVec<Mat*> >(fprop_data.length());
    bprop_data = TVec< TVec<Mat*> >(bprop_data.length());
    PLASSERT( fprop_data.length() == bprop_data.length() );
    for (int i = 0; i < fprop_data.length(); i++) {
        TVec<Mat*>& fdata = fprop_data[i];
        TVec<Mat*>& bdata = bprop_data[i];
        const TVec<int>& fdata_idx = fprop_data_idx[i];
        const TVec<int>& bdata_idx = bprop_data_idx[i];
        fdata.resize(fdata_idx.length());
        bdata.resize(bdata_idx.length());
        for (int k = 0; k < fdata_idx.length(); k++) {
            int idx = fdata_idx[k];
            if (idx == -1)
                fdata[k] = NULL;
            else
                fdata[k] = &all_mats[idx];
        }
        for (int k = 0; k < bdata_idx.length(); k++) {
            int idx = bdata_idx[k];
            if (idx == -1)
                bdata[k] = NULL;
            else
                bdata[k] = &all_mats[idx];
        }
    }
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
