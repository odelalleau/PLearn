// -*- C++ -*-

// PartsDistanceKernel.h
// Copyright (C) 2008 Pascal Vincent
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

/*! \file PartsDistanceKernel.h */


/* *******************************************************      
 * $Id: PartsDistanceKernel.h 7664 2007-06-28 19:47:30Z nouiz $
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef PartsDistanceKernel_INC
#define PartsDistanceKernel_INC

#include <plearn/ker/Kernel.h>

namespace PLearn {
using namespace std;

//! This class implements an Ln distance (defaults to L2 i.e. euclidean distance).
class PartsDistanceKernel: public Kernel
{

private:

    typedef Kernel inherited;

public:

    real partsize;
    real n;  //!<  1 for L1, 2 for L2, etc...
    bool standardize;
    real min_stddev;
    real epsilon;
    bool pow_distance;

    Vec inv_stddev;

    PartsDistanceKernel();

    virtual void train(VMat data);
    virtual real evaluate(const Vec& x1, const Vec& x2) const;
    
    PLEARN_DECLARE_OBJECT(PartsDistanceKernel);

protected:
    mutable Vec elementdist;

    static void declareOptions(OptionList& ol);
};

DECLARE_OBJECT_PTR(PartsDistanceKernel);

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
