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
   * $Id: SumOfVariable.h,v 1.4 2004/02/20 21:11:53 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef SumOfVariable_INC
#define SumOfVariable_INC

#include "NaryVariable.h"

namespace PLearn {
using namespace std;


class SumOfVariable: public NaryVariable
{
  protected:
    //!  protected default constructor for persistence
    SumOfVariable() : distr(), f(), nsamples(), curpos() {}

  public:
  //protected:
    VMat distr;
    Func f;
    int nsamples;
    int curpos; //!<  current pos in VMat 
    // To avoid allocation/deallocations in fprop/bprop
    Vec input_value;
    Vec input_gradient;
    Vec output_value;
    
  public:
    //!  Sum_{inputs \in distr} f(inputs)
    SumOfVariable(VMat the_distr, Func the_f, int the_nsamples=-1);
    
    PLEARN_DECLARE_OBJECT(SumOfVariable);
    virtual void recomputeSize(int& l, int& w) const;
    virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
    virtual void fprop();
    virtual void bprop();
    virtual void fbprop();
    virtual void symbolicBprop();
    virtual void rfprop();
    
    void printInfo(bool print_gradient);
};

//!  sumOf
inline Var sumOf(VMat distr, Func f, int nsamples)
{ return new SumOfVariable(distr,f,nsamples); }

//!  deprecated old version do not use!
inline Var sumOf(Var output, const VarArray& inputs, VMat distr, int nsamples, VarArray parameters=VarArray())
{ return sumOf(distr,Func(inputs,output),nsamples); }

//!  meanOf
inline Var meanOf(VMat distr, Func f, int nsamples)
{ return new SumOfVariable(distr,f/nsamples,nsamples); }

//!  deprecated old version do not use!
inline Var meanOf(Var output, const VarArray& inputs, VMat distr, int nsamples, VarArray parameters=VarArray())
{ return meanOf(distr, Func(inputs,output), nsamples); }

} // end of namespace PLearn

#endif 
