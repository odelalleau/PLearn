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

#include "ExtendedVMatrix.h"

namespace PLearn {
using namespace std;


/** ExtendedVMatrix **/

PLEARN_IMPLEMENT_OBJECT(
    ExtendedVMatrix,
    "Extends a VMatrix by filling new rows/columns with a constant",
    "VMatrix that extends the underlying VMat by appending rows at \n"
    "its top and bottom and columns at its left and right.\n"
    "The appended rows/columns are filled with the given fill_value\n"
    "This can be used for instance to easily implement the usual trick \n"
    "to include the bias in the weights vectors, by appending a 1 to\n"
    "the inputs.\n");


ExtendedVMatrix::ExtendedVMatrix(bool call_build_)
    : inherited(call_build_),
      top_extent(0), bottom_extent(0), left_extent(0), right_extent(0),
      fill_value(0)
{
    if( call_build_ )
        build_();
}

ExtendedVMatrix::ExtendedVMatrix(VMat the_source,
                                 int the_top_extent, int the_bottom_extent,
                                 int the_left_extent, int the_right_extent,
                                 real the_fill_value, bool call_build_)
    : inherited(the_source,
                the_source->length()+the_top_extent+the_bottom_extent,
                the_source->width()+the_left_extent+the_right_extent,
                call_build_),
      top_extent(the_top_extent), bottom_extent(the_bottom_extent),
      left_extent(the_left_extent), right_extent(the_right_extent),
      fill_value(the_fill_value)
{
    if( call_build_ )
        build_();
}

void ExtendedVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &ExtendedVMatrix::source,
                  (OptionBase::learntoption | OptionBase::nosave),
                  "DEPRECATED - Use 'source' instead.");
    declareOption(ol, "top_extent", &ExtendedVMatrix::top_extent,
                  OptionBase::buildoption,
                  "Number of rows to add at the top");
    declareOption(ol, "bottom_extent", &ExtendedVMatrix::bottom_extent,
                  OptionBase::buildoption,
                  "Number of rows to add at the bottom");
    declareOption(ol, "left_extent", &ExtendedVMatrix::left_extent,
                  OptionBase::buildoption,
                  "Number of columns to add at the left");
    declareOption(ol, "right_extent", &ExtendedVMatrix::right_extent,
                  OptionBase::buildoption,
                  "Number of columns to add at the right");
    declareOption(ol, "fill_value", &ExtendedVMatrix::fill_value,
                  OptionBase::buildoption,
                  "Value to use to fill the added columns/rows");

    declareOption(
        ol, "extfieldnames", &ExtendedVMatrix::extfieldnames,
        OptionBase::buildoption,
        "The fieldnames to use for the added fields. Length must be equal to\n"
        "left_extent+right_extent.\n"
        "\n"
        "Default: [], i.e all are set to \"extended\"." );

    inherited::declareOptions(ol);
}

void ExtendedVMatrix::build()
{
    inherited::build();
    build_();
}

void ExtendedVMatrix::build_()
{
    this->length_ = source->length() + top_extent  + bottom_extent;
    this->width_  = source->width()  + left_extent + right_extent;

    if ( ! extfieldnames.isEmpty() )
        assert( extfieldnames.length() == left_extent + right_extent );

    TVec<string> fieldnames = source->fieldNames( );
    TVec<string> extended_fieldnames( width() );
    for ( int fno = 0, extno=0; fno < width(); fno++ )
        if ( fno < left_extent )
        {
            if ( extfieldnames.isEmpty() )
                extended_fieldnames[fno] = "extended";
            else
                extended_fieldnames[fno] = extfieldnames[extno++];
        }
        else if ( fno >= width()-right_extent )
        {
            if ( extfieldnames.isEmpty() )
                extended_fieldnames[fno] = "extended";
            else
                extended_fieldnames[fno] = extfieldnames[extno++];
        }
        else
            extended_fieldnames[fno]   = fieldnames[fno-left_extent];

    declareFieldNames( extended_fieldnames );

    setMetaInfoFromSource();
}

void ExtendedVMatrix::getNewRow(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length())
        PLERROR("In ExtendedVMatrix::getNewRow OUT OF BOUNDS");
    if(v.length() != width())
        PLERROR("In ExtendedVMatrix::getNewRow v.length() must be equal to the VMat's width");
#endif

    if(i<top_extent || i>=length()-bottom_extent)
        v.fill(fill_value);
    else
    {
        Vec subv = v.subVec(left_extent, source->width());
        source->getRow(i-top_extent,subv);
        if(left_extent>0)
            v.subVec(0,left_extent).fill(fill_value);
        if(right_extent>0)
            v.subVec(width()-right_extent,right_extent).fill(fill_value);
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ExtendedVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields.
    // ### that you wish to be deepCopied rather than.
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    deepCopyField(extfieldnames, copies);

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
