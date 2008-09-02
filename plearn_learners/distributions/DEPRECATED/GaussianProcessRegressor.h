// -*- C++ -*-

// GaussianProcessRegressor.h
//
// Copyright (C) 2003 Yoshua Bengio
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

/*! \file PLearn/plearn_learners/distributions/DEPRECATED/GaussianProcessRegressor.h */


#ifndef GaussianProcessRegressor_INC
#define GaussianProcessRegressor_INC

#include "PConditionalDistribution.h"
#include <plearn/ker/Kernel.h>

namespace PLearn {
using namespace std;

/*! Simple Gaussian Process Regression.

// prediction = E[E[y|x]|training_set] = E[y|x,training_set]
// prediction[j] = sum_i alpha_{ji} K(x,x_i)
//               = (K(x,x_i))_i' inv(K+sigma^2[j] I) targets
//
// Var[y[j]|x,training_set] = Var[E[y[j]|x]|training_set] + E[Var[y[j]|x]|training_set]
//  where
//  Var[E[y[j]|x]|training_set] = K(x,x)- (K(x,x_i))_i' inv(K+sigma^2[j]) (K(x,x_i))_i
//  and
//  E[Var[y[j]|x]|training_set] = Var[y[j]|x] = sigma^2[j] = noise
//
// costs:
//   MSE = sum_j (y[j] - prediction[j])^2
//   NLL = sum_j log Normal(y[j];prediction[j],Var[y[j]|x,training_set])
//

*/
class GaussianProcessRegressor: public PConditionalDistribution
{

public:
    typedef PConditionalDistribution inherited;  
    // Build options

    PP<Kernel> kernel; // kernel = prior covariance on functions
    int n_outputs; // dimension of the target variables
    Vec noise_sd; // output noise standard deviation, for each output dimension
    string Gram_matrix_normalization; // normalization method to apply to Gram matrix:
    // "none": no normalization
    // "centering_a_dot_product": this is the kernel PCA centering
    //     K_{ij} <-- K_{ij} - mean_i(K_ij) - mean_j(K_ij) + mean_{ij}(K_ij)
    // "centering_a_distance": this is the MDS transformation of squared distances to dot products
    //     K_{ij} <-- -0.5(K_{ij} - mean_i(K_ij) - mean_j(K_ij) + mean_{ij}(K_ij))
    // "divisive": this is the spectral clustering and Laplacian eigenmaps normalization
    //     K_{ij} <-- K_{ij}/sqrt(mean_i(K_ij) mean_j(K_ij))
    //
    int max_nb_evectors; // if -1 compute all eigenvectors, o/w compute only that many principal eigenvectors


    // temporary fields that don't need to be saved = NON-OPTIONS

    Mat alpha; // each row has the coefficients of K(x,x_j) in regression for i-th output
    mutable Vec Kxxi; // has K(x,x_i) for current input x
    mutable real Kxx; // has K(x,x)  for current input x
    Mat K; // non-sparse Gram matrix
    Mat eigenvectors; // principal eigenvectors (in the rows!)
    Vec eigenvalues; // and corresponding eigenvalues
    Vec meanK; // meanK[j]=mean_i(K_{ij})
    real mean_allK;

public:

    GaussianProcessRegressor();
    virtual ~GaussianProcessRegressor();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Set the input part before using the inherited methods
    virtual void setInput(const Vec& input) const;

    //! return log of probability density log(p(x))
    virtual double log_density(const Vec& x) const;

    //! return E[X] 
    virtual Vec expectation() const;

    //! return E[X] 
    virtual void expectation(Vec expected_y) const;

    //! return Var[X]
    virtual Mat variance() const;
    virtual void variance(Vec diag_variances) const;

private:
    void build_();
    
public:
    virtual void build();

    virtual void forget();

    virtual int outputsize() const;

    //! The role of the train method is to bring the learner up to stage==nstages,
    //! updating the stats with training costs measured on-line in the process.
    virtual void train();

    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! This should be defined in subclasses to compute the weighted costs from
    //! already computed output. 
    //! NOTE: In exotic cases, the cost may also depend on some info in the input, 
    //! that's why the method also gets so see it.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;
                                
    //! Default calls computeOutput and computeCostsFromOutputs
    //! You may overload this if you have a more efficient way to 
    //! compute both output and weighted costs at the same time.
    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                       Vec& output, Vec& costs) const;

    //! Default calls computeOutputAndCosts
    //! This may be overloaded if there is a more efficient way to compute the costs
    //! directly, without computing the whole output vector.
    virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;
    
  
    //! This should return the names of the costs computed by computeCostsFromOutpus
    virtual TVec<string> getTestCostNames() const;

    //! This should return the names of the objective costs that the train method computes and 
    //! for which it updates the VecStatsCollector train_stats
    virtual TVec<string> getTrainCostNames() const;

    virtual int nTestCosts() const { return 2; }

    virtual int nTrainCosts() const { return 2; }

    //! returns the index of the given cost in the vector of testcosts (returns -1 if not found)
    int getTestCostIndex(const string& costname) const;

    //! returns the index of the given cost in the vector of traincosts (objectives) (returns -1 if not found)
    int getTrainCostIndex(const string& costname) const;

protected:
    static void declareOptions(OptionList& ol);

    // covariance = K + sigma^2 I
    // multiply (K+sigma^2 I)^{-1} by vector v, put result in Cinv_v
    // TRICK USING PRINCIPAL E-VECTORS OF K:
    //   Let C = sum_{i=1}^m lambda_i v_i v_i' + sigma I
    //   with v_i orthonormal eigenvectors. Then, it can also be written
    //       C = sum_{i=1}^m (lambda_i +sigma) v_i v_i' + sum_{i=m+1}^n sigma v_i v_i'
    //   whose inverse is simply
    //       inverse(C) = sum_{i=1}^m 1/(lambda_i +sigma) v_i v_i' + sum_{i=m+1}^n 1/sigma v_i v_i'
    //                  = sum_{i=1}^m (1/(lambda_i +sigma) - 1/sigma) v_i v_i' + 1/sigma I
    // set Cinv_v = inverse(C)*v, using given sigma in C
    void inverseCovTimesVec(real sigma, Vec v, Vec Cinv_v) const;
    // return u'*inverse(C)*u, using given sigma in C
    real QFormInverse(real sigma2, Vec u) const;

    //! to be used for hyper-parameter selection, this is the negative log-likelihood of the
    //! training data.
    real BayesianCost();

public:
    PLEARN_DECLARE_OBJECT(GaussianProcessRegressor);

};

DECLARE_OBJECT_PTR(GaussianProcessRegressor);

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
