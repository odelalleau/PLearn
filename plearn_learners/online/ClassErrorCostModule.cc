// -*- C++ -*-

// ClassErrorCostModule.cc
//
// Copyright (C) 2007 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file ClassErrorCostModule.cc */



#include "ClassErrorCostModule.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ClassErrorCostModule,
    "Multiclass classification error",
    "If input_size > 1, outputs 0 if target == argmax(input), 1 else\n"
    "If input_size == 1, outputs 0 if target is the closest integer to\n"
    "input[0], 1 else.\n"
    "There is no gradient to compute (it returns an error if you try), so if\n"
    "you use this module inside a CombiningCostsModule, put its weight to 0.\n"
    );

ClassErrorCostModule::ClassErrorCostModule()
{
    output_size = 1;
    target_size = 1;
}

void ClassErrorCostModule::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &ClassErrorCostModule::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ClassErrorCostModule::build_()
{
    PLASSERT( output_size == 1 );
    PLASSERT( target_size == 1 );
}

void ClassErrorCostModule::build()
{
    inherited::build();
    build_();
}


void ClassErrorCostModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(trainvec, copies);
}

///////////
// fprop //
///////////
void ClassErrorCostModule::fprop(const Vec& input, const Vec& target,
                                 Vec& cost) const
{
    cost.resize( output_size );
    fprop( input, target, cost[0] );
}

void ClassErrorCostModule::fprop(const Vec& input, const Vec& target,
                                 real& cost) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );

    if( input_size == 1 ) // is target[0] the closest integer to input[0]?
        cost = ( round(input[0]) == round(target[0]) ) ? 0. : 1.;
    else // is target[0] equals to argmax(input)?
        cost = ( argmax(input) == int(round(target[0])) ) ? 0. : 1.;
}

/////////////////
// bpropUpdate //
/////////////////
void ClassErrorCostModule::bpropUpdate(const Vec& input, const Vec& target,
                                       real cost)
{
}

/* Not supposed to happen
void ClassErrorCostModule::bpropUpdate(const Vec& input, const Vec& target,
                                       real cost,
                                       Vec& input_gradient,
                                       bool accumulate=false)
{
}
*/

//////////////////
// bbpropUpdate //
//////////////////
void ClassErrorCostModule::bbpropUpdate(const Vec& input, const Vec& target,
                                        real cost)
{
}

/* Not supposed to happen
void ClassErrorCostModule::bbpropUpdate(const Vec& input, const Vec& target,
                                       real cost,
                                       Vec& input_gradient,
                                       Vec& input_diag_hessian,
                                       bool accumulate=false)
{
}
*/

////////////
// forget //
////////////
void ClassErrorCostModule::forget()
{
}

//////////
// name //
//////////
TVec<string> ClassErrorCostModule::name()
{
    return TVec<string>(1, "class_error");
}

//////////////////////
// bpropDoesNothing //
//////////////////////
bool ClassErrorCostModule::bpropDoesNothing()
{
    return true;
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
