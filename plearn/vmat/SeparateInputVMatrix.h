// -*- C++ -*-

// SeparateInputVMatrix.h
//
// Copyright (C) 2005 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file SeparateInputVMatrix.h */


#ifndef SeparateInputVMatrix_INC
#define SeparateInputVMatrix_INC

#include <plearn/vmat/SourceVMatrix.h>

namespace PLearn {

//!  Separates the input in nsep parts and
//!  distributes them on different rows.
//!  Also copies target and weight parts
//!  for each of these rows
class SeparateInputVMatrix: public SourceVMatrix
{

private:

    typedef SourceVMatrix inherited;

protected:

    // *********************
    // * protected options *
    // *********************

public:

    // ************************
    // * public build options *
    // ************************

    //! Number of separations of the input. The input size has to be
    //! a multiple of that value.
    int nsep;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    SeparateInputVMatrix();

    //! Constructor with the source vmat and the number of
    //! separations in arguments
    SeparateInputVMatrix(VMat the_source, int the_nsep);


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

    //! Fill the vector 'v' with the content of the i-th row.
    //! v is assumed to be the right size.
    virtual void getNewRow(int i, const Vec& v) const;

public:

    // Simply calls inherited::build() then build_().
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(SeparateInputVMatrix);

};

DECLARE_OBJECT_PTR(SeparateInputVMatrix);

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
