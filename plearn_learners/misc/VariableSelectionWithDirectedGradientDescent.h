// -*- C++ -*-

// VariableSelectionWithDirectedGradientDescent.h
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


/* **************************************************************************************************************    
 * $Id: VariableSelectionWithDirectedGradientDescent.h, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout         *
 * This file is part of the PLearn library.                                                                   *
 ************************************************************************************************************** */


#ifndef VariableSelectionWithDirectedGradientDescent_INC
#define VariableSelectionWithDirectedGradientDescent_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {
using namespace std;

class VariableSelectionWithDirectedGradientDescent: public PLearner
{
    typedef PLearner inherited;
private:        
    int inputsize;                              // input size of train set
    int targetsize;                             // output size of train set
    int weightsize;                             // weightsize size of train set
    int length;                                 // number of samples in train set
    int width;                                  // number of columns in train set
  
    real learning_rate;  
    Mat input_weights;
    TVec<bool> weights_selected;
    TVec<int> selected_variables;
  
    PP<ProgressBar> pb;
    int row;
    int col;
    Vec sample_input;
    Vec sample_target;
    real sample_weight;
    Vec sample_output;
    Vec sample_cost;  
    Vec train_criterion;
    real n7_value;
    real n8_value;
    real n9_value;
    real n10_value;
    real n10_gradient;
    real n9_gradient;
    real n8_gradient;
    real n7_gradient;
    Mat weights_gradient;
    int weights_gradient_max_col;
    real weights_gradient_max;
    //! Stores the sum of the absolute values of the gradient over all targets.
    Vec sum_of_abs_gradient;
  
  
public:

    //! Default constructor.
    VariableSelectionWithDirectedGradientDescent();
    
    PLEARN_DECLARE_OBJECT(VariableSelectionWithDirectedGradientDescent);

    virtual void         makeDeepCopyFromShallowCopy(CopiesMap &copies);
    virtual void         build();
    virtual void         train();
    virtual void         forget();
    virtual int          outputsize() const;
    virtual TVec<string> getTrainCostNames() const;
    virtual TVec<string> getTestCostNames() const;
    virtual void         computeOutput(const Vec& input, Vec& output) const;
    virtual void         computeCostsFromOutputs(const Vec& input, const Vec& output, const Vec& target, Vec& costs) const;
    //! Overridden to take into account the new value of targetsize.
    virtual void         setTrainingSet(VMat training_set, bool call_forget=true);


protected:

    static  void         declareOptions(OptionList& ol);

private:
    void         build_();
    void         verbose(string the_msg, int the_level);
};

DECLARE_OBJECT_PTR(VariableSelectionWithDirectedGradientDescent);

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
