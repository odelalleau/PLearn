// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * AUTHORS: Steven Pigeon & Yoshua Bengio
 * This file is part of the PLearn library.
 ******************************************************* */

// Utilities to translate a text stream into a sequence of tokens
// that are more manageable to model for building a statistical 
// language model. In particular, to convert numeric-looking words
// into a more compact representation where numbers are replaced by
// special codes (see "rules" below).


/*! \file PLearn/plearn/io/TypesNumeriques.h */

#ifndef MODULE_TYPES_NUMERIQUES
#define MODULE_TYPES_NUMERIQUES

#include <iostream>

namespace PLearn {
using namespace std;


extern const char   DIGITsymbols[]; 
extern const char   ALPHAsymbols[];

//!  true if string s contains any one of the characters in symbols.
bool containsChar(const char *s, const char *symbols);

//////////////////////////////////////////////////!<  
typedef enum {
    NT_NOT_NUMERIC          = 0x0000,
    NT_ORDINAL              = 0x0001, 
    NT_CARDINAL             = 0x0002,
    NT_CURRENCY             = 0x0004,
    NT_PREFIXED             = 0x0008,
    NT_SUFFIXED             = 0x0010,
    NT_RANGE                = 0x0020,
    NT_TIME                 = 0x0040, 
    NT_CODE                 = 0x0080, 
    NT_PERCENT              = 0x0100, 
    NT_UNKNOWN_NUMERIC_TYPE = 0x8000  //!<  looks numeric, but none of the above (ana or something)
} eNumericType;


//////////////////////////////////////////////////!<  
typedef struct
{
    char * pattern;
    int attributs;
} tRule;

//////////////////////////////////////////////////!<  
const tRule rules[] = 
{
    {"#an",     NT_CARDINAL + NT_PREFIXED },
    {"#n",      NT_CARDINAL               },
    {"#na",     NT_CARDINAL + NT_SUFFIXED },
    {"#ar",     NT_RANGE + NT_PREFIXED },
    {"#r",      NT_RANGE               },
    {"#ra",     NT_RANGE + NT_SUFFIXED },
    {"#n'a",    NT_ORDINAL  + NT_SUFFIXED },
    {"#ao",     NT_ORDINAL  + NT_PREFIXED },
    {"#o",      NT_ORDINAL                },
    {"#oa",     NT_ORDINAL  + NT_SUFFIXED },
    {"#o'a",    NT_ORDINAL  + NT_SUFFIXED },
    {"#$n",     NT_CURRENCY               },
    {"#$na",    NT_CURRENCY + NT_SUFFIXED },
    {"#$r",     NT_CURRENCY + NT_RANGE    },
    {"#$ra",    NT_CURRENCY + NT_RANGE + NT_SUFFIXED },
    {"#n:n",    NT_TIME },
    {"#n:n:n",  NT_TIME },
    {"#r:n",    NT_CODE },
    {"#n:r",    NT_CODE },
    {"",  NT_UNKNOWN_NUMERIC_TYPE}
};

//////////////////////////////////////////////////!<  


const char *eNumericTypeNames(int a);   //!<  converts a code in corresponding string
int numericType(const char *word);      //!<  assigns a code to a "word"
bool looksNumeric(const char *s);     //!<  tells wether this string looks like a numeric entity
void compactRepresentation(char *t); //!<  gives a (intermediate) code for a numeric string (starting with #)

} // end of namespace PLearn

#endif 
//MODULE_TYPES_NUMERIQUES


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
