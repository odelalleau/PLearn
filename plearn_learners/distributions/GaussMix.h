// -*- C++ -*-

// GaussMix.h
//
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

/*! \file GaussMix.h */
#ifndef GaussMix_INC
#define GaussMix_INC

#include "PDistribution.h"
#include <plearn/misc/PTimer.h>

namespace PLearn {
using namespace std;

class GaussMix: public PDistribution
{

private:

    typedef PDistribution inherited;

    //! Temporary storage used when computing posteriors.
    Vec log_likelihood_post, sample_row;

protected:

    // TODO Document (H3^-1 as in my tex file, for each Gaussian)
    TVec<Mat> H3_inverse;

    //! Used to measure the total training time.
    PP<PTimer> ptimer;

    //! All missing patterns found in the training set (stored in rows).
    //! A boolean value of 'true' indicates that a feature is missing.
    TMat<bool> missing_patterns;

    //! Missing patterns used as templates (obtained by k-median).
    TMat<bool> missing_template;

    //! Index of the current cluster whose spanning path is being walked on
    //! during training.
    int current_cluster;

    //! The i-th element is the index of the i-th training sample in the
    //! spanning path that contains it (i.e. if sample_to_path_index[i] = k,
    //! then the i-th training sample is in k-th position in the spanning path
    //! that contains it).
    TVec<int> sample_to_path_index;

    //! The k-th element is the list of ordered samples in the spanning path
    //! for the k-th cluster.
    TVec< TVec<int> > spanning_path;

    //! The k-th element is a vector that indicates whether at each step in the
    //! spanning path of the k-th cluster, a sample should use the previous
    //! covariance matrix computed in the path (true), or the one stored
    //! one step before it (false). Intuitively, a value of false means we are
    //! switching to another branch in the spanning tree.
    TVec< TVec<bool> > spanning_use_previous;

    //! The k-th element is a vector that indicates whether at each step in the
    //! spanning path of the k-th cluster, we should free the memory used by
    //! the previous covariance matrix (true), or we should instead keep this
    //! matrix for further use (false). Here, the 'previous' covariance matrix
    //! will be either the one just before (if the boolean given by
    //! 'spanning_use_previous' is also true), or the one before it (if it is
    //! false).
    TVec< TVec<bool> > spanning_can_free;

    //! Used to store the likelihood given by all Gaussians for each sample in
    //! the current cluster.
    Mat log_likelihood_post_clust;

    //! The k-th element is the list of samples in the k-th cluster.
    TVec< TVec<int> > clusters_samp;

    //! The list of all cholesky decompositions (of covariance matrices) that
    //! need to be kept in memory during training.
    mutable TVec<Mat> cholesky_queue;

    //! TODO Document (the VMats with the imputed missing values).
    TVec<VMat> imputed_missing;

    //! TODO Document (the mats with the imputed missing values by each
    //gaussian)
    TVec<Mat> clust_imputed_missing;

    //! TODO Document (sum of all posteriors in computePosteriors()).
    Vec sum_of_posteriors;

    //! TODO Document
    TVec<bool> no_missing_change;

    // TODO Document
    // List of inverse matrices (H_3^-1).
    mutable TVec<Mat> cond_var_inv_queue;

    //! The list of all lists of dimension indices (of covariance matrices)
    //! that need to be kept in memory during training.
    //! More precisely, the i-th element is a vector that lists the indices
    //! of the dimensions in cholesky_queue[i] (since each Choleksy matrix is
    //! taken from a subset of all original dimensions - the ones for which
    //! the sample value is not missing).
    mutable TVec< TVec<int> > indices_queue;

    // TODO Document
    // Same but for inverse matrices (H_3^-1).
    mutable TVec< TVec<int> > indices_inv_queue;

    //! Set at build time, this integer value depends uniquely on the 'type'
    //! option. It is meant to avoid too many useless string comparisons.
    int type_id;

    //! Mean and standard deviation of the training set.
    // TODO There may be a need to declare them as learnt options if one wants
    // to continue the training of a saved GaussMix.
    Vec mean_training, stddev_training;

    // TODO Document (to store the covariance of the error, that we need to add
    // when imputing missing values).
    TVec<Mat> error_covariance;

    //! The posterior probabilities P(j | s_i), where j is the index of a
    //! Gaussian and i is the index of a sample.
    Mat posteriors;

    //! The initial weights of the samples s_i in the training set, copied for
    //! efficiency concerns.
    Vec initial_weights;

    //! A matrix whose j-th line is a Vec with the weights of each sample s_i
    //! for Gaussian j, i.e. the initial weight of s_i multiplied by the
    //! posterior P(j | s_i).
    Mat updated_weights;

    TVec<Mat> eigenvectors_x;   //!< The eigenvectors of the covariance of X.
    mutable Mat eigenvalues_x;  //!< The eigenvalues of the covariance of X.
    TVec<Mat> y_x_mat;          //!< The product K2 * K1^-1 to compute E[Y|x].
    TVec<Mat> eigenvectors_y_x; //!< The eigenvectors of the covariance of Y|x.
    Mat eigenvalues_y_x;        //!< The eigenvalues of the covariance of Y|x.

    //! Used to store the conditional expectation E[Y | X = x].
    Mat center_y_x;

    //! The logarithm of P(j|x), where x is the predictor part.
    mutable Vec log_p_j_x;

    //! The probability P(j|x), where x is the predictor part (it is computed
    //! by exp(log_p_j_x)).
    mutable Vec p_j_x;

    //! The logarithm of the constant part in the joint Gaussian density:
    //! log(1/sqrt(2*pi^D * Det(C))).
    //! This is a vector of length L (one coefficient for each Gaussian).
    Vec log_coeff;

    //! The logarithm of the constant part in P(X) and P(Y | X = x), similar to
    //! what 'log_coeff' is for the joint distribution P(X,Y).
    Vec log_coeff_x, log_coeff_y_x;

    //! The j-th element is the full covariance matrix for Gaussian j. It is
    //! needed only with the 'general' type, in presence of missing values.
    mutable TVec<Mat> joint_cov;

    // TODO Document (inverse covariance of joint).
    mutable TVec<Mat> joint_inv_cov;

    //! The j-th element is the matrix L in the Cholesky decomposition S = L L'
    //! of the covariance matrix S of Gaussian j.
    TVec<Mat> chol_joint_cov;

    //! The (i,j)-th element is the matrix L in the Cholesky decomposition of
    //! the covariance for the pattern of missing values of the i-th template
    //! for the j-th Gaussian.
    // TMat<Mat> chol_cov_template;
    // TODO Remove: not needed anymore.

    //! Indicates at which stage the full joint covariance was computed in
    //! 'joint_cov' (so that we know there is no need to compute it again).
    TVec<int> stage_joint_cov_computed;

    //! The i-th element is the index of the missing template of sample i in
    //! the 'missing_template' matrix.
    TVec<int> sample_to_template;

    //! Hack to know that we are compute likelihood on a training sample.
    int current_training_sample;

    //! Index of the previous training sample whose likelihood (and associated
    //! covariance matrix) has been computed.
    //! If set to -1, there was no previous training sample (first sample in a
    //! cluster).
    //! If set to -2, we do not want to use the efficient missing algorithm at
    //! all.
    int previous_training_sample;

    //! A boolean indicating whether or not the last predictor part set
    //! through setPredictor(..) had a missing value.
    mutable bool previous_predictor_part_had_missing;

    //! Storage vector to save some memory allocations.
    mutable Vec y_centered;

    //! Storage for the (weighted) covariance matrix of the dataset.
    //! It is only used with when type == "general".
    Mat covariance;

    //! Temporary storage vector.
    mutable Vec log_likelihood_dens;

    // TODO Document (i-th element = if missing pattern has changed).
    // (note: only used when efficient_missing == 2)
    TVec<bool> need_recompute;

    // TODO Document (maps a sample in the original training set to its index
    // in the reordered train set).
    TVec<int> original_to_reordered;

    // *********************
    // * protected options *
    // *********************

    int D;
    Mat diags;
    Mat eigenvalues;
    TVec<Mat> eigenvectors;
    int n_eigen_computed;
    int nsamples;

public:

    // ************************
    // * public build options *
    // ************************

    real alpha_min;
    int efficient_k_median;
    int efficient_k_median_iter;
    int efficient_missing;
    real epsilon;
    bool impute_missing;
    int kmeans_iterations;
    int L;
    int max_samples_in_cluster;
    int min_samples_in_cluster;
    int n_eigen;
    real sigma_min;
    string type;

    // The following options are actually learnt options (since they are learnt
    // during training), but are public so that one may easily define a given
    // mixture of Gaussians.
    Vec alpha;
    Mat center;
    Vec sigma;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    GaussMix();

    // ******************
    // * Object methods *
    // ******************

    //! Overridden in order to detect changes that require a call to forget().
    virtual void changeOptions(const map<string,string>& name_value);

protected:

    // ********************
    // * GaussMix methods *
    // ********************

    //! Generate a sample 's' from the given Gaussian. If 'given_gaussian' is
    //! equal to -1, then a random Gaussian will be chosen according to the
    //! prior weights alpha.
    void generateFromGaussian(Vec& s, int given_gaussian) const;

    //! In the 'general' conditional type, will precompute the covariance
    //! matrix of Y|x.
    virtual bool setPredictorPredictedSizes(int the_predictor_size,
                                            int the_predicted_size,
                                            bool call_parent = true);


    //! Main implementation of 'setPredictorPredictedSizes', that needs to be
    //! 'const' as it currently needs to be called in setPredictor(..).
    void setPredictorPredictedSizes_const(int the_predictor_size,
                                          int the_predicted_size) const;

    //! Fill the 'initial_weights' vector with the weights from the given
    //! VMatrix (which must have a weights column).
    void getInitialWeightsFrom(const VMat& vmat);

    //! Given the posteriors, fill the centers and covariance of each Gaussian.
    virtual void computeMeansAndCovariances();

    //! Compute posteriors P(j | s_i) for each sample point and each Gaussian.
    //! Note that actual weights (stored in 'updated_weights') will not be
    //! until you explicitely call 'updateSampleWeights'.
    virtual void computePosteriors();

    //! Fill the 'log_like' vector with the log-likelihood of each Gaussian for
    //! the given sample.
    void computeAllLogLikelihoods(const Vec& sample, const Vec& log_like);

    //! Compute log p(y | x,j), with j < L the index of a mixture's component,
    //! and 'x' the current predictor part.
    //! If 'is_predictor' is set to true, then it is the likelihood of the des
    //! that will be returned, i.e. log p(X = y | j).
    real computeLogLikelihood(const Vec& y, int j, bool is_predictor = false)
                              const;

    //! Return log( 1 / sqrt( 2 Pi^dimension |C| ) ), i.e. the logarithm of the
    //! constant coefficient in the Gaussian whose covariance matrix has the
    //! given eigenvalues 'eigenvals' in dimension 'dimension'. If the length
    //! of 'eigenvals' is not equal to 'dimension', the missing eigenvalues are
    //! assumed to be equal to the last in 'eigenvals' (which must be sorted in
    //! decreasing order of eigenvalues).
    real precomputeGaussianLogCoefficient(const Vec& eigenvals, int dimension)
                                          const;

    //! When type is 'general', fill the 'log_coeff' vector with the result of
    //! precomputeGaussianLogCoefficient(..) applied to each component of the
    //! mixture.
    void precomputeAllGaussianLogCoefficients();

    //! Make sure everything has the right size when training starts.
    void resizeDataBeforeTraining();

    //! Make sure everything has the right size when the object is about to be
    //! used (e.g. after loading a saved GaussMix object).
    void resizeDataBeforeUsing();

    //! Compute the weight of each Gaussian (the coefficient 'alpha'). If a
    //! Gaussian's coefficient is too low (i.e. less than 'alpha_min') and
    //! 'allow_replace' is set to true, this Gaussian will be removed and
    //! replaced by a new one, while this method will return 'true' (otherwise
    //! it will return 'false').
    bool computeMixtureWeights(bool allow_replace = true);

    //! Replace the j-th Gaussian with another one (probably because that one is
    //! not appropriate). The new one is centered on a random point sampled from
    //! the Gaussian with highest weight alpha, and has the same covariance.
    //! The weight of the Gaussian with highest weight is divided by two, and
    //! the weight of the new Gaussian is equal to the result of this division
    //! (so that weights still sum to one).
    void replaceGaussian(int j);

    //! Update the sample weights according to their initial weights and the current
    //! posterior probabilities (see documentation of 'updated_weights' for the
    //! exact formula).
    void updateSampleWeights();

private:

    //! This does the actual building.
    void build_();

protected:

    //! Declares this class' options
    static void declareOptions(OptionList& ol);

    //! Perform K-means.
    void kmeans(const VMat& samples, int nclust, TVec<int>& clust_idx,
                Mat& clust, int maxit=9999);

    //! Fill 'chol_updated' with the Cholesky decomposition of the submatrix of
    //! 'full_matrix' corresponding to the selection of dimensions given by
    //! 'indices_updated'.
    //! This Cholesky decomposition is obtained from an existing previous
    //! Cholesky decomposition, given by 'chol_previous', and corresponding to
    //! another subset of dimensions of the full matrix, given by
    //! 'indices_previous'.
    //! Note that 'indices_updated' will be modified so that all dimensions
    //! common with 'indices_previous' will come first.
    void updateCholeskyFromPrevious(
        const Mat& chol_previous, Mat& chol_updated,
        const Mat& full_matrix,
        const TVec<int>& indices_previous, const TVec<int>& indices_updated)
        const;

    // TODO DOCUMENT
    // (use the inverse variance lemma to update the inverse covariance matrix)
    void updateInverseVarianceFromPrevious(
        const Mat& src, Mat& dst, const Mat& full,
        const TVec<int>& ind_src, const TVec<int>& ind_dst);

    // TODO DOCUMENT!!!
    void addToCovariance(const Vec& y, int j, const Mat& cov, real post);

    //! Overridden so as to compute specific GaussMix outputs.
    virtual void unknownOutput(char def, const Vec& input, Vec& output, int& k) const;

public:

    //! Reset the learner.
    virtual void forget();

    // Simply calls inherited::build() then build_().
    virtual void build();

    //! Transform a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods.
    PLEARN_DECLARE_OBJECT(GaussMix);

    // ********************
    // * PLearner methods *
    // ********************

    //! Trains the model.
    virtual void train();

    //! Overridden to take into account new outputs computed.
    virtual int outputsize() const;

    //! Overridden to compute the training time.
    virtual TVec<string> getTrainCostNames() const;

    //! Overridden to reorder the dataset when efficient_missing == 2.
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    // *************************
    // * PDistribution methods *
    // *************************

    //! Set the value for the predictor part of the conditional probability.
    virtual void setPredictor(const Vec& predictor, bool call_parent = true)
                              const;

    //! Return density p(y | x).
    virtual real log_density(const Vec& y) const;

    //! Return survival fn = P(Y>y | x).
    virtual real survival_fn(const Vec& y) const;

    //! Return survival fn = P(Y<y | x).
    virtual real cdf(const Vec& y) const;

    //! Compute E[Y | x].
    virtual void expectation(Vec& mu) const;

    //! Compute Var[Y | x] (currently not implemented).
    virtual void variance(Mat& cov) const;

    //! Generate a sample from this distribution.
    virtual void generate(Vec& s) const;

    /*
    //! "Get" methods.
    int getNEigenComputed() const;
    Mat getEigenvectors(int j) const;
    Vec getEigenvals(int j) const;
    Vec getLogLikelihoodDens() const { return log_likelihood_dens; }
    */

    /*********** Static members ************/

protected:

    //! Recursive function to compute a spanning path from previously computed
    //! cost values (given by 'message_up' and 'message_down').
    //! This function fills 'path' with the list of nodes to visit; it also
    //! fills 'span_can_free' and 'span_use_previous' with information about
    //! when to free memory and when to switch to another branch in the tree.
    static void traverse_tree(TVec<int>& path,
                              TVec<bool>& span_can_free,
                              TVec<bool>& span_use_previous,
                              bool free_previous,
                              bool use_previous,
                              int index_node, int previous_node,
                              const TVec<int>& parent,
                              const TVec< TVec<int> >& children,
                              const TVec<int>& message_up,
                              const TVec<int>& message_down);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(GaussMix);

} // end of namespace PLearn

#endif


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
