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
 * $Id: plearn_light.cc 3995 2005-08-25 13:58:23Z chapados $
 ******************************************************* */

//! All includes should go into plearn_inc.h.
#include <commands/plearn_version.h>
#include <commands/PLearnCommands/plearn_main.h>


/*****************
 * Miscellaneous *
 *****************/
// #include <plearn/db/UCISpecification.h>
// #include <plearn/io/openUrl.h>
// #include <plearn/math/ManualBinner.h>
// #include <plearn/math/SoftHistogramBinner.h>
// #include <plearn/misc/ShellScript.h>
// #include <plearn/misc/RunObject.h>
// #include <plearn_learners/misc/Grapher.h>
// #include <plearn_learners/misc/VariableSelectionWithDirectedGradientDescent.h>
#include <plearn_learners/testers/PTester.h>

/***********
 * Command *
 ***********/
#include <commands/PLearnCommands/VMatCommand.h>
#include <commands/PLearnCommands/VMatViewCommand.h>
// #include <commands/PLearnCommands/AutoRunCommand.h>
// #include <commands/PLearnCommands/DiffCommand.h>
// #include <commands/PLearnCommands/FieldConvertCommand.h>
#include <commands/PLearnCommands/HelpCommand.h>
// #include <commands/PLearnCommands/JulianDateCommand.h>
// #include <commands/PLearnCommands/KolmogorovSmirnovCommand.h>
#include <commands/PLearnCommands/LearnerCommand.h>
// #include <commands/PLearnCommands/PairwiseDiffsCommand.h>
#include <commands/PLearnCommands/ReadAndWriteCommand.h>
#include <commands/PLearnCommands/RunCommand.h>
#include <commands/PLearnCommands/ServerCommand.h>
// #include <commands/PLearnCommands/TestDependenciesCommand.h>
// #include <commands/PLearnCommands/TestDependencyCommand.h>

// * extra stuff from Boost to generate help *
// #include <commands/PLearnCommands/HTMLHelpCommand.h>

// //#include <commands/PLearnCommands/TxtmatCommand.h>
#include <commands/PLearnCommands/VerifyGradientCommand.h>
#include <commands/PLearnCommands/ExtractOptionCommand.h>

// /**************
//  * Dictionary *
//  **************/
// #include <plearn/dict/Dictionary.h>
// #include <plearn/dict/FileDictionary.h>
// #include <plearn/dict/VecDictionary.h>
// #include <plearn/dict/ConditionalDictionary.h>

// /****************
//  * HyperCommand *
//  ****************/
#include <plearn_learners/hyper/HyperOptimize.h>
#include <plearn_learners/hyper/HyperRetrain.h>
#include <plearn_learners/hyper/HyperSetOption.h>

// /**********
//  * Kernel *
//  **********/
// #include <plearn/ker/AdditiveNormalizationKernel.h>
// #include <plearn/ker/DistanceKernel.h>
// #include <plearn/ker/DotProductKernel.h>
// #include <plearn/ker/EpanechnikovKernel.h>
// #include <plearn/ker/GaussianKernel.h>
// #include <plearn/ker/GeodesicDistanceKernel.h>
// #include <plearn/ker/IIDNoiseKernel.h>
// #include <plearn/ker/NegOutputCostFunction.h>
// #include <plearn/ker/NeuralNetworkARDKernel.h>
// #include <plearn/ker/PolynomialKernel.h>
// #include <plearn/ker/RationalQuadraticARDKernel.h>
// #include <plearn/ker/SquaredExponentialARDKernel.h>
// #include <plearn/ker/SummationKernel.h>
// #include <plearn/ker/ThresholdedKernel.h>
// #include <plearn/ker/VMatKernel.h>

// /*************
//  * Optimizer *
//  *************/
// #include <plearn/opt/AdaptGradientOptimizer.h>
// #include <plearn/opt/ConjGradientOptimizer.h>
#include <plearn/opt/GradientOptimizer.h>
#include <plearn/opt/EXPERIMENTAL/AutoScaledGradientOptimizer.h>

// /****************
//  * OptionOracle *
//  ****************/
// #include <plearn_learners/hyper/CartesianProductOracle.h>
#include <plearn_learners/hyper/EarlyStoppingOracle.h>
#include <plearn_learners/hyper/ExplicitListOracle.h>
// #include <plearn_learners/hyper/OptimizeOptionOracle.h>

// /************
//  * PLearner *
//  ************/

// // Unsupervised preprocessing
// #include <plearn_learners/unsupervised/UniformizeLearner.h>
#include <plearn_learners/unsupervised/NormalizationLearner.h>

// // Classifiers
// #include <plearn_learners/classifiers/BinaryStump.h>
// #include <plearn_learners/classifiers/ClassifierFromConditionalPDistribution.h>
// #include <plearn_learners/classifiers/ClassifierFromDensity.h>
// #include <plearn_learners/classifiers/KNNClassifier.h>
// //#include <plearn_learners/classifiers/SVMClassificationTorch.h>
// #include <plearn_learners/classifiers/MultiInstanceNNet.h>
// //#include <plearn_learners/classifiers/OverlappingAdaBoost.h> // Does not currently compile.

// // Generic
// #include <plearn_learners/generic/AddCostToLearner.h>
// #include <plearn_learners/generic/AddLayersNNet.h>
// #include <plearn_learners/generic/BestAveragingPLearner.h>
// //#include <plearn_learners/generic/DistRepNNet.h>
// #include <plearn_learners/generic/NNet.h>
// #include <plearn_learners/generic/SelectInputSubsetLearner.h>
#include <plearn_learners/generic/ChainedLearners.h>
// #include <plearn_learners/generic/StackedLearner.h>
// #include <plearn_learners/generic/TestingLearner.h>
// #include <plearn_learners/generic/VPLPreprocessedLearner.h>
// #include <plearn_learners/generic/VPLPreprocessedLearner2.h>
// #include <plearn_learners/generic/VPLCombinedLearner.h>

// // Hyper
#include <plearn_learners/hyper/HyperLearner.h>

// // Meta
// #include <plearn_learners/meta/AdaBoost.h>
// #include <plearn_learners/meta/BaggingLearner.h>

// // Regressors
// #include <plearn_learners/regressors/ConstantRegressor.h>
// #include <plearn_learners/regressors/GaussianProcessRegressor.h>
// #include <plearn_learners/regressors/KernelRidgeRegressor.h>
// #include <plearn_learners/regressors/KNNRegressor.h>
// #include <plearn_learners/regressors/RankLearner.h>
// #include <plearn_learners/regressors/RegressorFromDistribution.h>

// // PDistribution
// #include <plearn_learners/distributions/SpiralDistribution.h>
// #include <plearn_learners/distributions/UniformDistribution.h>

// // Nearest-Neighbors
// #include <plearn_learners/nearest_neighbors/BallTreeNearestNeighbors.h>
// #include <plearn_learners/nearest_neighbors/ExhaustiveNearestNeighbors.h>
// #include <plearn_learners/nearest_neighbors/GenericNearestNeighbors.h>

// // Experimental
// #include <plearn_learners_experimental/DeepFeatureExtractorNNet.h>

// // Online
// #include <plearn_learners/online/BackConvolution2DModule.h>
// #include <plearn_learners/online/ClassErrorCostModule.h>
// #include <plearn_learners/online/CombiningCostsModule.h>
// #include <plearn_learners/online/Convolution2DModule.h>
// #include <plearn_learners/online/CostModule.h>
// #include <plearn_learners/online/DeepBeliefNet.h>
// #include <plearn_learners/online/GradNNetLayerModule.h>
// #include <plearn_learners/online/ModulesLearner.h>
// #include <plearn_learners/online/ModuleStackModule.h>
// #include <plearn_learners/online/NLLCostModule.h>
// #include <plearn_learners/online/OnlineLearningModule.h>
// #include <plearn_learners/online/ProcessInputCostModule.h>
// #include <plearn_learners/online/RBMBinomialLayer.h>
// #include <plearn_learners/online/RBMClassificationModule.h>
// #include <plearn_learners/online/RBMConnection.h>
// #include <plearn_learners/online/RBMConv2DConnection.h>
// #include <plearn_learners/online/RBMGaussianLayer.h>
// #include <plearn_learners/online/RBMLayer.h>
// #include <plearn_learners/online/RBMMatrixConnection.h>
// #include <plearn_learners/online/RBMMatrixTransposeConnection.h>
// #include <plearn_learners/online/RBMMixedConnection.h>
// #include <plearn_learners/online/RBMMixedLayer.h>
// #include <plearn_learners/online/RBMMultinomialLayer.h>
// #include <plearn_learners/online/RBMTruncExpLayer.h>
// #include <plearn_learners/online/SoftmaxModule.h>
// #include <plearn_learners/online/SquaredErrorCostModule.h>
// #include <plearn_learners/online/StackedAutoassociatorsNet.h>
// #include <plearn_learners/online/Subsampling2DModule.h>
// #include <plearn_learners/online/Supersampling2DModule.h>
// #include <plearn_learners/online/TanhModule.h>

// /************
//  * Splitter *
//  ************/
// #include <plearn/vmat/BinSplitter.h>
// #include <plearn/vmat/BootstrapSplitter.h>
// #include <plearn/vmat/ClassSeparationSplitter.h>
// #include <plearn/vmat/ConcatSetsSplitter.h>
// #include <plearn/vmat/DBSplitter.h>
#include <plearn/vmat/ExplicitSplitter.h>
// #include <plearn/vmat/FilterSplitter.h>
#include <plearn/vmat/FractionSplitter.h>
// #include <plearn/vmat/KFoldSplitter.h>
#include <plearn/vmat/NoSplitSplitter.h>
// #include <plearn/vmat/MultiTaskSeparationSplitter.h>
// #include <plearn/vmat/RepeatSplitter.h>
// #include <plearn/vmat/SourceVMatrixSplitter.h>
// #include <plearn/vmat/StackedSplitter.h>
// #include <plearn/vmat/TestInTrainSplitter.h>
// #include <plearn/vmat/ToBagSplitter.h>
#include <plearn/vmat/TrainTestSplitter.h>
// #include <plearn/vmat/TrainValidTestSplitter.h>

// /************
//  * Variable *
//  ************/
// #include <plearn/var/MatrixElementsVariable.h>

// /*********************
//  * VecStatsCollector *
//  *********************/
// #include <plearn/math/LiftStatsCollector.h>

// /***********
//  * VMatrix *
//  ***********/
// #include <plearn/vmat/AddMissingVMatrix.h>
// #include <plearn/vmat/AppendNeighborsVMatrix.h>
// #include <plearn/vmat/AsciiVMatrix.h>
#include <plearn/vmat/AutoVMatrix.h>
// #include <plearn/vmat/BootstrapVMatrix.h>
// #include <plearn/vmat/CenteredVMatrix.h>
// #include <plearn/vmat/ClassSubsetVMatrix.h>
// #include <plearn/vmat/CompactVMatrix.h>
#include <plearn/vmat/CompactFileVMatrix.h>
// #include <plearn/vmat/CompressedVMatrix.h>
// #include <plearn/vmat/CumVMatrix.h>
// #include <plearn/vmat/DatedJoinVMatrix.h>
// // #include <plearn/vmat/DictionaryVMatrix.h>
// #include <plearn/vmat/DisregardRowsVMatrix.h>
// #include <plearn/vmat/ExtractNNetParamsVMatrix.h>
#include <plearn/vmat/FilteredVMatrix.h>
// #include <plearn/vmat/FinancePreprocVMatrix.h>
// #include <plearn/vmat/GaussianizeVMatrix.h>
// #include <plearn/vmat/GeneralizedOneHotVMatrix.h>
// #include <plearn/vmat/GetInputVMatrix.h>
// #include <plearn/vmat/GramVMatrix.h>
// #include <plearn/vmat/IndexedVMatrix.h>
// #include <plearn/vmat/JulianizeVMatrix.h>
// #include <plearn/vmat/KNNVMatrix.h>
// #include <plearn/vmat/KNNImputationVMatrix.h>
// // Commented out because triggers WordNet, which does not work really fine yet.
// //#include <plearn/vmat/LemmatizeVMatrix.h>
// #include <plearn/vmat/LocalNeighborsDifferencesVMatrix.h>
// #include <plearn/vmat/LocallyPrecomputedVMatrix.h>
// #include <plearn/vmat/MeanImputationVMatrix.h>
// //#include <plearn/vmat/MixUnlabeledNeighbourVMatrix.h>
// #include <plearn/vmat/MultiInstanceVMatrix.h>
// #include <plearn/vmat/MultiTargetOneHotVMatrix.h>
// #include <plearn/vmat/MultiToUniInstanceSelectRandomVMatrix.h>
// #include <plearn/vmat/OneHotVMatrix.h>
// #include <plearn/vmat/PLearnerOutputVMatrix.h>
// #include <plearn/vmat/PairsVMatrix.h>
// #include <plearn/vmat/PrecomputedVMatrix.h>
// #include <plearn/vmat/ProcessDatasetVMatrix.h>
#include <plearn/vmat/ProcessingVMatrix.h>
// #include <plearn/vmat/ProcessSymbolicSequenceVMatrix.h>
// #include <plearn/vmat/RandomSamplesVMatrix.h>
// #include <plearn/vmat/RandomSamplesFromVMatrix.h>
// #include <plearn/vmat/RankedVMatrix.h>
#include <plearn/vmat/RegularGridVMatrix.h>
// #include <plearn/vmat/RemoveDuplicateVMatrix.h>
// #include <plearn/vmat/ReorderByMissingVMatrix.h>
// //#include <plearn/vmat/SelectAttributsSequenceVMatrix.h>
// #include <plearn/vmat/SelectRowsMultiInstanceVMatrix.h>
// #include <plearn/vmat/ShuffleColumnsVMatrix.h>
// #include <plearn/vmat/SortRowsVMatrix.h>
// #include <plearn/vmat/SparseVMatrix.h>
// #include <plearn/vmat/SplitWiseValidationVMatrix.h>
// #include <plearn/vmat/SubInputVMatrix.h>
// #include <plearn/vmat/TemporaryDiskVMatrix.h>
// #include <plearn/vmat/TemporaryFileVMatrix.h>
// #include <plearn/vmat/TextFilesVMatrix.h>
// #include <plearn/vmat/ThresholdVMatrix.h>
// #include <plearn/vmat/TransposeVMatrix.h>
// #include <plearn/vmat/UCIDataVMatrix.h>
// #include <plearn/vmat/UniformizeVMatrix.h>
// #include <plearn/vmat/VariableDeletionVMatrix.h>
// #include <plearn/vmat/ViewSplitterVMatrix.h>
// #include <plearn/vmat/VMatrixFromDistribution.h>



// **** Require LAPACK and BLAS

// Unsupervised/KernelProjection
// #include <plearn_learners/unsupervised/Isomap.h>
// #include <plearn_learners/unsupervised/KernelPCA.h>
// #include <plearn_learners/unsupervised/LLE.h>
#include <plearn_learners/unsupervised/PCA.h>
// #include <plearn_learners/unsupervised/SpectralClustering.h>

// Kernels
// #include <plearn/ker/LLEKernel.h>
// #include <plearn/ker/ReconstructionWeightsKernel.h>

// Regressors
// #include <plearn_learners/regressors/LinearRegressor.h>
// #include <plearn_learners/regressors/PLS.h>

// PDistribution
// #include <plearn_learners/distributions/GaussianDistribution.h>
// #include <plearn_learners/distributions/GaussMix.h>
// #include <plearn_learners/distributions/RandomGaussMix.h>
#include <plearn_learners/distributions/ParzenWindow.h>
// #include <plearn_learners/distributions/ManifoldParzen2.h>

// Experimental
// #include <plearn_learners_experimental/LinearInductiveTransferClassifier.h>

// SurfaceTemplate
// #include <plearn_learners_experimental/SurfaceTemplate/SurfaceTemplateLearner.h>

// ***************************************************
// ***   New EXPERIMENTAL stuff

#include <plearn/var/EXPERIMENTAL/SumVarianceOfLinearTransformedBernoullis.h>
#include <plearn/var/EXPERIMENTAL/SumVarianceOfLinearTransformedCategoricals.h>
#include <plearn/var/EXPERIMENTAL/SumEntropyOfBernoullis.h>
#include <plearn/var/EXPERIMENTAL/SumEntropyOfCategoricals.h>
#include <plearn/var/EXPERIMENTAL/LinearCombinationOfScalarVariables.h>
#include <plearn/var/EXPERIMENTAL/SaltPepperNoiseVariable.h>

// includes Pascal's gradient hack
#include <plearn_learners/generic/EXPERIMENTAL/NatGradNNet.h>


// Stuff used for DeepReconstructorNet experiments
#include <plearn/var/Variable.h>
#include <plearn/var/SquareVariable.h>
#include <plearn/math/TVec_impl.h>
#include <plearn/var/EXPERIMENTAL/MultiMaxVariable.h>
#include <plearn/var/SoftmaxVariable.h>
#include <plearn/var/SumSquareVariable.h>
#include <plearn/var/Func.h>
#include <plearn/var/EXPERIMENTAL/DoubleProductVariable.h>
#include <plearn/var/EXPERIMENTAL/TransposedDoubleProductVariable.h>
#include <plearn/var/EXPERIMENTAL/ProbabilityPairsVariable.h>
#include <plearn/var/EXPERIMENTAL/ProbabilityPairsInverseVariable.h>
#include <plearn/var/EXPERIMENTAL/SoftSoftMaxVariable.h>
#include <plearn/var/EXPERIMENTAL/LogSoftSoftMaxVariable.h>
#include <plearn_learners/generic/EXPERIMENTAL/DeepReconstructorNet.h>
#include <plearn/var/SourceVariable.h>
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/var/ConcatRowsVariable.h>
#include <plearn/var/ExpVariable.h>
#include <plearn/var/LogVariable.h>
#include <plearn/var/SigmoidVariable.h>
#include <plearn/var/ProductTransposeVariable.h>
#include <plearn/var/NegCrossEntropySigmoidVariable.h>
#include <plearn/var/LogSoftmaxVariable.h>
#include <plearn/var/ClassificationLossVariable.h>
#include <plearn/var/EXPERIMENTAL/MultiSampleVariable.h>
#include <plearn/var/EXPERIMENTAL/RandomForcedValuesVariable.h>
#include <plearn/var/EXPERIMENTAL/BernoulliSampleVariable.h>
#include <plearn/var/EXPERIMENTAL/TimesConstantScalarVariable2.h>
#include <plearn/var/PlusConstantVariable.h>
#include <plearn/var/TimesConstantVariable.h>

// Stuff used for transformationLearner experiments
#include <plearn_learners/distributions/EXPERIMENTAL/TransformationLearner.h>

// Stuff used for local Gaussian classifier and for knn with PartsDistanceKernel
#include <plearn_learners/classifiers/KNNClassifier.h>
#include <plearn/ker/EXPERIMENTAL/PartsDistanceKernel.h>
#include <plearn_learners/classifiers/EXPERIMENTAL/LocalGaussianClassifier.h>

// Stuff used for DiverseComponentAnalysis
#include <plearn_learners/unsupervised/EXPERIMENTAL/DiverseComponentAnalysis.h>

// Stuff used for DenoisingRecurrentNet
#include <plearn_learners_experimental/DenoisingRecurrentNet.h>

using namespace PLearn;

int main(int argc, char** argv)
{
    return plearn_main( argc, argv, 
                        PLEARN_MAJOR_VERSION, 
                        PLEARN_MINOR_VERSION, 
                        PLEARN_FIXLEVEL       );
}


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
