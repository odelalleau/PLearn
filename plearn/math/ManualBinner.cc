// -*- C++ -*-

// ManualBinner.cc
// 
// Copyright (C) 2002 Xavier Saint-Mleux
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

/*! \file ManualBinner.cc */
#include "ManualBinner.h"

namespace PLearn {
using namespace std;

ManualBinner::ManualBinner() 
    :Binner(), the_mapping(0), bin_positions()
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

ManualBinner::ManualBinner(Vec bin_positions_) 
    :Binner(), the_mapping(0), bin_positions(bin_positions_.copy())
{
    build_();
}


PLEARN_IMPLEMENT_OBJECT(ManualBinner, "Binner with predefined cut-points.", 
                        "ManualBinner implements a Binner for which cutpoints are predefined.  "
                        "It's getBinning function doesn't have to look at the data; it simply "
                        "builds a RealMapping from the supplied bin_positions.");

void ManualBinner::declareOptions(OptionList& ol)
{
    declareOption(ol, "bin_positions", &ManualBinner::bin_positions, OptionBase::buildoption,
                  "The supplied cut points; should be sorted in ascending order.");

    declareOption(ol, "the_mapping", &ManualBinner::the_mapping, OptionBase::learntoption,
                  "Pre-calculated RealMapping object that is returned by getBinning");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ManualBinner::build_()
{
    // create the pre-calculated RealMapping from bin_position
    if(the_mapping == 0)
        the_mapping= new RealMapping();
    the_mapping->clear();
    for(int i= 1; i < bin_positions.length(); ++i)
        the_mapping->addMapping(RealRange(']', bin_positions[i-1], bin_positions[i], ']'), i-1);
}

// ### Nothing to add here, simply calls build_
void ManualBinner::build()
{
    inherited::build();
    build_();
}


void ManualBinner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ManualBinner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


//! Returns a binning for a single column vmatrix v 
PP<RealMapping> ManualBinner::getBinning(VMat v) const
{ return the_mapping; }

PP<RealMapping> ManualBinner::getBinning() const
{ return the_mapping; }


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
