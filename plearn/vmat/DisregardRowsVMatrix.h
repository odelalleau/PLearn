// -*- C++ -*-

// DisregardRowsVMatrix.h
//
// Copyright (C) 2005 Christian Dorion 
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
 * $Id$ 
 ******************************************************* */

// Authors: Christian Dorion

/*! \file DisregardRowsVMatrix.h */


#ifndef DisregardRowsVMatrix_INC
#define DisregardRowsVMatrix_INC

#include <plearn/vmat/SelectRowsVMatrix.h>

namespace PLearn {

/*!
  A vmat that disregard rows containing any specified values.

  Typically, this vmat is used to select the rows of the source vmat which
  do not contain missing values. However, this behaviour can be changed by
  setting the 'disregard_missings' flag to false and by providing any
  real value list through 'disregard_values'.

  The default behavior of the class is to inspect all columns of the
  underlying vmat, but one may specify a subset of the source's fieldnames
  to restrict the inspection.
*/
class DisregardRowsVMatrix: public SelectRowsVMatrix
{
private:
    typedef SelectRowsVMatrix inherited;

    //! This does the actual building. 
    void build_();

protected:

    //! Stores the inspected_fieldnames column indices.
    TVec<int> _inspected_columns;

    //! Declares this class' options
    static void declareOptions(OptionList& ol);
  
    //! Fills the inherited::indices vector.
    virtual void inferIndices( );


public:
    // 
    //  public build options 
    // 
  
    /*!
      Field names of the source vmat for which a triggering value (see the
      disregard_values option) cause this vmat to neglect a row.

      If empty, all source's fieldnames are used.

      Default: [].
    */
    TVec<string> _inspected_fieldnames;

    /*!
      If any of these values is encountered in any column designated in
      inspected_fieldnames, the whole row is disregarded.

      Default: [ ]
    */
    Vec _disregard_values;
  
    /*!
      Should missing values cause a row to be neglected.

      Default: 1 (True)
    */
    bool _disregard_missings;

    /*!
      If positive, only the last 'maximum_length' rows kept from the source
      vmat will be considered, all other rows being disregarded.

      Default: -1. 
    */
    int _maximum_length;
  
public:
  
    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    DisregardRowsVMatrix();

    //! Alternative constructor that takes a VMat and immediately finds
    //! missing values
    DisregardRowsVMatrix(VMat source);
    
    // Simply calls inherited::build() then build_().
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(DisregardRowsVMatrix);

};

DECLARE_OBJECT_PTR(DisregardRowsVMatrix);

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
