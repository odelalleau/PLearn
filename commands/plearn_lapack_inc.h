// -*- C++ -*-

// plearn_lapack_inc.h
//
// Copyright (C) 2006 Pascal Vincent
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
 * $Id: plearn_lapack_inc.h 6346 2006-10-24 17:02:02Z lamblin $ 
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file plearn_lapack_inc.h */

/*! Include here all classes available in the PLearn repository 
  that do depend on LAPACK and BLAS (in addition to NSPR and boost)
  but no other fancy library.
*/

#ifndef plearn_lapack_inc_INC
#define plearn_lapack_inc_INC

// Do not include if USE_BLAS_SPECIALISATIONS is undefined
// (compiling with -noblas)
#ifdef USE_BLAS_SPECIALISATIONS

// Regressors
#include <plearn_learners/regressors/LinearRegressor.h>
#include <plearn_learners/regressors/PLS.h>

// Unsupervised/KernelProjection
#include <plearn_learners/unsupervised/NormalizationLearner.h>
#include <plearn_learners/unsupervised/Isomap.h>
#include <plearn_learners/unsupervised/KernelPCA.h>
#include <plearn_learners/unsupervised/LLE.h>
#include <plearn_learners/unsupervised/PCA.h>
#include <plearn_learners/unsupervised/SpectralClustering.h>

// Kernels
#include <plearn/ker/LLEKernel.h>
#include <plearn/ker/ReconstructionWeightsKernel.h>

// PDistribution
#include <plearn_learners/distributions/GaussianDistribution.h>
#include <plearn_learners/distributions/GaussMix.h>
#include <plearn_learners/distributions/RandomGaussMix.h>
#include <plearn_learners/distributions/ParzenWindow.h>
#include <plearn_learners/distributions/ManifoldParzen2.h>

// Experimental
#include <plearn_learners_experimental/LinearInductiveTransferClassifier.h>
#include <plearn_learners_experimental/SurfaceTemplate/SurfaceTemplateLearner.h>

// Online
#include <plearn_learners/online/RBMMatrixConnectionNatGrad.h>


#endif // USE_BLAS_SPECIALISATIONS

#endif // plearn_lapack_inc_INC


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
