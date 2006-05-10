// -*- C++ -*-

// StackedSplitter.h
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

/*! \file StackedSplitter.h */


#ifndef StackedSplitter_INC
#define StackedSplitter_INC

#include "Splitter.h"

namespace PLearn {
using namespace std;

class StackedSplitter: public Splitter
{

private:

    typedef Splitter inherited;

protected:
    // *********************
    // * protected options *
    // *********************

    // Fields below are not options.

    //! The last split asked to the initial splitter.
    int last_k_init;

    //! The last split returned by the initial splitter.
    TVec<VMat> last_split_init;

public:

    // ************************
    // * public build options *
    // ************************

    PP<Splitter> initial_splitter;
    TVec< PP<Splitter> > top_splitters;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    StackedSplitter();


    // ******************
    // * Object methods *
    // ******************

private:
    //! This does the actual building.
    void build_();

protected:
    //! Declares this class' options
    static void declareOptions(OptionList& ol);

public:
    // simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(StackedSplitter);


    // ********************************
    // *        Splitter methods      *
    // * (must be implemented in .cc) *
    // ********************************

    //! Overridden to forward to the initial splitter.
    virtual void setDataSet(VMat the_dataset);

    //! Returns the number of available different "splits"
    virtual int nsplits() const;

    //! Returns the number of sets per split
    virtual int nSetsPerSplit() const;

    //! Returns split number i
    virtual TVec<VMat> getSplit(int i=0);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(StackedSplitter);

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
