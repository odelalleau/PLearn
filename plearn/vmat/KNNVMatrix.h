// -*- C++ -*-

// KNNVMatrix.h
//
// Copyright (C) 2004 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file KNNVMatrix.h */


#ifndef KNNVMatrix_INC
#define KNNVMatrix_INC

#include <plearn/ker/Kernel.h>
#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;

class KNNVMatrix: public SourceVMatrix
{

private:

    typedef SourceVMatrix inherited;

    //! Used to store a row of the source VMatrix.
    mutable Vec source_row;

protected:
    // *********************
    // * protected options *
    // *********************

    // Fields below are not options.

    //! Store the nearest neighbours of each point.
    Mat nn;

    //! Store the pij weights, if a kernel_pij is provided.
    Mat pij;

public:

    // ************************
    // * public build options *
    // ************************

    VMat k_nn_mat;
    Ker kernel_pij;
    int knn;
    bool report_progress;

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    KNNVMatrix();

    // ******************
    // * VMatrix methods *
    // ******************

private:

    //! This does the actual building.
    void build_();

protected:

    //! Declares this class' options
    static void declareOptions(OptionList& ol);

    //! Return the index in the source matrix of the sample number i in
    //! this matrix. Also return in i_n the neighbour rank, and in i_ref
    //! the reference point.
    inline int getSourceIndexOf(int i, int& i_ref, int& i_n) const;

    //! Return the tag of the sample number p in a bag:
    //!   p == 0      => 1
    //!   p == knn-1  => 2
    //!   otherwise   => 0
    //! (If knn == 1, always return 3).
    inline int getTag(int p) const;

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods
    //  If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(KNNVMatrix);


    // **************************
    // **** VMatrix methods ****
    // **************************

protected:

    //! Needed because it's a SourceVMatrix.
    virtual void getNewRow(int i, const Vec& v) const;

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(KNNVMatrix);

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
