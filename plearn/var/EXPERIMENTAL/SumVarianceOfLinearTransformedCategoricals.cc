// -*- C++ -*-

// SumVarianceOfLinearTransformedCategoricals.cc
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

/*! \file SumVarianceOfLinearTransformedCategoricals.cc */


#include "SumVarianceOfLinearTransformedCategoricals.h"

namespace PLearn {
using namespace std;

/** SumVarianceOfLinearTransformedCategoricals **/

PLEARN_IMPLEMENT_OBJECT(
    SumVarianceOfLinearTransformedCategoricals,
    "Computes the sum of the variances of the elements of a linear transformation of a concatenation of independent random variables following a categorical distribution.",
    "By categorical distribution we mean multinomials with the number-of-trials parameter set to n=1.\n"
    "Let P=inpu1 a (l,d') matrix. Each row contains the parameters of groupsize.length() categoricals.\n"
    "  ( the sum of the groupsize elements equals d').\n"
    "Let H ~ MultipleCategorical(P) a (l,d') corresponding random variable, \n"
    "  this correponds to drawings from independent categorical variables whose probablity parameters are those in P \n"
    "Let W=input2 a (d,d') linear transformation matrix. \n"
    "Let X=H W^t a (l,d) random variable matrix corresponding to applying the transformation. \n"
    "  i.e. X_i = \sum_j H_ij W^t_j \n" 
    "SumVarianceOfLinearTransformedCategoricals computes the sum of the variances of the elements of X.\n"
    "i.e. \sum_ij Var[X_ij] \n"
    );

SumVarianceOfLinearTransformedCategoricals::SumVarianceOfLinearTransformedCategoricals()
    : inherited(0,0,1,1)
{}

SumVarianceOfLinearTransformedCategoricals::SumVarianceOfLinearTransformedCategoricals(Variable* input1, Variable* input2,
                           bool call_build_)
    : inherited(input1, input2, 1, 1, call_build_)
{
    if (call_build_)
        build_();
}

// constructor from input variable and parameters
// SumVarianceOfLinearTransformedCategoricals::SumVarianceOfLinearTransformedCategoricals(Variable* input1, Variable* input2,
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

void SumVarianceOfLinearTransformedCategoricals::recomputeSize(int& l, int& w) const
{
    l = 1;
    w = 1;
}

// ### computes value from input1 and input2 values
void SumVarianceOfLinearTransformedCategoricals::fprop()
{
    if(input1.width()!=input2.width())
        PLERROR("Incompatible sizes: width of P (input1) must equal width of W (input2)"); 

    Mat P = input1->matValue;
    Mat W = input2->matValue;
    int d = W.length();
    int l = P.length();
    // int m = W.width(); // should equal sum of groupsizes

    double simplesum = 0;
    double sqsum = 0;

    int ngroups = groupsizes.length();
    const int* pgroupsize = groupsizes.data();

    for(int t=0; t<l; t++)
    {
        const real* p = P[t];
        for(int i=0; i<d; i++)
        {
            const real* Wi = W[i];
            int k = 0;
            for(int groupnum=0; groupnum<ngroups; groupnum++)
            {
                double tmpsqsum = 0;
                int gs = pgroupsize[groupnum];
                while(gs--)
                {
                    real Wik = Wi[k];
                    real pk = p[k];
                    real Wik_pk = Wik*pk;
                    simplesum += Wik*Wik_pk;
                    tmpsqsum += Wik_pk;
                    k++;
                }
                sqsum += tmpsqsum*tmpsqsum;
            }
        }
    }
    value[0] = simplesum-sqsum;
}

// ### computes input1 and input2 gradients from gradient
void SumVarianceOfLinearTransformedCategoricals::bprop()
{
    Mat P = input1->matValue;
    Mat Pgrad = input1->matGradient;
    Mat W = input2->matValue;
    Mat Wgrad = input2->matGradient;
    int d = W.length();
    int l = P.length();
    // int m = W.width(); // should equal sum of groupsizes

    real gr = gradient[0];

    int ngroups = groupsizes.length();
    const int* pgroupsize = groupsizes.data();    

    group_sum_wik_pk.resize(ngroups);
    real* p_group_sum_wik_pk = group_sum_wik_pk.data();

    for(int t=0; t<l; t++)
    {
        const real* p = P[t];
        real* gp = Pgrad[t];
        for(int i=0; i<d; i++)
        {
            const real* Wi = W[i];
            int k = 0;
            for(int groupnum=0; groupnum<ngroups; groupnum++)
            {
                int gs = pgroupsize[groupnum];
                double sum_wik_pk = 0;
                while(gs--)
                {
                    sum_wik_pk += Wi[k]*p[k];
                    k++;
                }
                p_group_sum_wik_pk[groupnum] = sum_wik_pk;
            }
                    
            real* gWi = Wgrad[i];
            k = 0;
            for(int groupnum=0; groupnum<ngroups; groupnum++)
            {
                int gs = pgroupsize[groupnum];
                real grsum = p_group_sum_wik_pk[groupnum];
                while(gs--)
                {
                    real Wik = Wi[k];
                    gWi[k] += gr*2*p[k]*(Wik-grsum);
                    gp[k] += gr*Wik*(Wik-2*grsum);
                    k++;
                }
            }
        }
    }

}

// ### You can implement these methods:
// void SumVarianceOfLinearTransformedCategoricals::bbprop() {}
// void SumVarianceOfLinearTransformedCategoricals::symbolicBprop() {}
// void SumVarianceOfLinearTransformedCategoricals::rfprop() {}


// ### Nothing to add here, simply calls build_
void SumVarianceOfLinearTransformedCategoricals::build()
{
    inherited::build();
    build_();
}

void SumVarianceOfLinearTransformedCategoricals::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);
    deepCopyField(groupsizes, copies);
    deepCopyField(group_sum_wik_pk, copies);

    // ### If you want to deepCopy a Var field:
    // varDeepCopyField(somevariable, copies);
}

void SumVarianceOfLinearTransformedCategoricals::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "groupsizes", &SumVarianceOfLinearTransformedCategoricals::groupsizes,
                  OptionBase::buildoption,
                  "defines the dimensions of the categorical variables.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void SumVarianceOfLinearTransformedCategoricals::build_()
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
