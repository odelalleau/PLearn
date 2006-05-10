// -*- C++ -*-

// VMat_computeNearestNeighbors.cc
//
// Copyright (C) 2004 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file VMat_computeNearestNeighbors.cc */


#include "VMat_computeNearestNeighbors.h"
#include <plearn/vmat/VMat.h>
#include <plearn/math/BottomNI.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

// This is an efficient version of the most basic nearest neighbor search, using a Mat and euclidean distance
void computeNearestNeighbors(VMat dataset, Vec x, TVec<int>& neighbors, int ignore_row)
{
    int K = neighbors.length(); // how many neighbors do we want?
    BottomNI<real> neighbs(K);
    Vec row(dataset->width());
    for(int i=0; i<dataset->length(); i++)
        if(i!=ignore_row)
        {
            dataset->getRow(i,row);
            neighbs.update(powdistance(row,x), i);
        }
    neighbs.sort();
    TVec< pair<real,int> > indices = neighbs.getBottomN();
    int nonzero=0;
    for(int k=0; k<K; k++)
    {
        if(indices[k].first>0)
            nonzero++;
        neighbors[k] = indices[k].second;
    }
    if(nonzero==0)
        PLERROR("All neighbors had 0 distance. Use more neighbors. (There were %i other patterns with same values)",neighbs.nZeros());
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
