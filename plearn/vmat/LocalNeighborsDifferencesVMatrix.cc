// -*- C++ -*-

// LocalNeighborsDifferencesVMatrix.cc
//
// Copyright (C) 2004 Martin Monperrus 
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
   * $Id: LocalNeighborsDifferencesVMatrix.cc,v 1.3 2004/07/07 17:30:48 tihocan Exp $ 
   ******************************************************* */

// Authors: Martin Monperrus

/*! \file LocalNeighborsDifferencesVMatrix.cc */


#include "LocalNeighborsDifferencesVMatrix.h"
#include "VMat_maths.h"

namespace PLearn {
using namespace std;


LocalNeighborsDifferencesVMatrix::LocalNeighborsDifferencesVMatrix()
  :inherited(), n_neighbors(-1)
  /* ### Initialise all fields to their default value */
{
}

PLEARN_IMPLEMENT_OBJECT(LocalNeighborsDifferencesVMatrix, 
                        "Computes the difference between each input row and its nearest neighbors.", 
                        "For each row x of the source VMatrix, the resulting row will be the\n"
                        "concatenation of n_neighbors vectors, each of which is the difference\n"
                        "between one of the nearest neighbors of x in the source and x itself.\n"
                        );

void LocalNeighborsDifferencesVMatrix::getNewRow(int i, const Vec& v) const
{
  if (width_<0)
    PLERROR("LocalNeighborsDifferencesVMatrix::getNewRow called but build was not done yet");
  Mat differences = v.toMat(n_neighbors,source->width());
  static Vec neighbor_row, ith_row;
  neighbor_row.resize(source->width());
  ith_row.resize(source->width());
  source->getRow(i,ith_row);
  for (int k=0;k<n_neighbors;k++)
  {
    Vec diff_k = differences(k);
    source->getRow(neighbors(i,k),neighbor_row);
    substract(neighbor_row,ith_row, diff_k);
    // normalize result
    diff_k /= norm(diff_k);
  }
}

void LocalNeighborsDifferencesVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "n_neighbors", &LocalNeighborsDifferencesVMatrix::n_neighbors, OptionBase::buildoption,
                "Number of nearest neighbors. Determines the width of this vmatrix, which\n"
                "is source->width() * n_neighbors.\n");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void LocalNeighborsDifferencesVMatrix::build_()
{
  // find the nearest neighbors, if not done already
  if (source && (neighbors.length()==0 || source->length()!=length_ || source->width()*n_neighbors!=width_))
      // will not work if source is changed but has the same dimensions
  {
    length_ = source->length();
    width_ = source->width()*n_neighbors;
    neighbors.resize(source->length(),n_neighbors);
    static Vec row;
    row.resize(source->width());
    for (int i=0;i<source->length();i++)
    {
      source->getRow(i,row);
      TVec<int> neighbors_of_i = neighbors(i);
      computeNearestNeighbors(source,row,neighbors_of_i,i);
    }
  }
}

// ### Nothing to add here, simply calls build_
void LocalNeighborsDifferencesVMatrix::build()
{
  inherited::build();
  build_();
}

void LocalNeighborsDifferencesVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

} // end of namespace PLearn

