// -*- C++ -*-

// PLearnDiff.h
//
// Copyright (C) 2005 Olivier Delalleau 
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

// Authors: Olivier Delalleau

/*! \file PLearnDiff.h */


#ifndef PLearnDiff_INC
#define PLearnDiff_INC

#include <plearn/base/Object.h>

namespace PLearn {

//! Forward declaration.
template<class T> class TMat;

class PLearnDiff: public Object
{

private:
  
    typedef Object inherited;

protected:

    // *********************
    // * protected options *
    // *********************

    TMat<string> diffs;

public:

    // ************************
    // * public build options *
    // ************************

    real absolute_tolerance;
    real relative_tolerance;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    PLearnDiff();

    // ******************
    // * Object methods *
    // ******************

private: 
    //! This does the actual building. 
    void build_();

protected: 
    //! Declares this class' options.
    static void declareOptions(OptionList& ol);

public:
    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(PLearnDiff);

    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! If 'refer' != 'other' return 1 and add the difference 'name' with the
    //! corresponding reference and other values.
    //! Otherwise, do nothing and just return 0.
    int diff(const string& refer, const string& other, const string& name);

    //! Empty the 'diffs' matrix.
    void forget();

    //! Add 'prefix' in front of the last 'n' difference names in 'diffs'.
    void addDiffPrefix(const string& prefix, int n);

    //! Display the differences that were found.
    //! 'indent' specifies the number of whitespaces to add before each line.
    //! 'tab_step' specifies the canonical length of each element (name and
    //! values for each difference), in order to obtain a fancy output.
    void printDiffs(PStream& out = pout, unsigned int indent = 2,
                                         unsigned int tab_step = 20);

    //! Return the number of differences found.
    int nDiffs() { return diffs.length(); }

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PLearnDiff);

//! Just call diffs->addDiffPrefix(prefix, n).
//! This function is used so that it can be forward-declared.
void addDiffPrefix(PLearnDiff* diffs, const string& prefix, int n);
  
//! Just call diffs->diff(refer, other, name);
//! This function is used so that it can be forward-declared.
int diff(PLearnDiff* diffs, const string& refer, const string& other, const string& name);

//! Return the absolute tolerance of a PLearnDiff.
real get_absolute_tolerance(PLearnDiff* diffs);

//! Return the relative tolerance of a PLearnDiff.
real get_relative_tolerance(PLearnDiff* diffs);

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
