// -*- C++ -*-

// ThresholdedKernel.cc
//
// Copyright (C) 2005 Olivier Delalleau 
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

/*! \file ThresholdedKernel.cc */


#include "ThresholdedKernel.h"
#include <plearn/math/PRandom.h>

namespace PLearn {
using namespace std;

///////////////////////
// ThresholdedKernel //
///////////////////////
ThresholdedKernel::ThresholdedKernel():
    knn(2),
    knn_approximation(0),
    max_size_for_full_gram(5000),
    method("knn"),
    threshold(0)
{
}

PLEARN_IMPLEMENT_OBJECT(ThresholdedKernel,
    "Thresholds an underlying kernel.",
    ""
);

////////////////////
// declareOptions //
////////////////////
void ThresholdedKernel::declareOptions(OptionList& ol)
{
    declareOption(ol, "method", &ThresholdedKernel::method, OptionBase::buildoption,
                  "Which method is used to threshold the underlying kernel:\n"
                  " - 'knn' : if y is such that K(x,y) is strictly less than K(x,n_k(x)) where n_k(x)\n"
                  "           is the k-th neighbor of x as given by K, and K(x,y) < K(n_k(y), y), then\n"
                  "           K(x,y) is thresholded\n");

    declareOption(ol, "threshold", &ThresholdedKernel::threshold, OptionBase::buildoption,
                  "The value returned when K(x,y) is thresholded.");

    declareOption(ol, "knn", &ThresholdedKernel::knn, OptionBase::buildoption,
                  "When 'method' is 'knn', this is 'k' in n_k(x) (x will be counted if in data matrix).");

    declareOption(ol, "knn_approximation", &ThresholdedKernel::knn_approximation,
                                           OptionBase::buildoption,
                  "When 'method' is 'knn', this option can take several values:\n"
                  " - 0          : it is ignored, the 'knn' nearest neighbors are computed normally\n"
                  " - p > 1      : the value of K(x,n_k(x)) is estimated by computing K(x,y) for p\n"
                  "                values of y taken randomly in the dataset (the same value of y\n"
                  "                may be taken more than once)\n"
                  " - 0 < f <= 1 : same as above, with p = f * n (n = dataset size)");

    declareOption(ol, "max_size_for_full_gram", &ThresholdedKernel::max_size_for_full_gram, OptionBase::buildoption,
                  "When the dataset has more than 'max_size_for_full_gram' samples, the full Gram\n"
                  "matrix will not be computed in memory (less efficient, but scales better).");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ThresholdedKernel::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ThresholdedKernel::build_()
{
    if (source_kernel && !source_kernel->is_symmetric)
        PLERROR("In ThresholdedKernel::build_ - The source kernel must currently "
                "be symmetric");
    PLASSERT( knn_approximation >= 0);
    knn_approx = !fast_exact_is_equal(knn_approximation, 0);
    if (knn_approx) {
        if (knn_approximation > 1)
            n_approx = int(round(knn_approximation));
        // Otherwise, n_approx is set in setDataForKernelMatrix.
    } else
        n_approx = -1;
}

///////////////////////
// computeGramMatrix //
///////////////////////
void ThresholdedKernel::computeGramMatrix(Mat K) const {
    if (cache_gram_matrix && gram_matrix_is_cached) {
        K << gram_matrix;
        return;
    }
    source_kernel->computeGramMatrix(K);
    thresholdGramMatrix(K);
    if (cache_gram_matrix) {
        int l = K.length();
        gram_matrix.resize(l,l);
        gram_matrix << K;
        gram_matrix_is_cached = true;
    }
}

//////////////
// evaluate //
//////////////
real ThresholdedKernel::evaluate(const Vec& x1, const Vec& x2) const {
    real k_x1_x2 = source_kernel->evaluate(x1, x2);
    if (method == "knn") {
        if (knn_approx)
            evaluate_random_k_x_i(x1, k_x_xi);
        else
            source_kernel->evaluate_all_x_i(x1, k_x_xi);
        negateElements(k_x_xi);
        partialSortRows(k_x_xi_mat, knn_sub);
        if (k_x1_x2 >= - k_x_xi[knn_sub-1])
            return k_x1_x2;
        if (knn_approx)
            evaluate_random_k_x_i(x2, k_x_xi);
        else
            source_kernel->evaluate_all_i_x(k_x_xi, x2);
        partialSortRows(k_x_xi_mat, knn_sub);
        negateElements(k_x_xi);
        if (k_x1_x2 >= -k_x_xi[knn_sub-1])
            return k_x1_x2;
        return threshold;
    }
    PLERROR("ThresholdedKernel::evaluate: unsupported method '%s'", method.c_str());
    return MISSING_VALUE;
}

//////////////////
// evaluate_i_j //
//////////////////
real ThresholdedKernel::evaluate_i_j(int i, int j) const {
    real k_i_j = source_kernel->evaluate_i_j(i, j);
    if (method == "knn") {
        if (k_i_j >= knn_kernel_values[i] || k_i_j >= knn_kernel_values[j])
            return k_i_j;
        else
            return threshold;
    }
    PLERROR("ThresholdedKernel::evaluate_i_j: unsupported method '%s'", method.c_str());
    return MISSING_VALUE;
}

//////////////////
// evaluate_i_x //
//////////////////
real ThresholdedKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const {
    // Default = uses the Kernel implementation.
    // Alternative = return source_kernel->evaluate_i_x(i,x,squared_norm_of_x);
    return Kernel::evaluate_i_x(i, x, squared_norm_of_x);
}

////////////////////////
// evaluate_i_x_again //
////////////////////////
real ThresholdedKernel::evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x, bool first_time) const {
    if (method == "knn") {
        if (first_time) {
            if (knn_approx)
                evaluate_random_k_x_i(x, k_x_xi);
            else
                source_kernel->evaluate_all_i_x(x, k_x_xi);
            negateElements(k_x_xi);
            partialSortRows(k_x_xi_mat, knn_sub);
            k_x_threshold = - k_x_xi[knn_sub - 1];
        }
        real k_i_x = source_kernel->evaluate_i_x_again(i, x, squared_norm_of_x, first_time);
        if (k_i_x >= k_x_threshold || k_i_x >= knn_kernel_values[i])
            return k_i_x;
        else
            return threshold;
    }
    PLERROR("ThresholdedKernel::evaluate_i_x_again: unsupported method '%s'", method.c_str());
    return MISSING_VALUE;
}


///////////////////////////
// evaluate_random_k_x_i //
///////////////////////////
void ThresholdedKernel::evaluate_random_k_x_i(const Vec& x, const Vec& k_x_xi)
                        const
{
    PP<PRandom> random = PRandom::common(false); // PRandom with fixed seed.
    int k = k_x_xi.length();
    for (int j = 0; j < k; j++) {
        int i = random->uniform_multinomial_sample(n_examples);
        k_x_xi[j] = source_kernel->evaluate_x_i(x,i);
    }
}

//////////////////
// evaluate_x_i //
//////////////////
real ThresholdedKernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const {
    return Kernel::evaluate_x_i(x, i, squared_norm_of_x);
}

////////////////////////
// evaluate_x_i_again //
////////////////////////
real ThresholdedKernel::evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x, bool first_time) const {
    if (method == "knn") {
        if (first_time) {
            if (knn_approx)
                evaluate_random_k_x_i(x, k_x_xi);
            else
                source_kernel->evaluate_all_x_i(x, k_x_xi);
            negateElements(k_x_xi);
            partialSortRows(k_x_xi_mat, knn_sub);
            k_x_threshold = - k_x_xi[knn_sub - 1];
        }
        real k_x_i = source_kernel->evaluate_x_i_again(x, i, squared_norm_of_x, first_time);
        if (k_x_i >= k_x_threshold || k_x_i >= knn_kernel_values[i])
            return k_x_i;
        else
            return threshold;
    }
    return MISSING_VALUE;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ThresholdedKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(knn_kernel_values, copies);
    deepCopyField(k_x_xi, copies);
    deepCopyField(k_x_xi_mat, copies);
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void ThresholdedKernel::setDataForKernelMatrix(VMat the_data) {
    inherited::setDataForKernelMatrix(the_data);
    int n = the_data->length();
    PP<ProgressBar> pb;
    knn_sub = knn_approx ? int(round(knn * real(n_approx) / real(n)))
                             : knn;
    if (knn_sub <= 0)
        PLERROR("In ThresholdedKernel::setDataForKernelMatrix - Not "
                "enough neighbors considered");
    if (method == "knn") {
        knn_kernel_values.resize(n);
        if (knn_approx) {
            if (knn_approximation <= 1) {
                n_approx = int(round(knn_approximation * n));
                PLASSERT( n_approx >= 1 );
            } else {
                int k_int = int(round(knn_approximation));
                if (k_int > n)
                    PLERROR("In ThresholdedKernel::setDataForKernelMatrix - "
                            "'knn_approximation' (%d) cannot be more than the "
                            " number of data points (%d)", k_int, n);
            }
        }
        if (knn_sub > n)
            PLERROR("In ThresholdedKernel::setDataForKernelMatrix - The number"
                    " of nearest neighbors to compute (%d) must be less than "
                    "the length of the dataset (%d)", knn_sub, n);
        if (n <= max_size_for_full_gram && !knn_approx) {
            // Can afford to store the Gram matrix in memory.
            gram_matrix.resize(n,n);
            source_kernel->computeGramMatrix(gram_matrix);
            if (report_progress)
                pb = new ProgressBar("Finding nearest neighbors", n);
            Mat sorted_k_i(n, 1);
            for (int i = 0; i < n; i++) {
                sorted_k_i << gram_matrix(i);
                negateElements(sorted_k_i);       // For sorting.
                partialSortRows(sorted_k_i, knn);
                knn_kernel_values[i] = - sorted_k_i(knn - 1, 0);
                if (report_progress)
                    pb->update(i+1);
            }
            if (cache_gram_matrix) {
                // Since we have the Gram matrix at hand, we may cache it now.
                thresholdGramMatrix(gram_matrix);
                gram_matrix_is_cached = true;
            } else
                // Free memory.
                gram_matrix = Mat();
        } else {
            // Computing the whole Gram matrix will probably not fit in memory,
            // or we do not even want / afford to compute it.
            if (cache_gram_matrix) {
                // We will cache the sparse Gram matrix.
                sparse_gram_matrix.resize(n);
                for (int i = 0; i < n; i++)
                    sparse_gram_matrix[i].resize(0,2);
            }
            if (report_progress)
                pb = new ProgressBar("Computing Gram matrix of source kernel and "
                                     "finding nearest neighbors", n);
            int n_used = knn_approx ? n_approx : n;
            Mat k_i_mat(n_used, 1);
            Vec k_i(n_used);
            Vec row(2);
            TVec<int> neighb_i, neighb_j;
            PP<PRandom> random = PRandom::common(false); // Has fixed seed.
            for (int i = 0; i < n; i++) {
                if (knn_approx) {
                    for (int j = 0; j < n_approx; j++) {
                        int k = random->uniform_multinomial_sample(n);
                        k_i[j] = source_kernel->evaluate_i_j(i,k);
                    }
                } else {
                    for (int j = 0; j < n; j++)
                        k_i[j] = source_kernel->evaluate_i_j(i,j);
                }
                k_i_mat << k_i;
                negateElements(k_i_mat);  // For sorting.
                partialSortRows(k_i_mat, knn_sub);
                knn_kernel_values[i] = - k_i_mat(knn_sub - 1, 0);
                if (report_progress)
                    pb->update(i+1);
                if (cache_gram_matrix) {
                    if (knn_approx)
                        PLERROR("In ThresholdedKernel::setDataForKernelMatrix "
                                "- Cannot currently cache the Gram matrix when"
                                " using the knn approximation");
                    PLASSERT( !knn_approx );
                    // Let us cache the sparse Gram matrix.
                    if (!fast_exact_is_equal(threshold, 0))
                        PLWARNING("In ThresholdedKernel::setDataForKernelMatrix - The sparse "
                                  "Gram matrix will be cached based on a non-zero threshold");
                    real k_min = knn_kernel_values[i];
                    Mat& g_i = sparse_gram_matrix[i];
                    int ki = g_i.length();
                    neighb_i.resize(ki);
                    for (int j = 0; j < ki; j++)
                        neighb_i[j] = int(g_i(j,0));
                    for (int j = 0; j < n; j++)
                        if (k_i[j] >= k_min && neighb_i.find(j) == -1) {
                            row[0] = j;
                            row[1] = k_i[j];
                            g_i.appendRow(row);
                            Mat& g_j = sparse_gram_matrix[j];
                            int kj = g_j.length();
                            bool already_there = false;
                            for (int l = 0; l < kj; l++)
                                if (i == int(g_j(0,1))) {
                                    already_there = true;
                                    break;
                                }
                            if (!already_there) {
                                row[0] = i;
                                g_j.appendRow(row);
                            }
                        }
                    sparse_gram_matrix_is_cached = true;
                }
            }
        }
    }
    k_x_xi.resize(knn_approx ? n_approx : n);
    k_x_xi_mat = k_x_xi.toMat(k_x_xi.length(), 1);
    PLASSERT( !knn_kernel_values.hasMissing() );
}

/////////////////////////
// thresholdGramMatrix //
/////////////////////////
void ThresholdedKernel::thresholdGramMatrix(const Mat& K) const {
    PP<ProgressBar> pb;
    int n = K.length();
    if (K.width() != n)
        PLERROR("In ThresholdedKernel::thresholdGramMatrix - A square matrix is expected");
    if (report_progress)
        pb = new ProgressBar("Thresholding Gram matrix", n);
    if (method == "knn") {
        for (int i = 0; i < n; i++) {
            real* K_i = K[i];
            real knn_kernel_values_i = knn_kernel_values[i];
            for (int j = 0; j < n; j++, K_i++)
                if (*K_i < knn_kernel_values_i && *K_i < knn_kernel_values[j])
                    *K_i = threshold;
            if (report_progress)
                pb->update(i+1);
        }
    }
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
