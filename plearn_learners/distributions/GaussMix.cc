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
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include "GaussMix.h"
#include <plearn/math/pl_erf.h>   //!< For gauss_log_density_stddev().
#include <plearn/math/plapack.h>
#include <plearn/math/random.h>
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
    PDistribution(),
    D(-1),
    n_eigen_computed(-1),
    nsamples(-1),
    type_id(TYPE_UNKNOWN),
    alpha_min(1e-6),
    epsilon(1e-6),
    kmeans_iterations(5),
    L(1),
    n_eigen(-1),
    sigma_min(1e-6),
    type("spherical")
    // TODO Re-do correct initialization.
    /*
    conditional_updating_time(0),
    nsamples(0),
    training_time(0),
    */
{
    // nstages = 10;
}

PLEARN_IMPLEMENT_OBJECT(GaussMix, 
        // TODO Review help.
        // TODO Reformat help.
                        "Gaussian mixture, either set non-parametrically or trained by EM.", 
                        "GaussMix implements a mixture of L Gaussians.\n"
                        "There are 3 possible parametrization types:\n"
                        " - spherical : Gaussians have covar matrix = diag(sigma). Parameter used : sigma.\n"
                        " - diagonal  : Gaussians have covar matrix = diag(sigma_i). Parameters used : diags.\n"
                        " - general   : Gaussians have an unconstrained covariance matrix.\n"
                        "               The user specifies the number 'n_eigen' of eigenvectors kept when\n"
                        "               decomposing the covariance matrix. The remaining eigenvectors are\n"
                        "               considered as having a fixed eigenvalue equal to the next highest\n"
                        "               eigenvalue in the decomposition.\n"
                        /* To implement some day.
                           " - factor    : (not implemented!) as in the general case, the Gaussians are defined\n"
                           "               with K<=D vectors (through KxD matrix 'V'), but these need not be\n"
                           "               orthogonal/orthonormal.\n"
                           "               The covariance matrix used will be V(t)V + psi with psi a D-vector\n"
                           "               (through parameter diags).\n"
                        */
                        "\n"
                        "Two parameters are common to all 3 types :\n"
                        " - alpha : the ponderation factor of the Gaussians\n"
                        " - mu    : their centers\n"
                        "\n"
                        "In addition to the usual costs inherited from PDistribution, an additional output\n"
                        "can be computed by using the character 'p' in the 'outputs_def' option: this will\n"
                        "return an output containing the posterior log-probabilities P(j|Y,X) of each Gaussian.\n"
                        "\n"
                        "If specified to a positive value, the seed will be set to the given value.\n"
                        "If set to -1, the seed will be ignored."
                        // TODO More coherent behavior would be to do nothing
                        // when seed == 0 (and -1 would mean time seed).
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
        "One of: 'spherical', 'diagonal', 'general'.\n"
        "This is the type of covariance matrix for each Gaussian.\n"
        "   - spherical : spherical covariance matrix sigma * I\n"
        "   - diagonal  : diagonal covariance matrix\n"
        "   - general   : unconstrained covariance matrix (defined by its\n"
        "                 eigenvectors)\n");
    // To implement some day:
    //  "   - factor    : represented by Ks[i] principal components\n");
    // TODO Get rid?

    declareOption(ol, "n_eigen", &GaussMix::n_eigen, OptionBase::buildoption,
        "If type == 'general', the number of eigenvectors used to compute\n"
        "the covariance matrix. The remaining eigenvectors will be given an\n"
        "eigenvalue equal to the next highest eigenvalue. If set to -1, all\n"
        "eigenvectors will be kept.");
  
    declareOption(ol, "kmeans_iterations", &GaussMix::kmeans_iterations,
                                           OptionBase::buildoption,
        "Maximum number of iterations performed in initial K-means.");

    declareOption(ol, "alpha_min", &GaussMix::alpha_min,
                                   OptionBase::buildoption,
        "The minimum weight for each Gaussian.");
  
    declareOption(ol,"sigma_min", &GaussMix::sigma_min,
                                  OptionBase::buildoption,
        "The minimum standard deviation allowed.");
    // TODO Better help.
  
    declareOption(ol, "epsilon", &GaussMix::epsilon, OptionBase::buildoption,
        "A small number to check for near-zero probabilities.");

  
    // Learnt options.

    declareOption(ol, "alpha", &GaussMix::alpha, OptionBase::learntoption,
        "Coefficients of the Gaussians. They sum to 1 and are positive:\n"
        "they can be interpreted as priors P(Gaussian j).");

    declareOption(ol, "sigma", &GaussMix::sigma, OptionBase::learntoption,
        "The standard deviation in all directions, for 'spherical' type.\n");

    declareOption(ol, "eigenvalues", &GaussMix::eigenvalues,
                                     OptionBase::learntoption,
        "The eigenvalues associated with the principal eigenvectors:\n"
        "element (j,k) is the k-th eigenvalue of the j-th Gaussian.");

    declareOption(ol, "eigenvectors", &GaussMix::eigenvectors,
                                      OptionBase::learntoption,
        "Principal eigenvectors of each Gaussian (for the 'general' type).\n"
        "Element j is a matrix whose row k is the k-th eigenvector of the\n"
        "j-th Gaussian.");

    declareOption(ol, "log_coeff", &GaussMix::log_coeff,
                                   OptionBase::learntoption,
        "The logarithm of the constant part in the joint Gaussian density:\n"
        "log(1/sqrt(2*pi^D * Det(C))).");

    declareOption(ol, "center", &GaussMix::center, OptionBase::learntoption,
        "Centers of each Gaussian, stored in rows.");

    declareOption(ol, "log_p_j_x", &GaussMix::log_p_j_x,
                                   OptionBase::learntoption,
        "The logarithm of p(j|x), where x is the input part.");

    declareOption(ol, "p_j_x", &GaussMix::p_j_x, OptionBase::learntoption,
        "The probability p(j|x), where x is the input part (it is computed\n"
        "by exp(log_p_j_x).");

    declareOption(ol,"diags", &GaussMix::diags, OptionBase::learntoption,
        "Element (j,k) is the standard deviation of Gaussian j on the k-th\n"
        "dimension.");

    // TODO Check doc.
    declareOption(ol, "n_eigen_computed", &GaussMix::n_eigen_computed,
                                          OptionBase::learntoption,
        "Actual number of principal components computed with 'general' type.");

    declareOption(ol, "D", &GaussMix::D, OptionBase::learntoption,
        "Number of dimensions in input space.");
    // TODO Doc: Including input & target.

    // TODO Do we really need to save this?
    declareOption(ol, "center_y_x", &GaussMix::center_y_x, OptionBase::learntoption,
        "The expectation E[Y | x] for each Gaussian.");
 
    /*


   
   

    declareOption(ol, "log_p_x_j_alphaj", &GaussMix::log_p_x_j_alphaj,
                                          OptionBase::learntoption,
        "The logarithm of p(x|j) * alpha_j, where x is the input part.");

    declareOption(ol, "n_tries", &GaussMix::n_tries, OptionBase::learntoption,
        "Element i is the number of iterations needed to complete\n"
        "stage i (if > 1, some Gaussian has been replaced).");
    // TODO Do we need this?

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

    // Would be used if the 'factor' type is implemented some day.
        // TODO Get rid?
  
/*  declareOption(ol, "V_idx", &GaussMix::V_idx, OptionBase::buildoption,
    "Used for general and factore Gaussians : A vector of size L. V_idx[l] is the row index of the first vector of Gaussian 'l' in the matrix 'V' (also used to index vector 'lambda')"); */

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
    // Check 'diags' has correct size: it used to be (D x L) instead of (L x D).
    // TODO To be removed in the future.
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

    // Check type value.
    if (type == "spherical") {
        type_id = TYPE_SPHERICAL;
    } else if (type == "diagonal") {
        type_id = TYPE_DIAGONAL;
    } else if (type == "general") {
        type_id = TYPE_GENERAL;
    } else
        PLERROR("In GaussMix::build_ - Type '%s' is unknown", type.c_str());

    // TODO Doc.
    resizeDataBeforeUsing();

    // TODO Explain why.
    // TODO 'log_coeff' does not need to be declared as an option, does it?
    if (stage > 0)
        precomputeAllGaussianLogCoefficients();

    // TODO Explain why.
    GaussMix::setInputTargetSizes(n_input_, n_target_, false);
    GaussMix::setInput(input_part, false);

    /*
    alpha.resize(L);
    log_p_x_j_alphaj.resize(L);
    log_p_j_x.resize(L);
    p_j_x.resize(L);
    // Those are not used for every type:
    cov_x.resize(0);
    cov_y_x.resize(0);
    eigenvectors.resize(0);
    eigenvectors_x.resize(0);
    eigenvectors_y_x.resize(0);
    full_cov.resize(0);
    log_coeff.resize(0);
    sigma.resize(0);
    y_x_mat.resize(0);
    if (type == "spherical") {
        sigma.resize(L);
    } else if (type == "diagonal") {
        // Nothing to resize here.
    } else if (type == "general") {
        cov_x.resize(L);
        cov_y_x.resize(L);
        eigenvectors.resize(L);
        eigenvectors_x.resize(L);
        eigenvectors_y_x.resize(L);
        full_cov.resize(L);
        log_coeff.resize(L);
        y_x_mat.resize(L);
    } else
        PLERROR("In GaussMix::build_ - Type '%s' is unknown", type.c_str());
    if (n_input == 0) {
        // No input part: the p_j_x must be obtained from the alpha.
        for (int j = 0; j < L; j++) {
            log_p_j_x[j] = pl_log(alpha[j]);
        }
        p_j_x << alpha;
    }
    PDistribution::finishConditionalBuild();
    */
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
            // TODO See if harmonic mean is needed ?
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
            // Make sure there are no negative eigenvalues.
            for (int i = n_eigen_computed - 1; i >= 0; i--)
                if (eigenvals[i] < 0)
                    if (is_equal(eigenvals[i], 0))
                        // The eigenvalue is approximately zero: must be the
                        // consequence of some numerical precision loss.
                        eigenvals[i] = 0;
                    else
                        PLERROR("In GaussMix::computeMeansAndCovariances - "
                                "Eigenvalue %d (equal to %f) is negative",
                                i, eigenvals[i]);
                else
                    // Eigenvalues are sorted by decreasing order: once we
                    // found a non-negative one, we can be sure all other
                    // eigenvalues are also non-negative.
                    break;
        }
    }
    /*
    VMat weighted_train_set;
    Vec sum_columns(L);
    columnSum(posteriors, sum_columns);
    for (int j = 0; j < L; j++) {
        // Build the weighted dataset.
        if (sum_columns[j] < epsilon)
            PLWARNING("In GaussMix::computeMeansAndCovariances - A posterior is almost zero");
        VMat weights(columnmatrix(updated_weights(j)));
        weighted_train_set = new ConcatColumnsVMatrix(
            new SubVMatrix(train_set, 0, 0, nsamples, D), weights);
        weighted_train_set->defineSizes(D, 0, 1);
        if (type == "spherical") {
            Vec variance(D);
            Vec center = mu(j);
            computeInputMeanAndVariance(weighted_train_set, center, variance);
            sigma[j] = sqrt(mean(variance));   // TODO See if harmonic mean is needed ?
#ifdef BOUNDCHECK
#ifdef __INTEL_COMPILER
#pragma warning(disable:279)  // Get rid of compiler warning.
#endif
            if (isnan(sigma[j])) {
                PLWARNING("In GaussMix::computeMeansAndCovariances - A standard deviation is nan");
            }
#ifdef __INTEL_COMPILER
#pragma warning(default:279)
#endif
#endif
        } else if (type == "diagonal") {
            Vec stddev(D);
            Vec center = mu(j);
            computeInputMeanAndStddev(weighted_train_set, center, stddev);
            diags(j) << stddev;
        } else if (type == "general") {
            Vec center = mu(j);
            // weighted_train_set->saveAMAT
            //   ("weighted_train_set_" + tostring(j) + ".amat");
            computeInputMeanAndCovar(weighted_train_set, center, covariance);
            if (center.hasMissing()) {
                // There are features missing in all points assigned to this
                // Gaussian. We sample a new random value for these features.
                for (int i = 0; i < D; i++)
                    if (is_missing(center[i])) {
                        center[i] =
                            random->gaussian_mu_sigma(mean_training  [i],
                                                      stddev_training[i]);
#ifdef BOUNDCHECK
                        // Safety check: the corresponding row and column in
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
                PLERROR("In GaussMix::computeMeansAndCovariances - Found missing values "
                        "when computing weighted mean and covariance");
#endif
            Vec eigenvals = eigenvalues(j); // The eigenvalues vector of the j-th Gaussian.
            eigenVecOfSymmMat(covariance, n_eigen_computed, eigenvals, eigenvectors[j]);
            assert( eigenvals.length() == n_eigen_computed );
            // Make sure there are no negative eigenvalues.
            for (int i = n_eigen_computed - 1; i >= 0; i--)
                if (eigenvals[i] < 0)
                    if (is_equal(eigenvals[i], 0))
                        // The eigenvalue is approximately zero: must be the
                        // consequence of some numerical precision loss.
                        eigenvals[i] = 0;
                    else
                        PLERROR("In GaussMix::computeMeansAndCovariances - Eigenvalue %d "
                                "(equal to %f) is negative", i, eigenvals[i]);
                else
                    break;
        } else
            PLERROR("In GaussMix::computeMeansAndCovariances - Not implemented for this type of Gaussian");
    }
*/
}

//////////////////////////
// computeLogLikelihood //
//////////////////////////
real GaussMix::computeLogLikelihood(const Vec& y, int j, bool is_input) const
{
    static int size; // TODO Document all
    static int start;
    static Vec mu_y;  // The corresponding mean.
    static Vec mu; // TODO Doc.
    static Vec diag_j; // TODO Doc.
    static real log_likelihood;
    static Vec eigenvals; // TODO Doc
    static Mat eigenvecs; // TODO Doc
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

            assert( !y.hasMissing() ); // TODO Deal with this case.

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
        } else {
            assert( !y.hasMissing() ); // TODO Deal with this case.

            if (is_input) {
                log_likelihood = log_coeff_x[j];
                mu = center(j).subVec(0, n_input);
                eigenvals = eigenvalues_x(j);
                eigenvecs = eigenvectors_x[j];
                y_centered.resize(n_input);
            } else {
                log_likelihood = log_coeff_y_x[j];
                //mu = center(j).subVec(n_input, n_target); // TODO WRONG!
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
    assert( !isnan(log_likelihood) );
    return log_likelihood;
    /*
    static real p;
    static int size;  // The length of y (a target, or an input).
    static int start; // n_input if y is a target, and 0 otherwise.
    static Mat eigenvecs; // A pointer to the adequate eigenvectors.
    Vec eigenvals;    // A pointer to the adequate eigenvalues.
    if (type == "spherical" || type == "diagonal") {
        // Both types are very similar.
        if (is_input) {
            size = n_input;
            start  = 0;
        } else {
            size = n_target;
            start = n_input;
        }
        mu_y = mu(j).subVec(start, size);
        p = 0.0;
        if (type == "spherical") {
            // x   ~= N(mu_x, sigma)
            // y|x ~= N(mu_y, sigma)
            real sigma_j = max(sigma_min, sigma[j]);
            for (int k = 0; k < size; k++)
                if (!is_missing(y[k]))
                    p += gauss_log_density_stddev(y[k], mu_y[k], sigma_j);
        } else if (type == "diagonal") {
            for (int k = 0; k < size; k++)
                if (!is_missing(y[k]))
                    p += gauss_log_density_stddev(y[k], mu_y[k],
                                                  max(sigma_min, diags(j, k + start)));
        }
#ifdef BOUNDCHECK
#ifdef __INTEL_COMPILER
#pragma warning(disable:279)  // Get rid of compiler warning.
#endif
        if (isnan(p))
            PLWARNING("In GaussMix::computeLogLikelihood - Density is nan");
#ifdef __INTEL_COMPILER
#pragma warning(default:279)
#endif
#endif
        return p;
    } else if (type == "general") {
    */
        /* Work-in-progress... we want it to work!
           if (n_margin > 0)
           PLERROR("In GaussMix::computeLogLikelihood - Marginalization not implemented for the general type");
        */
        // We store log(p(y | x,j)) in the variable t.
    /*
        static real t;
        static Vec y_centered; // Storing y - mu(j).
        static real squared_norm_y_centered;
        static real one_over_lambda0;
        static real lambda0;
        static real lambda;
        static int n_eig;
        static TVec<int> cond_flags_backup;
        bool need_restore_conditional_flags = false;
        bool empty_conditional_flags = false;
        static Vec y_no_missing;
        if (y.hasMissing()) {
            // There are missing values: we need to marginalize them first.
            need_restore_conditional_flags = true;
            y_no_missing.resize(0);
            int start;
            if (is_input)
                start = 0;
            else
                start = n_input;
            cond_flags_backup.resize(0);
            if (conditional_flags.isEmpty()) {
                // No input / marginalized part.
                empty_conditional_flags = true;
                conditional_flags.resize(D);
                conditional_flags.fill(2);
            }
            for (int i = 0; i < y.length(); i++)
                if (is_missing(y[i])) {
                    int index = cond_sort[i + start];
                    cond_flags_backup.append(index);
                    cond_flags_backup.append(conditional_flags[index]);
                    conditional_flags[index] = 0;
                } else
                    y_no_missing.append(y[i]);
            setConditionalFlags(conditional_flags);
        } else {
            // Just copy the given vector 'y'.
            y_no_missing.resize(y.length());
            y_no_missing << y;
        }
        if (is_input) {
            mu_y = mu(j).subVec(0, n_input);
            size = n_input;
            eigenvals = eigenvalues_x(j);
            eigenvecs = eigenvectors_x[j];
            n_eig = n_input;
        } else {
            size = n_target;
            if (n_input > 0) {
                // y|x ~= N(mu_y + K2 K1^-1 (x - mu_x), K3 - K2 K1^-1 K2')
                mu_y = mu_y_x(j);
                eigenvals = eigenvalues_y_x(j);
                eigenvecs = eigenvectors_y_x[j];
                n_eig = n_target;
            } else {
                // No input part: we can directly use the general eigenvalues and
                // eigenvectors (we just need to get rid of the marginalized part).
                mu_y = mu(j).subVec(0, n_target);
                eigenvals = eigenvalues(j);
                eigenvecs = eigenvectors[j].subMat(0, 0, n_eigen_computed, n_target);
                n_eig = n_eigen_computed;
            }
        }
        if (size > 0)
            y_centered.resize(size);
        else {
            // 'y_centered' may have a null storage if we just resize it to
            // zero. Thus we first resize it to one then to zero.
            assert( size == 0 );
            y_centered.resize(1);
            y_centered.resize(0);
        }
        t = log_coeff[j];
        y_centered << y_no_missing;
        y_centered -= mu_y;
        squared_norm_y_centered = pownorm(y_centered);
        real var_min = sigma_min*sigma_min;
        lambda0 = max(var_min, eigenvals[n_eig - 1]);
#ifdef BOUNDCHECK
        if (lambda0 <= 0)
            PLERROR("GaussMix::computeLogLikelihood(y,%d), var_min=%g, lambda0=%g\n",j,var_min,lambda0);
#endif
        one_over_lambda0 = 1.0 / lambda0;
        // t -= 0.5  * 1/lambda_0 * ||y - mu||^2
        t -= 0.5 * one_over_lambda0 * squared_norm_y_centered;
        for (int k = 0; k < n_eig - 1; k++) {
            // t -= 0.5 * (1/lambda_k - 1/lambda_0) * ((y - mu)'.v_k)^2
            lambda = max(var_min, eigenvals[k]);
#ifdef BOUNDCHECK
            if (lambda<=0)
                PLERROR("GaussMix::computeLogLikelihood(y,%d), var_min=%g, lambda(%d)=%g\n",j,var_min,k,lambda);
#endif
            if (lambda > lambda0)
                t -= 0.5 * (1.0 / lambda - one_over_lambda0) * square(dot(eigenvecs(k), y_centered));
        }
        if (need_restore_conditional_flags) {
            if (empty_conditional_flags)
                conditional_flags.resize(0);
            else
                for (int i = 0; i < cond_flags_backup.length(); i += 2)
                    conditional_flags[cond_flags_backup[i]] = cond_flags_backup[i+1];
            setConditionalFlags(conditional_flags);
        }
        // TODO Check the value returned here when all features are missing.
        return t;
    } else {
        PLERROR("In GaussMix::computeLogLikelihood - Not implemented for this type of Gaussian");
        return 0;
    }
*/
}

//////////////////////////////
// computeAllLogLikelihoods //
//////////////////////////////
void GaussMix::computeAllLogLikelihoods(const Vec& sample, const Vec& log_like)
{
    assert( sample.length()   == D );
    assert( log_like.length() == L );
    // TODO See if we can optimize some stuff when calling
    // computeLogLikelihood for each value of j.
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
    // TODO See if better to store log_alpha, in order to save log computations
    // later on.
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
        mu.clear();
        real* coeff = n_input == 0 ? alpha.data() : p_j_x.data();
        for (int j = 0; j < L; j++)
            mu += center(j).subVec(n_input, n_target) * coeff[j];
    } else {
        assert( type_id == TYPE_GENERAL );
        // The case 'n_input == 0' is considered above.
        assert( n_input > 0 );
        assert( false ); // TODO See where to take mu_y_x.
    }
 
    /*
    // NB: 'mu' has been renamed into 'result' to avoid confusion with other 'mu'.
    static Vec mu_y;
    result.resize(n_target);
    if (type == "spherical" || type == "diagonal") {
        // The expectation is the same in the 'spherical' and 'diagonal' cases.
        result.clear();
        for (int j = 0; j < L; j++)
            result += mu(j).subVec(n_input, n_target) * p_j_x[j];
    } else if (type == "general") {
    */
        /* Another work-in-progress.
           if (n_margin > 0)
           PLERROR("In GaussMix::expectation - Marginalization not implemented");
        */
    /*
        result.fill(0);
        bool is_simple = (n_input == 0 && n_margin == 0);
        for (int j = 0; j < L; j++) {
            if (is_simple)
                mu_y = mu(j).subVec(0, n_target);
            else
                mu_y = mu_y_x(j).subVec(0, n_target);
            result += mu_y * p_j_x[j];
        }
    } else
        PLERROR("In GaussMix::expectation - Not implemented for this type");
        */
}

////////////
// forget //
////////////
void GaussMix::forget()
{
    inherited::forget();
    log_p_j_x.resize(0);
    p_j_x.resize(0);
    /*
    stage = 0;
    */
    /* TODO See inherited::forget() --> and adapt help about seed.
    if (seed_ >= 0)
        manual_seed(seed_);
        */
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
    // static Vec mu_y;
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
            Vec mu_y = center_y_x(j);
            assert( false ); // TODO Implement
        }
        /* Should work sooner or later...
           if (n_margin > 0)
           PLERROR("In GaussMix::generateFromGaussian - Marginalization not implemented for the general type");
        */
        // TODO Does this work or not ?
    /*
        if (n_margin > 0)
            PLERROR("In GaussMix::generateFromGaussian - Is marginalization "
                    "implemented for the general type ??");
        static Vec norm;
        static real lambda0;
        static int n_eig;
        static Mat eigenvecs;
        static Vec mu_y;
        Vec eigenvals;
        if (n_input == 0) {
            n_eig = n_eigen_computed;
            eigenvals = eigenvalues(j);
            eigenvecs = eigenvectors[j].subMat(0, 0, n_eigen_computed, n_target);
            mu_y = mu(j).subVec(0, n_target);
        } else {
            n_eig = n_target;
            eigenvals = eigenvalues_y_x(j);
            eigenvecs = eigenvectors_y_x[j];
            mu_y = mu_y_x(j);
        }
        norm.resize(n_eig - 1);
        random->fill_random_normal(norm);
        real var_min = sigma_min*sigma_min;
        lambda0 = max(var_min, eigenvals[n_eig - 1]);
        s.fill(0);
        for (int k = 0; k < n_eig - 1; k++)
            // TODO See if can use more optimized function.
            s += sqrt(max(var_min, eigenvals[k]) - lambda0) * norm[k] * eigenvecs(k);
        norm.resize(n_target);
        random->fill_random_normal(norm);
        s += norm * sqrt(lambda0);
        s += mu_y;
    */
    }
}

///////////////////////
// getNEigenComputed //
///////////////////////
/*
int GaussMix::getNEigenComputed() const {
    return n_eigen_computed;
}
*/

/////////////////////
// getEigenvectors //
/////////////////////
/*
Mat GaussMix::getEigenvectors(int j) const {
    //return eigenvectors[j];
}
*/

//////////////////
// getEigenvals //
//////////////////
/*
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
    // TODO Do something sensible.
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
    /*
    string strmat = " 4 3 [ 0, 0, 0, 10, 10, 10, 0, 10, 0, 10, 0, 0 ] ";
    PStream in = openString(strmat, PStream::plearn_ascii);
    in >> clust;
    in.flush();
    */

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
 
    /*
    // TODO There is some code duplication with computePosteriors.
#ifdef BOUNDCHECK
    if (stage == 0)
        PLERROR("GaussMix::log_density was called while model was not trained");
#endif
    log_likelihood_dens.resize(L);
    // First we need to compute the likelihood p(y,j | x) = p(y | x,j) * p(j | x).
    for (int j = 0; j < L; j++) {
        log_likelihood_dens[j] = computeLogLikelihood(y, j) + log_p_j_x[j];
#ifdef BOUNDCHECK
#ifdef __INTEL_COMPILER
#pragma warning(disable:279)  // Get rid of compiler warning.
#endif
        if (isnan(log_likelihood_dens[j])) {
            PLWARNING("In GaussMix::log_density - computeLogLikelihood returned nan");
        }
#ifdef __INTEL_COMPILER
#pragma warning(default:279)
#endif
#endif
    }
    return logadd(log_likelihood_dens);
    */
    //return MISSING_VALUE;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GaussMix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
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
                                                int dimension)
{
    int n_eig = eigenvals.length();
    assert( dimension >= n_eig );
    real log_det = 0;
    real var_min = square(sigma_min);
    for (int k = 0; k < n_eig; k++) {
#ifdef BOUNDCHECK
        if (var_min < epsilon && eigenvals[k] < epsilon)
            PLWARNING("In GaussMix::precomputeGaussianLogCoefficient - An "
                      "eigenvalue is near zero");
#endif
        log_det += pl_log(max(var_min, eigenvals[k]));
    }
    if (dimension > n_eig)
        // Only the first 'n_eig' eigenvalues are given: we assume
        // the other eigenvalues are equal to the last given one.
        log_det += pl_log(max(var_min, eigenvals[n_eig - 1]))
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
    if (type == "spherical") {
        sigma[j] = sigma[high];
    } else if (type == "diagonal") {
        diags(j) << diags(high);
    } else if (type == "general") {
        eigenvalues(j) << eigenvalues(high);
        eigenvectors[j] << eigenvectors[high];
        log_coeff[j] = log_coeff[high];
        // TODO Check nothing more needs being done.
    } else
        PLERROR("In GaussMix::replaceGaussian - Not implemented for this type");
        // TODO Should not have to do this.
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
    log_coeff.resize(0);
    log_coeff_x.resize(0);
    log_coeff_y_x.resize(0);
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
        log_coeff_x.resize(L);
        log_coeff_y_x.resize(L);
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
        if (n_eigen == -1 || n_eigen == D)
            // We need to compute all eigenvectors.
            n_eigen_computed = D;
        else
            n_eigen_computed = n_eigen + 1;
        eigenvalues.resize(L, n_eigen_computed);
        eigenvectors.resize(L);
        for (int i = 0; i < eigenvectors.length(); i++)
            eigenvectors[i].resize(n_eigen_computed, D);
    }

    /*
    // Those are not used for every type:
    eigenvalues.resize(0,0);
    eigenvalues_x.resize(0,0);
    eigenvalues_y_x.resize(0,0);
    if (type == "diagonal") {
        diags.resize(L,D);
    } else if (type == "general") {
        if (n_eigen == -1 || n_eigen == D)
            // We need to compute all eigenvectors.
            n_eigen_computed = D;
        else
            n_eigen_computed = n_eigen + 1;
        for (int i = 0; i < L; i++)
            eigenvectors[i].resize(n_eigen_computed, D);
        eigenvalues.resize(L, n_eigen_computed);
    } else if (type == "spherical") {
        // Nothing more to resize.
    } else {
        PLERROR("In GaussMix::resizeDataBeforeTraining - Not implemented for this type");
    }
    */
}

//////////////
// setInput //
//////////////
void GaussMix::setInput(const Vec& input, bool call_parent) const {
    static Vec log_p_x_j_alphaj;
    static Vec x_minus_mu_x; // Used to store 'x - mu_x'.

    if (call_parent)
        inherited::setInput(input);

    if (n_input == 0) {
        // There is no input part anyway: nothing to do.
        assert( input_part.isEmpty() );
        return;
    }
    
    // TODO Document this: defining a mixture should set stage > 0.
    if (stage == 0)
        // The Gaussian mixture is not ready yet (it has not yet been
        // trained): there is nothing more we can do.
        return;

    // We need to compute:
    // p(j | x) = p(x | j) p(j) / p(x)
    //          = p(x | j) p(j) / sum_k p(x | k) p(k)

//    assert( type_id != TYPE_GENERAL || n_input == 0 );
    // TODO See what to do with general type.
    // TODO Check ::expectation with n_input > 0

    if (type_id == TYPE_GENERAL) {
        // We need to compute E[Y|x,j].
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

    /*
    if (stage == 0 || n_input == 0) {
        // Nothing to do.
        return;
    }
    if (type == "spherical") {
        // Nothing special to do.
    } else if (type == "diagonal") {
        // Nothing special to do.
    } else if (type == "general") {
        // We need to compute E[Y|x,j].
        x_minus_mu_x.resize(n_input);
        for (int j = 0; j < L; j++) {
            x_minus_mu_x << input;
            x_minus_mu_x -= mu(j).subVec(0, n_input);
            mu_target = mu_y_x(j);
            if (n_input > 0)
                product(mu_target, y_x_mat[j], x_minus_mu_x);
            else
                mu_target.fill(0);
            mu_target += mu(j).subVec(n_input, n_target);
        }
    } else {
        PLERROR("In GaussMix::setInput - Not implemented for this type");
    }
    for (int j = 0; j < L; j++)
        log_p_x_j_alphaj[j] = computeLogLikelihood(input, j, true) + pl_log(alpha[j]);
    real log_p_x = logadd(log_p_x_j_alphaj);
    real t;
    for (int j = 0; j < L; j++) {
        t = log_p_x_j_alphaj[j] - log_p_x;
        log_p_j_x[j] = t;
        p_j_x[j] = exp(t);
    }
    */
}

///////////////////////////
// getInitialWeightsFrom //
///////////////////////////
void GaussMix::getInitialWeightsFrom(const VMat& vmat)
{
    static Vec tmp1, tmp2;
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
    static Mat inv_cov_x;
    static Mat full_cov;
    static Mat cov_y_x;
    static Mat work_mat1, work_mat2;
    static Mat cross_cov;
    bool sizes_changed = true;

    if (call_parent)
        sizes_changed =
            inherited::setInputTargetSizes(n_i, n_t, call_parent);

    if (n_input == -1 || n_target == -1 || D == -1)
        // Sizes are not defined yet, there is nothing we can do.
        return sizes_changed;

    if (type_id == TYPE_SPHERICAL || type_id == TYPE_DIAGONAL ) {
        // Nothing to do.
    } else {
        assert( type_id == TYPE_GENERAL );

        work_mat1.resize(n_target, n_input);
        work_mat2.resize(n_target, n_target);
        Vec eigenvals;
        real var_min = square(sigma_min);
        for (int j = 0; j < L; j++) {
            // Compute the mean and covariance of x and y|x for the j-th
            // Gaussian (we will need them to compute the likelihood).

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
            // TODO Add note that cov_x_j is destroyed.
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
                // TODO Are the formula below correct?
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
            // TODO Keep this test?
//            if (n_target > 0) {
            if (n_input > 0) {
                cross_cov = full_cov_j.subMat(n_input,0, n_target, n_input);
                product(work_mat1, cross_cov, inv_cov_x);
                productTranspose(work_mat2, work_mat1, cross_cov);
                cov_y_x_j -= work_mat2;
                y_x_mat[j] << work_mat1;
            }
//            }
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
    return sizes_changed;
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

    /*

    // Mark start of training
    clock_t training_start = clock();

    // When training, we want to learn the full joint distribution.
    TVec<int> old_flags;
    bool restore_flags = ensureFullJointDistribution(old_flags);

    // Make sure everything is the right size.
    resizeStuffBeforeTraining();

    if (stage == 0) {
        n_tries.resize(0);
        // Copy the sample weights.
        if (train_set->weightsize() <= 0) {
            initial_weights.fill(1);
        } else {
            Vec tmp1, tmp2;
            real w;
            for (int i = 0; i < nsamples; i++) {
                train_set->getExample(i, tmp1, tmp2, w);
                initial_weights[i] = w;
            }
        }
        // Perform K-means to initialize the centers of the mixture.
        TVec<int> clust_idx;  // Store the cluster index for each sample.
        kmeans(train_set, L, clust_idx, mu, kmeans_iterations);
        posteriors.fill(0);
        for (int i = 0; i < nsamples; i++)
            // Initially, P(j | s_i) = 0 if s_i is not in the j-th cluster,
            // and 1 otherwise.
            posteriors(i, clust_idx[i]) = 1;
        updateSampleWeights();
        computeWeights();
        computeMeansAndCovariances();
        precomputeStuff();
    }

    Vec sample(D);
    bool replaced_gaussian = false;
    ProgressBar* pb = 0;
    int n_steps = nstages - stage;
    if (report_progress)
        pb = new ProgressBar("Training GaussMix", n_steps);

    // Update training_time.
    if (training_time >= 0)
        training_time += real(clock() - training_start) / real(CLOCKS_PER_SEC);

    while (stage < nstages) {
        n_tries.resize(stage + 1);
        n_tries[stage] = 0;
        if (verbosity >= 5)
            pout << endl << "Number of tries = " << flush;
        do {
            training_start = clock();
            n_tries[stage]++;
            if (verbosity >= 5 && n_tries[stage] % 10 == 0)
                pout << n_tries[stage] << ", " << flush;
            computePosteriors();
            replaced_gaussian = computeWeights();
            if (training_time >= 0)
                training_time +=
                    real(clock() - training_start) / real(CLOCKS_PER_SEC);
        } while (replaced_gaussian);
        training_start = clock();
        if (verbosity >= 5)
            pout << n_tries[stage] << endl;
        computeMeansAndCovariances();
        precomputeStuff();
        stage++;
        if (report_progress)
            pb->update(n_steps - nstages + stage);
        if (training_time >= 0)
            training_time += real(clock() - training_start) / real(CLOCKS_PER_SEC);
        // TODO Would be nice to have an 'official' clock in PLearn.
    }
    if (pb)
        delete pb;
    training_start = clock();
    // Restore conditional flags if necessary.
    if (restore_flags)
        setConditionalFlags(old_flags);
    // Clear 'log_p_j_x', because it may be filled with '-inf' after the initial
    // build: saving it would cause PLearn to crash when reloading the object.
    log_p_j_x.clear();
    // Options have changed: build is necessary.
    build();
    // Finish the computation of training time.
    if (training_time >= 0)
        training_time += real(clock() - training_start) / real(CLOCKS_PER_SEC);
    */
}

//////////////////////////////////
// updateFromConditionalSorting //
//////////////////////////////////
    /*
void GaussMix::updateFromConditionalSorting() const {
    static Mat dummy_mat; // Used for de-allocation of matrices.
    static Mat inv_cov_x, work_mat1, work_mat2;
    Mat tmp_cov; // Not static for shared storage issues.
    Vec eigenvals;
    clock_t updating_start = clock();
    // Update the centers of the Gaussians.
    sortFromFlags(mu);
    if (type == "spherical") {
        // Nothing more to do.
    } else if (type == "diagonal") {
        sortFromFlags(diags);
    } else if (type == "general") {
    */

        /* Trying to implement the n_margin > 0 case.
           if (n_margin > 0)
           PLERROR("In GaussMix::updateFromConditionalSorting - Marginalization not "
           "implemented for the general type");
        */
    /*

        if ((n_input == 0 && n_margin == 0) || stage == 0)
            // Nothing to do.
            return;
        real var_min = square(sigma_min);
        real lambda0;
        eigenvalues_x.resize(L, n_input);
        eigenvalues_y_x.resize(L, n_target);
        inv_cov_x.resize(n_input, n_input);
        mu_y_x.resize(L, n_target);
        work_mat1.resize(n_target, n_input);
        work_mat2.resize(n_target, n_target);
        for (int j = 0; j < L; j++) {
            // Update the eigenvectors.
            sortFromFlags(eigenvectors[j]);

            cov_x[j] = dummy_mat; // Free potential reference to full_cov[j].

            if (n_input == 0) {
                // Resize to zero unused stuff just to make sure it cannot be
                // used by mistake.
                cov_y_x[j].resize(0,0);
                y_x_mat[j].resize(0,0);
                eigenvectors_y_x[j].resize(0,0);
                eigenvalues_y_x.resize(0,0);
                continue; // No more computations needed.
            }
            // In the following there is an input part (n_input > 0).

            // Compute the mean and covariance of x and y|x for the j-th Gaussian (we
            // will need them to compute the likelihood).

            // First we compute the joint covariance matrix from the eigenvectors and
            // eigenvalues:
            // full_cov = sum_k (lambda_k - lambda0) v_k v_k' + lambda0.I
            int n_total = n_input + n_target;
            full_cov[j].resize(n_total, n_total);
            tmp_cov = full_cov[j];
            eigenvals = eigenvalues(j);
            lambda0 = max(var_min, eigenvals[n_eigen_computed - 1]);

            tmp_cov.fill(0);
            Mat eigenvectors_j =
                eigenvectors[j].subMat(0,0, eigenvectors[j].length(), n_total);
            for (int k = 0; k < n_eigen_computed - 1; k++)
                externalProductScaleAcc(tmp_cov, eigenvectors_j(k),
                                        eigenvectors_j(k),
                                        max(var_min, eigenvals[k]) - lambda0);
            for (int i = 0; i < n_total; i++)
                tmp_cov(i,i) += lambda0;
            // By construction, the resulting matrix is symmetric. However,
            // it may happen that it is not exactly the case due to numerical
            // approximations. Thus we ensure it is perfectly symmetric.
            assert( tmp_cov.isSymmetric(false) );
            fillItSymmetric(tmp_cov);
            // Extract the covariance of the input x.
            cov_x[j] = full_cov[j].subMat(0, 0, n_input, n_input);
            // Compute its SVD.
            eigenvectors_x[j].resize(n_input, n_input);
            eigenvals = eigenvalues_x(j);
            eigenVecOfSymmMat(cov_x[j], n_input, eigenvals, eigenvectors_x[j]);
            // And its inverse (we'll need it for the covariance of y|x).
            inv_cov_x.fill(0);
            lambda0 = max(var_min, eigenvals[n_input - 1]);
            real one_over_lambda0 = 1 / lambda0;
            Mat& eigenvectors_x_j = eigenvectors_x[j];
            for (int k = 0; k < n_input - 1; k++)
                externalProductScaleAcc(
                        inv_cov_x, eigenvectors_x_j(k), eigenvectors_x_j(k),
                        1 / max(var_min, eigenvals[k]) - one_over_lambda0);
            for (int i = 0; i < n_input; i++)
                inv_cov_x(i,i) += one_over_lambda0;

            // Compute the covariance of y|x.
            // It is only needed when there is an input part, since otherwise
            // we can simply use the full covariance.
            cov_y_x[j].resize(n_target, n_target);
            cov_y_x[j] <<
                full_cov[j].subMat(n_input, n_input, n_target, n_target);
            y_x_mat[j].resize(n_target, n_input);
            if (n_target > 0) {
                tmp_cov = full_cov[j].subMat(n_input,0, n_target, n_input);
                product(work_mat1, tmp_cov, inv_cov_x);
                productTranspose(work_mat2, work_mat1, tmp_cov);
                cov_y_x[j] -= work_mat2;
                y_x_mat[j] << work_mat1;
            }
            // Compute SVD of the covariance of y|x.
            eigenvectors_y_x[j].resize(n_target, n_target);
            eigenvals = eigenvalues_y_x(j);
            // Ensure covariance matrix is perfectly symmetric.
            assert( cov_y_x[j].isSymmetric(false, true) );
            fillItSymmetric(cov_y_x[j]);
            eigenVecOfSymmMat(cov_y_x[j], n_target, eigenvals, eigenvectors_y_x[j]);
        }
    } else {
        PLERROR("In GaussMix::updateFromConditionalSorting - Not implemented for this type");
    }
    if (conditional_updating_time >= 0)
        conditional_updating_time +=
            real(clock() - updating_start) / real(CLOCKS_PER_SEC);
}
    */

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
