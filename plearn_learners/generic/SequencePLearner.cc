// -*- C++ -*-

// BPTT.cc
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

#include "AffineTransformVariable.h"
#include "AffineTransformWeightPenalty.h"
#include "BinaryClassificationLossVariable.h"
#include "ClassificationLossVariable.h"
#include "ConcatColumnsVariable.h"
#include "CrossEntropyVariable.h"
#include "ExpVariable.h"
#include "LiftOutputVariable.h"
#include "LogSoftmaxVariable.h"
#include "MulticlassLossVariable.h"
#include "NegCrossEntropySigmoidVariable.h"
#include "OneHotSquaredLoss.h"
#include "SigmoidVariable.h"
#include "SoftmaxVariable.h"
#include "SoftplusVariable.h"
#include "SumVariable.h"
#include "SumAbsVariable.h"
#include "SumOfVariable.h"
#include "SumSquareVariable.h"
#include "TanhVariable.h"
#include "TransposeProductVariable.h"
#include "Var_operators.h"
#include "Var_utils.h"

#include "ConcatColumnsVMatrix.h"
//#include "DisplayUtils.h"
#include "GradientOptimizer.h"
#include "BPTT.h"
#include "random.h"
#include "SubVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(SequencePLearner, "Abstract learner that learners with data set that are SequenceVMatrix", 
                        "Abstract learner that learners with data set that are SequenceVMatrix");

SequencePLearner::SequencePLearner() // DEFAULT VALUES FOR ALL OPTIONS
{
  batch_size = 1;
}

SequencePLearner::~SequencePLearner()
{
}

void SequencePLearner::declareOptions(OptionList& ol)
{
  declareOption(ol, "batch_size", &BPTT::batch_size, OptionBase::buildoption, 
                "    how many samples to use to estimate the average gradient before updating the weights\n"
                "    0 is equivalent to specifying training_set->length() \n");

  declareOption(ol, "params", &BPTT::params, OptionBase::learntoption, 
                "    The learned parameter vector");

  inherited::declareOptions(ol);
}

void SequencePLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies) {
  inherited::makeDeepCopyFromShallowCopy(copies);
}

void SequencePLearner::build()
{
  inherited::build();
  build_();
}

/*
  build the recurrent network and allocate places for bias and weigths
*/
void SequencePLearner::build_()
{

  /* tester si c'est une SeqVMatrix
  dynamic_cast<SequenceVMatrix*>((VMatrix*)train_set);
  */
}

int SequencePLearner::outputsize() const
{
  PLERROR("Unable to evaluate the output size without knowing the input set");
  return 0;
}

/*
  New function for a learner.
  In sequence we don't know the outputsize with knowing the input.
  The VMat must be a SequenceVMatrix.
*/

int SequencePLearner::outputsize(VMat set) const
{ 
  SequenceVMatrix *seq = dynamic_cast<SequenceVMatrix*>((VMatrix*)set);
  if (!seq)
    PLERROR("SequencePLearner::outputsize : The VMat given to BPTT::outputsize is not a SequenceVMatrix");
  return seq->getNbRowInSeqs(0, seq->getNbSeq());
}

void SequencePLearner::setTestSet(SequenceVMat s) {
  test_set = s;
}

/*
  Test the network.
*/
void SequencePLearner::test(VMat testset, PP<VecStatsCollector> test_stats, 
		VMat testoutputs, VMat testcosts) const {
  PLERROR("test(VMat, PP<VecStatsCollector>, VMat, VMat) cannot be used with a SequenceLearner");
}

void SequencePLearner::test(SequenceVMat testset, PP<VecStatsCollector> test_stats, 
		SequenceVMat testoutputs, SequenceVMat testcosts) const
{
  int l = testset->length();
  Mat input;
  Mat target;


  ProgressBar* pb = NULL;
  if(report_progress) 
    pb = new ProgressBar("Testing learner",l);

  for(int i=0; i<l; i++) {
    testset->getExample(i, input, target);
    Mat output = Mat(target.length(), target.width());
    Mat costs = Mat(target.length(), 1);
    if(testoutputs) {
      computeOutputAndCosts(input, target, output, costs);
      testoutputs->putOrAppendSequence(i,output);
    } else // no need to compute outputs
      computeCostsOnly(input, target, costs);
    if(testcosts)
      testcosts->putOrAppendSequence(i, costs);

    if(pb)
      pb->update(i);
  }
  
  if(pb)
    delete pb;
}

void SequencePLearner::computeOutput(const Vec& input, Vec& output) const {
  PLERROR("SequencePTester cannot use the computeOutput(const Vec&, Vec&) function");
}

void SequencePLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
				     const Vec& target, Vec& costs) const {
  PLERROR("SequencePTester cannot use the computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) function");  
}

void SequencePLearner::computeOutputAndCosts(const Vec& input, const Vec& target,
				 Vec& output, Vec& costs) const {
  PLERROR("SequencePTester cannot use the computeOutputAndCosts(const Vec&, const Vec&, Vec&, Vec&) function");  
}

void SequencePLearner::computeOutputAndCosts(const Mat& input, const Mat& target,
				 Mat& output, Mat& costs) const {
  computeOutput(input, output);
  computeCostsFromOutputs(input, output, target, costs);
}

void SequencePLearner::computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const {
  PLERROR("SequencePTester cannot use the computeCostsOnly(const Vec&, const Vec&, Vec&) function");  
}

void SequencePLearner::computeCostsOnly(const Mat& input, const Mat& target, Mat& costs) const {
  Mat output = Mat(target.length(), target.width());
  computeOutput(input, output);
  computeCostsFromOutputs(input, output, target, costs); 
}

/*
To test the verify gradient
*/
void SequencePLearner::run() {
  PLERROR("SequencePLearner::run() : Cannot run a SequencePLeaner Object");
}

} // end of namespace PLearn
