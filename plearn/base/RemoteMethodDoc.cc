// -*- C++ -*-

// RemoteMethodDoc.cc
// Copyright (c) 2006 Pascal Vincent

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


#include "RemoteMethodDoc.h"
// #include "stringutils.h"    //!< For addprefix.

namespace PLearn {
using namespace std;

void RemoteMethodDoc::checkConsistency() const
{
    int argdocsize = m_args_doc.size();
    int argtypesize = m_args_type.size();
    if(argdocsize != argtypesize)
        PLERROR("Number of ArgDoc (%d) inconsistent with number of arguments (%d)",argdocsize,argtypesize);
}
    
//! Returns a string repretenting the "prototype" (signature) of the function in the doc.
//! Argsep is used a sthe separator between arguments (typically ", " or ",\n")
string RemoteMethodDoc::getPrototypeString(string argsep) const
{
    string txt = returnType()+" "+name()+"(";
    list<ArgDoc>::const_iterator arg;
    list<ArgDoc>::const_iterator argbegin = argListDoc().begin();        
    list<ArgDoc>::const_iterator argend = argListDoc().end();
    list<string>::const_iterator argtype = argListType().begin();
    for(arg=argbegin; arg!=argend; ++arg, ++argtype)
    {
        if(arg!=argbegin)
            txt += argsep;
        txt += *argtype+" "+arg->m_argument_name;
    }
    txt += ")";
    return txt;
}

string RemoteMethodDoc::getFullHelpText() const
{
    int nargs = nArgs();
    string txt = getPrototypeString() + "\n";
    txt += bodyDoc() + "\n";
    if(nargs==0)
        txt += "TAKES NO ARGUMENTS.\n";
    else
    {
        list<ArgDoc>::const_iterator arg = argListDoc().begin();        
        list<ArgDoc>::const_iterator argend = argListDoc().end();
        list<string>::const_iterator argtype = argListType().begin();
        txt += "ARGUMENTS: \n";
        for(; arg!=argend; ++arg, ++argtype)
        {
            txt += arg->m_argument_name + " : " 
                +  *argtype + "\n"
                +  arg->m_doc + "\n";
        }
    }
    txt += "RETURNS: ";
    txt += returnType() + "\n";
    txt += returnDoc() + "\n";
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
