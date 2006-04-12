// -*- C++ -*-

// RandomNeighborsDifferencesVMatrix.cc
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
 * $Id$ 
 ******************************************************* */

// Authors: Martin Monperrus

/*! \file RandomNeighborsDifferencesVMatrix.cc */


#include "RandomNeighborsDifferencesVMatrix.h"
#include "VMat_computeNearestNeighbors.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/math/random.h>

namespace PLearn {
using namespace std;


RandomNeighborsDifferencesVMatrix::RandomNeighborsDifferencesVMatrix()
    :inherited(), n_neighbors(-1), append_current_point_indexe(false), append_random_neighbors_indexes(false)
    /* ### Initialise all fields to their default value */
{
}

PLEARN_IMPLEMENT_OBJECT(RandomNeighborsDifferencesVMatrix, 
                        "Computes the difference between each input row and n_neighbors random points in the source data set.", 
                        "For each row x of the source VMatrix, the resulting row will be the\n"
                        "concatenation of n_neighbors vectors, each of which is the difference\n"
                        "between one of the random neighbors of x in the source and x itself.\n"
    );

void RandomNeighborsDifferencesVMatrix::getNewRow(int i, const Vec& v) const
{
    if (width_<0)
        PLERROR("RandomNeighborsDifferencesVMatrix::getNewRow called but build was not done yet");
    
    // vue en matrice du vecteur de sortie, ca pointe sur les memes donnees.
    Mat differences = v.toMat(n_neighbors,source->width());
    neighbor_row.resize(source->width());
    ith_row.resize(source->width());
    source->getRow(i,ith_row);
    int rand_index;
    if(append_current_point_indexe)
        v[n_neighbors*source->width()+1] = i;
    for (int k=0;k<n_neighbors;k++)
    {
        rand_index = int(uniform_sample()*source->length());
        diff_k = differences(k);
        source->getRow(rand_index,neighbor_row);
        substract(neighbor_row,ith_row, diff_k);
        // normalize result
        // now it's done in ProjectionErrorVariable diff_k /= norm(diff_k);
        if(append_random_neighbors_indexes)
            v[n_neighbors*source->width()+(append_current_point_indexe?1:0)+k] = rand_index;
    } 
}

void RandomNeighborsDifferencesVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "n_neighbors", &RandomNeighborsDifferencesVMatrix::n_neighbors, OptionBase::buildoption,
                  "Number of nearest neighbors. Determines the width of this vmatrix, which\n"
                  "is source->width() * n_neighbors.\n");
    declareOption(ol, "append_current_point_indexe", &RandomNeighborsDifferencesVMatrix::append_current_point_indexe, OptionBase::buildoption,
                  "Indication that the indexe of the current data point should be appended \n"
                  "to the row of the VMatrix.\n");
    declareOption(ol, "append_random_neighbors_indexes", &RandomNeighborsDifferencesVMatrix::append_random_neighbors_indexes, OptionBase::buildoption,
                  "Indication that the indexes of the random data points should be appended \n"
                  "to the row of the VMatrix.\n");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RandomNeighborsDifferencesVMatrix::build_()
{
    if (source)
        // will not work if source is changed but has the same dimensions
    {
        width_ = source->width()*n_neighbors;
        if(append_current_point_indexe) width_ += 1;
        if(append_random_neighbors_indexes) width_ += n_neighbors;
        length_ = source->length();
    }
}

// ### Nothing to add here, simply calls build_
void RandomNeighborsDifferencesVMatrix::build()
{
    inherited::build();
    build_();
}

void RandomNeighborsDifferencesVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(neighbor_row, copies);
    deepCopyField(ith_row, copies);
}

} // end of namespace PLearn


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
