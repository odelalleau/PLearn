// -*- C++ -*-

// plearnmain.h
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux, Rejean Ducharme
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
   * $Id: plearnmain.h,v 1.1 2002/09/05 07:56:30 plearner Exp $
   ******************************************************* */

#include "general.h"
#include "TypeFactory.h"

namespace PLearn <%
using namespace std;

template<class T>
void displayRegisteredSubClassesOf(const string& baseclassname, ostream& out)
{
  out << "******************************************* " << endl;
  out << "**  Registered subclasses of " << baseclassname << " ** " << endl;
  out << "******************************************* " << endl;
  const TypeMap& tmap = TypeFactory::instance().getTypeMap();
  TypeMap::const_iterator it = tmap.begin();
  TypeMap::const_iterator itend = tmap.end();
  while(it!=itend)
    {
      Object* o = (*(it->second))();
      if(dynamic_cast<T*>(o))
        out << it->first << endl;
      if(o)
        delete o;
      ++it;
    }
  out << "-------------------------------------" << endl;
}

//! reads a modelalias -> object_representation map from a model.aliases file
map<string, string> getModelAliases(const string& filename);
vector<string> getMultipleModelAliases(const string& model);
void displayObjectHelp(ostream& out, const string& classname, bool fulloptions=false);
int plearnmain(int argc, char** argv);

%> // end of namespace PLearn




