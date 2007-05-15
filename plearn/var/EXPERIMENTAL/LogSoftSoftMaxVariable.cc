// -*- C++ -*-

// LogSoftSoftMaxVariable.cc
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

/*! \file LogSoftSoftMaxVariable.cc */


#include "LogSoftSoftMaxVariable.h"

namespace PLearn {
using namespace std;

/** LogSoftSoftMaxVariable **/

PLEARN_IMPLEMENT_OBJECT(
    LogSoftSoftMaxVariable,
    "Kind of softmax variable",
    "Let X:=input1, A:=input2\nThen output(n,k) = X(n,k) - log(sum_j{ exp[ X(n,j) + A(k,j) ] })"
    );


// constructor from input variables.
LogSoftSoftMaxVariable::LogSoftSoftMaxVariable(Variable* input1, Variable* input2)
    : inherited(input1, input2, input1->length(), input1->width())
{
    build_();
}


void LogSoftSoftMaxVariable::recomputeSize(int& l, int& w) const
{
        if (input1) {
            l = input1->length(); // the computed length of this Var
            w = input1->width(); // the computed width
        } else
            l = w = 0;
}

// ### computes value from input1 and input2 values
void LogSoftSoftMaxVariable::fprop()
{
    Mat X = input1->matValue,
        A = input2->matValue;

    real sum;

    for (int n=0; n<X.length(); n++)    
        for (int k=0; k<X.width(); k++)
        {
            sum=0;
            for (int i=0; i<X.width(); i++)
                sum += safeexp(X(n,i) + A(k,i));
            matValue(n,k) = X(n,k) - safelog(sum);
        }    
}

// ### computes input1 and input2 gradients from gradient
void LogSoftSoftMaxVariable::bprop()
{
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
                temp = matGradient(n,j)*safeexp(matValue(n,j) + X(n,k) + A(j,k) - X(n,j));

                if(k==j)
                    grad_X(n,k) += matGradient(n,j)*(1.-safeexp(matValue(n,k)));
                else
                    grad_X(n,k) -= temp;                    
                                                                                            
                grad_A(j,k) -= temp;
            }
}

// ### You can implement these methods:
// void LogSoftSoftMaxVariable::bbprop() {}
// void LogSoftSoftMaxVariable::symbolicBprop() {}
// void LogSoftSoftMaxVariable::rfprop() {}


// ### Nothing to add here, simply calls build_
void LogSoftSoftMaxVariable::build()
{
    inherited::build();
    build_();
}

void LogSoftSoftMaxVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
    PLERROR("LogSoftSoftMaxVariable::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void LogSoftSoftMaxVariable::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &LogSoftSoftMaxVariable::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void LogSoftSoftMaxVariable::build_()
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

    if (input2.length() != input1.width() || input2.width() != input1.width())
        PLERROR("Length and width of input2 must be equal to width of input1 in LogSoftSoftMaxVariable");
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
