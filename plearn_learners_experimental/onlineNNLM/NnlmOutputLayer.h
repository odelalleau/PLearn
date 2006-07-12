// -*- C++ -*-

// NnlmOutputLayer.h
//
// Copyright (C) 2006 Pierre-Antoine Manzagol
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

// Authors: Pierre-Antoine Manzagol

/*! \file NnlmOutputLayer.h */


#ifndef NnlmOutputLayer_INC
#define NnlmOutputLayer_INC

#include <plearn/base/Object.h>
#include <plearn/math/TMat_maths.h>
#include <plearn_learners/online/OnlineLearningModule.h>

namespace PLearn {

/**
 * Implements an output layer for the Neural Network Language Model. 
 *
 * Given 'r' the output of the previous layer (the representation of the input),
 * and 't' the target class, this module models p(r|t) as a mixture between a gaussian
 * model and a uniform: p(r|t) = umc * p_g(r|t) + (1-umc) p_u(r|t).
 * We have p_u(r|c) = 1.0 / 2^input_size
 * 
 * The output is then computed from p(r,t) = p(r|t) * p(t):
 *   - cost = DISCRIMINANT: output is NL of p(t|r) = p(r,t) / sum_{u=0}^{target_cardinality} p(r,u)
 *   - cost = DISCRIMINANT APPROXIMATED: output is NL of p(t|r)_approx =  p(r,t) / sum_{u' \in Candidates} p(r,u')
 *   - cost = NON DISCRIMINANT: output is NL of p(r,t)
 *
 * Learning of \mu_u and \Sigma_u can be:
 *   - EMPIRICAL
 *   - DISCRIMINANT
 *
 *
 * @todo 
 * @deprecated 
 * 
 */
class NnlmOutputLayer : public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

    //! specifies the range of the values of 'target'
    int target_cardinality;
    //! specifies the range of the values of 'context' (ex: + 'missing' tag)
    int context_cardinality;

    //! minimal value \sigma^2 can have
    real sigma2min;

    //! Discriminant learning - dl
    real dl_start_learning_rate;
    real dl_decrease_constant;

    //! Empirical learning - el
    //! discounts the gaussians' old parameters in the computation of the new ones
    real el_start_discount_rate;
    real el_decrease_constant;


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    NnlmOutputLayer();

    //! Sets t, the target
    void setTarget(int the_target) const;
    //! Sets the context. The Candidates set of the approximated discriminant cost is determined from the context
    void setContext(int the_context) const;
    //! Sets the cost used in the fprop()
    void setCost(int the_cost);

    //! Used to evaluate class counts
    void resetClassCounts();
    void incrementClassCount(int the_target);
    void applyClassCounts();  // computes the pi[]

    //! Used for a fresh evaluation of mu and sigma
    //! Manipulate sumR and sumR2
    void resetTestVars();
    void updateTestVars(const Vec& input);
    void applyTestVars();

    //! Resizes variables and sets pretty much everything back to a 'zero' value
    void resetParameters();

    //! Computes -log( p(r,t) )
    void compute_nl_p_rt(const Vec& input, Vec& output) const;

    //! Computes the approximation -log( p(t|r) ) using only some candidates for normalization
    void compute_approx_nl_p_t_r(const Vec& input, Vec& output) const;

    //! Computes -log( p(t|r) )
    void compute_nl_p_t_r(const Vec& input, Vec& output) const;
    void getBestCandidates(Vec& candidate_tags, Vec& probabilities) const;

		//! Compute gradients of different costs with respect to input
    void computeNonDiscriminantGradient() const;
    void computeApproxDiscriminantGradient() const;
    void computeDiscriminantGradient() const;
    void addCandidateContribution( int c ) const;
    
		//! Compute and apply gradients of different costs with respect to mus
    void applyMuGradient() const;
    void applyMuTargetGradient() const;
    void applyMuCandidateGradient(int c) const;

		//! Compute and apply gradients of different costs with respect to sigmas
    void applySigmaGradient() const;
    void applySigmaTargetGradient() const;
    void applySigmaCandidateGradient(int c) const;

    //! Computes 'cost' for the 'target'
    //! given the input, compute the output (possibly resize it  appropriately)
    virtual void fprop(const Vec& input, Vec& output) const;

    //! Adapt based on the output gradient: this method should only
    //! be called just after a corresponding fprop; it should be
    //! called with the same arguments as fprop for the first two arguments
    //! (and output should not have been modified since then).
    //! Since sub-classes are supposed to learn ONLINE, the object
    //! is 'ready-to-be-used' just after any bpropUpdate.
    /*virtual void bpropUpdate(const Vec& input, const Vec& output,
                             const Vec& output_gradient);*/

    //! NOT IMPLEMENTED - GRADIENT COMPUTED IN NnlmOnlineLearner
    //! And I'm not sure why... TODO find out
    //! this version allows to obtain the input gradient as well
    //! N.B. THE DEFAULT IMPLEMENTATION IN SUPER-CLASS JUST RAISES A PLERROR.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                              Vec& input_gradient,
                              const Vec& output_gradient);

    //! reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();

    //! optionally perform some processing after training, or after a
    //! series of fprop/bpropUpdate calls to prepare the model for truly
    //! out-of-sample operation.  THE DEFAULT IMPLEMENTATION PROVIDED IN
    //! THE SUPER-CLASS DOES NOT DO ANYTHING.
    // virtual void finalize();


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(NnlmOutputLayer);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################


public:
    //#####  Public NOT Options  ##############################################

    //! keeps track of updates
    int step_number;

    //! We use a mixture with a uniform to prevent negligeable probabilities
    //! which cause gradient explosions. 
    //! Should be learned as mean of p(g|r)
    //! (probability that gaussian is responsible for observation, given r)
    //! uniform mixture coefficient
    real umc;

    //! pi(i) = empirical mean when of c==i, ie p(c)
    Vec pi;

    //! Gaussian parameters - p_g(r|c)
    Mat mu;
    Mat sigma2;

    //! EMPIRICAL LEARNING
    //! Intermediaries
    int s_sumI;  // sum_t 1
    TVec<int> sumI;     // sumI(i) -> sum_t 1_{c==i}

    Mat sumR;     // sumR(i) -> sum_t r_t 1_{c==i}
    Mat sumR2;    // sumR2(i) -> sum_t r_t^2 1_{c==i}

    Mat test_sumR;
    Mat test_sumR2;

    // TODO THIS COULD BE A LEARNT OPTION
    //! Holds candidates
    TVec<int> shared_candidates;    // frequent (ie paying) words
    TVec< TVec<int> > candidates;   // context specific candidates

    // for learning umc
    //mutable real log_p_g_r;
    //mutable real sum_log_p_g_r;

    //#####  Don't need to be saved  ##########################################

    enum{COST_DISCR=0, COST_APPROX_DISCR=1, COST_NON_DISCR=2};  // ### Watchout... also defined in NnlmOnlineLearner.
    enum{LEARNING_DISCRIMINANT=0, LEARNING_EMPIRICAL=1};        // Granted, this is not good.

    // Specifies learning procedure
    int learning;

    //!Must be set before calling fprop
    //{
        //! the cost
        int cost;

        //! the current word -> we use its parameters to compute output
        mutable int target;
        mutable int the_real_target;
        mutable int context;
    //}

    //##### Intermediates ######################################################

    mutable real s;
    mutable real g_exponent;
    mutable real g_det_covariance;
    mutable real log_g_normalization;

    mutable Vec vec_log_p_rg_t;
    mutable Vec vec_log_p_r_t;
    mutable Vec vec_log_p_rt;
    mutable real log_sum_p_ru;

    // holds \Sigma^-1 (r-\mu)
    mutable Mat beta;

    // holds pi[] * p_rg_t * \Sigma^-1 (r-\mu)
    //mutable Mat gamma;

    mutable Vec nd_gradient;
    mutable Vec ad_gradient;
    mutable Vec fd_gradient;

    mutable Vec gradient_log_tmp;
    mutable Vec gradient_log_tmp_pos;
    mutable Vec gradient_log_tmp_neg;

    //! The original way of computing the mus and sigmas (ex. mu memorize \sum r
    //! and then divide) had the effect learning slowed down with time.
    //! We use this discount rate now.
    //! TODO validate computation of mus and sigmas
    //! gaussian_learning_discount_rate
    mutable real el_dr;
    mutable real dl_lr;

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(NnlmOutputLayer);

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
