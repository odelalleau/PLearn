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

#ifndef BPTTVariable_INC
#define BPTTVariable_INC

#include "NaryVariable.h"
#include "SequenceVMatrix.h"

namespace PLearn {
using namespace std;


class BPTTVariable: public NaryVariable
{
  protected:
    //!  protected default constructor for persistence

  Mat neuron;  // Values of each neuron (in, hidden, out) for each timestep
  Mat neu_gradient;  // Gradient values of each neuron (in, hidden, out) for each timestep
  Mat cost;  // The cost at each timestep, maybe there'll be just a cost a 
                  // the end depending on the type of cost
  int neuron_size; // length of the neuron array (i.e The number of time step)

  TMat<int> indexDest;
  /* This table is an map of all the edges witch the key
     is the no of the vertex of destination of the edge.
     indexDest[i][0] give the number of edge that comes to
     the vertex i. indexDest[i][1],..., indexDest[i][n] where
     n = indexDest[i][0], contains the no of edge that corresponds
     to the array graph.
  */
  TVec<int> order; // Contains the topological sort of the graph.
                   // This is the order we will calculate each value of each node

  void updateIndexDest();
  void updateOrder();
  void topsort(int, TVec<int>, int*);

  void computeCost(int, Vec);

  bool isInput(int v) { return v < nunits_input; }
  bool isOutput(int v) { return v >= nunits_input + nunits_hidden; }
  real inputValue(int, int);

  real get_neuron(int, int);
  real get_cost(int, int);
  real get_gradient(int, int);

  void set_neuron(int, int, real);
  void set_cost(int, int, real);
  void set_indexDest(int, int, int);
  void set_gradient(int, int, real);

  real squash(int, real);
  real squash_d(int, real);
  real computeGradErr(real, real);
  real computeErr(real, real);

  int currpos;

  public:

  TMat<int> links; 
  /* This table has a fixed width of 3 and a length that
     correspond to the number of edges. The first column
     is the no of vertex of origin, the second colum is the
     no of vertex of destination, the third is the delay of
     the edge.
  */
  int nunits; // number of units
  int nunits_input; // number of units
  int nunits_hidden; // number of units
  int nunits_output; // number of units
  real alpha; // coef. to calculate the weight update
  TVec<string> units_type;
  string cost_type; // Function to minimise
  SequenceVMatrix *seqs;
  int batch_size;    

  public:
  
  BPTTVariable() : currpos(0), batch_size(0) {}
  BPTTVariable(VarArray, SequenceVMatrix*, int, TMat<int> , int, int, int, 
	       TVec<string> , string);
  
  PLEARN_DECLARE_OBJECT(BPTTVariable);
  
  int get_indexDest(int, int);

  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual void fprop();
  virtual void bprop();
  virtual void fbprop();
  virtual void symbolicBprop();
  virtual void rfprop();  

  void updateGradient();
  void updateWeights();
  void updateBias();

  void computeOutputFromInput(const Mat&, Mat&);
  void computeCostFromOutput(const Mat&, const Mat&, Mat&);

  void nextBatch();

  void printState();
  void printOrder();
};
  
  DECLARE_OBJECT_PTR(BPTTVariable);

} // end of namespace PLearn

#endif 
