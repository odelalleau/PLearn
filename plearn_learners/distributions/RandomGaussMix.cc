// -*- C++ -*-

// RandomGaussMix.cc
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

/*! \file RandomGaussMix.cc */


#include "RandomGaussMix.h"
#include <plearn/io/load_and_save.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RandomGaussMix,
    "Mixture of Gaussians where the means and covariances are randomly chosen",
    "The means' distribution is provided by the user. The principal\n"
    "directions are obtained from Gram-Schmidt orthogonalization of a random\n"
    "matrix. The variances in the principal directions are obtained from\n"
    "a user-provided distribution. The weights of the Gaussians are also\n"
    "taken from another user-provided distribution.\n"
    "Note that for the sake of simplicity, this is an unconditional\n"
    "distribution.\n"
);

////////////////////
// RandomGaussMix //
////////////////////
RandomGaussMix::RandomGaussMix()
{
    type = "general";
}

////////////////////
// declareOptions //
////////////////////
void RandomGaussMix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "mean_distribution",
                      &RandomGaussMix::mean_distribution,
                      OptionBase::buildoption,
        "The distribution from which means are sampled. A sample from this\n"
        "distribution should be a D-dimensional vector, representing the mean\n"
        "of a Gaussian in the input space.");

    declareOption(ol, "variance_distribution",
                      &RandomGaussMix::variance_distribution,
                      OptionBase::buildoption,
        "The distribution from which variances are sampled. A sample from\n"
        "this distribution should be a D-dimensional vector, representing\n"
        "the variance in each of the D principal directions of the Gaussian.");

    declareOption(ol, "weight_distribution",
                      &RandomGaussMix::weight_distribution,
                      OptionBase::buildoption,
        "The distribution from which the weight of each Gaussian is sampled.\n"
        "It should output a single non-negative scalar (the weights will be\n"
        "normalized afterwards so that they sum to 1).");

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);

    // Hide unused options.
    redeclareOption(ol, "type", &RandomGaussMix::type,
                                OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "n_eigen", &RandomGaussMix::n_eigen,
                                   OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "efficient_missing",
                         &RandomGaussMix::efficient_missing,
                         OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "efficient_k_median",
                        &RandomGaussMix::efficient_k_median,
                        OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "efficient_k_median_iter",
                        &RandomGaussMix::efficient_k_median_iter,
                        OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "impute_missing", &RandomGaussMix::impute_missing,
                                          OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "kmeans_iterations",
                        &RandomGaussMix::kmeans_iterations,
                        OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "alpha_min", &RandomGaussMix::alpha_min,
                                     OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol,"sigma_min", &RandomGaussMix::sigma_min,
                                     OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "epsilon", &RandomGaussMix::epsilon,
                                   OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "predictor_size",
                        &RandomGaussMix::predictor_size,
                        OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "predicted_size",
                        &RandomGaussMix::predicted_size,
                        OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "predictor_part",
                        &RandomGaussMix::predictor_part,
                        OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "n_predictor",
                        &RandomGaussMix::n_predictor,
                        OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "expdir", &RandomGaussMix::expdir,
                        OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "forget_when_training_set_changes",
                        &RandomGaussMix::forget_when_training_set_changes,
                        OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "nstages", &RandomGaussMix::nstages,
                                   OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "nservers", &RandomGaussMix::nservers,
                                    OptionBase::nosave,
        "Not used in RandomGaussMix.");

    redeclareOption(ol, "save_trainingset_prefix",
                        &RandomGaussMix::save_trainingset_prefix,
                        OptionBase::nosave,
        "Not used in RandomGaussMix.");
}

///////////
// build //
///////////
void RandomGaussMix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void RandomGaussMix::build_()
{
    if (!variance_distribution || !mean_distribution || !weight_distribution)
        return;

    // Need to reset the underlying distributions' seeds so that the generated
    // values are always the same at each build.
    mean_distribution->resetGenerator(mean_distribution->seed_);
    variance_distribution->resetGenerator(variance_distribution->seed_);
    weight_distribution->resetGenerator(weight_distribution->seed_);

    D = mean_distribution->getNPredicted();

    // Generate random Gaussian parameters.
    eigenvalues.resize(L, D);
    center.resize(L, D);
    alpha.resize(L);
    eigenvectors.resize(L);
    for (int j = 0; j < L; j++) {
        // Generate random matrix and perform Gram-Schmidt orthonormalization.
        Mat& eigenvecs = eigenvectors[j];
        eigenvecs.resize(D, D);
        int n_basis = -1;
        // It might happen that the rows of the random matrix are not
        // sufficiently independent, in which case we just try again.
        while (n_basis != D) {
            random_gen->fill_random_uniform(eigenvecs, -1, 1);
            n_basis = GramSchmidtOrthogonalization(eigenvecs);
        }
        // Generate random eigenvalues.
        Vec eigenvals = eigenvalues(j);
        variance_distribution->generate(eigenvals);
        PLASSERT( eigenvals.length() == D );
        // Note that eigenvalues must be sorted in decreasing order.
        sortElements(eigenvals);
        eigenvals.swap();
        // Generate random mean.
        Vec mean_j = center(j);
        mean_distribution->generate(mean_j);
        PLASSERT( mean_j.length() == D );
        // Generate random weight.
        Vec alpha_j = alpha.subVec(j, 1);
        weight_distribution->generate(alpha_j);
        PLASSERT( alpha_j.length() == 1 );
    }
    // Normalize 'alpha' so that it sums to 1.
    real sum = 0;
    for (int j = 0; j < L; j++) {
        real alpha_j = alpha[j];
        if (alpha_j < 0)
            PLERROR("In RandomGaussMix::build_ - The weight of a Gaussian "
                    "cannot be negative");
        sum += alpha_j;
    }
    PLASSERT( sum > 0 );
    alpha /= sum;
    // Set a few parameters that are needed.
    alpha_min = min(alpha);
    sigma_min = 1e-10;
    inputsize_ = D;
    n_eigen_computed = D;
    stage = 1;
    // Rebuild the mixture.
    inherited::build();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RandomGaussMix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("RandomGaussMix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

///////////
// train //
///////////
void RandomGaussMix::train()
{
    // This class does not need to be trained.
    return;
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
