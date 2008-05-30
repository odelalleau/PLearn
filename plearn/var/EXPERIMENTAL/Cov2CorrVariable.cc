// -*- C++ -*-

// Cov2CorrVariable.cc
//
// Copyright (C) 2008 Pascal Vincent
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

/*! \file Cov2CorrVariable.cc */

#include "Cov2CorrVariable.h"
//#include "Var_operators.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** Cov2CorrVariable **/

PLEARN_IMPLEMENT_OBJECT(
        Cov2CorrVariable,
        "Convert a covariance matrix to a correlation matrix",
        "i.e. it divides off-diagonal term A_ij by sqrt(A_ii A_jj + epsilon).\n"
        "Diagonal terms are set according to option diaognal_choice\n"
);

////////////////////
// Cov2CorrVariable //
////////////////////

Cov2CorrVariable::Cov2CorrVariable():
    diagonal_choice(1),
    epsilon(0.)
{}

Cov2CorrVariable::Cov2CorrVariable(Variable* input, int diagonal_choice_,
                                   double epsilon_, bool call_build_):
    inherited(input, input->length(), input->width(), call_build_),
    diagonal_choice(diagonal_choice_),
    epsilon(epsilon_)
{
    if (call_build_)
        build_();
}

void Cov2CorrVariable::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "diagonal_choice", &Cov2CorrVariable::diagonal_choice, OptionBase::buildoption,
        "Controls how to fill the diagonal.\n"
        "  0: fill it with 0\n"
        "  1: fill it with 1\n"
        "  2: keep the diagonal of the input (i.e. the original variances)\n");
    declareOption(
        ol, "epsilon", &Cov2CorrVariable::epsilon, OptionBase::buildoption,
        "value to add to product of variances before taking their sqrt.\n");
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void Cov2CorrVariable::build() {
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void Cov2CorrVariable::build_() {
    // Nothing to do here.
}

///////////////////
// recomputeSize //
///////////////////
void Cov2CorrVariable::recomputeSize(int& l, int& w) const
{
    if (input) {
        l = input->length();
        w = input->width();
    } else
        l = w = 0;
}


void Cov2CorrVariable::fprop()
{
    Mat C = input->matValue;
    int l = C.length();
    int w = C.width();
    for(int i=0; i<l; i++)
        for(int j=0; j<w; j++)
        {
            if(i!=j)
                matValue(i,j) = C(i,j)/sqrt(C(i,i)*C(j,j)+epsilon);
            else // diagonal element
            {
                double diagval = 0;
                switch(diagonal_choice)
                {
                case 0:
                    diagval = 0;
                    break;
                case 1:
                    diagval = 1;
                    break;
                case 2:
                    diagval = C(i,j);
                    break;
                default:
                    PLERROR("Invalid diagonal_choice option");
                }
                matValue(i,j) = diagval;
            }
        }
}

/*
  Let C the covariance matrix and D the correlation matrix.
  D_ij = C_ij (C_ii C_jj + epsilon)^(-1/2)

  dL/dC_ij = \sum_i'j' dL/dD_i'j' dD_i'j'/dC_ij
 
  case i!=j:
     dL/dC_ij = dL/dD_ij dD_ij/dC_ij
              = dL/dD_ij (C_ii C_jj + epsilon)^(-1/2)
              = dL/dD_ij D_ij/C_ij

  case i==j

    D_ik = C_ik (C_ii C_kk + epsilon)^(-1/2)
    dD_ik/dC_ii = C_ik (-1/2) C_kk (C_ii C_kk + epsilon)^(-3/2)
                = -1/2 C_kk D_ik / (C_ii C_kk + epsilon)
    D_ki = C_ki (C_kk C_ii + epsilon)^(-1/2)
    dD_ki/dC_ii = C_ki (-1/2) C_kk (C_kk C_ii + epsilon)^(-3/2)
                = -1/2 C_kk D_ki / (C_ii C_kk + epsilon)

     dL/dC_ii = \sum_{k!=i}{   dL/dD_ik dD_ik/dC_ii 
                             + dL/dD_ki dD_ki/dC_ii }
              = \sum_{k!=i}{   dL/dD_ik [ -1/2 C_kk D_ik/(C_ii C_kk + epsilon) ]
                             + dL/dD_ki [ -1/2 C_kk D_ki/(C_ii C_kk + epsilon) ]

*/

void Cov2CorrVariable::bprop()
{
    Mat C = input->matValue;
    Mat Cg = input->matGradient;
    Mat D = matValue;
    Mat Dg = matGradient;
    int l = C.length();
    int w = C.width();
    for(int i=0; i<l; i++)
        for(int j=0; j<w; j++)
        {
            if(i!=j)
            {
                Cg(i,j) += Dg(i,j)*D(i,j)/C(i,j);
                double h = -0.5*Dg(i,j)*D(i,j)/(C(i,i)*C(j,j)+epsilon);
                Cg(i,i) += C(j,j)*h;
                Cg(j,j) += C(i,i)*h;
            }
            else if(diagonal_choice==2)
                Cg(i,i) += Dg(i,i);
        }
}


void Cov2CorrVariable::bbprop()
{
    PLERROR("bbprop not implemented for this variable");
}


void Cov2CorrVariable::symbolicBprop()
{
    PLERROR("symbolic bprop not yet implemented for this variable");
}


void Cov2CorrVariable::rfprop()
{
    PLERROR("rfprop no yet implemented for this vairable");
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
