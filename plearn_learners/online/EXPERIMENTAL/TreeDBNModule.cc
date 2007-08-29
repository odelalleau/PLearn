// TreeDBNModule.cc
//
// Copyright (C) 2007 Vytenis Sakenas
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

// Authors: Vytenis Sakenas

/*! \file TreeDBNModule.cc */



#include "TreeDBNModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
   TreeDBNModule,
  "Hierarchical deep network.",
   "Hierarchical deep network. In every level, a RBM takes input from n_parents_per_node lower"
       " layer RBMs. All RBMs in a layer share weights. So, for example, a network with 3 layers and"
       " n_parents_per_node=2 will have 1, 2 and 4 RBMs in top, middle and bottom layers respectively."
       " Typical usage is providing RBM modules for every layer through modules option, possibly adding "
       "additional ports we want to compute and setting flags like propagate_gradient, propagate_energy_gradient"
       " and propagate_full_gradient to a desired state."
       "Ports:\n"
       "\tinput, output_1 ... output_n"
       "where n is number of layers"
);

//////////////////
// TreeDBNModule //
//////////////////
TreeDBNModule::TreeDBNModule() : n_parents_per_node(2), n_shared_parents(0), gradient_multiplier(1.0),
                               propagate_gradient(false), propagate_energy_gradient(false), propagate_full_gradient(false)
/* ### Initialize all fields to their default value here */
{
}

////////////////////
// declareOptions //
////////////////////
void TreeDBNModule::declareOptions(OptionList& ol)
{
   // Now call the parent class' declareOptions
   inherited::declareOptions(ol);

       declareOption(ol, "modules", &TreeDBNModule::modules,
                  OptionBase::buildoption,
                  "RBMModule list that is used to build DBN.");

       declareOption(ol, "n_parents_per_node", &TreeDBNModule::n_parents_per_node,
                                 OptionBase::buildoption,
                                 "How many parents each node has.");

       // Not implemented.
       //declareOption(ol, "n_shared_parents", &TreeDBNModule::n_shared_parents,
       //                        OptionBase::buildoption,
       //                        "Number of parents that two adjacent nodes share.");

       declareOption(ol, "propagate_gradient", &TreeDBNModule::propagate_gradient,
                                 OptionBase::buildoption,
                                 "Whether we propagate gradient through hierarchy.");

       declareOption(ol, "propagate_full_gradient", &TreeDBNModule::propagate_full_gradient,
                                 OptionBase::buildoption,
                                 "If propagate_gradient==true then this flag determines that gradient should be propagated"
                                 " through full hierarchy. Else propagation is only done through the rightmost branch.");

       declareOption(ol, "propagate_energy_gradient", &TreeDBNModule::propagate_energy_gradient,
                                 OptionBase::buildoption,
                                 "Whether we compute and propagate free energy gradient from top layer.");

	// Probabaly not useful.
       declareOption(ol, "gradient_multiplier", &TreeDBNModule::gradient_multiplier,
                                 OptionBase::buildoption,
                                 "Value that propagated gradient is multiplied before propagating from top layer.");

       declareOption(ol, "ports", &TreeDBNModule::ports,
                                 OptionBase::buildoption,
                                 "A sequence of pairs of strings, where each pair is of the form\n"
                                                 "\"P\":\"M.N\" with 'M' the name of an underlying module, 'N' one of\n"
                                                 "its ports, and 'P' the name under which the TreeDBNModule sees this\n"
                                                 "port. See the class help for an example. If 'P' is an empty string,\n"
                                                 "then the port name will be 'M.N'.");

}



////////////////////
// declareMethods //
////////////////////
void TreeDBNModule::declareMethods(RemoteMethodMap& rmm)
{
   // Insert a backpointer to remote methods; note that this
   // different than for declareOptions()
       rmm.inherited(inherited::_getRemoteMethodMap_());

       declareMethod(
                       rmm, "initSampling", &TreeDBNModule::initSampling,
       (BodyDoc("Initializes network for sampling. This function must be called before any calls to sample().\n"),
        ArgDoc ("gibbsTop", "Number of gibbs steps to do in top rbm.")));

       declareMethod(
                       rmm, "clearCache", &TreeDBNModule::clearCache,
       (BodyDoc("Clears all caches. Call this after changing any of the module parameters.\n")));

       declareMethod(
                       rmm, "sample", &TreeDBNModule::sample,
       (BodyDoc("Samples the network. Returns a sample on the visible layer.\n"),
        ArgDoc("gibbsTop", "Number of gibbs steps in the top layer for each sample."),
        RetDoc ("Sample.")));
}

//! Add a port to the module with given name, which is filled from a rbm modules[rbm_index]
//! an port port_name and provided port width. If a port you add is not directly filled from
//! a rbm then provide rbm_index=-1. If port_width is not provided then it is determined from
//! the rbm it is filled from.
void TreeDBNModule::appendPort(string name, int rbm_index, string port_name, int port_width = -1)
{
       port_names.append(name);
       port_rbms.append(rbm_index);

       if (rbm_index >= 0) {
               int index = modules[rbm_index]->getPortIndex(port_name);
               PLASSERT(index >= 0);
               port_index.append( index );
       }
       else
               port_index.append( -1 );

       if (port_width == -1) {
               // We need to extract actual port size
               port_width = modules[rbm_index]->getPortWidth(port_name);
       }

       TVec <int> sz(2, -1);
       sz[1] = port_width;
       port_sizes.appendRow(sz);
}

////////////
// build_ //
////////////
void TreeDBNModule::build_()
{
       n_layers = modules.length();
       time = 0;

       // Fill ports
       port_names.clear();
       port_rbms.clear();
       port_index.clear();
       port_sizes.clear();
       appendPort("input", -1, "", modules[0]->visible_layer->size);

       layer_sizes.resize(n_layers);

       // Add output ports for every layer rbm
       for (int i = 1; i <= n_layers; ++i) {
               appendPort("output_" + tostring(i), i-1, "hidden.state");
               layer_sizes[i-1] = 1<<(n_layers-i);
       }

       // Add ports that are forwarded from internal modules
       for (int i = 0; i < ports.size(); ++i) {
               string s = ports[i].second;

               size_t dot = s.find('.');
               PLASSERT( dot != string::npos );
               string module_name = s.substr(0, dot);
               string port_name = s.substr(dot + 1);

               bool valid_redirect = false;
               for (int j = 0; j < n_layers; ++j) {
                       if (modules[j]->name == module_name) {
                               appendPort(ports[i].first, j, port_name);
                               valid_redirect = true;
                       }
               }

               PLASSERT(valid_redirect);
       }

       // Make sure storage matrix vectors will not be resized and we will not loose pointers.
       mats.resize(1000);
       mats.resize(0);
       cache_mats.resize(1000);
       cache_mats.resize(0);

       step_size.resize(n_layers);
       step_size[0] = 2;
       for (int i = 1; i < n_layers; ++i) {
               step_size[i] = n_parents_per_node * step_size[i-1];
       }

       // Prepare arrays for holding fprop and bprop data
       bprop_data.resize(n_layers);
       fprop_data.resize(n_layers);
       bprop_data_cache.resize(n_layers);                                      // do not cache (?)
       fprop_data_cache.resize(n_layers);

       for (int i = 0; i < n_layers; ++i) {
               int np = modules[i]->nPorts();
               bprop_data[i].resize(np);
               fprop_data[i].resize(np);
               bprop_data_cache[i].resize(np);
               fprop_data_cache[i].resize(np);
               bprop_data[i].fill((Mat*)NULL);
               fprop_data[i].fill((Mat*)NULL);
               bprop_data_cache[i].fill((Mat*)NULL);
               fprop_data_cache[i].fill((Mat*)NULL);
       }

       // Here we will hold last full input to lower layer
       // It is done to be able to check if input is a shifted
       // version of previous input.
       last_full_input.resize(0);

       // Safety check
       for (int i = 0; i < n_layers-1; ++i)
               PLASSERT(modules[i]->hidden_layer->size * n_parents_per_node == modules[i+1]->visible_layer->size);

       // Forward random number generator to all underlying modules.
       if (random_gen) {
               cout << "Forget in build" << endl;
               for (int i = 0; i < modules.length(); i++) {
                       if (!modules[i]->random_gen) {
				cout << "pass forget" << endl;
                               modules[i]->random_gen = random_gen;
                               modules[i]->build();
                               modules[i]->forget();
                       }
               }
       }
}

///////////
// build //
///////////
void TreeDBNModule::build()
{
   inherited::build();
   build_();
   Profiler::activate();
}

////////////////////
// bpropAccUpdate //
////////////////////
void TreeDBNModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                                                  const TVec<Mat*>& ports_gradient)
{
       PLASSERT( ports_value.length() == nPorts() && ports_gradient.length() == nPorts());

       Profiler::start("full bprop");
       if (!propagate_gradient) {			// Only unsupervised learning in a module
               for (int layer = n_layers-1; layer >= 0; layer--) {
                       int n_mod_ports = modules[layer]->nPorts();

                       bprop_data[layer].resize(n_mod_ports);
                       bprop_data[layer].fill((Mat*)NULL);
                       int mod_batch_size = fprop_data[layer][modules[layer]->getPortIndex("hidden.state")]->length();

                       if (modules[layer]->reconstruction_connection != NULL) {
                               bprop_data[layer][modules[layer]->getPortIndex("reconstruction_error.state")] = createMatrix(mod_batch_size, 1, mats);
                               bprop_data[layer][modules[layer]->getPortIndex("reconstruction_error.state")]->fill(1);
                       }

                       Profiler::start("bprop");
                       modules[layer]->bpropAccUpdate(fprop_data[layer], bprop_data[layer]);
                       Profiler::end("bprop");
               }
       } else
       {
               if (!propagate_full_gradient)           // Propagate only rightmost branch
               {
                       // For top RBM we provide energy gradient only and get gradient on visible
                       bprop_data[n_layers - 1].resize( modules[n_layers-1]->nPorts() );
                       bprop_data[n_layers - 1].fill((Mat*)NULL);

                       int mod_batch_size = fprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("visible")]->length();

                       if (propagate_energy_gradient) {
                               bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("energy")] = createMatrix(mod_batch_size, 1, mats);
                               bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("energy")]->fill(1);
                       }

                       if (modules[n_layers-1]->reconstruction_connection != NULL) {
                               bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("reconstruction_error.state")] = 
												createMatrix(mod_batch_size, 1, mats);
                               bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("reconstruction_error.state")]->fill(1);
                       }

                       // Take external gradient on output
                       int out_grad = getPortIndex("output_"+tostring(n_layers));

                       if ( ports_gradient[out_grad] == NULL || ports_gradient[out_grad]->isEmpty() ) {
                               // Make gradient zero
                               ports_gradient[out_grad] = createMatrix(mod_batch_size, modules[n_layers-1]->hidden_layer->size, mats);
                               ports_gradient[out_grad]->fill(0);
                               PLWARNING("Top RBM output port has no gradient information. Using 0 gradient.");
                       }
                       //PLASSERT(ports_gradient[out_grad] != NULL);


                       bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("hidden.state")] = 
							createMatrix(mod_batch_size, ports_gradient[out_grad]->width(), mats);
                       *bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("hidden.state")] << *ports_gradient[out_grad];

                       // Ask for visible gradient
                       bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("visible")] = 
							createMatrix(0, modules[n_layers-1]->visible_layer->size, mats);

                       Profiler::start("bprop");
                       modules[n_layers-1]->bpropAccUpdate(fprop_data[n_layers-1], bprop_data[n_layers-1]);
                       Profiler::end("bprop");


                       Mat *mat = bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("visible")];
                       for (int i = 0; i < mat->length(); ++i)
                               for (int j = 0; j < mat->width(); ++j)
                                       (*mat)[i][j] *= gradient_multiplier;


                       // Now for every layer take upper layers visible gradient
                       // and pass it to current layers hidden.state port.
                       for (int layer = n_layers-1; layer > 0; layer--) {
                               int n_mod_ports = modules[layer-1]->nPorts();

                               bprop_data[layer-1].resize(n_mod_ports);
                               bprop_data[layer-1].fill((Mat*)NULL);

                               int mod_batch_size = fprop_data[layer-1][modules[layer-1]->getPortIndex("visible")]->length();
                               int width = modules[layer-1]->hidden_layer->size;


                               Mat *hidden_state = createMatrix(mod_batch_size, width, mats);
                               Mat *rbm_visible = bprop_data[layer][modules[layer]->getPortIndex("visible")];

                               int parent_width = modules[layer-1]->hidden_layer->size;
                               int minibatch_size = ports_value[getPortIndex("input")]->length();

                               TVec <int> used(mod_batch_size, 0);	// Ensure that we right gradient only once (the one we need is first one)

                               // do the same thing like in fprop
                               for (int mbi = 0, index = 0; mbi < minibatch_size; ++mbi)
                               {
                                       if (mbi_time[mbi] < step_size[layer]) {
                                               // Computed all rbms in upper layer
                                               for (int i = 0; i < layer_sizes[layer]; ++i)
                                               {
                                                       // Here parents are this layer rbm (where we want to write gradient)
                                                       for (int parent = 0; parent < n_parents_per_node; ++parent) {
                                                               int row_id = mod_batch_length[layer-1][mbi] - 
											hash(mbi_time[mbi], layer-1, 2*i + parent);
                                                               if (row_id < 0) {
                                                                       // It must be in cache - do nothing
                                                               } else {
                                                                       if (!used[row_id])
                                                                               (*hidden_state)(row_id) <<
										(*rbm_visible)(index).subVec(parent*parent_width, parent_width);
                                                                       used[row_id]++;
                                                               }
                                                       }
                                                       ++index;
                                               }
                                       } else {
                                               // Compute only last rbm
                                               for (int parent = 0; parent < n_parents_per_node; ++parent) {
                                                       int row_id = mod_batch_length[layer-1][mbi] - 
										hash(mbi_time[mbi], layer-1, 2*(layer_sizes[layer]-1) + parent);
                                                       if (row_id < 0) {
                                                               // It must be in cache - do nothing
                                                       } else {
                                                               if (!used[row_id])
                                                                       (*hidden_state)(row_id) << 
										(*rbm_visible)(index).subVec(parent*parent_width, parent_width);
                                                               used[row_id]++;
                                                       }
                                               }
                                               ++index;
                                       }
                               }

                               // Provide hidden gradient..
                               bprop_data[layer-1][modules[layer-1]->getPortIndex("hidden.state")] = hidden_state;

                               // add a gradient that is provided externally on output_i port
                               Mat *xgrad = ports_gradient[getPortIndex("output_"+tostring(layer))];
                               if (xgrad != NULL && !xgrad->isEmpty()) {
                                       //cout << "grad_flow: " << layer << " " << (*xgrad)(0)[0] << endl;
                                       // Length of xgrad is <= hidden_state so we need to sum row by row
                                       for (int mbi = 0; mbi < minibatch_size; ++mbi) {
                                               (*hidden_state)(mod_batch_length[layer-1][mbi]-1) += (*xgrad)(mbi);
                                       }
                               }

                               // and ask for visible gradient
                               bprop_data[layer-1][modules[layer-1]->getPortIndex("visible")] = 
							createMatrix(0, modules[layer-1]->visible_layer->size, mats);

                               if (modules[layer-1]->reconstruction_connection != NULL) {
                                       bprop_data[layer-1][modules[layer-1]->getPortIndex("reconstruction_error.state")] =
												createMatrix(mod_batch_size, 1, mats);
                                       bprop_data[layer-1][modules[layer-1]->getPortIndex("reconstruction_error.state")]->fill(1);
                               }

                               Profiler::start("bprop");
                               modules[layer-1]->bpropAccUpdate(fprop_data[layer-1], bprop_data[layer-1]);
                               Profiler::end("bprop");
                       }  // for every layer
               } else                          // Propagate through all hierarchy
               {
			bprop_data[n_layers - 1].resize( modules[n_layers-1]->nPorts() );
			bprop_data[n_layers - 1].fill((Mat*)NULL);
		
			int mod_batch_size = fprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("visible")]->length();
		
			if (propagate_energy_gradient) {
				bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("energy")] = createMatrix(mod_batch_size, 1, mats);
				bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("energy")]->fill(1);
			}
		
			if (modules[n_layers-1]->reconstruction_connection != NULL) {
				bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("reconstruction_error.state")] =
												createMatrix(mod_batch_size, 1, mats);
				bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("reconstruction_error.state")]->fill(1);
			}
		
			// Take external gradient on output
			int out_grad = getPortIndex("output_"+tostring(n_layers));
		
			if ( ports_gradient[out_grad] == NULL || ports_gradient[out_grad]->isEmpty() ) {
				// Make gradient zero
				ports_gradient[out_grad] = createMatrix(mod_batch_size, modules[n_layers-1]->hidden_layer->size, mats);
				ports_gradient[out_grad]->fill(0);
				PLWARNING("Top RBM output port has no gradient information. Using 0 gradient.");
			}
			//PLASSERT(ports_gradient[out_grad] != NULL);
		
		
			bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("hidden.state")] = 
								createMatrix(mod_batch_size, ports_gradient[out_grad]->width(), mats);
			*bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("hidden.state")] << *ports_gradient[out_grad];
		
			// Ask for visible gradient
			bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("visible")] = 
								createMatrix(0, modules[n_layers-1]->visible_layer->size, mats);
		
			Profiler::start("bprop");
			modules[n_layers-1]->bpropAccUpdate(fprop_data[n_layers-1], bprop_data[n_layers-1]);
			Profiler::end("bprop");
		
		
			Mat *mat = bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("visible")];
			for (int i = 0; i < mat->length(); ++i)
				for (int j = 0; j < mat->width(); ++j)
					(*mat)[i][j] *= gradient_multiplier;
		
			int minibatch_size = ports_value[getPortIndex("input")]->length();
		
			// Now for every layer take upper layers visible gradient
			// and pass it to current layers hidden.state port.
			for (int layer = n_layers-1; layer > 0; layer--) {
				int n_mod_ports = modules[layer-1]->nPorts();
		
				bprop_data[layer-1].resize(n_mod_ports);
				bprop_data[layer-1].fill((Mat*)NULL);
		
				int mod_batch_size = minibatch_size*layer_sizes[layer-1];
				int width = modules[layer-1]->hidden_layer->size;
		
				Mat *hidden_state = createMatrix(mod_batch_size, width, mats);
				Mat *rbm_visible = bprop_data[layer][modules[layer]->getPortIndex("visible")];
		
				int parent_width = modules[layer-1]->hidden_layer->size;
		
				for (int mbi = 0, index = 0; mbi < minibatch_size; ++mbi)
				{
					for (int i = 0; i < layer_sizes[layer-1]; ++i)
					{
						// Write gradient from parent
						int parent_ix = mbi*layer_sizes[layer] + i/n_parents_per_node;
						int child_ix = i%n_parents_per_node;
						(*hidden_state)(index++) << (*rbm_visible)(parent_ix).subVec(child_ix*parent_width, parent_width);
					}
				}
		
				// Provide hidden gradient..
				bprop_data[layer-1][modules[layer-1]->getPortIndex("hidden.state")] = hidden_state;
		
				// add a gradient that is provided externally on output_i port
				Mat *xgrad = ports_gradient[getPortIndex("output_"+tostring(layer))];
				if (xgrad != NULL && !xgrad->isEmpty()) {
					//cout << "grad_flow: " << layer << " " << (*xgrad)(0)[0] << endl;
					// Length of xgrad is <= hidden_state so we need to sum row by row
					for (int mbi = 0; mbi < minibatch_size; ++mbi) {
						(*hidden_state)(mbi*layer_sizes[layer-1]+layer_sizes[layer-1]-1) += (*xgrad)(mbi);
					}
				}
		
				// and ask for visible gradient
				bprop_data[layer-1][modules[layer-1]->getPortIndex("visible")] = 
									createMatrix(0, modules[layer-1]->visible_layer->size, mats);
		
				if (modules[layer-1]->reconstruction_connection != NULL) {
					bprop_data[layer-1][modules[layer-1]->getPortIndex("reconstruction_error.state")] = 
												createMatrix(mod_batch_size, 1, mats);
					bprop_data[layer-1][modules[layer-1]->getPortIndex("reconstruction_error.state")]->fill(1);
				}

				/*for (int i = 0; i < n_mod_ports; ++i) {
					cout << i << " " << modules[layer-1]->getPorts()[i] << " ";
					if (full_fprop_data[i])
						cout << full_fprop_data[i]->length() << endl;
					else
						cout << "NULL" << endl;
				}*/
		
				Profiler::start("bprop");
				modules[layer-1]->bpropAccUpdate(fprop_data[layer-1], bprop_data[layer-1]);
				Profiler::end("bprop");
			}	// for every layer
			//updateCache();		// no cache update as we dont have any
		}


		// Following code would work without need of doing full_fprop. However because RBMMixedLayer caches nll
		// during fprop and then reuses it in bprop it is not possible.
		/*{
			// For top RBM we provide energy gradient only and get gradient on visible
			bprop_data[n_layers - 1].resize( modules[n_layers-1]->nPorts() );
			bprop_data[n_layers - 1].fill((Mat*)NULL);
		
			int mod_batch_size = fprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("visible")]->length();
		
			if (propagate_energy_gradient) {
				bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("energy")] = createMatrix(mod_batch_size, 1, mats);
				bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("energy")]->fill(1);
			}
		
			if (modules[n_layers-1]->reconstruction_connection != NULL) {
				bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("reconstruction_error.state")] = createMatrix(mod_batch_size, 1, mats);
				bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("reconstruction_error.state")]->fill(1);
			}
		
			// Take external gradient on output
			int out_grad = getPortIndex("output_"+tostring(n_layers));
		
			if ( ports_gradient[out_grad] == NULL || ports_gradient[out_grad]->isEmpty() ) {
				// Make gradient zero
				ports_gradient[out_grad] = createMatrix(mod_batch_size, modules[n_layers-1]->hidden_layer->size, mats);
				ports_gradient[out_grad]->fill(0);
				PLWARNING("Top RBM output port has no gradient information. Using 0 gradient.");
			}
			//PLASSERT(ports_gradient[out_grad] != NULL);
		
		
			bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("hidden.state")] = createMatrix(mod_batch_size, ports_gradient[out_grad]->width(), mats);
			*bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("hidden.state")] << *ports_gradient[out_grad];
		
			// Ask for visible gradient
			bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("visible")] = createMatrix(0, modules[n_layers-1]->visible_layer->size, mats);
		
			Profiler::start("bprop");
			modules[n_layers-1]->bpropAccUpdate(fprop_data[n_layers-1], bprop_data[n_layers-1]);
			Profiler::end("bprop");
		
		
			Mat *mat = bprop_data[n_layers-1][modules[n_layers-1]->getPortIndex("visible")];
			for (int i = 0; i < mat->length(); ++i)
				for (int j = 0; j < mat->width(); ++j)
					(*mat)[i][j] *= gradient_multiplier;
		
			int minibatch_size = ports_value[getPortIndex("input")]->length();
		
			// Now for every layer take upper layers visible gradient
			// and pass it to current layers hidden.state port.
			for (int layer = n_layers-1; layer > 0; layer--) {
				int n_mod_ports = modules[layer-1]->nPorts();
		
				bprop_data[layer-1].resize(n_mod_ports);
				bprop_data[layer-1].fill((Mat*)NULL);
		
				int mod_batch_size = minibatch_size*layer_sizes[layer-1];
				int width = modules[layer-1]->hidden_layer->size;
		
				// We need to make new fprop_data vector with full(expanded) data.
				TVec <Mat*> full_fprop_data(n_mod_ports, (Mat*)NULL);
				for (int i = 0; i < n_mod_ports; ++i) {
					if (fprop_data[layer-1][i] != NULL && !fprop_data[layer-1][i]->isEmpty()
						// HACK to make it work with a hack in RBMModule when visible_activations.state is not computed
						&& (fprop_data[layer-1][i]->length() > 1 || fprop_data[layer-1][i]->width() > 1) ) {
						full_fprop_data[i] = createMatrix(mod_batch_size, fprop_data[layer-1][i]->width(), mats);
					}
				}
		
				Mat *hidden_state = createMatrix(mod_batch_size, width, mats);
				Mat *rbm_visible = bprop_data[layer][modules[layer]->getPortIndex("visible")];
		
				int parent_width = modules[layer-1]->hidden_layer->size;
		
				for (int mbi = 0, index = 0; mbi < minibatch_size; ++mbi)
				{
					for (int i = 0; i < layer_sizes[layer-1]; ++i)
					{
						// Fill full_fprop_data properly
						int row_id = mod_batch_length[layer-1][mbi] - hash(mbi_time[mbi], layer-1, i);
						for (int j = 0; j < n_mod_ports; ++j) {
							if (full_fprop_data[j] != NULL) {
								if (row_id < 0) {
									// Fill from cache
									PLASSERT_MSG(fprop_data_cache[layer-1][j], "Cache is NULL");
									int row_in_cache = fprop_data_cache[layer-1][j]->length()+row_id;
									PLASSERT_MSG(row_in_cache >= 0, "Cache is provided but is too small");
									(*full_fprop_data[j])(index) << (*fprop_data_cache[layer-1][j])(row_in_cache);
								} else {
									(*full_fprop_data[j])(index) << (*fprop_data[layer-1][j])(row_id);
								}
							}
						}
		
						// Write gradient from parent
						int parent_ix = mbi*layer_sizes[layer] + i/n_parents_per_node;
						int child_ix = i%n_parents_per_node;
						(*hidden_state)(index++) << (*rbm_visible)(parent_ix).subVec(child_ix*parent_width, parent_width);
					}
				}
		
		
				// Provide hidden gradient..
				bprop_data[layer-1][modules[layer-1]->getPortIndex("hidden.state")] = hidden_state;
		
				// add a gradient that is provided externally on output_i port
				Mat *xgrad = ports_gradient[getPortIndex("output_"+tostring(layer))];
				if (xgrad != NULL && !xgrad->isEmpty()) {
					//cout << "grad_flow: " << layer << " " << (*xgrad)(0)[0] << endl;
					// Length of xgrad is <= hidden_state so we need to sum row by row
					for (int mbi = 0; mbi < minibatch_size; ++mbi) {
						(*hidden_state)(mbi*layer_sizes[layer-1]+layer_sizes[layer-1]-1) += (*xgrad)(mbi);
					}
				}
		
				// and ask for visible gradient
				bprop_data[layer-1][modules[layer-1]->getPortIndex("visible")] = createMatrix(0, modules[layer-1]->visible_layer->size, mats);
		
				if (modules[layer-1]->reconstruction_connection != NULL) {
					bprop_data[layer-1][modules[layer-1]->getPortIndex("reconstruction_error.state")] = createMatrix(mod_batch_size, 1, mats);
					bprop_data[layer-1][modules[layer-1]->getPortIndex("reconstruction_error.state")]->fill(1);
				}

				for (int i = 0; i < n_mod_ports; ++i) {
					cout << i << " " << modules[layer-1]->getPorts()[i] << " ";
					if (full_fprop_data[i])
						cout << full_fprop_data[i]->length() << endl;
					else
						cout << "NULL" << endl;
				}
		
				Profiler::start("bprop");
				modules[layer-1]->bpropAccUpdate(full_fprop_data, bprop_data[layer-1]);
				Profiler::end("bprop");
			}	// for every layer
			updateCache();
		}*/


       }

       //cout << "end back" << endl;
   // Ensure all required gradients have been computed.
       checkProp(ports_gradient);

       Profiler::end("full bprop");
}




//////////////////////
// bpropDoesNothing //
//////////////////////
/* THIS METHOD IS OPTIONAL
// the default implementation returns false
bool TreeDBNModule::bpropDoesNothing()
{
}
*/

//////////////
// finalize //
//////////////
/* THIS METHOD IS OPTIONAL
void TreeDBNModule::finalize()
{
}
*/

////////////
// forget //
////////////
void TreeDBNModule::forget()
{
       cout << "Forget" << endl;
       for (int i  = 0; i < n_layers; ++i)
               modules[i]->forget();
}

//! Check if b equals a shifted left by k.
bool TreeDBNModule::check_shift(Vec &a, Vec& b, int k)
{
       PLASSERT(a.length() == b.length());

       for (int i = k; i < a.length(); ++i) {
               if ( !fast_is_equal(a[i], b[i-k]) )
                       return false;
       }

       return true;
}


//! Provided pseudotime, rbm layer and rbm index (both zero based) in the layer returns 
//! distance from the end of computed fprop_data where rbm with same 
//! parameters was computed. For example, if provided parameters 6, 1, 1 
//! it returns -3, then it means that second rbm in the second layer
//! was computed and is stored in fprop_data[fprop_data.length-3]
// OK
int TreeDBNModule::hash(int t, int k, int i)
{
       if (t < step_size[k]) return layer_sizes[k] - i;            // all rbms were computed
       if (i == layer_sizes[k] - 1) return 1;                                          // last rbm in layer asked, and was computed

  // check if there was a moment when this input was fed to the last rbm in the layer
       if ( (layer_sizes[k] - 1 - i)*step_size[k] <= t) {
               int t_diff = (layer_sizes[k] - 1 - i)*step_size[k];
      // In first step_size[k] time steps we added layer_size[k] entries.
               return t_diff + max(0, step_size[k] - (t - t_diff) - 1)*(layer_sizes[k]-1) + 1;
       }

  // the only option is that this input was fed to some intermediate rbm
       int ix = i + t/step_size[k];                    // Index of that rbm
       int t_diff = (ix - i)*step_size[k];             //
       return t_diff + max(0, step_size[k] - (t - t_diff) - 1)*(layer_sizes[k]-1) + layer_sizes[k] - 1 - ix + 1;
}

// helper function that creates matrix of given size in
// mats vector and returns pointer to it.
Mat* TreeDBNModule::createMatrix(int length, int width, TVec <Mat> &mats)
{
       mats.append(Mat(length, width));
       return &mats.lastElement();
}


//! Unoptimized version of fprop
void TreeDBNModule::full_fprop(const TVec<Mat*>& ports_value)
{
       Profiler::start("full fprop");
       mats.resize(0);

       vector <string> prts = modules[0]->getPorts();

	Mat* input = ports_value[getPortIndex("input")];
	int minibatch_size = input->length();

	mbi_time.resize(minibatch_size);
	mod_batch_length.resize(n_layers, minibatch_size);

	// Process layerwise
	for (int layer = 0; layer < n_layers; ++layer)
	{
		fprop_data[layer].resize(modules[layer]->nPorts());
		fprop_data[layer].fill((Mat*)NULL);

		// Count number of rows
		int nRows = layer_sizes[layer]*minibatch_size;

		// Prepare matrices
		Mat* rbm_visible = createMatrix(nRows, modules[layer]->visible_layer->size, mats);
		fprop_data[layer][modules[layer]->getPortIndex("visible")] = rbm_visible;

		//Create all .state matrices
		for (int i = 0; i < modules[layer]->nPorts(); ++i) {
			string pname = modules[layer]->getPorts()[i];
			if ( pname.length() > 6 && ".state" == pname.substr(pname.length()-6) ) {
				if (fprop_data[layer][i] == NULL)
					fprop_data[layer][i] = createMatrix(0, 0, mats);
			}
		}

		if (modules[layer]->reconstruction_connection == NULL) {
			fprop_data[layer][modules[layer]->getPortIndex("reconstruction_error.state")] = NULL;
			fprop_data[layer][modules[layer]->getPortIndex("visible_reconstruction.state")] = NULL;
			fprop_data[layer][modules[layer]->getPortIndex("visible_reconstruction_activations.state")] = NULL;
		}

		// Create empty matrices for forwarded ports
		for (int i = 0; i < nPorts(); ++i) {
			if (port_rbms[i] >= 0) {
				if (ports_value[i] != NULL && fprop_data[port_rbms[i]][port_index[i]] == NULL)
					fprop_data[port_rbms[i]][port_index[i]] = createMatrix(0, 0, mats);
			}
		}

		// Go through all minibatch and fill visible expectations
		if (layer == 0)
		{       // Handle input layer in different manner
			int visible_size = modules[layer]->visible_layer->size;

			for (int mbi = 0, index = 0; mbi < minibatch_size; ++mbi)
			{
				for (int i = 0; i < layer_sizes[layer]; ++i)
				{
					(*rbm_visible)(index++) << (*input)(mbi).subVec(i*visible_size, visible_size);
				}
			}
		}
		else
		{
			// Take parent layer expectations
			Mat *expectations = fprop_data[layer-1][modules[layer-1]->getPortIndex("hidden.state")];

			int parent_width = modules[layer-1]->hidden_layer->size;
			for (int mbi = 0, index = 0; mbi < minibatch_size; ++mbi)
			{
				// Compute all rbms
				for (int i = 0; i < layer_sizes[layer]; ++i)
				{
					for (int parent = 0; parent < n_parents_per_node; ++parent) {
						int row_id = mbi*layer_sizes[layer-1] + i*n_parents_per_node + parent;
						(*rbm_visible)(index).subVec(parent*parent_width, parent_width) <<
									(*expectations)(row_id);
					}
					++index;
				}
			}
		}

		Profiler::start("fprop");
		//cout << "fprop: " << endl;
		//cout << (*fprop_data[layer][0]) << endl;
		//cout << "************" << endl;
		modules[layer]->fprop(fprop_data[layer]);
		Profiler::end("fprop");
	}

	time = 0;
	last_full_input.resize(input->width());
	last_full_input << (*input)(minibatch_size-1);

	// and write all required output to the provided ports ( output_i + requested )
	//cout << "write" << endl;
	for (int i = 0; i < nPorts(); ++i) {
		Mat *mat = ports_value[i];

		if ( mat != NULL && mat->isEmpty() ) {
			// We check of which layer output should be writen to the port
			int pl = port_rbms[i];
			if (pl >= 0) {
				mat->resize(minibatch_size, fprop_data[pl][port_index[i]]->width());
				//cout << modules[pl]->getPorts()[i] << endl;
				for (int j = 0; j < minibatch_size; ++j)
					(*mat)(j) << (*fprop_data[pl][port_index[i]])(layer_sizes[pl]*j + layer_sizes[pl]-1);
			} else
				PLERROR("Data was requested for a port, but not computed!");
		}
	}

       //cout << "redirected " << *ports_value[port_redirects[0][0].first] << endl;
	//cout << "ffprop end" << endl;
       Profiler::end("full fprop");

       //Profiler::report(cout);
}


//! Optimized fprop
void TreeDBNModule::fprop(const TVec<Mat*>& ports_value)
{
	if (propagate_gradient && propagate_full_gradient) {
		full_fprop(ports_value);
		return;
	}

       Profiler::start("full fprop");
       mats.resize(0);

       vector <string> prts = modules[0]->getPorts();
       //cout << "*********************" << endl;
       //for (int i = 0; i < prts.size(); ++i)
       //      cout << prts[i] << endl;
       //cout << "*********************" << endl;

	Mat* input = ports_value[getPortIndex("input")];
	int minibatch_size = input->length();
	int symbol_size = modules[0]->visible_layer->size/n_parents_per_node;

	mbi_time.resize(minibatch_size);
	mod_batch_length.resize(n_layers, minibatch_size);

	// Compute pseudo-time
	Vec v = (*input)(0), v2;
	if ( last_full_input != NULL && !last_full_input.isEmpty() && check_shift( last_full_input, v, symbol_size ) )
		mbi_time[0] = time + 1;
	else
		mbi_time[0] = 0;

	for (int mbi = 1; mbi < minibatch_size; ++mbi)
	{
		// Two cases: either it is a shifted version of the previous
		// or it is a new word
		v = (*input)(mbi-1);    v2 = (*input)(mbi);
		if ( check_shift( v, v2, symbol_size ) )
			mbi_time[mbi] = mbi_time[mbi-1] + 1;
		else
			mbi_time[mbi] = 0;
	}

	// Process layerwise
	for (int layer = 0; layer < n_layers; ++layer)
	{
		fprop_data[layer].resize(modules[layer]->nPorts());
		fprop_data[layer].fill((Mat*)NULL);

		// Count number of rows
		int nRows = 0;
		for (int mbi = 0; mbi < minibatch_size; ++mbi)
		{
			// We might need to compute either all or only last rbm
			if (mbi_time[mbi] < step_size[layer]) nRows += layer_sizes[layer];
			else ++nRows;
		}

		// Prepare matrices
		Mat* rbm_visible = createMatrix(nRows, modules[layer]->visible_layer->size, mats);
		fprop_data[layer][modules[layer]->getPortIndex("visible")] = rbm_visible;

		//Create all .state matrices
		for (int i = 0; i < modules[layer]->nPorts(); ++i) {
			string pname = modules[layer]->getPorts()[i];
			if ( pname.length() > 6 && ".state" == pname.substr(pname.length()-6) ) {
				if (fprop_data[layer][i] == NULL)
					fprop_data[layer][i] = createMatrix(0, 0, mats);
			}
		}

		//fprop_data[layer][modules[layer]->getPortIndex("hidden.state")] = createMatrix(0, 0, mats);
		//fprop_data[layer][modules[layer]->getPortIndex("hidden_activations.state")] = createMatrix(0, 0, mats);

		if (modules[layer]->reconstruction_connection == NULL) {
			fprop_data[layer][modules[layer]->getPortIndex("reconstruction_error.state")] = NULL;
			fprop_data[layer][modules[layer]->getPortIndex("visible_reconstruction.state")] = NULL;
			fprop_data[layer][modules[layer]->getPortIndex("visible_reconstruction_activations.state")] = NULL;
		}

		// Create empty matrices for forwarded ports
		for (int i = 0; i < nPorts(); ++i) {
			if (port_rbms[i] >= 0) {
				if (ports_value[i] != NULL && fprop_data[port_rbms[i]][port_index[i]] == NULL)
					fprop_data[port_rbms[i]][port_index[i]] = createMatrix(0, 0, mats);
			}
		}

		// Go through all minibatch and fill visible expectations
		if (layer == 0)
		{                               // Handle input layer in different manner
			int visible_size = modules[layer]->visible_layer->size;

			for (int mbi = 0, index = 0; mbi < minibatch_size; ++mbi)
			{
				// We might need to compute either all or only last rbm
				if (mbi_time[mbi] < step_size[layer]) {
					// Compute all rbms
					for (int i = 0; i < layer_sizes[layer]; ++i)
					{
						(*rbm_visible)(index++) << (*input)(mbi).subVec(i*visible_size, visible_size);
					}
				} else {
					// Compute only last rbm
					(*rbm_visible)(index++) << (*input)(mbi).subVec((layer_sizes[layer]-1)*visible_size, visible_size);
				}
				mod_batch_length[0][mbi] = index;
			}
		}
		else
		{
			// Take parent layer expectations
			Mat *expectations = fprop_data[layer-1][modules[layer-1]->getPortIndex("hidden.state")];
			Mat *expectations_cache = fprop_data_cache[layer-1][modules[layer-1]->getPortIndex("hidden.state")];

			int parent_width = modules[layer-1]->hidden_layer->size;
			for (int mbi = 0, index = 0; mbi < minibatch_size; ++mbi)
			{
				// We might need to compute either all or only last rbm
				if (mbi_time[mbi] < step_size[layer]) {
					// Compute all rbms
					for (int i = 0; i < layer_sizes[layer]; ++i)
					{
						for (int parent = 0; parent < n_parents_per_node; ++parent) {
							int row_id = mod_batch_length[layer-1][mbi] - hash(mbi_time[mbi], layer-1, n_parents_per_node*i + parent);
							//cout << "RID*: " << row_id << endl;
							if (row_id < 0) {
								// It must be in cache
								PLASSERT_MSG(expectations_cache, "Cache is NULL");
								int row_in_cache = expectations_cache->length()+row_id;
								PLASSERT_MSG(row_in_cache >= 0, "Cache is provided but is too small");
								(*rbm_visible)(index).subVec(parent*parent_width, parent_width) <<
										(*expectations_cache)(row_in_cache);
							} else {
								(*rbm_visible)(index).subVec(parent*parent_width, parent_width) <<
										(*expectations)(row_id);
							}
						}
						++index;
					}
				} else {
					// Compute only last rbm
					for (int parent = 0; parent < n_parents_per_node; ++parent) {
						int row_id = mod_batch_length[layer-1][mbi] - hash(mbi_time[mbi], layer-1, n_parents_per_node*(layer_sizes[layer]-1) + parent);
						//cout << "RID: " << row_id << endl;
						//cout << mbi_time[mbi] << " " << mod_batch_length[mbi] << " " << hash(mbi_time[mbi], layer-1, 2*(layer_sizes[layer]-1) + parent) << " "<< row_id << endl;
						if (row_id < 0) {
							// It must be in cache
							PLASSERT_MSG(expectations_cache, "Cache is NULL");
							int row_in_cache = expectations_cache->length()+row_id;
							PLASSERT_MSG(row_in_cache >= 0, "Cache is provided but is too small");
							(*rbm_visible)(index).subVec(parent*parent_width, parent_width) <<
									(*expectations_cache)(row_in_cache);
						} else {
							(*rbm_visible)(index).subVec(parent*parent_width, parent_width) <<
									(*expectations)(row_id);
						}
					}
					++index;
				}
				mod_batch_length[layer][mbi] = index;
			}
		}

		Profiler::start("fprop");
		//cout << "fprop: " << endl;
		//cout << (*fprop_data[layer][0]) << endl;
		//cout << "************" << endl;
		modules[layer]->fprop(fprop_data[layer]);
		Profiler::end("fprop");
	}

	time = mbi_time[minibatch_size-1];
	last_full_input.resize(input->width());
	last_full_input << (*input)(minibatch_size-1);

	// Final things: fill the cache...
	if (!propagate_gradient || !propagate_full_gradient)
		updateCache();

	// and write all required output to the provided ports ( output_i + requested )
	for (int i = 0; i < nPorts(); ++i) {
		Mat *mat = ports_value[i];

		if ( mat != NULL && mat->isEmpty() ) {
			// We check of which layer output should be writen to the port
			int pl = port_rbms[i];
			if (pl >= 0) {
				mat->resize(minibatch_size, fprop_data[pl][port_index[i]]->width());
				for (int j = 0; j < minibatch_size; ++j)
					(*mat)(j) << (*fprop_data[pl][port_index[i]])(mod_batch_length[pl][j] - 1);
			} else
				PLERROR("Data was requested for a port, but not computed!");
		}
	}

       //cout << "redirected " << *ports_value[port_redirects[0][0].first] << endl;

       Profiler::end("full fprop");

       //Profiler::report(cout);
}

//! Updates a cache with new fprop_data
void TreeDBNModule::updateCache()
{
       //cache_mats.resize(0);
       for (int i = 0; i < n_layers; ++i) {
               int n_ports = modules[i]->nPorts();
               for (int j = 0; j < n_ports; ++j) {

                       if (fprop_data[i][j] != NULL && !fprop_data[i][j]->isEmpty()) {
                               // Take last rows
                               int max_rows = layer_sizes[0]*n_parents_per_node;               // max we could need
                               if (fprop_data[i][j]->length() > max_rows) {
                                       //cout << "full cache" << endl;
                                       // copy submatrix
                                       if (fprop_data_cache[i][j] == NULL)
                                               fprop_data_cache[i][j] = createMatrix(max_rows, fprop_data[i][j]->width(), cache_mats);
                                       else
                                               fprop_data_cache[i][j]->resize(max_rows, fprop_data[i][j]->width());
                                       *fprop_data_cache[i][j] << fprop_data[i][j]->subMatRows(fprop_data[i][j]->length()-max_rows, max_rows);
                               } else {
                                       if (fprop_data_cache[i][j] == NULL) {           // have no cache, copy all
                                               //cout << "first cache " << i << " " << j << endl;
                                               fprop_data_cache[i][j] = createMatrix(fprop_data[i][j]->length(), fprop_data[i][j]->width(), cache_mats);
                                               *fprop_data_cache[i][j] << *fprop_data[i][j];
                                       } else {
                                               //cout << "part cache" << endl;
                                               // had something.., check how many rows we have to leave
                                               int rows_reuse = min(max_rows - fprop_data[i][j]->length(), fprop_data_cache[i][j]->length());
                                               Mat tmp(rows_reuse, fprop_data[i][j]->width());
                                               tmp << fprop_data_cache[i][j]->subMatRows(fprop_data_cache[i][j]->length() - rows_reuse, rows_reuse);
                                               fprop_data_cache[i][j]->resize(rows_reuse + fprop_data[i][j]->length(), fprop_data[i][j]->width());
                                               fprop_data_cache[i][j]->subMatRows(0, rows_reuse) << tmp;
                                               fprop_data_cache[i][j]->subMatRows(rows_reuse, fprop_data[i][j]->length()) << *fprop_data[i][j];
                                       }
                               }
                       }

                       // TODO if we stop calculate fprop_data for some port the cache should be deleted (?)
               }
       }
}

//! Clears the cache. Do this if parameters changed.
void TreeDBNModule::clearCache()
{
       time = 0;
       cache_mats.resize(0);
       for (int i = 0; i < n_layers; ++i) {
               int n_ports = modules[i]->nPorts();
               for (int j = 0; j < n_ports; ++j) {
                       fprop_data_cache[i][j] = NULL;
                       bprop_data_cache[i][j] = NULL;
               }
       }
}

//! Initializes sampling. Basically, it writes a random value to the
//! top rbm and does gibbsTop gibbs steps. Call this before calling sample().
void TreeDBNModule::initSampling(int gibbsTop)
{
       modules[n_layers-1]->min_n_Gibbs_steps = gibbsTop;

       Mat hidden(1, modules[n_layers-1]->hidden_layer->size);

       for (int i = 0; i < modules[n_layers-1]->hidden_layer->size; ++i)
       {
               hidden[0][i] = rand() & 1;
       }

       Mat exp;
       TVec <Mat*> fprop_data(modules[n_layers-1]->nPorts(), (Mat*)NULL);

       fprop_data[modules[n_layers-1]->getPortIndex("hidden_sample")] = &hidden;
       fprop_data[modules[n_layers-1]->getPortIndex("visible_sample")] = &exp;

       // Initialize with random sample
       modules[n_layers-1]->fprop(fprop_data);

       // Run chain for min_n_Gibbs_steps
       fprop_data.fill((Mat*)NULL);
       exp.resize(0,0);
       fprop_data[modules[n_layers-1]->getPortIndex("visible_sample")] = &exp;
       modules[n_layers-1]->fprop(fprop_data);
}


//! Returns a sample from the visible layer.
Vec TreeDBNModule::sample(int gibbsTop)
{
	modules[n_layers-1]->n_Gibbs_steps_per_generated_sample = gibbsTop;

       // Sample visible expectations from top layer rbm
       TVec <Mat> samples(n_layers);

       TVec <Mat*> fprop_data(modules[n_layers-1]->nPorts(), (Mat*)NULL);

       fprop_data[modules[n_layers-1]->getPortIndex("visible_sample")] = &samples[n_layers-1];

       modules[n_layers-1]->fprop(fprop_data);

       // Propagate expectations down the network
       for (int layer = n_layers-2; layer >= 0; --layer)
       {
               // Fill hidden sample for layer rbms
               int width = modules[layer]->hidden_layer->size;
               Mat hidden_sample(layer_sizes[layer], width);
               for (int i = 0; i < layer_sizes[layer]; ++i)
               {
                       hidden_sample(i) << samples[layer+1](i/n_parents_per_node).subVec((i%n_parents_per_node)*width, width);
               }

               TVec <Mat*> fp_data(modules[layer]->nPorts(), (Mat*)NULL);
               //fp_data[modules[layer]->getPortIndex("visible_reconstruction.state")] = &samples[layer];
               //fp_data[modules[layer]->getPortIndex("hidden.state")] = &hidden_sample;
               fp_data[modules[layer]->getPortIndex("visible_sample")] = &samples[layer];
               fp_data[modules[layer]->getPortIndex("hidden_sample")] = &hidden_sample;

               modules[layer]->fprop(fp_data);
       }

       Vec sample(samples[0].size());
       for (int i = 0; i < samples[0].length(); ++i)
               sample.subVec(i*samples[0].width(), samples[0].width()) << samples[0](i);

       return sample;
}

//////////////////
// getPortIndex //
//////////////////
/* Optional
int TreeDBNModule::getPortIndex(const string& port)
{}
*/

//////////////
// getPorts //
//////////////
const TVec<string>& TreeDBNModule::getPorts() {
       return port_names;
}

//////////////////
// getPortSizes //
//////////////////
/* Optional
const TMat<int>& TreeDBNModule::getPortSizes() {
}
*/

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void TreeDBNModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
   inherited::makeDeepCopyFromShallowCopy(copies);

   // ### Call deepCopyField on all "pointer-like" fields
   // ### that you wish to be deepCopied rather than
   // ### shallow-copied.
   // ### ex:
   deepCopyField(modules, copies);

   // ### Remove this line when you have fully implemented this method.
   //PLERROR("TreeDBNModule::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/////////////////////
// setLearningRate //
/////////////////////
/* OPTIONAL
// The default implementation raises a warning and does not do anything.
void TreeDBNModule::setLearningRate(real dynamic_learning_rate)
{
}
*/


}
// end of namespace PLearn


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
