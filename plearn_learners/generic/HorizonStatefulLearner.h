// -*- C++ -*-

// HorizonStatefulLearner.h
//
// Copyright (C) 2004 Nicolas Chapados
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

// Authors: Nicolas Chapados

/*! \file HorizonStatefulLearner.h */


#ifndef HorizonStatefulLearner_INC
#define HorizonStatefulLearner_INC

#include "StatefulLearner.h"

namespace PLearn {
using namespace std;

/**
 * A HorizonStatefulLearner is a StatefulLearner designed for forecasting
 * at horizon h.  It serves as a base class for more specialized
 * forecasters.  Contrarily to StatefulLearner, the HorizonStatefulLearner
 * introduces a few additional assumptions on the structure of the training
 * set and the interpretation of the various test methods:
 *
 * - In the training set, it assumes that the inputs and targets are spaced
 *   apart by the forecasting horizon.  Hence, for the inputs at row t, the
 *   targets are located at row t+horizon.
 *
 * - For computeOutputAndCosts (and friends), the inputs and outputs vector
 *   are FOR THE CURRENT TIME STEP, but the targets and costs correspond to
 *   the inputs/outputs that were horizon time steps IN THE PAST.  This
 *   means that derived classes will usually have to keep a memory of the
 *   past "horizon" inputs and/or outputs in order to compute meaningful
 *   costs.
 */  
class HorizonStatefulLearner: public StatefulLearner
{
    typedef StatefulLearner inherited;

public:
    //#####  Public Options  ###################################################

    //! Forecasting horizon for the learner; see detailed class help for
    //! interpretation.  (Default value = 0)
    int horizon;
  
public:
    //#####  Object Methods  ###################################################

    //! Default constructor.
    HorizonStatefulLearner();

    //! Simply calls inherited::build() then build_().
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods.
    PLEARN_DECLARE_ABSTRACT_OBJECT(HorizonStatefulLearner);

protected:   
    //! Declares this class' options.
    static void declareOptions(OptionList& ol);

private: 
    //! This does the actual building. 
    void build_();
};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(HorizonStatefulLearner);
  
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
