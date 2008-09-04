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

#ifndef MatrixSumOfVariable_INC
#define MatrixSumOfVariable_INC

#include "NaryVariable.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;


class MatrixSumOfVariable: public NaryVariable
{
    typedef NaryVariable inherited;

public:
    //protected:
    VMat distr;
    Func f;
    int nsamples;
    int input_size;
    int curpos; //!<  current pos in VMat 
    // To avoid allocation/deallocations in fprop/bprop
    Vec input_value;
    Vec input_gradient;
    Vec output_value;
    
public:
    //!  protected default constructor for persistence
    MatrixSumOfVariable() : distr(), f(), nsamples(), input_size(), curpos() {}
    //! \f$ Sum_{inputs \in distr} f(inputs)\f$
    MatrixSumOfVariable(VMat the_distr, Func the_f, int the_nsamples=-1, int the_input_size=-1);
    
    PLEARN_DECLARE_OBJECT(MatrixSumOfVariable);
    static void declareOptions(OptionList &ol);

    virtual void build();

    virtual void recomputeSize(int& l, int& w) const;
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
    virtual void fprop();
    virtual void bprop();
    virtual void fbprop();
    virtual void symbolicBprop();
    virtual void rfprop();
    
    void printInfo(bool print_gradient);

protected:
    void build_();
};

DECLARE_OBJECT_PTR(MatrixSumOfVariable);

// sumOf with matrix as inputs
inline Var sumOf(VMat distr, Func f, int nsamples, int input_size)
{ return new MatrixSumOfVariable(distr,f,nsamples,input_size); }

// meanOf with matrix as inputs
inline Var meanOf(VMat distr, Func f, int nsamples, int input_size)
{ return new MatrixSumOfVariable(distr, f/nsamples, nsamples, input_size); }

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
