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

#include "RemapLastColumnVMatrix.h"

namespace PLearn {
using namespace std;


/** RemapLastColumnVMatrix **/

PLEARN_IMPLEMENT_OBJECT(RemapLastColumnVMatrix, "ONE LINE DESC", "NO HELP");

RemapLastColumnVMatrix::RemapLastColumnVMatrix(bool call_build_)
    : inherited(call_build_), if_equals_val(0), then_val(0), else_val(0)
{
    if( call_build_ )
        build_();
}

RemapLastColumnVMatrix::RemapLastColumnVMatrix(VMat the_source,
                                               Mat the_mapping,
                                               bool call_build_)
    : inherited (the_source,
                 the_source->length(),
                 the_source->width()+the_mapping.width()-2,
                 call_build_),
      mapping(the_mapping)
{
    if( call_build_ )
        build_();
}

RemapLastColumnVMatrix::RemapLastColumnVMatrix(VMat the_source,
                                               real if_equals_value,
                                               real then_value,
                                               real else_value,
                                               bool call_build_)
    : inherited(the_source,
                the_source->length(),
                the_source->width(),
                call_build_),
      if_equals_val(if_equals_value),
      then_val(then_value),
      else_val(else_value)
{
    if( call_build_ )
        build_();
}

void RemapLastColumnVMatrix::build()
{
    inherited::build();
    build_();
}

void RemapLastColumnVMatrix::build_()
{
    int n_extra = mapping.width() - 2;
    if( mapping.isEmpty() )
        width_=source->width();
    else
        width_=source->width() + n_extra;

    if( !mapping.isEmpty() && n_extra > 0 )
    {
        // width() is different from source->width(),
        int n_last = source->width()-1;

        // determine sizes
        int source_is = source->inputsize();
        int source_ts = source->targetsize();
        int source_ws = source->weightsize();

        bool specified_sizes_are_inconsistent =
            inputsize_ < 0 || targetsize_ < 0 || weightsize_ <0 ||
            inputsize_ + targetsize_ + weightsize_ != width();

        if( specified_sizes_are_inconsistent )
        {
            if( source_is < 0 || source_ts < 0 || source_ws < 0 ||
                source_is + source_ts + source_ws != source->width() )
            {
                //source sizes are inconsistent, everything is input (default)
                inputsize_ = width();
                targetsize_ = 0;
                weightsize_ = 0;
            }
            else if( n_last < source_is )
            {
                // last column is input, remapped columns are considered input
                // other sizes are 0
                inputsize_ = source_is + n_extra;
                targetsize_ = 0;
                weightsize_ = 0;
            }
            else if( n_last < source_is + source_ts )
            {
                // last column is target, remapped columns are
                // considered target other sizes are set from source
                inputsize_ = source_is;
                targetsize_ = source_ts + n_extra;
                weightsize_ = 0;
            }
            else
            {
                inputsize_ = source_is;
                targetsize_ = source_ts;
                weightsize_ = source_ws + n_extra;
            }
        }
        // else don't modify specified sizes

        // set fieldname for added columns
        TVec<string> source_fieldnames = source->fieldNames();
        string last_fieldname = source_fieldnames[n_last];
        TVec<string> fieldnames( source_fieldnames.copy() );
        fieldnames.resize( width() );
        for( int i=0 ; i<=n_extra ; i++ )
        {
            fieldnames[n_last+i] = last_fieldname + "_" + tostring(i);
        }
        declareFieldNames( fieldnames );

    }
    setMetaInfoFromSource();
}

void RemapLastColumnVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "underlying_distr",
                  &RemapLastColumnVMatrix::source,
                  OptionBase::buildoption,
                  "DEPRECATED - Use 'source' instead.");

    declareOption(ol, "mapping", &RemapLastColumnVMatrix::mapping,
                  OptionBase::buildoption, "");

    declareOption(ol, "if_equals_val", &RemapLastColumnVMatrix::if_equals_val,
                  OptionBase::buildoption, "");

    declareOption(ol, "then_val", &RemapLastColumnVMatrix::then_val,
                  OptionBase::buildoption, "");

    declareOption(ol, "else_val", &RemapLastColumnVMatrix::else_val,
                  OptionBase::buildoption, "");

    inherited::declareOptions(ol);
}

void RemapLastColumnVMatrix::getNewRow(int i, const Vec& samplevec) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length())
        PLERROR("In RemapLastColumnVMatrix::getNewRow OUT OF BOUNDS");
    if(samplevec.length()!=width())
        PLERROR("In RemapLastColumnVMatrix::getNewRow samplevec.length()\n"
                "must be equal to the VMat's width (%d != %d).\n",
                samplevec.length(), width());
#endif
    if(mapping.isEmpty()) // use if-then-else mapping
    {
        source->getRow(i,samplevec);
        real& lastelem = samplevec.lastElement();
        if(fast_exact_is_equal(lastelem, if_equals_val))
            lastelem = then_val;
        else
            lastelem = else_val;
    }
    else // use mapping matrix
    {
        int source_width = source->width();
        int replacement_width = mapping.width()-1;
        source->getRow(i,samplevec.subVec(0,source_width));
        real val = samplevec[source_width-1];
        int k;
        for(k=0; k<mapping.length(); k++)
        {
            if(fast_exact_is_equal(mapping(k,0), val))
            {
                samplevec.subVec(source_width-1,replacement_width)
                    << mapping(k).subVec(1,replacement_width);
                break;
            }
        }
        if(k>=mapping.length())
            PLERROR("In RemapLastColumnVMatrix::getNewRow - there is a value"
                    " in the\n"
                    "last column that does not have any defined mapping.\n");
    }
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
