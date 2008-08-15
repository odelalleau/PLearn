// -*- C++ -*-

// StochasticBinarizeVMatrix.h
//
// Copyright (C) 2008 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file StochasticBinarizeVMatrix.h */


#ifndef StochasticBinarizeVMatrix_INC
#define StochasticBinarizeVMatrix_INC

#include <plearn/math/PRandom.h>
#include <plearn/vmat/SourceVMatrix.h>

namespace PLearn {

/**
 * The first sentence should be a BRIEF DESCRIPTION of what the class does.
 * Place the rest of the class programmer documentation here.  Doxygen supports
 * Javadoc-style comments.  See http://www.doxygen.org/manual.html
 *
 * @todo Write class to-do's here if there are any.
 *
 * @deprecated Write deprecated stuff here if there is any.  Indicate what else
 * should be used instead.
 */
class StochasticBinarizeVMatrix : public SourceVMatrix
{
    typedef SourceVMatrix inherited;

public:
    //#####  Public Build Options  ############################################

    bool rescale_to_0_1;
    int seed;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    StochasticBinarizeVMatrix();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(StochasticBinarizeVMatrix);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    PP<PRandom> random_gen;
    VMat rescaled_data;

    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    // ...

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Fill the vector 'v' with the content of the i-th row.
    //! v is assumed to be the right size.
    virtual void getNewRow(int i, const Vec& v) const;

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(StochasticBinarizeVMatrix);

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
