// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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

#ifndef OneHotVMatrix_INC
#define OneHotVMatrix_INC

#include "SourceVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;
 

/*!
  Sampling from this VMat will return the corresponding sample
  from the source VMat with last element ('target_classnum')
  replaced by a vector of target_values of size nclasses in which only
  target_values[target_classnum] is set to hot_value, and all the
  others are set to cold_value.
  In the special case where the VMat is built with nclasses==1, then
  it is assumed that we have a 2 class classification problem but
  we are using a single valued target.  For this special case only
  the_cold_value is used as target for classnum 0 and the_hot_value is
  used for classnum 1.
*/

class OneHotVMatrix: public SourceVMatrix
{
    typedef SourceVMatrix inherited;

protected:
//    VMat underlying_distr; // DEPRECATED - use 'source' instead
    int nclasses;
    real cold_value;
    real hot_value;
    int index;

public:
    // ******************
    // *  Constructors  *
    // ******************

    //!  default constructor (for automatic deserialization)
    OneHotVMatrix(bool call_build_ = false);

    //!  (see special case when nclasses==1 desribed above)
    OneHotVMatrix(VMat the_source, int the_nclasses,
                  real the_cold_value=0.0, real the_host_value=1.0,
                  int the_index=-1, bool call_build_ = false);

    PLEARN_DECLARE_OBJECT(OneHotVMatrix);

protected:

    virtual void getNewRow(int i, const Vec& samplevec) const;
    static void declareOptions(OptionList &ol);

public:

    virtual void build();

    virtual void reset_dimensions()
    {
        source->reset_dimensions();
        width_ = source->width() + nclasses - 1;
        length_ = source->length();
    }
    virtual real dot(int i1, int i2, int inputsize) const;
    virtual real dot(int i, const Vec& v) const;
private:
    void build_();
};

inline VMat onehot(VMat the_source, int nclasses,
                   real cold_value=0.0, real hot_value=1.0, int index=-1,
                   bool call_build_=false)
{ return new OneHotVMatrix(the_source, nclasses, cold_value, hot_value, index,
                           call_build_); }

DECLARE_OBJECT_PTR(OneHotVMatrix);

} // end of namespcae PLearn
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
