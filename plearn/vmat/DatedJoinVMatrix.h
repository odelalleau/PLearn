// -*- C++ -*-

// DatedJoinVMatrix.h
//
// Copyright (C) 2004 *Yoshua Bengio*
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
   * $Id: DatedJoinVMatrix.h,v 1.4 2004/03/19 18:36:57 yoshua Exp $
   ******************************************************* */

// Authors: *Yoshua Bengio*

/*! \file DatedJoinVMatrix.h */


#ifndef DatedJoinVMatrix_INC
#define DatedJoinVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;

class DatedJoinVMatrix: public RowBufferedVMatrix
{
public:
  typedef hash_multimap<Array<real>,int,hash_Array<real> > Maptype;
  //typedef hash_multimap<Array<real>,int> Maptype;

  typedef RowBufferedVMatrix inherited;

protected:
  // *********************
  // * protected options *
  // *********************

  Vec slave_row;
  Array<real> key;
  Maptype mp; // maps a key to a list of row indices in the slave

public:

  // ************************
  // * public build options *
  // ************************

  VMat master, slave;
  TVec<int> master_key_indices;
  TVec<int> slave_key_indices;
  TVec<string> master_key_names;
  TVec<string> slave_key_names;
  TVec<int> slave_field_indices;
  TVec<string> slave_field_names;
  int master_date_field_index;
  string master_date_field_name;
  int slave_date_interval_start_field_index;
  int slave_date_interval_end_field_index;
  string slave_date_interval_start_field_name;
  string slave_date_interval_end_field_name;
  int verbosity;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  DatedJoinVMatrix();

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

public:
  //!  This is the only method requiring implementation
  virtual void getRow(int i, Vec v) const;

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(DatedJoinVMatrix);

};
DECLARE_OBJECT_PTR(DatedJoinVMatrix);

} // end of namespace PLearn
#endif
