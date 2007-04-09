// -*- C++ -*-

// MeshVertex.h
//
// Copyright (C) 2004 Pascal Lamblin 
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
   * $Id: MeshVertex.h,v 1.4 2005/12/14 20:41:11 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file MeshVertex.h */


#ifndef MeshVertex_INC
#define MeshVertex_INC

#include <plearn/base/Object.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

class MeshVertex;
typedef PP< MeshVertex > MVertex;

class MeshVertex: public Object
{

private:
  
  typedef Object inherited;

public:

  // ************************
  // * public build options *
  // ************************

  int region;
  int bf;
  Vec coord;
  Vec norm;
  real error;
  Vec features;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  MeshVertex();

/*
  //! Copy constructor.
  MeshVertex( const MeshVertex& p );
*/

  //! Virtual destructor.
  virtual ~MeshVertex() {}

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
  PLEARN_DECLARE_OBJECT(MeshVertex);

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

/*
  //! operator= overloading
  MeshVertex& operator=( const MeshVertex& p );
*/

  // ******************
  // * Static methods *
  // ******************

  static int compare( const MeshVertex& p, const MeshVertex& q );

  static real dist3D( const MeshVertex& p, const MeshVertex& q );

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(MeshVertex);
  
} // end of namespace PLearn

#endif
