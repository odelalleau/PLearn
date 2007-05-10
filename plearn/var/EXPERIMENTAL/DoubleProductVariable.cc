// -*- C++ -*-

// DoubleProductVariable.cc
//
// Copyright (C) 2007 Simon Lemieux
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

// Authors: Simon Lemieux

/*! \file DoubleProductVariable.cc */


#include "DoubleProductVariable.h"

namespace PLearn {
using namespace std;

/** DoubleProductVariable **/

PLEARN_IMPLEMENT_OBJECT(
    DoubleProductVariable,
    "ONE LINE USER DESCRIPTION",
    "MULTI LINE\nHELP FOR USERS"
    );

//DoubleProductVariable::DoubleProductVariable()
    /* ### Initialize all fields to their default value */
//{
    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
//}

// constructors from input variables.
// NaryVariable constructor (inherited) takes a VarArray as argument.
// You can either construct from a VarArray (if the number of parent Var is not
// fixed, for instance), or construct a VarArray from Var by operator &:
// input1 & input2 & input3. You can also do both, uncomment what you prefer.

// DoubleProductVariable::DoubleProductVariable(const VarArray& vararray)
// ### replace with actual parameters
//  : inherited(vararray, this_variable_length, this_variable_width),
//    parameter(default_value),
//    ...
// {
//     // ### You may (or not) want to call build_() to finish building the
//     // ### object
// }

DoubleProductVariable::DoubleProductVariable(Var& x, Var& w, Var& m)
// ### replace with actual parameters
    : inherited(x & w & m, x.length(), w.length()*m.length())
//    parameter(default_value),
//    ...
{
    // ### You may (or not) want to call build_() to finish building the
    // ### object
}

// constructor from input variable and parameters
// DoubleProductVariable::DoubleProductVariable(const VarArray& vararray
//                            param_type the_parameter, ...)
// ### replace with actual parameters
//  : inherited(vararray, this_variable_length, this_variable_width),
//    parameter(the_parameter),
//    ...
// {
//     // ### You may (or not) want to call build_() to finish building the
//     // ### object
// }

// constructor from input variable and parameters
// DoubleProductVariable::DoubleProductVariable(Var input1, Var input2,
//                            Var input3, ...,
//                            param_type the_parameter, ...)
// ### replace with actual parameters
//  : inherited(input1 & input2 & input3 & ...,
//              this_variable_length, this_variable_width),
//    parameter(the_parameter),
//    ...
// {
//     // ### You may (or not) want to call build_() to finish building the
//     // ### object
// }


//TODO : VÉRIFIER QUE CEST OK
void DoubleProductVariable::recomputeSize(int& l, int& w) const
{
    // ### usual code to put here is:
    
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
//TODO :vérifier que cest bien nw et pas nm
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
                //on est à S[n, i+j*nw]
                for(int k=0; k<d; k++)
                {
                    //TODO : vérifier que cest bien nw et pas nw dans la(les) ligne(s) suivante(s)
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

    // ### Remove this line when you have fully implemented this method.
    PLERROR("DoubleProductVariable::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
