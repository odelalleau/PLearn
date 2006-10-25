// -*- C++ -*-

// TObject.cc
//
// Copyright (C) 2005 Olivier Delalleau 
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

// Authors: Olivier Delalleau

/*! \file TObject.cc */


#include "TObject.h"

namespace PLearn {
using namespace std;

TObject::TObject() 
: object(0)
{
  allocator = new Torch::Allocator;
}

PLEARN_IMPLEMENT_OBJECT(TObject,
    "Interface between PLearn and a Torch Object",
    ""
);

void TObject::declareOptions(OptionList& ol)
{
  // declareOption(ol, "myoption", &TObject::myoption, OptionBase::buildoption,
  //               "Help text describing this option");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void TObject::build_()
{
  // Build the 'options' map.
  OptionList& options_vec = getOptionList();
  OptionBase::flag_t flags;
  for (OptionList::iterator it = options_vec.begin(); it != options_vec.end(); ++it) {
    flags = (*it)->flags();
    if (!(flags & OptionBase::nosave))
      options[(*it)->optionname()] = true;
    else
      options[(*it)->optionname()] = false;
  }

  // TODO We should not use the non saved options in updateFromTorch either.

  updateFromPLearn(0);
}

void TObject::build()
{
  inherited::build();
  build_();
}

void TObject::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("TObject::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

///////////////////
// torch_objects //
///////////////////
TObject::TObjectMap TObject::torch_objects;

//////////////////////
// updateFromPLearn //
//////////////////////
void TObject::updateFromPLearn(Torch::Object* ptr) {
  if (ptr) {
    if (object) {
      // First remove old mapping from torch_objects.
#if 0
      cout << "Erasing object " << object << endl;
      for (TObjectMap::const_iterator it = torch_objects.begin(); it != torch_objects.end(); it++) {
        string classname;
        if (it->first != object) {
          cout << "Accessing " << it->second << endl;
          classname = " (" + it->second->classname() + ")";
          cout << "Accessed!" << endl;
        }
        else
          classname = " (<-- erased)";
        cout << it->first << " -> " << it->second << classname << endl;
        cout << endl;
      }
#endif
      PLASSERT( torch_objects.find(object) != torch_objects.end() ); // Sanity check.
      torch_objects.erase(object);
    }
    object = ptr;
#if 0
      cout << "Adding object " << object << " mapped to " << this << " (" << this->classname() << ")" << endl;
      for (TObjectMap::const_iterator it = torch_objects.begin(); it != torch_objects.end(); it++) {
        cout << it->first << " -> " << it->second << flush;
        cout << " (" << it->second->classname() << ")" << endl;
      }
      cout << endl;
#endif
    PLASSERT( torch_objects.find(object) == torch_objects.end() ); // Sanity check.
    torch_objects[object] = this;
  }
  else if (!object)
    // There is no need to create a new object if there is already one.
    object = new(allocator) Torch::Object();
}

/////////////////////
// updateFromTorch //
/////////////////////
void TObject::updateFromTorch() {
  // Nothing to do.
}

//////////////
// ~TObject //
//////////////
TObject::~TObject() {
  delete allocator;
  // Remove maps to this object in 'torch_objects'. This may not be really
  // necessary, but better play it safe.
  for (TObjectMap::const_iterator it =  torch_objects.begin();
                                  it != torch_objects.end(); it++)
    if (it->second == this) {
      torch_objects.erase(it->first);
      break;
    }
}

} // end of namespace PLearn
