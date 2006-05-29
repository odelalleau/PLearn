// -*- C++ -*-

// ConjRosenbrock.cc
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

/*! \file ConjRosenbrock.cc */

// From C++ stdlib
#include <iostream>

// From PLearn
#include <plearn/base/lexical_cast.h>
#include <plearn/io/pl_log.h>
#include <plearn/io/openFile.h>

#include <plearn/var/Variable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/var/Func.h>
#include <plearn/var/PowVariable.h>
#include <plearn/var/SubMatVariable.h>
#include <plearn/var/SumVariable.h>

#include <plearn/opt/GradientOptimizer.h>
#include <plearn/opt/ConjGradientOptimizer.h>

#include "ConjRosenbrock.h"

namespace PLearn {
using namespace std;

// This Func returns the evaluation of the Rosenbrock function.
// We assume that the input/output are represented as ROW vectors.
static Func rosenbrock(int D)
{
    // D = length(x);
    // f = sum(100*(x(2:D)-x(1:D-1).^2).^2 + (1-x(1:D-1)).^2);
    Var input(1, D, "input");
    Var drop_first   = subMat(input, 0, 1, 1, D-1);
    Var drop_last    = subMat(input, 0, 0, 1, D-1);
    Var drop_last_sq = pow(drop_last, 2);
    Var diff_100x_sq = pow(drop_first - drop_last_sq, 2) * 100.0;
    Var second_term  = pow(1 - drop_last,2);
    Var rosenbrock   = sum(diff_100x_sq + second_term);

    return Func(VarArray(input), VarArray(rosenbrock));
}

PLEARN_IMPLEMENT_OBJECT(
    ConjRosenbrock,
    "Exercises the Conjugate Gradient optimizer through the Rosenbrock Function.",
    ""
);

//////////////////
// ConjRosenbrock //
//////////////////
ConjRosenbrock::ConjRosenbrock()
    : D(2)
{ }

///////////
// build //
///////////
void ConjRosenbrock::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ConjRosenbrock::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ConjRosenbrock::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void ConjRosenbrock::declareOptions(OptionList& ol)
{
    declareOption(ol, "opt", &ConjRosenbrock::opt, OptionBase::buildoption,
                  "Optimizer to use, with options.");

    declareOption(ol, "D", &ConjRosenbrock::D, OptionBase::buildoption,
                  "Dimensionality of the Rosenbrock problem to solve");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void ConjRosenbrock::build_()
{
    if (! opt)
        PLERROR("%s: the 'opt' option must be specified", __FUNCTION__);
}

/////////////
// perform //
/////////////
void ConjRosenbrock::perform()
{
    // Change verbosity 
    PL_Log::instance().verbosity(VLEVEL_DBG);
    PL_Log::instance().enableNamedLogging(
        "Optimizer,GradientOptimizer,ConjGradientOptimizer");

    // Set up the Rosenbrock problem
    Func r = rosenbrock(D);
    Vec input(D);                        // Initialized to 0.0
    pout << "rosenbrock(" << input << ") = " << r(input) << endl;
    r->verifyGradient(input,1e-6);       // Check gradient at 0^D
    input.fill(1.0);
    r->verifyGradient(input,1e-6);       // Check gradient at 1^D
    // r->verifyGradient(-1,1,1e-6);        // Check gradient at random point

    // Set up the optimizer and go
    r->inputs[0]->value.fill(0.0);
    opt->reset();
    opt->setToOptimize(r->inputs, r->outputs);
    opt->build();

    VecStatsCollector vsc;
    opt->optimizeN(vsc);

    // Print information
    pout << "\nAfter optimization:"
         << "\ninputs = " << r->inputs[0]->value
         << "\noutput = " << r->outputs[0]->value
         << "\n\nOptimization stats collector: " << vsc;
    
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
