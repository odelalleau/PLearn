// -*- C++ -*-

// BPTT.h
// Copyright (c) 2004 Jasmin Lapalme
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

#ifndef BPTT_INC
#define BPTT_INC

#include "PLearner.h"
#include "BPTTVariable.h"
#include "Optimizer.h"

namespace PLearn {
using namespace std;

  class BPTT: public PLearner
  {
  protected:
    BPTTVariable rec_net;
    VarArray params;
    Vec weights, bias;
    TMat<int> links;
    int nneuron_input; // number of input neuron
    int nneuron_hidden; // number of hidden neuron
    int nneuron_output; // number of output neuron
    real alpha; // coef use when we update the weights
    TVec<string> units_type;
    string cost_type; // Function to minimize

    SequenceVMatrix temp;

  public:

    typedef PLearner inherited;

    PP<Optimizer> optimizer; // the optimizer to use (no default)

    int batch_size; // how many samples to use to estimate gradient before an update
                    // 0 means the whole training set (default: 1)


  private:
    void build_();

  public:

    BPTT();
    virtual ~BPTT();
    PLEARN_DECLARE_OBJECT(BPTT);

    virtual void build();
    virtual void forget(); // simply calls initializeParams()

    virtual int outputsize() const;
    virtual TVec<string> getTrainCostNames() const;
    virtual TVec<string> getTestCostNames() const;

    virtual void train();

    virtual void computeOutput(const Vec& input, Vec& output) const;
    
    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                       Vec& output, Vec& costs) const;

    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;


    virtual void makeDeepCopyFromShallowCopy(CopiesMap &copies);

    virtual void run();

  protected:
    static void declareOptions(OptionList& ol);
    void initializeParams();

  };

  DECLARE_OBJECT_PTR(BPTT);

} // end of namespace PLearn

#endif

