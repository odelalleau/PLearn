// -*- C++ -*-

// PPath.h
//
// Copyright (C) 2005 Pascal Vincent 
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
   * $Id: PPath.h,v 1.1 2005/01/06 19:35:01 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent, Christian Dorion, Nicolas Chapados

/*! \file PPath.h */


#ifndef PPath_INC
#define PPath_INC

// Put includes here
#include <mozilla/nspr/prenv.h>
#include <plearn/io/PStream.h>

namespace PLearn {


  /*

    Un PPath est une string se comportant exactement comme une std:string à deux détails près:
    * la sérialisation effectue une traduction entre une forme canonique et une forme de filepath appropriée pour l'os courant.
    * il y a des méthodes supplémentaires pour gérer les opérations de path

    Les PPath écrits dans un PStream ont toujours la forme
    canonique:

    PPath:AXADB:toto/tutu
    PPath:PLEARN:toto/tutu
    PPath:ABSOLUTE:C:/toto/tutu

    PLUS VRAI: Les PPath lus depuis un PStream donnent toujours une string absolue de la forme appropriée pour l'OS
    NOUVELLE FACON: sont conservées as is. Il faut appeler absolute() pour obtenir une forme utilisable par les appels systèmes de l'OS.

    

    PPath:ABSOLUTE:C:/toto/tutu --> C:\toto\tutu
    PPath:REMOTEUCI:toto/tutu    --> http://www.uci.org/toto/tutu
    C:\toto\tutu --> C:\toto\tutu
    toto/tutu --> C:\path_du_directory_courant\toto\tutu
    http:     --> http:
    "syntaxe fuckee a la con de getDataSet" --> "syntaxe fuckee a la con de getDataSet"

  Attention: syntaxe de getDataSet 


---

Si on est sous windows

  PPath("C:\toto")/"tutu"/"tete"   --->   "C:\toto\tutu\tete"
  PPath("PPath:REMOTEUCI:toto/")/"tutu"/"tete" ---> "PPath:REMOTEUCI:toto/tutu/tete"

  PPath("toto/tutu")+"tata"  -->   "toto/tututata"

  

  */




  class PPath: public std::string
{
public:

  PPath(const string path);

  operator string() { return *this; }
  
  PPath canonical() const;  // returns a PPath in the canonical format
  PPath absolute() const;   // returns an absolute path in the form appropriate for the OS

  operator/=(const PPath& other);
  operator/(const PPath& other) const;

  PPath up() const;
  PPath extension() const;

};




} // end of namespace PLearn

#endif
