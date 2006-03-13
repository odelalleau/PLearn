// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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

#include "SubVMatrix.h"

namespace PLearn {
using namespace std;


/** SubVMatrix **/

PLEARN_IMPLEMENT_OBJECT(SubVMatrix,
                        "Views only a submatrix of a source VMatrix",
                        "");

////////////////
// SubVMatrix //
////////////////
SubVMatrix::SubVMatrix(bool call_build_)
    : inherited(call_build_),
      istart(0),
      jstart(0),
      fistart(-1),
      flength(-1)
{
    // don't call build_() when no source is set
}

SubVMatrix::SubVMatrix(VMat the_source,
                       int the_istart, int the_jstart,
                       int the_length, int the_width,
                       bool call_build_)
    : inherited(the_source, the_length, the_width, call_build_),
      istart(the_istart), jstart(the_jstart),
      fistart(-1), flength(-1)
{
    if( call_build_ )
        build_();
}

SubVMatrix::SubVMatrix(VMat the_source,
                       real the_fistart, int the_jstart,
                       real the_flength, int the_width,
                       bool call_build_)
    : inherited(the_source,
                int(the_flength * the_source->length()),
                the_width,
                call_build_),
      istart(-1), jstart(the_jstart),
      fistart(the_fistart), flength(the_flength)
    // Note that istart will be set to the right value in build_().
{
    if( call_build_ )
        build_();
}

////////////////////
// declareOptions //
////////////////////
void SubVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "parent", &SubVMatrix::source, OptionBase::buildoption,
                  "DEPRECATED - Use 'source' instead.");

    declareOption(ol, "istart", &SubVMatrix::istart, OptionBase::buildoption,
                  "Start i coordinate (row wise)");

    declareOption(ol, "jstart", &SubVMatrix::jstart, OptionBase::buildoption,
                  "Start j coordinate (column wise)");

    declareOption(ol, "fistart", &SubVMatrix::fistart, OptionBase::buildoption,
                  "If provided, will override istart to"
                  " fistart * source.length()");

    declareOption(ol, "flength", &SubVMatrix::flength, OptionBase::buildoption,
                  "If provided, will override length to"
                  " flength * source.length()");

    inherited::declareOptions(ol);
}

void SubVMatrix::build()
{
    inherited::build();
    build_();
}

void SubVMatrix::build_()
{
    int sl = source->length();
    int sw = source->width();

    if (fistart >= 0)
        istart = int(fistart * sl);

    if (flength >= 0)
        length_ = int(flength * sl);

    if(length_ < 0)
        length_ = sl - istart;

    if(width_ < 0 && sw >= 0)
        width_ = sw - jstart;

    if(istart+length() > sl || jstart+width() > sw)
        PLERROR("In SubVMatrix constructor OUT OF BOUNDS of source VMatrix");

    // Copy the source field names.
    fieldinfos.resize(width());
    if (source->getFieldInfos().size() > 0)
        for(int j=0; j<width(); j++)
            fieldinfos[j] = source->getFieldInfos()[jstart+j];

    // Copy the source string mappings.
    map_rs.resize(width());
    map_sr.resize(width());
    for (int j = jstart; j < width_ + jstart; j++) {
        map_rs[j - jstart] = source->getRealToStringMapping(j);
        map_sr[j - jstart] = source->getStringToRealMapping(j);
    }

    // determine sizes
/*
    if (width_ == source->width())
    {
        if(inputsize_<0) inputsize_ = source->inputsize();
        if(targetsize_<0) targetsize_ = source->targetsize();
        if(weightsize_<0) weightsize_ = source->weightsize();
    } else {
        // The width has changed: if no sizes are specified,
        // we assume it's all input and no target.
        if(targetsize_<0) targetsize_ = 0;
        if(weightsize_<0) weightsize_ = 0;
        if(inputsize_<0) inputsize_ = width_ - targetsize_ - weightsize_;
    }
// */

//*
    bool sizes_are_inconsistent =
        inputsize_ < 0 || targetsize_ < 0 || weightsize_ < 0 ||
        inputsize_ + targetsize_ + weightsize_ != width();
    if( sizes_are_inconsistent )
    {
        int source_is = source->inputsize();
        int source_ts = source->targetsize();
        int source_ws = source->weightsize();
        bool source_sizes_are_inconsistent =
            source_is < 0 || source_ts < 0 || source_ws < 0 ||
            source_is + source_ts + source_ws != source->width();

        // if source sizes are inconsistent too, we assume it's all input
        if( source_sizes_are_inconsistent )
        {
            inputsize_ = width();
            targetsize_ = 0;
            weightsize_ = 0;
        }
        else
        {
            // We can rely on the source sizes, and determine which columns
            // have been cropped
            if( jstart <= source_is )
            {
                inputsize_ = min(source_is - jstart, width());
                targetsize_ = min(source_ts, width() - inputsize());
                weightsize_ = width() - inputsize() - targetsize();
            }
            else if( jstart <= source_is + source_ts )
            {
                inputsize_ = 0;
                targetsize_ = min(source_is + source_ts - jstart, width());
                weightsize_ = width() - targetsize();
            }
            else // jstart <= source_is + source_ts + source_ws
            {
                inputsize_ = 0;
                targetsize_ = 0;
                weightsize_ = width();
            }
        }
    }
    // else, nothing to change
// */

    //  cerr << "inputsize: "<<inputsize_ << "  targetsize:"<<targetsize_<<"weightsize:"<<weightsize_<<endl;
}

void SubVMatrix::reset_dimensions()
{
    int delta_length = source->length()-length_;
    int delta_width = 0; // source->width()-width_; HACK
    source->reset_dimensions();
    length_=source->length()-delta_length;
    width_=source->width()-delta_width;
}

void SubVMatrix::getNewRow(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length())
        PLERROR("In SubVMatrix::getNewRow(i, v) OUT OF BOUND access");
#endif
    source->getSubRow(i+istart, jstart, v);
}

real SubVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length() || j<0 || j>=width())
        PLERROR("In SubVMatrix::get(i,j) OUT OF BOUND access");
#endif
    return source->get(i+istart,j+jstart);
}
void SubVMatrix::getSubRow(int i, int j, Vec v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length() || j<0 || j+v.length()>width())
        PLERROR("In SubVMatrix::getSubRow(i,j,v) OUT OF BOUND access");
#endif
    source->getSubRow(i+istart,j+jstart,v);
}

void SubVMatrix::getMat(int i, int j, Mat m) const
{
#ifdef BOUNDCHECK
    if(i<0 || i+m.length()>length() || j<0 || j+m.width()>width())
        PLERROR("In SubVMatrix::getMat OUT OF BOUND access");
#endif
    source->getMat(i+istart, j+jstart, m);
}

void SubVMatrix::put(int i, int j, real value)
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length() || j<0 || j>=width())
        PLERROR("In SubVMatrix::put(i,j,value) OUT OF BOUND access");
#endif
    return source->put(i+istart,j+jstart,value);
}
void SubVMatrix::putSubRow(int i, int j, Vec v)
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length() || j<0 || j>=width())
        PLERROR("In SubVMatrix::putSubRow(i,j,v) OUT OF BOUND access");
#endif
    source->putSubRow(i+istart,j+jstart,v);
}

void SubVMatrix::putMat(int i, int j, Mat m)
{
#ifdef BOUNDCHECK
    if(i<0 || i+m.length()>length() || j<0 || j+m.width()>width())
        PLERROR("In SubVMatrix::putMat(i,j,m) OUT OF BOUND access");
#endif
    source->putMat(i+istart, j+jstart, m);
}

VMat SubVMatrix::subMat(int i, int j, int l, int w)
{
    return source->subMat(istart+i,jstart+j,l,w);
}

/////////
// dot //
/////////
real SubVMatrix::dot(int i1, int i2, int inputsize) const
{
    if(jstart==0)
        return source->dot(istart+i1, istart+i2, inputsize);
    else
        return inherited::dot(i1, i2, inputsize);
}

real SubVMatrix::dot(int i, const Vec& v) const
{
    if(jstart==0)
        return source->dot(istart+i, v);
    else
        return inherited::dot(i, v);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SubVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

PP<Dictionary> SubVMatrix::getDictionary(int col) const
{
#ifdef BOUNDCHECK
    if(col<0 || col>=width())
        PLERROR("In SubVMatrix::getDictionary(col) OUT OF BOUND access");
#endif
    return source->getDictionary(col+jstart);
}

Vec SubVMatrix::getValues(int row, int col) const
{
#ifdef BOUNDCHECK
    if(row<0 || row>=length() || col<0 || col>=width())
        PLERROR("In SubVMatrix::getValues(row,col) OUT OF BOUND access");
#endif
    return source->getValues(row+istart,col+jstart);
}

Vec SubVMatrix::getValues(const Vec& input, int col) const
{
#ifdef BOUNDCHECK
    if(col<0 || col>=width())
        PLERROR("In SubVMatrix::getValues(row,col) OUT OF BOUND access");
#endif
    return source->getValues(input.subVec(jstart, input.length()-jstart),
                             col+jstart);
}

} // end of namespcae PLearn


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
