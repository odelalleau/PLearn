// -*- C++ -*-

// SoftSoftMaxVariable.cc
//
// Copyright (C) 2007 Simon Lemieux, Pascal Vincent
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

// Authors: Simon Lemieux, Pascal Vincent

/*! \file SoftSoftMaxVariable.cc */


#include "SoftSoftMaxVariable.h"

namespace PLearn {
using namespace std;

/** SoftSoftMaxVariable **/

PLEARN_IMPLEMENT_OBJECT(
    SoftSoftMaxVariable,
    "Kind of softmax variable",
    "Let X:=input1, A:=input2\nThen output(n,k) = exp(X(n,k))/(sum_j{exp[X(n,j)+A(k,j)]})"
    );


/*
All matrices must be contiguous space storage.
X is a (n,d) matrix
U is a (d,d) matrix
out is a (n,d) matrix

Beware: You must ensure that U_kk = 0 prior to calling these functions, as they assume this is true

*/

// Provided are two versions: a twopass version and asinglepass version. 
// The twopass version does a first pass to find the max (no transcendental involved), and a second pass where it calls a single transcendental (exp), 
// The singlepass version repeatedly calls scalar logadd which means two transcendentals exp and log (possibly yields a numerically more accurate result).
// I don't know which is faster.


#define SOFTSOFTMAX_SAFELOG safelog
#define SOFTSOFTMAX_EXP exp
#define SOFTSOFTMAX_SAFEEXP safeexp
#define SOFTSOFTMAX_LOGADD(a,b) ( ((a)>(b)) ? (a)+log1p(exp((b)-(a))) : (b)+log1p(exp((a)-(b))) )
// #define SOFTSOFTMAX_LOGADD(a,b) ( ((a)>(b)) ? (a)+softplus((b)-(a)) : (b)+softplus((a)-(b)) )
// #define SOFTSOFTMAX_LOGADD(a,b) logadd(a,b)


// Singlepass version does a stable logadd computation by repeatedly calling scalar logadd (as in normal reduction)
void softsoftmax_fprop_singlepass_version(int n, int d, 
                                          const real* __restrict__ const X, 
                                          const real* __restrict__ const U, 
                                          real* __restrict__ const H)
{
  int Hpos = 0;
  int xistart = 0;
  for(int i=0; i<n; i++, xistart+=d)
    {

      int upos  = 0;
      for(int j=0; j<d; j++)
        {
          real Xij = X[xistart+j];

          real res = X[xistart] + U[upos++] - Xij;
          for(int xpos=xistart+1; xpos<xistart+d; xpos++, upos++)
            {
              real newelem = X[xpos] + U[upos] - Xij;
              res = SOFTSOFTMAX_LOGADD(res,newelem);
            }

          H[Hpos++] = SOFTSOFTMAX_SAFEEXP(-res);
        }
    }
}

// Twopass version does a stable logadd computation by first finding the max
void softsoftmax_fprop_twopass_version(int n, int d, 
                                       const real* __restrict__ const X, 
                                       const real* __restrict__ const U, 
                                       real* __restrict__ const H)
{
  int Hpos = 0;
  int xistart = 0;
  for(int i=0; i<n; i++, xistart+=d)
    {
      int uposstart  = 0;
      for(int j=0; j<d; j++, uposstart+=d)
        {
          real maxelem = X[xistart] + U[uposstart];
          for(int xpos=xistart+1, upos=uposstart+1; xpos<xistart+d; xpos++, upos++)
            {
              real elem = X[xpos] + U[upos];
              if(elem>maxelem)
                maxelem = elem;
            }
          real res = 0;
          for(int xpos=xistart, upos=uposstart; xpos<xistart+d; xpos++, upos++)
            res += SOFTSOFTMAX_EXP(X[xpos] + U[upos] - maxelem);
          res = maxelem + SOFTSOFTMAX_SAFELOG(res) - X[xistart+j];

          H[Hpos++] = SOFTSOFTMAX_SAFEEXP(-res);
        }
    }
}

// Twopass version does a stable logadd computation by first finding the max
void softsoftmax_with_log_twopass_version(int n, int d, 
                                          const real* __restrict__ const X, 
                                          const real* __restrict__ const U, 
                                          real* __restrict__ const logH,
                                          real* __restrict__ const H)
{
  int Hpos = 0;
  int xistart = 0;
  for(int i=0; i<n; i++, xistart+=d)
    {
      int uposstart  = 0;
      for(int j=0; j<d; j++, uposstart+=d)
        {
          real maxelem = X[xistart] + U[uposstart];
          for(int xpos=xistart+1, upos=uposstart+1; xpos<xistart+d; xpos++, upos++)
            {
              real elem = X[xpos] + U[upos];
              if(elem>maxelem)
                maxelem = elem;
            }
          real res = 0;
          for(int xpos=xistart, upos=uposstart; xpos<xistart+d; xpos++, upos++)
            res += SOFTSOFTMAX_EXP(X[xpos] + U[upos] - maxelem);
          res = -(maxelem + SOFTSOFTMAX_SAFELOG(res) - X[xistart+j]);

          logH[Hpos] = res;
          H[Hpos] = SOFTSOFTMAX_SAFEEXP(res);
          Hpos++;
        }
    }
}


// Hardapprox version uses only the max of the denominator terms
void softsoftmax_fprop_hardapprox_version(int n, int d, 
                                          const real* __restrict__ const X, 
                                          const real* __restrict__ const U, 
                                          real* __restrict__ const H)
{
  int Hpos = 0;
  int xistart = 0;
  for(int i=0; i<n; i++, xistart+=d)
    {
      int uposstart  = 0;
      for(int j=0; j<d; j++, uposstart+=d)
        {
          real maxelem = X[xistart] + U[uposstart];
          for(int xpos=xistart+1, upos=uposstart+1; xpos<xistart+d; xpos++, upos++)
            {
              real elem = X[xpos] + U[upos];
              if(elem>maxelem)
                maxelem = elem;
            }
          H[Hpos++] = SOFTSOFTMAX_SAFEEXP(X[xistart+j]-maxelem);
        }
    }
}


void softsoftmax_bprop(int n, int d, 
                       const real* __restrict__ const X, 
                       const real* __restrict__ const U, 
                       const real* __restrict__ const logH,
                       const real* __restrict__ const H_gr,
                       real* __restrict__ const X_gr,
                       real* __restrict__ const U_gr)
{
    // Beware: must be passed logH and H_gr, where H_gr is the gradient on H, not on logH. 

    // note: X, logH, H_gr, X_gr  all have the same shape (n,d) 
    // Offset positions will be the same for these matrices, so we wont prefix the variable holding offset positions for these.
    // However, variable indicating offset positions kj in U and U_gr (which are (d,d) matrices) will be called Ukj_pos.


    for(int i=0, row_i_pos=0; i<n; i++, row_i_pos+=d)
    {
        for(int j=0; j<d; j++)
        {          
            int ij = row_i_pos+j; // ij index offset
            real l_ij = logH[ij];
            real h_ij = SOFTSOFTMAX_SAFEEXP(l_ij);
            real sumk = 0;
            for(int k=0, Ukj_pos=j; k<d; k++, Ukj_pos+=d)
            {
                // Ukj_pos = k*d+j;
                int ik = row_i_pos+k; // ik index offset
                real l_ik = logH[ik];
                real val_k = -H_gr[ik]*SOFTSOFTMAX_SAFEEXP(l_ij+l_ik+U[Ukj_pos]);
                if(k!=j)
                    U_gr[Ukj_pos] += val_k;
                sumk += val_k;
            }
            X_gr[ij] += H_gr[ij]*h_ij + sumk; 
        }
    }
}



// constructor from input variables.
SoftSoftMaxVariable::SoftSoftMaxVariable(Variable* input1, Variable* input2)
    : inherited(input1, input2, input1->length(), input1->width())
{
    build_();
}


void SoftSoftMaxVariable::recomputeSize(int& l, int& w) const
{
    // ### usual code to put here is:

        if (input1) {
            l = input1->length(); // the computed length of this Var
            w = input1->width(); // the computed width
        } else
            l = w = 0;
}

// ### computes value from input1 and input2 values
void SoftSoftMaxVariable::fprop()
{
    if(input1->matValue.isNotContiguous() || input2->matValue.isNotContiguous())
        PLERROR("SoftSoftMaxVariable input matrices must be contiguous.");

    int n = input1->matValue.length();
    int d = input1->matValue.width();
    
    if(input2->matValue.length()!=d || input2->matValue.width()!=d)
        PLERROR("SoftSoftMaxVariable second input matriuix (U) must be a square matrix of width and length matching the width of first input matrix");
        
    // make sure U's diagonal is 0
    Mat Umat = input2->matValue;
    for(int i=0; i<d; i++)
        Umat(i,i) = 0;

    const real* const X = input1->matValue.data();
    const real* const U = input2->matValue.data();
    real* const H = matValue.data();
    logH_mat.resize(n,d);
    real* const logH = logH_mat.data();
    
    softsoftmax_with_log_twopass_version(n, d, X, U, logH, H);
    // perr << "Twopass version: " << endl << matValue << endl;
    // softsoftmax_fprop_singlepass_version(n, d, X, U, H);
    // perr << "Singlepass version: " << endl << matValue << endl;
    // perr << "--------------------------------------" << endl;
}

// ### computes input1 and input2 gradients from gradient
void SoftSoftMaxVariable::bprop()
{
    int n = input1->matValue.length();
    int d = input1->matValue.width();
    const real* const X = input1->matValue.data();
    const real* const U = input2->matValue.data();
    // const real* const H = matValue.data();
    // For numerical reasons we use logH that has been computed during fprop and stored, rather than H that is in output->matValue. 
    const real* const logH = logH_mat.data(); 
    
    const real* const H_gr = matGradient.data();
    real* const X_gr = input1->matGradient.data();
    real* const U_gr = input2->matGradient.data();

    softsoftmax_bprop(n, d, X, U, logH, H_gr,  
                      X_gr, U_gr);

    /*
    Mat X = input1->matValue,
        A = input2->matValue,
        grad_X = input1->matGradient,
        grad_A = input2->matGradient;

    real temp;

    //chacun des exemples de X
    for (int n=0; n<X.length(); n++)
        //chaque coordonné dun exemple //correspond au gradient
        for (int k=0; k<X.width(); k++)
            //même exemple, coordonnée aussi // correspond à un exemple
            for (int j=0; j<X.width(); j++)
            {
                temp = matGradient(n,j)*matValue(n,j)*matValue(n,j)*safeexp(X(n,k)+A(j,k))/safeexp(X(n,j));

                if(k==j)
                    grad_X(n,k) += matGradient(n,j)*matValue(n,k)*(1.-matValue(n,k));
                else
                    grad_X(n,k) -= temp;                    
                                                                                            
                grad_A(j,k) -= temp;
            }
    */
}

// ### You can implement these methods:
// void SoftSoftMaxVariable::bbprop() {}
// void SoftSoftMaxVariable::symbolicBprop() {}
// void SoftSoftMaxVariable::rfprop() {}


// ### Nothing to add here, simply calls build_
void SoftSoftMaxVariable::build()
{
    inherited::build();
    build_();
}

void SoftSoftMaxVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### If you want to deepCopy a Var field:
    // varDeepCopyField(somevariable, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("SoftSoftMaxVariable::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void SoftSoftMaxVariable::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &SoftSoftMaxVariable::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void SoftSoftMaxVariable::build_()
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
