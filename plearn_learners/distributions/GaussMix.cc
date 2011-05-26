// -*- C++ -*-

// GaussMix.cc
//
// Copyright (C) 2003 Julien Keable
// Copyright (C) 2004-2006 University of Montreal
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

/*! \file GaussMix.cc */
#include "GaussMix.h"

#include <limits>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>

#include <plearn/io/load_and_save.h>
#include <plearn/math/Cholesky_utils.h>
#include <plearn/math/pl_erf.h>   //!< For gauss_log_density_stddev().
#include <plearn/math/plapack.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
//#include <plearn/sys/Profiler.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn/vmat/ReorderByMissingVMatrix.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/vmat/VMat_basic_stats.h>
#if 0
#include <plearn/vmat/SortRowsVMatrix.h>
#endif

namespace PLearn {
using namespace std;

#define TYPE_UNKNOWN    0
#define TYPE_SPHERICAL  1
#define TYPE_DIAGONAL   2
#define TYPE_GENERAL    3
//#define DIRECTED_HACK

//////////////
// GaussMix //
//////////////
GaussMix::GaussMix():
    ptimer(new PTimer()),
    type_id(TYPE_UNKNOWN),
    previous_predictor_part_had_missing(false),
    D(-1),
    n_eigen_computed(-1),
    nsamples(-1),
    alpha_min(1e-6),
    efficient_k_median(1),
    efficient_k_median_iter(100),
    efficient_missing(0),
    epsilon(1e-6),
    f_eigen(0),
    impute_missing(false),
    kmeans_iterations(5),
    L(1),
    max_samples_in_cluster(-1),
    min_samples_in_cluster(1),
    n_eigen(-1),
    sigma_min(1e-6),
    type("spherical")
{
    // Change the default value of 'nstages' to 10 to make the user aware that
    // in general it should be higher than 1.
    nstages = 10;
    current_training_sample = -1;
    previous_training_sample = -2; // Only use efficient_missing in training.
    ptimer->newTimer("init_time");
    ptimer->newTimer("training_time");
}

PLEARN_IMPLEMENT_OBJECT(GaussMix,
    "Gaussian mixture, either set non-parametrically or trained by EM.",
    "GaussMix implements a mixture of L Gaussians.\n"
    "There are 3 possible parameterization types:\n"
    " - spherical : Gaussians have covariance = diag(sigma^2).\n"
    "               Parameter used : sigma.\n"
    " - diagonal  : Gaussians have covariance = diag(sigma_1^2...sigma_d^2).\n"
    "               Parameter used : diags.\n"
    " - general   : Gaussians have an unconstrained covariance matrix.\n"
    "               The user specifies the number 'n_eigen' of eigenvectors\n"
    "               kept when performing the eigen-decomposition of the\n"
    "               the covariance matrix. The remaining eigenvectors are\n"
    "               considered as having a fixed eigenvalue equal to the\n"
    "               next highest eigenvalue in the decomposition.\n"
    "\n"
    "Some parameters are common to all 3 types :\n"
    " - alpha   : the weight of the Gaussians (= P(j)).\n"
    " - center  : the mean of the Gaussians\n"
    "\n"
    "If a GaussMix is not meant to be trained, its stage should be set to a\n"
    "strictly positive value, in order to indicate that it is ready to be\n"
    "used (of course, this means all parameters are properly set).\n"
    /* TODO Keep this cost? How?
    "\n"
    "In addition to the usual costs inherited from PDistribution, an additional output\n"
    "can be computed by using the character 'p' in the 'outputs_def' option: this will\n"
    "return an output containing the posterior log-probabilities P(j|Y,X) of each Gaussian.\n"
    */
);

////////////////////
// declareOptions //
////////////////////
void GaussMix::declareOptions(OptionList& ol)
{
    // Build options.

    declareOption(ol, "L", &GaussMix::L, OptionBase::buildoption,
        "Number of Gaussians in the mixture.");

    declareOption(ol, "type", &GaussMix::type, OptionBase::buildoption,
        "This is the type of covariance matrix for each Gaussian:\n"
        "   - spherical : spherical covariance matrix sigma^2 * I\n"
        "   - diagonal  : diagonal covariance matrix, given by standard\n"
        "                 deviations 'diags'\n"
        "   - general   : unconstrained covariance matrix (defined by its\n"
        "                 eigenvectors)\n");

    declareOption(ol, "n_eigen", &GaussMix::n_eigen, OptionBase::buildoption,
        "If type is 'general', the number of eigenvectors used to compute\n"
        "the covariance matrix. The remaining eigenvectors will be given an\n"
        "eigenvalue equal to the next highest eigenvalue. If set to -1, all\n"
        "eigenvectors will be kept.");

    declareOption(ol, "f_eigen", &GaussMix::f_eigen, OptionBase::buildoption,
        "If == 0, is ignored. Otherwise, it must be a fraction representing\n"
        "the fraction of eigenvectors that are kept (this value overrides\n"
        "any setting of the 'n_eigen' option).");

    declareOption(ol, "efficient_missing", &GaussMix::efficient_missing,
                                           OptionBase::buildoption,
        "If not 0, computations with missing values will be more efficient:\n"
        "- 1: most efficient method\n"
        "- 2: less naive method than 0, where we compute the matrices\n"
        "     only once per missing pattern (not as good as 1)\n"
        "- 3: same as 1, but using inverse variance lemma instead of\n"
        "     Cholesky (could be more efficient after all)");

    declareOption(ol, "efficient_k_median", &GaussMix::efficient_k_median,
                                            OptionBase::buildoption,
        "Starting number of clusters used.");

    declareOption(ol, "max_samples_in_cluster",
                  &GaussMix::max_samples_in_cluster,
                  OptionBase::buildoption,
        "Maximum number of samples allowed in each cluster (ignored if -1).\n"
        "More than 'efficient_k_median' clusters may be used in order to\n"
        "comply with this constraint.");

    declareOption(ol, "min_samples_in_cluster",
                  &GaussMix::min_samples_in_cluster,
                  OptionBase::buildoption,
        "Minimum number of samples allowed in each cluster.\n"
        "Less than 'efficient_k_median' clusters may be used in order to\n"
        "comply with this constraint.");

    declareOption(ol, "efficient_k_median_iter",
                                            &GaussMix::efficient_k_median_iter,
                                            OptionBase::buildoption,
        "Maximum number of iterations in k-median.");

    declareOption(ol, "impute_missing", &GaussMix::impute_missing,
                                        OptionBase::buildoption,
        "If true, missing values will be imputed their conditional mean when\n"
        "computing the covariance matrix. Note that even if the current\n"
        "default value of this option is false, the 'true' EM algorithm\n"
        "requires it to be set to true.");

    declareOption(ol, "kmeans_iterations", &GaussMix::kmeans_iterations,
                                           OptionBase::buildoption,
        "Maximum number of iterations performed in initial K-means.");

    declareOption(ol, "alpha_min", &GaussMix::alpha_min,
                                   OptionBase::buildoption,
        "The minimum weight for each Gaussian. Whenever a Gaussian falls\n"
        "below 'alpha_min', it is replaced by a new Gaussian. Note that a\n"
        "Gaussian may be replaced only once per stage (to avoid cycles).");

    declareOption(ol,"sigma_min", &GaussMix::sigma_min,
                                  OptionBase::buildoption,
        "The minimum standard deviation allowed. In all computations, any\n"
        "standard deviation below 'sigma_min' (or variance below its square)\n"
        "will be replaced by 'sigma_min' (or its square). This regularizes\n"
        "the Gaussians (and should not be too high nor too small).");

    declareOption(ol, "epsilon", &GaussMix::epsilon, OptionBase::buildoption,
        "A small number to check for near-zero probabilities.");

    // Learnt options.

    declareOption(ol, "alpha", &GaussMix::alpha, OptionBase::learntoption,
        "Coefficients of the Gaussians. They sum to 1 and are positive:\n"
        "they can be interpreted as priors P(Gaussian j).");

    declareOption(ol, "center", &GaussMix::center, OptionBase::learntoption,
        "Mean of each Gaussian, stored in rows.");

    declareOption(ol, "sigma", &GaussMix::sigma, OptionBase::learntoption,
        "The standard deviation in all directions, for 'spherical' type.\n");

    declareOption(ol,"diags", &GaussMix::diags, OptionBase::learntoption,
        "Element (j,k) is the standard deviation of Gaussian j on the k-th\n"
        "dimension, for 'diagonal' type.");

    declareOption(ol, "eigenvalues", &GaussMix::eigenvalues,
                                     OptionBase::learntoption,
        "The eigenvalues associated with the principal eigenvectors:\n"
        "element (j,k) is the k-th eigenvalue of the j-th Gaussian.");

    declareOption(ol, "eigenvectors", &GaussMix::eigenvectors,
                                      OptionBase::learntoption,
        "Principal eigenvectors of each Gaussian (for the 'general' type).\n"
        "Element j is a matrix whose row k is the k-th eigenvector of the\n"
        "j-th Gaussian.");

    /*
    declareOption(ol, "log_coeff", &GaussMix::log_coeff,
                                   OptionBase::nosave,
        "The logarithm of the constant part in the joint Gaussian density:\n"
        "log(1/sqrt(2*pi^D * Det(C))).");

    declareOption(ol, "log_p_j_x", &GaussMix::log_p_j_x,
                                   OptionBase::nosave,
        "The logarithm of p(j|x), where x is the input part.");

    declareOption(ol, "p_j_x", &GaussMix::p_j_x, OptionBase::nosave,
        "The probability p(j|x), where x is the input part (it is computed\n"
        "by exp(log_p_j_x).");
    */

    declareOption(ol, "n_eigen_computed", &GaussMix::n_eigen_computed,
                                          OptionBase::learntoption,
        "Actual number of principal components computed with 'general' type.\n"
        "It is either equal to the dimension (when all components are\n"
        "computed), or to n_eig+1.");

    declareOption(ol, "D", &GaussMix::D, OptionBase::learntoption,
        "Number of dimensions of the joint distribution.");

    /*
    // We should not have to save this (it is computed in setPredictor).
    declareOption(ol, "center_y_x", &GaussMix::center_y_x, OptionBase::nosave,
        "The expectation E[Y | x] for each Gaussian.");
        */

    /*
    declareOption(ol, "log_p_x_j_alphaj", &GaussMix::log_p_x_j_alphaj,
                                          OptionBase::learntoption,
        "The logarithm of p(x|j) * alpha_j, where x is the input part.");

    declareOption(ol, "n_tries", &GaussMix::n_tries, OptionBase::learntoption,
        "Element i is the number of iterations needed to complete\n"
        "stage i (if > 1, some Gaussian has been replaced).");

    declareOption(ol, "nsamples", &GaussMix::nsamples,
                                  OptionBase::learntoption,
        "Number of samples in the training set.");

    declareOption(ol, "training_time", &GaussMix::training_time,
                                       OptionBase::learntoption,
        "Time spent in training the model. If initially set to a negative\n"
        "value, it will not be updated during training.");

    declareOption(ol, "conditional_updating_time",
                      &GaussMix::conditional_updating_time,
                      OptionBase::learntoption,
        "Time spent in updating from conditional sorting. If initially set\n"
        "to a negative value, it will not be updated during training.");
    */

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void GaussMix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void GaussMix::build_()
{
    // Check type value.
    if (type == "spherical") {
        type_id = TYPE_SPHERICAL;
    } else if (type == "diagonal") {
        type_id = TYPE_DIAGONAL;
    } else if (type == "general") {
        type_id = TYPE_GENERAL;
    } else
        PLERROR("In GaussMix::build_ - Type '%s' is unknown", type.c_str());

    // Special case for the 'f_eigen' option: 1 means we keep everything.
    PLASSERT( f_eigen >= 0 && f_eigen <= 1 );
    if (is_equal(f_eigen, 1))
        n_eigen = -1;

    // Guess values for 'D' and 'n_eigen_computed' if they are not provided
    // (this could be the case for instance when specifying 'by hand' the
    // parameters of the mixture of Gaussians).
    // Make also a few checks to ensure all values are coherent.
    if (stage > 0) {
        PLASSERT( D == -1 || D == center.width() );
        if (D == -1)
            D = center.width();
        PLASSERT( n_eigen_computed == -1 ||
                n_eigen_computed == eigenvalues.width() );
        if (n_eigen_computed == -1)
            n_eigen_computed = eigenvalues.width();
        PLASSERT( n_eigen == -1 || n_eigen_computed <= n_eigen + 1 );
        PLASSERT( n_eigen_computed <= D );
    }

    // Make sure everything is correctly resized before using the object.
    resizeDataBeforeUsing();

    // If the learner is ready to be used, we need to precompute the logarithm
    // of the constant coefficient of each Gaussian.
    if (stage > 0)
        precomputeAllGaussianLogCoefficients();

    // Make GaussMix-specific operations for conditional distributions.
    GaussMix::setPredictorPredictedSizes(predictor_size, predicted_size, false);
    GaussMix::setPredictor(predictor_part, false);
}

///////////////////
// changeOptions //
///////////////////
void GaussMix::changeOptions(const map<string,string>& name_value)
{
    // When 'n_eigen' is changed for a learner that is already trained, we need
    // to call forget(), otherwise some asserts may fail during a subsequent
    // build.
    if (stage > 0 && (name_value.find("n_eigen") != name_value.end() ||
                      name_value.find("L")       != name_value.end() ||
                      name_value.find("seed")    != name_value.end() ||
                      name_value.find("sigma_min")!=name_value.end() ||
                      name_value.find("type")    != name_value.end() ))
        forget();
    inherited::changeOptions(name_value);
}

////////////////////////////////
// computeMeansAndCovariances //
////////////////////////////////
void GaussMix::computeMeansAndCovariances() {
    //Profiler::start("computeMeansAndCovariances");
    VMat weighted_train_set;
    Vec sum_columns(L);
    Vec storage_D(D);
    columnSum(posteriors, sum_columns);
    for (int j = 0; j < L; j++) {
        // Build the weighted dataset.
        if (sum_columns[j] < epsilon)
            PLWARNING("In GaussMix::computeMeansAndCovariances - A posterior "
                      "is almost zero");
        PLASSERT( !updated_weights(j).hasMissing() );
        VMat weights(columnmatrix(updated_weights(j)));
        bool use_impute_missing = impute_missing && stage > 0;
        VMat input_data = use_impute_missing ? imputed_missing[j]
                                             : train_set;

        /*
        input_data->saveAMAT("/u/delallea/tmp/input_data_" +
                tostring(this->stage) + ".amat", false, true);
        */

        weighted_train_set = new ConcatColumnsVMatrix(
            new SubVMatrix(input_data, 0, 0, nsamples, D), weights);
        weighted_train_set->defineSizes(D, 0, 1);
        Vec center_j = center(j);
        if (type_id == TYPE_SPHERICAL) {
            computeInputMeanAndVariance(weighted_train_set, center_j,
                                                            storage_D);
            // TODO Would it be better to use an harmonic mean?
            sigma[j] = sqrt(mean(storage_D));
            if (isnan(sigma[j]))
                PLERROR("In GaussMix::computeMeansAndCovariances - A "
                        "standard deviation is 'nan'");
        } else if (type_id == TYPE_DIAGONAL ) {
            computeInputMeanAndStddev(weighted_train_set, center_j,
                                                          storage_D);
            diags(j) << storage_D;
            if (storage_D.hasMissing())
                PLERROR("In GaussMix::computeMeansAndCovariances - A "
                        "standard deviation is 'nan'");
        } else {
            PLASSERT( type_id == TYPE_GENERAL );
            //Profiler::start("computeInputMeanAndCovar");
            computeInputMeanAndCovar(weighted_train_set, center_j, covariance);
            //Profiler::end("computeInputMeanAndCovar");
            if (use_impute_missing) {
                // Need to add the extra contributions.
                if (sum_of_posteriors[j] > 0) {
                    error_covariance[j] /= sum_of_posteriors[j];
                    PLASSERT( covariance.isSymmetric() );
                    PLASSERT( error_covariance[j].isSymmetric() );
                    covariance += error_covariance[j];
                    PLASSERT( covariance.isSymmetric() );
                }
            }
            if (center_j.hasMissing()) {
                // There are features missing in all points assigned to this
                // Gaussian. We sample a new random value for these features.
                for (int i = 0; i < D; i++)
                    if (is_missing(center_j[i])) {
                        center_j[i] =
                            random_gen->gaussian_mu_sigma(mean_training  [i],
                                                          stddev_training[i]);
#ifdef BOUNDCHECK
                        // Sanity check: the corresponding row and column in
                        // the covariance matrix should be missing.
                        for (int k = 0; k < D; k++) {
                            if (!is_missing(covariance(i,k)) ||
                                    !is_missing(covariance(k,i)))
                                PLERROR(
                                    "In GaussMix::computeMeansAndCovariances -"
                                    " Expected a missing value in covariance");
                        }
#endif
                    }
            }
            if (covariance.hasMissing())
                // The covariance matrix may have some missing values when not
                // enough samples were seen to get simultaneous observations of
                // some pairs of features.
                // Those missing values are replaced with zero.
                for (int i = 0; i < D; i++)
                    for (int k = i; k < D; k++)
                        if (is_missing(covariance(i,k))) {
                            covariance(i,k) = 0;
                            PLASSERT( is_missing(covariance(k,i)) ||
                                    covariance(k,i) == 0 );
                            covariance(k,i) = 0;
                        }
#ifdef BOUNDCHECK

            // At this point there should be no more missing values.
            if (covariance.hasMissing() || center.hasMissing())
                PLERROR("In GaussMix::computeMeansAndCovariances - Found "
                        "missing values when computing weighted mean and "
                        "covariance");
#endif
            // 'eigenvals' points to the eigenvalues of the j-th Gaussian.
            Vec eigenvals = eigenvalues(j);
            eigenVecOfSymmMat(covariance, n_eigen_computed, eigenvals,
                                                            eigenvectors[j]);
            PLASSERT( eigenvals.length() == n_eigen_computed );

            // Currently, the returned covariance matrix returned is not
            // guaranteed to be semi-definite positive. Thus we need to ensure
            // it is the case, by thresholding the negative eigenvalues to the
            // smallest positive one.
            for (int i = n_eigen_computed - 1; i >= 0; i--)
                if (eigenvals[i] > 0) {
                    for (int k = i + 1; k < n_eigen_computed; k++)
                        eigenvals[k] = eigenvals[i];
                    break;
                }
        }
    }
    //Profiler::end("computeMeansAndCovariances");
}

////////////////////////////////
// updateCholeskyFromPrevious //
////////////////////////////////
void GaussMix::updateCholeskyFromPrevious(
        const Mat& chol_previous, Mat& chol_updated,
        const Mat& full_matrix,
        const TVec<int>& indices_previous, const TVec<int>& indices_updated)
        const
{
    //Profiler::start("updateCholeskyFromPrevious");
    static TVec<bool> is_previous;
    static TVec<bool> is_updated;
    static TVec<int> indices_new;
    static Vec new_row;
    PLASSERT( chol_previous.length() == indices_previous.length() );
    if (indices_updated.isEmpty()) {
        // All values are missing: the returned matrix should be empty.
        chol_updated.resize(0, 0);
        //Profiler::end("updateCholeskyFromPrevious");
        return;
    }
    // Initialization.
    int n = chol_previous.length();
    int max_indice = -1;
    if (!indices_previous.isEmpty())
        max_indice = max(max_indice, max(indices_previous));
    if (!indices_updated.isEmpty())
        max_indice = max(max_indice, max(indices_updated));
    PLASSERT( max_indice >= 0 );
    is_updated.resize(max_indice + 1);
    is_previous.resize(max_indice + 1);
    is_updated.fill(false);
    is_previous.fill(false);
    indices_new.resize(0);
    // Find which indices need to be kept or appended.
    int p = indices_updated.length();
    for (int i = 0; i < p; i++)
        is_updated[indices_updated[i]] = true;
    for (int i = 0; i < n; i++)
        is_previous[indices_previous[i]] = true;
    // Delete unused dimensions from the Cholesky decomposition.
    //Profiler::start("updateCholeskyFromPrevious - Removing dimensions");
    chol_updated.resize(n, n);
    chol_updated << chol_previous;
    for (int i = n - 1; i >= 0; i--) {
        int dim_to_del = indices_previous[i];
        if (is_updated[dim_to_del])
            indices_new.append(dim_to_del);
        else
            choleskyRemoveDimension(chol_updated, i);
    }
    //Profiler::end("updateCholeskyFromPrevious - Removing dimensions");
    // Need to swap 'indices_new' since these indices have been added in the
    // opposite order.
    indices_new.swap();
    // Now add dimensions that were not here previously.
    //Profiler::start("updateCholeskyFromPrevious - Adding dimensions");
    for (int i = 0; i < p; i++)
        if (!is_previous[indices_updated[i]]) {
            int dim_to_add = indices_updated[i];
            indices_new.append(dim_to_add);
            int q = indices_new.length();
            new_row.resize(q);
            for (int j = 0; j < q; j++)
                new_row[j] = full_matrix(dim_to_add, indices_new[j]);
            choleskyAppendDimension(chol_updated, new_row);
        }
    //Profiler::end("updateCholeskyFromPrevious - Adding dimensions");
    // Finally update the 'indices_updated' list.
    indices_updated << indices_new;
    //Profiler::end("updateCholeskyFromPrevious");
}

///////////////////////////////////////
// updateInverseVarianceFromPrevious //
///////////////////////////////////////
// TODO Document
// Also, note that 'ind_dst' is going to be modified to reflect the reordering
// of dimensions..
void GaussMix::updateInverseVarianceFromPrevious(
        const Mat& src, Mat& dst, const Mat& full,
        const TVec<int>& ind_src, const TVec<int>& ind_dst,
        real* src_log_det, real* dst_log_det) const
{
    // The i-th element of 'is_src' ('is_dst') indicates whether the i-th
    // dimension is in the 'ind_src'('ind_dst') vector.
    static TVec<bool> is_src;
    static TVec<bool> is_dst;

    static TVec<int> dim_common;    // List of common dimensions.
    static TVec<int> dim_src_only;  // List of dimensions only in 'src'.
    static TVec<int> dim_dst_only;  // List of dimensions only in 'dst'.
    // List of dimensions in 'src' after it has been reordered so that the
    // common dimensions are first.
    static TVec<int> dim_reordered_src;

    // A copy of the 'src' matrix, but whose dimensions have been swapped to
    // match the order in 'dim_reordered_src'.
    static Mat src_reordered;
    
    // Temporary storage matrices.
    static Mat tmp;
    static Mat tmp2; 

    // This matrix will contain the inverse covariance corresponding to the
    // removal of dimensions who do not appear in 'ind_dst' (thus, it is the
    // final result if no dimension has to be added, otherwise it is just an
    // intermediate result).
    static Mat dst_only_removed;

    // Matrix storing the bottom-right part of the reordered source matrix
    // (corresponding to dimensions that need to be removed).
    static Mat B3;

    // Work matrices.
    static Mat W;
    static Mat P;
    static Mat B;

    // Safety checks.
    PLASSERT( src.length() == ind_src.length() );
    PLASSERT( (src_log_det  &&  dst_log_det) ||
            (!src_log_det && !dst_log_det) );

    if (src_log_det)
        // Initialize destination determinant to the source one.
        *dst_log_det = *src_log_det;

    int n = ind_src.length();
    int p = ind_dst.length();
    // int m = full.length();
    dst.resize(p, p);
    // Analyze the indices vectors.
    int max_indice = -1;
    if (!ind_src.isEmpty())
        max_indice = max(max_indice, max(ind_src));
    if (!ind_dst.isEmpty())
        max_indice = max(max_indice, max(ind_dst));
    // Note that 'max_indice' can be -1. This can currently happen if
    // the first sample in a cluster has no missing value.
    // In this case there is nothing to do: 'dst' will be empty.
    is_dst.resize(max_indice + 1);
    is_src.resize(max_indice + 1);
    is_dst.fill(false);
    is_src.fill(false);
    for (int i = 0; i < p; i++)
        is_dst[ind_dst[i]] = true;
    for (int i = 0; i < n; i++)
        is_src[ind_src[i]] = true;
    // Build the source inverse covariance matrix where dimensions are
    // reordered so that the first dimensions are those in common between
    // source and destination.
    dim_common.resize(0);
    dim_src_only.resize(0);
    dim_reordered_src.resize(n);
    for (int i = 0; i < n; i++) {
        if (is_dst[ind_src[i]])
            dim_common.append(i);
        else
            dim_src_only.append(i);
    }
    dim_reordered_src.subVec(0, dim_common.length()) << dim_common;
    dim_reordered_src.subVec(dim_common.length(), dim_src_only.length())
        << dim_src_only;
    src_reordered.setMod(dim_reordered_src.length());
    src_reordered.resize(dim_reordered_src.length(),
                         dim_reordered_src.length());
    for (int i = 0; i < dim_reordered_src.length(); i++) {
        int dim_reordered_src_i = dim_reordered_src[i];
        src_reordered(i, i) = src(dim_reordered_src_i, dim_reordered_src_i);
        for (int j = i + 1; j < dim_reordered_src.length(); j++) {
            real elem_i_j = src(dim_reordered_src_i, dim_reordered_src[j]);
            src_reordered(i, j) = elem_i_j;
            src_reordered(j, i) = elem_i_j;
        }
    }
        /* Old code doing the same thing.
    tmp.resize(src.length(), dim_reordered_src.length());
    // TODO Not efficient! Optimize!
    selectColumns(src, dim_reordered_src, tmp);
    src_reordered.resize(n, n);
    selectRows(tmp, dim_reordered_src, src_reordered);
    */

    // Remove the dimensions that are not present in the destination
    // matrix.
    int n_common = dim_common.length();
    dst_only_removed.resize(n_common, n_common);
    int n_src_only = dim_src_only.length();
    if (n_src_only == 0) {
        // Nothing to remove.
        dst_only_removed << src_reordered;
    } else {
        // Compute the matrix corresponding to the removal of the dimensions
        // that appear only in the source matrix.
        PLASSERT( src_reordered.isSymmetric() );
        Mat B1 = src_reordered.subMat(0, 0, n_common, n_common);
        Mat B2 = src_reordered.subMat(0, n_common, n_common, n_src_only);
        B3.setMod(n_src_only);
        B3.resize(n_src_only, n_src_only);
        B3 << src_reordered.subMat(n_common, n_common, n_src_only, n_src_only);
        PLASSERT( B3.isSymmetric() );
        dst_only_removed << B1;
        tmp.resize(B3.length(), B3.width());
        matInvert(B3, tmp);
        // Another commented-out assert due to it possibly failing (numerical
        // imprecisions).
        // PLASSERT( tmp.isSymmetric(false) );
        fillItSymmetric(tmp);
        tmp2.resize(tmp.length(), B2.length());
        productTranspose(tmp2, tmp, B2);
        tmp.resize(B2.length(), tmp2.width());
        product(tmp, B2, tmp2);
        dst_only_removed -= tmp;
        // Another commented-out assert due to it possibly failing (numerical
        // imprecisions).
        // PLASSERT( dst_only_removed.isSymmetric(false, true) );
        fillItSymmetric(dst_only_removed);
        // Update the log-determinant if needed.
        if (src_log_det) {
            //Profiler::start("det when removing");
            *dst_log_det += det(src_reordered.subMat(n_common, n_common,
                                                     n_src_only, n_src_only),
                                true);
            //Profiler::end("det when removing");
        }
    }

    // At this point, the dimensions that are not present in the
    // destination matrix have been removed. Now, we need to add the
    // dimensions that need to be added (those that are present in the
    // destination but not in the source).
    dim_dst_only.resize(0);
    for (int i = 0; i < p; i++)
        if (!is_src[ind_dst[i]])
            dim_dst_only.append(ind_dst[i]);
    int n_dst_only = dim_dst_only.length();
    // Reorder properly the indices in 'ind_dst': first the common indices,
    // then those only in 'dst'.
    for (int i = 0; i < n_common; i++)
        ind_dst[i] = ind_src[dim_common[i]];
    for (int i = 0; i < n_dst_only; i++)
        ind_dst[i + n_common] = dim_dst_only[i];
    // Replace dimensions in 'src' by dimensions in the full matrix.
    for (int i = 0; i < dim_common.length(); i++)
        dim_common[i] = ind_src[dim_common[i]];
    if (n_dst_only == 0) {
        // No dimension to add.
        dst << dst_only_removed;
    } else {
        // TODO This is probably not very efficient, and could be optimized.
        tmp.resize(full.length(), dim_dst_only.length());
        selectColumns(full, dim_dst_only, tmp);
        W.resize(dim_common.length(), tmp.width());
        selectRows(tmp, dim_common, W);
        P.resize(dim_dst_only.length(), tmp.width());
        selectRows(tmp, dim_dst_only, P);
        B.resize(W.width(), dst_only_removed.width());
        transposeProduct(B, W, dst_only_removed);
        tmp.setMod(W.width());
        tmp.resize(B.length(), W.width());
        // It can happen that n_common == 0, i.e. there are no common
        // dimensions. In such a case, P contains the desired covariance.
        if (n_common > 0) {
            product(tmp, B, W);
            negateElements(tmp);
        } else
            tmp.fill(0);
        tmp += P;
        tmp2.resize(tmp.length(), tmp.width());
        // Commented-out as it may cause an unwanted crash.
        // PLASSERT( tmp.isSymmetric(false, true) );
        fillItSymmetric(tmp);
        matInvert(tmp, tmp2);
        // Commented-out as it may cause an unwanted crash.
        // PLASSERT( tmp2.isSymmetric(false) );
        fillItSymmetric(tmp2);
        dst.subMat(n_common, n_common, n_dst_only, n_dst_only) << tmp2;
        if (n_common > 0) {
            tmp.resize(B.width(), tmp2.width());
            transposeProduct(tmp, B, tmp2);
            tmp2.resize(tmp.length(), B.width());
            product(tmp2, tmp, B);
            negateElements(tmp);
            dst.subMat(0, n_common, n_common, n_dst_only) << tmp;
            Mat dst_top_left = dst.subMat(0, 0, n_common, n_common);
            dst_top_left << tmp2;
            dst_top_left += dst_only_removed;
        }
        // Update the log-determinant if needed.
        if (src_log_det) {
            //Profiler::start("det when adding");
            *dst_log_det -= det(dst.subMat(n_common,   n_common,
                                           n_dst_only, n_dst_only), true);
            //Profiler::end("det when adding");
        }
    }
    // Ensure 'dst' is symmetric, since we did not fill the bottom-left block.
    fillItSymmetric(dst);
}

/////////////////////
// addToCovariance //
/////////////////////
void GaussMix::addToCovariance(const Vec& y, int j,
                               const Mat& cov, real post)
{
    //Profiler::start("addToCovariance");
    PLASSERT( y.length() == cov.length() && y.length() == cov.width() );
    PLASSERT( n_predictor == 0 );
    PLASSERT( impute_missing );
    static TVec<int> coord_missing;
    static Mat inv_cov_y_missing;
    static Mat H_inv_tpl;
    static TVec<int> ind_inv_tpl;
    static Mat H_inv_tot;
    static TVec<int> ind_inv_tot;

    coord_missing.resize(0);
    for (int k = 0; k < y.length(); k++)
        if (is_missing(y[k]))
            coord_missing.append(k);

    Mat& inv_cov_y = joint_inv_cov[j];
    if (previous_training_sample == -1) {
        int n_missing = coord_missing.length();
        inv_cov_y_missing.setMod(n_missing);
        inv_cov_y_missing.resize(n_missing, n_missing);
        for (int k = 0; k < n_missing; k++)
            for (int q = 0; q < n_missing; q++)
                inv_cov_y_missing(k,q) =
                    inv_cov_y(coord_missing[k], coord_missing[q]);
        cond_var_inv_queue.resize(1);
        Mat& cond_inv = cond_var_inv_queue[0];
        cond_inv.resize(inv_cov_y_missing.length(), inv_cov_y_missing.width());
        matInvert(inv_cov_y_missing, cond_inv);
        // Take care of numerical imprecisions that may cause the inverse not
        // to be exactly symmetric.
        PLASSERT( cond_inv.isSymmetric(false, true) );
        fillItSymmetric(cond_inv);
        indices_inv_queue.resize(1);
        TVec<int>& ind = indices_inv_queue[0];
        ind.resize(n_missing);
        ind << coord_missing;
    }

    int path_index =
        sample_to_path_index[current_training_sample];
    int queue_index;
    if (spanning_use_previous[current_cluster][path_index])
        queue_index = cond_var_inv_queue.length() - 1;
    else
        queue_index = cond_var_inv_queue.length() - 2;

    H_inv_tpl = cond_var_inv_queue[queue_index];
    ind_inv_tpl = indices_inv_queue[queue_index];
    int n_inv_tpl = H_inv_tpl.length();
    H_inv_tot.resize(n_inv_tpl, n_inv_tpl);
    ind_inv_tot = coord_missing;

    bool same_covariance = no_missing_change[current_training_sample];

    if (!same_covariance)
        updateInverseVarianceFromPrevious(H_inv_tpl, H_inv_tot,
                joint_inv_cov[j], ind_inv_tpl, ind_inv_tot);

    Mat* the_H_inv = same_covariance ? &H_inv_tpl : &H_inv_tot;
    TVec<int>* the_ind_inv = same_covariance? &ind_inv_tpl : &ind_inv_tot;

    // Add this matrix (weighted by the coefficient 'post') to the given 'cov'
    // full matrix.
    for (int i = 0; i < the_ind_inv->length(); i++) {
        int the_ind_inv_i = (*the_ind_inv)[i];
        for (int k = 0; k < the_ind_inv->length(); k++)
            cov(the_ind_inv_i, (*the_ind_inv)[k]) += post * (*the_H_inv)(i, k);
    }

    bool cannot_free =
        !spanning_can_free[current_cluster][path_index];
    if (cannot_free)
        queue_index++;
    cond_var_inv_queue.resize(queue_index + 1);
    indices_inv_queue.resize(queue_index + 1);

    static Mat dummy_mat;
    H_inv_tpl = dummy_mat;

    if (!same_covariance || cannot_free) {
        Mat& M = cond_var_inv_queue[queue_index];
        M.resize(H_inv_tot.length(), H_inv_tot.width());
        M << H_inv_tot;
        TVec<int>& ind = indices_inv_queue[queue_index];
        ind.resize(the_ind_inv->length());
        ind << *the_ind_inv;
    }

    //Profiler::end("addToCovariance");
}

//////////////////////////
// computeLogLikelihood //
//////////////////////////
real GaussMix::computeLogLikelihood(const Vec& y, int j, bool is_predictor) const
{
    //Profiler::start("computeLogLikelihood");
    static int size;    // Size of the vector whose density is computed.
    // Index where we start (usually 0 when 'is_predictor', and 'n_predictor'
    // otherwise).
    static int start;
    // Storage of mean.
    static Vec mu_y;
    static Vec mu;

    static Vec diag_j; // Points to the standard deviations of Gaussian j.

    // Used to point to the correct eigenvalues / eigenvectors.
    static Vec eigenvals;
    static Mat eigenvecs;

    // Stuff when there are missing values: we need to do a lot more
    // computations (with the current rather dumb implementation).
    static Vec mu_y_missing;
    static Mat cov_y_missing;
    static Mat dummy_storage;
    static TVec<Mat> covs_y_missing;
    static TVec<Vec> mus_y_missing;
    static Vec y_missing;
    static Vec eigenvals_missing;
    static TVec<Vec> eigenvals_allj_missing;
    static Mat* eigenvecs_missing;
    static Mat eigenvecs_missing_storage;
    static TVec<Mat> eigenvecs_allj_missing;
    static TVec<int> non_missing;
    static Mat work_mat1, work_mat2;
    static Mat eigenvalues_x_miss;
    static TVec<Mat> eigenvectors_x_miss;
    static Mat full_cov;
    static Mat cov_x_j;
    static Vec y_non_missing;
    static Vec center_non_missing;
    static Mat cov_y_x;

    // Dummy matrix and vector to release some storage pointers so that some
    // matrices can be resized.
    static Mat dummy_mat;
    static Vec dummy_vec;

    eigenvecs_missing = &eigenvecs_missing_storage;

    Mat* the_cov_y_missing = &cov_y_missing;
    Vec* the_mu_y_missing = &mu_y_missing;

    // Will contain the final result (the desired log-likelihood).
    real log_likelihood;

    if (type_id == TYPE_SPHERICAL || type_id == TYPE_DIAGONAL) {
        // Easy case: the covariance matrix is diagonal.
        if (is_predictor) {
            size = n_predictor;
            start  = 0;
        } else {
            size = n_predicted;
            start = n_predictor;
        }
        mu_y = center(j).subVec(start, size);
        if (type_id == TYPE_DIAGONAL) {
            PLASSERT( diags.length() == L && diags.width() == n_predictor+n_predicted );
            diag_j = diags(j).subVec(start, size);
        }
        log_likelihood = 0;
        // x   ~= N(mu_x, cov (diagonal))
        // y|x ~= N(mu_y, cov (diagonal))
        for (int k = 0; k < size; k++)
            if (!is_missing(y[k])) {
                real stddev =
                    type_id == TYPE_SPHERICAL ? sigma[j]
                                              : diag_j[k];
                stddev = max(sigma_min, stddev);
                log_likelihood +=
                    gauss_log_density_stddev(y[k], mu_y[k], stddev);
            }
    } else {
        PLASSERT( type_id == TYPE_GENERAL );
        log_likelihood = 0; // Initialize resultresult  to zero.
        // TODO Put both cases (n_predictor == 0 and other) in same code (they are
        // very close one to each other).
        if (n_predictor == 0) {
            // Simple case: there is no predictor part.
            PLASSERT( !is_predictor );
            PLASSERT( y.length() == n_predicted );

            // When not in training mode, 'previous_training_sample' is set to
            // -2, and 'current_training_sample' is set to -1.
            // In such a case, it is not necessary to do all computations.
            // TODO It would be good to have one single flag for both lines
            // below. Maybe current_training_sample != -1 would be enough?
            bool eff_missing = (efficient_missing == 1 ||
                                efficient_missing == 3        ) &&
                               (previous_training_sample != -2);
            bool imp_missing = impute_missing &&
                               (current_training_sample != -1);
            bool eff_naive_missing = (efficient_missing == 2) &&
                                     (current_training_sample != -1);

            if (y.hasMissing() || eff_missing || imp_missing) {
                // TODO This will probably make the 'efficient_missing' method
                // perform slower on data with no missing value. This should be
                // optimized.

                // We need to recompute almost everything.
                // First the full covariance.
                Mat& cov_y = joint_cov[j];
                Mat* inv_cov_y = impute_missing ? &joint_inv_cov[j] : 0;
                real var_min = square(sigma_min);
                if (stage_joint_cov_computed[j] != this->stage) {
                    stage_joint_cov_computed[j] = this->stage;
                    cov_y.resize(D, D);
                    eigenvals = eigenvalues(j);
                    real lambda0 = max(var_min, eigenvals.lastElement());
                    cov_y.fill(0);
                    Mat& eigenvectors_j = eigenvectors[j];

                    PLASSERT( eigenvectors_j.width() == D );

                    for (int k = 0; k < n_eigen_computed - 1; k++)
                        externalProductScaleAcc(
                                cov_y, eigenvectors_j(k), eigenvectors_j(k),
                                max(var_min, eigenvals[k]) - lambda0);

                    for (int i = 0; i < D; i++)
                        cov_y(i,i) += lambda0;

                    // By construction, the resulting matrix is symmetric. However,
                    // it may happen that it is not exactly the case due to numerical
                    // approximations. Thus we ensure it is perfectly symmetric.
                    PLASSERT( cov_y.isSymmetric(false) );
                    fillItSymmetric(cov_y);

                    if (impute_missing) {
                        // We also need to compute the inverse covariance
                        // matrix.
                        PLASSERT( inv_cov_y );
                        inv_cov_y->resize(D, D);
                        inv_cov_y->fill(0);
                        real l0 = 1 / lambda0;
                        for (int k = 0; k < n_eigen_computed - 1; k++)
                            externalProductScaleAcc(
                                    *inv_cov_y, eigenvectors_j(k),
                                    eigenvectors_j(k),
                                    1 / max(var_min, eigenvals[k]) - l0);
                        for (int i = 0; i < D; i++)
                            (*inv_cov_y)(i, i) += l0;
                        // For the same reason as above.
                        PLASSERT( inv_cov_y->isSymmetric(false) );
                        fillItSymmetric(*inv_cov_y);
                    }

                    /*
                    if (efficient_missing) {
                        // Now compute its Cholesky decomposition.
                        Mat& chol_cov_y = chol_joint_cov[j];
                        choleskyDecomposition(cov_y, chol_cov_y);

                        // And do the same for missing templates.
                        TVec<bool> miss_pattern;
                        for (int i = 0; i < efficient_k_median; i++) {
                            miss_pattern = missing_template(i);
                            int n_non_missing = miss_pattern.length();
                            non_missing.resize(0);
                            for (int k = 0; k < miss_pattern.length(); k++)
                                if (miss_pattern[k])
                                    n_non_missing--;
                                else
                                    non_missing.append(k);

                            cov_y_missing.resize(n_non_missing, n_non_missing);
                            for (int k = 0; k < n_non_missing; k++)
                                for (int q = 0; q < n_non_missing; q++)
                                    cov_y_missing(k,q) =
                                        cov_y(non_missing[k], non_missing[q]);
                            Mat& chol_cov_tpl = chol_cov_template(i, j);
                            choleskyDecomposition(cov_y_missing, chol_cov_tpl);
                        }
                    }
                    */
                }
                /*
                // Then extract what we want.
                int tpl_idx;
                TVec<bool> missing_tpl;
                if (efficient_missing) {
                PLASSERT( current_training_sample != -1 );
                tpl_idx =
                    sample_to_template[current_training_sample];
                missing_tpl = missing_template(tpl_idx);
                }
                */
                /*
                static TVec<int> com_non_missing, add_non_missing, add_missing;
                com_non_missing.resize(0);
                add_non_missing.resize(0);
                // 'add_missing' will contain those coordinate in the template
                // covariance matrix that need to be deleted (because they are
                // missing in the current template).
                add_missing.resize(0);
                */

                non_missing.resize(0);
                static TVec<int> coord_missing;
                coord_missing.resize(0);
                // int count_tpl_dim = 0;
                for (int k = 0; k < n_predicted; k++)
                    if (!is_missing(y[k]))
                        non_missing.append(k);
                    else
                        coord_missing.append(k);

                int n_non_missing = non_missing.length();
                if (eff_missing && previous_training_sample == -1) {
                    // No previous training sample: we need to compute from
                    // scratch the Cholesky decomposition.
                    the_cov_y_missing->setMod(n_non_missing);
                    the_cov_y_missing->resize(n_non_missing, n_non_missing);
                    for (int k = 0; k < n_non_missing; k++)
                        for (int q = 0; q < n_non_missing; q++)
                            (*the_cov_y_missing)(k,q) =
                                cov_y(non_missing[k], non_missing[q]);
                    cholesky_queue.resize(1);
                    // pout << "length = " << cholesky_queue.length() << endl;
                    Mat& chol = cholesky_queue[0];
                    if (efficient_missing == 1)
                        choleskyDecomposition(*the_cov_y_missing, chol);
                    else {
                        PLASSERT( efficient_missing == 3 );
                        log_det_queue.resize(1);
                        log_det_queue[0] = det(*the_cov_y_missing, true);
                        chol.resize(the_cov_y_missing->length(),
                                    the_cov_y_missing->length());
                        PLASSERT( the_cov_y_missing->isSymmetric() );
                        matInvert(*the_cov_y_missing, chol);
                        // Commenting-out this assert: it can actually fail due
                        // to some numerical imprecisions during matrix
                        // inversion, which is a bit annoying.
                        // PLASSERT( chol.isSymmetric(false, true) );
                        fillItSymmetric(chol);
                    }
                    indices_queue.resize(1);
                    TVec<int>& ind = indices_queue[0];
                    ind.resize(n_non_missing);
                    ind << non_missing;
                }

                mu_y = center(j).subVec(0, n_predicted);
                the_mu_y_missing->resize(n_non_missing);
                y_missing.resize(n_non_missing);
                // Fill in first the coordinates which are in the template,
                // then the coordinates specific to this data point.
                /*
                static TVec<int> tot_non_missing;
                if (efficient_missing) {
                tot_non_missing.resize(com_non_missing.length() +
                                       add_non_missing.length());
                tot_non_missing.subVec(0, com_non_missing.length())
                    << com_non_missing;
                tot_non_missing.subVec(com_non_missing.length(),
                                       add_non_missing.length())
                    << add_non_missing;
                for (int k = 0; k < tot_non_missing.length(); k++) {
                    mu_y_missing[k] = mu_y[tot_non_missing[k]];
                    y_missing[k] = y[tot_non_missing[k]];
                }
                }
                */
                if (!eff_missing) {
                if (!eff_naive_missing) {
                    dummy_storage.setMod(n_non_missing);
                    dummy_storage.resize(n_non_missing, n_non_missing);
                    the_cov_y_missing = &dummy_storage;
                } else {
                    PLASSERT( efficient_missing == 2 );
                    covs_y_missing.resize(L);
                    Mat& cov_y_missing_j = covs_y_missing[j];
                    cov_y_missing_j.resize(n_non_missing, n_non_missing);
                    the_cov_y_missing = &cov_y_missing_j;
                    mus_y_missing.resize(L);
                    Vec& mu_y_missing_j = mus_y_missing[j];
                    mu_y_missing_j.resize(n_non_missing);
                    the_mu_y_missing = &mu_y_missing_j;
                }

                for (int k = 0; k < n_non_missing; k++)
                    y_missing[k] = y[non_missing[k]];

                if (!eff_naive_missing ||
                    need_recompute[current_training_sample]) {
                for (int k = 0; k < n_non_missing; k++) {
                    (*the_mu_y_missing)[k] = mu_y[non_missing[k]];
                    for (int q = 0; q < n_non_missing; q++) {
                        (*the_cov_y_missing)(k,q) =
                            cov_y(non_missing[k], non_missing[q]);
                    }
                }
                }
                }
                /*
                if (n_non_missing == 0) {
                    log_likelihood = 0;
                } else {*/
                    // Perform SVD of cov_y_missing.
                    if (!eff_missing) {
                    if (!eff_naive_missing ||
                                    need_recompute[current_training_sample]) {
                    eigenvals_allj_missing.resize(L);
                    eigenvecs_allj_missing.resize(L);
                    // TODO We probably do not need this 'cov_backup', since
                    // the matrix 'the_cov_y_missing' should not be re-used.
                    // Once this is tested and verified, it could be removed
                    // for efficiency reasons.
                    static Mat cov_backup;
                    cov_backup.setMod(the_cov_y_missing->width());
                    cov_backup.resize(the_cov_y_missing->length(),
                                      the_cov_y_missing->width());
                    cov_backup << *the_cov_y_missing;
                    eigenVecOfSymmMat(cov_backup, n_non_missing,
                            eigenvals_allj_missing[j],
                            eigenvecs_allj_missing[j]);

                    PLASSERT( eigenvals_allj_missing[j].length()==n_non_missing);
                    PLASSERT( !cov_backup.hasMissing() );
                    }
                    eigenvals_missing = eigenvals_allj_missing[j];
                    eigenvecs_missing = &eigenvecs_allj_missing[j];
                    }

                    real log_det = 0;
                    static Mat L_tpl;
                    static TVec<int> ind_tpl;
                    static Mat L_tot;
                    static TVec<int> ind_tot;
                    int n_tpl = -1;
                    int queue_index = -1;
                    int path_index = -1;
                    bool same_covariance = false;
                    real log_det_tot, log_det_tpl;
                    if (eff_missing) {
                        path_index =
                            sample_to_path_index[current_training_sample];
                        // pout << "path index = " << path_index << endl;
                        L_tot.resize(n_non_missing, n_non_missing);
                        if (spanning_use_previous[current_cluster][path_index])
                            queue_index = cholesky_queue.length() - 1;
                        else
                            queue_index = cholesky_queue.length() - 2;
                        L_tpl = cholesky_queue[queue_index];
                        ind_tpl = indices_queue[queue_index];
                        if (efficient_missing == 3)
                            log_det_tpl = log_det_queue[queue_index];

                        n_tpl = L_tpl.length();
                        L_tot.resize(n_tpl, n_tpl);
                        /*
                        ind_tot.resize(n_non_missing);
                        ind_tot << non_missing;
                        */
                        ind_tot = non_missing;

                        // Optimization: detect when the same covariance matrix
                        // can be re-used.
                        // TODO What about just the dimensions being reordered?
                        // Are we losing time in such cases?
                        same_covariance =
                            ind_tpl.length() == ind_tot.length() &&
                            previous_training_sample >= 0;
                        if (same_covariance)
                            for (int i = 0; i < ind_tpl.length(); i++)
                                if (ind_tpl[i] != ind_tot[i]) {
                                    same_covariance = false;
                                    break;
                                }

                        /*
                        Mat tmp;
                        if (add_missing.length() > 0) {
                            tmp.resize(L_tot.length(), L_tot.width());
                            productTranspose(tmp, L_tot, L_tot);
                            VMat tmp_vm(tmp);
                            tmp_vm->saveAMAT("/u/delallea/tmp/before.amat", false,
                                    true);
                        }
                        */

                        // Remove some rows / columns.
                        /*
                        int p = add_missing.length() - 1;
                        for (int k = p; k >= 0; k--) {
                            choleskyRemoveDimension(L_tot, add_missing[k]); //(-k+p);
                            */
                            /*
                            tmp.resize(L_tot.length(), L_tot.width());
                            productTranspose(tmp, L_tot, L_tot);
                            VMat tmp_vm(tmp);
                            tmp_vm->saveAMAT("/u/delallea/tmp/before_" +
                                    tostring(add_missing[k]) + ".amat", false,
                                    true);
                                    */
                        /*
                        }
                        */
                    }
                    if ((efficient_missing == 1 || efficient_missing == 3) &&
                        current_training_sample >= 0)
                        no_missing_change[current_training_sample] =
                            same_covariance;

                    // Now we must perform updates to compute the Cholesky
                    // decomposition of interest.
                    static Vec new_vec;
                    int n = -1;
                    Mat* the_L = 0;
                    if (eff_missing) {
                    //L_tot.resize(n_non_missing, n_non_missing);
                        /*
                    for (int k = 0; k < add_non_missing.length(); k++) {
                        new_vec.resize(L_tot.length() + 1);
                        for (int q = 0; q < new_vec.length(); q++)
                            new_vec[q] = cov_y(tot_non_missing[q],
                                               add_non_missing[k]);
                        choleskyAppendDimension(L_tot, new_vec);
                    }
                        */
                    if (!same_covariance) {
                        if (efficient_missing == 1) {
                            //Profiler::start("updateCholeskyFromPrevious, em1");
                            updateCholeskyFromPrevious(L_tpl, L_tot,
                                    joint_cov[j], ind_tpl, ind_tot);
                            //Profiler::end("updateCholeskyFromPrevious, em1");
                        } else {
                            PLASSERT( efficient_missing == 3 );
                            //Profiler::start("updateInverseVarianceFromPrevious, em3");
                            updateInverseVarianceFromPrevious(L_tpl, L_tot,
                                    joint_cov[j], ind_tpl, ind_tot,
                                    &log_det_tpl, &log_det_tot);
                            //Profiler::end("updateInverseVarianceFromPrevious, em3");
#if 0
                            // Check that the inverse is correctly computed.
                            VMat L_tpl_vm(L_tpl);
                            VMat L_tot_vm(L_tot);
                            VMat joint_cov_vm(joint_cov[j]);
                            Mat data_tpl(1, ind_tpl.length());
                            for (int q = 0; q < ind_tpl.length(); q++)
                                data_tpl(0, q) = ind_tpl[q];
                            Mat data_tot(1, ind_tot.length());
                            for (int q = 0; q < ind_tot.length(); q++)
                                data_tot(0, q) = ind_tot[q];
                            VMat ind_tpl_vm(data_tpl);
                            VMat ind_tot_vm(data_tot);
                            L_tpl_vm->saveAMAT("/u/delallea/tmp/L_tpl_vm.amat",
                                    false, true);
                            L_tot_vm->saveAMAT("/u/delallea/tmp/L_tot_vm.amat",
                                    false, true);
                            joint_cov_vm->saveAMAT("/u/delallea/tmp/joint_cov_vm.amat",
                                    false, true);
                            ind_tpl_vm->saveAMAT("/u/delallea/tmp/ind_tpl_vm.amat",
                                    false, true);
                            ind_tot_vm->saveAMAT("/u/delallea/tmp/ind_tot_vm.amat",
                                    false, true);
#endif
                        }
                    }
                    // Note to myself: indices in ind_tot will be changed.

                    // Debug check.
                    /*
                    static Mat tmp_mat;
                    tmp_mat.resize(L_tot.length(), L_tot.length());
                    productTranspose(tmp_mat, L_tot, L_tot);
                    // pout << "max = " << max(tmp_mat) << endl;
                    // pout << "min = " << min(tmp_mat) << endl;
                    */
                    the_L = same_covariance ? &L_tpl : &L_tot;
                    real* the_log_det = same_covariance ? &log_det_tpl
                                                        : &log_det_tot;
                    n = the_L->length();
                    if (efficient_missing == 1) {
                        for (int i = 0; i < n; i++)
                            log_det += pl_log((*the_L)(i, i));
                    } else {
                        PLASSERT( efficient_missing == 3 );
#if 0
                        VMat the_L_vm(*the_L);
                        the_L_vm->saveAMAT("/u/delallea/tmp/L.amat", false,
                                true);
#endif
                        if (is_missing(*the_log_det)) {
                            // That can happen due to numerical imprecisions.
                            // In such a case we have to recompute the
                            // determinant and the inverse.
                            PLASSERT( !same_covariance );
                            the_cov_y_missing->setMod(n_non_missing);
                            the_cov_y_missing->resize(n_non_missing, n_non_missing);
                            for (int k = 0; k < n_non_missing; k++)
                                for (int q = 0; q < n_non_missing; q++)
                                    (*the_cov_y_missing)(k,q) =
                                        cov_y(non_missing[k], non_missing[q]);
                            *the_log_det = det((*the_cov_y_missing), true);
                            matInvert(*the_cov_y_missing, *the_L);
                            fillItSymmetric(*the_L);
                        }

                        // Note: we need to multiply the log-determinant by 0.5
                        // compared to 'efficient_missing == 1' because the
                        // determinant computed from Cholesky is the one for L,
                        // which is the squared root of the one of the full
                        // matrix.
                        log_det += 0.5 * *the_log_det;
                    }
                    PLASSERT( !(isnan(log_det) || isinf(log_det)) );
                    log_likelihood = -0.5 * (n * Log2Pi) - log_det;
                    }

                    y_centered.resize(n_non_missing);
                    if (!eff_missing) {
                    mu_y = *the_mu_y_missing;
                    eigenvals = eigenvals_missing;
                    eigenvecs = *eigenvecs_missing;

                    y_centered << y_missing;
                    y_centered -= mu_y;
                    }

                    real* center_j = center[j];
                    if (eff_missing) {
                        for (int k = 0; k < n_non_missing; k++) {
                            int ind_tot_k = ind_tot[k];
                            y_centered[k] =
                                y[ind_tot_k] - center_j[ind_tot_k];
                        }

                    static Vec tmp_vec1;
                    if (impute_missing && current_training_sample >= 0) {
                        // We need to store the conditional expectation of the
                        // sample missing values.
                        static Vec tmp_vec2;
                        tmp_vec1.resize(the_L->length());
                        tmp_vec2.resize(the_L->length());
                        if (efficient_missing == 1)
                            choleskySolve(*the_L, y_centered, tmp_vec1, tmp_vec2);
                        else {
                            PLASSERT( efficient_missing == 3 );
                            product(tmp_vec1, *the_L, y_centered);
                        }
                        static Mat K2;
                        int ind_tot_length = ind_tot.length();
                        K2.resize(cov_y.length() - ind_tot_length,
                                  ind_tot.length());
                        for (int i = 0; i < K2.length(); i++)
                            for (int k = 0; k < K2.width(); k++)
                                K2(i,k) = cov_y(coord_missing[i],
                                                non_missing[k]);
                        static Vec cond_mean;
                        cond_mean.resize(coord_missing.length());
                        product(cond_mean, K2, tmp_vec1);
                        static Vec full_vec;
                        // TODO Right now, we store the full data vector. It
                        // may be more efficient to only store the missing
                        // values.
                        full_vec.resize(D);
                        full_vec << y;
                        for (int i = 0; i < coord_missing.length(); i++)
                            full_vec[coord_missing[i]] =
                                cond_mean[i] + center_j[coord_missing[i]];
                        clust_imputed_missing[j](path_index) << full_vec;
                    }

                    if (n > 0) {
                        if (efficient_missing == 1) {
                            tmp_vec1.resize(y_centered.length());
                            choleskyLeftSolve(*the_L, y_centered, tmp_vec1);
                            log_likelihood -= 0.5 * pownorm(tmp_vec1);
                        } else {
                            PLASSERT( efficient_missing == 3 );
                            log_likelihood -= 0.5 * dot(y_centered, tmp_vec1);
                        }
                    }
                    // Now remember L_tot for the generations to come.
                    // TODO This could probably be optimized to avoid useless
                    // copies of the covariance matrix.
                    bool cannot_free =
                        !spanning_can_free[current_cluster][path_index];
                    if (cannot_free)
                        queue_index++;
                    cholesky_queue.resize(queue_index + 1);
                    indices_queue.resize(queue_index + 1);
                    if (efficient_missing == 3)
                        log_det_queue.resize(queue_index + 1);
                    // pout << "length = " << cholesky_queue.length() << endl;

                    // Free a reference to element in cholesky_queue. This
                    // is needed because this matrix is going to be resized.
                    L_tpl = dummy_mat;

                    if (!same_covariance || cannot_free) {
                    Mat& chol = cholesky_queue[queue_index];
                    chol.resize(L_tot.length(), L_tot.width());
                    chol << L_tot;
                    TVec<int>& ind = indices_queue[queue_index];
                    ind.resize(ind_tot.length());
                    ind << ind_tot;
                    if (efficient_missing == 3)
                        log_det_queue[queue_index] = log_det_tot;
                    }

                    // pout << "queue_index = " << queue_index << endl;

                    }

                    if (!eff_missing) {
                    // real squared_norm_y_centered = pownorm(y_centered);
                    int n_eig = n_non_missing;

                    real lambda0 = var_min;
                    if (!eigenvals.isEmpty() && eigenvals.lastElement() > lambda0)
                        lambda0 = eigenvals.lastElement();
                    PLASSERT( lambda0 > 0 );
                    real one_over_lambda0 = 1.0 / lambda0;

                    log_likelihood = precomputeGaussianLogCoefficient(
                            eigenvals, n_non_missing);

                    static Vec y_centered_copy;
                    y_centered_copy.resize(y_centered.length());
                    y_centered_copy << y_centered; // Backup vector.
                    for (int k = 0; k < n_eig - 1; k++) {
                        real lambda = max(var_min, eigenvals[k]);
                        PLASSERT( lambda > 0 );
                        Vec eigen_k = eigenvecs(k);
                        real dot_k = dot(eigen_k, y_centered);
                        log_likelihood -= 0.5 * square(dot_k) / lambda;
                        multiplyAcc(y_centered, eigen_k, -dot_k);
                    }
                    log_likelihood -=
                        0.5 * pownorm(y_centered) * one_over_lambda0;
                    y_centered << y_centered_copy; // Restore original vector.

#if 0
                    // Old code, that had stability issues when dealing with
                    // large numbers.

                    // log_likelihood -= 0.5  * 1/lambda_0 * ||y - mu||^2
                    log_likelihood -=
                        0.5 * one_over_lambda0 * squared_norm_y_centered;

                    for (int k = 0; k < n_eig - 1; k++) {
                        // log_likelihood -= 0.5 * (1/lambda_k - 1/lambda_0)
                        //                       * ((y - mu)'.v_k)^2
                        real lambda = max(var_min, eigenvals[k]);
                        PLASSERT( lambda > 0 );
                        if (lambda > lambda0)
                            log_likelihood -=
                                0.5 * (1.0 / lambda - one_over_lambda0)
                                    * square(dot(eigenvecs(k), y_centered));
                    }
#endif

                    // Release pointer to 'eigenvecs_missing'.
                    eigenvecs = dummy_mat;
                    eigenvecs_missing = &eigenvecs_missing_storage;

                    if (impute_missing && current_training_sample >= 0) {
                        // We need to store the conditional expectation of the
                        // sample missing values.
                        // For this we compute H3^-1, since this expectation is
                        // equal to mu_y - H3^-1 H2 (x - mu_x).
                        static Mat H3;
                        static Mat H2;
                        Mat& H3_inv = H3_inverse[j];
                        int n_missing = coord_missing.length();
                        if (!eff_naive_missing ||
                                need_recompute[current_training_sample]) {
                        H3.setMod(n_missing);
                        H3.resize(n_missing, n_missing);
                        H3_inv.resize(n_missing, n_missing);
                        for (int i = 0; i < n_missing; i++)
                            for (int k = 0; k < n_missing; k++)
                                H3(i,k) = (*inv_cov_y)(coord_missing[i],
                                        coord_missing[k]);
                        PLASSERT( H3.isSymmetric(true, true) );
                        matInvert(H3, H3_inv);
                        // PLASSERT( H3_inv.isSymmetric(false, true) );
                        fillItSymmetric(H3_inv);
                        }

                        H2.resize(n_missing, n_non_missing);
                        for (int i = 0; i < n_missing; i++)
                            for (int k = 0; k < n_non_missing; k++)
                                H2(i,k) = (*inv_cov_y)(coord_missing[i],
                                                       non_missing[k]);
                        static Vec H2_y_centered;
                        H2_y_centered.resize(n_missing);
                        product(H2_y_centered, H2, y_centered);
                        static Vec cond_mean;
                        cond_mean.resize(n_missing);
                        product(cond_mean, H3_inv, H2_y_centered);
                        static Vec full_vec;
                        // TODO Right now, we store the full data vector. It
                        // may be more efficient to only store the missing
                        // values.
                        full_vec.resize(D);
                        full_vec << y;
                        for (int i = 0; i < n_missing; i++)
                            full_vec[coord_missing[i]] =
                                center_j[coord_missing[i]] - cond_mean[i];
                        PLASSERT( !full_vec.hasMissing() );
                        imputed_missing[j]->putRow(current_training_sample,
                                                   full_vec);
                    }

                    }
            //}
            } else {
                log_likelihood = log_coeff[j];

                mu_y = center(j).subVec(0, n_predicted);
                eigenvals = eigenvalues(j);
                eigenvecs = eigenvectors[j];

                y_centered.resize(n_predicted);
                y_centered << y;
                y_centered -= mu_y;
                real squared_norm_y_centered = pownorm(y_centered);
                real var_min = square(sigma_min);
                int n_eig = n_eigen_computed;
                real lambda0 = max(var_min, eigenvals[n_eig - 1]);
                PLASSERT( lambda0 > 0 );

                real one_over_lambda0 = 1.0 / lambda0;
                // log_likelihood -= 0.5  * 1/lambda_0 * ||y - mu||^2
                log_likelihood -= 0.5 * one_over_lambda0 * squared_norm_y_centered;

                for (int k = 0; k < n_eig - 1; k++) {
                    // log_likelihood -= 0.5 * (1/lambda_k - 1/lambda_0)
                    //                       * ((y - mu)'.v_k)^2
                    real lambda = max(var_min, eigenvals[k]);
                    PLASSERT( lambda > 0 );
                    if (lambda > lambda0)
                        log_likelihood -= 0.5 * (1.0 / lambda - one_over_lambda0)
                            * square(dot(eigenvecs(k), y_centered));
                }
            }
        } else {
            if (y.hasMissing()) {
                // TODO Code duplication is ugly!
                if (is_predictor) {
                    non_missing.resize(0);
                    for (int k = 0; k < y.length(); k++)
                        if (!is_missing(y[k]))
                            non_missing.append(k);
                    int n_non_missing = non_missing.length();
                    int n_predicted_ext = n_predicted + (n_predictor - n_non_missing);

                    work_mat1.resize(n_predicted_ext, n_non_missing);
                    work_mat2.resize(n_predicted_ext, n_predicted_ext);
                    real var_min = square(sigma_min);
                    eigenvalues_x_miss.resize(L, n_non_missing);
                    eigenvectors_x_miss.resize(L);
                    // Compute the mean and covariance of x and y|x for the j-th
                    // Gaussian (we will need them to compute the likelihood).
                    // TODO Do we really compute the mean of y|x here?
                    // TODO This is pretty ugly but it seems to work: replace by
                    // better-looking code.

                    // First we compute the joint covariance matrix from the
                    // eigenvectors and eigenvalues:
                    // full_cov = sum_k (lambda_k - lambda0) v_k v_k' + lambda0.I

                    PLASSERT( n_predictor + n_predicted == D );

                    Mat& full_cov_j = full_cov;
                    full_cov_j.resize(D, D);
                    eigenvals = eigenvalues(j);
                    real lambda0 = max(var_min, eigenvals[n_eigen_computed - 1]);

                    full_cov_j.fill(0);
                    Mat& eigenvectors_j = eigenvectors[j];
                    PLASSERT( eigenvectors_j.width() == D );

                    for (int k = 0; k < n_eigen_computed - 1; k++)
                        externalProductScaleAcc(
                                full_cov_j, eigenvectors_j(k),
                                eigenvectors_j(k),
                                max(var_min, eigenvals[k]) - lambda0);

                    for (int i = 0; i < D; i++)
                        full_cov_j(i,i) += lambda0;

                    // By construction, the resulting matrix is symmetric. However,
                    // it may happen that it is not exactly the case due to numerical
                    // approximations. Thus we ensure it is perfectly symmetric.
                    PLASSERT( full_cov_j.isSymmetric(false) );
                    fillItSymmetric(full_cov_j);

                    // Extract the covariance of the predictor x.
                    Mat cov_x_j_miss = full_cov.subMat(0, 0, n_predictor, n_predictor);
                    cov_x_j.resize(n_non_missing, n_non_missing);
                    for (int k = 0; k < n_non_missing; k++)
                        for (int p = k; p < n_non_missing; p++)
                            cov_x_j(k,p) = cov_x_j(p,k) =
                                cov_x_j_miss(non_missing[k], non_missing[p]);

                    // Compute its SVD.
                    eigenvectors_x_miss[j].resize(n_non_missing, n_non_missing);
                    eigenvals = eigenvalues_x_miss(j);
                    eigenVecOfSymmMat(cov_x_j, n_non_missing, eigenvals,
                                      eigenvectors_x_miss[j]);

                    y_non_missing.resize(n_non_missing);
                    center_non_missing.resize(n_non_missing);
                    for (int k = 0; k < n_non_missing; k++) {
                        center_non_missing[k] = center(j, non_missing[k]);
                        y_non_missing[k] = y[non_missing[k]];
                    }


                    log_likelihood =
                        precomputeGaussianLogCoefficient(eigenvals, n_non_missing);
                    eigenvecs = eigenvectors_x_miss[j];
                    y_centered.resize(n_non_missing);
                    y_centered << y_non_missing;
                    mu = center_non_missing;

                } else {
                    // We need to re-do everything again, now this sucks!
                    // First the full covariance (of y|x).
                    Mat& cov_y = cov_y_x;
                    real var_min = square(sigma_min);
                    cov_y.resize(n_predicted, n_predicted);
                    eigenvals = eigenvalues_y_x(j);
                    real lambda0 = max(var_min, eigenvals.lastElement());
                    cov_y.fill(0);
                    Mat& eigenvectors_j = eigenvectors_y_x[j];
                    int n_eig = eigenvectors_j.length();

                    PLASSERT( eigenvectors_j.width() == n_predicted );

                    for (int k = 0; k < n_eig - 1; k++)
                        externalProductScaleAcc(
                                cov_y, eigenvectors_j(k), eigenvectors_j(k),
                                max(var_min, eigenvals[k]) - lambda0);

                    for (int i = 0; i < n_predicted; i++)
                        cov_y(i,i) += lambda0;

                    // By construction, the resulting matrix is symmetric. However,
                    // it may happen that it is not exactly the case due to numerical
                    // approximations. Thus we ensure it is perfectly symmetric.
                    PLASSERT( cov_y.isSymmetric(false) );
                    fillItSymmetric(cov_y);
                    // Then extract what we want.
                    non_missing.resize(0);
                    for (int k = 0; k < n_predicted; k++)
                        if (!is_missing(y[k]))
                            non_missing.append(k);
                    mu_y = center_y_x(j);
                    int n_non_missing = non_missing.length();
                    the_mu_y_missing->resize(n_non_missing);
                    y_missing.resize(n_non_missing);
                    the_cov_y_missing->resize(n_non_missing, n_non_missing);
                    for (int k = 0; k < n_non_missing; k++) {
                        (*the_mu_y_missing)[k] = mu_y[non_missing[k]];
                        y_missing[k] = y[non_missing[k]];
                        for (int q = 0; q < n_non_missing; q++) {
                            (*the_cov_y_missing)(k,q) =
                                cov_y(non_missing[k], non_missing[q]);
                        }
                    }
                    if (n_non_missing == 0) {
                        log_likelihood = 0;
                    } else {
                        // Perform SVD of cov_y_missing.
                        eigenVecOfSymmMat(*the_cov_y_missing, n_non_missing,
                                eigenvals_missing, *eigenvecs_missing);

                        mu_y = *the_mu_y_missing;
                        eigenvals = eigenvals_missing;
                        eigenvecs = *eigenvecs_missing;

                        y_centered.resize(n_non_missing);
                        y_centered << y_missing;
                        y_centered -= mu_y;
                        real squared_norm_y_centered = pownorm(y_centered);
                        int n_eigen = n_non_missing;

                        lambda0 = max(var_min, eigenvals.lastElement());
                        PLASSERT( lambda0 > 0 );
                        real one_over_lambda0 = 1.0 / lambda0;

                        log_likelihood = precomputeGaussianLogCoefficient(
                                eigenvals, n_non_missing);
                        // log_likelihood -= 0.5  * 1/lambda_0 * ||y - mu||^2
                        log_likelihood -=
                            0.5 * one_over_lambda0 * squared_norm_y_centered;

                        for (int k = 0; k < n_eigen - 1; k++) {
                            // log_likelihood -= 0.5 * (1/lambda_k - 1/lambda_0)
                            //                       * ((y - mu)'.v_k)^2
                            real lambda = max(var_min, eigenvals[k]);
                            PLASSERT( lambda > 0 );
                            if (lambda > lambda0)
                                log_likelihood -=
                                    0.5 * (1.0 / lambda - one_over_lambda0)
                                    * square(dot(eigenvecs(k), y_centered));
                        }
                        // Allow future resize of 'eigenvecs_missing'.
                        eigenvecs = dummy_mat;
                    }

                    //Profiler::end("computeLogLikelihood");
                    return log_likelihood;
                }

                if (y_centered.length() > 0) {
                    y_centered -= mu;

                    real squared_norm_y_centered = pownorm(y_centered);
                    real var_min = square(sigma_min);
                    int n_eig = eigenvals.length();

                    real lambda0 = max(var_min, eigenvals.lastElement());
                    PLASSERT( lambda0 > 0 );

                    real one_over_lambda0 = 1.0 / lambda0;
                    // log_likelihood -= 0.5  * 1/lambda_0 * ||y - mu||^2
                    log_likelihood -= 0.5 * one_over_lambda0 * squared_norm_y_centered;

                    for (int k = 0; k < n_eig - 1; k++) {
                        // log_likelihood -= 0.5 * (1/lambda_k - 1/lambda_0)
                        //                       * ((y - mu)'.v_k)^2
                        real lambda = max(var_min, eigenvals[k]);
                        PLASSERT( lambda > 0 );
                        PLASSERT( lambda >= lambda0 );
                        if (lambda > lambda0)
                            log_likelihood -= 0.5 * (1.0 / lambda - one_over_lambda0)
                                * square(dot(eigenvecs(k), y_centered));
                    }
                }
            } else {

                if (is_predictor) {
                    log_likelihood = log_coeff_x[j];
                    mu = center(j).subVec(0, n_predictor);
                    eigenvals = eigenvalues_x(j);
                    eigenvecs = eigenvectors_x[j];
                    y_centered.resize(n_predictor);
                } else {
                    log_likelihood = log_coeff_y_x[j];
                    mu = center_y_x(j);
                    eigenvals = eigenvalues_y_x(j);
                    eigenvecs = eigenvectors_y_x[j];
                    y_centered.resize(n_predicted);
                }

                y_centered << y;
                y_centered -= mu;

                real squared_norm_y_centered = pownorm(y_centered);
                real var_min = square(sigma_min);
                int n_eig = eigenvals.length();

                real lambda0 = max(var_min, eigenvals[n_eig - 1]);
                PLASSERT( lambda0 > 0 );

                real one_over_lambda0 = 1.0 / lambda0;
                // log_likelihood -= 0.5  * 1/lambda_0 * ||y - mu||^2
                log_likelihood -= 0.5 * one_over_lambda0 * squared_norm_y_centered;

                for (int k = 0; k < n_eig - 1; k++) {
                    // log_likelihood -= 0.5 * (1/lambda_k - 1/lambda_0)
                    //                       * ((y - mu)'.v_k)^2
                    real lambda = max(var_min, eigenvals[k]);
                    PLASSERT( lambda > 0 );
                    PLASSERT( lambda >= lambda0 );
                    if (lambda > lambda0)
                        log_likelihood -= 0.5 * (1.0 / lambda - one_over_lambda0)
                            * square(dot(eigenvecs(k), y_centered));
                }
            }

            // Free a potential reference to 'eigenvalues_x_miss' and
            // 'eigenvectors_x_miss'.
            eigenvals = dummy_vec;
            eigenvecs = dummy_mat;
        }
    }
    PLASSERT( !isnan(log_likelihood) );
    //Profiler::end("computeLogLikelihood");
    return log_likelihood;
}

//////////////////////////////
// computeAllLogLikelihoods //
//////////////////////////////
void GaussMix::computeAllLogLikelihoods(const Vec& sample, const Vec& log_like)
{
    PLASSERT( sample.length()   == D );
    PLASSERT( log_like.length() == L );
    for (int j = 0; j < L; j++)
        log_like[j] = computeLogLikelihood(sample, j);
}

///////////////////////
// computePosteriors //
///////////////////////
void GaussMix::computePosteriors() {
    //Profiler::start("computePosteriors");
    sample_row.resize(D);
    if (impute_missing) {
        sum_of_posteriors.resize(L); // TODO Do that in resize method.
        sum_of_posteriors.fill(0);
    }
    log_likelihood_post.resize(L);
    if (impute_missing)
        // Clear the additional 'error_covariance' matrix.
        for (int j = 0; j < L; j++)
            error_covariance[j].fill(0);
    if (efficient_missing == 1 || efficient_missing == 3) {
        // Loop over all clusters.
        for (int k = 0; k < missing_template.length(); k++) {
            const TVec<int>& samples_clust = spanning_path[k];
            int n_samp = samples_clust.length();
            log_likelihood_post_clust.resize(n_samp, L);
            current_cluster = k;
            if (impute_missing)
                for (int j = 0; j < L; j++)
                    clust_imputed_missing[j].resize(n_samp, D);
            for (int j = 0; j < L; j++) {
                // For each Gaussian, go through all samples in the cluster.
                previous_training_sample = -1;
                for (int i = 0; i < samples_clust.length(); i++) {
                    int s = samples_clust[i];
                    current_training_sample = s;
                    train_set->getSubRow(s, 0, sample_row);
                    log_likelihood_post_clust(i, j) =
                        computeLogLikelihood(sample_row, j) + pl_log(alpha[j]);
                    previous_training_sample = current_training_sample;
                    current_training_sample = -1;
                }
            }
            previous_training_sample = -2;
            // Get the posteriors for all samples in the cluster.
            for (int i = 0; i < samples_clust.length(); i++) {
                real log_sum_likelihood = logadd(log_likelihood_post_clust(i));
                int s = samples_clust[i];
                for (int j = 0; j < L; j++) {
                    real post = exp(log_likelihood_post_clust(i, j) -
                                    log_sum_likelihood);
                    posteriors(s, j) = post;
                    if (impute_missing)
                        sum_of_posteriors[j] += post;
                }
            }
            if (!impute_missing)
                continue;
            // We should now be ready to impute missing values.
            for (int i = 0; i < samples_clust.length(); i++) {
                int s = samples_clust[i];
                for (int j = 0; j < L; j++) {
                    PLASSERT( !clust_imputed_missing[j](i).hasMissing() );
                    // TODO We are most likely wasting memory here.
                    imputed_missing[j]->putRow(s, clust_imputed_missing[j](i));
                }
            }

            // If the 'impute_missing' method is used, we now need to compute
            // the extra contribution to the covariance matrix.
            for (int j = 0; j < L; j++) {
                // For each Gaussian, go through all samples in the cluster.
                previous_training_sample = -1;
                for (int i = 0; i < samples_clust.length(); i++) {
                    int s = samples_clust[i];
                    current_training_sample = s;
                    train_set->getSubRow(s, 0, sample_row);
                    addToCovariance(sample_row, j, error_covariance[j],
                            posteriors(s, j));
                    previous_training_sample = current_training_sample;
                    current_training_sample = -1;
                }
            }
            previous_training_sample = -2;
        }
    } else {
        previous_training_sample = -1;
    for (int i = 0; i < nsamples; i++) {
        train_set->getSubRow(i, 0, sample_row);
        // First we need to compute the likelihood P(s_i | j).
        current_training_sample = i;
        computeAllLogLikelihoods(sample_row, log_likelihood_post);
        PLASSERT( !log_likelihood_post.hasMissing() );
        for (int j = 0; j < L; j++)
            log_likelihood_post[j] += pl_log(alpha[j]);
        real log_sum_likelihood = logadd(log_likelihood_post);
        for (int j = 0; j < L; j++) {
            // Compute the posterior
            // P(j | s_i) = P(s_i | j) * alpha_i / (sum_i ")
            real post = exp(log_likelihood_post[j] - log_sum_likelihood);
            posteriors(i,j) = post;
            if (impute_missing)
                sum_of_posteriors[j] += post;
        }
        // Add contribution to the covariance matrix if needed.
        if (impute_missing) {
            for (int j = 0; j < L; j++) {
                real post = posteriors(i,j);
                int k_count = 0;
                for (int k = 0; k < sample_row.length(); k++)
                    if (is_missing(sample_row[k])) {
                        int l_count = 0;
                        for (int l = 0; l < sample_row.length(); l++)
                            if (is_missing(sample_row[l])) {
                                error_covariance[j](k, l) +=
                                    post * H3_inverse[j](k_count, l_count);
                                l_count++;
                            }
                        k_count++;
                    }
            }
            int dummy_test = 0;
            dummy_test++;
        }
        previous_training_sample = current_training_sample;
        current_training_sample = -1;
    }
    previous_training_sample = -2;
    }
    //Profiler::end("computePosteriors");
}

///////////////////////////
// computeMixtureWeights //
///////////////////////////
bool GaussMix::computeMixtureWeights(bool allow_replace) {
    bool replaced_gaussian = false;
    if (L==1)
        alpha[0] = 1;
    else {
        alpha.fill(0);
        for (int i = 0; i < nsamples; i++)
            for (int j = 0; j < L; j++)
                alpha[j] += posteriors(i,j);
        alpha /= real(nsamples);
        for (int j = 0; j < L && !replaced_gaussian; j++)
            if (alpha[j] < alpha_min && allow_replace
                                     && stage_replaced[j] != this->stage) {
                // alpha[j] is too small! We need to remove this Gaussian from
                // the mixture, and find a new (better) one.
                replaceGaussian(j);
                replaced_gaussian = true;
                stage_replaced[j] = this->stage;
            }
    }
    return replaced_gaussian;
}

/////////////////
// expectation //
/////////////////
void GaussMix::expectation(Vec& mu) const
{
    mu.resize(n_predicted);
    if (type_id == TYPE_SPHERICAL || type_id == TYPE_DIAGONAL ||
       (type_id == TYPE_GENERAL && n_predictor == 0)) {
        // The expectation is the same in the 'spherical' and 'diagonal' cases.
        mu.fill(0);
        real* coeff = n_predictor == 0 ? alpha.data() : p_j_x.data();
        for (int j = 0; j < L; j++)
            mu += center(j).subVec(n_predictor, n_predicted) * coeff[j];
    } else {
        PLASSERT( type_id == TYPE_GENERAL );
        // The case 'n_predictor == 0' is considered above.
        PLASSERT( n_predictor > 0 );
        mu.fill(0);
        for (int j = 0; j < L; j++)
            mu += center_y_x(j) * p_j_x[j];
    }
}

////////////////////////
// missingExpectation //
////////////////////////
void GaussMix::missingExpectation(const Vec& input, Vec& mu)
{
    static TVec<int> coord_missing;
    static TVec<int> coord_non_missing;
    static TVec<int> coord_reordered;
    static Vec input_non_missing;
    static Mat center_backup;
    static Mat mat_storage;
    static TVec<Mat> eigenvectors_backup;
    if (!input.hasMissing()) {
        mu.resize(0);
        return;
    }
    if (type_id != TYPE_GENERAL)
        PLERROR("In GaussMix::missingExpectation - Not implemented for this "
                "type");

    // Create coordinate indices lists.
    coord_missing.resize(0);
    coord_non_missing.resize(0);
    input_non_missing.resize(0);
    for (int i = 0; i < input.length(); i++)
        if (is_missing(input[i]))
            coord_missing.append(i);
        else {
            coord_non_missing.append(i);
            input_non_missing.append(input[i]);
        }
    int n_missing = coord_missing.length();
    int n_non_missing = coord_non_missing.length();
    coord_reordered.resize(input.length());
    coord_reordered.subVec(0, n_non_missing) << coord_non_missing;
    coord_reordered.subVec(n_non_missing, n_missing) << coord_missing;

    // Backup existing data.
    center_backup.resize(center.length(), center.width());
    center_backup << center;
    eigenvectors_backup.resize(eigenvectors.length());
    for (int i = 0; i < eigenvectors.length(); i++) {
        Mat& eigenvecs_backup = eigenvectors_backup[i];
        Mat& eigenvecs = eigenvectors[i];
        eigenvecs_backup.resize(eigenvecs.length(), eigenvecs.width());
        eigenvecs_backup << eigenvecs;
    }
    int predictor_size_backup = predictor_size;
    int predicted_size_backup = predicted_size;

    // Update components to match the new reordered coordinates.
    selectColumns(center_backup, coord_reordered, center);
    for (int i = 0; i < eigenvectors.length(); i++)
        selectColumns(eigenvectors_backup[i], coord_reordered,
                                              eigenvectors[i]);

    // Set this distribution as conditional to compute the expectation of the
    // missing (predicted) part given the observed (predictor) part.
    setPredictorPredictedSizes(n_non_missing, n_missing);
    setPredictor(input_non_missing);

    // Compute the expectation.
    expectation(mu);

    // Restore everything.
    setPredictorPredictedSizes(predictor_size_backup, predicted_size_backup);
    center << center_backup;
    for (int i = 0; i < eigenvectors.length(); i++) {
        Mat& eigenvecs_backup = eigenvectors_backup[i];
        Mat& eigenvecs = eigenvectors[i];
        eigenvecs << eigenvecs_backup;
    }
}

////////////
// forget //
////////////
void GaussMix::forget()
{
    inherited::forget();
    log_p_j_x.resize(0);
    p_j_x.resize(0);
    D = -1;
    n_eigen_computed = -1;
    ptimer->resetAllTimers();
    stage_replaced.fill(-1);
    /*
       if (training_time >= 0)
       training_time = 0;
       if (conditional_updating_time >= 0)
       conditional_updating_time = 0;
       n_tries.resize(0);
    */
}

//////////////
// generate //
//////////////
void GaussMix::generate(Vec& x) const
{
    generateFromGaussian(x, -1);
}

//////////////////////////
// generateFromGaussian //
//////////////////////////
void GaussMix::generateFromGaussian(Vec& sample, int given_gaussian) const {
    // TODO Why not having p_j_x point to alpha when n_predictor == 0 ? This may
    // make the code cleaner (but check what happens with serialization...).
    int j;    // The index of the Gaussian to use.

    // The assert below may fail if one forgets to provide a predictor part
    // through the 'setPredictor' method.
    PLASSERT( n_predictor == 0 || p_j_x.length() == L );

    if (given_gaussian < 0)
        j = random_gen->multinomial_sample(n_predictor == 0 ? alpha : p_j_x);
    else
        j = given_gaussian % alpha.length();

    sample.resize(n_predicted);

    if (type_id == TYPE_SPHERICAL || type_id == TYPE_DIAGONAL) {
        Vec mu_y = center(j).subVec(n_predictor, n_predicted);
        for (int k = 0; k < n_predicted; k++) {
            real stddev = type_id == TYPE_SPHERICAL ? sigma[j]
                                                    : diags(j, k + n_predictor);
            stddev = max(sigma_min, stddev);
            sample[k] = random_gen->gaussian_mu_sigma(mu_y[k], stddev);
        }
    } else {
        PLASSERT( type_id == TYPE_GENERAL );
        static Vec norm_vec;
        if (n_predictor == 0) {
            // Simple case.
            PLASSERT( eigenvectors[j].width() == n_predicted );
            PLASSERT( center(j).length() == n_predicted );

            Vec eigenvals = eigenvalues(j);
            Mat eigenvecs = eigenvectors[j].subMat(0, 0, n_eigen_computed,
                                                         n_predicted);
            int n_eig = n_eigen_computed;
            Vec mu_y = center(j);

            norm_vec.resize(n_eig - 1);
            random_gen->fill_random_normal(norm_vec);
            real var_min = square(sigma_min);
            real lambda0 = max(var_min, eigenvals[n_eig - 1]);
            sample.fill(0);
            for (int k = 0; k < n_eig - 1; k++)
                // TODO See if can use more optimized function.
                sample += sqrt(max(var_min, eigenvals[k]) - lambda0)
                          * norm_vec[k] * eigenvecs(k);
            norm_vec.resize(n_predicted);
            random_gen->fill_random_normal(norm_vec);
            sample += norm_vec * sqrt(lambda0);
            sample += mu_y;
        } else {
            // TODO Get rid of code duplication with above.

            Vec eigenvals = eigenvalues_y_x(j);
            Mat eigenvecs = eigenvectors_y_x[j];

            int n_eig = n_predicted;
            Vec mu_y = center_y_x(j);

            norm_vec.resize(n_eig - 1);
            random_gen->fill_random_normal(norm_vec);
            real var_min = square(sigma_min);
            real lambda0 = max(var_min, eigenvals[n_eig - 1]);
            sample.fill(0);
            for (int k = 0; k < n_eig - 1; k++)
                // TODO See if can use more optimized function.
                sample += sqrt(max(var_min, eigenvals[k]) - lambda0)
                          * norm_vec[k] * eigenvecs(k);
            norm_vec.resize(n_predicted);
            random_gen->fill_random_normal(norm_vec);
            sample += norm_vec * sqrt(lambda0);
            sample += mu_y;
        }
    }
    PLASSERT( !sample.hasMissing() );
}

/*
///////////////////////
// getNEigenComputed //
///////////////////////
int GaussMix::getNEigenComputed() const {
    return n_eigen_computed;
}

/////////////////////
// getEigenvectors //
/////////////////////
Mat GaussMix::getEigenvectors(int j) const {
    //return eigenvectors[j];
}

//////////////////
// getEigenvals //
//////////////////
Vec GaussMix::getEigenvals(int j) const {
    //return eigenvalues(j);
}
*/

////////////
// kmeans //
////////////
void GaussMix::kmeans(const VMat& samples, int nclust, TVec<int>& clust_idx,
                      Mat& clust, int maxit)
// TODO Put it into the PLearner framework.
{
    int vmat_length = samples.length();
    clust.resize(nclust,samples->inputsize());
    clust_idx.resize(vmat_length);

    Vec input(samples->inputsize());
    Vec target(samples->targetsize());
    real weight;

    TVec<int> old_clust_idx(vmat_length);
    bool ok=false;

    // Compute mean and standard deviation for all fields (will be used to
    // generate some random values to replace missing values).
    computeMeanAndStddev(samples, mean_training, stddev_training);

    if (mean_training.hasMissing())
        // Some features are completely missing: we assume mean is 0 and
        // standard deviation is 1.
        for (int i = 0; i < mean_training.length(); i++)
            if (is_missing(mean_training[i])) {
                mean_training[i] = 0;
                stddev_training[i] = 1;
            }

    if (stddev_training.hasMissing())
        // There may be only one sample with a non-missing value, we assume the
        // standard deviation is 1 (probably not always a good idea, but it
        // should not really matter in any real-life application).
        for (int i = 0; i < stddev_training.length(); i++)
            if (is_missing(stddev_training[i]))
                stddev_training[i] = 1;

    // Build a nclust-long vector of samples indexes to initialize cluster
    // centers. In order to avoid some local minima, try to span as much of the
    // space as possible by systematically choosing as initial cluster center
    // the point 'farthest' from current centers.
    TVec<int> start_idx(nclust, -1);

    // Store the distance from each point to the 'nclust' cluster centers.
    Mat distances(vmat_length, nclust);
    Vec min_distances(vmat_length);
    int farthest_sample = random_gen->uniform_multinomial_sample(vmat_length);
    if (!original_to_reordered.isEmpty())
        farthest_sample = original_to_reordered[farthest_sample];
    Vec input_k;
    for (int i=0; i<nclust; i++)
    {
        start_idx[i] = farthest_sample;
        samples->getExample(farthest_sample,input,target,weight);
        clust(i) << input;
        // Ensure there are no missing values in the initial centers.
        // To do so we generate random values based on 'mean' and 'stddev' if
        // the center we picked turns out to have missing values.
        Vec cl_center = clust(i);
        for (int k = 0; k < cl_center.length(); k++)
            if (is_missing(cl_center[k]))
                cl_center[k] =
                    random_gen->gaussian_mu_sigma(mean_training[k],
                                                  stddev_training[k]);
        if (i < nclust - 1) {
            // Find next cluster center.
            for (int k = 0; k < vmat_length; k++) {
                samples->getExample(k, input_k, target, weight);
                real dist = 0;
                int count = 0;
                for (int j = 0; j < input_k.length(); j++)
                    if (!is_missing(input_k[j])) {
                        dist += fabs(input_k[j] - cl_center[j]);
                        count++;
                    }
                if (count > 0)
                    dist /= real(count);
                distances(k, i) = dist;
                min_distances[k] = min(distances(k).subVec(0, i + 1));
            }
            farthest_sample = argmax(min_distances);
        }
    }

    PP<ProgressBar> pb;
    if (report_progress)
        pb = new ProgressBar("Performing K-Means to initialize centers", maxit);
    int iteration = maxit;
    TVec<VecStatsCollector> clust_stat(nclust);
    Vec clust_i;
    Vec nnonmissing(input.length());
    while(!ok && iteration--)
    {
        for (int i = 0; i < clust_stat.length(); i++)
            clust_stat[i].forget();
        old_clust_idx << clust_idx;
        for(int i=0;i<vmat_length;i++)
        {
            samples->getExample(i,input,target,weight);
            real dist,bestdist = REAL_MAX;
            int bestclust=0;
            if (nclust>1) for(int j=0;j<nclust;j++)
                if((dist = powdistance(input, clust(j), 2.0, true)) < bestdist)
                {
                    bestdist=dist;
                    bestclust=j;
                }
            clust_idx[i] = bestclust;
            clust_stat[bestclust].update(input, weight);
        }

        for (int i = 0; i < nclust; i++) {
            clust_i = clust(i);
            int j;
            for (j = 0;
                 j < input.length()
                    && clust_stat[i].stats.length() > 0
                    && is_equal(clust_stat[i].getStats(j).nnonmissing(), 0);
                 j++) {}
            if (j < input.length())
                // There have been some samples assigned to this cluster.
                clust_stat[i].getMean(clust_i);
            else {
                // Re-initialize randomly the cluster center.
                int new_center =
                    random_gen->uniform_multinomial_sample(vmat_length);
                if (!original_to_reordered.isEmpty())
                    new_center = original_to_reordered[new_center];
                samples->getExample(new_center, input, target, weight);
                clust_i << input;
            }
            // Replace missing values by randomly generated values.
            for (int k = 0; k < clust_i.length(); k++)
                if (is_missing(clust_i[k]))
                    clust_i[k] =
                        random_gen->gaussian_mu_sigma(mean_training  [k],
                                                      stddev_training[k]);
        }

        ok=true;

        if (nclust>1)
            for(int i=0;i<vmat_length;i++)
                if(old_clust_idx[i]!=clust_idx[i])
                {
                    ok=false;
                    break;
                }
        if (report_progress)
            pb->update(maxit - iteration + 1);
    }
    if (report_progress && verbosity >= 2 && iteration > 0)
        pout << "K-Means performed in only " << maxit - iteration << " iterations."
             << endl;
}

/////////////////
// log_density //
/////////////////
real GaussMix::log_density(const Vec& y) const
{
    log_likelihood_dens.resize(L);
    // First we need to compute the likelihood
    //   p(y,j | x) = p(y | x,j) * p(j | x).
    for (int j = 0; j < L; j++) {
        real logp_j_x = n_predictor == 0 ? pl_log(alpha[j])
                                     : log_p_j_x[j];
        log_likelihood_dens[j] = computeLogLikelihood(y, j) + logp_j_x;
        PLASSERT( !isnan(log_likelihood_dens[j]) );
    }
    return logadd(log_likelihood_dens);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GaussMix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(log_likelihood_post,      copies);
    deepCopyField(sample_row,               copies);
    deepCopyField(H3_inverse,               copies);
    deepCopyField(ptimer,                   copies);
    deepCopyField(missing_patterns,         copies);
    deepCopyField(missing_template,         copies);
    deepCopyField(sample_to_path_index,     copies);
    deepCopyField(spanning_path,            copies);
    deepCopyField(spanning_use_previous,    copies);
    deepCopyField(spanning_can_free,        copies);
    deepCopyField(log_likelihood_post_clust,copies);
    deepCopyField(clusters_samp,            copies);
    deepCopyField(cholesky_queue,           copies);
    deepCopyField(log_det_queue,            copies);
    deepCopyField(imputed_missing,          copies);
    deepCopyField(clust_imputed_missing,    copies);
    deepCopyField(sum_of_posteriors,        copies);
    deepCopyField(no_missing_change,        copies);
    deepCopyField(cond_var_inv_queue,       copies);
    deepCopyField(indices_queue,            copies);
    deepCopyField(indices_inv_queue,        copies);
    deepCopyField(mean_training,            copies);
    deepCopyField(stddev_training,          copies);
    deepCopyField(error_covariance,         copies);
    deepCopyField(posteriors,               copies);
    deepCopyField(initial_weights,          copies);
    deepCopyField(updated_weights,          copies);
    deepCopyField(eigenvectors_x,           copies);
    deepCopyField(eigenvalues_x,            copies);
    deepCopyField(y_x_mat,                  copies);
    deepCopyField(eigenvectors_y_x,         copies);
    deepCopyField(eigenvalues_y_x,          copies);
    deepCopyField(center_y_x,               copies);
    deepCopyField(log_p_j_x,                copies);
    deepCopyField(p_j_x,                    copies);
    deepCopyField(log_coeff,                copies);
    deepCopyField(log_coeff_x,              copies);
    deepCopyField(log_coeff_y_x,            copies);
    deepCopyField(joint_cov,                copies);
    deepCopyField(joint_inv_cov,            copies);
    deepCopyField(chol_joint_cov,           copies);
    // deepCopyField(chol_cov_template,        copies);
    deepCopyField(stage_joint_cov_computed, copies);
    deepCopyField(stage_replaced,           copies);
    deepCopyField(sample_to_template,       copies);
    deepCopyField(y_centered,               copies);
    deepCopyField(covariance,               copies);
    deepCopyField(log_likelihood_dens,      copies);
    deepCopyField(need_recompute,           copies);
    deepCopyField(original_to_reordered,    copies);

    deepCopyField(diags,                    copies);
    deepCopyField(eigenvalues,              copies);
    deepCopyField(eigenvectors,             copies);

    deepCopyField(alpha,                    copies);
    deepCopyField(center,                   copies);
    deepCopyField(sigma,                    copies);

    // TODO Update!
}

////////////////
// outputsize //
////////////////
int GaussMix::outputsize() const {
    int os = inherited::outputsize();
    for (size_t i = 0; i < outputs_def.length(); i++)
        if (outputs_def[i] == 'p')
            // We add L-1 because in inherited::outpusize() this was already
            // counted as 1.
            os += L - 1;
    return os;
}

//////////////////////////////////////////
// precomputeAllGaussianLogCoefficients //
//////////////////////////////////////////
void GaussMix::precomputeAllGaussianLogCoefficients()
{
    if (type_id == TYPE_SPHERICAL || type_id == TYPE_DIAGONAL) {
        // Nothing to do.
    } else {
        PLASSERT( type_id == TYPE_GENERAL );
        // Precompute the log_coeff.
        for (int j = 0; j < L; j++)
            log_coeff[j] = precomputeGaussianLogCoefficient(eigenvalues(j), D);
    }
}

//////////////////////////////////////
// precomputeGaussianLogCoefficient //
//////////////////////////////////////
real GaussMix::precomputeGaussianLogCoefficient(const Vec& eigenvals,
                                                int dimension) const
{
#ifdef BOUNDCHECK
    real last_eigenval = numeric_limits<double>::infinity();
#endif
    int n_eig = eigenvals.length();
    PLASSERT( dimension >= n_eig );
    real log_det = 0;
    real var_min = square(sigma_min);
    for (int k = 0; k < n_eig; k++) {
#ifdef BOUNDCHECK
        if (var_min < epsilon && eigenvals[k] < epsilon)
            PLWARNING("In GaussMix::precomputeGaussianLogCoefficient - An "
                      "eigenvalue is near zero");
        if (eigenvals[k] > last_eigenval)
            PLERROR("In GaussMix::precomputeGaussianLogCoefficient - The "
                    "eigenvalues must be sorted in decreasing order");
        last_eigenval = eigenvals[k];
#endif
        log_det += pl_log(max(var_min, eigenvals[k]));
    }
    if (dimension > n_eig)
        // Only the first 'n_eig' eigenvalues are given: we assume
        // the other eigenvalues are equal to the last given one.
        log_det += pl_log(max(var_min, eigenvals.lastElement()))
                 * (dimension - n_eig);
    return -0.5 * (dimension * Log2Pi + log_det);
}

/////////////////////
// replaceGaussian //
/////////////////////
void GaussMix::replaceGaussian(int j) {
    // This is supposed to be called only during training, when there is no
    // predictor part (we use the full joint distribution).
    PLASSERT( n_predictor == 0 );
    // Find the Gaussian with highest weight.
    int high = argmax(alpha);
    PLASSERT( high != j );
    // Generate the new center from this Gaussian.
    Vec new_center = center(j);
    generateFromGaussian(new_center, high);
    // Copy the covariance.
    if (type_id == TYPE_SPHERICAL) {
        sigma[j] = sigma[high];
    } else if (type_id == TYPE_DIAGONAL) {
        diags(j) << diags(high);
    } else {
        PLASSERT( type_id == TYPE_GENERAL );
        eigenvalues(j) << eigenvalues(high);
        eigenvectors[j] << eigenvectors[high];
        log_coeff[j] = log_coeff[high];
        stage_joint_cov_computed[j] = -1;
    }
    // Arbitrarily takes half of the weight of this Gaussian.
    alpha[high] /= 2.0;
    alpha[j] = alpha[high];
}

///////////////////////////
// resizeDataBeforeUsing //
///////////////////////////
void GaussMix::resizeDataBeforeUsing()
{
    eigenvectors_x.resize(0);
    eigenvectors_y_x.resize(0);
    joint_cov.resize(0);
    joint_inv_cov.resize(0);
    chol_joint_cov.resize(0);
    log_coeff.resize(0);
    log_coeff_x.resize(0);
    log_coeff_y_x.resize(0);
    stage_joint_cov_computed.resize(0);
    y_x_mat.resize(0);

    // chol_cov_template.resize(0, 0);
    center_y_x.resize(0, 0);
    eigenvalues_x.resize(0, 0);
    eigenvalues_y_x.resize(0, 0);

    // Type-specific data.
    switch(type_id)
    {
    case TYPE_SPHERICAL:
    case TYPE_DIAGONAL:
        break;
    case TYPE_GENERAL:
        eigenvectors_x.resize(L);
        eigenvectors_y_x.resize(L);
        joint_cov.resize(L);
        joint_inv_cov.resize(L);
        chol_joint_cov.resize(L);
        log_coeff_x.resize(L);
        log_coeff_y_x.resize(L);
        stage_joint_cov_computed.resize(L);
        stage_joint_cov_computed.fill(-1);
        y_x_mat.resize(L);

        // if (efficient_missing)
            // chol_cov_template.resize(efficient_k_median, L);
        if (n_predictor >= 0)
            eigenvalues_x.resize(L, n_predictor);
        if (n_predicted >= 0) 
        {
            center_y_x.resize(L, n_predicted);
            eigenvalues_y_x.resize(L, n_predicted);
        }
        log_coeff.resize(L);
        break;

    default:
        PLERROR("Invalid type_id");
    }
}

//////////////////////////////
// resizeDataBeforeTraining //
//////////////////////////////
void GaussMix::resizeDataBeforeTraining() {
    PLASSERT( train_set );

    n_eigen_computed = -1;

    nsamples = train_set->length();
    D = train_set->inputsize();

    if (f_eigen > 0){
        if (is_equal(f_eigen, 1))
            n_eigen = -1;
        else {
            n_eigen = int(round(f_eigen * D));
            if (n_eigen == 0)
                // We always want to keep at least one eigenvector.
                n_eigen = 1;
        }
    }

    alpha.resize(L);
    clust_imputed_missing.resize(0);
    eigenvectors.resize(0);
    H3_inverse.resize(0);
    imputed_missing.resize(0);
    mean_training.resize(0);
    no_missing_change.resize(0);
    sigma.resize(0);
    stddev_training.resize(0);

    center.resize(L, D);
    covariance.resize(0, 0);
    diags.resize(0, 0);
    eigenvalues.resize(0, 0);
    error_covariance.resize(0);
    initial_weights.resize(nsamples);
    //posteriors.resize(nsamples, L);
    //updated_weights.resize(L, nsamples);
    stage_replaced.resize(L);
    stage_replaced.fill(-1);

    // Type-specific data.
    switch(type_id)
    {
    case TYPE_SPHERICAL:
        sigma.resize(L);
        break;
    case TYPE_DIAGONAL:
        diags.resize(L, D);
        break;
    case TYPE_GENERAL:
        eigenvectors.resize(L);

        if (n_eigen == -1 || n_eigen == D)
            // We need to compute all eigenvectors.
            n_eigen_computed = D;
        else 
        {
            if (n_eigen > D || n_eigen < 1)
                PLERROR("In GaussMix::resizeDataBeforeTraining - Invalid value"
                        " for 'n_eigen' (%d), should be between 1 and %d",
                        n_eigen, D);
            n_eigen_computed = n_eigen + 1;
        }
        eigenvalues.resize(L, n_eigen_computed);
        for (int i = 0; i < eigenvectors.length(); i++)
            eigenvectors[i].resize(n_eigen_computed, D);
        if (impute_missing) 
        {
            H3_inverse.resize(L);
            error_covariance.resize(L);
            imputed_missing.resize(L);
            for (int j = 0; j < L; j++) 
            {
                error_covariance[j].resize(D, D);
                imputed_missing[j] = new MemoryVMatrix(nsamples, D);
            }
            /*
            PPath fname = "/u/delallea/tmp/imputed_missing.pmat";
            imputed_missing = new FileVMatrix(fname, nsamples, D);
            */
            // TODO May be useful to handle other types of VMats for large
            // datasets.
            // TODO Move outside of this method.
            if (efficient_missing == 1 || efficient_missing == 3)
                clust_imputed_missing.resize(L);
        }
        if (efficient_missing == 1 || efficient_missing == 3)
            no_missing_change.resize(nsamples);
        break;

    default:
        PLERROR("Invalid type_id");
    }
}

//////////////////
// setPredictor //
//////////////////
void GaussMix::setPredictor(const Vec& predictor, bool call_parent) const {
    static Vec log_p_x_j_alphaj;
    static Vec x_minus_mu_x; // Used to store 'x - mu_x'.
    static TVec<int> missing, non_missing;
    static Mat work_mat1, work_mat2;
    static Mat full_cov;
    static Mat cov_x_j;
    static Mat inv_cov_x;
    static Mat cov_y_x;
    static Mat cross_cov;
    static TVec<Mat> eigenvectors_x_miss;
    static Mat eigenvalues_x_miss;
    static TVec<Mat> y_x_mat_miss;

    if (call_parent)
        inherited::setPredictor(predictor);

    if (n_predictor == 0) {
        // There is no predictor part anyway: nothing to do.
        PLASSERT( predictor_part.isEmpty() );
        return;
    }

    if (stage == 0)
        // The Gaussian mixture is not ready yet (it has not yet been
        // trained): there is nothing more we can do.
        // Note that this is also why one needs to set a stage > 0 if the
        // Gaussian mixture parameters are set by hand (and not learnt).
        return;

    // We need to compute:
    // p(j | x) = p(x | j) p(j) / p(x)
    //          = p(x | j) p(j) / sum_k p(x | k) p(k)

    if (type_id == TYPE_GENERAL) {
        // We need to compute E[Y|x,j].
        if (!predictor_part.hasMissing()) {
            // Simple case: the predictor part has no missing value, and we can
            // re-use the quantities computed in setPredictorPredictedSizes(..).

            // If the previous predictor part set had missing values, we will
            // need to recompute some important variables (e.g. eigenvectors /
            // values of y|x). This can be done by re-setting the sizes.
            // TODO This is a bit hackish... we may want to actually store the
            // appropriate data elsewhere so that there is no need to recompute
            // it again.
            if (previous_predictor_part_had_missing)
                setPredictorPredictedSizes_const();

            previous_predictor_part_had_missing = false;
            x_minus_mu_x.resize(n_predictor);
            Vec mu_target;
            for (int j = 0; j < L; j++) {
                x_minus_mu_x << predictor_part;
                x_minus_mu_x -= center(j).subVec(0, n_predictor);
                mu_target = center_y_x(j);
                if (n_predictor > 0)
                    product(mu_target, y_x_mat[j], x_minus_mu_x);
                else
                    mu_target.fill(0);
                mu_target += center(j).subVec(n_predictor, n_predicted);
            }
        } else {
            previous_predictor_part_had_missing = true;
            // TODO Code duplication is ugly!
            non_missing.resize(0);
            missing.resize(0);
            for (int k = 0; k < predictor_part.length(); k++)
                if (!is_missing(predictor_part[k]))
                    non_missing.append(k);
                else
                    missing.append(k);
            int n_non_missing = non_missing.length();
            int n_missing = missing.length();
            int n_predicted_ext = n_predicted + n_missing;
            PLASSERT( n_missing + n_non_missing == n_predictor );

            work_mat1.resize(n_predicted_ext, n_non_missing);
            work_mat2.resize(n_predicted_ext, n_predicted_ext);
            Vec eigenvals;
            real var_min = square(sigma_min);
            eigenvalues_x_miss.resize(L, n_non_missing);
            eigenvectors_x_miss.resize(L);
            for (int j = 0; j < L; j++) {
                // First we compute the joint covariance matrix from the
                // eigenvectors and eigenvalues:
                // full_cov = sum_k (lambda_k - lambda0) v_k v_k' + lambda0.I
                // TODO Do we really need to compute the full matrix?

                PLASSERT( n_predictor + n_predicted == D );

                Mat& full_cov_j = full_cov;
                full_cov_j.resize(D, D);
                eigenvals = eigenvalues(j);
                real lambda0 = max(var_min, eigenvals[n_eigen_computed - 1]);

                full_cov_j.fill(0);
                Mat& eigenvectors_j = eigenvectors[j];
                PLASSERT( eigenvectors_j.width() == D );

                for (int k = 0; k < n_eigen_computed - 1; k++)
                    externalProductScaleAcc(
                            full_cov_j, eigenvectors_j(k),
                            eigenvectors_j(k),
                            max(var_min, eigenvals[k]) - lambda0);

                for (int i = 0; i < D; i++)
                    full_cov_j(i,i) += lambda0;

                // By construction, the resulting matrix is symmetric. However,
                // it may happen that it is not exactly the case due to numerical
                // approximations. Thus we ensure it is perfectly symmetric.
                PLASSERT( full_cov_j.isSymmetric(false) );
                fillItSymmetric(full_cov_j);

                // Extract the covariance of the predictor x.
                Mat cov_x_j_miss = full_cov.subMat(0, 0, n_predictor, n_predictor);
                cov_x_j.setMod(n_non_missing);
                cov_x_j.resize(n_non_missing, n_non_missing);
                for (int k = 0; k < n_non_missing; k++)
                    for (int p = k; p < n_non_missing; p++)
                        cov_x_j(k,p) = cov_x_j(p,k) =
                            cov_x_j_miss(non_missing[k], non_missing[p]);

                // Compute its inverse.
                /*
                inv_cov_x.resize(n_non_missing, n_non_missing);
                matInvert(cov_x_j, inv_cov_x);
                //PLASSERT( inv_cov_x.isSymmetric(false) );
                fillItSymmetric(inv_cov_x);
                */

#if 1
                // Compute its SVD.
                eigenvectors_x_miss[j].resize(n_non_missing, n_non_missing);
                eigenvals = eigenvalues_x_miss(j);
                eigenVecOfSymmMat(cov_x_j, n_non_missing, eigenvals,
                                  eigenvectors_x_miss[j]);

                // And its inverse (we'll need it for the covariance of y|x).
                inv_cov_x.resize(n_non_missing, n_non_missing);
                inv_cov_x.fill(0);
                if (n_non_missing > 0) {
                    // I am not sure about this assert, but since we extract the
                    // covariance of x from a matrix whose eigenvalues are all more
                    // than 'var_min', it looks like the eigenvalues of the
                    // covariance of x should also be more than 'var_min'. If I am
                    // wrong, remove the assert and see if it is needed to
                    // potentially set lambda0 to var_min.
                    PLASSERT( eigenvals.lastElement() > var_min ||
                            eigenvals.lastElement() / var_min > 0.99 );
                    lambda0 = eigenvals.lastElement();
                    real one_over_lambda0 = 1 / lambda0;
                    Mat& eigenvectors_x_j = eigenvectors_x_miss[j];
                    for (int k = 0; k < n_non_missing - 1; k++)
                        externalProductScaleAcc(
                                inv_cov_x, eigenvectors_x_j(k), eigenvectors_x_j(k),
                                1 / max(var_min, eigenvals[k]) - one_over_lambda0);
                    for (int i = 0; i < n_non_missing; i++)
                        inv_cov_x(i,i) += one_over_lambda0;
                }
#endif

                // Compute the covariance of y|x.
                // It is only needed when there is a predictor part, since
                // otherwise we can simply use the full covariance.
                // TODO See if we can use simpler formulas.
                Mat& cov_y_x_j = cov_y_x; // TODO Can we get rid of cov_y_x_j?
                cov_y_x_j.resize(n_predicted_ext, n_predicted_ext);
                cov_y_x_j.subMat(0, 0, n_predicted, n_predicted) <<
                    full_cov_j.subMat(n_predictor, n_predictor, n_predicted, n_predicted);
                for (int k = 0; k < n_missing; k++) {
                    int x_missing = missing[k];
                    for (int p = 0; p < n_predicted_ext; p++) {
                        if (p < n_predicted)
                            cov_y_x_j(n_predicted + k, p) =
                                cov_y_x_j(p, n_predicted + k) =
                                full_cov_j(x_missing, p + n_predictor);
                        else
                            cov_y_x_j(n_predicted + k, p) =
                                cov_y_x_j(p, n_predicted + k) =
                                full_cov_j(x_missing, missing[p - n_predicted]);
                    }
                }

                y_x_mat_miss.resize(L);
                y_x_mat_miss[j].resize(n_predicted, n_non_missing);
                if (n_non_missing > 0) {
                    cross_cov.resize(n_predicted_ext, n_non_missing);
                    for (int k = 0; k < n_non_missing; k++) {
                        for (int p = 0; p < n_predicted_ext; p++) {
                            if (p < n_predicted)
                                cross_cov(p, k) =
                                    full_cov_j(non_missing[k],p + n_predictor);
                            else
                                cross_cov(p, k) =
                                    full_cov_j(non_missing[k],
                                               missing[p - n_predicted]);
                        }
                    }
                                                            
                    /*
                       // Old (and BUGGED) code!
                    cross_cov =
                        full_cov_j.subMat(n_non_missing, 0,
                                          n_predicted_ext, n_non_missing);
                                          */
                    product(work_mat1, cross_cov, inv_cov_x);
                    productTranspose(work_mat2, work_mat1, cross_cov);
                    cov_y_x_j -= work_mat2;
                    y_x_mat_miss[j] << work_mat1.subMat(0, 0,
                                                   n_predicted, n_non_missing);
                }
                // Compute SVD of the covariance of y|x.
                eigenvectors_y_x[j].resize(n_predicted, n_predicted);
                eigenvals = eigenvalues_y_x(j);
                // Extract the covariance of the predicted part we are really
                // interested in.
                cov_y_x = cov_y_x_j.subMat(0, 0, n_predicted, n_predicted);
                // Ensure covariance matrix is perfectly symmetric.
                PLASSERT( cov_y_x.isSymmetric(false, true) );
                fillItSymmetric(cov_y_x);
                eigenVecOfSymmMat(cov_y_x, n_predicted,
                                  eigenvals, eigenvectors_y_x[j]);
                log_coeff_y_x[j] =
                    precomputeGaussianLogCoefficient(eigenvals, n_predicted);
            }

            x_minus_mu_x.resize(n_non_missing);
            Vec mu_target;
            for (int j = 0; j < L; j++) {
                for (int k = 0; k < n_non_missing; k++)
                    x_minus_mu_x[k] =
                        predictor_part[non_missing[k]] - center(j, non_missing[k]);
                mu_target = center_y_x(j);
                if (n_non_missing > 0)
                    product(mu_target, y_x_mat_miss[j], x_minus_mu_x);
                else
                    mu_target.fill(0);
                mu_target += center(j).subVec(n_predictor, n_predicted);
            }

        }
    }

    log_p_x_j_alphaj.resize(L);
    for (int j = 0; j < L; j++)
        log_p_x_j_alphaj[j] = computeLogLikelihood(predictor_part, j, true)
                            + pl_log(alpha[j]);

    real log_p_x = logadd(log_p_x_j_alphaj);

    log_p_j_x.resize(L);
    p_j_x.resize(L);
    for (int j = 0; j < L; j++) {
        real t = log_p_x_j_alphaj[j] - log_p_x;
        log_p_j_x[j] = t;
        p_j_x[j] = exp(t);
    }

}

///////////////////////////
// getInitialWeightsFrom //
///////////////////////////
void GaussMix::getInitialWeightsFrom(const VMat& vmat)
{
    PLASSERT( vmat->weightsize() == 1 );
    Vec tmp1, tmp2;
    real w;
    PLASSERT( vmat );
    PP<ProgressBar> pb;
    if (report_progress)
        pb = new ProgressBar("Getting sample weights from data set",
                             vmat->length());
    for (int i = 0; i < vmat->length(); i++) {
        vmat->getExample(i, tmp1, tmp2, w);
        initial_weights[i] = w;
        if (report_progress)
            pb->update(i + 1);
    }
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> GaussMix::getTrainCostNames() const
{
    static TVec<string> costs;
    if (costs.isEmpty()) {
        costs.append("init_time");
        costs.append("training_time");
    }
    return costs;
}

////////////////////////////////
// setPredictorPredictedSizes //
////////////////////////////////
bool GaussMix::setPredictorPredictedSizes(int n_i, int n_t,
                                   bool call_parent)
{
    bool sizes_changed = false;
    if (call_parent)
        sizes_changed =
            inherited::setPredictorPredictedSizes(n_i, n_t, call_parent);
    setPredictorPredictedSizes_const();
    return sizes_changed;
}

//////////////////////////////////////
// setPredictorPredictedSizes_const //
//////////////////////////////////////
void GaussMix::setPredictorPredictedSizes_const() const
{
    static Mat inv_cov_x;
    static Mat full_cov;
    static Mat cov_y_x;
    static Mat work_mat1, work_mat2;
    static Mat cross_cov;

    if (n_predictor == -1 || n_predicted == -1 || D == -1)
        // Sizes are not defined yet, there is nothing we can do.
        return;

    if (type_id == TYPE_SPHERICAL || type_id == TYPE_DIAGONAL ) {
        // Nothing to do.
    } else {
        PLASSERT( type_id == TYPE_GENERAL );

        work_mat1.resize(n_predicted, n_predictor);
        work_mat2.resize(n_predicted, n_predicted);
        Vec eigenvals;
        real var_min = square(sigma_min);
        // Resize some data accordingly.
        if (n_predictor >= 0)
            eigenvalues_x.resize(L, n_predictor);
        if (n_predicted >= 0) 
        {
            center_y_x.resize(L, n_predicted);
            eigenvalues_y_x.resize(L, n_predicted);
        }
        for (int j = 0; j < L; j++) {
            // Compute the covariance of x and y|x for the j-th Gaussian (we
            // will need them to compute the likelihood).

            // First we compute the joint covariance matrix from the
            // eigenvectors and eigenvalues:
            // full_cov = sum_k (lambda_k - lambda0) v_k v_k' + lambda0.I

            PLASSERT( n_predictor + n_predicted == D );
            Mat& full_cov_j = full_cov;
            full_cov_j.resize(D, D);
            eigenvals = eigenvalues(j);
            real lambda0 = max(var_min, eigenvals[n_eigen_computed - 1]);

            full_cov_j.fill(0);
            Mat& eigenvectors_j = eigenvectors[j];
            PLASSERT( eigenvectors_j.width() == D );

            for (int k = 0; k < n_eigen_computed - 1; k++)
                externalProductScaleAcc(full_cov_j, eigenvectors_j(k),
                                        eigenvectors_j(k),
                                        max(var_min, eigenvals[k]) - lambda0);
            for (int i = 0; i < D; i++)
                full_cov_j(i,i) += lambda0;

            // By construction, the resulting matrix is symmetric. However,
            // it may happen that it is not exactly the case due to numerical
            // approximations. Thus we ensure it is perfectly symmetric.
            PLASSERT( full_cov_j.isSymmetric(false) );
            fillItSymmetric(full_cov_j);

            // Extract the covariance of the predictor x.
            Mat cov_x_j = full_cov_j.subMat(0, 0, n_predictor, n_predictor);

            // Compute its SVD.
            eigenvectors_x[j].resize(n_predictor, n_predictor);
            eigenvals = eigenvalues_x(j);
            eigenVecOfSymmMat(cov_x_j, n_predictor, eigenvals, eigenvectors_x[j]);
            // Note that the computation above will have destroyed 'cov_x_j',
            // i.e. a part of the full covariance matrix.
            log_coeff_x[j] =
                precomputeGaussianLogCoefficient(eigenvals, n_predictor);


            // And its inverse (we'll need it for the covariance of y|x).
            inv_cov_x.resize(n_predictor, n_predictor);
            inv_cov_x.fill(0);
            if (n_predictor > 0) {
                // I am not sure about this assert, but since we extract the
                // covariance of x from a matrix whose eigenvalues are all more
                // than 'var_min', it looks like the eigenvalues of the
                // covariance of x should also be more than 'var_min'. If I am
                // wrong, remove the assert and see if it is needed to
                // potentially set lambda0 to var_min.
                PLASSERT( eigenvals[n_predictor - 1] > var_min ||
                        eigenvals[n_predictor - 1] / var_min > 0.99 );
                lambda0 = eigenvals[n_predictor - 1];
                real one_over_lambda0 = 1 / lambda0;
                Mat& eigenvectors_x_j = eigenvectors_x[j];
                for (int k = 0; k < n_predictor - 1; k++)
                    externalProductScaleAcc(
                        inv_cov_x, eigenvectors_x_j(k), eigenvectors_x_j(k),
                        1 / max(var_min, eigenvals[k]) - one_over_lambda0);
                for (int i = 0; i < n_predictor; i++)
                    inv_cov_x(i,i) += one_over_lambda0;
            }

            // Compute the covariance of y|x.
            // It is only needed when there is a predictor part, since
            // otherwise we can simply use the full covariance.
            // TODO See if we can use simpler formulas.
            Mat& cov_y_x_j = cov_y_x; // TODO Can we get rid of cov_y_x_j?
            cov_y_x_j.resize(n_predicted, n_predicted);
            cov_y_x_j <<
                full_cov_j.subMat(n_predictor, n_predictor, n_predicted, n_predicted);
            y_x_mat[j].resize(n_predicted, n_predictor);
            if (n_predictor > 0) {
                cross_cov = full_cov_j.subMat(n_predictor, 0,
                                              n_predicted, n_predictor);
                product(work_mat1, cross_cov, inv_cov_x);
                productTranspose(work_mat2, work_mat1, cross_cov);
                cov_y_x_j -= work_mat2;
                y_x_mat[j] << work_mat1;
            }
            // Compute SVD of the covariance of y|x.
            // TODO Note that if n_predictor == 0 (e.g. when using the Manifold
            // Parzen algorithm), the covariance of y|x is also the full
            // covariance, and thus we should instead re-use directly the
            // (possibly few) eigenvectors of the full covariance matrix
            // instead of wasting time and memory in the computations below.
            eigenvectors_y_x[j].resize(n_predicted, n_predicted);
            eigenvals = eigenvalues_y_x(j);
            // Ensure covariance matrix is perfectly symmetric.
            PLASSERT( cov_y_x_j.isSymmetric(false, true) );
            fillItSymmetric(cov_y_x_j);
            eigenVecOfSymmMat(cov_y_x_j, n_predicted, eigenvals, eigenvectors_y_x[j]);
            log_coeff_y_x[j] =
                precomputeGaussianLogCoefficient(eigenvals, n_predicted);
        }
    }
}

////////////////////
// setTrainingSet //
////////////////////
void GaussMix::setTrainingSet(VMat training_set, bool call_forget)
{
    if (efficient_missing != 2) {
        inherited::setTrainingSet(training_set, call_forget);
        return;
    }

    PP<ReorderByMissingVMatrix> reordered_training_set =
        new ReorderByMissingVMatrix();
    reordered_training_set->source = training_set;
    reordered_training_set->build();
    inherited::setTrainingSet((ReorderByMissingVMatrix*)reordered_training_set,
                              call_forget);
    // Now fill in the vector that indicates when the matrices need to be
    // recomputed.
    need_recompute.resize(training_set->length());
    need_recompute << reordered_training_set->missing_pattern_change;

    original_to_reordered.resize(training_set->length());
    for (int i = 0; i < training_set->length(); i++)
        original_to_reordered[reordered_training_set->indices[i]] = i;
}

// Boost graph property for edges in a binary tree.
struct MissingFlag {
    // Indicates whether a bit is flagged as missing.
    bool is_missing;
};

// Boost graph property for nodes in a binary tree.
// It is only used in leafs, to store the pattern's index.
struct NoProperty {
    int index;
};

/////////////////
// create_list //
/////////////////
void create_list(const TVec<int>& parent_, const TVec< TVec<int> >& children_,
                 TVec<int>& nodes_, TVec<bool>& use_previous_,
                 TVec<bool>& can_free_, int current_, bool cur_use_prev,
                 bool cur_can_free)
{
    // Create list of nodes in the tree.
    nodes_.append(current_);
    use_previous_.append(cur_use_prev);
    can_free_.append(cur_can_free);
    for (int i = 0; i < children_[current_].length(); i++) {
        cur_use_prev = (i == 0);
        cur_can_free = (i == children_[current_].length() - 1);
        create_list(parent_, children_, nodes_, use_previous_, can_free_,
                    children_[current_][i], cur_use_prev, cur_can_free);
    }
}

///////////
// train //
///////////
void GaussMix::train()
{
    ptimer->startTimer("training_time");
    // Standard PLearner checks.
    if (!initTrain())
        return;

    // When training, we want to learn the full joint distribution.
    int backup_predicted_size = predicted_size;
    int backup_predictor_size = predictor_size;
    bool need_restore_sizes = setPredictorPredictedSizes(0, -1);

    // Initialization before training.
    if (stage == 0) {
        ptimer->startTimer("init_time");

        // Precompute nodes of the missing graph.
        typedef boost::adjacency_list<boost::listS, boost::vecS,
                boost::directedS, NoProperty, MissingFlag> BinaryBitsTree;
        typedef boost::graph_traits<BinaryBitsTree>::vertex_iterator vertex_iter;
        typedef boost::graph_traits<BinaryBitsTree>::vertex_descriptor vertex_descr;
        typedef boost::graph_traits<BinaryBitsTree>::out_edge_iterator oedge_iter;
        typedef boost::graph_traits<BinaryBitsTree>::edge_descriptor edge_descr;
        typedef std::pair<oedge_iter, oedge_iter> oedge_iter_pair;

        BinaryBitsTree tree(1);
        const vertex_descr& root_vertex = *(boost::vertices(tree).first);
        PP<ProgressBar> pb;
        if ((efficient_missing == 1 || efficient_missing == 3)
                && report_progress)
            pb = new ProgressBar("Finding unique missing patterns",
                                 train_set->length());
        Vec input, target;
        real weight;
        int n_unique = 0;
        missing_patterns.resize(0, train_set->inputsize());
        TVec<bool> pattern(train_set->inputsize());
        sample_to_template.resize(train_set->length());
        TVec< TVec<int> > pattern_to_samples;
        for (int i = 0; (efficient_missing == 1 || efficient_missing == 3)
                && i < train_set->length(); i++) {
            train_set->getExample(i, input, target, weight);
            vertex_descr current_vertex = root_vertex;
            for (int k = 0; k < input.length(); k++) {
                bool bit = is_missing(input[k]);
                pattern[k] = bit;

                const oedge_iter_pair& oeiter_pair =
                    boost::out_edges(current_vertex, tree);
                oedge_iter oeiter = oeiter_pair.first;
                while (oeiter != oeiter_pair.second &&
                        tree[*oeiter].is_missing != bit) {
                    oeiter++;
                }
                if (oeiter == oeiter_pair.second) {
                    // Could not find this bit: need to create new vertex and
                    // edge.
                    const vertex_descr& new_vertex = boost::add_vertex(tree);
                    const edge_descr& new_edge =
                        boost::add_edge(current_vertex, new_vertex,tree).first;
                    tree[new_edge].is_missing = bit;
                    current_vertex = new_vertex;
                    if (k == input.length() - 1) {
                        // This is a leaf.
                        n_unique++;
                        missing_patterns.appendRow(pattern);
                        int index = missing_patterns.length() - 1;
                        tree[current_vertex].index = index;
                        pattern_to_samples.append(TVec<int>());
                    }
                } else {
                    // We found an existing edge.
                    current_vertex = boost::target(*oeiter, tree);
                }
                if (k == input.length() - 1) {
                    // Leaf node.
                    // First step: each sample is assigned to its missing
                    // pattern.
                    int pattern_idx = tree[current_vertex].index;
                    sample_to_template[i] = pattern_idx;
                    pattern_to_samples[pattern_idx].append(i);
                    // pout << sample_to_template[i] << endl;
                }
            }
            if (report_progress)
                pb->update(i + 1);
        }

        //TVec<int> sample_to_pattern = sample_to_template.copy();

        if ((efficient_missing == 1 || efficient_missing == 3)
                && verbosity >= 2)
            pout << "Found " << n_unique << " unique missing patterns" << endl;

        if (efficient_missing == 1 || efficient_missing == 3) {
            // Perform some kind of k-median on missing patterns for initial
            // clustering of missing patterns.
            TVec<int> indices(0, missing_patterns.length() - 1, 1);
            // TODO Use random_gen (but -> different k-means initialization)
            PRandom::common(false)->shuffleElements(indices);
            int n_clusters = min(efficient_k_median,
                                 missing_patterns.length());
            missing_template.resize(
                    n_clusters, missing_patterns.width());
            TVec<int> missing_assign(missing_patterns.length(), -1);
            for (int i = 0; i < n_clusters; i++) {
                missing_template(i) << missing_patterns(indices[i]);
            }
            bool finished = false;
            TVec<int> n_diffs(n_clusters);
            int count_iter = 0;
            if (report_progress)
                pb = new ProgressBar("Performing k-median on " +
                        tostring(missing_patterns.length())    +
                        " missing patterns", efficient_k_median_iter);
            TMat<int> majority(n_clusters, missing_patterns.width());
            static TVec<int> n_assigned;
            while (!finished && count_iter < efficient_k_median_iter) {
                finished = true;
                // Assign each missing pattern to closest template.
                n_assigned.resize(n_clusters);
                n_assigned.fill(0);
                for (int i = 0; i < missing_patterns.length(); i++) {
                    n_diffs.fill(0);
                    for (int j = 0; j < n_clusters; j++)
                        for (int k = 0; k < missing_patterns.width(); k++)
                            if (missing_patterns(i, k) !=
                                missing_template(j, k))
                                n_diffs[j]++;
                    int new_assign = argmin(n_diffs);
                    if (new_assign != missing_assign[i])
                        finished = false;
                    missing_assign[i] = new_assign;
                    n_assigned[new_assign]++;
                }
                // Recompute missing templates.
                majority.fill(0);
                for (int i = 0; i < missing_patterns.length(); i++) {
                    int assign = missing_assign[i];
                    for (int k = 0; k < missing_patterns.width(); k++) {
                        if (missing_patterns(i, k))
                            majority(assign, k)++;
                        else
                            majority(assign, k)--;
                    }
                }
                for (int j = 0; j < n_clusters; j++) {
                    bool not_too_many_samples =
                        max_samples_in_cluster == -1 ||
                        n_assigned[j] <= max_samples_in_cluster;
                    bool not_too_few_samples =
                        n_assigned[j] >= min_samples_in_cluster ||
                        (n_clusters == 1)                       ||
                        n_assigned[j] == -1; // Newly created cluster.
                    bool is_valid_cluster = not_too_many_samples    &&
                                            not_too_few_samples     &&
                                            n_assigned[j] != -1;
                    if (is_valid_cluster) {
                    for (int k = 0; k < missing_template.width(); k++)
                        if (majority(j, k) > 0)
                            missing_template(j, k) = true;
                        else if (majority(j, k) < 0)
                            missing_template(j, k) = false;
                        else
                            // TODO Use random_gen (but be careful to effects,
                            // e.g. kmeans initialization).
                            missing_template(j, k) =
                                (PRandom::common(false)->uniform_sample() < 0.5);
                    } else if (!not_too_many_samples) {
                        // This cluster has too many points assigned to it
                        // (more than 'max_samples_in_cluster'). We split it in
                        // two, by picking two new centers, randomly chosen in
                        // this cluster.
                        static TVec<int> cluster_samples;
                        cluster_samples.resize(0);
                        for (int i = 0; i < missing_assign.length(); i++)
                            if (missing_assign[i] == j)
                                cluster_samples.append(i);
                        int center_1 =
                            PRandom::common(false)->uniform_multinomial_sample(cluster_samples.length());
                        missing_template(j) << missing_patterns(center_1);
                        bool found_valid_center_2 = false;
                        int center_2 = -1;
                        while (!found_valid_center_2) {
                            center_2 =
                                PRandom::common(false)->uniform_multinomial_sample(cluster_samples.length());
                            found_valid_center_2 = false;
                            for (int k = 0; k < missing_template.width(); k++)
                                if (missing_template(j, k) !=
                                        missing_patterns(center_2, k)) {
                                    found_valid_center_2 = true;
                                    break;
                                }
                        }
                        n_clusters++;
                        majority.resize(n_clusters, majority.width());
                        n_diffs.resize(n_clusters);
                        n_assigned.resize(n_clusters);
                        n_assigned.last() = -1;
                        missing_template.resize(n_clusters,
                                                missing_template.width());
                        missing_template(n_clusters - 1) <<
                            missing_patterns(center_2);
                        finished = false;
                        if (verbosity >= 10)
                            pout << "Cluster " << j << " split in two (" <<
                                n_assigned[j] << " > " <<
                                max_samples_in_cluster << "), there are now "
                                << n_clusters << " clusters." << endl;
                    } else if (!not_too_few_samples) {
                        // This cluster has no point assigned to it.
                        // If we can merge it with an existing cluster, we do
                        // so, otherwise we assign its center to a new pattern
                        // chosen randomly in the patterns set.
                        int candidate = 0;
                        while (candidate < n_clusters) {
                            if (n_assigned[candidate] > 0 && candidate != j &&
                                    (max_samples_in_cluster == -1 ||
                                    n_assigned[candidate] + n_assigned[j] <=
                                        max_samples_in_cluster)) {
                                // This candidate cluster can be added the
                                // points in the j-th cluster without violating
                                // the maximum number of samples constraint.
                                break;
                            }
                            candidate++;
                        }
                        if (candidate < n_clusters) {
                            // We have found a valid candidate: we can delete
                            // this cluster.
                            // Note that actually, we have no reason to believe
                            // that the samples in this cluster are going to be
                            // assigned to our candidate template.
                            n_assigned[candidate] += n_assigned[j];
                            n_clusters--;
                            for (int k = j; k < n_clusters; k++) {
                                n_assigned[k] = n_assigned[k + 1];
                                missing_template(k) << missing_template(k + 1);
                            }
                            n_assigned.resize(n_clusters);
                            missing_template.resize(n_clusters,
                                    missing_template.width());
                            n_diffs.resize(n_clusters);
                            majority.resize(n_clusters, majority.width());
                            if (verbosity >= 10)
                                pout << "Cluster " << j << " deleted (" <<
                                    n_assigned[j] << " < " <<
                                    min_samples_in_cluster << "), there are now "
                                    << n_clusters << " clusters." << endl;
                        } else {
                            // No valid candidate: we reset this cluster
                            // randomly.
                            int random_pattern =
                            PRandom::common(false)->uniform_multinomial_sample(
                                    missing_patterns.length());
                        missing_template(j) <<
                            missing_patterns(random_pattern);
                        missing_assign[random_pattern] = j;
                            if (verbosity >= 10)
                                pout << "Cluster " << j << " has been reset to"
                                     << " a random new center" << endl;
                        }
                        finished = false;
                    } else if (n_assigned[j] == -1) {
                        // Note: this case happens only for a newly created
                        // center (when we split a cluster in two).
                        finished = false;
                    }
                }

                count_iter++;
                if (report_progress)
                    pb->update(count_iter);
            }
            if (finished && verbosity >= 2)
                pout << "K-median stopped after only " << count_iter
                     << " iterations" << endl;

            if (finished && verbosity >= 5)
                pout << "Number of points in each cluster: " << n_assigned
                     << endl;

            // Because right now we only want to perform updates, we need to
            // make sure there will be no need for downdates.
            /* Actually we can do downdates now!
            for (int i = 0; i < missing_patterns.length(); i++) {
                int assign = missing_assign[i];
                for (int k = 0; k < missing_patterns.width(); k++)
                    if (missing_patterns(i, k))
                        missing_template(assign, k) = true;
            }
            */

            // Second step to fill 'sample_to_template'.
            for (int i = 0; i < sample_to_template.length(); i++)
                sample_to_template[i] = missing_assign[sample_to_template[i]];

            // Fill in list for each cluster.
            TVec< TVec<int> > clusters(missing_template.length());
            for (int i = 0; i < missing_patterns.length(); i++)
                clusters[missing_assign[i]].append(i);

            TVec<int> parent;
            // Fill in list for each sample.
            // TODO Note: cluster_samp and sample_to_template may not really be
            // useful.
            clusters_samp.resize(missing_template.length());
            for (int i = 0; i < clusters_samp.length(); i++)
                clusters_samp[i].resize(0);
            for (int i = 0; i < train_set->length(); i++)
                // clusters_samp[missing_assign[sample_to_template[i]]].append(i);
                clusters_samp[sample_to_template[i]].append(i);

            if (efficient_missing == 1 || efficient_missing == 3) {
#ifdef DIRECTED_HACK
                typedef boost::adjacency_list < boost::vecS, boost::vecS,
                    boost::directedS,
                    boost::property<boost::vertex_distance_t, int>,
                    boost::property<boost::edge_weight_t, int > > DistGraph;
#else
                typedef boost::adjacency_list < boost::vecS, boost::vecS,
                    boost::undirectedS,
                    boost::property<boost::vertex_distance_t, int>,
                    boost::property<boost::edge_weight_t, int > > DistGraph;
#endif
                // TODO According to
                // http://boost-consulting.com/boost/libs/graph/doc/adjacency_matrix.html
                // we should be using adjacency_matrix instead!
                // TODO Do I really need all these properties? (in particular
                // the vertex property?)
                typedef std::pair<int, int> Edge;

                spanning_path.resize(missing_template.length());
                spanning_use_previous.resize(missing_template.length());
                spanning_can_free.resize(missing_template.length());
                for (int tpl = 0; tpl < missing_template.length(); tpl++) {
                // Find minimum spanning tree of the missing patterns' graph.
                TVec<int> cluster_tpl = clusters[tpl];
                int n = cluster_tpl.length();
                n = (n * (n - 1)) / 2;
                if (report_progress && verbosity >= 2)
                    pb = new ProgressBar("Building graph of missing patterns",
                                         n);
#ifdef DIRECTED_HACK
                n *= 2;
#endif
                TVec<int> weights(n);
                TVec<Edge> edges(n);
                weights.resize(0);
                edges.resize(0);
                int progress = 0;
                /*
                PStream out = openFile("/u/delallea/tmp/edges.amat",
                        PStream::raw_ascii, "w");
                        */
                for (int i = 0; i < cluster_tpl.length(); i++) {
                    for (int j = i + 1; j < cluster_tpl.length(); j++) {
                        edges.append( Edge(i, j) );
#ifdef DIRECTED_HACK
                        edges.append( Edge(j, i) );
#endif
                        int w = 0;
#ifdef DIRECTED_HACK
                        int w_minus = 0;
#endif
                        bool* missing_i = missing_patterns[cluster_tpl[i]];
                        bool* missing_j = missing_patterns[cluster_tpl[j]];
                        for (int k = 0; k < missing_patterns.width(); k++) {
                            if (*missing_i != *missing_j)
#ifdef DIRECTED_HACK
                                if (*missing_j)
                                    w++;
                                else
                                    w_minus++;
#else
                                w++;
#endif
                            missing_i++;
                            missing_j++;
                        }
#ifdef DIRECTED_HACK
                        weights.append(10 * w + w_minus);
                        weights.append(w + 10 * w_minus);
#else
                        weights.append(w);
#endif
                        /*
                        out << "E(" << i << ", " << j << "), ";
                        out << w << ", ";
                        */
                    }
                    progress += cluster_tpl.length() - i - 1;
                    if (pb)
                        pb->update(progress);
                }
                // out.flush();
                parent.resize(0);
                if (edges.isEmpty()) {
                    parent.resize(1);
                    parent[0] = 0;
                } else {
                Edge* edges_ptr = edges.data();
                DistGraph dist_graph(
                        edges_ptr,
                        edges_ptr + edges.length(),
                        weights.data(), cluster_tpl.length());
                // boost::property_map<DistGraph, boost::edge_weight_t>::type
                //    weightmap = boost::get(boost::edge_weight, dist_graph);
                typedef vector < boost::graph_traits <
                                    DistGraph >::vertex_descriptor > Predec;
                Predec pred(boost::num_vertices(dist_graph));
                if (verbosity >= 2)
                    pout << "Computing minimum spanning tree... " << flush;
                boost::prim_minimum_spanning_tree(dist_graph, &pred[0]);
                if (verbosity >= 2)
                    pout << "Done" << endl;
                // Convert 'pred' to a PLearn parent vector.
                parent.resize(int(pred.size()));
                for (std::size_t i = 0; i != pred.size(); i++)
                    parent[int(i)] = int(pred[i]);

                /*
                // Code to save the graph to display it in Matlab.
                out = openFile("/u/delallea/tmp/tree.amat",
                        PStream::raw_ascii, "w");
                for (int i = 0; i < parent.length(); i++)
                    if (parent[i] != i)
                        out << parent[i] + 1 << " ";
                    else
                        out << 0 << " ";

                out = openFile("/u/delallea/tmp/weight.amat",
                        PStream::raw_ascii, "w");
                for (int i = 0; i < parent.length(); i++) {
                    int j = parent[i];
                    // Looking for weight between nodes i and j.
                    int w = 0;
                    bool* missing_i = missing_patterns[cluster_tpl[i]];
                    bool* missing_j = missing_patterns[cluster_tpl[j]];
                    for (int k = 0; k < missing_patterns.width(); k++) {
                        if (*missing_i != *missing_j)
                            w++;
                        missing_i++;
                        missing_j++;
                    }
                    out << w << " ";
                }
                */
                // Free memory used by weights and edges.
                weights = TVec<int>();
                edges = TVec<Edge>();

                }
#if 0
                Mat parent_mat(1, parent.length());
                for (int p = 0; p < parent.length(); p++)
                    parent_mat(0, p) = parent[p];
                VMat parent_vm(parent_mat);
                parent_vm->saveAMAT("/u/delallea/tmp/parent.amat", false,
                        true);
                // Easy verification of cost.
                int sum_add = 0;
                int sum_min = 0;
                for (int q = 0; q < parent.length(); q++) {
                    if (parent[q] == q)
                        continue;
                    TVec<bool> v1 = missing_patterns(q);
                    TVec<bool> v2 = missing_patterns(parent[q]);
                    for (int r = 0; r < v1.length(); r++) {
                        if (v1[r] && !v2[r])
                            sum_add++;
                        else if (!v1[r] && v2[r])
                            sum_min++;
                    }
                }
                pout << "Easy check: " << sum_add << " and " << sum_min <<
                    endl;
#endif

                n = cluster_tpl.length();
#ifdef DIRECTED_HACK
#else
                // Compute list of nodes, from top to bottom.
                TVec<int> top_to_bottom;
                TVec<int> status(n, 0);
                PLASSERT( parent.length() == n );
                // Status: 0 = still has a parent
                //         1 = candidate with no parent
                //         2 = done
                TVec< TVec<int> > children(n);
                for (int i = 0; i < parent.length(); i++)
                    if (parent[i] != i)
                        children[ parent[i] ].append(i);
                    else
                        status[int(i)] = 1;
                // Ensure there is only a single one in the resulting tree.
                PLASSERT( status.find(1, status.find(1) + 1) == -1 );
                int count = 0;
                // Now we're ready to loop over all elements.
                while (true) {
                    int last_count = count;
                    bool loop = false;
                    // Find the next candidate with no parent.
                    while (status[count] != 1 &&
                           (!loop || count != last_count)) {
                        count++;
                        if (count >= n) {
                            count -= n;
                            loop = true;
                        }
                    }
                    if (count == last_count && loop) {
                        // We must have gone through all nodes.
                        PLASSERT( status.find(0) == -1 );
                        break;
                    }
                    status[count] = 2;
                    top_to_bottom.append(count);
                    TVec<int> child = children[count];
                    for (int i = 0; i < child.length(); i++) {
                        int j = child[i];
                        PLASSERT( status[j] == 0 );
                        status[j] = 1;
                    }
                }

                // Initialize messages.
                TVec<int> message_up(n, 0);
                TVec<int> message_down(n, 0);

                // Upward pass of messages.
                for (int i = n - 1; i >= 0; i--) {
                    int k = top_to_bottom[i];
                    TVec<int> child = children[k];
                    if (child.isEmpty())
                        // Leaf node.
                        continue;
                    int max = -1;
                    bool balanced = false;
                    for (int j = 0; j < child.length(); j++) {
                        int msg_up = message_up[child[j]];
                        if (msg_up > max) {
                            max = msg_up;
                            balanced = false;
                        } else if (msg_up == max)
                            balanced = true;
                    }
                    if (balanced)
                        max++;
                    PLASSERT( max >= 0 );
                    message_up[k] = max;
                }

                // Downward pass of messages.
                for (int q = 0; q < n; q++) {
                    int j = top_to_bottom[q];
                    int i = parent[j];
                    TVec<int> brothers = children[i];
                    int max = -1;
                    bool balanced = false;
                    for (int k = 0; k < brothers.length(); k++) {
                        int brother_k = brothers[k];
                        if (brother_k == j)
                            // We do not consider this node.
                            continue;
                        int msg_up = message_up[brother_k];
                        if (msg_up > max) {
                            max = msg_up;
                            balanced = false;
                        } else if (msg_up == max)
                            balanced = true;
                    }
                    int msg_down = message_down[i];
                    if (msg_down > max) {
                        max = msg_down;
                        balanced = false;
                    } else if (msg_down == max)
                        balanced = true;
                    if (balanced)
                        max++;
                    // Note that 'max' can be zero when we have only one single
                    // point.
                    PLASSERT( max > 0 || n == 1);
                    message_down[j] = max;
                }

                // Compute the cost.
                TVec<int> cost(n, -1);
                for (int i = 0; i < n; i++) {
                    int msg_up = message_up[i];
                    int msg_down = message_down[i];
                    if (msg_up == msg_down)
                        cost[i] = msg_up + 1;
                    else
                        cost[i] = max(msg_up, msg_down);
                }
                int min_cost = min(cost);
                if (verbosity >= 5)
                    pout << "Minimum cost: " << min_cost << endl;

                // Find the node to start from.
                int start_node = argmin(cost);
                PLASSERT( cost[start_node] == min_cost );
#endif // DIRECTED_HACK

                // Compute a node ordering giving rise to the mininum cost.
                TVec<int>& span_path = spanning_path[tpl];
                TVec<bool>& span_use_previous = spanning_use_previous[tpl];
                TVec<bool>& span_can_free = spanning_can_free[tpl];
                span_path.resize(0);
                span_use_previous.resize(0);
                span_can_free.resize(0);
                // Note: 'free_previous' is set to 'false', meaning we might be
                // using one more matrix than necessary. TODO Investigate
                // exactly how this should be done.
#ifdef DIRECTED_HACK
                // Compute list of nodes, in the order they will be visited in
                // the optimization process. Note that this may not be optimal
                // memory-wise.
                TVec< TVec<int> > children(n);
                // First find the root and fill the children lists.
                int root = -1;
                for (int i = 0; i < parent.length(); i++)
                    if (parent[i] == i)
                        root = i;
                    else
                        children[ parent[i] ].append(i);
                PLASSERT( root >= 0 );
                // Then deduce the ordered list of nodes.
                create_list(parent, children, span_path, span_use_previous,
                            span_can_free, root, true, false);
#else
                traverse_tree(span_path, span_can_free, span_use_previous,
                              false, true, start_node, -1, parent,
                              children, message_up, message_down);
#endif
                PLASSERT( span_path.length()          == n );
                PLASSERT( span_can_free.length()      == n );
                PLASSERT( span_use_previous.length()  == n );
                // At this point the index in 'span_path' are the index within
                // the cluster 'tpl': we replace them by the global sample
                // index.
                for (int i = 0; i < span_path.length(); i++)
                    span_path[i] = cluster_tpl[span_path[i]];

                // Consistency check: compute the average distance from one
                // node to the next in the path.
                int sum = 0;
                int counter = 0;
                Vec stats_diff(missing_patterns.width() + 1);
                stats_diff.fill(0);
                for (int i = 0; i < span_path.length() - 1; i++) {
                    int first = span_path[i];
                    int next = span_path[i + 1];
                    int dist = 0;
                    for (int k = 0; k < missing_patterns.width(); k++)
                        if (missing_patterns(first, k) !=
                            missing_patterns(next, k))
                            dist++;
                    sum += dist;
                    counter ++;
                    stats_diff[dist]++;
                }
                real avg_dist = 0;
                if (counter > 0)
                    avg_dist = sum / real(counter);
                // TODO Note that the quantity below is not exactly what we're
                // interested in: it does not take into account the fact that
                // we come back in the tree (branch switching).
                if (verbosity >= 5)
                    pout << "Average distance to next pattern: " << avg_dist
                         << endl;
                /*
                Mat tomat = stats_diff.toMat(stats_diff.length(), 1);
                VMat save_vmat(tomat);
                save_vmat->saveAMAT("/u/delallea/tmp/span_" +
                        tostring(efficient_k_median) + ".amat", true, true);
                        */

              }
                // Transform 'spanning_path' to obtain a path through samples,
                // instead of a path through missing patterns.
                // First get the list of samples associated to each missing
                // pattern.
                TVec<int> the_path;
                TVec<bool> the_can_free;
                TVec<bool> the_use_prev;
                sample_to_path_index.resize(train_set->length());
                sample_to_path_index.fill(-1);
                for (int i = 0; i < spanning_path.length(); i++) {
                    TVec<int>& span_path = spanning_path[i];
                    TVec<bool>& span_can_free = spanning_can_free[i];
                    TVec<bool>& span_use_prev = spanning_use_previous[i];

                    the_path.resize(span_path.length());
                    the_can_free.resize(span_can_free.length());
                    the_use_prev.resize(span_use_prev.length());
                    the_path     << span_path;
                    the_can_free << span_can_free;
                    the_use_prev << span_use_prev;
                    span_path.resize(0);
                    span_can_free.resize(0);
                    span_use_prev.resize(0);
                    int count = 0;
                    for (int j = 0; j < the_path.length(); j++) {
                        const TVec<int>& samples_list =
                            pattern_to_samples[the_path[j]];
                        span_path.append(samples_list);
                        span_can_free.append(the_can_free[j]);
                        span_use_prev.append(the_use_prev[j]);
                        for (int k = 0; k < samples_list.length(); k++) {
                            PLASSERT(sample_to_path_index[samples_list[k]]==-1);
                            sample_to_path_index[samples_list[k]] = count;
                            count++;
                            // Other samples with same pattern will reuse the
                            // same covariance matrix. However, right now, it
                            // is not completely efficient since the matrix
                            // will still be copied.
                            if (k > 0) {
                                span_can_free.append(true);
                                span_use_prev.append(true);
                            }
                        }
                    }
#ifdef BOUNDCHECK
                    int n_samples_in_cluster = clusters_samp[i].length();
                    PLASSERT( span_path.length()      == n_samples_in_cluster );
                    PLASSERT( span_can_free.length()  == n_samples_in_cluster );
                    PLASSERT( span_use_prev.length()  == n_samples_in_cluster );
#endif
                }
                // Make sure all samples belong to a path.
                PLASSERT( sample_to_path_index.find(-1) == -1 );
            }

            // Compute some statistics on the distances to templates.
#if 0
            Vec current_vec, previous_vec;
            Vec count_added(10000, real(0));
            Vec count_removed(10000, real(0));
            int max_added = 0;
            int max_removed = 0;
            int sum_added = 0;
            int sum_removed = 0;
            int counter_added = 0;
            int counter_removed = 0;
            map<int, int> current_to_previous;
            TVec<int> is_there(train_set->length(), 0);
            for (int i = 0; i < spanning_path.length(); i++) {
                TVec<int>& span_path = spanning_path[i];
                TVec<bool>& span_use_prev = spanning_use_previous[i];
                TVec<bool>& span_can_free = spanning_can_free[i];
                TVec<int> cached_nodes;
                cached_nodes.append(0);
                int queue_index = 0;
                for (int k = 1; k < span_path.length(); k++) {
                    if (span_use_prev[k])
                        queue_index = cached_nodes.length() - 1;
                    else
                        queue_index = cached_nodes.length() - 2;
                    int previous = cached_nodes[queue_index];
                    int index_current = span_path[k];
                    train_set->getExample(index_current,
                                          input, target, weight);
                    current_vec.resize(input.length());
                    current_vec << input;

                    int index_previous = span_path[previous];
                    train_set->getExample(index_previous, input,
                                          target, weight);
                    previous_vec.resize(input.length());
                    previous_vec << input;
                    is_there[index_current] = 1;
                    is_there[index_previous] = 1;
                    int current_pattern = sample_to_pattern[index_current];
                    int previous_pattern = sample_to_pattern[index_previous];
                    if (current_pattern          == previous_pattern ||
                        parent[current_pattern]  == previous_pattern ||
                        parent[previous_pattern] == current_pattern)
                    {} else
                    {
                        PLERROR("Houston, we have a problem!");
                    }
                    if (current_pattern != previous_pattern)
                        current_to_previous[index_current] = index_previous;
                    int n_added = 0;
                    int n_removed = 0;
                    for (int q = 0; q < input.length(); q++) {
                        if (is_missing(current_vec[q])) {
                            if (!missing_patterns(current_pattern, q))
                                PLERROR("No way!");
                        } else if (missing_patterns(current_pattern, q))
                            PLERROR("Way no!");
                            
                        if (is_missing(previous_vec[q]) &&
                            !is_missing(current_vec[q]))
                            n_added++;
                        else if (!is_missing(previous_vec[q]) &&
                                 is_missing(current_vec[q]))
                            n_removed++;
                    }
                    count_added[n_added]++;
                    count_removed[n_removed]++;
                    sum_added += n_added;
                    sum_removed += n_removed;
                    counter_added++;
                    counter_removed++;
                    if (n_added > max_added)
                        max_added = n_added;
                    if (n_removed > max_removed)
                        max_removed = n_removed;
                    if (span_can_free[k])
                        cached_nodes.resize(queue_index);
                    else if (!span_use_prev[k])
                        cached_nodes.resize(cached_nodes.length() - 1);
                    cached_nodes.append(k);
                }
            }
            if (is_there.find(0) != -1)
                PLERROR("OMG!");
            pout << "Mean added  : " << sum_added << "/" << counter_added << " = "
                << sum_added / real(counter_added) << endl;
            pout << "Mean removed: " << sum_removed << "/" << counter_removed << " = "
                << sum_removed / real(counter_removed) << endl;

            Mat cur_to_prev(current_to_previous.size(), 2);
            map<int, int>::const_iterator it = current_to_previous.begin();
            int count_i = 0;
            for (; it != current_to_previous.end(); it++, count_i++) {
                if (it->first < it->second) {
                    cur_to_prev(count_i, 0) = it->first;
                    cur_to_prev(count_i, 1) = it->second;
                } else {
                    cur_to_prev(count_i, 0) = it->second;
                    cur_to_prev(count_i, 1) = it->first;
                }
            }
            PP<SortRowsVMatrix> cur_to_prev_vm = new SortRowsVMatrix();
            cur_to_prev_vm->source = VMat(cur_to_prev);
            cur_to_prev_vm->sort_columns = TVec<int>(0, 1, 1);
            cur_to_prev_vm->build();
            cur_to_prev_vm->saveAMAT("/u/delallea/tmp/cur_to_prev.amat",
                    false, true);

            count_added.resize(max_added + 1);
            count_removed.resize(max_removed + 1);
            Mat added_mat = count_added.toMat(1, count_added.length());
            Mat removed_mat = count_removed.toMat(1, count_removed.length());
            VMat(added_mat)->saveAMAT("/u/delallea/tmp/added.amat", false,
                    true);
            VMat(removed_mat)->saveAMAT("/u/delallea/tmp/removed.amat", false,
                    true);

            /*
            Vec stats_diff(missing_patterns.width());
            stats_diff.fill(0);
            for (int i = 0; i < missing_patterns.length(); i++) {
                int assign = missing_assign[i];
                int n_diffs = 0;
                for (int k = 0; k < missing_patterns.width(); k++)
                    if (missing_patterns(i, k) != missing_template(assign, k))
                        n_diffs++;
                stats_diff[n_diffs]++;
            }
            Mat tomat = stats_diff.toMat(stats_diff.length(), 1);
            VMat save_vmat(tomat);
            save_vmat->saveAMAT("/u/delallea/tmp/save_" +
                    tostring(efficient_k_median) + ".amat", true, true);
            stats_diff.resize(missing_template.length());
            stats_diff.fill(0);
            for (int i = 0; i < missing_patterns.length(); i++) {
                stats_diff[missing_assign[i]]++;
            }
            tomat = stats_diff.toMat(stats_diff.length(), 1);
            save_vmat = VMat(tomat);
            save_vmat->saveAMAT("/u/delallea/tmp/clust_" +
                    tostring(efficient_k_median) + ".amat", true, true);
            Mat dist_mat(missing_template.length(),
                         missing_template.length());
            for (int i = 0; i < missing_template.length(); i++) {
                for (int j = 0; j < missing_template.length(); j++) {
                    int n_diffs = 0;
                    for (int k = 0; k < missing_template.width(); k++)
                        if (missing_template(i, k) != missing_template(j, k))
                            n_diffs++;
                    dist_mat(i, j) = n_diffs;
                }
            }
            save_vmat = VMat(dist_mat);
            save_vmat->saveAMAT("/u/delallea/tmp/dist_" +
                    tostring(efficient_k_median) + ".amat", true, true);
                    */
#endif
        }

        // n_tries.resize(0); Old code, may be removed in the future...
        resizeDataBeforeTraining();

        // Get sample weights.
        if (train_set->weightsize() <= 0)
            initial_weights.fill(1);
        else
            getInitialWeightsFrom(train_set);

        // Perform K-means to initialize the centers of the mixture.
        TVec<int> clust_idx;  // Store the cluster index for each sample.
        kmeans(train_set, L, clust_idx, center, kmeans_iterations);

        // Initialize posteriors: P(j | s_i) = 0 if s_i is not in the j-th
        // cluster, and 1 otherwise.
        posteriors.fill(0);
        for (int i = 0; i < nsamples; i++)
            posteriors(i, clust_idx[i]) = 1;

        // Initialize everything from the K-Means clustering result.
        updateSampleWeights();
        computeMixtureWeights(false);
        computeMeansAndCovariances();
        precomputeAllGaussianLogCoefficients();
        /*
        Mat alpha_m(alpha.toMat(1, alpha.length()));
        VMat alpha_vm(alpha_m);
        alpha_vm->saveAMAT("/u/delallea/tmp/alpha.amat", false, true);
        VMat center_vm(center);
        center_vm->saveAMAT("/u/delallea/tmp/center.amat", false, true);
        PLASSERT(eigenvalues.width() == D);
        for (int j = 0; j < L; j++) {
            Vec eigenvals = eigenvalues(j);
            Mat& eigenvecs = eigenvectors[j];
            Mat covar(D, D);
            covar.fill(0);
            for (int k = 0; k < D; k++)
                externalProductScaleAcc(covar, eigenvecs(k), eigenvecs(k),
                        eigenvals[k]);
            VMat covar_vm(covar);
            string filename = "/u/delallea/tmp/covar_" + tostring(j) + ".amat";
            covar_vm->saveAMAT(filename, false, true);
        }
        */
        ptimer->stopTimer("init_time");
    }

    PP<ProgressBar> pb;
    int n_steps = nstages - stage;
    if (report_progress)
        pb = new ProgressBar("Training GaussMix", n_steps);

    /*
    TVec<Mat> save_center;
    save_center.resize(L);
    for (int i = 0; i < save_center.length(); i++)
        save_center[i].resize(n_steps, D);
    */
    int count_step = 0;

    bool replaced_gaussian = false;
    while (stage < nstages) {
        do {
            computePosteriors();
            updateSampleWeights();
            replaced_gaussian = computeMixtureWeights(true);
            // Note: for debugging purpose, 'true' may be replaced by 'false'
            // to ensure no Gaussian is removed.
        } while (replaced_gaussian);
        computeMeansAndCovariances();
        precomputeAllGaussianLogCoefficients();
        // for (int i = 0; i < save_center.length(); i++)
        //    save_center[i](count_step) << center(i);
        count_step++;
        stage++;
        if (report_progress)
            pb->update(n_steps - nstages + stage);
        /*
        if (verbosity >= 10)
            pout << "Highest eigenvalue: " << max(eigenvalues) << endl;
        */
    }

    // Restore original predictor and predicted sizes if necessary.
    if (need_restore_sizes) {
        setPredictorPredictedSizes(backup_predictor_size,
                                   backup_predicted_size);
        // Because the sizes have changed, some data may need to be resized
        // accordingly.
        resizeDataBeforeUsing();
    }

    /*
    for (int i = 0; i < save_center.length(); i++) {
        VMat vm(save_center[i]);
        vm->saveAMAT("save_center_" + tostring(i) + ".amat");
    }
    */
    ptimer->stopTimer("training_time");
    static Vec train_stats_update;
    train_stats_update.resize(2);
    train_stats_update[0] = ptimer->getTimer("init_time");
    train_stats_update[1] = ptimer->getTimer("training_time");
    train_stats->forget(); // Forget potential old total training time.
    train_stats->update(train_stats_update);
}

///////////////////
// traverse_tree //
///////////////////
void GaussMix::traverse_tree(TVec<int>& path,
                             TVec<bool>& span_can_free,
                             TVec<bool>& span_use_previous,
                             bool free_previous,
                             bool use_previous,
                             int index_node, int previous_node,
                             const TVec<int>& parent,
                             const TVec< TVec<int> >& children,
                             const TVec<int>& message_up,
                             const TVec<int>& message_down)
{
    TVec<int> candidates;
    TVec<int> messages;
    TVec<int> child = children[index_node];
    for (int i = 0; i < child.length(); i++)
        if (child[i] != previous_node)
            candidates.append(child[i]);
    for (int i = 0; i < candidates.length(); i++)
        messages.append(message_up[candidates[i]]);
    if (parent[index_node] != index_node &&
        parent[index_node] != previous_node)
    {
        candidates.append(parent[index_node]);
        messages.append(message_down[parent[index_node]]);
    }

    if (child.length() > 1000)
        PLWARNING("In GaussMix::traverse_tree - Should implement a faster "
                  "sorting algorithm");

    path.append(index_node);
    span_can_free.append(free_previous);
    span_use_previous.append(use_previous);

    for (int i = 0; i < candidates.length(); i++) {
        int arg_min = i;
        for (int j = i + 1; j < candidates.length(); j++)
            if (messages[j] < messages[arg_min])
                arg_min = j;
        int tmp = messages[i];
        messages[i] = messages[arg_min];
        messages[arg_min] = tmp;
        tmp = candidates[i];
        candidates[i] = candidates[arg_min];
        candidates[arg_min] = tmp;
        int node = candidates[i];
        PLASSERT( node != index_node && node != previous_node );
        bool can_free = (i == candidates.length() - 1);
        bool can_use_previous = (i == 0);
        traverse_tree(path, span_can_free, span_use_previous, can_free,
                can_use_previous, node, index_node, parent,
                children, message_up, message_down);
    }
}

///////////////////
// unknownOutput //
///////////////////
void GaussMix::unknownOutput(char def, const Vec& input, Vec& output, int& k) const {
    switch(def) {
    case 'p': // Log posteriors P(j | y).
    {
        output.resize(k + L);
        // Compute p(y | x).
        real log_p_y_x = log_density(predicted_part);
        // This also fills the vector 'log_likelihood_dens' with likelihoods p(y,j | x),
        // which is exactly what we need in order to compute the posteriors.
        for (int j = 0; j < L; j++)
            output[j + k] = log_likelihood_dens[j] - log_p_y_x;
        k += L;
        break;
    }
    default:
        inherited::unknownOutput(def, input, output, k);
        break;
    }
}

/////////////////////////
// updateSampleWeights //
/////////////////////////
void GaussMix::updateSampleWeights() {
    for (int j = 0; j < L; j++) {
        updated_weights(j) << initial_weights;
        columnmatrix(updated_weights(j)) *= posteriors.column(j);
    }
}

/////////////////
// survival_fn //
/////////////////
real GaussMix::survival_fn(const Vec& x) const
{
    //PLERROR("survival_fn not implemented for GaussMix"); return 0.0;
    return MISSING_VALUE;
}

/////////
// cdf //
/////////
real GaussMix::cdf(const Vec& x) const
{
    //PLERROR("cdf not implemented for GaussMix"); return 0.0;
    return MISSING_VALUE;
}

//////////////
// variance //
//////////////
void GaussMix::variance(Mat& cov) const
{
    // TODO Variance could be at least implemented for L == 1.
    PLERROR("variance not implemented for GaussMix");
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
