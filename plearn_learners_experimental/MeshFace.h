// -*- C++ -*-

// MeshFace.h
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
   * $Id: MeshFace.h,v 1.5 2004/11/17 01:42:02 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file MeshFace.h */


#ifndef MeshFace_INC
#define MeshFace_INC

#ifdef TEXTURE
#warn( "I didn't implement TEXTURE stuff" )
#endif

#include <plearn/base/Object.h>

#include <boost/config.hpp>
#include <boost/utility.hpp>             // for boost::tie
#include <boost/graph/graph_traits.hpp>  // for boost::graph_traits
#include <boost/graph/adjacency_list.hpp>


namespace PLearn {
using namespace std;
using namespace boost;

class MeshFace;
typedef PP< MeshFace > MFace;

class MeshFace: public Object
{

private:
  
  typedef Object inherited;

public:

  // ************************
  // * public build options *
  // ************************

//  int id;

  // All these TVec are supposed to have a length of 3.
  typedef adjacency_list_traits< listS, listS, undirectedS > g_traits;
  typedef g_traits::vertex_descriptor vertex_descriptor;
  typedef g_traits::edge_descriptor edge_descriptor;
  TVec< vertex_descriptor > pts;
  TVec< edge_descriptor > edges;
  TVec<int> adj_faces;

  Vec face_norm;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  MeshFace();

/*
  //! Copy constructor.
  MeshFace( const MeshFace& f );
*/

  //! Virtual destructor
  virtual ~MeshFace() {}


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
  PLEARN_DECLARE_OBJECT(MeshFace);

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  // (PLEASE IMPLEMENT IN .cc)
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

/*
  MeshFace& operator=( const MeshFace& f );
*/

  // ******************
  // * Static methods *
  // ******************

//  static int compare( const MeshFace& e, const MeshFace& f );
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(MeshFace);

} // end of namespace PLearn

#endif
