// -*- C++ -*-

// ARDBaseKernel.cc
//
// Copyright (C) 2006 Nicolas Chapados
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

// Authors: Nicolas Chapados

/*! \file ARDBaseKernel.cc */


#include "ARDBaseKernel.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ARDBaseKernel,
    "Base class for kernels that carry out Automatic Relevance Determination (ARD)",
    "The purpose of this Kernel is to introduce utility options that are\n"
    "generally shared by Kernels that perform Automatic Relevance Determination\n"
    "(ARD).  It does not introduce any specific behavior related to those\n"
    "options (since exactly where the ARD hyperparameters are used is very\n"
    "kernel-specific, this is left to derived classes).\n"
    "\n"
    "Note that to make its operations more robust when used with unconstrained\n"
    "optimization of hyperparameters, all hyperparameters of this kernel are\n"
    "specified in the inverse softplus domain.  See IIDNoiseKernel for more\n"
    "explanations.\n"
    );

ARDBaseKernel::ARDBaseKernel()
    : m_isp_signal_sigma(0.0),
      m_isp_global_sigma(0.0)
{ }


//#####  declareOptions  ######################################################

void ARDBaseKernel::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "isp_signal_sigma",
        &ARDBaseKernel::m_isp_signal_sigma,
        OptionBase::buildoption,
        "Inverse softplus of the global signal variance.  Default value=0.0");

    declareOption(
        ol, "isp_global_sigma",
        &ARDBaseKernel::m_isp_global_sigma,
        OptionBase::buildoption,
        "Inverse softplus of the global length-scale.  Note that if ARD is\n"
        "performed on input-specific sigmas, this hyperparameter should have a\n"
        "fixed value (and not be varied during the optimization).  Default\n"
        "value=0.0.\n");

    declareOption(
        ol, "isp_input_sigma",
        &ARDBaseKernel::m_isp_input_sigma,
        OptionBase::buildoption,
        "If specified, contain input-specific length-scales that can be\n"
        "individually optimized for (these are the ARD hyperparameters).\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


void ARDBaseKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}


void ARDBaseKernel::build_()
{ }


void ARDBaseKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(m_isp_input_sigma, copies);
    deepCopyField(m_input_sigma,     copies);
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
