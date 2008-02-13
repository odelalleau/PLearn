// -*- C++ -*-

// nlplearn_inc.h
//
// Copyright (C) 2004-2005 Olivier Delalleau 
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

// Authors: Hugo Larochelle

/*! \file nlplearn_inc.h */

/*! Include here all classes available in the PLearn CVS repository 
  for NLP applications
*/

#ifndef nlplearn_inc_INC
#define nlplearn_inc_INC

// Version number.
// HUGO: I just used those from plearn_inc.h
#define PLEARN_MAJOR_VERSION 0
#define PLEARN_MINOR_VERSION 92
#define PLEARN_FIXLEVEL 0

/*******************************************************************************
 * Python (experimental) -- python includes must come FIRST, as per Python doc *
 *******************************************************************************/
#include <plearn/python/PythonCodeSnippet.h>
#include <plearn/python/PythonProcessedVMatrix.h>

/*****************
 * Miscellaneous *
 *****************/
#include <plearn_learners/misc/Grapher.h>
#include <plearn_learners/misc/VariableSelectionWithDirectedGradientDescent.h>
#include <plearn/math/ManualBinner.h>
#include <plearn/math/SoftHistogramBinner.h>
#include <plearn/misc/ShellScript.h>
#include <plearn/misc/RunObject.h>
#include <plearn/db/UCISpecification.h>
#include <plearn_learners/testers/PTester.h>

/***********
 * Command *
 ***********/
#include <commands/PLearnCommands/AutoRunCommand.h>
#include <commands/PLearnCommands/DiffCommand.h>
#include <commands/PLearnCommands/FieldConvertCommand.h>
#include <commands/PLearnCommands/FillFeatureSetCommand.h>
#include <commands/PLearnCommands/HelpCommand.h>
#include <commands/PLearnCommands/JulianDateCommand.h>
#include <commands/PLearnCommands/KolmogorovSmirnovCommand.h>
#include <commands/PLearnCommands/LearnerCommand.h>
#include <commands/PLearnCommands/OutputFeaturesCommand.h>
#include <commands/PLearnCommands/ReadAndWriteCommand.h>
#include <commands/PLearnCommands/RunCommand.h>
#include <commands/PLearnCommands/ServerCommand.h>
#include <commands/PLearnCommands/TestDependenciesCommand.h>
#include <commands/PLearnCommands/TestDependencyCommand.h>
#include <commands/PLearnCommands/VMatCommand.h>
#include <commands/PLearnCommands/VMatViewCommand.h>

/**************
 * Dictionary *
 **************/
#include <plearn/dict/Dictionary.h>
#include <plearn/dict/FileDictionary.h>
#include <plearn/dict/VecDictionary.h>
#include <plearn/dict/WordNetSenseDictionary.h>
#include <plearn/dict/ConditionalDictionary.h>

/**************
 * FeatureSet *
 **************/

#include <plearn/feat/FeatureSet.h>
#include <plearn/feat/ConcatDisjointFeatureSet.h>
#include <plearn/feat/CachedFeatureSet.h>
#include <plearn/feat/HashMapFeatureSet.h>
#include <plearn/feat/WordNetFeatureSet.h>
#include <plearn/feat/PythonFeatureSet.h>
#include <plearn/feat/IdentityFeatureSet.h>
#include <plearn/feat/CachedFeatureSet.h>

/****************
 * HyperCommand *
 ****************/
#include <plearn_learners/hyper/HyperOptimize.h>
#include <plearn_learners/hyper/HyperRetrain.h>

/*************
 * Optimizer *
 *************/
//#include <plearn/opt/AdaptGradientOptimizer.h>
//#include <plearn/opt/ConjGradientOptimizer.h>
//#include <plearn/opt/GradientOptimizer.h>

/****************
 * OptionOracle *
 ****************/
#include <plearn_learners/hyper/CartesianProductOracle.h>
#include <plearn_learners/hyper/EarlyStoppingOracle.h>
#include <plearn_learners/hyper/ExplicitListOracle.h>
#include <plearn_learners/hyper/OptimizeOptionOracle.h>

/************
 * PLearner *
 ************/

// Generic
#include <plearn_learners/generic/AddCostToLearner.h>
//#include <plearn_learners/generic/DistRepNNet.h>
#include <plearn_learners/generic/FeatureSetNNet.h>
#include <plearn_learners/classifiers/FeatureSetNaiveBayesClassifier.h>

// Classifier
#include <plearn_learners/classifiers/ClassifierFromConditionalPDistribution.h>

// Hyper
#include <plearn_learners/hyper/HyperLearner.h>

// Unsupervised/KernelProjection
#include <plearn_learners/unsupervised/Isomap.h>
#include <plearn_learners/unsupervised/KernelPCA.h>
#include <plearn_learners/unsupervised/LLE.h>
#include <plearn_learners/unsupervised/PCA.h>
#include <plearn_learners/unsupervised/SpectralClustering.h>
#include <plearn_learners/unsupervised/UniformizeLearner.h>

// Distributions
#include <plearn_learners/distributions/NGramDistribution.h>

/************
 * Splitter *
 ************/
#include <plearn/vmat/ConcatSetsSplitter.h>
#include <plearn/vmat/ExplicitSplitter.h>
#include <plearn/vmat/FilterSplitter.h>
#include <plearn/vmat/FractionSplitter.h>
#include <plearn/vmat/KFoldSplitter.h>
#include <plearn/vmat/NoSplitSplitter.h>
#include <plearn/vmat/RepeatSplitter.h>
#include <plearn/vmat/SourceVMatrixSplitter.h>
#include <plearn/vmat/StackedSplitter.h>
#include <plearn/vmat/TextFilesVMatrix.h>
#include <plearn/vmat/TrainTestSplitter.h>
#include <plearn/vmat/TrainValidTestSplitter.h>
//#include <plearn/vmat/ClassSeparationSplitter.h>

/***********
 * VMatrix *
 ***********/
#include <plearn/vmat/AutoVMatrix.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/vmat/BootstrapVMatrix.h>
#include <plearn/vmat/DictionaryVMatrix.h>
#include <plearn/vmat/GetInputVMatrix.h>
#include <plearn/vmat/LemmatizeVMatrix.h>
#include <plearn/vmat/PLearnerOutputVMatrix.h>
#include <plearn/vmat/PrecomputedVMatrix.h>
#include <plearn/vmat/ProcessSymbolicSequenceVMatrix.h>
#include <plearn/vmat/RandomSamplesFromVMatrix.h>
#include <plearn/vmat/SortRowsVMatrix.h>
#endif


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
