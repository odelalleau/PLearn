// -*- C++ -*-

// FilePath.h
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
   * $Id: FilePath.h,v 1.1 2005/01/06 02:09:37 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent, Christian Dorion, Nicolas Chapados

/*! \file FilePath.h */


#ifndef FilePath_INC
#define FilePath_INC

// Put includes here
#include <mozilla/nspr/prenv.h>
#include <plearn/io/PStream.h>

namespace PLearn {


  /*
Canonique:

./           (directory courant du process)
${DIRPATH}/  (directory du fichier courant)

${HOME}/  (directory de l'utilisateur)

Un nombre d'autres qui seraient pris de l'environnement de l'utilisateur (voir PR_GetEnv)
Ex:
${PLEARNDIR}/

${PLEARNDATASETS}/

  */

class FilePath
{
protected:
  string abspath;  // internal absolute representation

public:

  //! peut recevoir un path canonique ou non
  //! avec des slash ou des backslash, peu importe.
  FilePath(const string& path, const string& dirpath="");

  //! retourne un path utilisable dans fopen
  operator char*();
  operator string();
  
  // gere les / \ \ ...
  /*
   toto + "ab" -> totoab
   toto + "/ab" -> toto/ab
   toto + "/ab\\cd/e////f" -> /ab/

  */
  operator+=(const string& append);
  operator+ ...

  operator/=
  operator/ 

  // retourne version canonique
  // (attention, ne PAS y mettre de ${DIRPATH} )
  // meme format peu importe la platforme
  // pas de trailing slash
  string canonical() const;
  
  // retourne path absolu
  // platform dependent
  string absolute() const;

};




} // end of namespace PLearn

#endif
