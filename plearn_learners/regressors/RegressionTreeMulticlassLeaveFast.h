// -*- C++ -*-

// RegressionTreeMulticlassLeaveFast.h
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
 * $Id: RegressionTreeMulticlassLeaveFast.h, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout     *
 * This file is part of the PLearn library.                                      *
 ********************************************************************************* */

#ifndef RegressionTreeMulticlassLeaveFast_INC
#define RegressionTreeMulticlassLeaveFast_INC

#include "RegressionTreeLeave.h"

namespace PLearn {
using namespace std;

class RegressionTreeMulticlassLeaveFast: public RegressionTreeLeave
{
    typedef RegressionTreeLeave inherited;
  
private:

/*
  Build options: they have to be set before building
*/

    int nb_class;
    string objective_function;       
 
/*
  Learnt options: they are sized and initialized if need be, in initLeave(...)
*/

    real loss_function_factor;
    Vec multiclass_weights_sum;
 
public:
    RegressionTreeMulticlassLeaveFast();
    virtual              ~RegressionTreeMulticlassLeaveFast();
    PLEARN_DECLARE_OBJECT(RegressionTreeMulticlassLeaveFast);

    static  void         declareOptions(OptionList& ol);
    virtual void         makeDeepCopyFromShallowCopy(CopiesMap &copies);
    virtual void         build();
    void         initStats();
    void         addRow(int row);
    void         addRow(int row, real target, real weight);
    void         addRow(int row, Vec outputv, Vec errorv);
    void         addRow(int row, real target, real weight, Vec outputv, Vec errorv);
    void         removeRow(int row, real target, real weight);
    void         removeRow(int row, Vec outputv, Vec errorv);
    void         removeRow(int row, real target, real weight, Vec outputv, Vec errorv);
    void         getOutputAndError(Vec& output, Vec& error)const;
    void         printStats();
  
private:
    void         build_();
};

DECLARE_OBJECT_PTR(RegressionTreeMulticlassLeaveFast);

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
