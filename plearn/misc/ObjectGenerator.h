// -*- C++ -*-

// ObjectGenerator.h
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
   * $Id: ObjectGenerator.h,v 1.1 2004/06/17 20:41:21 ducharme Exp $
   ******************************************************* */

#include "Object.h"
#include "TVec.h"

#ifndef ObjectGenerator_INC
#define ObjectGenerator_INC

namespace PLearn {
using namespace std;

class ObjectGenerator: public Object
{
private:
  typedef Object inherited;

protected:

  //! Did we begin to generate new Objects???
  bool generation_began;

public:

  //! The template Object from which we will generate other Objects
  PP<Object> template_object;


 // ******************
 // * Object methods *
 // ******************

private:
  void build_();

protected:
  static void declareOptions(OptionList& ol);

  //! Does the last generated Object is the last of the list
  //! MUST be define by a subclass
  virtual bool lastObject() const =0;

public:

  //! Default constructor
  ObjectGenerator();

  //! This will generate the next object in the list of all options
  //! MUST be define by a subclass
  virtual PP<Object> generateNextObject() =0;

  //! This will generate a list of all possible Objects.
  //! By default, just loop over generateNextObject()
  virtual TVec< PP<Object> > generateAllObjects();

  //! simply calls inherited::build() then build_()
  virtual void build();

  virtual void forget();

  //! Provides a help message describing this class
  static string help();

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_ABSTRACT_OBJECT(ObjectGenerator);
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ObjectGenerator);

} // end of namespace PLearn

#endif // ObjectGenerator_INC
