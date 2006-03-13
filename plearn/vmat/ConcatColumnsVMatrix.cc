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

#include "ConcatColumnsVMatrix.h"
#include <plearn/base/tostring.h>

namespace PLearn {
using namespace std;

/** ConcatColumnsVMatrix **/

PLEARN_IMPLEMENT_OBJECT(ConcatColumnsVMatrix, "ONE LINE DESCR", "NO HELP");

ConcatColumnsVMatrix::ConcatColumnsVMatrix(TVec<VMat> the_sources)
    : sources(the_sources),
      no_duplicate_fieldnames(false)
{
    if (sources.size())
        build_();
}

ConcatColumnsVMatrix::ConcatColumnsVMatrix(VMat d1, VMat d2)
    : sources(2),
      no_duplicate_fieldnames(false)
{
    sources[0] = d1;
    sources[1] = d2;
    build_();
}


void ConcatColumnsVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "array", &ConcatColumnsVMatrix::sources,
                  (OptionBase::learntoption | OptionBase::nosave),
                  "DEPRECATED - Use 'sources' instead.");

    declareOption(ol, "sources", &ConcatColumnsVMatrix::sources,
                  OptionBase::buildoption,
                  "The VMat to concatenate (horizontally).");

    declareOption(ol, "no_duplicate_fieldnames",
                  &ConcatColumnsVMatrix::no_duplicate_fieldnames,
                  OptionBase::buildoption,
                  "If set to 1, will ensure no fieldnames are duplicated by"
                  " adding\n"
                  "numerical values '.1', '.2', ...\n");

    inherited::declareOptions(ol);
}

void ConcatColumnsVMatrix::build()
{
    inherited::build();
    build_();
}

void ConcatColumnsVMatrix::build_()
{
    length_ = width_ = 0;
    if(sources.size())
        length_ = sources[0]->length();
//   else
//     PLERROR("ConcatColumnsVMatrix expects >= 1 underlying-sources, got 0");

    for(int i=0; i<sources.size(); i++)
    {
        if(sources[i]->length()!=length_)
            PLERROR("ConcatColumnsVMatrix: Problem concatenating two VMat with"
                    " different lengths");
        if(sources[i]->width() == -1)
            PLERROR("In ConcatColumnsVMatrix constructor. Non-fixed width"
                    " distribution not supported");
        width_ += sources[i]->width();
    }

    // Copy the original fieldinfos.  Be careful if only some of the
    // matrices have fieldinfos
    fieldinfos.resize(width_);
    TVec<string> names;
    int fieldindex = 0;
    init_map_sr();
    for (int i=0; i<sources.size(); ++i) 
    {
        int len = sources[i]->getFieldInfos().size();
        if (len > 0) // infos exist for this VMat
        {
            for (int j=0; j<len; ++j) {
                map<string,real> map = sources[i]->getStringToRealMapping(j);
                if (!map.empty())
                    setStringMapping(fieldindex, map);
                fieldinfos[fieldindex++] = sources[i]->getFieldInfos()[j];
            }
        }
        else // infos don't exist for this VMat, use the index as the name for those fields.
        {
            len = sources[i]->width();
            for(int j=0; j<len; ++j) {
                map<string,real> map = sources[i]->getStringToRealMapping(j);
                if (!map.empty())
                    setStringMapping(fieldindex, map);
                fieldinfos[fieldindex] = VMField(tostring(fieldindex));
                fieldindex++;
            }
        }
    }
    if (no_duplicate_fieldnames) {
        unduplicateFieldNames();
    }
}

void ConcatColumnsVMatrix::getNewRow(int i, const Vec& samplevec) const
{
    if (length_==-1)
        PLERROR("In ConcatColumnsVMatrix::getNewRow(int i, Vec samplevec) not supported for distributions with different (or infinite) lengths\nCall sample without index instead");
    int pos = 0;
    for(int n=0; n<sources.size(); n++)
    {
        int nvars = sources[n]->width();
        Vec samplesubvec = samplevec.subVec(pos, nvars);
        sources[n]->getRow(i,samplesubvec);
        pos += nvars;
    }
}

real ConcatColumnsVMatrix::getStringVal(int col, const string & str) const
{
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
    int pos=0,k=0;
    while(col>=pos+sources[k]->width())
    {
        pos += sources[k]->width();
        k++;
    }
//  return sources[k]->getStringVal(pos+col,str);
    return sources[k]->getStringVal(col-pos,str);
}

string ConcatColumnsVMatrix::getValString(int col, real val) const
{
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
    int pos=0,k=0;
    while(col>=pos+sources[k]->width())
    {
        pos += sources[k]->width();
        k++;
    }
//  return sources[k]->getValString(pos+col,val);
    return sources[k]->getValString(col-pos,val);
}

const map<string,real>& ConcatColumnsVMatrix::getStringMapping(int col) const
{
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
    int pos=0,k=0;
    while(col>=pos+sources[k]->width())
    {
        pos += sources[k]->width();
        k++;
    }
//  return sources[k]->getStringToRealMapping(pos+col);
    return sources[k]->getStringToRealMapping(col-pos);
}

string ConcatColumnsVMatrix::getString(int row, int col) const
{
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
    int pos=0,k=0;
    while(col>=pos+sources[k]->width())
    {
        pos += sources[k]->width();
        k++;
    }
//  return sources[k]->getString(row,pos+col);
    return sources[k]->getString(row,col-pos);
}



real ConcatColumnsVMatrix::dot(int i1, int i2, int inputsize) const
{
    real res = 0.;
    for(int k=0; ;k++)
    {
        const VMat& vm = sources[k];
        int vmwidth = vm.width();
        if(inputsize<=vmwidth)
        {
            res += vm->dot(i1,i2,inputsize);
            break;
        }
        else
        {
            res += vm->dot(i1,i2,vmwidth);
            inputsize -= vmwidth;
        }
    }
    return res;
}

real ConcatColumnsVMatrix::dot(int i, const Vec& v) const
{
    if (length_==-1)
        PLERROR("In ConcatColumnsVMatrix::getRow(int i, Vec samplevec) not supported for distributions with different (or infinite) lengths\nCall sample without index instead");

    real res = 0.;
    int pos = 0;
    for(int n=0; n<sources.size(); n++)
    {
        int nvars = std::min(sources[n]->width(),v.length()-pos);
        if(nvars<=0)
            break;
        Vec subv = v.subVec(pos, nvars);
        res += sources[n]->dot(i,subv);
        pos += nvars;
    }
    return res;
}

PP<Dictionary> ConcatColumnsVMatrix::getDictionary(int col) const
{
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
    int pos=0,k=0;
    while(col>=pos+sources[k]->width())
    {
        pos += sources[k]->width();
        k++;
    }
    return sources[k]->getDictionary(col-pos);
}


Vec ConcatColumnsVMatrix::getValues(int row, int col) const
{
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
    int pos=0,k=0;
    while(col>=pos+sources[k]->width())
    {
        pos += sources[k]->width();
        k++;
    }
    return sources[k]->getValues(row,col-pos);
}

Vec ConcatColumnsVMatrix::getValues(const Vec& input, int col) const
{
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
    int pos=0,k=0;
    while(col>=pos+sources[k]->width())
    {
        pos += sources[k]->width();
        k++;
    }
    return sources[k]->getValues(input, col-pos);

}

void ConcatColumnsVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(sources, copies);
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
