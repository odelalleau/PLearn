// -*- C++ -*-

// plearn_inc.h
//
// Copyright (C) 2004 Olivier Delalleau 
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
   * $Id: plearn_inc.h,v 1.3 2004/06/23 16:49:24 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file plearn_inc.h */

/*! Include here all classes available in the PLearn CVS repository */

#ifndef plearn_inc_INC
#define plearn_inc_INC

/*****************
 * Miscellaneous *
 *****************/
#include "Grapher.h"
#include "RunObject.h"

/***********
 * Command *
 ***********/
#include "AutoRunCommand.h"
#include "FieldConvertCommand.h"
#include "HelpCommand.h"
#include "JulianDateCommand.h"
#include "KolmogorovSmirnovCommand.h"
#include "LearnerCommand.h"
#include "ReadAndWriteCommand.h"
#include "RunCommand.h"
#include "TestDependenciesCommand.h"
#include "TestDependencyCommand.h"
#include "VMatCommand.h"

/**********
 * Kernel *
 **********/
#include "AdditiveNormalizationKernel.h"
#include "DotProductKernel.h"
#include "GaussianKernel.h"
#include "GeodesicDistanceKernel.h"
#include "NegOutputCostFunction.h"

/*************
 * Optimizer *
 *************/
#include "AdaptGradientOptimizer.h"
#include "ConjGradientOptimizer.h"
#include "GradientOptimizer.h"

/************
 * PLearner *
 ************/
#include "AddCostToLearner.h"
#include "ClassifierFromDensity.h"
#include "ConstantRegressor.h"
#include "LinearRegressor.h"
#include "MultiInstanceNNet.h"
#include "NNet.h"
#include "PCA.h"
#include "PLS.h"
#include "StackedLearner.h"
// Distribution (deprecated)
#include "LocallyWeightedDistribution.h"
// EmbeddedLearner
#include "SelectInputSubsetLearner.h"
// KernelProjection
#include "Isomap.h"
#include "KernelPCA.h"
#include "SpectralClustering.h"
// PDistribution
#include "ConditionalDensityNet.h"
#include "GaussianDistribution.h"
#include "ManifoldParzen2.h"
#include "SpiralDistribution.h"
#include "UniformDistribution.h"
// SequencePLearner
#include "BPTT.h"

/***********
 * PTester *
 ***********/
#include "VarLengthPTester.h"

/************
 * Splitter *
 ************/
#include "ExplicitSplitter.h"
#include "FilterSplitter.h"
#include "FractionSplitter.h"
#include "KFoldSplitter.h"
#include "RepeatSplitter.h"
#include "SourceVMatrixSplitter.h"
#include "TestInTrainSplitter.h"
#include "ToBagSplitter.h"
#include "TrainTestSplitter.h"
#include "TrainValidTestSplitter.h"

/*********************
 * VecStatsCollector *
 *********************/
#include "LiftStatsCollector.h"

/***********
 * VMatrix *
 ***********/
#include "AutoVMatrix.h"
#include "BatchVMatrix.h"
#include "BootstrapVMatrix.h"
#include "CumVMatrix.h"
#include "DelaySequenceVMatrix.h"
#include "FilteredVMatrix.h"
#include "IndexedVMatrix.h"
#include "LocalNeighborsDifferencesVMatrix.h"
#include "MultiInstanceVMatrix.h"
#include "PrecomputedVMatrix.h"
#include "ProcessingVMatrix.h"
#include "RegularGridVMatrix.h"
#include "SequenceVMatrix.h"
#include "SortRowsVMatrix.h"
#include "SubInputVMatrix.h"
#include "VMatrixFromDistribution.h"
#include "XORSequenceVMatrix.h"

#endif

