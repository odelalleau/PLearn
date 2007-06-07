// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
// Copyright (C) 2003 Olivier Delalleau
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


/* ******************************************************************      
   * $Id: MissingIndicatorVMatrix.h 3658 2005-07-06 20:30:15  Godbout $
   ****************************************************************** */

/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef MissingIndicatorVMatrix_INC
#define MissingIndicatorVMatrix_INC

#include <plearn/vmat/SourceVMatrix.h>
#include <plearn/math/BottomNI.h>

namespace PLearn {
using namespace std;

class MissingIndicatorVMatrix: public VMatrix
{
  typedef VMatrix inherited;
  
public:

  //! The source VMatrix with missing values.
  VMat         source;
  
  //! A referenced train set.
  //! A missing indicator is added for variables with missing values in this data set.
  //! It is used in combination with the option number_of_train_samples_to_use.
  VMat         train_set;
  
  //! The number of samples from the train set that will be examined to see
  //! if an indicator should be added for each variable.
  real         number_of_train_samples_to_use;
  

                        MissingIndicatorVMatrix();
                        MissingIndicatorVMatrix(VMat the_source, VMat the_train_set, real the_number_of_train_samples_to_use);
  virtual               ~MissingIndicatorVMatrix();

  static void           declareOptions(OptionList &ol);

  virtual void          build();
  virtual void          makeDeepCopyFromShallowCopy(CopiesMap& copies);

  virtual void         getExample(int i, Vec& input, Vec& target, real& weight);
  virtual real         get(int i, int j) const;
  virtual void         put(int i, int j, real value);
  virtual void         getSubRow(int i, int j, Vec v) const;
  virtual void         putSubRow(int i, int j, Vec v);
  virtual void         appendRow(Vec v);
  virtual void         insertRow(int i, Vec v);  
  virtual void         getRow(int i, Vec v) const;
  virtual void         putRow(int i, Vec v);
  virtual void         getColumn(int i, Vec v) const;

private:
  
  int          train_length;
  int          train_width;
  int          train_inputsize;
  int          train_targetsize;
  int          train_weightsize;
  int          train_row;
  int          train_col;
  Vec          train_input;
  TVec<string> train_field_names;
  TVec<int>    train_var_missing;
  int          source_length;
  int          source_width;
  int          source_inputsize;
  int          source_targetsize;
  int          source_weightsize;
  Vec          source_input;
  TVec<int>    source_rel_pos;
  int          new_width;
  int          new_inputsize;
  int          new_col;
  TVec<string> new_field_names;

          void         build_();
          void         buildNewRecordFormat();
  
  PLEARN_DECLARE_OBJECT(MissingIndicatorVMatrix);

};

DECLARE_OBJECT_PTR(MissingIndicatorVMatrix);

} // end of namespcae PLearn
#endif
