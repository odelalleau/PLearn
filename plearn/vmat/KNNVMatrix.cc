// -*- C++ -*-

// KNNVMatrix.cc
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
   * $Id: KNNVMatrix.cc,v 1.4 2004/02/20 22:28:05 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file KNNVMatrix.cc */

#include "DistanceKernel.h"
#include "KNNVMatrix.h"

namespace PLearn {
using namespace std;

////////////////
// KNNVMatrix //
////////////////
KNNVMatrix::KNNVMatrix() 
: knn(6)
{}

PLEARN_IMPLEMENT_OBJECT(KNNVMatrix, 
    "A VMatrix that sees the nearest neighbours of each sample in the source VMat.", 
    "Each sample is followed by its (knn-1) nearest neighbours.\n"
    "To each row is appended an additional target, which is:\n"
    " - 1 if it is the first of a bag of neighbours,\n"
    " - 2 if it is the last of a bag,\n"
    " - 0 if it is none of these,\n"
    " - 3 if it is both (only for knn == 1).\n"
    "In addition, if a kernel_pij kernel is provided,, in the input part of the VMatrix\n"
    "is appended p_ij, where\n"
    "  p_ij = K(x_i,x_j) / \\sum_{k \\in knn(i), k != i} K(x_i,x_k)\n"
    "where K = kernel_pij, and j != i (for j == i, p_ij = -1).");

////////////////////
// declareOptions //
////////////////////
void KNNVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "knn", &KNNVMatrix::knn, OptionBase::buildoption,
      "The number of nearest neighbours to consider (including the point itself).");

  declareOption(ol, "kernel_pij", &KNNVMatrix::kernel_pij, OptionBase::buildoption,
      "An optional kernel used to compute the pij weights (see help).");

// Kinda useless to declare it as an option if we recompute it in build().
// TODO See how to be more efficient.
//  declareOption(ol, "nn", &KNNVMatrix::nn, OptionBase::learntoption,
//      "The matrix containing the index of the knn nearest neighbours of\n"
//      "each data point.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void KNNVMatrix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void KNNVMatrix::build_() {
  if (source) {
    // Compute the pairwise distances.
    DistanceKernel dk(2);
    dk.setDataForKernelMatrix(source);
    int n = source->length();
    Mat distances(n,n);
    dk.computeGramMatrix(distances);
    // Deduce the nearest neighbours.
    nn = dk.computeNeighbourMatrixFromDistanceMatrix(distances);
    // Only keep the (knn) nearest ones.
    // TODO Free the memory used by the other neighbours.
    // TODO Make the matrix be a TMat<int> instead of a Mat.
    nn.resize(n, knn);
    // Initialize correctly the various fields.
    copySizesFrom(source);
    targetsize_++;
    length_ = n * knn;
    width_ = source->width() + 1;

    if (kernel_pij) {
      inputsize_++;
      width_++;
      // Compute the pij.
      kernel_pij->setDataForKernelMatrix(source);
      int n = source->length();
      pij.resize(n, knn-1);
      for (int i = 0; i < n; i++) {
        real sum = 0;
        real k_ij;
        for (int j = 1; j < knn; j++) {
          // We omit the first nearest neighbour, which is the point itself.
          k_ij = kernel_pij->evaluate_i_j(i, int(nn(i,j)));
          pij(i,j-1) = k_ij;
          sum += k_ij;
        }
        pij.row(i) /= sum;
      }
    }
  }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void KNNVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
    PLERROR("KNNVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

//////////////////////
// getSourceIndexOf //
//////////////////////
int KNNVMatrix::getSourceIndexOf(int i, int& i_ref, int& i_n) const {
  i_ref = i / knn;
  i_n = i % knn;
  int i_neighbour_source = int(nn(i_ref, i_n));
  return i_neighbour_source;
}

////////////
// getRow //
////////////
void KNNVMatrix::getRow(int i, Vec v) const {
  static Vec w;
  w.resize(source->width());
  int i_n;
  int i_ref;
  int real_i = getSourceIndexOf(i, i_ref, i_n);
  source->getRow(real_i, w);
  if (kernel_pij) {
    v.subVec(0, source->inputsize()) << w.subVec(0, source->inputsize());
    if (i_n > 0) {
      v[source->inputsize()] = pij(i_ref, i_n - 1);
    } else {
      v[source->inputsize()] = -1;
    }
  } else {
    v.subVec(0, source->inputsize() + source->targetsize())
      << w.subVec(0, source->inputsize() + source->targetsize());
  }
  v.subVec(inputsize(), source->targetsize())
    << w.subVec(source->inputsize(), source->targetsize());
  v[inputsize() + source->targetsize()] = getTag(i_n);
  if (weightsize() > 0) {
    v.subVec(inputsize() + targetsize(), weightsize())
      << w.subVec(source->inputsize() + source->targetsize(), source->weightsize());
  }
}

////////////
// getTag //
////////////
int KNNVMatrix::getTag(int p) const {
  if (knn == 1) return 3;
  if (p == 0) return 1;
  if (p == knn - 1) return 2;
  return 0;
}

} // end of namespace PLearn
