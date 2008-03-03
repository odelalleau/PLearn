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
 * $Id$
 ******************************************************* */

// Authors: Martin Monperrus

/*! \file LocalNeighborsDifferencesVMatrix.cc */


#include "LocalNeighborsDifferencesVMatrix.h"
#include "VMat_computeNearestNeighbors.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;


LocalNeighborsDifferencesVMatrix::LocalNeighborsDifferencesVMatrix()
    :inherited(), n_neighbors(-1), concat_neighbors(false), append_indexes(false)
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
    if (concat_neighbors)
    {
        // resize des varaibles
        //v.resize(source->width()); //we assume that the vector has already the good size
        neighbor_row.resize(source->width());
        ith_row.resize(source->width());
        diff_k = v;
        // recuperation de ce qu'il faut
        source->getRow(i,ith_row);
        source->getRow(neighbors(i,n_neighbors-1),neighbor_row);

        // on renvoie la valeur
        substract(neighbor_row,ith_row, diff_k);

        if(append_indexes)
        {
            v[source->width()] = i;
            v[source->width()+1] = neighbors(i,n_neighbors-1);
        }
        if(append_neighbors)
        {
            v.subVec(source->width()+(append_indexes?2:0),source->width()) << neighbor_row;
        }

    }
    else
    {
        // vue en matrice du vecteur de sortie, ca pointe sur les memes donnees.
        Mat differences = v.toMat(n_neighbors,source->width());
        neighbor_row.resize(source->width());
        ith_row.resize(source->width());
        source->getRow(i,ith_row);
        for (int k=0;k<n_neighbors;k++)
        {
            diff_k = differences(k);
            source->getRow(neighbors(i,k),neighbor_row);
            substract(neighbor_row,ith_row, diff_k);
            // normalize result
            // now it's done in ProjectionErrorVariable diff_k /= norm(diff_k);

            if(append_neighbors)
            {
                v.subVec(n_neighbors*source->width()+(append_indexes?n_neighbors+1:0)+k*source->width(),source->width()) << neighbor_row;
            }

        }

        if(append_indexes)
        {
            v[n_neighbors*source->width()] = i;
            for(int k=0; k<n_neighbors; k++)
                v[n_neighbors*source->width()+k+1] = neighbors(i,k);
        }
    }

}

void LocalNeighborsDifferencesVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "n_neighbors", &LocalNeighborsDifferencesVMatrix::n_neighbors, OptionBase::buildoption,
                  "Number of nearest neighbors. Determines the width of this vmatrix, which\n"
                  "is source->width() * n_neighbors.\n");
    declareOption(ol, "concat_neighbors", &LocalNeighborsDifferencesVMatrix::concat_neighbors, OptionBase::buildoption,
                  "If false, returns the concatenation of nearest neighbors(x) -x  from 1 to n_neighbors , else, returns the n_neighbor nearest neighbor(x) - x\n"
                  "(I know, it doesn't make sense. This should be corrected...)");
    declareOption(ol, "append_indexes", &LocalNeighborsDifferencesVMatrix::append_indexes, OptionBase::buildoption,
                  "Indication that the indexes of the current data point and of the k nearest neighbors\n"
                  "should be appended to the row of the VMatrix.\n");
    declareOption(ol, "append_neighbors", &LocalNeighborsDifferencesVMatrix::append_indexes, OptionBase::buildoption,
                  "Indication that the neighbor vectors of the k nearest neighbors\n"
                  "should be appended to the row of the VMatrix.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void LocalNeighborsDifferencesVMatrix::build_()
{
    // find the nearest neighbors, if not done already
    if (source && (neighbors.length()==0 || source->length()!=length_ || source->width()*n_neighbors!=width_))
        // will not work if source is changed but has the same dimensions
    {
        if (concat_neighbors)
        {
            width_ = source->width();
            if(append_indexes) width_ += 2;
            if(append_neighbors) width_ += source->width();
        }
        else
        {
            width_ = source->width()*n_neighbors;
            if(append_indexes) width_ += n_neighbors+1;
            if(append_neighbors) width_ += source->width()*n_neighbors;
        }

        length_ = source->length();
        neighbors.resize(source->length(),n_neighbors);
        a_row.resize(source->width());
        for (int i=0;i<source->length();i++)
        {
            source->getRow(i,a_row);
            TVec<int> neighbors_of_i = neighbors(i);
            computeNearestNeighbors(source,a_row,neighbors_of_i,i);
        }
    }

    updateMtime(source);
}

// ### Nothing to add here, simply calls build_
void LocalNeighborsDifferencesVMatrix::build()
{
    inherited::build();
    build_();
}

void LocalNeighborsDifferencesVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(neighbor_row, copies);
    deepCopyField(ith_row, copies);
    deepCopyField(a_row, copies);
    deepCopyField(neighbors, copies);
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
