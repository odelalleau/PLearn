// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
//
// This file is part of the PLearn Library. This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation, version 2.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this library; see the file GPL.txt  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// As a special exception, you may compile and link this library with files
// not covered by the GNU General Public License, and distribute the resulting
// executable file under the terms of your choice, without the requirement to
// distribute the complete corresponding source code, provided you have
// obtained explicit written permission to do so from Pascal Vincent (primary
// author of the library) or Yoshua Bengio or the University of Montreal.
// This exception does not however invalidate any other reasons why the
// executable file might be covered by the GNU General Public License.
//
// See the following URL for more information on PLearn:
// http://plearn.sourceforge.net 

 

/* *******************************************************      
   * $Id: TypesNumeriques.h,v 1.1 2004/07/12 13:37:07 tihocan Exp $
   * AUTHORS: Steven Pigeon & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */

// Utilities to translate a text stream into a sequence of tokens
// that are more manageable to model for building a statistical 
// language model. In particular, to convert numeric-looking words
// into a more compact representation where numbers are replaced by
// special codes (see "rules" below).


/*! \file PLearnLibrary/PLearnUtil/TypesNumeriques.h */

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
