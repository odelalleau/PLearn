// -*- C++ -*-

// TObject.h
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
   * $Id: TObject.h,v 1.3 2005/02/23 21:50:50 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file TObject.h */


#ifndef TObject_INC
#define TObject_INC

#include <plearn/base/Object.h>
#include <torch/Object.h>

// Define macros to ease the task of writing updateFromPLearn() and updateFromTorch()

#define FROM_P_BASIC(OPTION, MEMBER, TORCH_OBJECT, TORCH_MEMBER)      \
    if (options[#OPTION]) TORCH_OBJECT->TORCH_MEMBER = this->MEMBER;
    
#define FROM_T_BASIC(OPTION, MEMBER, TORCH_OBJECT, TORCH_MEMBER)      \
    if (options[#OPTION]) this->MEMBER = TORCH_OBJECT->TORCH_MEMBER;

#define FROM_P_TVEC(OPTION, TVEC, TORCH_OBJECT, TORCH_PTR, TORCH_LENGTH)  \
    if (options[#OPTION]) {                                               \
      TORCH_OBJECT->TORCH_PTR    = this->TVEC ? this->TVEC->data() : 0;   \
      TORCH_OBJECT->TORCH_LENGTH = this->TVEC.length();                   \
    }
        
#define FROM_T_TVEC(OPTION, TVEC, TORCH_OBJECT, TORCH_PTR, TORCH_LENGTH)          \
    if (options[#OPTION]) {                                                       \
      if (TORCH_OBJECT->TORCH_PTR) {                                              \
        this->TVEC.resize(TORCH_OBJECT->TORCH_LENGTH);                            \
        this->TVEC.copyFrom(TORCH_OBJECT->TORCH_PTR, TORCH_OBJECT->TORCH_LENGTH); \
      } else                                                                      \
        this->TVEC.resize(0);                                                     \
    }

#define FROM_P_OBJ(OPTION, OBJ, OBJ_PTR, OBJ_CLASS, TORCH_OBJECT, TORCH_PTR)  \
    if (options[#OPTION])                                                     \
      TORCH_OBJECT->TORCH_PTR = this->OBJ ? this->OBJ->OBJ_PTR : 0;

#define FROM_T_OBJ(OPTION, OBJ, OBJ_PTR, OBJ_CLASS, TORCH_OBJECT, TORCH_PTR)                              \
    if (options[#OPTION])  {                                                                              \
      if (TORCH_OBJECT->TORCH_PTR != (this->OBJ ? this->OBJ->OBJ_PTR : 0)) {                              \
        TObjectMap::const_iterator it = torch_objects.find(TORCH_OBJECT->TORCH_PTR);                      \
        if (it == torch_objects.end())                                                                    \
          PLERROR("In FROM_T_OBJ - Could not find the associated TObject - I currently prefer to crash"); \
        this->OBJ = (OBJ_CLASS*) it->second;                                                              \
      }                                                                                                   \
      if (this->OBJ)                                                                                      \
        this->OBJ->updateFromTorch();                                                                     \
    }


namespace PLearn {

class TObject: public Object
{

private:
  
  typedef Object inherited;

protected:

  //! Allocator for Torch memory management.
  Torch::Allocator* allocator;

  //! The underlying Torch Machine object.
  Torch::Object* object;

  //! Defines which options need to be considered in the updateFromPLearn()
  //! method (these are all the options which do not have the 'nosave' flag).
  map<string, bool> options;
  
  // *********************
  // * protected options *
  // *********************

public:

  // ************************
  // * public build options *
  // ************************

  // ### declare public option fields (such as build options) here
  // ...

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  TObject();

  // ******************
  // * Object methods *
  // ******************

private: 

  //! This does the actual building. 
  void build_();

protected: 

  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:

  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(TObject);

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  //! Update the underlying Torch object from this object's options.
  virtual void updateFromPLearn(Torch::Object* ptr);

  //! Update this object's options from the underlying Torch object.
  virtual void updateFromTorch();

  //! Destructor to free allocated memory.
  virtual ~TObject() {delete allocator;}

  // ******************
  // * Static members *
  // ******************

public:

  typedef map<const Torch::Object*, PLearn::TObject*> TObjectMap;

  //! Mapping between a Torch Object and its corresponding PLearn TObject.
  static TObjectMap torch_objects;

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(TObject);
  
} // end of namespace PLearn

#endif
