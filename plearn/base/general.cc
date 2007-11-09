// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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

#include "general.h"
#include <sys/stat.h>
#include <nspr/prsystem.h>
#include <plearn/base/tostring.h>
#ifdef _MSC_VER
#include <io.h>
#endif

//#include "stringutils.h"

namespace PLearn {
using namespace std;

static PLearnInit _plearn_init_;

#if defined(WIN32) && !defined(__CYGWIN__)
#include <io.h> 
// norman: potentially dangerous if there is a function called with the same name in this
//         file. Beware!
#define umask _umask
#endif 

PLearnInit::PLearnInit()
{
    umask(002);
}

PLearnInit::~PLearnInit(){}

char* strcopy(char* s)
{
    if (!s) return 0;
    char* ss=new char[strlen(s)+1];
    strcpy(ss,s);
    return ss;
}

// print a number without unnecessary trailing zero's, into buffer
void pretty_print_number(char* buffer, real number)
{
    char* t;
    char* s;
    double dnum = double(number);
    sprintf(buffer,"%.15g",dnum);
    for (s=buffer; *s!='\0'; s++)
        if (*s == '.') break;
    if (*s == '.')
    {
        for (t = s + 1; isdigit(*t); t++)
            if (*t != '0')
                s = t + 1;
        for (;*t != '\0';) *s++ = *t++;
        *s = '\0';
    }   
}

bool isMapKeysAreInt(map<real,int>& m)
{
    map<real,int>::iterator it;
    for (it = m.begin(); it!= m.end(); ++it)
    {
        real key_rvalue = it->first;
        int key_ivalue = int(key_rvalue);
        if (!fast_exact_is_equal(key_rvalue, key_ivalue))
            return false;
    }
    return true;
}


string hostname()
{
    char tmp[1024];
    if(PR_GetSystemInfo(PR_SI_HOSTNAME,tmp,500)==PR_SUCCESS)
        return tostring(tmp);
    else{
        char* h = getenv("HOSTNAME");
        if (!h)
            h = getenv("HOST");
        if (!h)
            PLERROR("hostname: could not find the host name from NSPR"
                    " or from the variable $HOSTNAME or $HOST in environment!");
        return h;
    }
}

string prgname(const string& setname)
{
    static string prgname_ = "plearn";
    if(setname!="")
        prgname_ = setname;
    return prgname_;
}

#ifdef WIN32
#undef umask
#endif 
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
