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

#include "OneHotVMatrix.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;


/** OneHotVMatrix **/

PLEARN_IMPLEMENT_OBJECT(OneHotVMatrix,
        "Transform an index into a one-hot vector.",
        "Sampling from this VMat will return the corresponding sample from\n"
        "the source VMat with last element ('target_classnum') replaced by\n"
        "a vector of target_values of size nclasses in which only\n"
        "target_values[target_classnum] is set to hot_value, and all the\n"
        "others are set to cold_value.\n"
        "In the special case where the VMat is built with nclasses==1, then\n"
        "it is assumed that we have a 2 class classification problem but\n"
        "we are using a single valued target.  For this special case only\n"
        "the_cold_value is used as target for classnum 0 and the_hot_value\n"
        "is used for classnum 1.\n"
);

///////////////////
// OneHotVMatrix //
///////////////////
OneHotVMatrix::OneHotVMatrix(bool call_build_)
    : inherited(call_build_),
      nclasses(0), cold_value(0.0), hot_value(1.0), index(-1)
{
    if( call_build_ )
        build_();
}

OneHotVMatrix::OneHotVMatrix(VMat the_source, int the_nclasses,
                             real the_cold_value, real the_hot_value,
                             int the_index, bool call_build_)
    : inherited(the_source,
                the_source->length(),
                the_source->width()+the_nclasses-1,
                call_build_),
      nclasses(the_nclasses),
      cold_value(the_cold_value),
      hot_value(the_hot_value),
      index(the_index)
{
    if( call_build_ )
        build_();
}

///////////
// build //
///////////
void OneHotVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void OneHotVMatrix::build_()
{
    int source_inputsize = source->inputsize();
    int source_targetsize = source->targetsize();
    int source_weightsize = source->weightsize();
    int source_width = source->width();
    int source_length = source->length();

    length_ = source_length;
    width_ = source_width + nclasses - 1;

    if(source_inputsize+source_targetsize+source_weightsize != source_width
       || source_targetsize != 1 ) // source->sizes are inconsistent
    {
        if( index < 0 )
        {
            index = source_width - 1;
            updateNClassesAndWidth();
        }
        if( inputsize_ + targetsize_ + weightsize_ != width() )
        {
            // sizes are not set, or inconsistently
            inputsize_ = index;
            targetsize_ = nclasses;
            weightsize_ = width() - inputsize_ - targetsize_;
        }
    }
    else // source->sizes are consistent
    {
        if( index < 0 )
        {
            index = source_inputsize;
            updateNClassesAndWidth();
        }
        if( inputsize_ + targetsize_ + weightsize_ != width() )
        {
            inputsize_ = source_inputsize;
            targetsize_ = source_targetsize;
            weightsize_ = source_weightsize;

            if( index < inputsize_ )
                inputsize_ += nclasses-1;
            else if( index < inputsize_ + targetsize_ )
                targetsize_ += nclasses-1;
            else
                weightsize_ += nclasses-1;
        }
    }

    TVec<string> fieldnames = source->fieldNames().copy();

    string target_name = fieldnames[ index ];
    fieldnames.resize( width() );
    for( int i=0 ; i<nclasses ; i++ )
        fieldnames[ index+i ] = target_name + "_" + tostring(i);

    fieldnames.subVec( index + nclasses, width() - index - nclasses )
        << source->fieldNames().subVec( index + 1, source_width - index - 1 );


    declareFieldNames( fieldnames );
    setMetaInfoFromSource();
}

////////////////////
// declareOptions //
////////////////////
void OneHotVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "underlying_distr", &OneHotVMatrix::source,
                  (OptionBase::learntoption | OptionBase::nosave),
                  "DEPRECATED - use 'source' instead.");

    declareOption(ol, "nclasses", &OneHotVMatrix::nclasses,
                  OptionBase::buildoption,
        "Number of classes. If set to zero, then this number will be\n"
        "automatically found from the source VMat.");

    declareOption(ol, "cold_value", &OneHotVMatrix::cold_value,
                  OptionBase::buildoption,
        "Value used for non active elements in the one-hot vector.");

    declareOption(ol, "hot_value", &OneHotVMatrix::hot_value,
                  OptionBase::buildoption,
        "Value used for the active element in the one-hot vector.");

    declareOption(ol, "index", &OneHotVMatrix::index,
                  OptionBase::buildoption,
        "Index of the column on which we apply the one-hot transformation.\n"
        "By default, if targetsize==1 we take the target column, otherwise\n"
        "we take the last column.");
    
    inherited::declareOptions(ol);
}

///////////////
// getNewRow //
///////////////
void OneHotVMatrix::getNewRow(int i, const Vec& samplevec) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length())
        PLERROR("In OneHotVMatrix::getNewRow OUT OF BOUNDS");
    if(samplevec.length()!=width())
        PLERROR("In OneHotVMatrix::getNewRow samplevec.length() must be\n"
                "equal to the VMat's width\n");
#endif
    Vec left = samplevec.subVec(0, index);
    Vec modified = samplevec.subVec(index, nclasses);
    Vec right = samplevec.subVec(index+nclasses, width()-index-nclasses);
    source->getSubRow(i, 0, left);
    int classnum = int(round(source->get(i, index)));
    fill_one_hot(modified, classnum, cold_value, hot_value);
    source->getSubRow(i, index+1, right);
}

/////////
// dot //
/////////
real OneHotVMatrix::dot(int i1, int i2, int inputsize) const
{
    return source->dot(i1, i2, inputsize);
}

real OneHotVMatrix::dot(int i, const Vec& v) const
{
    return source->dot(i, v);
}


////////////////////////////
// updateNClassesAndWidth //
////////////////////////////
void OneHotVMatrix::updateNClassesAndWidth()
{
    if (nclasses > 0)
        return;
    PLASSERT( nclasses == 0 && index >= 0 );
    real max = -1;
    for (int i = 0; i < source->length(); i++) {
        real val = source->get(i, index);
        if (val > max)
            max = val;
    }
    nclasses = int(round(max));
    width_ += nclasses;
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
