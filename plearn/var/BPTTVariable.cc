// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2004 Jasmin Lapalme

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


#include "BPTTVariable.h"
#include "PLMPI.h"
#include "DisplayUtils.h"
#include "pl_math.h"
#include "Var.h"
#include "TanhVariable.h"
#include "ExpVariable.h"
#include "IdentityVariable.h"

namespace PLearn {
  using namespace std;



  /** BPTTVariable **/

  BPTTVariable::BPTTVariable(VarArray params, SequenceVMatrix* the_seqs, int bs, 
			     TMat<int> the_links, int the_nunits_input, int the_nunits_hidden,
			     int the_nunits_output, TVec<string> the_units_type, 
			     string the_cost_type)
    :NaryVariable(params, 1, 1)
  {
    seqs = the_seqs;
    batch_size = bs;
    links = the_links;
    currpos = 0;
    nunits_input = the_nunits_input;
    nunits_hidden = the_nunits_hidden;
    nunits_output = the_nunits_output;
    nunits = nunits_input + nunits_hidden + nunits_output;
    units_type = the_units_type;
    cost_type = the_cost_type;
    
    updateIndexDest();
    updateOrder();
    buildSquashVar();

    if (cost_type == "MSE") {
      resize(1,1);
      gradient[0] = 0.0;
      value[0] = 0.0;
    }
  }

  /* get and set access to all the tables */

  real BPTTVariable::get_neuron(int i, int j) {
    return neuron(i, j);
  }

  real BPTTVariable::get_cost(int i, int j) {
    return cost(i, j);
  }

  real BPTTVariable::get_gradient(int i, int j) {
    return neu_gradient(i, j);
  }

  int BPTTVariable::get_indexDest(int i, int j) {
    return indexDest(i, j);
  }

  void BPTTVariable::set_neuron(int i, int j, real val) {
    neuron[i][j] = val;
  }

  void BPTTVariable::set_cost(int i, int j, real val) {
    cost[i][j] = val;
  }

  void BPTTVariable::set_gradient(int i, int j, real val) {
    neu_gradient[i][j] = val;
  }

  void BPTTVariable::set_indexDest(int i, int j, int val) {
    indexDest[i][j] = val;
  }


  PLEARN_IMPLEMENT_OBJECT(BPTTVariable, "This var contains all the recurrent network", "This var contains all the recurrent network.");

  /*
    Topological sort of the node in the network with the links of delay = 0.
    We assure that we do not calculate the value of a node without knowing the
    value of each of his ancestor.
  */
  void BPTTVariable::updateOrder() {
    order = TVec<int>(nunits);
    TVec<int> mark = TVec<int>(nunits, 0); // 0 = unmark, 1 = mark
    int orderCount = 0;
    for (int i = 0; i < nunits; i++) {
      if (mark[i] == 0) {
	topsort(i, mark, &orderCount);
      }
    }
  }

  /*
    Help function for the topological sort
  */

  void BPTTVariable::topsort(int v, TVec<int> mark, int* orderCount) {
    mark[v] = 1;
    for (int i = 1; i <= get_indexDest(v, 0); i++) {
      int edge_num = get_indexDest(v, i);
      int src_neuron = links(edge_num, 0);
      int delay = links(edge_num, 2);
      if (delay == 0 && mark[src_neuron] == 0) {
	topsort(src_neuron, mark, orderCount);
      }
    }
    order[*orderCount] = v;
    (*orderCount)++;
  }

  /*
    build the VarArray for the squash function of each node. Like this
    the test on units_type is done one time and not each node at each time
    step.
  */
  void BPTTVariable::buildSquashVar() {
    units_value = VarArray(nunits);
    squash_units_value = VarArray(nunits);
    squash_proppath = TVec<VarArray>(nunits);

    for (int i = 0; i < nunits; i++) {
      units_value[i] = var(0);
      if (units_type[i] == "TANH") {
	squash_units_value[i] = tanh(units_value[i]);
      } else if (units_type[i] == "EXP") {
	squash_units_value[i] = exp(units_value[i]);
      } else if (units_type[i] == "ID") {
	squash_units_value[i] = units_value[i];
      } else 
	PLERROR("%s is not a valide units_type", units_type[i].c_str());
      squash_proppath[i] = propagationPath(units_value[i], squash_units_value[i]);
    }
  }

  /*
    Create the map of links by the destination neuron.
    Remember that index[n][0] contains the number of links that comes
    to the neuron n
  */
  void BPTTVariable::updateIndexDest() {
    int nr = links.nrows();
    indexDest = TMat<int>(nunits,nr+1, 0);
    for (int i = 0; i < links.nrows(); i++) {
      int dst = links(i,1);
      int nb = get_indexDest(dst, 0) + 1;
      set_indexDest(dst, 0, nb);
      set_indexDest(dst, nb, i);
    }

  }


  void BPTTVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies) { }

  /*
    Does the forward pass on nsamples sequences
  */
  void BPTTVariable::fprop() {
    int l = seqs->getNbSeq() - currpos; // Number of seqs remaining
    int nsamples = batch_size>0 ? batch_size : l;
  
    Var weights = varray[0];
    Var bias = varray[1];
    neuron_size = seqs->getNbRowInSeqs(currpos, nsamples);
    neuron = Mat(neuron_size, nunits, (real)0.0);
    cost = Mat(neuron_size, seqs->targetsize(), (real)0.0);
    int ncol = seqs->inputsize() + seqs->targetsize() + seqs->weightsize();
    int t = 0;
    value[0] = 0; // Total_err

    for (int i = currpos; i < currpos + nsamples; i++) { // Loop on each seq
      int nrow = seqs->getNbRowInSeq(i);
      Mat s = Mat(nrow, ncol);
      seqs->getSeq(i, s);
      for (int j = 0; j < s.nrows(); j++) { // Loop on each example in seq
	Vec row = s(j);
	for (int o = 0; o < nunits; o++) { // Loop on each units(neuron)
	  int v = order[o];
	  if (isInput(v) && !is_missing(row[v])) {
	    set_neuron(t, v, row[v]);
	  } else {
	    real total = bias->value[v];
	    for (int e = 1; e <= get_indexDest(v,0); e++) { // Loop on each edge that comes to the neuron
	      int edge_num = get_indexDest(v,e);
	      int src_neuron = links(edge_num, 0);
	      int delay = links(edge_num, 2);
	      if (j - delay >= 0) {
		real w = weights->value[edge_num];
		real x = get_neuron(t-delay,src_neuron);
		total += w * x;
	      }
	    }
	    set_neuron(t, v, squash(v, total));
	  }
	}
	computeCost(t, row);
	t++;
      }
    }
  }
  
  /*
    Compute the output from an input sequence with the learn weights and bias
  */

  void BPTTVariable::computeOutputFromInput(const Mat &input, Mat &output) {
    Var weights = varray[0];
    Var bias = varray[1];
   
    neuron = Mat(input.nrows(), nunits, (real)0.0);
    for (int t = 0; t < input.nrows(); t++) { // Loop on each example in seq
      Vec row = input(t);
      for (int o = 0; o < nunits; o++) { // Loop on each units(neuron)
	int v = order[o];
	if (isInput(v) && !is_missing(row[v])) {
	  set_neuron(t, v, row[v]);
	} else {
	  real total = bias->value[v];
	  for (int e = 1; e <= get_indexDest(v,0); e++) { // Loop on each edge that comes to the neuron
	    int edge_num = get_indexDest(v,e);
	    int src_neuron = links(edge_num, 0);
	    int delay = links(edge_num, 2);
	    if (t - delay >= 0) {
	      real w = weights->value[edge_num];
	      real x = get_neuron(t-delay,src_neuron);
	      total += w * x;
	    }
	  }
	  set_neuron(t, v, squash(v, total));
	  if (isOutput(v)) {
	    output[t][v-nunits_input-nunits_hidden] = get_neuron(t, v);
	  }
	}
      }
    }
  }

  /*
    Compute the costs of each output units with a specified output and target
  */

  void BPTTVariable::computeCostFromOutput(const Mat &output, const Mat &target, Mat &costs) {
    for (int i = 0; i < output.nrows(); i++) {
      for (int j = 0; j < output.ncols(); j++) {
	costs[i][j] = computeErr(output[i][j], target[i][j]);
      }
    }
  }

  /*
    We call fbprop() because we cannot do a bprop without doing a fprop before
  */
  void BPTTVariable::bprop() { fbprop(); }
  
  /*
    Calls the fprop and after we calculate the gradient on each node and time step.
    With that we update the gradient of the weigths and bias.
  */
  void BPTTVariable::fbprop() {
    fprop();

    int l = seqs->getNbSeq() - currpos; // Number of seqs remaining
    int nsamples = batch_size>0 ? batch_size : l;
    int ncol = seqs->inputsize() + seqs->targetsize() + seqs->weightsize();
    neu_gradient = Mat(neuron_size, nunits, (real)0.0); // faire clear a la place

    Var weights = varray[0];
    int t = neuron_size - 1;

    for (int i = currpos + nsamples - 1; i >= currpos; i--) { // Backward loop on seqs
      int nrow = seqs->getNbRowInSeq(i);
      Mat s = Mat(nrow, ncol);
      seqs->getSeq(i, s);
      for (int j = s.nrows() - 1; j >= 0; j--) { // Backward loop on each row
	Vec row = s(j);
	for (int o = nunits - 1; o >= 0; o--) { // Backward loop on each units(neuron)
	  int u = order[o];
	  if (isOutput(u) && !is_missing(row[u-nunits_hidden])) {
	    real err = computeGradErr(get_neuron(t, u),row[u-nunits_hidden]) * gradient[0];
	    set_gradient(t, u, get_gradient(t, u) + err);
	  }
	  real der = squash_d(u, get_neuron(t, u));
	  set_gradient(t, u, der * get_gradient(t, u));
	  real delta = get_gradient(t, u);
	  for (int e = 1; e <= get_indexDest(u, 0); e++) {
	    int edge_num = get_indexDest(u,e);
	    int src_neuron = links(edge_num, 0);
	    int delay = links(edge_num, 2);
	    if (j - delay >= 0) {
	      real w = weights->value[edge_num];
	      real old_grad = get_gradient(t - delay, src_neuron);
	      set_gradient(t - delay, src_neuron, old_grad + w * delta);
	    }
	  }
	}
	t--;
      }
    }
    updateGradient();
    nextBatch();
  }

  /*
    update the currpos pointer to the next batch.
  */
  void BPTTVariable::nextBatch() {
    currpos += batch_size;
  }

  /*
    Update the gradient of each weight and bias.
  */
  void BPTTVariable::updateGradient() {
    Var weights = varray[0];
    Var bias = varray[1];
    for (int w = 0; w < links.nrows(); w++) {
      weights->gradient[w] = 0.0;
    }
    for (int u = 0; u < nunits; u++) {
      bias->gradient[u] = 0.0;
    }
    for (int t = 0; t < neuron_size; t++) {
      for (int w = 0; w < links.nrows(); w++) {
	int src_neuron = links(w, 0);
	int dst_neuron = links(w, 1);
	int delay = links(w, 2);
	if (t - delay >= 0) {
	  real g = get_gradient(t, dst_neuron);
	  real x = get_neuron(t-delay, src_neuron);
	  if (!is_missing(x))
	    weights->gradient[w] += g * x;
	}
      }
      for (int u = 0; u < nunits; u++) {
	if (!isInput(u))
	  bias->gradient[u] += get_gradient(t, u);
      }      
    }
  }

  /*
     Compute the gradient on the error with a given output and target.
     =====
     The test on the cost_type has to be change for a Var.
     ====
  */
  real BPTTVariable::computeGradErr(real o, real t) {
    if (cost_type == "MSE") {
      return o - t;
    }
    PLERROR("This cost type is unknown : %s", cost_type.c_str());
    return 0.0;
  }

  /*
     Compute the error with a given output and target.
     =====
     The test on the cost_type has to be change for a Var.
     ====
  */
  real BPTTVariable::computeErr(real o, real t) {
    if (cost_type == "MSE") {
      return (o - t) * (o - t) / 2.0;
    }
    PLERROR("This cost type is unknown : %s", cost_type.c_str());
    return 0.0;
  }

  /*
    calculate the overall cost on a row and add it to the total
    of all the pass in value[0](The output of BPTTVariable)
  */
  void BPTTVariable::computeCost(int t, Vec row) {
    if (cost_type == "MSE") {
      for (int i = 0; i < seqs->targetsize(); i++) {
	real target = row[seqs->inputsize() + i];
	real out = get_neuron(t, nunits - seqs->targetsize() + i);
	if (!is_missing(target) && !is_missing(out)) {
	  set_cost(t, i, computeErr(out,target));
	  value[0] += get_cost(t, i);
	}
      }
    }
  }

  /*
    Compute the derivative squash function
  */
  real BPTTVariable::squash_d(int v, real r) {
    squash_units_value[v] = r;
    units_value[v]->clearGradient();
    squash_proppath[v].clearGradient();
    squash_units_value[v]->fillGradient(1.0);
    squash_proppath[v].bprop();

    return units_value[v]->gradient[0];
  }

  /*
    Compute the squash function
  */
  real BPTTVariable::squash(int v, real r) {
    units_value[v] = r;
    squash_proppath[v].clearGradient();
    squash_proppath[v].fprop();

    return squash_units_value[v]->value[0];
  }

  /*
    For own use. Normally the Learner or Optimizer that use this var does it automatically.
    Update the weights.
  */
  void BPTTVariable::updateWeights() {
    Var weights = varray[0];
    for (int w = 0; w < links.nrows(); w++) {
      weights->value[w] += weights->gradient[w];
    }    
  }

  /*
    For own use. Normally the Learner or Optimizer that use this var does it automatically.
    Update the bias.
  */
  void BPTTVariable::updateBias() {
    Var bias = varray[1];
    for (int u = 0; u < nunits; u++) {
      bias->value[u] += bias->gradient[u];
    }    
  }
  
  /*
    Debug function. Print the state of the Var
  */
  void BPTTVariable::printState() {
    Var weights = varray[0];
    /*    for (int t = 0; t < neuron_size; t++) {
      cout << "t=";
      if (t < 10) cout << " ";
      if (t < 100) cout << " ";
      cout << t << " ";
      for (int n = 0; n < nunits; n++) {
	if (neuron[t][n] < 10) cout << " ";
	if (neuron[t][n] < 100) cout << " ";
	cout << setprecision(3) << neuron[t][n];
      }
      cout << endl;
    }
    cout << endl;
    for (int t = 0; t < neuron_size; t++) {
      cout << "t=";
      if (t < 10) cout << " ";
      if (t < 100) cout << " ";
      cout << t << " ";
      for (int n = 0; n < nunits; n++) {
	if (neu_gradient[t][n] < 10) cout << " ";
	if (neu_gradient[t][n] < 100) cout << " ";
	cout << setprecision(3) << neu_gradient[t][n] << " ";
      }
      cout << endl;
      }*/
    for (int w = 0; w < links.nrows(); w++) {
      int src_neuron = links(w, 0);
      int dst_neuron = links(w, 1);      
      int delay = links(w, 2);
      cout << "w(" << src_neuron << "," << dst_neuron << 
	delay << ")=" <<  weights->value[w] << endl;
    }

    cout << "alpha : " << gradient[0] << endl;
  }  

  /*
    print the order in witch, the fprop and bprop will see the node.
  */
  void BPTTVariable::printOrder() {
    for (int i = 0; i < nunits; i++) {
      cout << order[i] << endl;
    }
  }
  
  void BPTTVariable::symbolicBprop() {}
  
  
  void BPTTVariable::rfprop() {}
  
} // end of namespace PLearn


  
