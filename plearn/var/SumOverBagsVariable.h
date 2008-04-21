// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef SumOverBagsVariable_INC
#define SumOverBagsVariable_INC

#include "NaryVariable.h"

namespace PLearn {
using namespace std;


class SumOverBagsVariable: public NaryVariable
{
    typedef NaryVariable inherited;

public:
//protected:
    VMat vmat;
    Func f;
    bool average;
    int max_bag_size;
    int n_samples;
    bool transpose;

    int curpos; //!<  current pos in VMat 
    // To avoid allocation/deallocations in fprop/bprop
    Vec output_value;
    Mat input_values;
    Vec bag_size_vec;
    Vec bag_target_and_bag_signal;
    Vec bag_target;
    Vec bag_signal;
    Vec bag_weight;
    Array<Vec> f_inputs; // (matrix of bag inputs, the bag size, the bag target, the bag weight)
    Array<Vec> unused_gradients;
    Array<Vec> output_av; // Array<Vec>(output_value)
    Array<Vec> gradient_av; // Array<Vec>(gradient)
    int bag_size;

public:
    //!  protected default constructor for persistence
    SumOverBagsVariable();
    //! Sum_{bags \in vmat} f(inputs and targets in bag)
    //! By convention a bag is a sequence of rows of the vmat in which the last column of the target
    //! indicates whether the row is the first one (and/or) the last one, with its two least significant bits:
    //!   last_column_of_target == 1 ==> first row
    //!   last_column_of_target == 2 ==> last row
    //!   last_column_of_target == 0 ==> intermediate row
    //!   last_column_of_target == 1+2==3 ==> single-row bag (both first and last)
    //! the last column of the target is not given in the call to f, but a bag_size input is provided instead.
    //! The inputs to f are: (matrix of bag inputs, the bag size, the bag target, the bag weight).
    SumOverBagsVariable(VMat the_vmat, Func the_f, int maxbagsize,
                        int nsamples, bool average,
                        bool transpose = false, bool call_build_ = true);

    //! Tags to use in the vmat given to this variable.
    static const int TARGET_COLUMN_FIRST = 1;
    static const int TARGET_COLUMN_LAST = 2;
    static const int TARGET_COLUMN_SINGLE = 3;  // Beginning == End
    static const int TARGET_COLUMN_INTERMEDIATE = 0;

    PLEARN_DECLARE_OBJECT(SumOverBagsVariable);

    virtual void build();

    virtual void recomputeSize(int& l, int& w) const;
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
    virtual void fprop();
    virtual void bprop();
    virtual void fbprop();
    virtual void fpropOneBag(bool do_bprop=false);
    
    void printInfo(bool print_gradient);

    static void declareOptions(OptionList& ol);

protected:
    void build_();
};

DECLARE_OBJECT_PTR(SumOverBagsVariable);

//!  sumOf
inline Var sumOverBags(VMat vmat, Func f, int max_bag_size, int nsamples, bool average = false, bool transpose = false)
{ return new SumOverBagsVariable(vmat, f, max_bag_size, nsamples, average, transpose); }

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
