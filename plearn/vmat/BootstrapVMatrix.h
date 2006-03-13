// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003-2005 Olivier Delalleau
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


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef BootstrapVMatrix_INC
#define BootstrapVMatrix_INC

#include "SelectRowsVMatrix.h"
#include <plearn/math/PRandom.h>

namespace PLearn {
using namespace std;
 
class BootstrapVMatrix: public SelectRowsVMatrix
{
    typedef SelectRowsVMatrix inherited;

protected:

    //! Random number generator for shuffling the data.
    PP<PRandom> rgen;

public:

    //! Public build options
    real frac;
    int n_elems;
    long own_seed;
    long seed;
    bool shuffle;

public:

    //! Default constructor.
    BootstrapVMatrix();
  
    //! Construct a boostrap of another VMatrix.
    //! Note: 'the_seed' sets the new 'own_seed' option, not the old 'seed' one.
    BootstrapVMatrix(VMat m, real frac, bool shuffle = false, long the_seed = -2);

    PLEARN_DECLARE_OBJECT(BootstrapVMatrix);

    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    static void declareOptions(OptionList &ol);

private:

    void build_();

};

DECLARE_OBJECT_PTR(BootstrapVMatrix);

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
