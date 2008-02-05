// -*- C++ -*-

// RegressionTreeMulticlassLeave.h
// Copyright (c) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio and University of Montreal
// Copyright (c) 2002 Jean-Sebastien Senecal, Xavier Saint-Mleux, Rejean Ducharme
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


/* *********************************************************************************   
 * $Id: RegressionTreeMulticlassLeave.h, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout     *
 * This file is part of the PLearn library.                                      *
 ********************************************************************************* */

#ifndef RegressionTreeMulticlassLeave_INC
#define RegressionTreeMulticlassLeave_INC

#include "RegressionTreeLeave.h"

namespace PLearn {
using namespace std;

class RegressionTreeMulticlassLeave: public RegressionTreeLeave
{
    typedef RegressionTreeLeave inherited;
  
private:

/*
  Build options: they have to be set before building
*/

    Vec multiclass_outputs;
    string objective_function;       
 
/*
  Learnt options: they are sized and initialized if need be, in initLeave(...)
*/

    real l1_loss_function_factor;
    real l2_loss_function_factor;
    Vec multiclass_weights_sum;
 
/*
  Work fields: they are sized and initialized if need be, at buid time
*/  
 
    int multiclass_winer;
  
public:
    RegressionTreeMulticlassLeave();
    virtual              ~RegressionTreeMulticlassLeave();
    PLEARN_DECLARE_OBJECT(RegressionTreeMulticlassLeave);

    static  void         declareOptions(OptionList& ol);
    virtual void         makeDeepCopyFromShallowCopy(CopiesMap &copies);
    virtual void         build();
    void         initStats();
    void         addRow(int row, Vec outputv, Vec errorv);
    void         addRow(int row);
    void         removeRow(int row, Vec outputv, Vec errorv);
    virtual void getOutputAndError(Vec& output, Vec& error);
    void         printStats();
  
private:
    void         build_();
};

DECLARE_OBJECT_PTR(RegressionTreeMulticlassLeave);

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
