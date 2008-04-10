// -*- C++ -*-

// LogaddOnBagsModule.cc
//
// Copyright (C) 2008 Jerome Louradour
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

// Author: Jerome Louradour

/*! \file LogaddOnBagsModule.cc */



#include "LogaddOnBagsModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    LogaddOnBagsModule,
    "For each input i, on a bag of size N, outputs:"
       " Logadd([ input_i(1), ..., input_i(N) ]).",
    "The 'input' port is typically connected to the output\n"
    "port of class activities (input_size = number of classes).\n"
    "see OnBagsModule for details on bags.\n");

LogaddOnBagsModule::LogaddOnBagsModule()
{
    output_size = -1;
}

void LogaddOnBagsModule::declareOptions(OptionList& ol)
{
    inherited::declareOptions(ol);

    redeclareOption(ol, "output_size", &LogaddOnBagsModule::output_size,
                  OptionBase::learntoption,
                  "Size of the 'output' port (same as 'input').");
}

void LogaddOnBagsModule::build_()
{
    PLASSERT( input_size > 0 );
    output_size = input_size;
    accumulated_output.resize(output_size);
    inherited::build();
}

void LogaddOnBagsModule::build()
{
    inherited::build();
    build_();
}

void LogaddOnBagsModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(accumulated_output, copies);
}

///////////
// fprop //
///////////

void LogaddOnBagsModule::fpropInit(const Vec& input)
{
    accumulated_output << input;
}
void LogaddOnBagsModule::fpropAcc(const Vec& input)
{
    for( int i = 0; i < input_size; i++ )
        accumulated_output[i] = logadd(accumulated_output[i],
                                       input[i]); 
}
void LogaddOnBagsModule::fpropOutput(Vec& output)
{
    output.resize( output_size );
    output << accumulated_output;
}

/////////////////
// bpropUpdate //
/////////////////

void LogaddOnBagsModule::bprop( const Mat& baginputs,
                                const Vec& bagoutput_gradient,
                                Mat& baginputs_gradients)
{
    int nsamples = baginputs.length();
    baginputs_gradients.resize( nsamples, input_size);
    for( int i = 0; i < input_size; i++ )
    {
        Vec tmp_input_gradient;
        tmp_input_gradient.resize( nsamples );
        softmax( baginputs.column(i).toVecCopy() ,
                 tmp_input_gradient );
        tmp_input_gradient *= bagoutput_gradient[i];
        baginputs_gradients.column(i) << tmp_input_gradient;
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
