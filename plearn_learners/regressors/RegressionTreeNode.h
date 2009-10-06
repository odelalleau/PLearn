// -*- C++ -*-

// RegressionTreeNode.h
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
 * $Id: RegressionTreeNode.h, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout     *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

#ifndef RegressionTreeNode_INC
#define RegressionTreeNode_INC

#include <plearn/base/Object.h>
#include <plearn/base/PP.h>
#include <plearn/math/TVec.h>
#include <boost/tuple/tuple.hpp>
#include "RegressionTreeRegisters.h"
#include "RegressionTree.h"

namespace PLearn {
using namespace std;
class RegressionTreeRegisters;
class RegressionTreeLeave;
class RegressionTreeNode;

class RegressionTreeNode: public Object
{
    friend class RegressionTree;
    typedef Object inherited;
  
private:

/*
  Build options: they have to be set before building
*/

    int  missing_is_valid;

    PP<RegressionTree> tree;
    PP<RegressionTreeLeave> leave;
    
/*
  Learnt options: they are sized and initialized if need be, in initNode(...)
*/
 
    Vec leave_output;
    Vec leave_error;
    int split_col;
    int split_balance;
    real split_feature_value;
    real after_split_error;
    PP<RegressionTreeNode> missing_node;
    PP<RegressionTreeLeave> missing_leave;
    PP<RegressionTreeNode> left_node;
    PP<RegressionTreeLeave> left_leave;
    PP<RegressionTreeNode> right_node;
    PP<RegressionTreeLeave> right_leave;
    
    //only there to reload old version. Put static to use less space.
    static int dummy_int;
    static Vec tmp_vec;
    static PP<RegressionTreeLeave> dummy_leave_template;
    static PP<RegressionTreeRegisters> dummy_train_set;
public:  
    RegressionTreeNode();
    RegressionTreeNode(int missing_is_valid);
    virtual              ~RegressionTreeNode();
    
    PLEARN_DECLARE_OBJECT(RegressionTreeNode);

    static  void         declareOptions(OptionList& ol);
    virtual void         makeDeepCopyFromShallowCopy(CopiesMap &copies);
    virtual void         build();
    void         finalize();
    void         initNode(PP<RegressionTree> tree,
                          PP<RegressionTreeLeave> leave);
    void         lookForBestSplit();
    inline void  compareSplit(int col, real left_leave_last_feature,
                              real right_leave_first_feature,
                              Vec left_error, Vec right_error,
                              Vec missing_error);
    int          expandNode();
    inline int   getSplitBalance()const{
        if (split_col < 0) return tree->getSortedTrainingSet()->length();
        return split_balance;}
    inline real  getErrorImprovment()const{
        if (split_col < 0) return -1.0;
        real err=leave_error[0] + leave_error[1] - after_split_error;
        PLASSERT(is_equal(err,0)||err>0);
        return err;
    }
    inline int          getSplitCol() const{return split_col;}
    inline real         getSplitValue() const{return split_feature_value;}
    TVec< PP<RegressionTreeNode> >  getNodes();
    void         computeOutputAndNodes(const Vec& inputv, Vec& outputv,
                                       TVec<PP<RegressionTreeNode> >* nodes=0);
    inline void         computeOutput(const Vec& inputv, Vec& outputv)
    {computeOutputAndNodes(inputv,outputv);}
    inline bool         haveChildrenNode(){return left_node;}
    
private:
    void         build_();
    void         verbose(string msg, int level); 
    static tuple<real,real,int> bestSplitInRow(int col, TVec<RTR_type>& candidates,
                                               Vec left_error, Vec right_error,
                                               const Vec missing_error,
                                               PP<RegressionTreeLeave> right_leave,
                                               PP<RegressionTreeLeave> left_leave,
                                               PP<RegressionTreeRegisters> train_set,
                                               Vec values, 
                                               TVec<pair<RTR_target_t,RTR_weight_t> > t_w
        );
};

DECLARE_OBJECT_PTR(RegressionTreeNode);

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
