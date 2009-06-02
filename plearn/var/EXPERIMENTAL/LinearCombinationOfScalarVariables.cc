// -*- C++ -*-

// LinearCombinationOfScalarVariables.cc
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

/*! \file LinearCombinationOfScalarVariables.cc */


#include "LinearCombinationOfScalarVariables.h"

namespace PLearn {
using namespace std;

/** LinearCombinationOfScalarVariables **/

PLEARN_IMPLEMENT_OBJECT(
    LinearCombinationOfScalarVariables,
    "Computes a linear combination of scalar variables",
    "Given an varray of input variables, computes sum_k coefs[k]*varray[k]->value[0] \n"
    "i.e. you may also include non-scalar variables, but only their first element will be used \n"
    );

LinearCombinationOfScalarVariables::LinearCombinationOfScalarVariables()
    :constant_term(0),
     appendinputs(false)
{}

// constructors from input variables.
// NaryVariable constructor (inherited) takes a VarArray as argument.
// You can either construct from a VarArray (if the number of parent Var is not
// fixed, for instance), or construct a VarArray from Var by operator &:
// input1 & input2 & input3. You can also do both, uncomment what you prefer.

// LinearCombinationOfScalarVariables::LinearCombinationOfScalarVariables(const VarArray& vararray)
// ### replace with actual parameters
//  : inherited(vararray, this_variable_length, this_variable_width),
//    parameter(default_value),
//    ...
// {
//     // ### You may (or not) want to call build_() to finish building the
//     // ### object
// }

// LinearCombinationOfScalarVariables::LinearCombinationOfScalarVariables(Var input1, Var input2,
//                            Var input3, ...)
// ### replace with actual parameters
//  : inherited(input1 & input2 & input3 & ...,
//              this_variable_length, this_variable_width),
//    parameter(default_value),
//    ...
// {
//     // ### You may (or not) want to call build_() to finish building the
//     // ### object
// }

// constructor from input variable and parameters
LinearCombinationOfScalarVariables::LinearCombinationOfScalarVariables(const VarArray& vararray,
                                                                       Vec the_coefs)
    : inherited(vararray, 1, appendinputs?(1+vararray.length()):1), 
      coefs(the_coefs), constant_term(0), appendinputs(false)
{
    build_();
}

// constructor from input variable and parameters
// LinearCombinationOfScalarVariables::LinearCombinationOfScalarVariables(Var input1, Var input2,
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

void LinearCombinationOfScalarVariables::recomputeSize(int& l, int& w) const
{
    l = 1;
    if(!appendinputs)
        w = 1;
    else
        w = 1+varray.length();
}

// ### computes value from varray values
void LinearCombinationOfScalarVariables::fprop()
{
    real res = constant_term;
    int n = varray.length();
    if(appendinputs)
    {
        for(int k=0; k<n; k++)
        {
            real val = varray[k]->valuedata[0];
            valuedata[1+k] = val;
            res += coefs[k]*val;
        }
    }
    else
    {
        for(int k=0; k<n; k++)
            res += coefs[k]*varray[k]->valuedata[0];
    }
    valuedata[0] = res;
}

// ### computes varray gradients from gradient
void LinearCombinationOfScalarVariables::bprop()
{
    real gr = gradientdata[0];
    for(int k=0; k<varray.length(); k++)
        varray[k]->gradientdata[0] += gr*coefs[k];
}

// ### You can implement these methods:
// void LinearCombinationOfScalarVariables::bbprop() {}
// void LinearCombinationOfScalarVariables::symbolicBprop() {}
// void LinearCombinationOfScalarVariables::rfprop() {}


// ### Nothing to add here, simply calls build_
void LinearCombinationOfScalarVariables::build()
{
    inherited::build();
    build_();
}

void LinearCombinationOfScalarVariables::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(coefs, copies);
}

void LinearCombinationOfScalarVariables::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "coefs", &LinearCombinationOfScalarVariables::coefs,
                  OptionBase::buildoption,
                  "The vector of coefficients for the linear combination (weights for the sum)");

    declareOption(ol, "constant_term", &LinearCombinationOfScalarVariables::constant_term,
                  OptionBase::buildoption,
                  "A constant term whose valiue will be added to the linear combination");

    declareOption(ol, "appendinputs", &LinearCombinationOfScalarVariables::appendinputs,
                  OptionBase::buildoption,
                  "If true, the values of the first element of all input variables \n"
                  "(with which we compute the weighted sum) will be appended in the \n"
                  "produced output vector (just after their weighted sum). \n"
                  "Note: gradient received on those appended elements is ignored (i.e. not backpropagated).\n"
                  "  these are only useful as extra information (a decomposition of the cost) for reporting purpose");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void LinearCombinationOfScalarVariables::build_()
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
