// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: SourceVariable.h,v 1.3 2003/08/13 08:13:17 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef SourceVariable_INC
#define SourceVariable_INC

#include "VarArray.h"

namespace PLearn <%
using namespace std;

class SourceVariable: public Variable
{
protected:
  //!  Default constructor for persistence
  SourceVariable() {}

public:
  SourceVariable(int thelength, int thewidth);
  SourceVariable(const Vec& v, bool vertical=true);
  SourceVariable(const Mat& m);


  virtual void setParents(const VarArray& parents)
  { PLERROR("In Variable::setParents  trying to set parents of a SourceVariable..."); }


  virtual bool markPath();
  virtual void buildPath(VarArray& proppath);
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  PLEARN_DECLARE_OBJECT(SourceVariable);
  
  
  virtual void fprop();
  virtual void bprop();
  virtual void bbprop();
  virtual void symbolicBprop();
  virtual void rfprop();
  virtual VarArray sources();
  virtual VarArray random_sources();
  virtual VarArray ancestors();
  virtual void unmarkAncestors();
  virtual VarArray parents();
  bool isConstant() { return true; }

  void printInfo(bool print_gradient) { 
    cout << getName() << "[" << (void*)this << "] " << *this << " = " << value;
    if (print_gradient) cout << " gradient=" << gradient;
    cout << endl; 
  }
};


%> // end of namespace PLearn

#endif

