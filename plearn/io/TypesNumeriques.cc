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
   * $Id: TypesNumeriques.cc,v 1.1 2004/07/12 13:37:07 tihocan Exp $
   * AUTHORS: Steven Pigeon & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */

#include <cstdlib>
#include <cstring>
#include "TypesNumeriques.h"

namespace PLearn {
using namespace std;


//////////////////////////////////////////////////
const char    DIGITsymbols[] = "0123456789";
const char    ALPHAsymbols[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *ORDINALS[] = {"d","nd","th","st",0}; // 3d, 2nd, 12th, 1st

//////////////////////////////////////////////////
const char *eNumericTypeNames(int a)
 {
  static char retour[128];

  // all that applies
  if (a==NT_NOT_NUMERIC)
       return "not numeric";
  else {
        retour[0]=0;
        if (a & NT_ORDINAL)  strcat(retour,"ordinal ");
        if (a & NT_CARDINAL) strcat(retour,"cardinal ");
        if (a & NT_CURRENCY) strcat(retour,"currency ");
        if (a & NT_PREFIXED) strcat(retour,"prefixe ");
        if (a & NT_SUFFIXED) strcat(retour,"suffixe ");
        if (a & NT_PERCENT)  strcat(retour,"pourcentage ");
        if (a & NT_RANGE)    strcat(retour,"range ");
        if (a & NT_TIME)     strcat(retour,"temps ");
        if (a & NT_CODE)     strcat(retour,"code ");
        if (a & NT_UNKNOWN_NUMERIC_TYPE) strcat(retour," ??? ");
        return retour;
       }
 }

//////////////////////////////////////////////////
bool containsChar(const char *s, const char *symbols)
 {
  bool found = false;
  int  i=0;
  while (!found && symbols[i])
   {
    found = (bool)strchr(s,symbols[i]);
    i++;
   }
  return found;
 }


//////////////////////////////////////////////////
char * stringPos(const char *s, const char *strings[])
 {
  char *t = 0;
  int  i=0;
  while (!t && strings[i])
   {
    t = strstr(s,strings[i]);
    i++;
   }
  return t;
 }


//////////////////////////////////////////////////
bool looksNumeric(const char *s)
 {
  return containsChar(s,DIGITsymbols);
 }

//////////////////////////////////////////////////
bool elementOf(const char *s, const char t)
 {
  return (bool)strchr(s,t);
 }

//////////////////////////////////////////////////
void compactRepresentationTranslate(char *t)
 {
  int d=0;
  int s=0;

  while (t[s])
   { 
    if (elementOf(DIGITsymbols,t[s]))
         {
          t[d++]='n';
          // skip to the next non-digit
          do { s++; } while (t[s] && (elementOf(DIGITsymbols,t[s]) || (t[s]==',')) );
         }
    else if (elementOf(ALPHAsymbols,t[s]))
              {
               if ( (stringPos(&t[s],ORDINALS)==&t[s]) // starts here
                    && (t[d-1]=='n') ) // and the previous run was composed of digits
                    t[d++]='o';
               else t[d++]='a';
               // skip to the next non-alpha
               do { s++; } while (t[s] && elementOf(ALPHAsymbols,t[s]));
              }
         else t[d++]=t[s++];
   }
  t[d]=0;

 }

//////////////////////////////////////////////////
void compactRepresentationShrinkNum(char *t)
 {
  // remplace n.n ou .n par n, 
  // mais laisse les constructions du genre n.n.n intactes
  int d=0;
  int s=0;

  while (t[s])
   {
    if ( (strstr(&t[s],"n.n") == &t[s]) && 
         (t[s+3]!='.') && 
         ( (s-1<0) || (t[s-1]!='.') )
         )
         {
          t[d++]='n';
          s+=3;
         }
    else if ( (strstr(&t[s],".n") == &t[s]) &&
              (t[s+2]!='.') &&
              ( (s-1<0) || t[s-1]!='n')) 
              {
               t[d++]='n';
               s+=2;
              }
         else t[d++]=t[s++];
   }
  t[d]=0;
 }

//////////////////////////////////////////////////
void compactRepresentationRangesAndOrdinals(char *t)
 {
  // remplace n-n par r et no par o
  int d=0;
  int s=0;

  while (t[s])
   {
    if ( strstr(&t[s],"n-n") == &t[s])
         {
          t[d++]='r';
          s+=3;
         }
    else if ( strstr(&t[s],"no") == &t[s])
              {
               t[d++]='o';
               s+=2;
              }
         else t[d++]=t[s++];
   }
  t[d]=0;
 }

//////////////////////////////////////////////////
void compactRepresentation(char *t)
 {
  compactRepresentationTranslate(t); // remplace les lettres et chiffres par des codes.
  compactRepresentationShrinkNum(t); // replace n.n par n, etc.
  compactRepresentationRangesAndOrdinals(t); // remplace n-n par r et no par o

  int s=0;
  int d=0;

  // strip les tirets -  
  while (t[s]) 
   if (t[s]!='-') 
        t[d++]=t[s++]; 
   else s++;

  t[d]=0;

  // copie une seule instance du meme symbole.
  s=0;
  d=0;
  while (t[s])
   {
    t[d++]=t[s++];
    while (t[s] && (t[s]==t[d-1])) s++;
   }

  if (t[d-1]=='.') d--; // trailing .
  t[d]=0;

  char c = '#';
  d=0;
  do 
   {
    char tt = t[d];
    t[d]=c;
    c=tt;
    d++;
   } while (c);
  t[d]=0;
 }

//////////////////////////////////////////////////
int numericType(const char *mot)
 {
  if (looksNumeric(mot))
       {
        int classe=0;
        char t[128];
        bool pourcent=false;
        strcpy(t,mot);

        compactRepresentation(t); 

        // skips the # in the begining

        if (char *tt= strchr(t,'%')) 
         *tt=0, pourcent = true; // delete trailing %

        for (int i=0; (rules[i].pattern[0]) && (!classe); i++)
         if (strcmp(rules[i].pattern,t)==0) classe = rules[i].attributs;

        if (pourcent) classe += NT_PERCENT;

        return classe ?  classe : NT_UNKNOWN_NUMERIC_TYPE;
       }
  else return NT_NOT_NUMERIC;
 }



} // end of namespace PLearn
