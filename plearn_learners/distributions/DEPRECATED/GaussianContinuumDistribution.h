// -*- C++ -*-

// GaussianContinuumDistribution.h
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

// Authors: Yoshua Bengio & Martin Monperrus

/*! \file PLearn/plearn_learners/distributions/DEPRECATED/GaussianContinuumDistribution.h */


#ifndef GaussianContinuumDistribution_INC
#define GaussianContinuumDistribution_INC

#include "UnconditionalDistribution.h"
#include <plearn/io/PStream.h>
#include <plearn_learners/generic/PLearner.h>
#include <plearn/var/Func.h>
#include <plearn/opt/Optimizer.h>
#include <plearn_learners/distributions/PDistribution.h>
#include <plearn/ker/DistanceKernel.h>

namespace PLearn {
using namespace std;

class GaussianContinuumDistribution: public UnconditionalDistribution
{

private:

    typedef UnconditionalDistribution inherited;
  
protected:
    // NON-OPTION FIELDS
    int n;
    Func cost_of_one_example;
    //Func verify_gradient_func;
    Var x, noise_var; // input vector
    Var b, W, c, V, muV, smV, smb, snV, snb; // explicit view of the parameters (also in parameters field).
    //Var W_src, c_src, V_src, muV_src, smV_src, smb_src, snV_src, snb_src; 
    //VarArray mu_neighbors, sm_neighbors, sn_neighbors, hidden_neighbors, input_neighbors, index_neighbors, tangent_plane_neighbors;
    Var tangent_targets, tangent_targets_and_point; // target for the tangent vectors for one example 
    Var tangent_plane;
    Var mu, sm, sn, mu_noisy; // parameters of the conditional models
    Var p_x, p_target, p_neighbors, p_neighbors_and_point, target_index, neigbor_indexes;
    Var sum_nll;
    Var min_sig, min_d;
    Var fixed_min_sig, fixed_min_d;

    PP<PDistribution> dist;

    // Random walk fields
    Array<VMat> ith_step_generated_set;

    // p(x) computation fields
    VMat train_and_generated_set;
    TMat<int> train_nearest_neighbors;
    TVec< Mat > Bs, Fs;
    Mat mus;
    Vec sms;
    Vec sns;

    Mat Ut_svd, V_svd;  // for SVD computation
    Vec S_svd;      // idem
    mutable Vec z, zm, zn, x_minus_neighbor, w;
    mutable Vec t_row, neighbor_row;
    mutable Vec t_dist;
    mutable Mat distances;

    mutable DistanceKernel dk;

    real best_validation_cost;

    // *********************
    // * protected options *
    // *********************

    // ### declare protected option fields (such as learnt parameters) here
    VarArray parameters;

public:

    mutable TVec<int> t_nn;
    mutable Vec log_gauss;                        //!< To store log(P(x|k)).
    mutable Mat w_mat;                            //!< To store local coordinates.
    VMat reference_set;

    // ************************
    // * public build options *
    // ************************

    // ### declare public option fields (such as build options) here

    real weight_mu_and_tangent;
    bool include_current_point;
    real random_walk_step_prop;
    bool use_noise;
    bool use_noise_direction;
    real noise;
    string noise_type;
    int n_random_walk_step;
    int n_random_walk_per_point;
    bool save_image_mat;
    bool walk_on_noise;
    real min_sigma;
    real min_diff;
    real fixed_min_sigma;
    real fixed_min_diff;
    real min_p_x;
    bool sm_bigger_than_sn;
    int n_neighbors; // number of neighbors used for gradient descent
    int n_neighbors_density; // number of neighbors for the p(x) density estimation
    int mu_n_neighbors; // number of neighbors to learn the mus
    int n_dim; // number of reduced dimensions (number of tangent vectors to compute)
    real sigma_grad_scale_factor;
    int update_parameters_every_n_epochs;
    string variances_transfer_function; // "square", "exp" or "softplus"
    PP<Optimizer> optimizer; // to estimate the function that predicts local tangent vectors given the input
    Var embedding;
    Func output_embedding;
    Func output_f;
    Func output_f_all;
    Func predictor; // predicts everything about the gaussian
    Func projection_error_f; // map output to projection error
    Func noisy_data;

    // manual construction of the tangent_predictor
    string architecture_type; // "neural_network" or "linear" or "" or "embedding_neural_nework" or "embedding_quadratic" 
    string output_type; // "tangent_plane", "embedding", or "tangent_plane+embedding".
    int n_hidden_units;

    int batch_size;

    real norm_penalization; // penalizes sum_i (||f_i||^2-1)^2
    real svd_threshold;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    GaussianContinuumDistribution();


    // ********************
    // * PLearner methods *
    // ********************

private: 

    //! This does the actual building. 
    void build_();

    void compute_train_and_validation_costs();

    void make_random_walk();
  
    void update_reference_set_parameters();

    void knn(const VMat& vm, const Vec& x, const int& k, TVec<int>& neighbors, bool sortk) const; 

protected: 
  
    //! Declares this class' options.
    static void declareOptions(OptionList& ol);

    //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0 (this is the stage of a fresh learner!).
    virtual void forget();
    virtual void initializeParams();

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! Simply calls inherited::build() then build_().
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods.
    // If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS.
    PLEARN_DECLARE_OBJECT(GaussianContinuumDistribution);

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
    virtual void resetGenerator(long g_seed) const;
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

    Mat getEigenvectors(int j) const;
  
    Vec getTrainPoint(int j) const;

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(GaussianContinuumDistribution);
  
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
