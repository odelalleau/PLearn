// -*- C++ -*-

// AppendNeighborsVMatrix.h
//
// Copyright (C) 2004 Martin Monperrus 
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
 * $Id: AppendNeighborsVMatrix.h 3994 2005-08-25 13:35:03Z chapados $ 
 ******************************************************* */

// Authors: Martin Monperrus & Yoshua Bengio

/*! \file AppendNeighborsVMatrix.h */


#ifndef AppendNeighborsVMatrix_INC
#define AppendNeighborsVMatrix_INC

#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;

/*! * AppendNeighborsVMatrix * */

/**
 * Appends the nearest neighbors of the input samples of a source VMatrix.
 */

class AppendNeighborsVMatrix: public SourceVMatrix
{

private:

    typedef SourceVMatrix inherited;

    //! Used to store data and save memory allocations.
    mutable Vec input, target;
    mutable real weight;

protected:
    // NON-OPTION FIELDS

    //! Matrix of nearest neighbor indices
    TMat<int> input_parts; 

    // *********************
    // * protected options *
    // *********************

public:

    // ************************
    // * public build options *
    // ************************

    //! Number of nearest neighbors
    int n_neighbors;

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    AppendNeighborsVMatrix();

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

    //!  This is the only method requiring implementation
    virtual void getNewRow(int i, const Vec& v) const;

public:
    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(AppendNeighborsVMatrix);

};
DECLARE_OBJECT_PTR(AppendNeighborsVMatrix);

inline VMat append_neighbors(VMat source, int n_neighbors)
{
    AppendNeighborsVMatrix* vmat = new AppendNeighborsVMatrix();
    vmat->source=source;
    vmat->n_neighbors=n_neighbors;
    vmat->build();
    return vmat;
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
