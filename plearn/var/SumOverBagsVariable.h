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
   * $Id: SumOverBagsVariable.h,v 1.2 2004/02/19 15:25:31 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef SumOverBagsVariable_INC
#define SumOverBagsVariable_INC

#include "NaryVariable.h"

namespace PLearn <%
using namespace std;


class SumOverBagsVariable: public NaryVariable
{
  protected:
    //!  protected default constructor for persistence
    SumOverBagsVariable() : vmat(), f(), max_bag_size(-1), n_samples(1), curpos() {}

  public:

  typedef NaryVariable inherited;

  //protected:
    VMat vmat;
    Func f;
    int max_bag_size;
    int n_samples;

    int curpos; //!<  current pos in VMat 
    // To avoid allocation/deallocations in fprop/bprop
  Vec output_value;
  Mat input_values;
  Vec bag_size_vec;
  Vec bag_target;
  Vec bag_weight;
  Array<Vec> f_inputs; // (matrix of bag inputs, the bag size, the bag target, the bag weight)
  Array<Vec> unused_gradients;
  Array<Vec> output_av; // Array<Vec>(output_value)
  Array<Vec> gradient_av; // Array<Vec>(gradient)
  int bag_size;

  public:
    //! Sum_{bags \in vmat} f(inputs and targets in bag)
    //! By convention a bag is a sequence of rows of the vmat whose target is missing
    //! except for the last row of the bag. The input to f is a VarArray whose
    //! elements are the following: matrix of bag inputs, the bag size, the bag target, the bag weight.
    SumOverBagsVariable(VMat the_vmat, Func the_f, int maxbagsize, int nsamples);
    
    PLEARN_DECLARE_OBJECT(SumOverBagsVariable);
    virtual void build();
    virtual void recomputeSize(int& l, int& w) const;
    virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
    virtual void fprop();
    virtual void bprop();
    virtual void fbprop();
    virtual void fpropOneBag(bool do_bprop=false);
    
    void printInfo(bool print_gradient);

    static void declareOptions(OptionList& ol);

  private:
    void build_();
};

//!  sumOf
inline Var sumOverBags(VMat vmat, Func f, int max_bag_size, int nsamples)
{ return new SumOverBagsVariable(vmat,f,max_bag_size,nsamples); }

%> // end of namespace PLearn

#endif 
