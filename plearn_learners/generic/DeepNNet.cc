// -*- C++ -*-

// DeepNNet.cc
//
// Copyright (C) 2005 Yoshua Bengio 
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

/* *******************************************************      
   * $Id: DeepNNet.cc,v 1.2 2005/01/18 04:50:56 yoshua Exp $ 
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file DeepNNet.cc */


#include "DeepNNet.h"

namespace PLearn {
using namespace std;

DeepNNet::DeepNNet() 
/* ### Initialize all fields to their default value here */
  : n_layers(3),
    n_units_per_layer(10),
    L1_regularizer(1e-5),
    initial_learning_rate(1e-4),
    learning_rate_decay(1e-6),
    output_cost("mse"),
    add_connections(true),
    remove_connections(true),
    initial_sparsity(0.9);
{
}

PLEARN_IMPLEMENT_OBJECT(DeepNNet, 
                        "Deep multi-layer neural networks with sparse adaptive connections", 
                        "This feedforward neural network can have many layers, but its weight\n"
                        "matrices are sparse and can be optionally adapted (adding new connections\n"
                        "where that would create the largest gradient).");

void DeepNNet::declareOptions(OptionList& ol)
{
  declareOption(ol, "n_layers", &DeepNNet::n_layers, OptionBase::buildoption,
                "Number of layers, including the output but not input layer");

  declareOption(ol, "default_n_units_per_hidden_layer", &DeepNNet::default_n_units_per_hidden_layer, 
                OptionBase::buildoption, "If n_units_per_layer is not specified, it is given by this value for all hidden layers");

  declareOption(ol, "n_units_per_layer", &DeepNNet::n_units_per_layer, OptionBase::buildoption,
                "Number of units per layer, including the output but not input layer.\n"
                "The last (output) layer number of units is overridden by the outputsize option");

  declareOption(ol, "L1_regularizer", &DeepNNet::L1_regularizer, OptionBase::buildoption,
                "amount of penalty on sum_{l,i,j} |weights[l][i][j]|");

  declareOption(ol, "initial_learning_rate", &DeepNNet::initial_learning_rate, OptionBase::buildoption,
                "learning_rate = initial_learning_rate/(1 + iteration*learning_rate_decay)\n"
                "where iteration is incremented after each example is presented");

  declareOption(ol, "learning_rate_decay", &DeepNNet::learning_rate_decay, OptionBase::buildoption,
                "see the comment for initial_learning_rate.");

  declareOption(ol, "output_cost", &DeepNNet::output_cost, OptionBase::buildoption,
                "String-valued option specifies output non-linearity and cost:\n"
                "  'mse': mean squared error for regression with linear outputs\n"
                "  'nll': negative log-likelihood of P(class|input) with softmax outputs");

  declareOption(ol, "add_connections", &DeepNNet::add_connections, OptionBase::buildoption,
                "whether to add connections when the potential connections average absolute"
                "gradient becomes larger than that of existing connections");

  declareOption(ol, "remove_connections", &DeepNNet::remove_connections, OptionBase::buildoption,
                "whether to remove connections when their weight becomes too small");

  declareOption(ol, "initial_sparsity", &DeepNNet::initial_sparsity, OptionBase::buildoption,
                "initial fraction of weights that are set to 0.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void DeepNNet::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.

  // these would be -1 if a train_set has not be set already
  if (inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
  {
    activations.resize(n_layers);
    if (sources.length() != n_layers) // in case we are called after loading the object we don't need to do this:
    {
      if (n_units_per_layer.length()==0)
      {
        n_units_per_layer.resize(n_layers);
        for (int l=0;l<n_layers;l++)
          n_units_per_layer[l] = default_n_units_per_layer;
      }
      sources.resize(n_layers);
      weights.resize(n_layers);
      biases.resize(n_layers);
      for (int l=0;l<n_layers;l++)
      {
        sources[l].resize(n_units_per_layer[l]);
        weights[l].resize(n_units_per_layer[l]);
        biases[l].resize(n_units_per_layer[l]);
        int n_previous = (l==0)? int((1-initial_sparsity)*inputsize_) :
          int((1-initial_sparsity)*n_units_per_layer[l-1]);
        for (int i=0;i<n_units_per_layer[l];i++)
        {
          sources[l][i].resize(n_previous);
          weights[l][i].resize(n_previous);
        }
      }
      initializeParams();
    }
    if (add_connections)
    {
      gradients.resize(n_layers);
      avg_gradients_norm.resize(n_layers);
    }

  }

}

// ### Nothing to add here, simply calls build_
void DeepNNet::build()
{
  inherited::build();
  build_();
}


void DeepNNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("DeepNNet::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int DeepNNet::outputsize() const
{
  // Compute and return the size of this learner's output (which typically
  // may depend on its inputsize(), targetsize() and set options).
}

void DeepNNet::forget()
{
  //! (Re-)initialize the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    /*!
      A typical forget() method should do the following:
         - initialize a random number generator with the seed option
         - initialize the learner's parameters, using this random generator
         - stage = 0
    */
}
    
void DeepNNet::train()
{
    // The role of the train method is to bring the learner up to stage==nstages,
    // updating train_stats with training costs measured on-line in the process.

    /* TYPICAL CODE:

      static Vec input  // static so we don't reallocate/deallocate memory each time...
      static Vec target // (but be careful that static means shared!)
      input.resize(inputsize())    // the train_set's inputsize()
      target.resize(targetsize())  // the train_set's targetsize()
      real weight

      if(!train_stats)  // make a default stats collector, in case there's none
         train_stats = new VecStatsCollector()

      if(nstages<stage) // asking to revert to a previous stage!
         forget()  // reset the learner to stage=0

      while(stage<nstages)
        {
          // clear statistics of previous epoch
          train_stats->forget() 
          
          //... train for 1 stage, and update train_stats,
          // using train_set->getSample(input, target, weight)
          // and train_stats->update(train_costs)
          
          ++stage
          train_stats->finalize() // finalize statistics for this epoch
        }
    */
}


void DeepNNet::computeOutput(const Vec& input, Vec& output) const
{
  // Compute the output from the input.
  // int nout = outputsize();
  // output.resize(nout);
  // ...
}    

void DeepNNet::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output. 
// ...
}                                

TVec<string> DeepNNet::getTestCostNames() const
{
  // Return the names of the costs computed by computeCostsFromOutpus
  // (these may or may not be exactly the same as what's returned by getTrainCostNames).
  // ...
}

TVec<string> DeepNNet::getTrainCostNames() const
{
  // Return the names of the objective costs that the train method computes and 
  // for which it updates the VecStatsCollector train_stats
  // (these may or may not be exactly the same as what's returned by getTestCostNames).
  // ...
}


} // end of namespace PLearn
