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

// Roughly copy from a code of Christian Jauvin(jauvinc@iro) that Implemented the EM
// method for text generation.
// I (Jasmin) adapt it for music generation and to PLearn.

#ifndef EMPLEARNER_INC
#define EMPLEARNER_INC

#include "SequencePLearner.h"
#include "SequenceVMatrix.h"

namespace PLearn {
using namespace std;

  class EMPLearner: public SequencePLearner
  {
  protected:

    typedef map<int, real> SimpleR;
    typedef map<int, int> SimpleI;
    typedef map<int, SimpleR> DoubleR;
    typedef map<int, SimpleI> DoubleI;
    typedef map<int, DoubleR> TripleR;
    typedef map<int, DoubleI> TripleI;

    int max_n_bins;
    real log_base;

    real zero_gram;
    int note_count;

    Mat bigram_mixture, bigram_mixture_posterior;
    Mat trigram_mixture, trigram_mixture_posterior;

    SequenceVMatrixStream train_stream;
    SequenceVMatrixStream valid_stream;
    SequenceVMatrixStream test_stream;
    

    SimpleI cunigram;
    SimpleR punigram;
    DoubleI cbigram;
    DoubleR pbigram;
    TripleI ctrigram;
    TripleR ptrigram;

    void init_prob();
    void verify_prob();
    void init_mixture();
    void update_mixture();
    void clear_mixture_posterior();
    int mapFrequency(int, int);

    int get_cunigram(int);
    int get_cbigram(int,int);
    int get_ctrigram(int,int,int);

    real get_punigram(int);
    real get_pbigram(int,int);
    real get_ptrigram(int,int,int);

    real get_zerogram();

    int get_notecount();

    void one_step_train();
    void one_step_valid();
    void one_step_test();

  public:

    typedef SequencePLearner inherited;

  private:
    void build_();

  public:

    EMPLearner();
    virtual ~EMPLearner();
    PLEARN_DECLARE_OBJECT(EMPLearner);

    virtual void build();
    virtual void forget(); // simply calls initializeParams()

    virtual TVec<string> getTrainCostNames() const;
    virtual TVec<string> getTestCostNames() const;

    virtual void train();

    // The vec version of compute function cannot be use in a Sequence learner
    virtual void computeOutput(const Mat&, Mat&) const;
    
    virtual void computeCostsFromOutputs(const Mat&, const Mat&, const Mat&, Mat&) const;

    virtual void run();
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  protected:
    static void declareOptions(OptionList& ol);
    void initializeParams();

  };

  DECLARE_OBJECT_PTR(EMPLearner);

} // end of namespace PLearn

#endif

