// -*- C++ -*-4 2002/09/08 21:59:21 morinf

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Pascal Vincent, Frederic Morin

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
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearn/plearn_learners/testers/DEPRECATED/TestMethod.h */

#ifndef TestMethod_INC
#define TestMethod_INC

//#include "Learner.h"
//#include "VMat.h"
#include <plearn/vmat/Splitter.h>

namespace PLearn {
using namespace std;


// **** DEPRECATED CLASS ****
// Replaced by PTester (similar concept)

class TestMethod : public Object
{
    typedef Object inherited;

private:
    void build_();

protected:
    TMat<string> requested_stats; //! a n by 4 matrix of parsed statnames 

public:

    // ** Build Options **
    PP<Splitter> splitter;
    TVec<string> statnames; //!< a list of n statistics name of the form ex: "E[E[train.class_error]]"
    bool forget_learner; //!< should we call forget on the learner prior to every train?

    // Constructor
    TestMethod() :forget_learner(true) {}

    //! Train/Tests the given learner against the given dataset
    //! And returns a Vec with the computed statistics corresponding to the requested statnames 
    //  virtual Vec test(PP<Learner> learner, const VMat &dataset);

    PLEARN_DECLARE_OBJECT(TestMethod);
    static void declareOptions(OptionList &ol);

    virtual void build();
    
}; // class TestMethod

DECLARE_OBJECT_PTR(TestMethod);


}; // end of namespace PLearn

#endif // TestMethod_INC


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
