// -*- C++ -*-

// lexical_cast.cc
//
// Copyright (C) 2005 Christian Dorion 
// Copyright (C) 2006 University of Montreal
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

///////////////
// pl_strtod //
///////////////
double pl_strtod(const char* nptr, char** endptr)
{
#ifdef WIN32
    int i = 0;
    char c;
    while (isspace(c = nptr[i++])) {}
    const char* end_parsing;
    bool success = false;
    bool minus = false;
    bool infinity = false;
    bool missing = false;
    if (c != 0) {
        if (c == '+')
            c = nptr[i++];
        else if (c == '-') {
            c = nptr[i++];
            minus = true;
        }
        if (c == 'i' || c == 'I') {
            // Try to read an 'inf'.
            c = nptr[i++];
            if (c == 'n' || c == 'N') {
                c = nptr[i++];
                if (c == 'f' || c == 'F') {
                    success = true;
                    infinity = true;
                    end_parsing = nptr + i;
                }
            }
        } else if (c == 'n' || c == 'N') {
            // Try to read a 'nan'.
            c = nptr[i++];
            if (c == 'a' || c == 'A') {
                c = nptr[i++];
                if (c == 'n' || c == 'N') {
                    success = true;
                    missing = true;
                    end_parsing = nptr + i;
                }
            }
        } else {
            // Try to read a 'normal' number. In such a case the standard
            // 'strtod' function should work properly.
            return strtod(nptr, endptr);
        }
    }
    if (success) {
        // Ensure there are no weird trailing characters.
        while (isspace(c = nptr[i++])) {}
        if (c != 0)
            success = false;
    }
    if (!success) {
        // Could not perform the conversion.
        if (endptr)
            *endptr = (char*) nptr;
        return 0;
    }
    if (endptr)
        *endptr = (char*) end_parsing;
    if (missing)
        return MISSING_VALUE;
    if (infinity)
        return minus ? - INFINITY : INFINITY;
    PLASSERT( false );
    return 0;
#else
    // Under other operating systems, there shoud be no problems with NaN and
    // Infinity.
    return strtod(nptr, endptr);
#endif
}

///////////////
// pl_strtof //
///////////////
float pl_strtof(const char* nptr, char** endptr)
{
#ifdef WIN32
    return float(pl_strtod(nptr, endptr));
#else
    return strtof(nptr, endptr);
#endif
}

/////////////////
// pl_isnumber //
/////////////////
bool pl_isnumber(const string& str, double* dbl)
{
    double d;
    string s=removeblanks(str);
    char* l;
    d = pl_strtod(s.c_str(),&l);
    if(s=="")d=MISSING_VALUE;
    if(dbl!=NULL)*dbl=d;
    if(s=="")
        return false;
    return ((unsigned char)(l-s.c_str())==s.length());
}

bool pl_isnumber(const string& str, float* dbl) {
    float d;
    string s=removeblanks(str);
    char* l;
    d = pl_strtof(s.c_str(),&l);
    if(s=="")d=MISSING_VALUE;
    if(dbl!=NULL)*dbl=d;
    if(s=="")
        return false;
    return ((unsigned char)(l-s.c_str())==s.length());
}

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
    double result = pl_strtod(nptr,&endptr);
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
