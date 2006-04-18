// -*- C++ -*-

// OnlineLearningModule.cc
//
// Copyright (C) 2005 Yoshua Bengio 
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

/* *******************************************************      
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file OnlineLearningModule.cc */


#include "OnlineLearningModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    OnlineLearningModule,
    "Learn to map inputs to outputs, online, using caller-provided gradients.",
    "This pure virtual class (i.e. an interface) can basically do two things:\n"
    "  * map an input to an output\n"
    "  * modify itself when told in what direction the output should have \n"
    "    changed (i.e. output gradient),  while optionally giving back the \n"
    "    information about how the input should also have changed \n"
    "    (i.e. input gradient)\n"
    );

OnlineLearningModule::OnlineLearningModule() :
    input_size(-1),
    output_size(-1),
    estimate_simpler_diag_hessian( false )
{
}

void OnlineLearningModule::bpropUpdate(const Vec& input, const Vec& output,
                                       Vec& input_gradient,
                                       const Vec& output_gradient)
{
    PLERROR("In OnlineLearningModule.cc: method 'bpropUpdate' not"
            " implemented.\n"
            "Please implement it in your derived class or don't call"
            " bpropUpdate.\n");
}

void OnlineLearningModule::bpropUpdate(const Vec& input, const Vec& output,
                                       const Vec& output_gradient)
{
    Vec input_gradient(input.length());
    bpropUpdate(input, output, input_gradient, output_gradient);
}

//! Default method for bbpropUpdate functions, so that it compiles but crashes
//! if not implemented but used.
void OnlineLearningModule::bbpropUpdate(const Vec& input, const Vec& output,
                                        const Vec& output_gradient,
                                        const Vec& output_diag_hessian)
{
    Vec input_gradient(input.length());
    Vec input_diag_hessian(input.length());
    bbpropUpdate(input, output, input_gradient, output_gradient,
                 input_diag_hessian, output_diag_hessian);
}

void OnlineLearningModule::bbpropUpdate(const Vec& input, const Vec& output,
                                        Vec& input_gradient,
                                        const Vec& output_gradient,
                                        Vec& input_diag_hessian,
                                        const Vec& output_diag_hessian)
{
    PLERROR("In OnlineLearningModule.cc: method 'bbpropUpdate' not"
            "implemented.\n"
            "Please implement it in your derived class, or use"
            "'bpropUpdate'.\n");
}


void OnlineLearningModule::build()
{
    inherited::build();
    build_();
}

void OnlineLearningModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

void OnlineLearningModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "input_size", &OnlineLearningModule::input_size,
                  OptionBase::buildoption,
                  "Size of the input");

    declareOption(ol, "output_size", &OnlineLearningModule::output_size,
                  OptionBase::buildoption,
                  "Size of the output");

    declareOption(ol, "estimate_simpler_diag_hessian",
                  &OnlineLearningModule::estimate_simpler_diag_hessian,
                  OptionBase::buildoption,
                  "Should we compute a simpler diagonal estimation of the"
                  " input Hessian\n"
                  "matrix, using only the first (positive) term in:\n"
                  "  d²C/dx² ~= d²C/dy² (dy/dx)² [+ dC/dy d²y/dx²]\n");


    declareOption(ol, "expdir", &OnlineLearningModule::expdir,
                  OptionBase::buildoption, 
                  "Path of the directory associated with this module,\n"
                  "in which it should save any file it wishes to create. \n"
                  "The directory will be created if it does not already"
                  " exist.\n"
                  "If expdir is the empty string (the default),\n"
                  "then the module should not create *any* file.\n");

    declareOption(ol, "random_gen",
                  &OnlineLearningModule::random_gen,
                  OptionBase::buildoption, 
                  "Pointer to an optional random number generator,\n"
                  "e.g. for initializing parameters or any non-deterministic"
                  " operation\n"
                  "required by the module.\n");

    inherited::declareOptions(ol);
}

void OnlineLearningModule::build_()
{
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
