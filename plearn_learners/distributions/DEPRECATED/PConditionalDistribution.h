// -*- C++ -*-

// PConditionalDistribution.h
// 
// Copyright (C) 2004 Université de Montréal
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


#ifndef PConditionalDistribution_INC
#define PConditionalDistribution_INC

#include "PDistribution.h"

namespace PLearn {
using namespace std;

class PConditionalDistribution: public PDistribution
{

private:

    typedef PDistribution inherited;  

protected:

    // *********************
    // * protected options *
    // *********************

    // ### declare protected option fields (such as learnt parameters) here
    // ...
    
public:

    // the part of the PLearner's inputsize() that corresponds to the size
    // of the conditioning variable. It should be < inputsize().
    int input_part_size;

private: 

    //! This does the actual building. 
    // (Please implement in .cc)
    void build_();

protected: 
    //! Declares this class' options
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);

public:

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    PConditionalDistribution();

    //! Simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Set the input part before using the inherited methods.
    virtual void setInput(const Vec& input) const;

    virtual void computeOutput(const Vec& input, Vec& output) const;

    // Declares a few other classes and functions related to this class
    PLEARN_DECLARE_OBJECT(PConditionalDistribution);
};

  
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
