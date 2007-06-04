// -*- C++ -*-

// DoubleProductVariable.cc
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

/*! \file DoubleProductVariable.cc */


#include "DoubleProductVariable.h"

namespace PLearn {
using namespace std;

/** DoubleProductVariable **/

PLEARN_IMPLEMENT_OBJECT(
    DoubleProductVariable,
    "ONE LINE USER DESCRIPTION",
    "Let X,W and M be the inputs, nw the length of W and d their width (they all have the same width)"
    " \nThen output(n,i+j*nw) = sum_k{X(n,k)*W(i,k)*M(j,k)}");

DoubleProductVariable::DoubleProductVariable(Var& x, Var& w, Var& m)
    : inherited(x & w & m, x.length(), w.length()*m.length())
{
    build_();
}


void DoubleProductVariable::recomputeSize(int& l, int& w) const
{   
        if (varray.size() > 0) {
            l = varray[0].length() ; // the computed length of this Var
            w = varray[1].length()*varray[2].length(); // the computed width
        } else
            l = w = 0;    
}

// ### computes value from varray values
void DoubleProductVariable::fprop()
{
    Mat x = varray[0]->matValue,
        w = varray[1]->matValue,
        m = varray[2]->matValue;

    int nx = x.length(),
        nw = w.length(),
        nm = m.length(),
        d = x.width();// ( = w.width() = m.width() )


    for(int n=0; n<nx; n++)
        for(int i=0; i<nw; i++)        
            for(int j=0; j<nm; j++)
            {
                matValue(n,i+j*nw) = 0.;
                for(int k=0; k<d; k++)
                    matValue(n,i+j*nw) += x(n,k)*w(i,k)*m(j,k);
            }
}
// ### computes varray gradients from gradient
void DoubleProductVariable::bprop()
{
    Mat x = varray[0]->matValue,
        w = varray[1]->matValue,
        m = varray[2]->matValue,
        x_grad = varray[0]->matGradient,
        w_grad = varray[1]->matGradient,
        m_grad = varray[2]->matGradient,
        s_grad = matGradient;

    int nx = x.length(),
        nw = w.length(),
        nm = m.length(),
        d = x.width();// ( = w.width()= m.width() )

    
    for(int n=0; n<nx; n++)
        for(int i=0 ;i<nw; i++)
            for(int j=0; j<nm; j++)
            {
                for(int k=0; k<d; k++)
                {             
                    x_grad(n,k) += s_grad(n,i+j*nw)*w(i,k)*m(j,k);
                    w_grad(i,k) += s_grad(n,i+j*nw)*x(n,k)*m(j,k);
                    m_grad(j,k) += s_grad(n,i+j*nw)*x(n,k)*w(i,k);
                }                
            }
}
// ### You can implement these methods:
// void DoubleProductVariable::bbprop() {}
// void DoubleProductVariable::symbolicBprop() {}
// void DoubleProductVariable::rfprop() {}


// ### Nothing to add here, simply calls build_
void DoubleProductVariable::build()
{
    inherited::build();
    build_();
}

void DoubleProductVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### If you want to deepCopy a Var field:
    // varDeepCopyField(somevariable, copies);
}

void DoubleProductVariable::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &DoubleProductVariable::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DoubleProductVariable::build_()
{
    // ### This method should do the real buildincg of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.
   
    if (varW().width() != varX().width() || varW().width() != varM().width())
        PLERROR("All input matrix widths must be equal in DoubleProductVariable");
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
