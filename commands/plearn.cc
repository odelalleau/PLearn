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
   * $Id: plearn.cc,v 1.23 2003/05/20 15:42:11 plearner Exp $
   ******************************************************* */

#include "plearn_main.h"

// Available Splitters:
#include "ExplicitSplitter.h"
#include "TrainTestSplitter.h"
#include "KFoldSplitter.h"

// Available VMats:
#include "AutoVMatrix.h"
#include "IndexedVMatrix.h"

// New generation system
#include "PExperiment.h"
#include "NNet.h"
#include "HyperLearner.h"
#include "TryAll.h"

// All Available Learners: 
// #include "KNN.h"
//#include "Classification1HiddenNN.h"
//#include "Mixture2.h"
#include "ClassifierFromDensity.h"
#include "RegressorFromDensity.h"
#include "Distribution.h"
#include "GaussianDistribution.h"
#include "LocallyWeightedDistribution.h"
#include "NeuralNet.h"
#include "NNet.h"
#include "GradientOptimizer.h"
#include "AdaptGradientOptimizer.h"
#include "ConjGradientOptimizer.h"
// #include "AutoStepGradientOptimizer.h"

#include "ConstantModel.h"
#include "MultiLearner.h"
#include "LinearRegressor.h"

#include "EnsembleLearner.h"

// #include "SVM.h"

#include "ParzenDensity.h"
#include "ParzenRegressor.h"
#include "ManifoldParzenDensity.h"

#include "Experiment.h"


// Commands
#include "HelpCommand.h"
#include "KolmogorovSmirnovCommand.h"
#include "ReadAndWriteCommand.h"

using namespace PLearn;

int main(int argc, char** argv)
{
  PIStringStream in("il etait une fois dans l'ouest");
  string w;
  in >> w;
  cerr << "Word: " << w << endl;
  w.clear();
  cerr << "Word: " << w << endl;
  return plearn_main(argc, argv);
}

