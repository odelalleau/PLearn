// -*- C++ -*-

// RegressionTree.h
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


/* ********************************************************************************    
 * $Id: RegressionTree.h, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout         *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

/*! \file PLearnLibrary/PLearnAlgo/RegressionTree.h */

#ifndef RegressionTree_INC
#define RegressionTree_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {
using namespace std;
class RegressionTreeQueue;
class RegressionTreeLeave;
class RegressionTreeRegisters;
class RegressionTreeNode;

class RegressionTree: public PLearner
{
    typedef PLearner inherited;
  
private:

/*
  Build options: they have to be set before training
*/

    bool  missing_is_valid;
    real loss_function_weight;
    int maximum_number_of_nodes;
    int compute_train_stats;   
    real complexity_penalty_factor;
    bool output_confidence_target;
    Vec multiclass_outputs;
    PP<RegressionTreeLeave> leave_template;    
    PP<RegressionTreeRegisters> sorted_train_set;
  
/*
  Learnt options: they are sized and initialized if need be, at stage 0
*/

    PP<RegressionTreeNode> root;
    PP<RegressionTreeLeave> first_leave;
    PP<RegressionTreeQueue> priority_queue;
 
/*
  Work fields: they are sized and initialized if need be, at buid time
*/  
 
    int length;
    real l2_loss_function_factor;
    real l1_loss_function_factor;
    TVec<int> split_cols;
    Vec       split_values;
    TVec<PP<RegressionTreeNode> > *nodes;

    mutable Vec tmp_vec;
    mutable Vec tmp_computeCostsFromOutput;

public:
    RegressionTree();
    virtual              ~RegressionTree();
    
    PLEARN_DECLARE_OBJECT(RegressionTree);

    static  void         declareOptions(OptionList& ol);
    virtual void         makeDeepCopyFromShallowCopy(CopiesMap &copies);
    virtual void         build();
    virtual void         train();
    virtual void         forget();
    virtual int          outputsize() const;
    virtual TVec<string> getTrainCostNames() const;
    virtual TVec<string> getTestCostNames() const;
    virtual void         computeOutput(const Vec& input, Vec& output) const;
    virtual void         computeOutputAndCosts(const Vec& input,
                                               const Vec& target,
                                               Vec& output, Vec& costs) const;
    virtual void         computeOutputAndNodes(const Vec& input, Vec& output,
                                               TVec<PP<RegressionTreeNode> >* nodes=0) const;
    virtual void         computeCostsFromOutputs(const Vec& input, const Vec& output, const Vec& target, Vec& costs) const;
    virtual void         computeCostsFromOutputsAndNodes(const Vec& input,
                                                         const Vec& output, 
                                                         const Vec& target,
                                                         const TVec<PP<RegressionTreeNode> >& nodes,
                                                         Vec& costs) const;
private:
    void                   build_();
    void                   initialiseTree();
    PP<RegressionTreeNode> expandTree();
    void                   verbose(string msg, int level);
};

DECLARE_OBJECT_PTR(RegressionTree);

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
