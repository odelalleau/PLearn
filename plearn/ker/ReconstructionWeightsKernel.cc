// -*- C++ -*-

// ReconstructionWeightsKernel.cc
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
   * $Id: ReconstructionWeightsKernel.cc,v 1.1 2004/07/15 21:06:17 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file ReconstructionWeightsKernel.cc */


#include "DistanceKernel.h"
#include "DotProductKernel.h"
#include "plapack.h"          //!< For solveLinearSystem().
#include "ReconstructionWeightsKernel.h"
#include "SelectRowsVMatrix.h"
#include "ShiftAndRescaleVMatrix.h"

namespace PLearn {
using namespace std;

/////////////////////////////////
// ReconstructionWeightsKernel //
/////////////////////////////////
ReconstructionWeightsKernel::ReconstructionWeightsKernel() 
: build_in_progress(false),
  knn(5),
  regularizer(1e-6)
{
  is_symmetric = false;
}

PLEARN_IMPLEMENT_OBJECT(ReconstructionWeightsKernel,
    "Computes the reconstruction weights of a point given its neighbors.",
    "K(x_i, x_j) = the weight of x_j in the reconstruction of x_i by its knn\n"
    "nearest neighbors. More precisely, we compute weights W_{ij} such that\n"
    "|| x_i - \\sum_j W_{ij} x_j ||^2 is minimized, and K(x_i,x_j) = W_{ij}.\n"
    "When applied outside of the training set, this kernel will return:\n"
    " - K(x, y) = 0\n"
    " - K(x_i, x) = 0\n"
    " - K(x, x_i) = the weight of x_i in the reconstruction of x by its knn\n"
    "               nearest neighbors in the training set.\n"
    "Thus note that this is NOT a symmetric kernel, and that using the\n"
    "evaluate_i_x() or evaluate_x_i() methods on a training point will\n"
    "not return the same result as evaluate_i_j().\n"
);

////////////////////
// declareOptions //
////////////////////
void ReconstructionWeightsKernel::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // Build options.

  declareOption(ol, "knn", &ReconstructionWeightsKernel::knn, OptionBase::buildoption,
      "The number of nearest neighbors considered.");

  declareOption(ol, "regularizer", &ReconstructionWeightsKernel::regularizer, OptionBase::buildoption,
      "A factor multiplied by the trace of the local Gram matrix and added to\n"
      "the diagonal to ensure stability when solving the linear system.");

  declareOption(ol, "distance_kernel", &ReconstructionWeightsKernel::distance_kernel, OptionBase::buildoption,
      "The kernel used to compute the distances.\n"
      "If not specified, then the usual Euclidean distance will be used.");

  declareOption(ol, "dot_product_kernel", &ReconstructionWeightsKernel::dot_product_kernel, OptionBase::buildoption,
      "The kernel used to compute dot products in the neighborhood of each data point.\n"
      "If not specified, then the usual Euclidean dot product will be used.");
    

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ReconstructionWeightsKernel::build()
{
  build_in_progress = true;
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void ReconstructionWeightsKernel::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.

  if (distance_kernel) {
    dist_ker = distance_kernel;
  } else {
    dist_ker = new DistanceKernel(2);
    dist_ker->report_progress = this->report_progress;
  }

  if (dot_product_kernel) {
    dp_ker = dot_product_kernel;
  } else {
    dp_ker = new DotProductKernel();
    dp_ker->build();
  }
  build_in_progress = false;
  // This code, normally executed in Kernel::build_, can only be executed
  // now beause the kernels 'dist_ker' and 'dp_ker' have to be initialized.
  if (specify_dataset) {
    this->setDataForKernelMatrix(specify_dataset);
  }
}

////////////////////
// computeWeights //
////////////////////
void ReconstructionWeightsKernel::computeWeights() {
  static Vec ones;
  static Vec point_i;
  static Mat local_gram;
  if (!data)
    PLERROR("In ReconstructionWeightsKernel::computeWeights - Can only be called if 'data' has been set");
  point_i.resize(data_inputsize);
  local_gram.resize(knn - 1, knn - 1);
  ones.resize(knn - 1);
  ones.fill(1);
  weights.resize(n_examples, knn - 1); // Allocate memory for the weights.
  weights.resize(0, knn - 1);
  // First compute the nearest neighbors.
  Mat distances(n_examples, n_examples);
  dist_ker->computeGramMatrix(distances);
  neighbors =
    computeKNNeighbourMatrixFromDistanceMatrix(distances, knn, true, report_progress);
  distances = Mat(); // Free memory.
  // Then compute the weights for each point i.
  TVec<int> neighbors_of_i(knn - 1);
  PP<SelectRowsVMatrix> neighborhood = new SelectRowsVMatrix();
  neighborhood->source = data;
  neighborhood->indices = neighbors_of_i; // Making sure it is of right length.
  neighborhood->build();
  PP<ShiftAndRescaleVMatrix> centered_neighborhood = new ShiftAndRescaleVMatrix();
  centered_neighborhood->vm = (SelectRowsVMatrix*) neighborhood;
  centered_neighborhood->no_scale = true;
  centered_neighborhood->build();
  ProgressBar* pb = 0;
  if (report_progress)
    pb = new ProgressBar("Computing reconstruction weights", n_examples);
  for (int i = 0; i < n_examples; i++) {
    // Isolate the neighbors.
    neighbors_of_i = neighbors(i).subVec(1, knn - 1);
    neighborhood->indices = neighbors_of_i;
    neighborhood->build();
    data->getSubRow(i, 0, point_i);
    centered_neighborhood->shift = point_i;
    // NB: we do not build 'centered_neighborhood' for efficiency concerns.
    // Compute the local Gram matrix.
    dp_ker->setDataForKernelMatrix((ShiftAndRescaleVMatrix*) centered_neighborhood);
    dp_ker->computeGramMatrix(local_gram);
    // Add regularization on the diagonal.
    regularizeMatrix(local_gram, regularizer);
    // Solve linear system.
    Vec weights_i = solveLinearSystem(local_gram, ones);
    // Normalize to get final solution.
    normalize(weights_i, 2);
    weights.appendRow(weights_i);
    if (report_progress)
      pb->update(i+1);
  }
  if (pb)
    delete pb;
}

//////////////
// evaluate //
//////////////
real ReconstructionWeightsKernel::evaluate(const Vec& x1, const Vec& x2) const {
  PLERROR("In ReconstructionWeightsKernel::evaluate - Not implemented");
  return 0;
}

//////////////////
// evaluate_i_j //
//////////////////
real ReconstructionWeightsKernel::evaluate_i_j(int i, int j) const {
  static TVec<int> neighbors_of_i;
  neighbors_of_i = neighbors(i);
  for (int k = 1; k < knn; k++) {
    if (neighbors_of_i[k] == j) {
      return weights(i, k - 1);
    }
  }
  return 0;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ReconstructionWeightsKernel::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("ReconstructionWeightsKernel::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void ReconstructionWeightsKernel::setDataForKernelMatrix(VMat the_data) {
  if (build_in_progress)
    return;
  inherited::setDataForKernelMatrix(the_data);
  dist_ker->setDataForKernelMatrix(the_data);
  computeWeights();
}

} // end of namespace PLearn

