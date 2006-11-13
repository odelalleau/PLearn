// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2006 Xavier Saint-Mleux
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


/*! \file PLearn/plearn/vmat/EncodedVMatrix.h */

#ifndef EncodedVMatrix_INC
#define EncodedVMatrix_INC

#include "VMat.h"
#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;

class EncodedVMatrix: public SourceVMatrix
{
    typedef SourceVMatrix inherited;

public:

    TVec<map<real,real> > encodings;
    TVec<real> defaults;
    TVec<bool> encode;

    //! For all constructors, the original VMFields are copied upon construction
    EncodedVMatrix(bool call_build_=false);

    EncodedVMatrix(VMat the_source, TVec<map<real,real> > encodings_, TVec<real> defaults_, 
		   TVec<bool> encode_, bool call_build_=true);

    PLEARN_DECLARE_OBJECT(EncodedVMatrix);
    
    //! Transform a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void getNewRow(int i, const Vec& v) const;

    static void encodeRow(const TVec<map<real,real> >& encodings, const TVec<real>& defaults, 
                               const TVec<bool>& encode, const Vec& v);

private:
    //! This does the actual building.
    void build_();

protected:
    //! Declares this class' options
    static void declareOptions(OptionList& ol);

public:
    // simply calls inherited::build() then build_()
    virtual void build();

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(EncodedVMatrix);

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
