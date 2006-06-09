// -*- C++ -*-

// KNNImputationVMatrix.cc
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

/*! \file KNNImputationVMatrix.cc */


#include "KNNImputationVMatrix.h"
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/vmat/VMat_basic_stats.h>
#include <plearn_learners/nearest_neighbors/ExhaustiveNearestNeighbors.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    KNNImputationVMatrix,
    "Impute missing values in its source by their average in a neighborhood.",

    "Each missing value is replaced by the mean of the observed values in\n"
    "the neighborhood of a sample, where the neighborhood is defined by a\n"
    "number of nearest neighbors 'knn' that have an observed value for this\n"
    "same variable.\n"
    "If no sample can be found with an observed value, the global mean of\n"
    "the variable is used.\n"
    "In its current implementation, the distance relationships are obtained\n"
    "from the full dataset (i.e. with no missing values), that must also be\n"
    "provided.\n"
);

//////////////////////////
// KNNImputationVMatrix //
//////////////////////////
KNNImputationVMatrix::KNNImputationVMatrix():
    knn(5),
    n_train_samples(-1),
    report_progress(true)
{}

////////////////////
// declareOptions //
////////////////////
void KNNImputationVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "knn", &KNNImputationVMatrix::knn,
                             OptionBase::buildoption,
        "Number of nearest neighbors considered.");

    declareOption(ol, "full_source", &KNNImputationVMatrix::full_source,
                                     OptionBase::buildoption,
        "Same dataset as 'source', but with no missing values.");

    declareOption(ol, "n_train_samples",
                  &KNNImputationVMatrix::n_train_samples,
                  OptionBase::buildoption,
        "If > 0, only samples in the first 'n_train_samples' will be\n"
        "considered candidate nearest neighbors.");

    declareOption(ol, "report_progress",
                  &KNNImputationVMatrix::report_progress,
                  OptionBase::buildoption,
        "Whether or not to display a progress bar.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void KNNImputationVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void KNNImputationVMatrix::build_()
{
    if (!source)
        return;

    assert( full_source );
    assert( full_source->length() == source->length() );
    assert( full_source->width()  == source->width()  );

    VMat candidates = full_source;
    if (n_train_samples > 0)
        candidates = new SubVMatrix(full_source, 0, 0, n_train_samples,
                                                       source->width());

    // Prepare nearest neighbor learner.
    PP<ExhaustiveNearestNeighbors> nn_learner =
        new ExhaustiveNearestNeighbors();
    nn_learner->num_neighbors = candidates->length();
    nn_learner->copy_target = false;
    nn_learner->copy_index = true;
    nn_learner->build();
    nn_learner->setTrainingSet(candidates);
    nn_learner->train();

    // Compute global mean.
    Vec global_mean;
    PLearn::computeMean(candidates, global_mean);

    // Perform actual missing values imputation.
    Vec input, target, output, input_nn;
    real weight;
    imputed_input.resize(0, source->inputsize());
    Vec imputed_row(source->inputsize());
    sample_index_to_imputed_index.resize(full_source->length());
    sample_index_to_imputed_index.fill(-1);
    for (int i = 0; i < full_source->length(); i++) {
        source->getExample(i, input, target, weight);
        if (input.hasMissing()) {
            nn_learner->computeOutput(input, output);
            for (int k = 0; k < input.length(); k++)
                if (is_missing(input[k])) {
                    int j = 0;
                    int count_neighbors = 0;
                    real mean = 0;
                    while (count_neighbors < knn && j < output.length()) {
                        int neighbor_index = int(round(output[j]));
                        source->getExample(neighbor_index, input_nn, target,
                                                                     weight);
                        if (!is_missing(input_nn[k])) {
                            mean += input_nn[k];
                            count_neighbors++;
                        }
                        j++;
                    }
                    if (count_neighbors > 0) {
                        // Found some neighbors with an observed value for
                        // variable 'k'.
                        mean /= count_neighbors;
                    } else {
                        mean = global_mean[k];
                    }
                    imputed_row[k] = mean;
                } else
                    imputed_row[k] = input[k];
            imputed_input.appendRow(imputed_row);
            sample_index_to_imputed_index[i] = imputed_input.length() - 1;
        }
    }

    // Obtain meta information from source.
    setMetaInfoFromSource();
}

///////////////
// getNewRow //
///////////////
void KNNImputationVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getRow(i, v);
    if (v.hasMissing()) {
        int idx = sample_index_to_imputed_index[i];
        assert( idx >= 0 );
        v.subVec(0, inputsize_) << imputed_input(idx);
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void KNNImputationVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(full_source,                      copies);
    deepCopyField(imputed_input,                    copies);
    deepCopyField(sample_index_to_imputed_index,    copies);
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
