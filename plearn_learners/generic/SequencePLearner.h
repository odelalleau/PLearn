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

#ifndef SEQUENCEPLEARNER_INC
#define SEQUENCEPLEARNER_INC

#include "PLearner.h"
#include "SequenceVMatrix.h"

namespace PLearn {
using namespace std;

 class SequencePLearner: public PLearner
  {
  protected:
    VarArray params;
    SequenceVMat test_set;
    
  public:

    typedef PLearner inherited;

    int batch_size; // how many samples to use to estimate gradient before an update
                    // 0 means the whole training set (default: 1)

  private:
    void build_();

  public:

    SequencePLearner();
    virtual ~SequencePLearner();
    PLEARN_DECLARE_ABSTRACT_OBJECT(SequencePLearner);

    virtual void build();
    virtual void forget()=0;

    virtual int outputsize() const;
    virtual int outputsize(VMat) const; // Give the outputsize depending on the VMat(must be a SequenceVMatrix) set
    virtual TVec<string> getTrainCostNames() const =0;
    virtual TVec<string> getTestCostNames() const =0;

    virtual void train()=0;

    // The vec version of compute function cannot be use in a Sequence learner
    virtual void computeOutput(const Vec&, Vec&) const;
    
    // SUB-CLASS WRITTING
    virtual void computeOutput(const Mat&, Mat&) const =0;
    
    virtual void computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) const;

    // SUB-CLASS WRITTING
    virtual void computeCostsFromOutputs(const Mat&, const Mat&, const Mat&, Mat&) const =0;

    virtual void computeCostsOnly(const Vec&, const Vec&, Vec&) const;
    virtual void computeCostsOnly(const Mat&, const Mat&, Mat&) const;

    virtual void computeOutputAndCosts(const Vec&, const Vec&, Vec&, Vec&) const;
    virtual void computeOutputAndCosts(const Mat&, const Mat&, Mat&, Mat&) const;

    /*
      These two functions work together. init_step must be called to initialise
      the inputs to be able to generate the other step.
      In a sequencePLearner, we use the output of step t-1 as an input of step t.
      With this input we can generate the ouput at step t.
      We can admit that these functions replace the compute function that are above.
    */
    virtual void get_next_step(Vec&) = 0;
    virtual void init_step(const Mat&) = 0;


    virtual void run();
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    void setTestSet(SequenceVMat);

    virtual void test(VMat testset, PP<VecStatsCollector> test_stats, 
                      VMat testoutputs=0, VMat testcosts=0) const;
    virtual void test(SequenceVMat testset, PP<VecStatsCollector> test_stats, 
                      SequenceVMat testoutputs, SequenceVMat testcosts) const;

  protected:
    static void declareOptions(OptionList& ol);

  };
  DECLARE_OBJECT_PTR(SequencePLearner);
} // end of namespace PLearn

// #include "EMPLearner.h" // Commented out because missing in the PLearn cvs.

#endif

