// -*- C++ -*-

// BinarizeModule.cc
//
// Copyright (C) 2007 Yoshua Bengio
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

// Authors: Yoshua Bengio

/*! \file BinarizeModule.cc */



#include "BinarizeModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    BinarizeModule,
    "Map probabilities in (0,1) to a bit in {0,1}, by sampling or hard threshold\n",
    "Map input probabilities in (0,1) to a bit in {0,1}, either according to\n"
    "a hard threshold (> 0.5), or by sampling, and ALLOW GRADIENTS\n"
    "TO PROPAGATE BACKWARDS. The heuristic for gradient propagation is the following:\n"
    "  If the output is incorrect (sign of gradient pushes towards the other value)\n"
    "    then propagate gradient as is,\n"
    "    else do not propagate any gradient.\n"
);

//////////////////
// BinarizeModule //
//////////////////
BinarizeModule::BinarizeModule()
    : stochastic(true),copy_gradients(false),saturate_gradients(false)
{
}

////////////////////
// declareOptions //
////////////////////
void BinarizeModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "stochastic", &BinarizeModule::stochastic,
                  OptionBase::buildoption,
                  "If true then sample the output bits stochastically, else use a hard threshold.\n");

    declareOption(ol, "copy_gradients", &BinarizeModule::copy_gradients,
                  OptionBase::buildoption,
                  "If true then simply copy the gradients through with no alteration.\n");

    declareOption(ol, "saturate_gradients", &BinarizeModule::saturate_gradients,
                  OptionBase::buildoption,
                  "If true then multiply output gradients by p(1-p) for input probability p.\n");

    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void BinarizeModule::build_()
{
}

///////////
// build //
///////////
void BinarizeModule::build()
{
    inherited::build();
    build_();
}

////////////////////
// bpropAccUpdate //
////////////////////
void BinarizeModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                    const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_value.length() == nPorts() && ports_gradient.length() == nPorts());

    Mat* input = ports_value[0];
    Mat* output = ports_value[1];
    Mat* input_gradient = ports_gradient[0];
    Mat* output_gradient = ports_gradient[1];
    
    int mbs=output->length();
    if (input_gradient)
    {
        input_gradient->resize(mbs,output->width());
        for (int t=0;t<mbs;t++)
        {
            real* yt = (*output)[t];
            real* dyt = (*output_gradient)[t];
            real* dxt = (*input_gradient)[t];
            real* xt = (*input)[t];
            if (copy_gradients)
                for (int i=0;i<output->width();i++)
                    dxt[i] += dyt[i];
            else if (saturate_gradients)
                for (int i=0;i<output->width();i++)
                    dxt[i] += dyt[i]*xt[i]*(1-xt[i]);
            else for (int i=0;i<output->width();i++)
                if ((yt[i]-0.5)*dyt[i] > 0)
                    dxt[i] += dyt[i];
        }
    }
}

//////////////////////
// bpropDoesNothing //
//////////////////////
/* THIS METHOD IS OPTIONAL
// the default implementation returns false
bool BinarizeModule::bpropDoesNothing()
{
}
*/

//////////////
// finalize //
//////////////
/* THIS METHOD IS OPTIONAL
void BinarizeModule::finalize()
{
}
*/

////////////
// forget //
////////////
void BinarizeModule::forget()
{
}

///////////
// fprop //
///////////
void BinarizeModule::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( ports_value.length() == nPorts() );
    // check which ports are input (ports_value[i] && !ports_value[i]->isEmpty())
    // which ports are output (ports_value[i] && ports_value[i]->isEmpty())
    // and which ports are ignored (!ports_value[i]).
    // If that combination of (input,output,ignored) is feasible by this class
    // then perform the corresponding computation. Otherwise launch the error below.
    // See the comment in the header file for more information.

    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    Mat* input = ports_value[0];
    Mat* output = ports_value[1];
    int mbs=input->length();
    output->resize(mbs,input->width());
    for (int t=0;t<mbs;t++)
    {
        real* xt = (*input)[t];
        real* yt = (*output)[t];
        int w=input->width();
        if (stochastic)
            for (int i=0;i<w;i++)
                yt[i]=random_gen->binomial_sample(xt[i]);
        else
            for (int i=0;i<w;i++)
                yt[i]=xt[i]>=0.5?1:0;
    }
}

//////////////////
// getPortIndex //
//////////////////
/* Optional
int BinarizeModule::getPortIndex(const string& port)
{}
*/

//////////////
// getPorts //
//////////////
const TVec<string>& BinarizeModule::getPorts() {
    return inherited::getPorts();
}

//////////////////
// getPortSizes //
//////////////////
const TMat<int>& BinarizeModule::getPortSizes() {
    port_sizes.resize(2,2);
    port_sizes.fill(-1);
    return port_sizes;
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void BinarizeModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("BinarizeModule::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/////////////////////
// setLearningRate //
/////////////////////
/* OPTIONAL
// The default implementation raises a warning and does not do anything.
void BinarizeModule::setLearningRate(real dynamic_learning_rate)
{
}
*/


}
// end of namespace PLearn


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
