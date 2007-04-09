

// -*- C++ -*-

// ConditionalCDFSmoother.cc
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

/*! \file ConditionalCDFSmoother.cc */

#include "ConditionalCDFSmoother.h"

namespace PLearn {
using namespace std;

ConditionalCDFSmoother::ConditionalCDFSmoother() {}

ConditionalCDFSmoother::ConditionalCDFSmoother(PP<HistogramDistribution>& prior_cdf_) 
    :Smoother(), prior_cdf(prior_cdf_)
{}


PLEARN_IMPLEMENT_OBJECT(ConditionalCDFSmoother, 
                        "Smoother that combines a detailed prior curve with a rough input curve.\n",
                        "This smoother is meant to smooth conditional distribution functions, using\n"
                        "a high-resolution prior cdf provided as a HistogramDistribution. Its 'smooth'\n"
                        "function takes a lower-resolution curve and smooths it using the prior\n"
                        "to fill the gaps.");

void ConditionalCDFSmoother::declareOptions(OptionList& ol)
{
    declareOption(ol, "prior_cdf", &ConditionalCDFSmoother::prior_cdf, OptionBase::buildoption,
                  "Prior CDF used to smooth other functions");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ConditionalCDFSmoother::build_()
{
}

void ConditionalCDFSmoother::build()
{
    inherited::build();
    build_();
}

void ConditionalCDFSmoother::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(prior_cdf, copies);
}

real ConditionalCDFSmoother::smooth(const Vec& source_function, Vec& smoothed_function, 
                                    Vec bin_positions, Vec dest_bin_positions) const
{
    PLERROR("smooth not implemented for ConditionalCDFSmoother.");
    return 0.0;
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
