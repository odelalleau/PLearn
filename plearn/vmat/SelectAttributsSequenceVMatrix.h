// -*- C++ -*-

// SelectAttributsSequenceVMatrix.h
//
// Copyright (C) 2004 Hugo Larochelle 
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
   * $Id: SelectAttributsSequenceVMatrix.h,v 1.9 2004/10/08 20:39:44 lamblin Exp $ 
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file SelectAttributsSequenceVMatrix.h */


#ifndef SelectAttributsSequenceVMatrix_INC
#define SelectAttributsSequenceVMatrix_INC

#include <plearn/vmat/RowBufferedVMatrix.h>
#include <plearn/vmat/VMat.h>
#include <plearn/math/pl_math.h>
#include <plearn/base/ProgressBar.h>
//#include <hash_map>

namespace PLearn {
using namespace std;

//! This class creates context rows from a sequence of elements (symbol and attributs) in a VMat
class SelectAttributsSequenceVMatrix: public RowBufferedVMatrix
{

private:

  typedef RowBufferedVMatrix inherited;

  typedef hash_map<int,int> this_hash_map;

  //! Indication that the context is of fixed length
  bool fixed_context;
  //! Maximum length of a context
  int max_context_length;
  //! Number of attributs
  int n_attributs;

  // fields for usage of last row buffer
  
  //! Position in the last context of the target element (symbol and attributs)
  mutable int current_target_pos;
  //! Position in the source sequence of the first element in the last context
  mutable int lower_bound;
  //! Position in the source sequence of the last element in the last context
  mutable int upper_bound;
  //! Last context
  mutable Vec current_row_i;
  //! Mapping from the position in the source sequence to the position in the last context
  mutable this_hash_map current_context_pos;

  //! Indices of the selected contexts
  TVec<int> indices;

  //! Temporary fields
  mutable TVec<int> left_positions, right_positions;
  
  //! Temporary fields
  mutable Vec left_context, right_context, 
    row, element, target_element;

protected:

  // *********************
  // * protected options *
  // *********************

public:

  // ************************
  // * public build options *
  // ************************

  //! Number of elements at the left of the target element. 
  //! If < 0, all elements to the left are included until a delimiter is met.
  int n_left_context;
 
  //! Number of elements at the right of the target element. 
  //! If < 0, all elements to the right are included until a delimiter is met.
  int n_right_context;

  //! Offset for the position of the element on which conditions are tested (default = 0).
  int conditions_offset;

  //! Indication that the specified conditions are for the exclusion (true) or inclusion (false) of elements in the VMatrix.
  bool conditions_for_exclusion;

  //! Indication that ignored elements of context should be replaced by the next nearest valid element.
  bool full_context;

  //! Indication that the only output fields of a VMatrix row should be the output fields of the target element
  bool put_only_target_output;

  //! Indication that the last accessed context should be put in a buffer.
  bool use_last_context;

  //! Conditions to be satisfied for the exclusion or inclusion (see conditions_for_exclusion) of elements in the VMatrix.
  TVec< TVec< pair<int,int> > > conditions;
  
  //! Conditions, in string format, to be satisfied for the exclusion or inclusion (see conditions_for_exclusion) of elements in the VMatrix.
  TVec< TVec< pair<int,string> > > string_conditions;
  
  //! Delimiters of context.
  TVec< TVec< pair<int,int> > > delimiters;

  //! Delimiters, in string format, of context.
  TVec< TVec< pair<int,string> > > string_delimiters;

  //! Elements to be ignored in context.
  TVec< TVec< pair<int,int> > > ignored_context;

  //! Elements, in string format, to be ignored in context.
  TVec< TVec< pair<int,string> > > string_ignored_context;

  //! Source VMat, from which contexts are extracted.
  VMat source;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  SelectAttributsSequenceVMatrix();

  SelectAttributsSequenceVMatrix::SelectAttributsSequenceVMatrix(VMat s, int l_context,int r_context);

  // ******************
  // * Object methods *
  // ******************

private: 

  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

  void from_string_to_int_format(TVec< TVec< pair<int,string> > >& str_format, TVec< TVec< pair<int,int> > >& int_format)
    {
      int int_format_length = int_format.length();
      int_format.resize(int_format_length + str_format.length());
      for(int i=0; i<str_format.length(); i++)
        from_string_to_int_format(str_format[i], int_format[i+int_format_length]);
    }

  void from_string_to_int_format(TVec< pair<int,string> >& str_format, TVec< pair<int,int> >& int_format)
    {
      int int_format_length = int_format.length();
      int_format.resize(int_format_length+str_format.length());
      for(int i=0; i<str_format.length(); i++)
      {
        int_format[int_format_length+i].first = str_format[i].first;
        real value = source->getStringVal(str_format[i].first,str_format[i].second);
        if(is_missing(value)) PLERROR("In SelectAttributsSequenceVMatrix::from_string_to_int_format : %s not a valid string symbol for column %d", 
                                           str_format[i].second.c_str(), str_format[i].first);
        int_format[int_format_length+i].second = (int)value;
      }
    }

  bool is_true(const TVec< TVec< pair<int,int> > >& conditions, const Vec row) const
    {
      bool condition_satisfied;
      for(int i=0; i<conditions.length(); i++)
      {
        condition_satisfied = true;
        if(conditions[i].length() == 0) PLERROR("SelectAttributsSequenceVMatrix::is_true : conditions[%d] should not be empty", i);
        for(int j=0; j<conditions[i].length(); j++)
        {
          if(row[conditions[i][j].first] != conditions[i][j].second)
          {
            condition_satisfied = false;
            break;
          }
        }
        if(condition_satisfied) return true;
      }
      
      return false;
    }
  
protected: 

  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

  //! Fill the vector 'v' with the content of the i-th row.
  //! v is assumed to be the right size.
  virtual void getNewRow(int i, const Vec& v) const;

public:

  // Simply call inherited::build() then build_().
  virtual void build();

  //! Transform a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  //! returns value associated with a string (or MISSING_VALUE if there's no association for this string)
  virtual real getStringVal(int col, const string & str) const
    {
      if(source)
      {
        int src_col;
        if(col < inputsize_)
          src_col = col%source->inputsize();
        else
          src_col = source->inputsize() + (col-inputsize_)%source->targetsize();
        return source->getStringVal(src_col,str);
      }
      
      return MISSING_VALUE;
    }

  //! returns the string associated with value val 
  //! for field# col. Or returns "" if no string is associated.
  virtual string getValString(int col, real val) const
    {
      if(source)
      {
        int src_col;
        if(col < inputsize_)
          src_col = col%source->inputsize();
        else
          src_col = source->inputsize() + (col-inputsize_)%source->targetsize();
        return source->getValString(src_col,val);
      }
      
      return "";
    }

  //! Declare name and deepCopy methods.
  PLEARN_DECLARE_OBJECT(SelectAttributsSequenceVMatrix);

  virtual Vec getValues(int row, int col) const
    {
      if(row < 0 || row >= length_) PLERROR("In SelectAttributsSequenceVMatrix::getValues() : invalid row %d, length()=%d", row, length_);
      if(col < 0 || col >= length_) PLERROR("In SelectAttributsSequenceVMatrix::getValues() : invalid col %d, width()=%d", col, width_);
      if(source)
      {
        int src_col;
        if(col < inputsize_)
          src_col = col%source->inputsize();
        else
          src_col = source->inputsize() + (col-inputsize_)%source->targetsize();
        return source->getValues(indices[row],src_col);
      }
      return Vec(0);
    }

  virtual Vec getValues(Vec input, int col) const
    {
      if(col < 0 || col >= length_) PLERROR("In SelectAttributsSequenceVMatrix::getValues() : invalid col %d, width()=%d", col, width_);
      if(source)
      {
        int src_col;
        if(col < inputsize_)
          src_col = col%source->inputsize();
        else
          src_col = source->inputsize() + (col-inputsize_)%source->targetsize();
        return source->getValues(input,src_col);
      }
      return Vec(0);
    }

  virtual int getDimension(int row, int col) const
    {
      if(row < 0 || row >= length_) PLERROR("In SelectAttributsSequenceVMatrix::getDimension() : invalid row %d, length()=%d", row, length_);
      if(col < 0 || col >= length_) PLERROR("In SelectAttributsSequenceVMatrix::getDimension() : invalid col %d, width()=%d", col, width_);
      if(source)
      {
        int src_col;
        if(col < inputsize_)
          src_col = col%source->inputsize();
        else
          src_col = source->inputsize() + (col-inputsize_)%source->targetsize();
        return source->getDimension(indices[row],src_col);
      }
      return -1;
    }


};

DECLARE_OBJECT_PTR(SelectAttributsSequenceVMatrix);

} // end of namespace PLearn
#endif
