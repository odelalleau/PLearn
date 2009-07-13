// -*- C++ -*-

// SumEntropyOfCategoricals.cc
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

/*! \file SumEntropyOfCategoricals.cc */


#include "SumEntropyOfCategoricals.h"

namespace PLearn {
using namespace std;

/** SumEntropyOfCategoricals **/

PLEARN_IMPLEMENT_OBJECT(
    SumEntropyOfCategoricals,
    "Computes the sum of the entropies of independent categorical variables (multinomials with number-of-trials n=1) whose probability parameters are given as input.",
    "i.e. if input is a matrix P, computes sum_ij - [P_ij log(P_ij)].\n"
    "Note that this formula does not depend on whether the parameters in P are those of a single categorical variable \n"
    "or represent the parameters of several independent categorical variables."
    );

SumEntropyOfCategoricals::SumEntropyOfCategoricals()
    : inherited(0, 1, 1)
{
}

// constructor from input variable.
SumEntropyOfCategoricals::SumEntropyOfCategoricals(Variable* input)
    : inherited(input, 1, 1)
{
    build_();
}

// constructor from input variable and parameters
// SumEntropyOfCategoricals::SumEntropyOfCategoricals(Variable* input, param_type the_parameter,...)
// ### replace with actual parameters
//  : inherited(input, this_variable_length, this_variable_width),
//    parameter(the_parameter),
//    ...
//{
//    // ### You may (or not) want to call build_() to finish building the
//    // ### object
//}

void SumEntropyOfCategoricals::recomputeSize(int& l, int& w) const
{
    l = w = 1;
}

// ### computes value from input's value
void SumEntropyOfCategoricals::fprop()
{
    double res = 0;
    Mat P = input->matValue;
    int l = P.length();
    int w = P.width();

    for(int i=0; i<l; i++)
    {
        const real* P_i = P[i];
        for(int j=0; j<w; j++)
        {
            real P_ij = P_i[j];
            if( P_ij>1e-25 )
                res += P_ij*pl_log(P_ij);
        }
    }
    valuedata[0] = -(real)res;
}

// ### computes input's gradient from gradient
void SumEntropyOfCategoricals::bprop()
{
    /*
      [-p log(p)]' = -[log(p) + 1]
    */
    real gr = gradientdata[0];
    Mat P = input->matValue;
    Mat Pgr = input->matGradient;

    int l = P.length();
    int w = P.width();

    for(int i=0; i<l; i++)
    {
        const real* P_i = P[i];
        real* Pgr_i = Pgr[i];
        for(int j=0; j<w; j++)
        {
            real P_ij = P_i[j];
            real logPij = P_ij>1e-25 ?pl_log(P_ij): -57.5;
            Pgr_i[j] -= gr*(logPij + 1);
        }
    }
}

// ### You can implement these methods:
// void SumEntropyOfCategoricals::bbprop() {}
// void SumEntropyOfCategoricals::symbolicBprop() {}
// void SumEntropyOfCategoricals::rfprop() {}


// ### Nothing to add here, simply calls build_
void SumEntropyOfCategoricals::build()
{
    inherited::build();
    build_();
}

void SumEntropyOfCategoricals::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

void SumEntropyOfCategoricals::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &SumEntropyOfCategoricals::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void SumEntropyOfCategoricals::build_()
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
