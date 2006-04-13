// -*- C++ -*-

// NonLocalManifoldParzen.h
//
// Copyright (C) 2004 Yoshua Bengio & Hugo Larochelle
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

// Authors: Yoshua Bengio & Hugo Larochelle

/*! \file NonLocalManifoldParzen.h */


#ifndef NonLocalManifoldParzen_INC
#define NonLocalManifoldParzen_INC

#include "UnconditionalDistribution.h"
#include <plearn/io/PStream.h>
#include <plearn_learners/generic/PLearner.h>
#include <plearn/var/Func.h>
#include <plearn/opt/Optimizer.h>
#include <plearn_learners/distributions/PDistribution.h>
#include <plearn/ker/DistanceKernel.h>

namespace PLearn {
using namespace std;

class NonLocalManifoldParzen: public UnconditionalDistribution
{

private:

    typedef UnconditionalDistribution inherited;

protected:

    // *********************
    // * protected options *
    // *********************

    // ### declare protected option fields (such as learnt parameters) here


    // NON-OPTION FIELDS
    //! Input size
    int n;
    //! Cost of one example
    Func cost_of_one_example;
    //! Input vector
    Var x;
    //! Parameters of the neural network
    Var b, W, c, V, muV, snV, snb;
    //! Tangent vector targets
    Var tangent_targets;
    //! Tangent vectors spanning the tangent plane, given by
    //! the neural network
    Var tangent_plane;
    //! Mean of the gaussian
    Var mu;
    //! Sigma^2_noise of the gaussian
    Var sn;
    //Var mu_noisy, noise_var;
    //PP<PDistribution> dist;
    //! Sum of NLL cost
    Var sum_nll;
    //! Mininum value of sigma^2_noise
    Var min_sig;
    //! Initial (approximate) value of sigma^2_noise
    Var init_sig;
    //! Embedding computed by the (embedding) neural network
    Var embedding;
    //! Function to output the embedding
    Func output_embedding;
    //! Predictor of the parameters of the gaussian at x
    Func predictor; // predicts everything about the gaussian

    //TVec< Mat > Us;
    //Mat mus,sms;
    //Vec sns;

    //! log_density and Kernel methods' temporary variables
    mutable Mat U_temp, F, distances;
    //! log_density and Kernel methods' temporary variables
    mutable Vec mu_temp,sm_temp,sn_temp,diff,z, x_minus_neighbor,
        t_row, neighbor_row, log_gauss,t_dist;
    //! log_density and Kernel methods' temporary variables
    mutable TVec<int> t_nn;
    //! log_density and Kernel methods' temporary variables
    mutable DistanceKernel dk;

    //! SVD computation variables
    mutable Mat Ut_svd, V_svd;
    //! SVD computation variables
    mutable Vec S_svd;

    //! Predictions for mu
    Mat mus;
    //! Predictions for sn
    Vec sns;
    //! Predictions for sm
    Mat sms;
    //! Predictions for F
    TVec<Mat> Fs;

    //! Position of the current example in
    //! the training set
    int curpos;

    VMat train_set_with_targets;
    VMat targets_vmat;
    Var totalcost;
    int nsamples;

public:

    // ************************
    // * public build options *
    // ************************

    // ### declare public option fields (such as build options) here

    //! Parameters of the model
    //! It is put here so that these parameters can
    //! be shared by different NonLocalManifoldParzen
    //! objects (eventually, should use the "friend class"
    //! principal instead)
    VarArray parameters;


    // Embedding penalty weight
    //real weight_embedding;

    //! Weight decay for all weights
    real weight_decay;
    //! Penalty type to use on the weights
    string penalty_type;

    //real noise_grad_factor;
    //real noise;
    //string noise_type;
    //int omit_last;
    //bool magnified_version;

    //! Indication that the mean of the gaussians should be learned
    bool learn_mu;
    //! Reference set of points in the gaussian mixture
    VMat reference_set;
    //! Initial (approximate) value of sigma^2_noise
    real sigma_init;
    //! Minimum value of sigma^2_noise
    real sigma_min;
    //! Number of gaussians
    int L;
    //! User specified hidden layer NaryVariable subclass
    Var hidden_layer;
    //! Logarithm of number of gaussians
    real log_L;
    //! Number of neighbors used for gradient descent
    int nneighbors;
    //! Number of neighbors for the p(x) density estimation
    int nneighbors_density;
    //! Number of neighbors to learn the mus
    int mu_nneighbors;
    //! Number of reduced dimensions (number of tangent vectors to compute)
    int ncomponents;
    //! Threshold applied on the update rule for sigma^2_noise
    real sigma_threshold_factor;
    //! Variance transfer function ("square", "exp" or "softplus")
    string variances_transfer_function;
    //! Optimizer of the neural network
    PP<Optimizer> optimizer;
    //! Architecture type of the neural network ("single_neural_network" or "embedding_neural_nework")
    string architecture_type;
    //! Number of hidden units
    int n_hidden_units;
    //! Batch size of the gradient-based optimization
    int batch_size;
    //! SVD threshold on the eigen values
    real svd_threshold;
    //! Number of steps in the random walk
    int rw_n_step;
    //! Size of the step
    real rw_size_step;
    //! Which principal component to follow
    int rw_ith_component;
    //! File name for the rw saves
    string rw_file_name;
    //! Number of iterations between rw saves
    int rw_save_every;
    //! Indication that the predicted parameters should be stored
    bool store_prediction;
    //! Parameters to share
    VarArray shared_parameters;
    //! Number of optimizer stages
    int optstage_per_lstage;
    //! Number of iterations since the last save
    //! after which the parameters
    //! must be saved
    int save_every;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    NonLocalManifoldParzen();


    // ********************
    // * PLearner methods *
    // ********************

private:

    //! This does the actual building.
    void build_();

    //void update_reference_set_parameters();

    void knn(const VMat& vm, const Vec& x, const int& k, TVec<int>& neighbors, bool sortk) const;

protected:

    //! Declares this class' options.
    static void declareOptions(OptionList& ol);

    virtual void initializeParams();

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! Simply calls inherited::build() then build_().
    virtual void build();

    //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0 (this is the stage of a fresh learner!).
    virtual void forget();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods.
    // If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS.
    PLEARN_DECLARE_OBJECT(NonLocalManifoldParzen);

    // *******************************
    // **** PDistribution methods ****
    // *******************************

    //! Return log of probability density log(p(y)).
    virtual real log_density(const Vec& x) const;

    //! Return log density of ith point in reference_set
    real log_density(int i);

    //! The role of the train method is to bring the learner up to stage==nstages,
    //! updating the train_stats collector with training costs measured on-line in the process.
    virtual void train();

    /* Not implemented for now
    //! Return E[Y].
    virtual void expectation(Vec& mu) const;

    //! Return Var[Y].
    virtual void variance(Mat& cov) const;

    //! Return a pseudo-random sample generated from the distribution.
    virtual void generate(Vec& y) const;

    //! Reset the random number generator used by generate() using the given seed.
    virtual void resetGenerator(long g_seed);
    */

    //! Produce outputs according to what is specified in outputs_def.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Returned value depends on outputs_def.
    virtual int outputsize() const;

    /* Not needed anymore
    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;
    */


    // *** SUBCLASS WRITING: ***
    // While in general not necessary, in case of particular needs
    // (efficiency concerns for ex) you may also want to overload
    // some of the following methods:
    // virtual void computeOutputAndCosts(const Vec& input, const Vec& target, Vec& output, Vec& costs) const;
    // virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;
    // virtual void test(VMat testset, PP<VecStatsCollector> test_stats, VMat testoutputs=0, VMat testcosts=0) const;
    // virtual int nTestCosts() const;
    // virtual int nTrainCosts() const;

    //Mat getEigenvectors(int j) const;
    //Vec getTrainPoint(int j) const;
    real evaluate(const Vec x1,const Vec x2,real scale=1);
};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(NonLocalManifoldParzen);

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
