// -*- C++ -*-

// ConjRosenbrock.h
//
// Copyright (C) 2006 Nicolas Chapados
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

// Authors: Nicolas Chapados

/*! \file ConjRosenbrock.h */


#ifndef ConjRosenbrock_INC
#define ConjRosenbrock_INC

#include <plearn/misc/PTest.h>

namespace PLearn {

/**
 *  Exercises the Conjugate Gradient optimizer through the Rosenbrock Function.
 */
class ConjRosenbrock : public PTest
{
    typedef PTest inherited;

public:
    //#####  Public Build Options  ############################################

    //! Optimizer to use, with options.
    PP<Optimizer> opt;

    //! Dimensionality of the Rosenbrock problem to solve
    int D;
    
public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    ConjRosenbrock();


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(ConjRosenbrock);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //#####  PLearn::PTest Protocol  ##########################################

    //! The method performing the test. A typical test consists in some output
    //! (to pout and / or perr), and updates of this object's options.
    virtual void perform();

protected:
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //! This does the actual building.
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ConjRosenbrock);

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
