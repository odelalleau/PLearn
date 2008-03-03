// -*- C++ -*-

// KNNVMatrix.cc
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

/*! \file KNNVMatrix.cc */

#include <plearn/ker/DistanceKernel.h>
#include <plearn/base/tostring.h>
#include "KNNVMatrix.h"
#include "SelectRowsVMatrix.h"
#include "SubVMatrix.h"

namespace PLearn {
using namespace std;

////////////////
// KNNVMatrix //
////////////////
KNNVMatrix::KNNVMatrix()
    : knn(6),
      report_progress(1)
{}

PLEARN_IMPLEMENT_OBJECT(KNNVMatrix,
                        "A VMatrix that sees the nearest neighbours of each sample in the source VMat.",
                        "Each sample is followed by its (knn-1) nearest neighbours.\n"
                        "To each row is appended an additional target, which is:\n"
                        " - 1 if it is the first of a bag of neighbours,\n"
                        " - 2 if it is the last of a bag,\n"
                        " - 0 if it is none of these,\n"
                        " - 3 if it is both (only for knn == 1).\n"
                        "In addition, if a kernel_pij kernel is provided,, in the input part of the VMatrix\n"
                        "is appended p_ij, where\n"
                        "  p_ij = K(x_i,x_j) / \\sum_{k \\in knn(i), k != i} K(x_i,x_k)\n"
                        "where K = kernel_pij, and j != i (for j == i, p_ij = -1).");

////////////////////
// declareOptions //
////////////////////
void KNNVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "k_nn_mat", &KNNVMatrix::k_nn_mat, OptionBase::buildoption,
                  "TODO comment");

    declareOption(ol, "knn", &KNNVMatrix::knn, OptionBase::buildoption,
                  "The number of nearest neighbours to consider (including the point itself).");

    declareOption(ol, "kernel_pij", &KNNVMatrix::kernel_pij, OptionBase::buildoption,
                  "An optional kernel used to compute the pij weights (see help).");

    declareOption(ol, "report_progress", &KNNVMatrix::report_progress, OptionBase::buildoption,
                  "TODO comment");

// Kinda useless to declare it as an option if we recompute it in build().
// TODO See how to be more efficient.
//  declareOption(ol, "nn", &KNNVMatrix::nn, OptionBase::learntoption,
//      "The matrix containing the index of the knn nearest neighbours of\n"
//      "each data point.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void KNNVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void KNNVMatrix::build_() {
    updateMtime(source);
    updateMtime(k_nn_mat);
    if (source) {
        int n = source->length();
        bool recompute_nn = true;
        if (k_nn_mat) {
            if (k_nn_mat->length() > 0) {
                // We are given precomputed k nearest neighbours, what a good news.
                if (k_nn_mat->length() == source->length()) {
                    if (k_nn_mat->width() < knn) {
                        PLWARNING("In KNNVMatrix::build_ - Not enough neighbours in the given k_nn_mat, will recompute nearest neighbours");
                    } else {
                        // Looks like this is the right thing.
                        recompute_nn = false;
                        nn.resize(n, knn);
                        for (int i = 0; i < n; i++) {
                            k_nn_mat->getSubRow(i, 0, nn(i));
                        }
                    }
                } else {
                    // Lengths differ: maybe the source VMat is a subset of the matrix
                    // whose nearest neighbours have been computed.
                    // Let's try a SelectRowsVMatrix.
                    PP<SelectRowsVMatrix> smat = dynamic_cast<SelectRowsVMatrix*>((VMatrix*) source);
                    if (!smat.isNull() && smat->source->length() == k_nn_mat->length()) {
                        // Bingo !
                        // Safety warning just in case it is not what we want.
                        PLWARNING("In KNNVMatrix::build_ - Will consider the given k_nn_mat has been computed on source's distr VMat");
                        recompute_nn = false;
                        // Now we need to retrieve the nearest neighbours within the SelectRowsVMatrix.
                        nn.resize(n, knn);
                        Vec store_nn(k_nn_mat->width());
                        for (int i = 0; i < n; i++) {
                            nn(i,0) = i;  // The nearest neighbour is always itself.
                            k_nn_mat->getRow(smat->indices[i], store_nn);
                            int k = 1;
                            for (int j = 1; j < knn; j++) {
                                bool ok = false;
                                while (!ok && k < store_nn.length()) {
                                    int q = smat->indices.find(int(store_nn[k]));
                                    if (q >= 0) {
                                        // The k-th nearest neighbour in smat->distr is in smat.
                                        ok = true;
                                        nn(i,j) = q;
                                    }
                                    k++;
                                }
                                if (k == store_nn.length()) {
                                    // We didn't find the j-th nearest neighbour.
                                    PLERROR("In KNNVMatrix::build_ - Not enough neighbours in the SelectRowsVMatrix");
                                }
                            }
                        }
                    } else {
                        // Maybe it's a SubVMatrix of the matrix whose nearest neighbours have been computed.
                        PP<SubVMatrix> smat_sub = dynamic_cast<SubVMatrix*>((VMatrix*) source);
                        if (    !smat_sub.isNull()
                                &&  smat_sub->source->length() == k_nn_mat->length()
                                &&  smat_sub->width() == smat_sub->source->width()) {
                            // Bingo !
                            // Safety warning just in case it is not what we want.
                            PLWARNING("In KNNVMatrix::build_ - Will consider the given k_nn_mat has been computed on source's parent VMat");
                            recompute_nn = false;
                            nn.resize(n, knn);
                            Vec store_nn(k_nn_mat->width());
                            for (int i = 0; i < n; i++) {
                                nn(i,0) = i;  // The nearest neighbour is always itself.
                                k_nn_mat->getRow(i + smat_sub->istart, store_nn);
                                int k = 1;
                                for (int j = 1; j < knn; j++) {
                                    bool ok = false;
                                    while (!ok && k < store_nn.length()) {
                                        int q = int(store_nn[k]) - smat_sub->istart;
                                        if (q >= 0 && q < smat_sub->length()) {
                                            // The k-th nearest neighbour in
                                            // smat_sub->source is in smat_sub.
                                            ok = true;
                                            nn(i,j) = q - smat_sub->istart;
                                        }
                                        k++;
                                    }
                                    if (k == store_nn.length()) {
                                        // We didn't find the j-th nearest neighbour.
                                        PLERROR("In KNNVMatrix::build_ - Not enough neighbours in the SubVMatrix");
                                    }

                                }
                            }
                        } else {
                            // What the hell is this ?
                            PLWARNING("In KNNVMatrix::build_ - Don't know what to do with k_nn_mat, will recompute the nearest neighbours");
                        }
                    }
                }
            }
        }

        if (recompute_nn) {
            // First make sure we can store the result if needed.
            if (k_nn_mat) {
                if (k_nn_mat->length() > 0) {
                    PLERROR("In KNNVMatrix::build_ - The given k_nn_mat already has data, free it first");
                }
            }
            // Compute the pairwise distances.
            DistanceKernel dk(2);
            if (report_progress) {
                dk.report_progress = true;
                dk.build();
            }
            dk.setDataForKernelMatrix(source);
            Mat distances(n,n);
            dk.computeGramMatrix(distances);
            // Deduce the nearest neighbours.
            nn = dk.computeNeighbourMatrixFromDistanceMatrix(distances);
            // Only keep the (knn) nearest ones.
            // TODO Free the memory used by the other neighbours.
            // TODO Make the matrix be a TMat<int> instead of a Mat.
            nn.resize(n, knn);
            // Store the result.
            if (k_nn_mat) {
                for (int i = 0; i < n; i++) {
                    k_nn_mat->appendRow(nn(i));
                }
            }
        }

        // Initialize correctly the various fields.
        targetsize_ = source->targetsize() + 1;
        length_ = n * knn;
        width_ = source->width() + 1;
        setMetaInfoFromSource();

        // Compute the p_ij if needed.
        if (kernel_pij) {
            // TODO REPORT PROGRESS IF NEEDED.
            inputsize_++;
            width_++;
            kernel_pij->setDataForKernelMatrix(source);
            int l = source->length();
            pij.resize(l, knn-1);
            for (int i = 0; i < l; i++) {
                real sum = 0;
                real k_ij;
                for (int j = 1; j < knn; j++) {
                    // We omit the first nearest neighbour, which is the point itself.
                    k_ij = kernel_pij->evaluate_i_j(i, int(nn(i,j)));
                    pij(i,j-1) = k_ij;
                    sum += k_ij;
                }
                pij.row(i) /= sum;
            }
        }
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void KNNVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    deepCopyField(source_row, copies);
    deepCopyField(nn, copies);
    deepCopyField(pij, copies);
    // Currently commented out because some of the VMats used for k_nn_mat
    // may not implement deep copy correctly.
    // TODO Put back when other VMats are fine.
//  deepCopyField(k_nn_mat, copies);
    deepCopyField(kernel_pij, copies);

    PLWARNING("In KNNVMatrix::makeDeepCopyFromShallowCopy - k_nn_mat will not be deep copied");
    //  PLERROR("KNNVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

//////////////////////
// getSourceIndexOf //
//////////////////////
int KNNVMatrix::getSourceIndexOf(int i, int& i_ref, int& i_n) const {
    i_ref = i / knn;
    i_n = i % knn;
    int i_neighbour_source = int(nn(i_ref, i_n));
    return i_neighbour_source;
}

///////////////
// getNewRow //
///////////////
void KNNVMatrix::getNewRow(int i, const Vec& v) const {
    source_row.resize(source->width());
    int i_n;
    int i_ref;
    int real_i = getSourceIndexOf(i, i_ref, i_n);
    source->getRow(real_i, source_row);
    if (kernel_pij) {
        v.subVec(0, source->inputsize()) << source_row.subVec(0, source->inputsize());
        if (i_n > 0) {
            v[source->inputsize()] = pij(i_ref, i_n - 1);
        } else {
            v[source->inputsize()] = -1;
        }
    } else {
        v.subVec(0, source->inputsize() + source->targetsize())
            << source_row.subVec(0, source->inputsize() + source->targetsize());
    }
    v.subVec(inputsize(), source->targetsize())
        << source_row.subVec(source->inputsize(), source->targetsize());
    v[inputsize() + source->targetsize()] = getTag(i_n);
    if (weightsize() > 0) {
        v.subVec(inputsize() + targetsize(), weightsize())
            << source_row.subVec(source->inputsize() + source->targetsize(), source->weightsize());
    }
}

////////////
// getTag //
////////////
int KNNVMatrix::getTag(int p) const {
    // TODO Better use the constants defined in SumOverBagsVariable.h.
    if (knn == 1) return 3;
    if (p == 0) return 1;
    if (p == knn - 1) return 2;
    return 0;
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
