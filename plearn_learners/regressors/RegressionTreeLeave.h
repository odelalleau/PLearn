// -*- C++ -*-

// RegressionTreeLeave.h
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
 * $Id: RegressionTreeLeave.h, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout     *
 * This file is part of the PLearn library.                                      *
 ********************************************************************************* */

#ifndef RegressionTreeLeave_INC
#define RegressionTreeLeave_INC

#include <plearn/base/Object.h>
#include "RegressionTreeRegisters.h"
namespace PLearn {
using namespace std;

class RegressionTreeLeave: public Object
{
    typedef Object inherited;
 
    friend class RegressionTreeNode;
    friend class RegressionTree;

    Vec dummy_vec;

public:
    bool missing_leave;
    real loss_function_weight;
    static int  verbosity;

protected:

/*
  Build options: they have to be set before building
*/

    RTR_type_id  id;
    PP<RegressionTreeRegisters> train_set;
 
/*
  Learnt options: they are sized and initialized if need be, in initLeave(...)
*/

    int  length;
    real weights_sum;
    real targets_sum;
    real weighted_targets_sum;
    real weighted_squared_targets_sum;
    real loss_function_factor;
 
public:
    RegressionTreeLeave();
    virtual              ~RegressionTreeLeave();
    PLEARN_DECLARE_OBJECT(RegressionTreeLeave);

    static  void         declareOptions(OptionList& ol);
    virtual void         makeDeepCopyFromShallowCopy(CopiesMap &copies);
    virtual void         build();
    void         initLeave(PP<RegressionTreeRegisters> the_train_set);
    virtual void         initStats();
    virtual void         addRow(int row);
    virtual void         addRow(int row, real target, real weight);
    virtual void         addRow(int row, Vec outputv, Vec errorv);
    virtual void         addRow(int row, real target, real weight, Vec outputv, Vec errorv);
    virtual void         removeRow(int row, Vec outputv, Vec errorv);
    virtual void         removeRow(int row, real target, real weight, Vec outputv, Vec errorv);
    inline void          registerRow(int row)
    {train_set->registerLeave(id, row);}
    inline int           getId()const{return id;}
    inline int           getLength()const{return length;}
    virtual void         getOutputAndError(Vec& output, Vec& error)const;
    virtual void         printStats();
  
private:
    void         build_();
    void         verbose(string msg, int level);
};

DECLARE_OBJECT_PTR(RegressionTreeLeave);

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
