// -*- C++ -*-

// plearn_inc_gg.h
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

/* *******************************************************      
 * $Id: plearn_inc_gg.h 5606 2006-05-16 15:03:12Z lheureup $ 
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file plearn_inc_gg.h */

/*! Include here all classes available in the PLearn CVS repository 
  that do NOT depend upon fancy external libraries.
*/

#ifndef plearn_inc_gg_INC
#define plearn_inc_gg_INC

// Version number.
#define PLEARN_MAJOR_VERSION 0
#define PLEARN_MINOR_VERSION 92
#define PLEARN_FIXLEVEL 0

/******************************************************
 * Python includes must come FIRST, as per Python doc *
 ******************************************************/
#include <plearn/python/PythonIncludes.h>

/*****************
 * Miscellaneous *
 *****************/
//gg#include <plearn_learners/misc/Grapher.h>
//gg#include <plearn_learners/misc/VariableSelectionWithDirectedGradientDescent.h>
//gg#include <plearn/math/ManualBinner.h>
//gg#include <plearn/math/SoftHistogramBinner.h>
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
#include <commands/PLearnCommands/HelpCommand.h>
#include <commands/PLearnCommands/JulianDateCommand.h>
//gg#include <commands/PLearnCommands/KolmogorovSmirnovCommand.h>
#include <commands/PLearnCommands/LearnerCommand.h>
#include <commands/PLearnCommands/PairwiseDiffsCommand.h>
#include <commands/PLearnCommands/ReadAndWriteCommand.h>
#include <commands/PLearnCommands/RunCommand.h>
#include <commands/PLearnCommands/ServerCommand.h>
#include <commands/PLearnCommands/TestDependenciesCommand.h>
#include <commands/PLearnCommands/TestDependencyCommand.h>
//#include <commands/PLearnCommands/TxtmatCommand.h>


/**************
 * Dictionary *
 **************/
//gg#include <plearn_learners/language/Dictionary/Dictionary.h>
//gg#include <plearn_learners/language/Dictionary/FileDictionary.h>
//gg#include <plearn_learners/language/Dictionary/VecDictionary.h>
//gg#include <plearn_learners/language/Dictionary/ConditionalDictionary.h>

/****************
 * HyperCommand *
 ****************/
#include <plearn_learners/hyper/HyperOptimize.h>
#include <plearn_learners/hyper/HyperRetrain.h>

/**********
 * Kernel *
 **********/
//gg#include <plearn/ker/AdditiveNormalizationKernel.h>
//gg#include <plearn/ker/DistanceKernel.h>
//gg#include <plearn/ker/DotProductKernel.h>
//gg#include <plearn/ker/EpanechnikovKernel.h>
//gg#include <plearn/ker/GaussianKernel.h>
//gg#include <plearn/ker/GeodesicDistanceKernel.h>
//gg#include <plearn/ker/LLEKernel.h>
//gg#include <plearn/ker/NegOutputCostFunction.h>
//#include <plearn/ker/PolynomialKernel.h>
//gg#include <plearn/ker/ReconstructionWeightsKernel.h>
//gg#include <plearn/ker/ThresholdedKernel.h>
//gg#include <plearn/ker/VMatKernel.h>

/*************
 * Optimizer *
 *************/
#include <plearn/opt/AdaptGradientOptimizer.h>
#include <plearn/opt/ConjGradientOptimizer.h>
#include <plearn/opt/GradientOptimizer.h>

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

// Classifiers
//gg#include <plearn_learners/meta/AdaBoost.h>
//gg#include <plearn_learners/classifiers/BinaryStump.h>
#include <plearn_learners/classifiers/ClassifierFromConditionalPDistribution.h>
#include <plearn_learners/classifiers/ClassifierFromDensity.h>
#include <plearn_learners/classifiers/KNNClassifier.h>
#include <plearn_learners/classifiers/MultiInstanceNNet.h>
//#include <plearn_learners/classifiers/OverlappingAdaBoost.h> // Does not currently compile.

// Generic
#include <plearn_learners/generic/AddCostToLearner.h>
#include <plearn_learners/generic/AddLayersNNet.h>
#include <plearn_learners/generic/DistRepNNet.h>
#include <plearn_learners/generic/NNet.h>
#include <plearn_learners/generic/SelectInputSubsetLearner.h>
#include <plearn_learners/generic/StackedLearner.h>
#include <plearn_learners/generic/TestingLearner.h>
#include <plearn_learners/generic/VPLPreprocessedLearner.h>

// Hyper
#include <plearn_learners/hyper/HyperLearner.h>

// Regressors
#include <plearn_learners/regressors/ConstantRegressor.h>
//gg#include <plearn_learners/regressors/KernelRidgeRegressor.h>
#include <plearn_learners/regressors/KNNRegressor.h>
#include <plearn_learners/regressors/LinearRegressor.h>
//gg#include <plearn_learners/regressors/PLS.h>
//gg#include <plearn_learners/regressors/RankLearner.h>

// Unsupervised/KernelProjection
//gg#include <plearn_learners/unsupervised/Isomap.h>
//gg#include <plearn_learners/unsupervised/KernelPCA.h>
//gg#include <plearn_learners/unsupervised/LLE.h>
//gg#include <plearn_learners/unsupervised/PCA.h>
//gg#include <plearn_learners/unsupervised/SpectralClustering.h>
//gg#include <plearn_learners/unsupervised/UniformizeLearner.h>

// PDistribution
#include <plearn_learners/distributions/GaussianDistribution.h>
#include <plearn_learners/distributions/GaussMix.h>
#include <plearn_learners/distributions/ManifoldParzen2.h>
#include <plearn_learners/distributions/ParzenWindow.h>
#include <plearn_learners/distributions/RandomGaussMix.h>
#include <plearn_learners/distributions/SpiralDistribution.h>
#include <plearn_learners/distributions/UniformDistribution.h>

// Nearest-Neighbors
#include <plearn_learners/nearest_neighbors/BallTreeNearestNeighbors.h>
//gg#include <plearn_learners/nearest_neighbors/ExhaustiveNearestNeighbors.h>
//gg#include <plearn_learners/nearest_neighbors/GenericNearestNeighbors.h>

// Online
#include <plearn_learners/online/GaussianDBNClassification.h>
#include <plearn_learners/online/GradNNetLayerModule.h>
#include <plearn_learners/online/HintonDeepBeliefNet.h>
#include <plearn_learners/online/NLLErrModule.h>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/RBMParameters.h>
#include <plearn_learners/online/RBMGenericParameters.h>
#include <plearn_learners/online/RBMLayer.h>
#include <plearn_learners/online/RBMBinomialLayer.h>
#include <plearn_learners/online/RBMMultinomialLayer.h>
#include <plearn_learners/online/RBMGaussianLayer.h>
#include <plearn_learners/online/RBMMixedLayer.h>
#include <plearn_learners/online/RBMJointGenericParameters.h>

//gg#include <plearn_learners/online/SquaredErrModule.h>
//gg#include <plearn_learners/online/TanhModule.h>
//gg#include <plearn_learners/online/StackedModulesLearner.h>


/**********
 * Python *
 **********/
//gg#include <plearn/python/PythonCodeSnippet.h>
//gg#include <plearn/python/PythonProcessedVMatrix.h>

/************
 * Splitter *
 ************/
#include <plearn/vmat/BinSplitter.h>
#include <plearn/vmat/ConcatSetsSplitter.h>
#include <plearn/vmat/DBSplitter.h>
#include <plearn/vmat/ExplicitSplitter.h>
#include <plearn/vmat/FilterSplitter.h>
#include <plearn/vmat/FractionSplitter.h>
#include <plearn/vmat/KFoldSplitter.h>
#include <plearn/vmat/NoSplitSplitter.h>
#include <plearn/vmat/RepeatSplitter.h>
#include <plearn/vmat/SourceVMatrixSplitter.h>
#include <plearn/vmat/StackedSplitter.h>
#include <plearn/vmat/TestInTrainSplitter.h>
#include <plearn/vmat/ToBagSplitter.h>
#include <plearn/vmat/TrainTestSplitter.h>
#include <plearn/vmat/TrainValidTestSplitter.h>

/************
 * Variable *
 ************/
#include <plearn/var/MatrixElementsVariable.h>

/*********************
 * VecStatsCollector *
 *********************/
#include <plearn/math/LiftStatsCollector.h>

/***********
 * VMatrix *
 ***********/
#include <plearn/vmat/AddMissingVMatrix.h>
#include <plearn/vmat/AsciiVMatrix.h>
#include <plearn/vmat/AutoVMatrix.h>
#include <plearn/vmat/BootstrapVMatrix.h>
#include <plearn/vmat/CenteredVMatrix.h>
#include <plearn/vmat/CompactVMatrix.h>
#include <plearn/vmat/CompressedVMatrix.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/vmat/ConstantVMatrix.h>
#include <plearn/vmat/CumVMatrix.h>
#include <plearn/vmat/DatedJoinVMatrix.h>
#include <plearn/vmat/DictionaryVMatrix.h>
#include <plearn/vmat/DisregardRowsVMatrix.h>
#include <plearn/vmat/ExtractNNetParamsVMatrix.h>
#include <plearn/vmat/FilteredVMatrix.h>
#include <plearn/vmat/FinancePreprocVMatrix.h>
#include <plearn/vmat/GeneralizedOneHotVMatrix.h>
#include <plearn/vmat/GetInputVMatrix.h>
#include <plearn/vmat/GramVMatrix.h>
#include <plearn/vmat/IndexedVMatrix.h>
#include <plearn/vmat/JulianizeVMatrix.h>
#include <plearn/vmat/KNNVMatrix.h>
// Commented out because triggers WordNet, which does not work really fine yet.
//#include <plearn/vmat/LemmatizeVMatrix.h>
#include <plearn/vmat/LocalNeighborsDifferencesVMatrix.h>
#include <plearn/vmat/LocallyPrecomputedVMatrix.h>
//#include <plearn/vmat/MixUnlabeledNeighbourVMatrix.h>
#include <plearn/vmat/MultiInstanceVMatrix.h>
#include <plearn/vmat/MultiToUniInstanceSelectRandomVMatrix.h>
#include <plearn/vmat/OneHotVMatrix.h>
#include <plearn/vmat/PLearnerOutputVMatrix.h>
#include <plearn/vmat/PairsVMatrix.h>
#include <plearn/vmat/PrecomputedVMatrix.h>
#include <plearn/vmat/ProcessDatasetVMatrix.h>
#include <plearn/vmat/ProcessingVMatrix.h>
#include <plearn/vmat/ProcessSymbolicSequenceVMatrix.h>
#include <plearn/vmat/RandomSamplesVMatrix.h>
#include <plearn/vmat/RankedVMatrix.h>
#include <plearn/vmat/RegularGridVMatrix.h>
#include <plearn/vmat/RemoveDuplicateVMatrix.h>
#include <plearn/vmat/ReorderByMissingVMatrix.h>
//#include <plearn/vmat/SelectAttributsSequenceVMatrix.h>
#include <plearn/vmat/SelectRowsMultiInstanceVMatrix.h>
#include <plearn/vmat/ShuffleColumnsVMatrix.h>
#include <plearn/vmat/SortRowsVMatrix.h>
#include <plearn/vmat/SparseVMatrix.h>
#include <plearn/vmat/SubInputVMatrix.h>
#include <plearn/vmat/TemporaryDiskVMatrix.h>
#include <plearn/vmat/TemporaryFileVMatrix.h>
#include <plearn/vmat/TextFilesVMatrix.h>
#include <plearn/vmat/ThresholdVMatrix.h>
#include <plearn/vmat/TransposeVMatrix.h>
#include <plearn/vmat/UCIDataVMatrix.h>
#include <plearn/vmat/ViewSplitterVMatrix.h>
#include <plearn/vmat/VMatrixFromDistribution.h>

/*******************
 * SurfaceTemplate *
 ******************/
//gg#include <plearn_learners_experimental/SurfaceTemplate/ScoreLayerVariable.h>
//gg#include <plearn_learners_experimental/SurfaceTemplate/SurfaceTemplateLearner.h>


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
