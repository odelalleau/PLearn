// -*- C++ -*-

// VerifyGradientCommand.cc
//
// Copyright (C) 2008 Pascal Vincent
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

/*! \file VerifyGradientCommand.cc */


#include "VerifyGradientCommand.h"
#include <plearn/var/SourceVariable.h>
#include <plearn/var/Func.h>
#include <plearn/math/PRandom.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'VerifyGradientCommand' command in the command registry
PLearnCommandRegistry VerifyGradientCommand::reg_(new VerifyGradientCommand);

VerifyGradientCommand::VerifyGradientCommand()
    : PLearnCommand(
        "verify_gradient",
        "Verifies the gradient computation in a Variable through finite differences.",
        "verify_gradient 'MyVariable()' stepsize which_component input1_length input1_width input1_min input1_max input2_length input2_width input2_min input2_max \n"
        "The variable constructor can take option=value arguments if needed.\n"
        "stepsize is the size of the step taken for finite difference\n"
        "which_component specifies which element of MyVariable we feed gradients in (if MyVariable is not a scalar for ex.)\n"
        "if which_component is -1, then all elements will be fed gradient (same as considering gradient to the sum of all elements)\n"
        "input1_... and input2_... are required if the variable is a unary resp. binary variable.\n"
        "SourceVariables input1 and input2 of the specified size will be constructed \n"
        "and randomly initialized with a uniform between corresponding min and max.\n"
        )
{}

//! The actual implementation of the 'VerifyGradientCommand' command
void VerifyGradientCommand::run(const vector<string>& args)
{
    if(args.size()<2)
        PLERROR("Not enough arguments provided");
    string varspec = args[0];
    Object* obj = newObject(varspec);
    Variable* varp = dynamic_cast<Variable*>(obj);
    if(!varp)
        PLERROR("The object you specified does not seem to be a known Variable (is it spelled correctly, did your executable link with it?)");
    Var var = varp;

    double stepsize = todouble(args[1]);
    int which_component = toint(args[2]);

    unsigned int nparents = (args.size()-3)/4;
    if(3+nparents*4 != args.size())
        PLERROR("You must specify 4 numbers (length, width, min, max) for each of the parents variable");

    VarArray parents;
    for(unsigned int k=0; k<nparents; k++)
    {
        int l = toint(args[3+k*4]);
        int w = toint(args[3+k*4+1]);
        double mi = todouble(args[3+k*4+2]);
        double ma = todouble(args[3+k*4+3]);
        perr << "Params: " << l << " " << w << " " << mi << " " << ma << endl;
        SourceVariable* sourcevar = new SourceVariable(l, w, "uniform", mi, ma);
        sourcevar->randomInitialize(PRandom::common());
        Var inputk = sourcevar;
        parents.append(inputk);
    }
    var->setParents(parents);
    var->build();

    pout << "parents:" << *parents[0] << endl;
    Func f(parents, var);
    pout << "func inputs:" << *(f->inputs[0]) << endl;

    f->verifyGradient(stepsize, which_component);
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
