// -*- C++ -*-

// RBMLayer.cc
//
// Copyright (C) 2006 Pascal Lamblin & Dan Popovici
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

// Authors: Pascal Lamblin & Dan Popovici

/*! \file RBMPLayer.cc */



#include "RBMLayer.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/math/PRandom.h>
#include "RBMParameters.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    RBMLayer,
    "Virtual class for a layer of an RBM",
    "");

RBMLayer::RBMLayer() :
    size(-1),
    expectation_is_up_to_date(false)
{
}

void RBMLayer::reset()
{
    activations.clear();
    sample.clear();
    expectation.clear();
    expectation_is_up_to_date = false;
}

void RBMLayer::declareOptions(OptionList& ol)
{
    declareOption(ol, "units_types", &RBMLayer::units_types,
                  OptionBase::learntoption,
                  "Each character of this string describes the type of an"
                  " up unit:\n"
                  "  - 'l' if the energy function of this unit is linear\n"
                  "    (binomial or multinomial unit),\n"
                  "  - 'q' if it is quadratic (for a gaussian unit).\n");

    declareOption(ol, "random_gen", &RBMLayer::random_gen,
                  OptionBase::buildoption,
                  "Random generator.");

    declareOption(ol, "size", &RBMLayer::size,
                  OptionBase::buildoption,
                  "Numer of units.");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMLayer::build_()
{
    if( size <= 0 )
        return;

    if( !random_gen )
        random_gen = new PRandom();
    random_gen->build();
}

void RBMLayer::build()
{
    inherited::build();
    build_();
}

void RBMLayer::printActivation()
{
    cout << "activations: " << activations << endl;
    cout << "expectation: " << expectation << endl;
}


void RBMLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(random_gen, copies);
    deepCopyField(activations, copies);
    deepCopyField(sample, copies);
    deepCopyField(expectation, copies);
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
