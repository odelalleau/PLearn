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

namespace PLearn {
  using namespace std;



  /** BPTTVariable **/

  BPTTVariable::BPTTVariable(VarArray params, SequenceVMatrix* the_seqs, int bs, 
			     TMat<int> the_links, int the_nunits_input, int the_nunits_hidden,
			     int the_nunits_output, real the_alpha, TVec<string> the_units_type, 
			     string the_cost_type)
    :NaryVariable(params, 1, 1)
  {
    cout << "BPTTVariable" << endl;
    seqs = the_seqs;
    batch_size = bs;
    links = the_links;
    currpos = 0;
    nunits_input = the_nunits_input;
    nunits_hidden = the_nunits_hidden;
    nunits_output = the_nunits_output;
    alpha = the_alpha;
    nunits = nunits_input + nunits_hidden + nunits_output;
    units_type = the_units_type;
    cost_type = the_cost_type;
    
    updateIndexDest();
    updateOrder();
    if (cost_type == "MSE") {
      resize(1,1);
    }
  }

  real BPTTVariable::get_neuron(int i, int j) {
    return neuron(i, j);
  }

  real BPTTVariable::get_cost(int i, int j) {
    return cost(i, j);
  }

  real BPTTVariable::get_gradient(int i, int j) {
    return gradient(i, j);
  }

  int BPTTVariable::get_indexDest(int i, int j) {
    return indexDest(i, j);
  }

  void BPTTVariable::set_neuron(int i, int j, real val) {
    real *r = neuron[i] + j;
    *r = val;
  }

  void BPTTVariable::set_cost(int i, int j, real val) {
    real *r = cost[i] + j;
    *r = val;
  }

  void BPTTVariable::set_gradient(int i, int j, real val) {
    real *r = gradient[i] + j;
    *r = val;
  }

  void BPTTVariable::set_indexDest(int i, int j, int val) {
    int *r = indexDest[i] + j;
    *r = val;
  }


  PLEARN_IMPLEMENT_OBJECT(BPTTVariable, "This var contains all the recurrent network", "This var contains all the recurrent network.");

  void BPTTVariable::updateOrder() {
    order = TVec<int>(nunits);
    TVec<int> mark = TVec<int>(nunits, 0); // 0 = unmark, 1 = mark
    int orderCount = 0;
    for (int i = nunits - 1; i >= 0; i--) {
      if (mark[i] == 0) {
	topsort(i, mark, &orderCount);
      }
    }
  }

  void BPTTVariable::topsort(int v, TVec<int> mark, int* orderCount) {
    mark[v] = 1;
    for (int i = 1; i <= get_indexDest(v, 0); i++) {
      int edge_num = get_indexDest(v, i);
      int src_neuron = links(edge_num, 0);
      if (mark[src_neuron] == 0) {
	topsort(src_neuron, mark, orderCount);
      }
    }
    order[*orderCount] = v;
    (*orderCount)++;
  }

  void BPTTVariable::updateIndexDest() {
    /* Create the map of links by the destination neuron.
       Remember that index[n][0] contains the number of links that comes
       to the neuron n */
    cout << "BPTTVariable" << endl;
    int nr = links.nrows();
    indexDest = TMat<int>(nunits,nr+1, 0);
    for (int i = 0; i < links.nrows(); i++) {
      int dst = links(i,1);
      int nb = get_indexDest(dst, 0) + 1;
      set_indexDest(dst, 0, nb);
      set_indexDest(dst, nb, i);
    }

    for (int i = 0; i < nunits; i++) {
      cout << "Le neuron " << i << " est connecte de ";
      for (int j = 1; j <= get_indexDest(i, 0); j++) {
	int edge_num = get_indexDest(i,j);
	int src_neuron = links(edge_num, 0);
	cout << src_neuron << " ";
      }
      cout << endl;
    }
  }


  void BPTTVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {

  }

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
	  if (isInput(v) && row[v] != MISSING_VALUE) {
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
	  }
	}
	computeCost(t, row);
	t++;
      }
    }
  }

  void BPTTVariable::bprop()
  { fbprop(); }
  
  
  void BPTTVariable::fbprop() {
    fprop();

    int l = seqs->getNbSeq() - currpos; // Number of seqs remaining
    int nsamples = batch_size>0 ? batch_size : l;
    int ncol = seqs->inputsize() + seqs->targetsize() + seqs->weightsize();
    gradient = Mat(neuron_size, nunits, (real)0.0);

    Var weights = varray[0];
    int t = neuron_size - 1;
    
    for (int i = currpos + nsamples - 1; i >= currpos; i--) { // Backward loop on seqs
      int nrow = seqs->getNbRowInSeq(i);
      Mat s = Mat(nrow, ncol);
      seqs->getSeq(i, s);
      for (int j = s.nrows() - 1; j >= 0; j--) {
	Vec row = s(j);
	for (int o = nunits - 1; o >= 0; o--) { // Backward loop on each units(neuron)
	  int u = order[o];
	  if (isOutput(u)) {
	    real err = computeGradErr(get_neuron(t, u),row[u-nunits_hidden]);
	    set_gradient(t, u, get_gradient(t, u) + err);
	  }
	  real der = squash_d(u, get_neuron(t, u));
	  set_gradient(t, u, der * get_gradient(t, u));
	  real delta = get_gradient(t, u);
	  for (int e = 1; e <= get_indexDest(u, 0); e++) {
	    int edge_num = get_indexDest(u,e);
	    int src_neuron = links(edge_num, 0);
	    int delay = links(edge_num, 2);
	    if (t - delay >= 0) {
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
  }

  void BPTTVariable::nextBatch() {
    currpos += batch_size;
  }

  void BPTTVariable::updateWeights() {
    Var weights = varray[0];
    for (int w = 0; w < links.nrows(); w++) {
      weights->value[w] = weights->value[w] + weights->gradient[w] * alpha;
    }    
  }

  void BPTTVariable::updateGradient() {
    Var weights = varray[0];
    for (int w = 0; w < links.nrows(); w++) {
      int src_neuron = links(w, 0);
      int dst_neuron = links(w, 1);
      int delay = links(w, 2);
      real total = 0.0;
      for (int t = delay; t < neuron_size; t++) {
	real g = get_gradient(t, dst_neuron);
	real x = get_neuron(t-delay, src_neuron);
	total += g * x;
      }
      weights->gradient[w] = total;
    }
  }

  real BPTTVariable::computeGradErr(real o, real t) {
    if (cost_type == "MSE") {
      return o - t;
    }
    return 0.0;
  }

  void BPTTVariable::computeCost(int t, Vec row) {
    if (cost_type == "MSE") {
      for (int i = 0; i < seqs->targetsize(); i++) {
	real target = row[seqs->inputsize() + i];
	real out = get_neuron(t, nunits - seqs->targetsize() + i);
	if (target != MISSING_VALUE && out != MISSING_VALUE) {
	  set_cost(t, i, (out - target) * (out - target) / 2.0);
	  value[0] += get_cost(t, i);
	}
      }
    }
  }

  real BPTTVariable::squash_d(int v, real r) {
    if (units_type[v] == "TANH")
      return 1.0 - r * r;
    else if (units_type[v] == "LIN")
      return 1.0;
    PLERROR("BPTTVariable : The cost_type %s is unknown", cost_type[v]);
    return 0.0;    
  }
  
  real BPTTVariable::squash(int v, real r) {
    if (units_type[v] == "TANH")
      return tanh(r);
    else if (units_type[v] == "LIN")
      return r;
    PLERROR("BPTTVariable : The cost_type %s is unknown", cost_type[v]);
    return 0.0;
  }
  
  void BPTTVariable::printState() {
    Var weights = varray[0];
    for (int t = 0; t < neuron_size; t++) {
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
	if (gradient[t][n] < 10) cout << " ";
	if (gradient[t][n] < 100) cout << " ";
	cout << setprecision(3) << gradient[t][n] << " ";
      }
      cout << endl;
    }
    for (int w = 0; w < links.nrows(); w++) {
      int src_neuron = links(w, 0);
      int dst_neuron = links(w, 1);      
      int delay = links(w, 2);
      cout << "w(" << src_neuron << "," << dst_neuron << "," << 
	delay << ")=" <<  weights->value[w] << endl;
    }
    cout << "alpha : " << alpha << endl;
  }  

  void BPTTVariable::printOrder() {
    for (int i = 0; i < nunits; i++) {
      cout << order[i] << endl;
    }
  }
  
  void BPTTVariable::symbolicBprop()
  {
    
  }
  
  
  void BPTTVariable::rfprop()
  {
    
  }
  
} // end of namespace PLearn


  
