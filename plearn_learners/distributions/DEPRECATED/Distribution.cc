

// -*- C++ -*-

// Distribution.cc
// 
// Copyright (C) *YEAR* *AUTHOR(S)* 
// ...
// Copyright (C) *YEAR* *AUTHOR(S)* 
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

/*! \file PLearn/plearn_learners/distributions/DEPRECATED/Distribution.cc */
#include "Distribution.h"
#include <plearn/ker/NegOutputCostFunction.h>

namespace PLearn {
using namespace std;

Distribution::Distribution() 
    :Learner(0,1,1), use_returns_what("l")
{
    // cost function is -log_density
    setTestCostFunctions(neg_output_costfunc());
}

PLEARN_IMPLEMENT_OBJECT(Distribution,
                        "This class is deprecated, use PDistribution instead.",
                        "NO HELP");

void Distribution::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave
  
    declareOption(ol, "use_returns_what", &Distribution::use_returns_what, OptionBase::buildoption,
                  "A string where the characters have the following meaning: \n"
                  "'l'-> log_density, 'd' -> density, 'c' -> cdf, 's' -> survival_fn, 'e' -> expectation, 'v' -> variance");
  
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void Distribution::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.

    outputsize_ = use_returns_what.length();
}

// ### Nothing to add here, simply calls build_
void Distribution::build()
{
    inherited::build();
    build_();
}


void Distribution::train(VMat training_set)
{ 
    if(training_set->width() != inputsize()+targetsize())
        PLERROR("In Distribution::train(VMat training_set) training_set->width() != inputsize()+targetsize()");
  
    setTrainingSet(training_set);
  
    // ### Please implement the actual training of the model.
    // ### For models with incremental training, to benefit 
    // ### from the "testing during training" and early-stopping 
    // ### mechanisms, you should make sure to call measure at 
    // ### every "epoch" (whatever epoch means for your algorithm).
    // ### ex:
    // if(measure(epoch,costvec)) 
    //     break; // exit training loop because early-stopping contditions were met
}

void Distribution::use(const Vec& input, Vec& output)
{
    int l = (int)use_returns_what.length();
    for(int i=0; i<l; i++)
    {
        switch(use_returns_what[i])
        {
        case 'l':
            output[i] = (real) log_density(input);
            break;
        case 'd':
            output[i] = (real) density(input);
            break;
        case 'c':
            output[i] = (real) cdf(input);
            break;
        case 's':
            output[i] = (real) survival_fn(input);
            break;
        default:
            PLERROR("In Distribution::use unknown use_returns_what character");
        }
    }
}

void Distribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    Learner::makeDeepCopyFromShallowCopy(copies);
}

double Distribution::log_density(const Vec& x) const
{ PLERROR("density not implemented for this Distribution"); return 0; }

double Distribution::density(const Vec& x) const
{ return exp(log_density(x)); }
  
double Distribution::survival_fn(const Vec& x) const
{ PLERROR("survival_fn not implemented for this Distribution"); return 0; }

double Distribution::cdf(const Vec& x) const
{ PLERROR("cdf not implemented for this Distribution"); return 0; }

Vec Distribution::expectation() const
{ PLERROR("expectation not implemented for this Distribution"); return Vec(); }

Mat Distribution::variance() const
{ PLERROR("variance not implemented for this Distribution"); return Mat(); }

void Distribution::generate(Vec& x) const
{ PLERROR("generate not implemented for this Distribution"); }


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
