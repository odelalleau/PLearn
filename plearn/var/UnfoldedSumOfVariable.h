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
   * $Id: UnfoldedSumOfVariable.h,v 1.2 2004/02/16 16:25:40 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef UnfoldedSumOfVariable_INC
#define UnfoldedSumOfVariable_INC

#include "NaryVariable.h"

namespace PLearn <%
using namespace std;


class UnfoldedSumOfVariable: public NaryVariable
{
  protected:
    //!  protected default constructor for persistence
    UnfoldedSumOfVariable() : distr(), f(), max_bag_size(), curpos() {}

  public:
  //protected:
    VMat distr;
    Func f;
    int max_bag_size;

    int curpos; //!<  current pos in VMat 
    // To avoid allocation/deallocations in fprop/bprop
  Vec input_value;
  Vec target;
    Mat input_values;
    Mat output_values;
  int bag_size;

  TVec<VarArray> inputs; // all the input Var's
  TVec<VarArray> outputs; // and the corresponding output Var's, 
  TVec<VarArray> f_paths; // the duplicates of f prop. path for each input/output pair: inputs[i]->outputs[i]

  public:
    //!  Sum_{consecutive inputs \in distr until one has non-missing end_of_bag field} f(inputs)
    //! the end_of_bag field is at column end_of_bag_column in each row of the VMat.
    UnfoldedSumOfVariable(VMat the_distr, Func the_f, int maxbagsize);
    
    PLEARN_DECLARE_OBJECT(UnfoldedSumOfVariable);
    virtual void build();
    virtual void recomputeSize(int& l, int& w) const;
    virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
    virtual void fprop();
    virtual void bprop();
    
    void printInfo(bool print_gradient);

    static void declareOptions(OptionList& ol);

  private:
    void build_();
};

//!  sumOf
inline Var unfoldedSumOf(VMat distr, Func f, int nsamples)
{ return new UnfoldedSumOfVariable(distr,f,nsamples); }

//!  deprecated old version do not use!
inline Var unfoldedSumOf(Var output, const VarArray& inputs, VMat distr, int nsamples, VarArray parameters=VarArray())
{ return unfoldedSumOf(distr,Func(inputs,output),nsamples); }

//!  meanOf
inline Var unfoldedMeanOf(VMat distr, Func f, int nsamples)
{ return new UnfoldedSumOfVariable(distr,f/nsamples,nsamples); }

//!  deprecated old version do not use!
inline Var unfoldedMeanOf(Var output, const VarArray& inputs, VMat distr, int nsamples, VarArray parameters=VarArray())
{ return unfoldedMeanOf(distr, Func(inputs,output), nsamples); }

%> // end of namespace PLearn

#endif 
