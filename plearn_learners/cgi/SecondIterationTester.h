// -*- C++ -*-

// SecondIterationTester.h
// 
// Copyright (C) 2002 Pascal Vincent, Frederic Morin
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
 * $Id: SecondIterationTester.h 5587 2006-05-12 16:31:54Z plearner $ 
 ******************************************************* */

/*! \file SecondIterationTester.h */
#ifndef SecondIterationTester_INC
#define SecondIterationTester_INC

#include <plearn/vmat/ExplicitSplitter.h>
#include <plearn_learners/testers/PTester.h>

namespace PLearn {
using namespace std;

class SecondIterationTester: public PTester
{    

private:

    typedef PTester inherited;

public:

    // ************************
    // * public build options *
    // ************************
  
    // See declareOptions method in .cc for the role of these options.

    // ****************
    // * Constructors *
    // ****************

    // Default constructor
    SecondIterationTester();


    // ******************
    // * Object methods *
    // ******************

private: 
    //! This does the actual building. 
    // (Please implement in .cc)
    void build_();

protected: 
    //! Declares this class' options
    static void declareOptions(OptionList& ol);

public:
    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(SecondIterationTester);
    
  
    //! This returns the currently set expdir (see setExperimentDirectory)
    void setSplitter(string splitter_template);

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

DECLARE_OBJECT_PTR(SecondIterationTester);

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
