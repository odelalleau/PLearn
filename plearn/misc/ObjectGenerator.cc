// -*- C++ -*-

// ObjectGenerator.cc
// Copyright (C) 2004 Rejean Ducharme
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
   * $Id: ObjectGenerator.cc,v 1.1 2004/06/17 20:41:21 ducharme Exp $
   ******************************************************* */

#include "ObjectGenerator.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(ObjectGenerator, "ONE LINE DESCR", "NO HELP");

ObjectGenerator::ObjectGenerator() : generation_began(false)
{}

string ObjectGenerator::help()
{
  // ### Provide some useful description of what the class is ...
  return
    "ObjectGenerator is the base class for implementing object-generation techniques. \n"
    "The OptionGenerator take a template Object, and from a list of options, \n"
    "it will generate another Object (or a complete list). \n";
}

void ObjectGenerator::build_()
{
  if (template_object.isNull())
    PLERROR("An ObjectGenerator MUST contain a template Object");
}

void ObjectGenerator::build()
{
  inherited::build();
  build_();
}

void ObjectGenerator::forget()
{
  generation_began = false;
}

void ObjectGenerator::declareOptions(OptionList& ol)
{
   declareOption(ol, "template_object", &ObjectGenerator::template_object,
       OptionBase::buildoption, "The template Object from which all the others will be built. \n");

  inherited::declareOptions(ol);
}

TVec< PP<Object> > ObjectGenerator::generateAllObjects()
{
  TVec< PP<Object> > all_objs;

  forget();
  while (!lastObject())
  {
    PP<Object> next_obj = generateNextObject();
    all_objs.append(next_obj);
  }
  generation_began = true;

  return all_objs;
}

} // end of namespace PLearn

