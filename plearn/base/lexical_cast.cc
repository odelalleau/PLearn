// -*- C++ -*-

// lexical_cast.cc
//
// Copyright (C) 2005 Christian Dorion 
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

// Authors: Christian Dorion

/*! \file lexical_cast.cc */


#include "lexical_cast.h"
#include <plearn/base/plerror.h>
#include <plearn/math/pl_math.h>

// For removeblanks: REMOVE AS SOON AS POSSIBLE
#include <plearn/base/stringutils.h> 

namespace PLearn {
using namespace std;

// this function handle numbers with exponents (such as 10.2E09)
// as well as Nans. String can have trailing whitespaces on both sides
bool pl_isnumber(const string& str, double* dbl)
{
    double d;
    string s=removeblanks(str);
    char* l;
    d = strtod(s.c_str(),&l);
    if(s=="")d=MISSING_VALUE;
    if(dbl!=NULL)*dbl=d;
    return ((unsigned char)(l-s.c_str())==s.length());
}

// norman: there is no strtof in .NET
#ifndef WIN32
bool pl_isnumber(const string& str, float* dbl) {
    float d;
    string s=removeblanks(str);
    char* l;
    d = strtof(s.c_str(),&l);
    if(s=="")d=MISSING_VALUE;
    if(dbl!=NULL)*dbl=d;
    return ((unsigned char)(l-s.c_str())==s.length());
}
#endif // WIN32

// Return true if conversion to a long is possible
bool pl_islong(const string& s)
{
    // c_str() might yield very short-lived temporaries, don't assume those
    // pointers live beyond the syntactic expression.
    const char *nptr;
    char *endptr;
    strtol((nptr=s.c_str()), &endptr, 10);
    return endptr && (endptr - nptr == int(s.size()));
}


long tolong(const string& s, int base)
{
    const char* nptr = s.c_str();
    char* endptr;
    long result = strtol(nptr,&endptr,base);
    if(endptr==nptr) { // no character to be read
        string err = string("in toint string is not an int: ") + s;
        PLERROR(err.c_str());
    }
    return result;
}

double todouble(const string& s)
{
    const char* nptr = s.c_str();
    char* endptr;
    double result = strtod(nptr,&endptr);
    if(endptr==nptr) // no character to be read
        result = MISSING_VALUE;
    return result;
}

bool tobool(const string& s)
{
    if (s=="true" || s=="1") return true;
    if (s=="false" || s=="0") return false;
    PLERROR("tobool: can't convert string %s into a boolean",s.c_str());
    return false;
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
