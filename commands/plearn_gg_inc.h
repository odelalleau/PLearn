// -*- C++ -*-

// plearn_gg_inc.h
//
// Copyright (C) 2005 Olivier Delalleau 
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
   * $Id: plearn_gg_inc.h 3110 2005-02-23 01:32:46Z tihocan $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file plearn_gg_inc.h */

/*! Include here all classes available in the PLearn CVS repository
    that are dependent upon some external libraries.
 */

#ifndef plearn_gg_inc_INC
#define plearn_gg_inc_INC

#include <plearn_learners/second_iteration/Experimentation.h>
#include <plearn_learners/second_iteration/Preprocessing.h>
#include <plearn_learners/second_iteration/CheckDond2FileSequence.h>
#include <plearn_learners/second_iteration/MergeDond2Files.h>
#include <plearn_learners/second_iteration/ComputeDond2Target.h>
#include <plearn_learners/second_iteration/FixDond2BinaryVariables.h>
#include <plearn_learners/second_iteration/AnalyzeDond2DiscreteVariables.h>
#include <plearn_learners/second_iteration/DichotomizeDond2DiscreteVariables.h>
#include <plearn_learners/second_iteration/SecondIterationWrapper.h>
#include <plearn_learners/second_iteration/ComputePurenneError.h>
#include <plearn_learners/second_iteration/AnalyzeFieldStats.h>
#include <plearn_learners/second_iteration/NeighborhoodConditionalMean.h>
#include <plearn_learners/second_iteration/TestImputations.h>
#include <plearn_learners/second_iteration/GaussianizeVMatrix.h>
#include <plearn_learners/second_iteration/MissingIndicatorVMatrix.h>
#include <plearn_learners/second_iteration/MeanMedianModeImputationVMatrix.h>
#include <plearn_learners/second_iteration/ConditionalMeanImputationVMatrix.h>
#include <plearn_learners/second_iteration/CovariancePreservationImputationVMatrix.h>
#include <plearn_learners/second_iteration/NeighborhoodImputationVMatrix.h>
//#include <plearn_learners/second_iteration/BallTreeNearestNeighbors.h>
#include <plearn_learners/second_iteration/WeightedDistance.h>
#include <plearn_learners/regressors/RegressionTree.h>
#include <plearn/vmat/VariableDeletionVMatrix.h>
/*
#include <plearn/vmat/MeanMedianModeImputationVMatrix.h>
#include <plearn/vmat/LocalMeanImputationVMatrix.h>
#include <plearn/vmat/VariableDeletionVMatrix.h>
#include <plearn/vmat/MissingIndicatorVMatrix.h>
#include <plearn_learners/regressors/LocalMedBoost.h>
#include <plearn_learners/regressors/BaseRegressorWrapper.h>
#include <plearn_learners/regressors/BaseRegressorManifoldWrapper.h>
#include <plearn_learners/regressors/BaseRegressorManifoldConstant.h>
#include <plearn_learners/regressors/CompareRegressor.h>
#include <plearn_learners/regressors/DistanceLearner.h>
#include <plearn_learners/regressors/MultiClassDistanceLearner.h>
#include <plearn_learners/regressors/OnlineNNet.h>
*/

#endif

