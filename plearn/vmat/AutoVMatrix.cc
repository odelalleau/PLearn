// -*- C++ -*-

// AutoVMatrix.cc
// Copyright (C) 2002 Pascal Vincent
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
 * $Id: AutoVMatrix.cc,v 1.15 2005/02/03 17:13:41 ducharme Exp $
 * This file is part of the PLearn library.
 ******************************************************* */


#include "AutoVMatrix.h"
#include "MemoryVMatrix.h"
#include <plearn/db/getDataSet.h>
#include <plearn/db/databases.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(AutoVMatrix,
    "Automatically builds an appropriate VMat given its specification.",
    "AutoVMatrix tries to interpret the given 'specification' (it will call getDataSet) and\n"
    "will be a wrapper around the appropriate VMatrix type, simply forwarding calls to it.\n"
    "AutoVMatrix can be used to access the UCI databases.\n");

AutoVMatrix::AutoVMatrix(const PPath& the_specification, bool load_in_memory)
  :specification(the_specification), load_data_in_memory(load_in_memory)
{ build_(); }

void AutoVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "specification", &AutoVMatrix::specification, OptionBase::buildoption,
                "This is any string understood by getDataSet. Typically a file or directory path.\n"
                 + loadUCIDatasetsHelp());
  declareOption(ol, "load_in_memory", &AutoVMatrix::load_data_in_memory, OptionBase::buildoption,
                "Boolean value to specify if we want to store data in memory (in a MemoryVMatrix)."
                "Default=false.\n");
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);

  // Hide the 'vm' option, that is overwritten at build time anyway.
  redeclareOption(ol, "vm", &AutoVMatrix::vm, OptionBase::nosave, "");
}

void AutoVMatrix::build_()
{
  if(specification=="")
    setVMat(VMat());
  else if (load_data_in_memory)
  {
    VMat data = getDataSet(specification);
    VMat memdata = new MemoryVMatrix(data);
    setVMat(memdata);
  }
  else
    setVMat(getDataSet(specification));
}

void AutoVMatrix::build()
{
  inherited::build();
  build_();
}
 
/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void AutoVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies) {
  inherited::makeDeepCopyFromShallowCopy(copies);
}

} 

