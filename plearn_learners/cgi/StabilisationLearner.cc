// -*- C++ -*-

// StabilisationLearner.cc
//
// Copyright (C) 2009 Frederic Bastien
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

// Authors: Frederic Bastien

/*! \file StabilisationLearner.cc */


#include "StabilisationLearner.h"
#include <plearn/io/pl_log.h>
namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    StabilisationLearner,
    "Stabilise the prediction to the old one if the confidence of the new one is under a threshold.",
    "This class is used to don't have example that ping-pong between too"
    " different class prediction. If the new prediction if different from"
    " the old one, we need to have at least the 'thresold' as *confidence*. The"
    " confidence is not well grouned in the theory, but is a good euristic.");

StabilisationLearner::StabilisationLearner()
    :threshold(0)
{
}

void StabilisationLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "threshold", &StabilisationLearner::threshold,
                  OptionBase::buildoption,
                  "The distance needed from 0.5 to accept the change");

    inherited::declareOptions(ol);
}

void StabilisationLearner::build_()
{
}

// ### Nothing to add here, simply calls build_
void StabilisationLearner::build()
{
    inherited::build();
    build_();
}


void StabilisationLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


int StabilisationLearner::outputsize() const
{
    return 1;
}

void StabilisationLearner::forget()
{
    inherited::forget();
}

void StabilisationLearner::train()
{
}


void StabilisationLearner::computeOutput(const Vec& input, Vec& output) const
{
    real pred=int(input[0]);
    real l1=input[1];
    real l2=input[2];
    real old_=int(input[3]);
    real old,ret;
    if(old_==3) old=2;
    else old=old_;

//    if not isNaN(real):     ret = real
    if(old==pred)           ret = pred;
    else if(old==0 and pred==2) ret = 1;
    else if(old==2 and pred==0) ret = 1;
    else if(old==0 and pred==1)
        ret = (l1-threshold)>=0.5;//#(l1-0.5)>threshold
    else if(old==1 and pred==0)
        ret = (l1+threshold)>=0.5;
    else if(old==1 and pred==2)
        ret = ((l2-threshold)>=0.5)+1;
    else if(old==2 and pred==1)
        ret = ((l2+threshold)>=0.5)+1;
    else{
        ret = pred;
        NORMAL_LOG<< "We don't know what to do with old="<<old<<" and pred="<<pred<<endl;    
    }
    output[0]=ret;

}

void StabilisationLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    costs[0]=target[0]!=output[0];
    real old_=int(input[3]);
    real old;
    if(old_==3) old=2;
    else old=old_;
    costs[1]=output[0]!=old;
}

TVec<string> StabilisationLearner::getTestCostNames() const
{
    TVec<string> names;
    names.append("class_error");
    names.append("changed");
    return names;
}

TVec<string> StabilisationLearner::getTrainCostNames() const
{
    TVec<string> names;
    return names;
}

TVec<string> StabilisationLearner::getOutputNames() const
{
    TVec<string> names(1);
    names[0]="SALES_CATEG_STAB";
    return names;
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
