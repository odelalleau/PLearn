// -*- C++ -*-

// VMat_Func_utils.cc
//
// Copyright (C) 2004 Pascal Vincent
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
 * $Id$
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file VMat_Func_utils.cc */


#include "VMat_Func_utils.h"
#include <plearn/var/Func.h>
#include <plearn/vmat/VMat.h>

namespace PLearn {
using namespace std;

// Put function implementations here.
void evaluateSumOfFprop(VMat vm, Func f, Vec& output_result, int nsamples)
{
    //if (f->outputs.size()!=1)
    //  PLERROR("In evaluateSumOfFprop: function must have a single variable output (maybe you can concat the vars into a single one, if this is really what you want)");

    static int curpos = 0;
    int l = vm->length();
    int w = vm->width();
    if (nsamples == -1) nsamples = l;
    Vec input_value(w);
    Vec output_value(output_result.length());

    f->recomputeParents();
    output_result.clear();

    for(int i=0; i<nsamples; i++)
    {
        vm->getRow(curpos++, input_value);
        f->fprop(input_value, output_value);
        output_result += output_value;
        if(curpos == l) curpos = 0;
    }
}

void evaluateSumOfFbprop(VMat vm, Func f, Vec& output_result, Vec& output_gradient, int nsamples)
{
//  if(f->outputs.size()!=1)
    //   PLERROR("In evaluateSumOfFprop: function must have a single variable output (maybe you can concat the vars into a single one, if this is really what you want)");

    static int curpos = 0;
    int l = vm->length();
    int w = vm->width();
    if (nsamples == -1) nsamples = l;
    Vec input_value(w);
    Vec input_gradient(w);
    Vec output_value(output_result.length());

    f->recomputeParents();
    output_result.clear();

    for(int i=0; i<nsamples; i++)
    {
        vm->getRow(curpos++, input_value);
        f->fbprop(input_value, output_value, input_gradient, output_gradient);
        //displayFunction(f, true);
        output_result += output_value;
        if(curpos == l) curpos = 0;
    }
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
