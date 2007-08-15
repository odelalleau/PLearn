// -*- C++ -*-

// RemoteMethodMap.cc
//
// Copyright (C) 2006 Nicolas Chapados
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

// Authors: Nicolas Chapados

/*! \file RemoteMethodMap.cc */

#include "RemoteMethodMap.h"

namespace PLearn {
using namespace std;

//#####  RemoteMethodMap Implementation  ######################################

RemoteMethodMap::~RemoteMethodMap()
{ }

bool RemoteMethodMap::insert(const string& methodname, int arity,
                             PP<RemoteTrampoline> trampoline)
{
    return m_methods.insert(make_pair(make_pair(methodname,arity),
                                      trampoline)).second;
}

RemoteMethodMap::MethodMap::size_type
RemoteMethodMap::erase(const string& methodname, int arity)
{
    return m_methods.erase(make_pair(methodname,arity));
}

const RemoteTrampoline* RemoteMethodMap::lookup(const string& methodname, int arity,
                                                bool search_inherited) const
{
    MethodMap::const_iterator it = m_methods.find(make_pair(methodname,arity));
    if (it != m_methods.end())
        return it->second;
    else if (search_inherited && m_inherited)
        return m_inherited->lookup(methodname,arity,search_inherited);
    else
        return 0;
}

vector< pair<string, int> > RemoteMethodMap::getMethodList() const
{
    int n = size();
    vector< pair<string,int> > flist(n);
    RemoteMethodMap::MethodMap::const_iterator it = begin();
    for(int i=0; i<n; ++i, ++it)
        flist[i] = it->first;
    return flist;
}

vector<string> RemoteMethodMap::getMethodPrototypes() const
{
    int n = size();
    vector<string> prototypes(n);
    RemoteMethodMap::MethodMap::const_iterator it = begin();
    for(int i=0; i<n; ++i, ++it)
        prototypes[i] = it->second->documentation().getPrototypeString();
    return prototypes;
}

string RemoteMethodMap::getMethodHelpText(const string& methodname, int arity) const
{
    string txt;
    if(arity>=0)
    {
        const RemoteTrampoline* tramp = lookup(methodname, arity, true);
        if(tramp==0)
            PLERROR("RemoteMethodMap contains no method %s with %d arguments",methodname.c_str(), arity);
        txt = tramp->documentation().getFullHelpText();
    }
    else
    {
        RemoteMethodMap::MethodMap::const_iterator it = begin();
        RemoteMethodMap::MethodMap::const_iterator itend = end();
        for(; it!=itend; ++it)
        {
            if(it->first.first==methodname)
                txt += it->second->documentation().getFullHelpText()+"\n";
        }
        if(txt=="")
            txt = "** No method named "+methodname+ " **\n";
    } 
    return txt;
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
