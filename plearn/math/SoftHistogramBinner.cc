// -*- C++ -*-

// SoftHistogramBinner.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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

/*! \file SoftHistogramBinner.cc */


#include "SoftHistogramBinner.h"

namespace PLearn {
using namespace std;

/////////////////////////
// SoftHistogramBinner //
/////////////////////////
SoftHistogramBinner::SoftHistogramBinner() 
/* ### Initialize all fields to their default value */
{
    // ...
    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(SoftHistogramBinner,
                        "Computes bins which may overlap, but contain the same number of elements.",
                        "This binner does not compute a mapping to each bin, because a sample may\n"
                        "appear in different bins. Instead, one should use it directly through the\n"
                        "getBins() method.\n"
                        "The number of bins is given through the 'n_bins' option. The bins split\n"
                        "uniformly the interval [min_val, max_val] defined by the data. Each bin\n"
                        "contains 'samples_per_bin' samples (if this value is >= 1), or this fraction\n"
                        "of the total number of samples (if it is < 1). The value '-1' can be used\n"
                        "to specify that each bin contains all samples (though this is probably\n"
                        "not very useful).\n"
    );

////////////////////
// declareOptions //
////////////////////
void SoftHistogramBinner::declareOptions(OptionList& ol)
{
    declareOption(ol, "samples_per_bin", &SoftHistogramBinner::samples_per_bin, OptionBase::buildoption,
                  "The number of samples in each bin (if >= 1), or as a fraction (if < 1).");

    declareOption(ol, "n_bins", &SoftHistogramBinner::n_bins, OptionBase::buildoption,
                  "The number of bins computed.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build //
////////////
void SoftHistogramBinner::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void SoftHistogramBinner::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
  
    // Check validity of the 'samples_per_bin' option.
    if (!is_equal(samples_per_bin, -1)                 &&
        !(samples_per_bin >= 0 && samples_per_bin < 1) &&
        !(samples_per_bin >= 1 && fabs(samples_per_bin - int(samples_per_bin)) < 1e-8))
        PLERROR("In SoftHistogramBinner::build_ - Invalid value for 'samples_per_bin'");
}

/////////////
// getBins //
/////////////
TVec< TVec<int> > SoftHistogramBinner::getBins(const Vec& v) const {
    // Find out how many samples we have in each bin.
    int n;
    if (is_equal(samples_per_bin, -1))
        n = v.length();
    else if (samples_per_bin < 1)
        n = int(samples_per_bin * v.length());
    else
        n = int(round(samples_per_bin));
    if (n == 0)
        PLERROR("In SoftHistogramBinner::getBins - You can't ask for empty bins");
    // Sort the data to make things easier.
    Mat w(v.length(), 2);
    w.column(0) << v;
    w.column(1) << TVec<int>(0, w.length() - 1, 1);
    sortRows(w);
    // Construct bins.
    TVec< TVec<int> > bins(n_bins);
    real min_w = w(0,0);
    real max_w = w(w.length() - 1, 0);
    real bin_width = (max_w - min_w) / real(n_bins);
    for (int i = 0; i < n_bins; i++) {
        real bin_left = min_w + i * bin_width;
        real bin_right = bin_left + bin_width;
        real bin_mid = (bin_left + bin_right) / 2;  // Center of the bin.
        // Find data point closest to bin_mid.
        int k = 0;
        int min;
        while (bin_mid > w(k,0)) k++;
        if (w(k,0) - bin_mid > bin_mid - w(k-1,0))
            min = k -1;
        else
            min = k;
        bins[i].append(int(w(min, 1)));
        // Expand to get n samples in the bin.
        int count = 1;
        int right = min;
        int left = min;
        int to_add;
        while (count < n) {
            if (right + 1 < w.length()) {
                // We can get more data on our right.
                if (left - 1 >= 0) {
                    // We can get more data on our left.
                    if (w(right + 1,0) - bin_mid < bin_mid - w(left - 1,0)) {
                        // Next point to add is on our right.
                        right++;
                        to_add = right;
                    } else {
                        // Next point to add is on our left.
                        left--;
                        to_add = left;
                    }
                } else {
                    // No more data on the left.
                    right++;
                    to_add = right;
                }
            } else {
                // No more data on the right.
                left--;
                to_add = left;
            }
            bins[i].append(int(w(to_add, 1)));
            count++;
        }
    }
    return bins;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SoftHistogramBinner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("SoftHistogramBinner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

///////////
// nBins //
///////////
int SoftHistogramBinner::nBins() const {
    return n_bins;
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
