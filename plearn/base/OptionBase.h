// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2002 Frederic Morin

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
   * $Id: OptionBase.h,v 1.1 2003/05/07 05:39:16 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/Option.h */

#ifndef OptionBase_INC
#define OptionBase_INC

#include "PP.h"
#include "pl_io.h"
#include <vector>

namespace PLearn <%
using namespace std;

// Predeclaration of Object
class Object;

//! Base class for option definitions
class OptionBase: public PPointable
{
public:
  typedef OBflag_t flag_t; // To change type of flag_t, change type of OBflag_t in pl_io.h instead

  //! 'buildoption' an option typically specified before calling the initial build 
  //! (semantically similat to a constructor parameter) ex: the number of hidden units in a neural net
  static const flag_t buildoption;       

  //! 'learntoption' a field whose proper value is computed by the class after construction
  //! (not to be set by the user before build) ex: the weights of a neural net
  static const flag_t learntoption;

  //! 'tuningoption' an option typically set after the initial build, to tune the object
  static const flag_t tuningoption;

  //! Do not include this option in the objet's serialisation (write method skips it)
  static const flag_t nosave; 

protected:
  string optionname_;  // the name of the option
  flag_t flags_; 
  string optiontype_;  // the datatype of the option ("int" ...)
  string defaultval_;  // string representation of the default value (will be printed by optionHelp())
  string description_; // A description of this option

public:

  //! Most of these parameters only serve to provide the user 
  //! with an informative help text. (only optionname and saveit are really important)
  OptionBase(const string& optionname, flag_t flags,
             const string& optiontype, const string& defaultval, 
             const string& description)
    :optionname_(optionname), flags_(flags), 
    optiontype_(optiontype), defaultval_(defaultval),
    description_(description) {}

  virtual void read(Object* o, PStream& in) const = 0;
  virtual void read_and_discard(PStream& in) const = 0;
  virtual void write(const Object* o, PStream& out) const = 0;

  // writes the option into a string instead of a stream
  // (calls write on a string stream) 
  string writeIntoString(const Object* o) const;
  
  virtual Object* getAsObject(Object* o) const = 0;
  virtual const Object* getAsObject(const Object* o) const = 0;
  virtual Object *getIndexedObject(Object *o, int i) const = 0;
  virtual const Object *getIndexedObject(const Object *o, int i) const = 0;    
  
  //! Returns the name of the class in to which this field belongs
  virtual string optionHolderClassName(const Object* o) const = 0;

  //! The name of the option (field)
  inline const string& optionname() const { return optionname_; }
  inline bool isOptionNamed(string name) const { return name == optionname(); }
  
  inline const string& optiontype() const { return optiontype_; }
  inline const string& defaultval() const { return defaultval_; }
  inline const string& description() const { return description_; }
  inline flag_t flags() const { return flags_; }

  //! change the string representation of the default value
  inline void setDefaultVal(const string& newdefaultval)
  { defaultval_ = newdefaultval; }
};

typedef vector< PP<OptionBase> > OptionList;

%> // end of namespace PLearn

#endif //!<  Option_INC
