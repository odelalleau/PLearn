// -*- C++ -*-

// plearn.cc
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux, Rejean Ducharme
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
   * $Id: plearn.cc,v 1.62 2004/05/19 13:05:28 tihocan Exp $
   ******************************************************* */

#include "plearn_main.h"

// Available Splitters:
#include "ExplicitSplitter.h"
#include "FractionSplitter.h"
#include "KFoldSplitter.h"
#include "TrainTestSplitter.h"
#include "TrainValidTestSplitter.h"

// Available VMats:
#include "AutoVMatrix.h"
#include "BatchVMatrix.h"
#include "SequenceVMatrix.h"
#include "IndexedVMatrix.h"
#include "RegularGridVMatrix.h"
#include "PrecomputedVMatrix.h"
#include "SortRowsVMatrix.h"
#include "VMatrixFromDistribution.h"

// Available Kernels:
#include "AdditiveNormalizationKernel.h"

// All Available Learners: 
// #include "KNN.h"
//#include "Classification1HiddenNN.h"
//#include "Mixture2.h"
#include "AddCostToLearner.h"
#include "ClassifierFromDensity.h"
//#include "RegressorFromDensity.h"
#include "Distribution.h"
#include "GaussianDistribution.h"
#include "KernelPCA.h"
#include "KernelProjection.h"
#include "LocallyWeightedDistribution.h"
#include "ManifoldParzen2.h"
#include "NeuralNet.h"
#include "NNet.h"
#include "BPTT.h"
#include "PCA.h"
#include "PLS.h"
#include "GradientOptimizer.h"
#include "AdaptGradientOptimizer.h"
#include "ConjGradientOptimizer.h"
// #include "AutoStepGradientOptimizer.h"

// #include "ConstantModel.h"
// #include "MultiLearner.h"
// #include "LinearRegressor.h"
// #include "EnsembleLearner.h"

// #include "SVM.h"

// #include "ParzenDensity.h"
// #include "ParzenRegressor.h"
// #include "ManinnnnnnfoldParzenDensity.h"

#include "Experiment.h"

// New generation system
#include "VarLengthPTester.h"
#include "NNet.h"
#include "Grapher.h"
#include "ConstantRegressor.h"
#include "MultiInstanceNNet.h"
#include "MultiInstanceVMatrix.h"
#include "LiftStatsCollector.h"

// Distributions
#include "SpiralDistribution.h"

// SequentialLearner
//#include "SequentialLearner.h"
//#include "MovingAverage.h"
//#include "EmbeddedSequentialLearner.h"
//#include "SequentialModelSelector.h"
//#include "SequentialValidation.h"

// Commands
#include "RunCommand.h"
#include "HelpCommand.h"
#include "AutoRunCommand.h"
#include "VMatCommand.h"
#include "LearnerCommand.h"
#include "KolmogorovSmirnovCommand.h"
#include "TestDependenciesCommand.h"
#include "TestDependencyCommand.h"
#include "ReadAndWriteCommand.h"
#include "JulianDateCommand.h"
#include "FieldConvertCommand.h"

using namespace PLearn;

int main(int argc, char** argv)
{
  return plearn_main(argc, argv);
}

