// -*- C++ -*-

// AppendNeighborsVMatrix.cc
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
 * $Id: AppendNeighborsVMatrix.cc 3994 2005-08-25 13:35:03Z chapados $ 
 ******************************************************* */

// Authors: Hugo Larochelle

/*! \file AppendNeighborsVMatrix.cc */


#include "AppendNeighborsVMatrix.h"
#include "GetInputVMatrix.h"
#include "VMat_computeNearestNeighbors.h"
#include <plearn/math/TMat_maths_impl.h>

namespace PLearn {
using namespace std;


AppendNeighborsVMatrix::AppendNeighborsVMatrix()
    :inherited(), n_neighbors(1), append_neighbor_indices(false)
    /* ### Initialise all fields to their default value */
{
}

PLEARN_IMPLEMENT_OBJECT(AppendNeighborsVMatrix, 
                        "Appends the nearest neighbors of the input samples of a source VMatrix.", 
                        "Takes a source VMatrix and appends in the input part the\n"
                        "nearest neighbors of the input vector, for each row. \n"
                        "The current row is excluded of the nearest neighbors\n"
                        "for that row.\n"
                        "Also, keeps the target and weight information.\n"
                        "Finally, the user can define a transformation function\n"
                        "to map the nearest neighbors to some other space. Note that\n"
                        "the nearest neighbors are still computed in the source VMatrix\n"
                        "input space."
    );

void AppendNeighborsVMatrix::getNewRow(int i, const Vec& v) const
{
    if (width_<0)
        PLERROR("AppendNeighborsVMatrix::getNewRow called but build was not done yet");

    for(int j=0; j<input_parts.width(); j++)
    {
        source->getExample(input_parts(i,j),input,target,weight);
        if(transformation)
        {
            transformation->fprop(input,transf);
            v.subVec(j*transf.length(),transf.length()) << transf;
            if(j==0) 
            {
                if(append_neighbor_indices)
                    v.subVec(input_parts.width()*transf.length(),input_parts.width()) << input_parts(i);
                v.subVec(inputsize_,source->targetsize()) << target;
                if(weightsize() > 0) v[width_-1] = weight;
            }
                    
        }
        else
        {
            v.subVec(j*source->inputsize(),source->inputsize()) << input;
            if(j==0) 
            {         
                if(append_neighbor_indices)
                    v.subVec(input_parts.width()*source->inputsize(),input_parts.width()) << input_parts(i);
                v.subVec(inputsize_,source->targetsize()) << target;
                if(weightsize() > 0) v[width_-1] = weight;
            }

        }
    }
}

void AppendNeighborsVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "n_neighbors", &AppendNeighborsVMatrix::n_neighbors, OptionBase::buildoption,
                  "Number of nearest neighbors. Determines the width of this vmatrix, which\n"
                  "is source->width() * n_neighbors.\n");

    declareOption(ol, "transformation", &AppendNeighborsVMatrix::transformation, OptionBase::buildoption,
                  "Transformation to apply on the nearest neighbors\n");

    declareOption(ol, "append_neighbor_indices", &AppendNeighborsVMatrix::append_neighbor_indices, OptionBase::buildoption,
                  "Indication that the nearest neighbor indices should\n"
                  "appended to the input part. The index of the current\n"
                  "sample is also appended.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void AppendNeighborsVMatrix::build_()
{
    // find the nearest neighbors, if not done already
    if (source && (input_parts.length() != source->length() || input_parts.width() != n_neighbors+1 ))
        // WARNING: will not work if source is changed but has the same dimensions
    {
        if(n_neighbors <=0)
           PLERROR("In  AppendNeighborsVMatrix::build_(): n_neighbors should be > 0"); 
        input_parts.resize(source->length(),n_neighbors+1); // +1 because we also get current row

        input.resize(source->inputsize());
        target.resize(source->targetsize());

        VMat neighbors_source;
        if(source->targetsize() + source->weightsize() > 0)
        {
            GetInputVMatrix* givm = new GetInputVMatrix(source);
            neighbors_source = givm;
        }
        else
            neighbors_source = source;

        for (int i=0;i<source->length();i++)
        {
            source->getExample(i,input,target,weight);
            TVec<int> neighbors_of_i = input_parts(i);
            computeNearestNeighbors(neighbors_source,input,neighbors_of_i,-1);
        }
    }

    if(source->length()!=length_ || (transformation ? transformation->outputs.nelems() * input_parts.width() : source->inputsize() * input_parts.width()) 
       + (append_neighbor_indices ? input_parts.width() : 0) + source->targetsize() + source->weightsize() != width_)
    {
        if(transformation)
        {
            if(transformation->inputs.nelems() != source->inputsize())
                PLERROR("Cannot use transformation with input size different from source->inputsize()");
            transf.resize(transformation->outputs.nelems());

            width_ = transformation->outputs.nelems() * input_parts.width()
                + (append_neighbor_indices ? input_parts.width() : 0) 
                + source->targetsize() + source->weightsize();
            length_ = source->length();

            if(inputsize_ < 0) 
                inputsize_ = transformation->outputs.nelems() * input_parts.width()
                    + (append_neighbor_indices ? input_parts.width() : 0);
            if(targetsize_ < 0) targetsize_ = source->targetsize();
            if(weightsize_ < 0) weightsize_ = source->weightsize();            
        }
        else
        {
            width_ = source->inputsize() * input_parts.width() 
                + (append_neighbor_indices ? input_parts.width() : 0) 
                + source->targetsize() + source->weightsize();
            length_ = source->length();

            if(inputsize_ < 0) 
                inputsize_ = source->inputsize() * input_parts.width()
                    + (append_neighbor_indices ? input_parts.width() : 0);
            if(targetsize_ < 0) targetsize_ = source->targetsize();
            if(weightsize_ < 0) weightsize_ = source->weightsize();            

        }

        if(width_ != inputsize_ + targetsize_ + weightsize_)
            PLERROR("In  AppendNeighborsVMatrix::build_(): width_ != inputsize_ + targetsize_ + weightsize_");
    }
}

// ### Nothing to add here, simply calls build_
void AppendNeighborsVMatrix::build()
{
    inherited::build();
    build_();
}

void AppendNeighborsVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(input, copies);
    deepCopyField(target, copies);
    deepCopyField(input_parts, copies);
    deepCopyField(transf, copies);
    deepCopyField(transformation, copies);
    
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
