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
   * $Id: KNNVMatrix.cc,v 1.1 2004/02/19 21:52:03 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file KNNVMatrix.cc */

#include "DistanceKernel.h"
#include "KNNVMatrix.h"

namespace PLearn <%
using namespace std;

////////////////
// KNNVMatrix //
////////////////
KNNVMatrix::KNNVMatrix() 
: knn(5)
{}

PLEARN_IMPLEMENT_OBJECT(KNNVMatrix, "ONE LINE DESCR", "NO HELP");

////////////////////
// declareOptions //
////////////////////
void KNNVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "knn", &KNNVMatrix::knn, OptionBase::buildoption,
      "The number of nearest neighbours to consider.");

  declareOption(ol, "nn", &KNNVMatrix::nn, OptionBase::learntoption,
      "The matrix containing the index of the knn nearest neighbours of\n"
      "each data point.");

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
    // Only keep the (knn) nearest ones (i.e. knn+1 because the nearest neighbour
    // is always the point itself).
    // TODO Free the memory used by the other neighbours.
    // TODO Make the matrix be a TMat<int> instead of a Mat.
    nn.resize(n, knn + 1);
    // Initialize correctly the various fields.
    copySizesFrom(source);
    length_ = n * (knn + 1);
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

/////////
// get //
/////////
real KNNVMatrix::get(int i, int j) const
{
  return source->get(getSourceIndexOf(i), j);
}

//////////////////////
// getSourceIndexOf //
//////////////////////
int KNNVMatrix::getSourceIndexOf(int i) const {
  int i_ref = i / (knn + 1);
  int i_neighbour = i % (knn + 1);
  int i_neighbour_source = int(nn(i_ref, i_neighbour));
  return i_neighbour_source;
}

////////////
// getRow //
////////////
void KNNVMatrix::getRow(int i, Vec v) const {
  getSubRow(i, 0, v);
}

///////////////
// getSubRow //
///////////////
void KNNVMatrix::getSubRow(int i, int j, Vec v) const
{
  source->getSubRow(getSourceIndexOf(i), j, v);
}

%> // end of namespace PLearn
