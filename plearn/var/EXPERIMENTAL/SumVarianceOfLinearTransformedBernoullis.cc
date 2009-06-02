// -*- C++ -*-

// SumVarianceOfLinearTransformedBernoullis.cc
//
// Copyright (C) 2009 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file SumVarianceOfLinearTransformedBernoullis.cc */


#include "SumVarianceOfLinearTransformedBernoullis.h"

namespace PLearn {
using namespace std;

/** SumVarianceOfLinearTransformedBernoullis **/

PLEARN_IMPLEMENT_OBJECT(
    SumVarianceOfLinearTransformedBernoullis,
    "Computes the sum of the variances of the components resulting from a linear transformation of independent bernoullis",
    "Let H be a (l,m) matrix of independent bernoullis.\n"
    "Let P be a (l,m) matrix containing their corresponding parameters (expectations or prob of 1) \n"
    "Let W a (m,d) linear transformation matrix, \n"
    "Define Y=HW, Y is a (l,d) matrix of random variables, with Y_ij = <H_i,W_.j> \n"
    "i.e. Y_ij is a linear combination of bernoullis in row i of H whose parameters are in row i of P, \n"
    "with the combination coefficients being in column j of W \n"
    "We have Var[Y_ij] = sum_k Var[H_ik] (W_kj)^2 \n"
    "                  = sum_k P_ik (1-P_ik) (W_kj)^2 \n"
    "This binary variable, given input1 P and input2 W, computes:\n"
    "   out = sum_ij Var[Y_ij] \n"
    "       = sum_ijk P_ik (1-P_ik) (W_kj)^2 \n"
    "       = sum_i {sum_k P_ik (1-P_ik) {sum_j (W_kj)^2}} \n"
    "       = sum_i {sum_k P_ik (1-P_ik) ||W_k.||^2 } \n"
    );

SumVarianceOfLinearTransformedBernoullis::SumVarianceOfLinearTransformedBernoullis()
    : inherited(0,0,1,1)
{}

SumVarianceOfLinearTransformedBernoullis::SumVarianceOfLinearTransformedBernoullis(Variable* input1, Variable* input2,
                           bool call_build_)
    : inherited(input1, input2, 1, 1, call_build_)
{
    if (call_build_)
        build_();
}

// constructor from input variable and parameters
// SumVarianceOfLinearTransformedBernoullis::SumVarianceOfLinearTransformedBernoullis(Variable* input1, Variable* input2,
//                            param_type the_parameter, ...,
//                            bool call_build_)
// ### replace with actual parameters
//  : inherited(input1, input2, this_variable_length, this_variable_width,
//              call_build_),
//    parameter(the_parameter),
//    ...
//{
//    if (call_build_)
//        build_();
//}

void SumVarianceOfLinearTransformedBernoullis::recomputeSize(int& l, int& w) const
{
    l = 1;
    w = 1;
}

void SumVarianceOfLinearTransformedBernoullis::computeWsqnorm()
{
    Mat W = input2->matValue;
    int m = W.length();
    int d = W.width();

    Wsqnorm.resize(m);
    real* pWsqnorm = Wsqnorm.data();
    for(int k=0; k<m; k++)
    {
        real* Wk = W[k];
        real sqnorm = 0;
        for(int j=0; j<d; j++)
        {
            real Wkj = Wk[j];
            sqnorm += Wkj*Wkj;
        }
        pWsqnorm[k] = sqnorm;
    }
}

// ### computes value from input1 and input2 values
void SumVarianceOfLinearTransformedBernoullis::fprop()
{
    Mat P = input1->matValue;
    int l = P.length();
    int m = P.width();

    if(m!=input2.length())
        PLERROR("Incompatible sizes: width of P (input1) must equal length of W (input2)"); 

    computeWsqnorm();
    const real* pWsqnorm = Wsqnorm.data();

    double out = 0;
    for(int i=0; i<l; i++)
    {
        const real* P_i = P[i];
        for(int k=0; k<m; k++)
        {
            real P_ik = P_i[k];
            out += P_ik*(1-P_ik)*pWsqnorm[k];
        }
    }
    value[0] = out;
}

// ### computes input1 and input2 gradients from gradient
void SumVarianceOfLinearTransformedBernoullis::bprop()
{
    Mat P = input1->matValue;
    Mat Pgrad = input1->matGradient;
    Mat W = input2->matValue;
    Mat Wgrad = input2->matGradient;
    int l = P.length();
    int m = W.length();
    int d = W.width();

    real gr = gradient[0];

    computeWsqnorm();
    const real* pWsqnorm = Wsqnorm.data();

    Wsqnormgrad.resize(m);
    Wsqnormgrad.fill(0);
    real* pWsqnormgrad = Wsqnormgrad.data(); 

    for(int i=0; i<l; i++)
    {
        const real* P_i = P[i];
        real* Pgrad_i = Pgrad[i];
        for(int k=0; k<m; k++)
        {
            real P_ik = P_i[k];
            Pgrad_i[k] += gr*(1-2*P_ik)*pWsqnorm[k];
            pWsqnormgrad[k] += P_ik*(1-P_ik);
        }
    }

    for(int k=0; k<m; k++)
    {
        real coefk = 2*gr*pWsqnormgrad[k];
        const real* W_k = W[k];
        real* Wgrad_k = Wgrad[k];
        for(int j=0; j<d; j++)
            Wgrad_k[j] += coefk*W_k[j];
    }
    
}

// ### You can implement these methods:
// void SumVarianceOfLinearTransformedBernoullis::bbprop() {}
// void SumVarianceOfLinearTransformedBernoullis::symbolicBprop() {}
// void SumVarianceOfLinearTransformedBernoullis::rfprop() {}


// ### Nothing to add here, simply calls build_
void SumVarianceOfLinearTransformedBernoullis::build()
{
    inherited::build();
    build_();
}

void SumVarianceOfLinearTransformedBernoullis::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);
    deepCopyField(Wsqnorm, copies);
    deepCopyField(Wsqnormgrad, copies);

    // ### If you want to deepCopy a Var field:
    // varDeepCopyField(somevariable, copies);
}

void SumVarianceOfLinearTransformedBernoullis::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &SumVarianceOfLinearTransformedBernoullis::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void SumVarianceOfLinearTransformedBernoullis::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.
}


} // end of namespace PLearn


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
