// -*- C++ -*-

// VMatKernel.h
//
// Copyright (C) 2005 Benoit Cromp 
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
   * $Id: VMatKernel.h,v 1.2 2005/05/26 21:08:49 tihocan Exp $ 
   ******************************************************* */

// Authors: Benoit Cromp

/*! \file VMatKernel.h */


#ifndef VMatKernel_INC
#define VMatKernel_INC

#include <plearn/ker/Kernel.h>
#include <plearn/vmat/VMat.h>

namespace PLearn {

class VMatKernel: public Kernel
{

private:

  typedef Kernel inherited;
  
protected:

  // *********************
  // * Protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...
    
public:

  // ************************
  // * Public build options *
  // ************************ 

  VMat source;

  // ### declare public option fields (such as build options) here
  // ...

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  // Make sure the implementation in the .cc initializes all fields to
  // reasonable default values.
  VMatKernel();

  // ******************
  // * Kernel methods *
  // ******************

private: 

  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  
  //! Declares this class' options.
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  // Declares other standard object methods.
  // If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS.
  PLEARN_DECLARE_OBJECT(VMatKernel);

  // ************************
  // **** Kernel methods ****
  // ************************

  //! Compute K(x1,x2).
  virtual real evaluate(const Vec& x1, const Vec& x2) const;

  //! Overridden for efficiency.
  virtual real evaluate_i_j(int i, int j) const;
  virtual void computeGramMatrix(Mat K) const;

  // *** SUBCLASS WRITING: ***
  // While in general not necessary, in case of particular needs 
  // (efficiency concerns for ex) you may also want to overload
  // some of the following methods:
  // virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const;
  // virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const;
  // virtual real evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x=-1, bool first_time = false) const;
  // virtual real evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x=-1, bool first_time = false) const;
  // virtual void setDataForKernelMatrix(VMat the_data);
  // virtual void addDataForKernelMatrix(const Vec& newRow);
  // virtual void setParameters(Vec paramvec);
  // virtual Vec getParameters() const;
  

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(VMatKernel);
  
} // end of namespace PLearn

#endif

