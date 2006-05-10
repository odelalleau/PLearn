// -*- C++ -*-

// ConstantVMatrix.h
//
// Copyright (C) 2005 Nicolas Chapados
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
 * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $
 ******************************************************* */

// Authors: Nicolas Chapados

/*! \file ConstantVMatrix.h */


#ifndef ConstantVMatrix_INC
#define ConstantVMatrix_INC

#include <plearn/vmat/VMat.h>


namespace PLearn {

/**
 *  This VMatrix returns a constant element (specified upon construction)
 */
class ConstantVMatrix: public VMatrix
{
    typedef VMatrix inherited;

public:
    //#####  Public Options  ##################################################

    //! The constant output to return
    real constant_output;


    //#####  Object Methods  ##################################################

    //! Default constructor (default return value is missing value)
    ConstantVMatrix();

    //! Initialize with a given size
    ConstantVMatrix(int the_length, int the_width,
                    real constant_output = MISSING_VALUE);

private:

    //! This does the actual building.
    void build_();

protected:

    //! Declares this class' options.
    static void declareOptions(OptionList& ol);

public:

    //! Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods
    //  If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(ConstantVMatrix);


    //#####  VMatrix Methods  #################################################

    //! Get element at i-th row, j-th column
    //  (Please implement in .cc)
    virtual real get(int i, int j) const;

    //! Get part or all of the i-th, starting at the j-th column,
    //! with v.length() elements; these elements are put in v.
    //  (Please implement in .cc)
    //  (default version repeatedly calls get(i,j) which may have a significant overhead)
    virtual void getSubRow(int i, int j, Vec v) const;

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ConstantVMatrix);

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
