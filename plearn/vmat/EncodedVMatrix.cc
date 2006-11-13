// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2006 Xavier Saint-Mleux
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

#include "EncodedVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(EncodedVMatrix,
                        "Applies an encoding (map) to each column of a sub-matrix.",
                        ""
                       );

EncodedVMatrix::EncodedVMatrix(bool call_build_)
    : inherited(call_build_)
{
    if(call_build_)
        build_();
}

EncodedVMatrix::EncodedVMatrix(VMat the_source, TVec<map<real,real> > encodings_, TVec<real> defaults_, 
                               TVec<bool> encode_, bool call_build_)
    : inherited(the_source,
                the_source->length(),
                the_source->width(),
                call_build_),
      encodings(encodings_),
      defaults(defaults_),
      encode(encode_)
{
    if(call_build_)
        build_();
}


void EncodedVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "encodings", &EncodedVMatrix::encodings,
                  OptionBase::buildoption,
                  "Maps used to encode fields.");

    declareOption(ol, "defaults", &EncodedVMatrix::defaults,
                  OptionBase::buildoption,
                  "Default values to use if no encoding is present for some field value.");

    declareOption(ol, "encode", &EncodedVMatrix::encode,
                  OptionBase::buildoption,
                  "True if this column should be encoded (one boolean per col.)");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void EncodedVMatrix::build_()
{
    if(source)
    {
        copySizesFrom(source);
        declareFieldNames(source->fieldNames());
        for(int i= 0; i < source.width(); ++i)
        {
            if(i < encode.length() && encode[i])
            {
                deleteStringMapping(i);
                map<string,real> mm= source->getStringToRealMapping(i);
                for(map<string,real>::iterator it= mm.begin(); it != mm.end(); ++it)
                    addStringMapping(i, it->first, encodings[i][it->second]);
            }
            else
                setStringMapping(i, source->getStringToRealMapping(i));
        }
    }
    
}

///////////////
// getNewRow //
///////////////
void EncodedVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getRow(i, v);
    encodeRow(encodings, defaults, encode, v.subVec(0, inputsize()));
}

void EncodedVMatrix::encodeRow(const TVec<map<real,real> >& encodings, const TVec<real>& defaults, 
                               const TVec<bool>& encode, const Vec& v)
{
    int l= v.length();
    if(l != encodings.length() ||
       l != defaults.length() ||
       l != encode.length())
        PLERROR("in EncodedVMatrix::encodeRow: encodings, defaults and encode should be the same"
                " size as a row's inputsize. (%d / %d / %d / %d)", encodings.length(), defaults.length(), encode.length(), l);
    
    for(int j= 0; j < v.length() && j < encodings.length(); ++j)
        if(encode[j])
        {
            map<real, real> enc= encodings[j];
            map<real, real>::iterator it= enc.find(v[j]);
            if(it == enc.end())
                v[j]= defaults[j];
            else
                v[j]= it->second;
        }
}


///////////
// build //
///////////
void EncodedVMatrix::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void EncodedVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    PLERROR("EncodedVMatrix::makeDeepCopyFromShallowCopy fully implemented?");
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(encodings, copies);
    deepCopyField(defaults, copies);
    deepCopyField(encode, copies);
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
