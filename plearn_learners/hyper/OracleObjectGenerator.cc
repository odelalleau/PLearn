// -*- C++ -*-

// OracleObjectGenerator.cc
//
// Copyright (C) 2004 ApSTAT Technologies Inc.
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

// Authors: Rejean Ducharme
/* *******************************************************
 * $Id$
 ******************************************************* */

#include "OracleObjectGenerator.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(OracleObjectGenerator, "Generates several variants of an object from varying options returned by an OptionsOracle.",
                        "The OracleObjectGenerator implements the object-generation technique \n"
                        "using an oracle (subclass of OptionsOracle) to produce a new list of parameters. \n");


OracleObjectGenerator::OracleObjectGenerator()
{}

void OracleObjectGenerator::build_()
{
    if (oracle.isNull())
        PLERROR("An OracleObjectGenerator MUST contain an oracle (an OptionsOracle).");

    oracle->build();
    last_params.resize(0);
}

void OracleObjectGenerator::build()
{
    inherited::build();
    build_();
}

void OracleObjectGenerator::forget()
{
    inherited::forget();
    oracle->forget();
    last_params.resize(0);
}

void OracleObjectGenerator::declareOptions(OptionList& ol)
{
    declareOption(ol, "oracle", &OracleObjectGenerator::oracle,
                  OptionBase::buildoption, "The OptionsOracle used to generate the new Object parameters. \n");

    declareOption(ol,"last_params", &OracleObjectGenerator::last_params,
                  OptionBase::learntoption,
                  "The last parameter returned by the oracle. \n");

    inherited::declareOptions(ol);
}

PP<Object> OracleObjectGenerator::generateNextObject()
{
    PP<Object> next_obj;

    TVec<string> new_params = generation_began ? oracle->generateNextTrial(last_params, FLT_MAX) : oracle->generateFirstTrial();

    if (new_params.size() > 0)
    {
        CopiesMap copies;
        next_obj = template_object->deepCopy(copies);
        TVec<string> option_names = oracle->getOptionNames();
        for (int i=0; i<option_names.size(); i++)
        {
            next_obj->setOption(option_names[i], new_params[i]);
        }
        next_obj->build();
    }

    last_params = new_params;
    generation_began = true;

    return next_obj;
}

void OracleObjectGenerator::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(last_params, copies);
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
