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

#include "GeneralizedOneHotVMatrix.h"

namespace PLearn {
using namespace std;


/** GeneralizedOneHotVMatrix **/

PLEARN_IMPLEMENT_OBJECT(GeneralizedOneHotVMatrix,
                        "ONE LINE DESC", "ONE LINE HELP");

GeneralizedOneHotVMatrix::GeneralizedOneHotVMatrix(bool call_build_)
    : inherited(call_build_)
{
    if( call_build_ )
        build_();
}

GeneralizedOneHotVMatrix::GeneralizedOneHotVMatrix(VMat the_source,
                                                   Vec the_index,
                                                   Vec the_nclasses,
                                                   Vec the_cold_value,
                                                   Vec the_hot_value,
                                                   bool call_build_)
    : inherited(the_source,
                the_source->length(),
                the_source->width()
                    + (int)sum(the_nclasses)-the_nclasses.length(),
                call_build_),
      index(the_index), nclasses(the_nclasses),
      cold_value(the_cold_value), hot_value(the_hot_value)
{
    if (min(index)<0 || max(index)>source->length()-1)
        PLERROR("In GeneralizedOneHotVMatrix: all values of index must be in"
                " range [0,%d]",
                source->length()-1);

    if (index.length()!=nclasses.length() ||
        cold_value.length()!=hot_value.length() ||
        index.length()!=hot_value.length())
        PLERROR("In GeneralizedOneHotVMatrix: index, nclasses, cold_value\n"
                "and hot_value must have same length.\n");

    if (call_build_)
        build_();
}

void GeneralizedOneHotVMatrix::build()
{
    inherited::build();
    build_();
}

void GeneralizedOneHotVMatrix::build_()
{
    length_ = source->length();
    width_ = source->width() + (int)sum(nclasses) - nclasses.length();

    TVec<string> fieldnames = source->fieldNames();
    TVec<string> extended_fieldnames( width() );
    int efn_pos = 0;
    for( int j=0 ; j<fieldnames.length() ; j++ )
    {
        const int index_pos = vec_find(index, (real)j);
        if( index_pos == -1 )
            extended_fieldnames[efn_pos++] = fieldnames[j];
        else
        {
            const int nb_class = (int)nclasses[index_pos];
            string name = fieldnames[j];
            for( int k=0 ; k<nb_class ; k++ )
                extended_fieldnames[efn_pos++] = name + "_" + tostring(k);

        }
    }
    declareFieldNames( extended_fieldnames );
    setMetaInfoFromSource();
}

void GeneralizedOneHotVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &GeneralizedOneHotVMatrix::source,
                  (OptionBase::learntoption | OptionBase::nosave),
                  "DEPRECATED - use 'source' instead.");
    declareOption(ol, "index", &GeneralizedOneHotVMatrix::index,
                  OptionBase::buildoption, "");
    declareOption(ol, "nclasses", &GeneralizedOneHotVMatrix::nclasses,
                  OptionBase::buildoption, "");
    declareOption(ol, "cold_value", &GeneralizedOneHotVMatrix::cold_value,
                  OptionBase::buildoption, "");
    declareOption(ol, "hot_value", &GeneralizedOneHotVMatrix::hot_value,
                  OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void GeneralizedOneHotVMatrix::getNewRow(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length())
        PLERROR("In OneHotVMatrix::getNewRow OUT OF BOUNDS");
    if(v.length()!=width())
        PLERROR("In GeneralizedOneHotVMatrix::getNewRow v.length() must be\n"
                "equal to the VMat's width.\n");
#endif

    Vec input(source->width());
    source->getRow(i, input);
    int v_pos = 0;
    for (int j=0; j<input.length(); j++) {
        const int index_pos = vec_find(index, (real)j);
        if (index_pos == -1)
            v[v_pos++] = input[j];
        else {
            const int nb_class = (int)nclasses[index_pos];
            Vec target = v.subVec(v_pos, nb_class);
            const real cold = cold_value[index_pos];
            const real hot = hot_value[index_pos];
            const int classnum = int(source->get(i,j));
            fill_one_hot(target, classnum, cold, hot);
            v_pos += nb_class;
        }
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GeneralizedOneHotVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields.
    // ### that you wish to be deepCopied rather than.
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    deepCopyField(index, copies);
    deepCopyField(nclasses, copies);
    deepCopyField(cold_value, copies);
    deepCopyField(hot_value, copies);
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
