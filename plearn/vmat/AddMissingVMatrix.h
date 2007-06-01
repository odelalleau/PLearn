// -*- C++ -*-

// AddMissingVMatrix.h
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
   * $Id: AddMissingVMatrix.h,v 1.1 2005/07/18 19:04:18 delallea Exp $
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file AddMissingVMatrix.h */


#ifndef AddMissingVMatrix_INC
#define AddMissingVMatrix_INC

#include <plearn/math/PRandom.h>
#include <plearn/vmat/SourceVMatrix.h>

namespace PLearn {

class AddMissingVMatrix: public SourceVMatrix
{

private:

  typedef SourceVMatrix inherited;

protected:

  PP<PRandom> random_gen;

  // *********************
  // * protected options *
  // *********************

public:

  // ************************
  // * public build options *
  // ************************

  bool add_missing_target;
  real missing_prop;
  int  only_on_first;
  long seed;
  //! Columns which will be filled with missing values
  TVec<int> missing_values_columns;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  AddMissingVMatrix();

  // ******************
  // * Object methods *
  // ******************

private:

  //! This does the actual building.
  void build_();

protected:

  //! Declares this class' options
  static void declareOptions(OptionList& ol);

  //! Fill the vector 'v' with the content of the i-th row.
  //! v is assumed to be the right size.
  virtual void getNewRow(int i, const Vec& v) const;

public:

  // Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  //! Return the Dictionary object for a certain field, or a null pointer
  //! if there isn't one
  virtual PP<Dictionary> getDictionary(int col) const;

  //! Gives the possible values for a certain field in the VMatrix
  virtual void getValues(int row, int col, Vec& values) const;

  //! Gives the possible values of a certain field (column) given the input 
  virtual void getValues(const Vec& input, int col, Vec& values) const;

  virtual string getValString(int col, real val) const;
  virtual real getStringVal(int col, const string & str) const;

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(AddMissingVMatrix);

};

DECLARE_OBJECT_PTR(AddMissingVMatrix);

inline VMat add_missing(VMat source, const TVec<int>& missing_values_columns)
  {
    AddMissingVMatrix* ret = new AddMissingVMatrix();
    ret->source = source;
    ret->missing_values_columns.resize(missing_values_columns.length());
    ret->missing_values_columns << missing_values_columns;
    ret->build();
    return ret;
  }

} // end of namespace PLearn

#endif

/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
