// AutoVMatrix.h
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
   * $Id: AutoVMatrix.h,v 1.8 2004/07/26 20:09:24 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef AutoVMatrix_INC
#define AutoVMatrix_INC

#include "ForwardVMatrix.h"

namespace PLearn {
using namespace std;

//! This class is a simple wrapper to an underlying VMatrix of another type
//! All it does is forward the method calls

class AutoVMatrix: public ForwardVMatrix
{

private:

  typedef ForwardVMatrix inherited;

public:

  PLEARN_DECLARE_OBJECT(AutoVMatrix);

  // ************************
  // * public build options *
  // ************************

  string specification; // the specification of the vmat (typically a file or directory path)

  // ****************
  // * Constructors *
  // ****************

  AutoVMatrix(const string& the_specification="");

private: 

  //! This does the actual building. 
  void build_();

protected:

  //! Declares this class' options
  static void declareOptions(OptionList& ol);

public:

  // Simply calls inherited::build() then build_()
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(AutoVMatrix);

} // end of namespace PLearn

#endif

