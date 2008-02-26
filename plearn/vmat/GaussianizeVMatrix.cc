// -*- C++ -*-

// GaussianizeVMatrix.cc
//
// Copyright (C) 2006 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file GaussianizeVMatrix.cc */


#include "GaussianizeVMatrix.h"
#include <plearn/math/pl_erf.h>
#include <plearn/vmat/VMat_computeStats.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    GaussianizeVMatrix,
    "Transforms its source VMatrix so that its features look Gaussian.",

    "This VMat transforms the features of its source that are obviously non-\n"
    "Gaussian, i.e. when the difference between the maximum and minimum\n"
    "value is too large compared to the standard deviation (the meaning of\n"
    "'too large' being controlled by the 'threshold_ratio' option).\n"
    "\n"
    "When this happens, the values of a features are sorted and their rank\n"
    "is used to transform them through the inverse cumulative of a normal\n"
    "Gaussian, resulting on a distribution that actually looks Gaussian.\n"
    "Note that, unless specified otherwise through the options, only the\n"
    "input features are transformed.\n"
    "\n"
    "It is important to note that only unique values are considered when\n"
    "computing the mapping, so that there is no 'hole' in the resulting\n"
    "distribution. This means the transformation learnt does not depend on\n"
    "the number of occurences of a specific value, but only on the ordering\n"
    "of the unique values encountered. The 'uniqueness' is defined by the\n"
    "PLearn 'is_equal' function, used to approximately compare real numbers.\n"
    "\n"
    "An additional 'train_source' VMat can also be specified in order to\n"
    "transform new data (in the 'source' option) while the transformation\n"
    "parameters are learned on a fixed 'train_source' VMat (e.g. when new\n"
    "test data are obtained and need to be properly Gaussianized).\n"
);

////////////////////////
// GaussianizeVMatrix //
////////////////////////
GaussianizeVMatrix::GaussianizeVMatrix():
    gaussianize_input(true),
    gaussianize_target(false),
    gaussianize_weight(false),
    gaussianize_extra(false),
    gaussianize_binary(false),
    threshold_ratio(10)
{}

////////////////////
// declareOptions //
////////////////////
void GaussianizeVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "threshold_ratio", &GaussianizeVMatrix::threshold_ratio,
                                         OptionBase::buildoption,
        "A source's feature will be Gaussianized when the following holds:\n"
        "(max - min) / stddev > threshold_ratio.");

    declareOption(ol, "gaussianize_input",
                  &GaussianizeVMatrix::gaussianize_input,
                  OptionBase::buildoption,
        "Whether or not to Gaussianize the input part.");

    declareOption(ol, "gaussianize_target",
                  &GaussianizeVMatrix::gaussianize_target,
                  OptionBase::buildoption,
        "Whether or not to Gaussianize the target part.");

    declareOption(ol, "gaussianize_weight",
                  &GaussianizeVMatrix::gaussianize_weight,
                  OptionBase::buildoption,
        "Whether or not to Gaussianize the weight part.");

    declareOption(ol, "gaussianize_extra",
                  &GaussianizeVMatrix::gaussianize_extra,
                  OptionBase::buildoption,
        "Whether or not to Gaussianize the extra part.");

    declareOption(ol, "gaussianize_binary",
                  &GaussianizeVMatrix::gaussianize_binary,
                  OptionBase::buildoption,
        "Whether or not to Gaussianize binary variable.");

    declareOption(ol, "train_source", &GaussianizeVMatrix::train_source,
                                      OptionBase::buildoption,
        "An optional VMat that will be used instead of 'source' to compute\n"
        "the transformation parameters from the distribution statistics.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void GaussianizeVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void GaussianizeVMatrix::build_()
{
    if (!source)
        return;

    if (train_source) {
        PLCHECK( train_source->width() == source->width() );
        PLCHECK( train_source->inputsize()  == source->inputsize() &&
                train_source->targetsize() == source->targetsize() &&
                train_source->weightsize() == source->weightsize() &&
                train_source->extrasize()  == source->extrasize() );
    }

    VMat the_source = train_source ? train_source : source;

    PLCHECK( the_source->inputsize() >= 0 && the_source->targetsize() >= 0 &&
            the_source->weightsize() >= 0 && the_source->extrasize() >= 0 );

    // Find which dimensions to Gaussianize.
    features_to_gaussianize.resize(0);
    int col = 0;
    if (gaussianize_input)
        features_to_gaussianize.append(
                TVec<int>(col, col + the_source->inputsize() - 1, 1));
    col += the_source->inputsize();
    if (gaussianize_target)
        features_to_gaussianize.append(
                TVec<int>(col, col + the_source->targetsize() - 1, 1));
    col += the_source->targetsize();
    if (gaussianize_weight)
        features_to_gaussianize.append(
                TVec<int>(col, col + the_source->weightsize() - 1, 1));
    col += the_source->weightsize();
    if (gaussianize_extra)
        features_to_gaussianize.append(
                TVec<int>(col, col + the_source->extrasize() - 1, 1));
    col += the_source->extrasize();

    TVec<StatsCollector> stats = the_source->
        getPrecomputedStatsFromFile("stats_gaussianizeVMatrix.psave", -1, true);

    // See which dimensions violate the Gaussian assumption and will be
    // actually Gaussianized, and store the corresponding list of values.
    TVec<int> candidates = features_to_gaussianize.copy();
    features_to_gaussianize.resize(0);
    values.resize(0);
    for (int i = 0; i < candidates.length(); i++) {
        int j = candidates[i];
        StatsCollector& stat = stats[j];
        if (fast_exact_is_equal(stat.stddev(), 0))
            continue;
        if (!gaussianize_binary && stat.isbinary())
            continue;
        if ((stat.max() - stat.min()) > threshold_ratio * stat.stddev()) {
            features_to_gaussianize.append(j);
            values.append(Vec());
            Vec& values_j = values.lastElement();
            map<real, StatsCollectorCounts>::const_iterator it, it_dummy;
            // Note that we obtain the approximate counts, so that almost equal
            // values have been merged together already.
            map<real,StatsCollectorCounts>* count_map =
                                                stat.getApproximateCounts();
            // We use a dummy iterator to get rid of the last element in the
            // map, which is the max real value.
            it_dummy = count_map->begin();
            it_dummy++;
            for (it = count_map->begin(); it_dummy != count_map->end();
                                          it++, it_dummy++)
            {
                values_j.append(it->first);
            }
        }
    }
    if(features_to_gaussianize.size()==0)
        PLWARNING("GaussianizeVMatrix::build_() 0 variable was gaussianized");
    // Obtain meta information from source.
    setMetaInfoFromSource();
}

///////////////
// getNewRow //
///////////////
void GaussianizeVMatrix::getNewRow(int i, const Vec& v) const
{
    PLASSERT( source );
    source->getRow(i, v);
    for (int k = 0; k < features_to_gaussianize.length(); k++) {
        int j = features_to_gaussianize[k];
        real current_val = v[j];
        if (is_missing(current_val))
            continue;
        // Find closest values in the training data.
        Vec& values_j = values[k];
        real interpol;
        if (current_val < values_j[0]) {
            // Smaller than the minimum.
            interpol = 0;
        } else if (current_val > values_j.lastElement()) {
            // Higher than the maximum.
            interpol = 1;
        } else {
            int min = 0;
            int max = values_j.length() - 1;
            while (max - min > 1) {
                int mid = (max + min) / 2;
                real mid_val = values_j[mid];
                if (current_val < mid_val)
                    max = mid;
                else if (current_val > mid_val)
                    min = mid;
                else {
                    // Found the exact value.
                    min = max = mid;
                }
            }
            if (min == max)
                interpol = min;
            else {
                PLASSERT( max - min == 1 );
                interpol = (current_val - values_j[min]) /
                          (values_j[max] - values_j[min]) + min;
                PLASSERT( !is_missing(interpol) );
            }
        }
        interpol /= (values_j.length() - 1);
        PLASSERT( interpol >= 0 && interpol <= 1 );
        // The expectation of the minimum and maximum of n numbers taken from a
        // uniform(0,1) distribution are respectively 1/n+1 and n/n+1: we shift
        // and rescale 'interpol' to be in [1/n+1, n/n+1] before using the
        // inverse of the Gaussian cumulative function.
        real n = values_j.length();
        interpol = (n - 1) / (n + 1) * interpol + 1 / (n + 1);
        v[j] = fast_gauss_01_quantile(interpol);
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GaussianizeVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("GaussianizeVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
