// -*- C++ -*-

// SourceVMatrix.h
//
// Copyright (C) 2003 Pascal Vincent 
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
   * $Id: SourceVMatrix.h,v 1.9 2004/07/07 17:30:48 tihocan Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file SourceVMatrix.h */

/*
SourceVmatrix is a base class for vmatrices that are defined as a function of 
another vmatrix (the "source").

It has a source buildoption to specify said source.

*/


#ifndef SourceVMatrix_INC
#define SourceVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;

class SourceVMatrix: public RowBufferedVMatrix
{

private:

  typedef RowBufferedVMatrix inherited;

protected:

  // *********************
  // * protected options *
  // *********************

  // Fields below are not options.

  //! To be used in subclasses for convenience.
  Vec sourcerow;

public:

  // ************************
  // * public build options *
  // ************************

  VMat source;

  //  TVec<string> dependencies; // a list of paths to files that this VMat depends on

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  SourceVMatrix();  
  
  SourceVMatrix(VMat the_source)
    :source(the_source) {}

  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

  //! Sets all meta info (length_, width_, inputsize_, targetsize_, weightsize_, fieldnames, ...)
  //! that is not already set, by copying it from the source's 
  //! Modification time is also set to the latest of the current mtime of this vmat and of the mtime of the source.
  /*! This method is mostly to simplify writing subclass' build_ method,
    which may call it after first setting the fields that it doesn't want
    copied from the source.
   */
  void setMetaInfoFromSource();

  //! Must be implemented in subclasses: default version returns an error.
  virtual void getNewRow(int i, const Vec& v) const;

public:

  //! Also sets the source's meta-data dir if it's not already set
  /*! If there are fields that have no corresponding .smap .notes or .binning info files
    but the source has those files for a field with the same name, then those of
    the source will be set also for this vmatrix. */
  virtual void setMetaDataDir(const string& the_metadatadir);

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(SourceVMatrix);

};

DECLARE_OBJECT_PTR(SourceVMatrix);

} // end of namespace PLearn
#endif
