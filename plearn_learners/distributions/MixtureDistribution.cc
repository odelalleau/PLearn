// -*- C++ -*-

// MixtureDistribution.cc
//
// Copyright (C) 2008 Olivier Delalleau
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

/*! \file MixtureDistribution.cc */


#include "MixtureDistribution.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    MixtureDistribution,
    "Weighted mixture of n distributions.",
    "Note that the weights are fixed and not learnt."
);

//////////////////////////
// MixtureDistribution //
//////////////////////////
MixtureDistribution::MixtureDistribution()
{}

////////////////////
// declareOptions //
////////////////////
void MixtureDistribution::declareOptions(OptionList& ol)
{
    declareOption(ol, "distributions", &MixtureDistribution::distributions,
                  OptionBase::buildoption,
        "Underlying distributions being mixed.");

    declareOption(ol, "weights", &MixtureDistribution::weights,
                  OptionBase::buildoption,
        "Weights of the distributions (must sum to 1). If left empty, then\n"
        "each distribution will be given a weight 1/number_of_distributions.");

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);

    // Hide unused options.

    redeclareOption(ol, "predicted_size",
                    &MixtureDistribution::predicted_size,
                    OptionBase::nosave,
        "Unused");

    redeclareOption(ol, "predictor_part",
                    &MixtureDistribution::predictor_part,
                    OptionBase::nosave,
        "Unused");

    redeclareOption(ol, "predictor_size",
                    &MixtureDistribution::predictor_size,
                    OptionBase::nosave,
        "Unused");


}

///////////
// build //
///////////
void MixtureDistribution::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void MixtureDistribution::build_()
{
    if (distributions.isEmpty())
        return;
    if (weights.isEmpty()) {
        int n = distributions.length();
        weights.resize(n);
        weights.fill(1 / real(n));
    }
    PLCHECK_MSG(weights.length() == distributions.length() &&
                 is_equal(PLearn::sum(weights), 1),
                 "There must be one weight for each distribution, and the "
                 "weights must sum to 1");
    getSizes();
}

/////////
// cdf //
/////////
real MixtureDistribution::cdf(const Vec& y) const
{
    PLERROR("cdf not implemented for MixtureDistribution"); return 0;
}

/////////////////
// expectation //
/////////////////
void MixtureDistribution::expectation(Vec& mu) const
{
    PLASSERT( !distributions.isEmpty() );
    mu.resize(distributions[0]->getNPredicted());
    mu.fill(0);
    for (int i = 0; i < distributions.length(); i++) {
        distributions[i]->expectation(work);
        multiplyAcc(mu, work, weights[i]);
    }
}

////////////
// forget //
////////////
void MixtureDistribution::forget()
{
    for (int i = 0; i < distributions.length(); i++)
        distributions[i]->forget();
    inherited::forget();
    getSizes();
}

//////////////
// generate //
//////////////
void MixtureDistribution::generate(Vec& y) const
{
    int j = random_gen->multinomial_sample(weights);
    distributions[j]->generate(y);
}

//////////////
// getSizes //
//////////////
void MixtureDistribution::getSizes() const {
    PLASSERT( !distributions.isEmpty() );
    n_predicted = distributions[0]->getNPredicted();
    n_predictor = distributions[0]->getNPredictor();
}

/////////////////
// log_density //
/////////////////
real MixtureDistribution::log_density(const Vec& y) const
{
    int n = distributions.length();
    work.resize(n);
    for (int i = 0; i < n; i++) {
        work[i] = distributions[i]->log_density(y) + pl_log(weights[i]);
    }
    return logadd(work);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void MixtureDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("MixtureDistribution::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// resetGenerator //
////////////////////
void MixtureDistribution::resetGenerator(long g_seed)
{
    for (int i = 0; i < distributions.length(); i++)
        distributions[i]->resetGenerator(g_seed);
    inherited::resetGenerator(g_seed);
}

//////////////////
// setPredictor //
//////////////////
void MixtureDistribution::setPredictor(const Vec& predictor, bool call_parent) const
{
    if (call_parent)
        inherited::setPredictor(predictor, true);
    for (int i = 0; i < distributions.length(); i++)
        distributions[i]->setPredictor(predictor, call_parent);
    getSizes();
}

////////////////////////////////
// setPredictorPredictedSizes //
////////////////////////////////
bool MixtureDistribution::setPredictorPredictedSizes(int the_predictor_size,
                                               int the_predicted_size,
                                               bool call_parent)
{
    bool sizes_have_changed = false;
    if (call_parent)
        sizes_have_changed = inherited::setPredictorPredictedSizes(
                the_predictor_size, the_predicted_size, true);
    for (int i = 0; i < distributions.length(); i++)
        distributions[i]->setPredictorPredictedSizes(the_predictor_size,
                                                     the_predicted_size,
                                                     call_parent);
    getSizes();
    // Returned value.
    return sizes_have_changed;
}

/////////////////
// survival_fn //
/////////////////
real MixtureDistribution::survival_fn(const Vec& y) const
{
    PLERROR("survival_fn not implemented for MixtureDistribution"); return 0;
}

///////////
// train //
///////////
void MixtureDistribution::train()
{
    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    PLCHECK( nstages == 1 && stage == 0 );
    for (int i = 0; i < distributions.length(); i++)
        distributions[i]->train();
    stage = 1;
    getSizes();
}

//////////////
// variance //
//////////////
void MixtureDistribution::variance(Mat& covar) const
{
    PLERROR("variance not implemented for MixtureDistribution");
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
