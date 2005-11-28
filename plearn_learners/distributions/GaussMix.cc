// -*- C++ -*-

// GaussMix.cc
// 
// Copyright (C) 2003 Julien Keable
// Copyright (C) 2004-2005 University of Montreal
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
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/math/pl_erf.h>   //!< For gauss_log_density_stddev().
#include <plearn/math/plapack.h>
//#include <plearn/math/random.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/vmat/VMat_basic_stats.h>

namespace PLearn {
using namespace std;

#define TYPE_UNKNOWN    0
#define TYPE_SPHERICAL  1
#define TYPE_DIAGONAL   2
#define TYPE_GENERAL    3

//////////////
// GaussMix //
//////////////
GaussMix::GaussMix():
    type_id(TYPE_UNKNOWN),
    previous_input_part_had_missing(false),
    D(-1),
    n_eigen_computed(-1),
    nsamples(-1),
    alpha_min(1e-6),
    epsilon(1e-6),
    kmeans_iterations(5),
    L(1),
    n_eigen(-1),
    sigma_min(1e-6),
    type("spherical")
{
    // Change the default value of 'nstages' to 10 to make the user aware that
    // in general it should be higher than 1.
    nstages = 10;
}

PLEARN_IMPLEMENT_OBJECT(GaussMix, 
    // TODO Review help.
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
  
    declareOption(ol, "kmeans_iterations", &GaussMix::kmeans_iterations,
                                           OptionBase::buildoption,
        "Maximum number of iterations performed in initial K-means.");

    declareOption(ol, "alpha_min", &GaussMix::alpha_min,
                                   OptionBase::buildoption,
        "The minimum weight for each Gaussian. Whenever a Gaussian falls\n"
        "below 'alpha_min', it is replaced by a new Gaussian.");
  
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
    // We should not have to save this (it is computed in setInput).
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
    /*
    // TODO This code could now be safely removed, I think.
    // Check 'diags' has correct size: it used to be (D x L) instead of (L x D).
    if (diags.length() == D && diags.width() == L) {
        if (L == D)
            PLWARNING("In GaussMix::build_ - The 'diags' option has recently been "
                      "modified to be the transpose of its previous value. Since "
                      "it is square it is not possible to know if it is correct.");
        else
            PLERROR("In GaussMix::build_ - 'diags' has not the right size, you "
                    "are probably trying to load an old GaussMix object created "
                    "before 'diags' was transposed: please transpose it by hand");
    }
    */

    // Check type value.
    if (type == "spherical") {
        type_id = TYPE_SPHERICAL;
    } else if (type == "diagonal") {
        type_id = TYPE_DIAGONAL;
    } else if (type == "general") {
        type_id = TYPE_GENERAL;
    } else
        PLERROR("In GaussMix::build_ - Type '%s' is unknown", type.c_str());

    // Guess values for 'D' and 'n_eigen_computed' if they are not provided
    // (this could be the case for instance when specifying 'by hand' the
    // parameters of the mixture of Gaussians).
    // Make also a few checks to ensure all values are coherent.
    if (stage > 0) {
        assert( D == -1 || D == center.width() );
        if (D == -1)
            D = center.width();
        assert( n_eigen_computed == -1 ||
                n_eigen_computed == eigenvalues.width() );
        if (n_eigen_computed == -1)
            n_eigen_computed = eigenvalues.width();
        assert( n_eigen == -1 || n_eigen_computed <= n_eigen + 1 );
        assert( n_eigen_computed <= D );
    }

    // Make sure everything is correctly resized before using the object.
    resizeDataBeforeUsing();

    // If the learner is ready to be used, we need to precompute the logarithm
    // of the constant coefficient of each Gaussian.
    if (stage > 0)
        precomputeAllGaussianLogCoefficients();

    // Make GaussMix-specific operations for conditional distributions.
    GaussMix::setInputTargetSizes(n_input_, n_target_, false);
    GaussMix::setInput(input_part, false);
}

////////////////////////////////
// computeMeansAndCovariances //
////////////////////////////////
void GaussMix::computeMeansAndCovariances() {
    VMat weighted_train_set;
    Vec sum_columns(L);
    Vec storage_D(D);
    columnSum(posteriors, sum_columns);
    for (int j = 0; j < L; j++) {
        // Build the weighted dataset.
        if (sum_columns[j] < epsilon)
            PLWARNING("In GaussMix::computeMeansAndCovariances - A posterior "
                      "is almost zero");
        VMat weights(columnmatrix(updated_weights(j)));
        weighted_train_set = new ConcatColumnsVMatrix(
            new SubVMatrix(train_set, 0, 0, nsamples, D), weights);
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
            assert( type_id == TYPE_GENERAL );
            computeInputMeanAndCovar(weighted_train_set, center_j, covariance);
            if (center_j.hasMissing()) {
                // There are features missing in all points assigned to this
                // Gaussian. We sample a new random value for these features.
                for (int i = 0; i < D; i++)
                    if (is_missing(center_j[i])) {
                        center_j[i] =
                            random->gaussian_mu_sigma(mean_training  [i],
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
                            assert( is_missing(covariance(k,i)) ||
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
            assert( eigenvals.length() == n_eigen_computed );

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
}

//////////////////////////
// computeLogLikelihood //
//////////////////////////
real GaussMix::computeLogLikelihood(const Vec& y, int j, bool is_input) const
{
    static int size;    // Size of the vector whose density is computed.
    // Index where we start (usually 0 when 'is_input', and 'n_input'
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
    static Vec y_missing;
    static Vec eigenvals_missing;
    static Mat eigenvecs_missing;
    static TVec<int> non_missing;
    static Mat work_mat1, work_mat2;
    static Mat eigenvalues_x_miss;
    static TVec<Mat> eigenvectors_x_miss;
    static Mat full_cov;
    static Mat cov_x_j;
    static Vec y_non_missing;
    static Vec center_non_missing;
    static Mat cov_y_x;

    // Dummy matrix to release some storage pointers so that some matrices can
    // be resized.
    static Mat dummy_mat;

    // Will contain the final result (the desired log-likelihood).
    real log_likelihood;

    if (type_id == TYPE_SPHERICAL || type_id == TYPE_DIAGONAL) {
        // Easy case: the covariance matrix is diagonal.
        if (is_input) {
            size = n_input;
            start  = 0;
        } else {
            size = n_target;
            start = n_input;
        }
        mu_y = center(j).subVec(start, size);
        if (type_id == TYPE_DIAGONAL) {
            assert( diags.length() == L && diags.width() == n_input+n_target );
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
        assert( type_id == TYPE_GENERAL );
        // TODO Put both cases (n_input == 0 and other) in same code (they are
        // very close one to each other).
        if (n_input == 0) {
            // Simple case: there is no input part.
            assert( !is_input );
            assert( y.length() == n_target );

            if (y.hasMissing()) {
                // We need to recompute almost everything.
                // First the full covariance.
                Mat& cov_y = joint_cov[j];
                real var_min = square(sigma_min);
                if (stage_joint_cov_computed[j] != this->stage) {
                    stage_joint_cov_computed[j] = this->stage;
                    cov_y.resize(D, D);
                    eigenvals = eigenvalues(j);
                    real lambda0 = max(var_min, eigenvals.lastElement());
                    cov_y.fill(0);
                    Mat& eigenvectors_j = eigenvectors[j];

                    assert( eigenvectors_j.width() == D );

                    for (int k = 0; k < n_eigen_computed - 1; k++)
                        externalProductScaleAcc(
                                cov_y, eigenvectors_j(k), eigenvectors_j(k),
                                max(var_min, eigenvals[k]) - lambda0);

                    for (int i = 0; i < D; i++)
                        cov_y(i,i) += lambda0;

                    // By construction, the resulting matrix is symmetric. However,
                    // it may happen that it is not exactly the case due to numerical
                    // approximations. Thus we ensure it is perfectly symmetric.
                    assert( cov_y.isSymmetric(false) );
                    fillItSymmetric(cov_y);
                }
                // Then extract what we want.
                non_missing.resize(0);
                for (int k = 0; k < n_target; k++)
                    if (!is_missing(y[k]))
                        non_missing.append(k);
                mu_y = center(j).subVec(0, n_target);
                int n_non_missing = non_missing.length();
                mu_y_missing.resize(n_non_missing);
                y_missing.resize(n_non_missing);
                cov_y_missing.resize(n_non_missing, n_non_missing);
                for (int k = 0; k < n_non_missing; k++) {
                    mu_y_missing[k] = mu_y[non_missing[k]];
                    y_missing[k] = y[non_missing[k]];
                    for (int j = 0; j < n_non_missing; j++) {
                        cov_y_missing(k,j) =
                            cov_y(non_missing[k], non_missing[j]);
                    }
                }
                if (n_non_missing == 0) {
                    log_likelihood = 0;
                } else {
                    // Perform SVD of cov_y_missing.
                    eigenVecOfSymmMat(cov_y_missing, n_non_missing,
                                      eigenvals_missing, eigenvecs_missing);

                    mu_y = mu_y_missing;
                    eigenvals = eigenvals_missing;
                    eigenvecs = eigenvecs_missing;

                    y_centered.resize(n_non_missing);
                    y_centered << y_missing;
                    y_centered -= mu_y;
                    real squared_norm_y_centered = pownorm(y_centered);
                    int n_eig = n_non_missing;

                    real lambda0 = max(var_min, eigenvals.lastElement());
                    assert( lambda0 > 0 );
                    real one_over_lambda0 = 1.0 / lambda0;

                    log_likelihood = precomputeGaussianLogCoefficient(
                                         eigenvals, n_non_missing);
                    // log_likelihood -= 0.5  * 1/lambda_0 * ||y - mu||^2
                    log_likelihood -=
                        0.5 * one_over_lambda0 * squared_norm_y_centered;

                    for (int k = 0; k < n_eig - 1; k++) {
                        // log_likelihood -= 0.5 * (1/lambda_k - 1/lambda_0)
                        //                       * ((y - mu)'.v_k)^2
                        real lambda = max(var_min, eigenvals[k]);
                        assert( lambda > 0 );
                        if (lambda > lambda0)
                            log_likelihood -=
                                0.5 * (1.0 / lambda - one_over_lambda0)
                                    * square(dot(eigenvecs(k), y_centered));
                    }
                    // Release pointer to 'eigenvecs_missing'.
                    eigenvecs = dummy_mat;
                }
            } else {
            log_likelihood = log_coeff[j];

            mu_y = center(j).subVec(0, n_target);
            eigenvals = eigenvalues(j);
            eigenvecs = eigenvectors[j];

            y_centered.resize(n_target);
            y_centered << y;
            y_centered -= mu_y;
            real squared_norm_y_centered = pownorm(y_centered);
            real var_min = square(sigma_min);
            int n_eig = n_eigen_computed;
            real lambda0 = max(var_min, eigenvals[n_eig - 1]);
            assert( lambda0 > 0 );

            real one_over_lambda0 = 1.0 / lambda0;
            // log_likelihood -= 0.5  * 1/lambda_0 * ||y - mu||^2
            log_likelihood -= 0.5 * one_over_lambda0 * squared_norm_y_centered;

            for (int k = 0; k < n_eig - 1; k++) {
                // log_likelihood -= 0.5 * (1/lambda_k - 1/lambda_0)
                //                       * ((y - mu)'.v_k)^2
                real lambda = max(var_min, eigenvals[k]);
                assert( lambda > 0 );
                if (lambda > lambda0)
                    log_likelihood -= 0.5 * (1.0 / lambda - one_over_lambda0)
                                      * square(dot(eigenvecs(k), y_centered));
            }
            }
            // TODO Fix indentation everywhere.
        } else {
            if (y.hasMissing()) {
                // TODO Code duplication is ugly!
            if (is_input) {
            non_missing.resize(0);
            for (int k = 0; k < y.length(); k++)
                if (!is_missing(y[k]))
                    non_missing.append(k);
            int n_non_missing = non_missing.length();
            int n_target_ext = n_target + (n_input - n_non_missing);

            work_mat1.resize(n_target_ext, n_non_missing);
            work_mat2.resize(n_target_ext, n_target_ext);
            Vec eigenvals;
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

                assert( n_input + n_target == D );

                Mat& full_cov_j = full_cov;
                full_cov_j.resize(D, D);
                eigenvals = eigenvalues(j);
                real lambda0 = max(var_min, eigenvals[n_eigen_computed - 1]);

                full_cov_j.fill(0);
                Mat& eigenvectors_j = eigenvectors[j];
                assert( eigenvectors_j.width() == D );

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
                assert( full_cov_j.isSymmetric(false) );
                fillItSymmetric(full_cov_j);

                // Extract the covariance of the input x.
                Mat cov_x_j_miss = full_cov.subMat(0, 0, n_input, n_input);
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
                    cov_y.resize(n_target, n_target);
                    eigenvals = eigenvalues_y_x(j);
                    real lambda0 = max(var_min, eigenvals.lastElement());
                    cov_y.fill(0);
                    Mat& eigenvectors_j = eigenvectors_y_x[j];
                    int n_eig = eigenvectors_y_x.length();

                    assert( eigenvectors_j.width() == n_target );

                    for (int k = 0; k < n_eig - 1; k++)
                        externalProductScaleAcc(
                                cov_y, eigenvectors_j(k), eigenvectors_j(k),
                                max(var_min, eigenvals[k]) - lambda0);

                    for (int i = 0; i < n_target; i++)
                        cov_y(i,i) += lambda0;

                    // By construction, the resulting matrix is symmetric. However,
                    // it may happen that it is not exactly the case due to numerical
                    // approximations. Thus we ensure it is perfectly symmetric.
                    assert( cov_y.isSymmetric(false) );
                    fillItSymmetric(cov_y);
                // Then extract what we want.
                non_missing.resize(0);
                for (int k = 0; k < n_target; k++)
                    if (!is_missing(y[k]))
                        non_missing.append(k);
                mu_y = center_y_x(j);
                int n_non_missing = non_missing.length();
                mu_y_missing.resize(n_non_missing);
                y_missing.resize(n_non_missing);
                cov_y_missing.resize(n_non_missing, n_non_missing);
                for (int k = 0; k < n_non_missing; k++) {
                    mu_y_missing[k] = mu_y[non_missing[k]];
                    y_missing[k] = y[non_missing[k]];
                    for (int j = 0; j < n_non_missing; j++) {
                        cov_y_missing(k,j) =
                            cov_y(non_missing[k], non_missing[j]);
                    }
                }
                if (n_non_missing == 0) {
                    log_likelihood = 0;
                } else {
                    // Perform SVD of cov_y_missing.
                    eigenVecOfSymmMat(cov_y_missing, n_non_missing,
                                      eigenvals_missing, eigenvecs_missing);

                    mu_y = mu_y_missing;
                    eigenvals = eigenvals_missing;
                    eigenvecs = eigenvecs_missing;

                    y_centered.resize(n_non_missing);
                    y_centered << y_missing;
                    y_centered -= mu_y;
                    real squared_norm_y_centered = pownorm(y_centered);
                    int n_eig = n_non_missing;

                    real lambda0 = max(var_min, eigenvals.lastElement());
                    assert( lambda0 > 0 );
                    real one_over_lambda0 = 1.0 / lambda0;

                    log_likelihood = precomputeGaussianLogCoefficient(
                                         eigenvals, n_non_missing);
                    // log_likelihood -= 0.5  * 1/lambda_0 * ||y - mu||^2
                    log_likelihood -=
                        0.5 * one_over_lambda0 * squared_norm_y_centered;

                    for (int k = 0; k < n_eig - 1; k++) {
                        // log_likelihood -= 0.5 * (1/lambda_k - 1/lambda_0)
                        //                       * ((y - mu)'.v_k)^2
                        real lambda = max(var_min, eigenvals[k]);
                        assert( lambda > 0 );
                        if (lambda > lambda0)
                            log_likelihood -=
                                0.5 * (1.0 / lambda - one_over_lambda0)
                                    * square(dot(eigenvecs(k), y_centered));
                    }
                    // Allow future resize of 'eigenvecs_missing'.
                    eigenvecs = dummy_mat;
                }

                return log_likelihood;
            }

            if (y_centered.length() > 0) {
            y_centered -= mu;

            real squared_norm_y_centered = pownorm(y_centered);
            real var_min = square(sigma_min);
            int n_eig = eigenvals.length();

            real lambda0 = max(var_min, eigenvals.lastElement());
            assert( lambda0 > 0 );

            real one_over_lambda0 = 1.0 / lambda0;
            // log_likelihood -= 0.5  * 1/lambda_0 * ||y - mu||^2
            log_likelihood -= 0.5 * one_over_lambda0 * squared_norm_y_centered;

            for (int k = 0; k < n_eig - 1; k++) {
                // log_likelihood -= 0.5 * (1/lambda_k - 1/lambda_0)
                //                       * ((y - mu)'.v_k)^2
                real lambda = max(var_min, eigenvals[k]);
                assert( lambda > 0 );
                assert( lambda >= lambda0 );
                if (lambda > lambda0)
                    log_likelihood -= 0.5 * (1.0 / lambda - one_over_lambda0)
                                      * square(dot(eigenvecs(k), y_centered));
            }
            }
            } else {

            if (is_input) {
                log_likelihood = log_coeff_x[j];
                mu = center(j).subVec(0, n_input);
                eigenvals = eigenvalues_x(j);
                eigenvecs = eigenvectors_x[j];
                y_centered.resize(n_input);
            } else {
                log_likelihood = log_coeff_y_x[j];
                mu = center_y_x(j);
                eigenvals = eigenvalues_y_x(j);
                eigenvecs = eigenvectors_y_x[j];
                y_centered.resize(n_target);
            }

            y_centered << y;
            y_centered -= mu;

            real squared_norm_y_centered = pownorm(y_centered);
            real var_min = square(sigma_min);
            int n_eig = eigenvals.length();

            real lambda0 = max(var_min, eigenvals[n_eig - 1]);
            assert( lambda0 > 0 );

            real one_over_lambda0 = 1.0 / lambda0;
            // log_likelihood -= 0.5  * 1/lambda_0 * ||y - mu||^2
            log_likelihood -= 0.5 * one_over_lambda0 * squared_norm_y_centered;

            for (int k = 0; k < n_eig - 1; k++) {
                // log_likelihood -= 0.5 * (1/lambda_k - 1/lambda_0)
                //                       * ((y - mu)'.v_k)^2
                real lambda = max(var_min, eigenvals[k]);
                assert( lambda > 0 );
                assert( lambda >= lambda0 );
                if (lambda > lambda0)
                    log_likelihood -= 0.5 * (1.0 / lambda - one_over_lambda0)
                                      * square(dot(eigenvecs(k), y_centered));
            }
            }
        }
    }
    assert( !isnan(log_likelihood) );
    return log_likelihood;
}

//////////////////////////////
// computeAllLogLikelihoods //
//////////////////////////////
void GaussMix::computeAllLogLikelihoods(const Vec& sample, const Vec& log_like)
{
    assert( sample.length()   == D );
    assert( log_like.length() == L );
    for (int j = 0; j < L; j++)
        log_like[j] = computeLogLikelihood(sample, j);
}

///////////////////////
// computePosteriors //
///////////////////////
void GaussMix::computePosteriors() {
    sample_row.resize(D);
    log_likelihood_post.resize(L);
    for (int i = 0; i < nsamples; i++) {
        train_set->getSubRow(i, 0, sample_row);
        // First we need to compute the likelihood P(s_i | j).
        computeAllLogLikelihoods(sample_row, log_likelihood_post);
        assert( !log_likelihood_post.hasMissing() );
        for (int j = 0; j < L; j++)
            log_likelihood_post[j] += pl_log(alpha[j]);
        real log_sum_likelihood = logadd(log_likelihood_post);
        for (int j = 0; j < L; j++)
            // Compute the posterior P(j | s_i) = P(s_i | j) * alpha_i / (sum_i ")
            posteriors(i, j) = exp(log_likelihood_post[j] - log_sum_likelihood);
    }
}

///////////////////////////
// computeMixtureWeights //
///////////////////////////
bool GaussMix::computeMixtureWeights() {
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
            if (alpha[j] < alpha_min) {
                // alpha[j] is too small! We need to remove this Gaussian from
                // the mixture, and find a new (better) one.
                replaceGaussian(j);
                replaced_gaussian = true;
            }
    }
    return replaced_gaussian;
}

/////////////////
// expectation //
/////////////////
void GaussMix::expectation(Vec& mu) const
{
    mu.resize(n_target);
    if (type_id == TYPE_SPHERICAL || type_id == TYPE_DIAGONAL ||
       (type_id == TYPE_GENERAL && n_input == 0)) {
        // The expectation is the same in the 'spherical' and 'diagonal' cases.
        mu.fill(0);
        real* coeff = n_input == 0 ? alpha.data() : p_j_x.data();
        for (int j = 0; j < L; j++)
            mu += center(j).subVec(n_input, n_target) * coeff[j];
    } else {
        assert( type_id == TYPE_GENERAL );
        // The case 'n_input == 0' is considered above.
        assert( n_input > 0 );
        mu.fill(0);
        for (int j = 0; j < L; j++)
            mu += center_y_x(j) * p_j_x[j];
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
    // TODO Why not having p_j_x point to alpha when n_input == 0 ? This may
    // make the code cleaner (but check what happens with serialization...).
    int j;    // The index of the Gaussian to use.

    // The assert below may fail if one forgets to provide an input part
    // through the 'setInput' method.
    assert( n_input == 0 || p_j_x.length() == L );

    if (given_gaussian < 0)
        j = random->multinomial_sample(n_input == 0 ? alpha : p_j_x);
    else
        j = given_gaussian % alpha.length();

    sample.resize(n_target);

    if (type_id == TYPE_SPHERICAL || type_id == TYPE_DIAGONAL) {
        Vec mu_y = center(j).subVec(n_input, n_target);
        for (int k = 0; k < n_target; k++) {
            real stddev = type_id == TYPE_SPHERICAL ? sigma[j]
                                                    : diags(j, k + n_input);
            stddev = max(sigma_min, stddev);
            sample[k] = random->gaussian_mu_sigma(mu_y[k], stddev);
        }
    } else {
        assert( type_id == TYPE_GENERAL );
        static Vec norm_vec;
        if (n_input == 0) {
            // Simple case.
            assert( eigenvectors[j].width() == n_target );
            assert( center(j).length() == n_target );

            Vec eigenvals = eigenvalues(j);
            Mat eigenvecs = eigenvectors[j].subMat(0, 0, n_eigen_computed,
                                                         n_target);
            int n_eig = n_eigen_computed;
            Vec mu_y = center(j);

            norm_vec.resize(n_eig - 1);
            random->fill_random_normal(norm_vec);
            real var_min = square(sigma_min);
            real lambda0 = max(var_min, eigenvals[n_eig - 1]);
            sample.fill(0);
            for (int k = 0; k < n_eig - 1; k++)
                // TODO See if can use more optimized function.
                sample += sqrt(max(var_min, eigenvals[k]) - lambda0)
                          * norm_vec[k] * eigenvecs(k);
            norm_vec.resize(n_target);
            random->fill_random_normal(norm_vec);
            sample += norm_vec * sqrt(lambda0);
            sample += mu_y;
        } else {
            // TODO Get rid of code duplication with above.

            Vec eigenvals = eigenvalues_y_x(j);
            Mat eigenvecs = eigenvectors_y_x[j];

            int n_eig = n_target;
            Vec mu_y = center_y_x(j);

            norm_vec.resize(n_eig - 1);
            random->fill_random_normal(norm_vec);
            real var_min = square(sigma_min);
            real lambda0 = max(var_min, eigenvals[n_eig - 1]);
            sample.fill(0);
            for (int k = 0; k < n_eig - 1; k++)
                // TODO See if can use more optimized function.
                sample += sqrt(max(var_min, eigenvals[k]) - lambda0)
                          * norm_vec[k] * eigenvecs(k);
            norm_vec.resize(n_target);
            random->fill_random_normal(norm_vec);
            sample += norm_vec * sqrt(lambda0);
            sample += mu_y;
        }
    }
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
    int farthest_sample = random->uniform_multinomial_sample(vmat_length);
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
                cl_center[k] = random->gaussian_mu_sigma(mean_training[k],
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

    ProgressBar* pb = 0;
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
                    && clust_stat[i].getStats(j).nnonmissing() == 0;
                 j++) {}
            if (j < input.length())
                // There have been some samples assigned to this cluster.
                clust_stat[i].getMean(clust_i);
            else {
                // Re-initialize randomly the cluster center.
                int new_center = random->uniform_multinomial_sample(vmat_length);
                samples->getExample(new_center, input, target, weight);
                clust_i << input;
            }
            // Replace missing values by randomly generated values.
            for (int k = 0; k < clust_i.length(); k++)
                if (is_missing(clust_i[k]))
                    clust_i[k] = random->gaussian_mu_sigma(mean_training  [k],
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
    if (pb)
        delete pb;
    if (report_progress && verbosity >= 2 && iteration > 0)
        pout << "K-Means performed in only " << maxit - iteration << " iterations."
             << endl;
}

/////////////////
// log_density //
/////////////////
real GaussMix::log_density(const Vec& y) const
{ 
    static Vec log_likelihood_dens;
    log_likelihood_dens.resize(L);
    // First we need to compute the likelihood
    //   p(y,j | x) = p(y | x,j) * p(j | x).
    for (int j = 0; j < L; j++) {
        real logp_j_x = n_input == 0 ? pl_log(alpha[j])
                                     : log_p_j_x[j];
        log_likelihood_dens[j] = computeLogLikelihood(y, j) + logp_j_x;
        assert( !isnan(log_likelihood_dens[j]) );
    }
    return logadd(log_likelihood_dens);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GaussMix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    // TODO Implement.
    /*
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(sample_row, copies);
    deepCopyField(log_likelihood_post, copies);
    deepCopyField(x_minus_mu_x, copies);
    deepCopyField(mu_target, copies);
    deepCopyField(log_likelihood_dens, copies);
    deepCopyField(eigenvalues,copies);
    deepCopyField(eigenvectors,copies);
    deepCopyField(diags,copies);
    deepCopyField(log_coeff,copies);
    deepCopyField(log_p_j_x,copies);
    deepCopyField(log_p_x_j_alphaj,copies);
    deepCopyField(mu_y_x,copies);
    deepCopyField(n_tries, copies);
    deepCopyField(p_j_x,copies);

    deepCopyField(cov_x,copies);
    deepCopyField(cov_y_x,copies);
    deepCopyField(eigenvalues_x,copies);
    deepCopyField(eigenvalues_y_x,copies);
    deepCopyField(eigenvectors_x,copies);
    deepCopyField(eigenvectors_y_x,copies);
    deepCopyField(full_cov,copies);
    deepCopyField(y_x_mat,copies);

    deepCopyField(mean_training,copies);
    deepCopyField(stddev_training,copies);
    deepCopyField(posteriors,copies);
    deepCopyField(initial_weights,copies);
    deepCopyField(updated_weights,copies);
    deepCopyField(alpha,copies);
    deepCopyField(mu,copies);
    deepCopyField(sigma,copies);

    deepCopyField(covariance,copies);
    */
    PLERROR("GaussMix::makeDeepCopyFromShallowCopy not implemented");
}

////////////////
// outputsize //
////////////////
    /*
int GaussMix::outputsize() const {
    int os = inherited::outputsize();
    for (size_t i = 0; i < outputs_def.length(); i++)
        if (outputs_def[i] == 'p')
            // We add L-1 because in inherited::outpusize() this was already
            // counted as 1.
            os += L - 1;
    return os;
}
    */

//////////////////////////////////////////
// precomputeAllGaussianLogCoefficients //
//////////////////////////////////////////
void GaussMix::precomputeAllGaussianLogCoefficients()
{
    if (type_id == TYPE_SPHERICAL || type_id == TYPE_DIAGONAL) {
        // Nothing to do.
    } else {
        assert( type_id == TYPE_GENERAL );
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
    real last_eigenval = INFINITY;
#endif
    int n_eig = eigenvals.length();
    assert( dimension >= n_eig );
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
    // Find the Gaussian with highest weight.
    int high = argmax(alpha);
    // Generate the new center from this Gaussian.
    Vec new_center = center(j);
    generateFromGaussian(new_center, high);
    // Copy the covariance.
    if (type_id == TYPE_SPHERICAL) {
        sigma[j] = sigma[high];
    } else if (type_id == TYPE_DIAGONAL) {
        diags(j) << diags(high);
    } else { 
        assert( type_id == TYPE_GENERAL );
        eigenvalues(j) << eigenvalues(high);
        eigenvectors[j] << eigenvectors[high];
        log_coeff[j] = log_coeff[high];
        // TODO Check nothing more needs being done. In particular, what
        // happens with conditional distributions / missing values?
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
    log_coeff.resize(0);
    log_coeff_x.resize(0);
    log_coeff_y_x.resize(0);
    stage_joint_cov_computed.resize(0);
    y_x_mat.resize(0);


    center_y_x.resize(0, 0);
    eigenvalues_x.resize(0, 0);
    eigenvalues_y_x.resize(0, 0);

    // Type-specific data.
    if (type_id == TYPE_SPHERICAL) {
    } else if (type_id == TYPE_DIAGONAL) {
    } else {
        assert( type_id == TYPE_GENERAL );

        eigenvectors_x.resize(L);
        eigenvectors_y_x.resize(L);
        joint_cov.resize(L);
        log_coeff_x.resize(L);
        log_coeff_y_x.resize(L);
        stage_joint_cov_computed.resize(L);
        stage_joint_cov_computed.fill(-1);
        y_x_mat.resize(L);

        if (n_input >= 0)
            eigenvalues_x.resize(L, n_input);
        if (n_target >= 0) {
            center_y_x.resize(L, n_target);
            eigenvalues_y_x.resize(L, n_target);
        }
        log_coeff.resize(L);
    }
}

//////////////////////////////
// resizeDataBeforeTraining //
//////////////////////////////
void GaussMix::resizeDataBeforeTraining() {
    assert( train_set );

    n_eigen_computed = -1;

    nsamples = train_set->length();
    D = train_set->inputsize();

    alpha.resize(L);
    eigenvectors.resize(0);
    mean_training.resize(0);
    sigma.resize(0);
    stddev_training.resize(0);

    center.resize(L, D);
    covariance.resize(0, 0);
    diags.resize(0, 0);
    eigenvalues.resize(0, 0);
    initial_weights.resize(nsamples);
    posteriors.resize(nsamples, L);
    updated_weights.resize(L, nsamples);

    // Type-specific data.
    if (type_id == TYPE_SPHERICAL) {
        sigma.resize(L);
    } else if (type_id == TYPE_DIAGONAL) {
        diags.resize(L, D);
    } else {
        assert( type_id == TYPE_GENERAL );

        eigenvectors.resize(L);

        if (n_eigen == -1 || n_eigen == D)
            // We need to compute all eigenvectors.
            n_eigen_computed = D;
        else
            n_eigen_computed = n_eigen + 1;
        eigenvalues.resize(L, n_eigen_computed);
        for (int i = 0; i < eigenvectors.length(); i++)
            eigenvectors[i].resize(n_eigen_computed, D);
    }
}

//////////////
// setInput //
//////////////
void GaussMix::setInput(const Vec& input, bool call_parent) const {
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
        inherited::setInput(input);

    if (n_input == 0) {
        // There is no input part anyway: nothing to do.
        assert( input_part.isEmpty() );
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
        if (!input_part.hasMissing()) {
            // Simple case: the input part has no missing value, and we can
            // re-use the quantities computed in setInputTargetSizes(..).

            // If the previous input part set had missing values, we will need
            // to recompute some important variables (e.g. eigenvectors /
            // values of y|x). This can be done by re-setting the sizes.
            // TODO This is a bit hackish... we may want to actually store the
            // appropriate data elsewhere so that there is no need to recompute
            // it again.
            if (previous_input_part_had_missing)
                setInputTargetSizes_const(n_input, n_target);

            previous_input_part_had_missing = false;
            x_minus_mu_x.resize(n_input);
            Vec mu_target;
            for (int j = 0; j < L; j++) {
                x_minus_mu_x << input_part;
                x_minus_mu_x -= center(j).subVec(0, n_input);
                mu_target = center_y_x(j);
                if (n_input > 0)
                    product(mu_target, y_x_mat[j], x_minus_mu_x);
                else
                    mu_target.fill(0);
                mu_target += center(j).subVec(n_input, n_target);
            }
        } else {
            previous_input_part_had_missing = true;
            // TODO Code duplication is ugly!
            non_missing.resize(0);
            missing.resize(0);
            for (int k = 0; k < input_part.length(); k++)
                if (!is_missing(input_part[k]))
                    non_missing.append(k);
                else
                    missing.append(k);
            int n_non_missing = non_missing.length();
            int n_missing = missing.length();
            int n_target_ext = n_target + n_missing;
            assert( n_missing + n_non_missing == n_input );

            work_mat1.resize(n_target_ext, n_non_missing);
            work_mat2.resize(n_target_ext, n_target_ext);
            Vec eigenvals;
            real var_min = square(sigma_min);
            eigenvalues_x_miss.resize(L, n_non_missing);
            eigenvectors_x_miss.resize(L);
            for (int j = 0; j < L; j++) {
                // First we compute the joint covariance matrix from the
                // eigenvectors and eigenvalues:
                // full_cov = sum_k (lambda_k - lambda0) v_k v_k' + lambda0.I
                // TODO Do we really need to compute the full matrix?

                assert( n_input + n_target == D );

                Mat& full_cov_j = full_cov;
                full_cov_j.resize(D, D);
                eigenvals = eigenvalues(j);
                real lambda0 = max(var_min, eigenvals[n_eigen_computed - 1]);

                full_cov_j.fill(0);
                Mat& eigenvectors_j = eigenvectors[j];
                assert( eigenvectors_j.width() == D );

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
                assert( full_cov_j.isSymmetric(false) );
                fillItSymmetric(full_cov_j);

                // Extract the covariance of the input x.
                Mat cov_x_j_miss = full_cov.subMat(0, 0, n_input, n_input);
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
                    assert( eigenvals.lastElement() >= var_min );
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

                // Compute the covariance of y|x.
                // It is only needed when there is an input part, since otherwise
                // we can simply use the full covariance.
                // TODO See if we can use simpler formulas.
                Mat& cov_y_x_j = cov_y_x; // TODO Can we get rid of cov_y_x_j?
                cov_y_x_j.resize(n_target_ext, n_target_ext);
                cov_y_x_j.subMat(0, 0, n_target, n_target) <<
                    full_cov_j.subMat(n_input, n_input, n_target, n_target);
                for (int k = 0; k < n_missing; k++) {
                    int x_missing = missing[k];
                    for (int p = 0; p < n_target_ext; p++) {
                        if (p < n_target)
                            cov_y_x_j(n_target + k, p) =
                                cov_y_x_j(p, n_target + k) =
                                full_cov_j(x_missing, p);
                        else
                            cov_y_x_j(n_target + k, p) =
                                cov_y_x_j(p, n_target + k) =
                                full_cov_j(x_missing, missing[p - n_target]);
                    }
                }
                    
                y_x_mat_miss.resize(L);
                y_x_mat_miss[j].resize(n_target, n_non_missing);
                if (n_non_missing > 0) {
                    cross_cov =
                        full_cov_j.subMat(n_non_missing, 0,
                                          n_target_ext, n_non_missing);
                    product(work_mat1, cross_cov, inv_cov_x);
                    productTranspose(work_mat2, work_mat1, cross_cov);
                    cov_y_x_j -= work_mat2;
                    y_x_mat_miss[j] << work_mat1.subMat(0, 0,
                                                   n_target, n_non_missing);
                }
                // Compute SVD of the covariance of y|x.
                eigenvectors_y_x[j].resize(n_target, n_target);
                eigenvals = eigenvalues_y_x(j);
                // Extract the covariance of the target part we are really
                // interested in.
                cov_y_x = cov_y_x_j.subMat(0, 0, n_target, n_target);
                // Ensure covariance matrix is perfectly symmetric.
                assert( cov_y_x.isSymmetric(false, true) );
                fillItSymmetric(cov_y_x);
                eigenVecOfSymmMat(cov_y_x, n_target, eigenvals, eigenvectors_y_x[j]);
                log_coeff_y_x[j] =
                    precomputeGaussianLogCoefficient(eigenvals, n_target);
            }

            x_minus_mu_x.resize(n_non_missing);
            Vec mu_target;
            for (int j = 0; j < L; j++) {
                for (int k = 0; k < n_non_missing; k++)
                    x_minus_mu_x[k] =
                        input_part[non_missing[k]] - center(j, non_missing[k]);
                mu_target = center_y_x(j);
                if (n_non_missing > 0)
                    product(mu_target, y_x_mat_miss[j], x_minus_mu_x);
                else
                    mu_target.fill(0);
                mu_target += center(j).subVec(n_input, n_target);
            }

        }
    }
 
    log_p_x_j_alphaj.resize(L);
    for (int j = 0; j < L; j++)
        log_p_x_j_alphaj[j] = computeLogLikelihood(input_part, j, true)
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
    assert( vmat->weightsize() == 1 );
    Vec tmp1, tmp2;
    real w;
    assert( vmat );
    ProgressBar* pb = 0;
    if (report_progress)
        pb = new ProgressBar("Getting sample weights from data set",
                             vmat->length());
    for (int i = 0; i < vmat->length(); i++) {
        vmat->getExample(i, tmp1, tmp2, w);
        initial_weights[i] = w;
        if (report_progress)
            pb->update(i + 1);
    }
    if (pb)
        delete pb;
}

/////////////////////////
// setInputTargetSizes //
/////////////////////////
bool GaussMix::setInputTargetSizes(int n_i, int n_t,
                                   bool call_parent)
{
    bool sizes_changed = true;
    if (call_parent)
        sizes_changed =
            inherited::setInputTargetSizes(n_i, n_t, call_parent);
    setInputTargetSizes_const(n_i, n_t);
    return sizes_changed;
}

///////////////////////////////
// setInputTargetSizes_const //
///////////////////////////////
void GaussMix::setInputTargetSizes_const(int n_i, int n_t) const
{
    static Mat inv_cov_x;
    static Mat full_cov;
    static Mat cov_y_x;
    static Mat work_mat1, work_mat2;
    static Mat cross_cov;

    if (n_input == -1 || n_target == -1 || D == -1)
        // Sizes are not defined yet, there is nothing we can do.
        return;

    if (type_id == TYPE_SPHERICAL || type_id == TYPE_DIAGONAL ) {
        // Nothing to do.
    } else {
        assert( type_id == TYPE_GENERAL );

        work_mat1.resize(n_target, n_input);
        work_mat2.resize(n_target, n_target);
        Vec eigenvals;
        real var_min = square(sigma_min);
        for (int j = 0; j < L; j++) {
            // Compute the covariance of x and y|x for the j-th Gaussian (we
            // will need them to compute the likelihood).

            // First we compute the joint covariance matrix from the
            // eigenvectors and eigenvalues:
            // full_cov = sum_k (lambda_k - lambda0) v_k v_k' + lambda0.I

            assert( n_input + n_target == D );
            Mat& full_cov_j = full_cov;
            full_cov_j.resize(D, D);
            eigenvals = eigenvalues(j);
            real lambda0 = max(var_min, eigenvals[n_eigen_computed - 1]);

            full_cov_j.fill(0);
            Mat& eigenvectors_j = eigenvectors[j];
            assert( eigenvectors_j.width() == D );

            for (int k = 0; k < n_eigen_computed - 1; k++)
                externalProductScaleAcc(full_cov_j, eigenvectors_j(k),
                                        eigenvectors_j(k),
                                        max(var_min, eigenvals[k]) - lambda0);
            for (int i = 0; i < D; i++)
                full_cov_j(i,i) += lambda0;

            // By construction, the resulting matrix is symmetric. However,
            // it may happen that it is not exactly the case due to numerical
            // approximations. Thus we ensure it is perfectly symmetric.
            assert( full_cov_j.isSymmetric(false) );
            fillItSymmetric(full_cov_j);

            // Extract the covariance of the input x.
            Mat cov_x_j = full_cov_j.subMat(0, 0, n_input, n_input);

            // Compute its SVD.
            eigenvectors_x[j].resize(n_input, n_input);
            eigenvals = eigenvalues_x(j);
            eigenVecOfSymmMat(cov_x_j, n_input, eigenvals, eigenvectors_x[j]);
            // Note that the computation above will have destroyed 'cov_x_j',
            // i.e. a part of the full covariance matrix.
            log_coeff_x[j] =
                precomputeGaussianLogCoefficient(eigenvals, n_input);


            // And its inverse (we'll need it for the covariance of y|x).
            inv_cov_x.resize(n_input, n_input);
            inv_cov_x.fill(0);
            if (n_input > 0) {
                // I am not sure about this assert, but since we extract the
                // covariance of x from a matrix whose eigenvalues are all more
                // than 'var_min', it looks like the eigenvalues of the
                // covariance of x should also be more than 'var_min'. If I am
                // wrong, remove the assert and see if it is needed to
                // potentially set lambda0 to var_min.
                assert( eigenvals[n_input - 1] >= var_min );
                lambda0 = eigenvals[n_input - 1];
                real one_over_lambda0 = 1 / lambda0;
                Mat& eigenvectors_x_j = eigenvectors_x[j];
                for (int k = 0; k < n_input - 1; k++)
                    externalProductScaleAcc(
                        inv_cov_x, eigenvectors_x_j(k), eigenvectors_x_j(k),
                        1 / max(var_min, eigenvals[k]) - one_over_lambda0);
                for (int i = 0; i < n_input; i++)
                    inv_cov_x(i,i) += one_over_lambda0;
            }

            // Compute the covariance of y|x.
            // It is only needed when there is an input part, since otherwise
            // we can simply use the full covariance.
            // TODO See if we can use simpler formulas.
            Mat& cov_y_x_j = cov_y_x; // TODO Can we get rid of cov_y_x_j?
            cov_y_x_j.resize(n_target, n_target);
            cov_y_x_j <<
                full_cov_j.subMat(n_input, n_input, n_target, n_target);
            y_x_mat[j].resize(n_target, n_input);
            if (n_input > 0) {
                cross_cov = full_cov_j.subMat(n_input,0, n_target, n_input);
                product(work_mat1, cross_cov, inv_cov_x);
                productTranspose(work_mat2, work_mat1, cross_cov);
                cov_y_x_j -= work_mat2;
                y_x_mat[j] << work_mat1;
            }
            // Compute SVD of the covariance of y|x.
            eigenvectors_y_x[j].resize(n_target, n_target);
            eigenvals = eigenvalues_y_x(j);
            // Ensure covariance matrix is perfectly symmetric.
            assert( cov_y_x_j.isSymmetric(false, true) );
            fillItSymmetric(cov_y_x_j);
            eigenVecOfSymmMat(cov_y_x_j, n_target, eigenvals, eigenvectors_y_x[j]);
            log_coeff_y_x[j] =
                precomputeGaussianLogCoefficient(eigenvals, n_target);
        }
    }
}


///////////
// train //
///////////
void GaussMix::train()
{
    // Standard PLearner checks.
    if (!initTrain())
        return;

    // When training, we want to learn the full joint distribution.
    bool need_restore_sizes = setInputTargetSizes(0, -1);

    // Initialization before training.
    if (stage == 0) {
        // n_tries.resize(0); TODO See
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
        computeMixtureWeights();
        computeMeansAndCovariances();
        precomputeAllGaussianLogCoefficients();
    }

    ProgressBar* pb = 0;
    int n_steps = nstages - stage;
    if (report_progress)
        pb = new ProgressBar("Training GaussMix", n_steps);

    TVec<Mat> save_center;
    // save_center.resize(L);
    for (int i = 0; i < save_center.length(); i++)
        save_center[i].resize(n_steps, D);
    int count_step = 0;

    bool replaced_gaussian = false;
    while (stage < nstages) {
        do {
            computePosteriors();
            updateSampleWeights();
            replaced_gaussian = computeMixtureWeights();
        } while (replaced_gaussian);
        computeMeansAndCovariances();
        precomputeAllGaussianLogCoefficients();
        for (int i = 0; i < save_center.length(); i++)
            save_center[i](count_step) << center(i);
        count_step++;
        stage++;
        if (report_progress)
            pb->update(n_steps - nstages + stage);
    }
    if (pb)
        delete pb;

    // Restore original input and target sizes if necessary.
    if (need_restore_sizes)
        setInputTargetSizes(n_input_, n_target_);

    for (int i = 0; i < save_center.length(); i++) {
        VMat vm(save_center[i]);
        vm->saveAMAT("save_center_" + tostring(i) + ".amat");
    }
}

///////////////////
// unknownOutput //
///////////////////
    /*
void GaussMix::unknownOutput(char def, const Vec& input, Vec& output, int& k) const {
    switch(def) {
    case 'p': // Log posteriors P(j | y).
    {
        output.resize(k + L);
        // Compute p(y | x).
        real log_p_y_x = log_density(target_part);
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
    */

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
