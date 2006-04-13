// -*- C++ -*-

// UnconditionalDistribution.cc
//
// Copyright (C) 2004-2006 Olivier Delalleau 
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

// Authors: Olivier Delalleau

/*! \file UnconditionalDistribution.cc */


#include "UnconditionalDistribution.h"

namespace PLearn {
using namespace std;

///////////////////////////////
// UnconditionalDistribution //
///////////////////////////////
UnconditionalDistribution::UnconditionalDistribution() 
{
    predictor_size = 0;
    predicted_size = -1;
}

PLEARN_IMPLEMENT_OBJECT(UnconditionalDistribution,
    "A simplified version of PDistribution for unconditional distributions.",

    "The only goal of this class is to hide the conditional side of\n"
    "PDistributions to make unconditional distributions simpler."
);

////////////////////
// declareOptions //
////////////////////
void UnconditionalDistribution::declareOptions(OptionList& ol)
{
    // First call the parent class' declareOptions().
    inherited::declareOptions(ol);

    // And modify some options for unconditional distributions.

    redeclareOption(ol, "outputs_def", &UnconditionalDistribution::outputs_def,
                                       OptionBase::buildoption,
        "See help for this option in PDistribution. Basically, this is the\n"
        "same, except that 'E' and 'V' are obviously not allowed.");
    // TODO Find a cool way to synchronize this help with the PDistribution
    // help?

    redeclareOption(ol, "predictor_size",
                        &UnconditionalDistribution::predictor_size,
                        OptionBase::nosave,
        "Unused in unconditional distributions.");

    redeclareOption(ol, "predicted_size",
                        &UnconditionalDistribution::predicted_size,
                        OptionBase::nosave,
        "Unused in unconditional distributions.");

    redeclareOption(ol, "predictor_part",
                        &UnconditionalDistribution::predictor_part,
                        OptionBase::nosave,
        "Unused in unconditional distributions.");

    redeclareOption(ol, "n_predictor",
                        &UnconditionalDistribution::n_predictor,
                        OptionBase::nosave,
        "Unused in unconditional distributions.");
}

///////////
// build //
///////////
void UnconditionalDistribution::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void UnconditionalDistribution::build_()
{
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void UnconditionalDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

//////////////////
// setPredictor //
//////////////////
void setPredictor(const Vec& predictor, bool call_parent) const
{
    PLERROR("In UnconditionalDistribution::setPredictor - Not implemented for "
            "unconditional distributions");
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
