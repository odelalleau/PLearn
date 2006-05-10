// PLearn ("A C++ Machine Learning Library")
// Copyright (C) 2002 Pascal Vincent and Julien Keable
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
 * This file is part of the PLearn library.
 ******************************************************* */

#include "StrTableVMatrix.h"
#include <plearn/base/stringutils.h>

namespace PLearn {
using namespace std;

StrTableVMatrix::StrTableVMatrix()
{}


/* This contructor takes a StringTable (which is simply a matrix of string) and converts it to a matrix of reals
   using real->string and string->real maps
*/
StrTableVMatrix::StrTableVMatrix(const StringTable & st)
    : inherited(Mat(st.length(),st.width()))
{
    map<string,real>::iterator it;
    double dbl;
    TVec<int> mapnum(st.width(),0);
    TVec<string> vec(st.width());
    TVec<bool> hasreal;
    Vec colmax;
    hasreal.resize(st.width());
    colmax.resize(st.width());

    for(int j=0;j<st.width();j++)
    {
        hasreal[j]=false;
        colmax[j]=0;
    }

    map_sr.resize(st.width());
    map_rs.resize(st.width());

    for(int j=0;j<st.width();j++)
        declareField(j,st.getFieldName(j), VMField::UnknownType);

    // 1st pass to detect maximums
    for(int i=0;i<st.length();i++)
    {
        vec=st(i);
        for(int j=0;j<st.width();j++)
            if(pl_isnumber(vec[j],&dbl))
            {
                hasreal[j]=true;
                if(!is_missing(dbl))
                    if(colmax[j]<dbl)
                        colmax[j]=dbl;
            }
    }

    for(int j=0;j<st.width();j++)
        if(hasreal[j])
            mapnum[j]=(int)ceil((double)colmax[j])+1;

    for(int i=0;i<st.length();i++)
    {
        vec=st(i);
        for(int j=0;j<st.width();j++)
            if(!pl_isnumber(vec[j],&dbl))
            {
                if((it=map_sr[j].find(vec[j]))==map_sr[j].end())
                {
                    data(i,j)=mapnum[j];
                    map_sr[j][vec[j]]=mapnum[j];
                    map_rs[j][mapnum[j]]=vec[j];
                    mapnum[j]++;
                }
                else data(i,j)=it->second;
            }
            else data(i,j)=dbl;
    }

    // We have modified the 'data' option.
    inherited::build();

}


void StrTableVMatrix::loadAllStringMappings()
{
    // For a StrTableVMatrix, smap are already created
    return;
}


PLEARN_IMPLEMENT_OBJECT(StrTableVMatrix, "ONE LINE DESCR", "NO HELP");

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
