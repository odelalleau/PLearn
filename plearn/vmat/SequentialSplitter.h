// -*- C++ -*-

// SequentialSplitter.h
// 
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2002 Frederic Morin
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
   * $Id: SequentialSplitter.h,v 1.4 2003/08/13 08:13:46 plearner Exp $ 
   ******************************************************* */

/*! \file SequentialSplitter.h */
#ifndef SequentialSplitter_INC
#define SequentialSplitter_INC

#include "Splitter.h"

namespace PLearn <%
using namespace std;

class SequentialSplitter: public Splitter
{
protected:
    // *********************
    // * protected options *
    // *********************

    // ### declare protected option fields (such as learnt parameters) here
    // ...
    
public:

    typedef Splitter inherited;

    // ************************
    // * public build options *
    // ************************

    int train_step;
    int min_train;

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    SequentialSplitter(int train_step_ = 0, int min_train_ = 0);

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

public:
    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Provides a help message describing this class
    static string help();

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(SequentialSplitter);


    // ********************************
    // *        Splitter methods      *
    // * (must be implemented in .cc) *
    // ********************************

    //! Returns the number of available different "splits"
    virtual int nsplits() const;

  //! Returns the number of sets per split
  virtual int nSetsPerSplit() const;

    //! Returns split number i
    virtual TVec<VMat> getSplit(int i=0);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(SequentialSplitter);
  
%> // end of namespace PLearn

#endif // SequentialSplitter_INC
