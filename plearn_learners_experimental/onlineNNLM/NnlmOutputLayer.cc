// -*- C++ -*-

// NnlmOutputLayer.cc
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

/*! \file NnlmOutputLayer.cc */


#include "NnlmOutputLayer.h"


namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NnlmOutputLayer,
    "Implements the output layer for the Neural Network Language Model.",
    "MULTI-LINE \nHELP");


//////////////////////
// class wordAndProb
//////////////////////
//! Used to sort words according to probability
class wordAndProb {
public:
  wordAndProb(int wt, int p) : wordtag(wt), probability(p){};
  int wordtag;
  double probability;
};
bool wordAndProbGT(const wordAndProb &a, const wordAndProb &b) 
{
    return a.probability > b.probability;
}

////////////////////
// NnlmOutputLayer
////////////////////
NnlmOutputLayer::NnlmOutputLayer() :
    OnlineLearningModule(),
    target_cardinality( -1 ),
    context_cardinality( -1 ),
    sigma2min( 0.000001 ), // ### VERY IMPORTANT!!!
    dl_start_learning_rate( 0.0 ),
    dl_decrease_constant( 0.0 ),
    el_start_discount_rate( 0.99 ), // ### VERY IMPORTANT!!!
    el_decrease_constant( 0.0 ), // ### VERY IMPORTANT!!!
    step_number( 0 ),
    umc( 0.999999 ), // ###
    learning( LEARNING_DISCRIMINANT ),
    cost( COST_DISCR ),
    target( -1 ),
    the_real_target( -1 ),
    context( -1 ),
    s( 0.0 ),
    g_exponent( 0.0 ),
    g_det_covariance( 0.0 ),
    log_g_normalization( -REAL_MAX ),
    log_sum_p_ru( -REAL_MAX )
    //log_p_g_r( -REAL_MAX ),
    //sum_log_p_g_r( -REAL_MAX ),
{
    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

///////////////////
// declareOptions
///////////////////
void NnlmOutputLayer::declareOptions(OptionList& ol)
{
    // * Build Options *
    // * Build Options *
    declareOption(ol, "target_cardinality",
                  &NnlmOutputLayer::target_cardinality,
                  OptionBase::buildoption,
                  "Number of target tags.");
    declareOption(ol, "context_cardinality",
                  &NnlmOutputLayer::context_cardinality,
                  OptionBase::buildoption,
                  "Number of context tags (usually, there will be the additional 'missing' tag).");

    declareOption(ol, "sigma2min",
                  &NnlmOutputLayer::sigma2min,
                  OptionBase::buildoption,
                  "Minimal value for the diagonal of the covariance matrix.");

    declareOption(ol, "dl_start_learning_rate",
                  &NnlmOutputLayer::dl_start_learning_rate,
                  OptionBase::buildoption,
                  "Discriminant learning start learning rate.");
    declareOption(ol, "dl_decrease_constant",
                  &NnlmOutputLayer::dl_decrease_constant,
                  OptionBase::buildoption,
                  "Discriminant learning decrease constant.");

    declareOption(ol, "el_start_discount_rate",
                  &NnlmOutputLayer::el_start_discount_rate,
                  OptionBase::buildoption,
                  "Empirical learning discount-rate of old gaussian values when computing the new values.");
    declareOption(ol, "el_decrease_constant",
                  &NnlmOutputLayer::el_decrease_constant,
                  OptionBase::buildoption,
                  "Empirical learning decrease constant of gaussian parameters discount rate.");

// ### 'learning' a build option?

    // * Learnt Options *
    // * Learnt Options *
    declareOption(ol, "step_number", &NnlmOutputLayer::step_number,
                  OptionBase::learntoption,
                  "The step number, incremented after each update.");

    declareOption(ol, "umc", &NnlmOutputLayer::umc,
                  OptionBase::learntoption,
                  "The uniform mixture coefficient. p(r|i) = umc p_gauss + (1-umc) p_uniform");

    declareOption(ol, "pi", &NnlmOutputLayer::pi,
                  OptionBase::learntoption,
                  "pi[t] -> moyenne empirique de y==t" );
    declareOption(ol, "mu", &NnlmOutputLayer::mu,
                  OptionBase::learntoption,
                  "mu(t) -> moyenne empirique des r quand y==t" );
    declareOption(ol, "sigma2", &NnlmOutputLayer::sigma2,
                  OptionBase::learntoption,
                  "sigma2(t) -> variance empirique des r quand y==t" );

    declareOption(ol, "sumR", &NnlmOutputLayer::sumR,
                  OptionBase::learntoption,
                  "sumR(i) -> sum_t r_t 1_{y==i}" );
    declareOption(ol, "sumR2", &NnlmOutputLayer::sumR2,
                  OptionBase::learntoption,
                  "sumR2(i) -> sum_t r_t^2 1_{y==i}" );
    declareOption(ol, "sumI", &NnlmOutputLayer::sumI,
                  OptionBase::learntoption,
                  "sumI(i) -> sum_t 1_{y==i}" );
    declareOption(ol, "s_sumI", &NnlmOutputLayer::s_sumI,
                  OptionBase::learntoption,
                  "sum_t 1" );

    // ### test vars?


    // ### other?

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


//////////
//build_
//////////
void NnlmOutputLayer::build_()
{

    // *** Sanity checks ***
    if( input_size <= 0 )  {
        PLERROR("NnlmOutputLayer::build_: 'input_size' <= 0 (%i).\n"
                "You should set it to a positive integer.\n", input_size);
    }  else if( output_size != 1 )  {
        PLERROR("NnlmOutputLayer::build_: 'output_size'(=%i) != 1\n"
                  , output_size);
    }

    // *** Parameters not initialized ***
    if( mu.size() == 0 )   {
        resetParameters();
        resetClassCounts();
    }

}

//////////
// build
//////////
void NnlmOutputLayer::build()
{
    inherited::build();
    build_();
}


/////////////////////
// resetClassCounts
/////////////////////
void NnlmOutputLayer::resetClassCounts()
{
    s_sumI = 0;
    sumI.resize( target_cardinality );
    sumI.fill( 0 );
    // HACK in case no OOV in trainset
    sumI[0] = 1;
}

////////////////////////
// incrementClassCount
////////////////////////
void NnlmOutputLayer::incrementClassCount(int the_target)
{
#ifdef BOUNDCHECK
    if( the_target >= target_cardinality )  {
        PLERROR("NnlmOutputLayer::incrementCount:'the_target'(=%i) >= 'target_cardinality'(=%i)\n",
                   the_target, target_cardinality);
    }
#endif

    sumI[the_target] += 1;;
    s_sumI += 1;
}

////////////////
// applyCounts
////////////////
void NnlmOutputLayer::applyClassCounts()
{
    for( int tgt=0; tgt < target_cardinality; tgt++ )  {
        pi[tgt] = (real)sumI[ tgt ] / (real)s_sumI;
    }
}


////////////////////
// resetParameters
////////////////////
// NOTE doesn't reset the class counts
void NnlmOutputLayer::resetParameters()
{
    step_number = 0;
    umc = 0.999999; // ###

    pi.resize( target_cardinality );
    pi.fill( 0.0 );
    mu.resize( target_cardinality, input_size);
    mu.fill( 0.0 );
    sigma2.resize( target_cardinality, input_size);
    sigma2.fill( 0.0 );

    sumR.resize( target_cardinality, input_size);
    sumR.fill( 0.0 );
    sumR2.resize( target_cardinality, input_size);
    sumR2.fill( 0.0 );

    test_sumR.resize( target_cardinality, input_size);
    test_sumR.fill( 0.0 );
    test_sumR2.resize( target_cardinality, input_size);
    test_sumR2.fill( 0.0 );

    nd_gradient.resize( input_size );
    nd_gradient.fill( 0.0 );
    ad_gradient.resize( input_size );
    ad_gradient.fill( 0.0 );
    fd_gradient.resize( input_size );
    fd_gradient.fill( 0.0 );


    gradient_log_tmp.resize( input_size );
    gradient_log_tmp.fill( 0.0 );
    gradient_log_tmp_pos.resize( input_size );
    gradient_log_tmp_pos.fill( 0.0 );
    gradient_log_tmp_neg.resize( input_size );
    gradient_log_tmp_neg.fill( 0.0 );

    //log_p_g_r = safelog( 0.9 );
    //sum_log_p_g_r = -REAL_MAX;

    vec_log_p_rg_t.resize( target_cardinality );
    vec_log_p_r_t.resize( target_cardinality );
    vec_log_p_rt.resize( target_cardinality );
    beta.resize( target_cardinality, input_size );
    gamma.resize( target_cardinality, input_size );

}

//////////////
// setTarget
//////////////
void NnlmOutputLayer::setTarget(int the_target) const
{
#ifdef BOUNDCHECK
    if( the_target >= target_cardinality )  {
        PLERROR("NnlmOutputLayer::setTarget:'the_target'(=%i) >= 'target_cardinality'(=%i)\n",
                   the_target, target_cardinality);
    }
#endif

    target = the_target;
}

///////////////
// setContext
///////////////
void NnlmOutputLayer::setContext(int the_context) const
{
#ifdef BOUNDCHECK
    if( the_context >= context_cardinality )  {
        PLERROR("NnlmOutputLayer::setContext:'the_context'(=%i) >= 'context_cardinality'(=%i)\n"
                  , the_context, context_cardinality);
    }
#endif

    context = the_context;
}

////////////
// setCost
////////////
// Sets the cost computed in the fprop
void NnlmOutputLayer::setCost(int the_cost)
{
#ifdef BOUNDCHECK
    if( the_cost > 2 || the_cost < 0 )  {
        PLERROR("NnlmOutputLayer::setCost:'the_cost'(=%i) > '2' or < '0'\n"
                  , the_cost);
    }
#endif

    cost = the_cost;
}

////////////////////////////////
// makeDeepCopyFromShallowCopy
////////////////////////////////
void NnlmOutputLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(pi, copies);
    deepCopyField(mu, copies);
    deepCopyField(sigma2, copies);

    deepCopyField(sumI, copies);
    deepCopyField(sumR, copies);
    deepCopyField(sumR2, copies);

    deepCopyField(test_sumR, copies);
    deepCopyField(test_sumR2, copies);

//    deepCopyField(its_input, copies);
    deepCopyField(nd_gradient, copies);
    deepCopyField(ad_gradient, copies);
    deepCopyField(fd_gradient, copies);

    deepCopyField(gradient_log_tmp, copies);
    deepCopyField(gradient_log_tmp_pos, copies);
    deepCopyField(gradient_log_tmp_neg, copies);

    deepCopyField(vec_log_p_rg_t, copies);
    deepCopyField(vec_log_p_r_t, copies);
    deepCopyField(vec_log_p_rt, copies);

    deepCopyField(beta, copies);
    deepCopyField(gamma, copies);

}

//////////
// fprop
//////////
//! given the input, compute the output (possibly resize it  appropriately)
//! The output is then computed from p(r,c) = p(r|c) * p(c):
//!   - cost = DISCRIMINANT: output is NL of p(c|r) = p(r,c) / sum_{c'=0}^{target_cardinality} p(r,c')
//!   - cost = DISCRIMINANT APPROXIMATED: output is NL of p(c|r)_approx =  p(r,c) / sum_{c' \in Candidates} p(r,c')
//!   - cost = NON DISCRIMINANT: output is NL of p(r,c)
//! p(r|i) = umc * p_gaussian(r|c) + (1-umc) / 2^n
//!
void NnlmOutputLayer::fprop(const Vec& input, Vec& output) const
{

    the_real_target = target;

    // *** Non-discriminant cost: -log( p(r,t) ) ***
    if( cost == COST_NON_DISCR ) {
        compute_nl_p_rt( input, output );
    }
    // *** Approx-discriminant cost ***
    else if( cost == COST_APPROX_DISCR )  {
        compute_approx_nl_p_t_r( input, output );
    }
    // *** Discriminant cost: -log( p(t|r) ) ***
    else if( cost == COST_DISCR )  {
        compute_nl_p_t_r( input, output );
    }
    else  {
        PLERROR("NnlmOutputLayer::fprop - invalid cost\n");
    }

}

////////////////////
// compute_nl_p_rt
////////////////////
//! Computes -log( p(r,t) ) = -log( ( umc p_gaussian(r|t) + (1-umc) p_uniform(r|t) ) p(t) )
void NnlmOutputLayer::compute_nl_p_rt(const Vec& input, Vec& output) const
{

    // *** Sanity check ***
    int in_size = input.size();
    if( in_size != input_size ) {
        PLERROR("NnlmOutputLayer::compute_nl_p_rt: 'input.size()' should be equal\n"
                " to 'input_size' (%i != %i)\n", in_size, input_size);
    }

    // *** Compute gaussian's exponent - 'g' means gaussian ***
    // NOTE \Sigma is a diagonal matrix, ie det() = \Prod and inverse is 1/...

    g_exponent = 0.0;
    g_det_covariance = 1.0;

    for(int i=0; i<input_size; i++) {

      // s = r[i] - mu_t[i]
      s = input[i] - mu(target, i);

      // memorize this calculation for gradients computation
      beta(target, i) = s / sigma2(target, i);

      g_exponent += s * beta(target, i);

      // determinant of covariance matrix
      g_det_covariance *= sigma2(target, i);
    }

    g_exponent *= -0.5;

    // ### Should we use logs here?
    cout << "g_exponent " << g_exponent << " g_det_covariance " << g_det_covariance << endl;

#ifdef BOUNDCHECK
    if( isnan(g_exponent) || isnan(g_det_covariance) ) {
      PLERROR( "NnlmOutputLayer::compute_nl_p_rt - NAN present.\n" );
    }
#endif

    // * Compute normalizing factor
    log_g_normalization = - 0.5 * ( (input_size) * safelog(2.0 * Pi) + safelog(g_det_covariance) );

    // * Compute log p(r,g|t) = log( p(r|t,g) p(g) ) = log( umc p_gaussian(r|t) )
    vec_log_p_rg_t[target] = safelog(umc) + g_exponent + log_g_normalization;

    // * Compute log p(r|t) = log( umc p_g(r|t) + (1-umc) p_u(r|t) )
    vec_log_p_r_t[target] = logadd( vec_log_p_rg_t[target] , safelog(1.0-umc) - (input_size) * safelog(2.0));

    // * Compute log p(r,t)
    vec_log_p_rt[target] = safelog(pi[target]) + vec_log_p_r_t[target];

    // * Compute output
    output[0] = - vec_log_p_rt[target];

#ifdef BOUNDCHECK
    if( isnan(vec_log_p_rt[target]) ) {
      PLERROR( "NnlmOutputLayer::compute_nl_p_rt - NAN present.\n" );
    }
#endif

    // * Compute posterior for coeff_class_conditional_uniform_mixture evaluation in the bpropUpdate
    // p(generated by gaussian| r) = a p_g(r|i) / p(r|i)
    //log_p_g_r = safelog(umc) + g_exponent + log_g_normalization - log_p_r_i;

}

////////////////////
// compute_nl_p_t_r
////////////////////
//! Computes -log( p(t|r) ) = -log( p(r,t) / \Sum_u p(r,u) )
void NnlmOutputLayer::compute_nl_p_t_r(const Vec& input, Vec& output) const
{
    Vec nl_p_rt;
    Vec nl_p_ru;

    nl_p_rt.resize( 1 );
    nl_p_ru.resize( 1 );


    // * Compute numerator
    compute_nl_p_rt( input, nl_p_rt );

    // * Compute denominator
    // Normalize over whole vocabulary

    log_sum_p_ru = -REAL_MAX;

    for(int u=0; u<target_cardinality; u++)  {
        setTarget( u );
        compute_nl_p_rt( input, nl_p_ru );
        log_sum_p_ru = logadd(log_sum_p_ru, -nl_p_ru[0]);
    }

    output[0] = nl_p_rt[0] + log_sum_p_ru;

#ifdef BOUNDCHECK
    if( isnan(output[0]) ) {
      PLERROR( "NnlmOutputLayer::compute_nl_p_t_r - NAN present.\n" );
    }
#endif

}

//! May be called after compute_nl_p_t_r to find out which words get highest probability 
//! according to the model
void NnlmOutputLayer::getBestCandidates(Vec& candidate_tags, Vec& probabilities) const
{
		candidate_tags.resize(10);
		probabilities.resize(10);

    std::vector< wordAndProb > tmp;
		Vec nl_p_ru(1);

    for(int u=0; u<target_cardinality; u++)  {
        setTarget( u );
        compute_nl_p_rt( input, nl_p_ru );

        tmp.push_back( wordAndProb( u, safeexp( - (nl_p_ru[0] + log_sum_p_ru) ) );
    }

    std::sort(tmp.begin(), tmp.end(), wordAndFreqGT);

    // HACK we don't check if itr has hit the end... unlikely target_cardinality is smaller than 10
    std::vector< wordAndProb >::iterator itr_vec;
    itr_vec=tmp.begin();
    for(int i=0; i<10; i++) {
    		candidate_tags[i] = itr_vec->wordtag;
    		probabilities[i] = itr_vec->probability;
        itr_vec++;
    }

    tmp.clear();
}


////////////////////////////
// compute_approx_nl_p_t_r
////////////////////////////
//! Computes the approximate discriminant cost
void NnlmOutputLayer::compute_approx_nl_p_t_r(const Vec& input, Vec& output) const
{
    // *** Compute for the target ***
    Vec vec_nd_cost(1);
    compute_nl_p_rt(input, vec_nd_cost);

//nd_cost = -log_p_rt;

    // *** Compute for the normalization candidates ***
    Vec nl_p_ru;
    nl_p_ru.resize( 1 );
    log_sum_p_ru = vec_log_p_rt[the_real_target];
    int c;

    // shared candidates
    for( int i=0; i< shared_candidates.length(); i++ )
    {
        c = shared_candidates[i];
        if( c!=the_real_target )  {
            setTarget( c );
            compute_nl_p_rt( input, nl_p_ru );
            log_sum_p_ru = logadd(log_sum_p_ru, -nl_p_ru[0]);
        }
    }

    // context candidates 
    for( int i=0; i< candidates[ context ].length(); i++ )
    {
        c = candidates[ context ][i];
        if( c!=the_real_target )  {
            setTarget( c );
            compute_nl_p_rt( input, nl_p_ru );
            log_sum_p_ru = logadd(log_sum_p_ru, -nl_p_ru[0]);
        }
    }

    // *** The approximate discriminant cost ***
    output[0] = vec_nd_cost[0] + log_sum_p_ru;

#ifdef BOUNDCHECK
    if( isnan(output[0]) ) {
      PLERROR( "NnlmOutputLayer::compute_approx_nl_p_t_r - NAN present.\n" );
    }
#endif

}

//--------------------------------------------------------------------------------------------------------------------------------

///////////////////////////////////
// computeNonDiscriminantGradient
///////////////////////////////////
//! MUST be called after the corresponding fprop
void NnlmOutputLayer::computeNonDiscriminantGradient() const
{
    real tmp = safeexp( vec_log_p_rg_t[the_real_target] - vec_log_p_r_t[the_real_target] );

    for(int i=0; i<input_size; i++) {
        nd_gradient[i] = beta( the_real_target, i) * tmp;
    }
}


//////////////////////////////////////
// computeApproxDiscriminantGradient
/////////////////////////////////////
//! MUST be called after the corresponding fprop
void NnlmOutputLayer::computeApproxDiscriminantGradient() const
{
    gradient_log_tmp.fill(-REAL_MAX);
    gradient_log_tmp_pos.fill(-REAL_MAX);
    gradient_log_tmp_neg.fill(-REAL_MAX);

    // * Compute nd gradient
    computeNonDiscriminantGradient();

    // * Compute ad specific term
    int c;

    // target
    addCandidateContribution( the_real_target );

    // shared candidates
    for( int i=0; i< shared_candidates.length(); i++ )
    {
        c = shared_candidates[i];
        if( c != the_real_target )
            addCandidateContribution( c );
    }

    // context candidates 
    for( int i=0; i< candidates[ context ].length(); i++ )
    {
        c = candidates[ context ][i];
        if( c != the_real_target )
            addCandidateContribution( c );
    }


    // *** The corresponding approx gradient ***
    for(int j=0; j<input_size; j++) {
        if( gradient_log_tmp_pos[j] > gradient_log_tmp_neg[j] ) {
            gradient_log_tmp[j] = logsub( gradient_log_tmp_pos[j], gradient_log_tmp_neg[j] );
            ad_gradient[j] = nd_gradient[j] - safeexp( gradient_log_tmp[j] - log_sum_p_ru);
        } else  {
            gradient_log_tmp[j] = logsub( gradient_log_tmp_neg[j], gradient_log_tmp_pos[j] );
            ad_gradient[j] = nd_gradient[j] + safeexp( gradient_log_tmp[j] - log_sum_p_ru);
        }
    }

}

////////////////////////////////
// computeDiscriminantGradient
////////////////////////////////
//! MUST be called after the corresponding fprop
void NnlmOutputLayer::computeDiscriminantGradient() const
{
    gradient_log_tmp.fill(-REAL_MAX);
    gradient_log_tmp_pos.fill(-REAL_MAX);
    gradient_log_tmp_neg.fill(-REAL_MAX);

    // * Compute nd gradient
    computeNonDiscriminantGradient();

    // * Compute ad specific term
    for( int u=0; u< target_cardinality; u++ )
    {
        addCandidateContribution( u );
    }


    // *** The corresponding approx gradient ***
    for(int j=0; j<input_size; j++) {
        if( gradient_log_tmp_pos[j] > gradient_log_tmp_neg[j] ) {
            gradient_log_tmp[j] = logsub( gradient_log_tmp_pos[j], gradient_log_tmp_neg[j] );
            fd_gradient[j] = nd_gradient[j] - safeexp( gradient_log_tmp[j] - log_sum_p_ru);
        } else  {
            gradient_log_tmp[j] = logsub( gradient_log_tmp_neg[j], gradient_log_tmp_pos[j] );
            fd_gradient[j] = nd_gradient[j] + safeexp( gradient_log_tmp[j] - log_sum_p_ru);
        }
    }

}

/////////////////////////////
// addCandidateContribution
/////////////////////////////
void NnlmOutputLayer::addCandidateContribution( int c ) const
{
    for(int i=0; i<input_size; i++) {
        if( beta(c,i) > 0)  {
            gradient_log_tmp_pos[i] = logadd( gradient_log_tmp_pos[i], 
                    vec_log_p_rg_t[c] + safelog( beta(c,i) ) +  safelog( pi[c] ) );
        } else  {
            gradient_log_tmp_neg[i] = logadd( gradient_log_tmp_neg[i], 
                    vec_log_p_rg_t[c] + safelog( -beta(c,i) ) +  safelog( pi[c] ) );
        }

        #ifdef BOUNDCHECK
        if( isnan(gradient_log_tmp_pos[i]) || isnan(gradient_log_tmp_neg[i]) ) {
          PLERROR("NnlmOutputLayer::computeApproxDiscriminantGradient - gradient_log_tmp_pos or gradient_log_tmp_neg is NAN.\n");
        }
        #endif
    }
}


//! Adapt based on the output gradient: this method should only
//! be called just after a corresponding fprop; it should be
//! called with the same arguments as fprop for the first two arguments
//! (and output should not have been modified since then).
//! Since sub-classes are supposed to learn ONLINE, the object
//! is 'ready-to-be-used' just after any bpropUpdate.
//! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
//! JUST CALLS
//!     bpropUpdate(input, output, input_gradient, output_gradient)
//! AND IGNORES INPUT GRADIENT.
/*void NnlmOutputLayer::bpropUpdate(const Vec& input, const Vec& output,
                               const Vec& output_gradient)
{
}*/

//////////////////
// resetTestVars
//////////////////
void NnlmOutputLayer::resetTestVars() {

    // *TEST*
    test_sumR.fill( 0 );
    test_sumR2.fill( 0 );

}

///////////////////
// updateTestVars
///////////////////
void NnlmOutputLayer::updateTestVars(const Vec& input)
{
    // * Update counts
    for(int i=0; i<input_size; i++) {
      test_sumR( target, i ) += input[i];
      test_sumR2( target, i ) += input[i]*input[i];
    }
}

//////////////////
// applyTestVars
//////////////////
void NnlmOutputLayer::applyTestVars()
{
    if( sumI[ target ] == 0 )  {
        PLERROR("NnlmOutputLayer::applyTestVars - sumI[ %i ] == 0\n", target);
    }

    for(int i=0; i<input_size; i++) {


        mu( target, i ) = test_sumR( target, i ) / (real) sumI[ target ];

        sigma2( target, i ) = (  mu(target, i) * mu(target, i) + 
                ( test_sumR2(target, i) -2.0 * mu(target, i) * test_sumR(target, i)  ) / sumI[ target ] );

        if(sigma2( target, i )<sigma2min) {
            sigma2( target, i ) = sigma2min;
        }

    }

    pi[target] = (real)sumI[ target ] / (real)s_sumI;

}


//! this version allows to obtain the input gradient as well
//! N.B. THE DEFAULT IMPLEMENTATION IN SUPER-CLASS JUST RAISES A PLERROR.
void NnlmOutputLayer::bpropUpdate(const Vec& input, const Vec& output,
                               Vec& input_gradient,
                               const Vec& output_gradient)
{

    int in_size = input.size();
    int out_size = output.size();
    int og_size = output_gradient.size();

    // *** Sanity checks
    if( in_size != input_size ) {
        PLERROR("NnlmOutputLayer::bpropUpdate:'input.size()' should be equal\n"
                " to 'input_size' (%i != %i)\n", in_size, input_size);
    }  else if( out_size != output_size )  {
        PLERROR("NnlmOutputLayer::bpropUpdate:'output.size()' should be"
                " equal\n"
                " to 'output_size' (%i != %i)\n", out_size, output_size);
    }  else if( og_size != output_size )  {
        PLERROR("NnlmOutputLayer::bpropUpdate:'output_gradient.size()'"
                " should\n"
                " be equal to 'output_size' (%i != %i)\n",
                og_size, output_size);
    }


    // *** Compute input_gradient ***
    // *** Compute input_gradient ***

    if( cost == COST_NON_DISCR ) {
        computeNonDiscriminantGradient();
        input_gradient << nd_gradient;
    }
    else if( cost == COST_APPROX_DISCR )  {
        computeApproxDiscriminantGradient();
        input_gradient << ad_gradient;
    }

    else if( cost == COST_DISCR )  {
        computeDiscriminantGradient();
        input_gradient << fd_gradient;
    }
    else  {
        PLERROR("NnlmOutputLayer::bpropUpdate - invalid cost\n");
    }


    // *** Discriminant learning of mu and sigma ***
    // *** Discriminant learning of mu and sigma ***

    if( learning == LEARNING_DISCRIMINANT )  {
        applyMuGradient();
        applySigmaGradient();
    }


    // *** Empirical learning of mu and sigma ***
    // *** Empirical learning of mu and sigma ***

    else if( learning == LEARNING_EMPIRICAL )  {

        // mu(i) -> moyenne empirique des x quand y=i
        // sigma2(i) -> variance empirique des x quand y=i

        // TODO I tend to think this update should be done before computing the cost, in the fprop,
        // since this non discriminant learning procedure does not require computation of the cost

        // TODO is this any good? If so, at least change variable names
        el_dr = el_start_discount_rate + step_number * el_decrease_constant;

        // * Update parameters - using discount
        // discount * ancien + (1-discount) * nouveau
        for(int i=0; i<input_size; i++) {
            // ###
            //mu( target, i ) = sumR( target, i ) / sumI[ target ];
            mu( target, i ) = el_dr * mu( target, i ) + (1.0 - el_dr) * input[i];

            // ### hum... 
            // sigma2( target, i ) = (  mu(target, i) * mu(target, i) + ( sumR2(target, i) -2.0 * mu(target, i) * sumR(target, i) ) / 
            //                        sumI[ target ] );
            sigma2( target, i ) = el_dr * (  mu(target, i) * mu(target, i) + ( sumR2(target, i) -2.0 * mu(target, i) * sumR(target, i)        ) / sumI[ target ] )
                + (1.0-el_dr) * input[i]*input[i];

            // Enforce minimal sigma
            if(sigma2( target, i )<sigma2min) {
                sigma2( target, i ) = sigma2min;
            }

            if( isnan( sigma2( target, i ) ) ) {
              PLERROR( "NnlmOutputLayer::bpropUpdate - isnan( sigma2( target, i ) )!\n" );
            }
        }

        // * Update counts
        for(int i=0; i<input_size; i++) {
          sumR( target, i ) += input[i];
          sumR2( target, i ) += input[i]*input[i];
        }

        // Update uniform mixture coefficient
        //sum_log_p_g_r = logadd( sum_log_p_g_r, log_p_g_r );
        //umc = safeexp( sum_log_p_g_r ) / s_sumI;
    }

    else  {
        PLERROR( "NnlmOutputLayer::bpropUpdate - invalid 'learning' value.\n");
    }

}

////////////////////
// applyMuGradient
////////////////////
//! MUST be called after the corresponding fprop
//! Computes gradients of the non discriminant cost with respect to mu and sigma
void NnlmOutputLayer::applyMuGradient() const
{
    dl_lr = dl_start_learning_rate / ( 1.0 + dl_decrease_constant * step_number);


    if( cost == COST_NON_DISCR ) {
        Vec mu_gradient( input_size );
        mu_gradient << nd_gradient;
        for( int i=0; i<input_size; i++ ) {
            mu_gradient[i] = - mu_gradient[i];
            mu(the_real_target,i) -= dl_lr * mu_gradient[i];
        }
    }


    else if( cost == COST_APPROX_DISCR )  {

        // for the target
        applyMuTargetGradient();

        // --- for the others ---
        int c;
        // shared candidates
        for( int i=0; i< shared_candidates.length(); i++ )
        {
            c = shared_candidates[i];
            if( c != the_real_target )  {
                applyMuCandidateGradient(c);
            }
        }

        // context candidates 
        for( int i=0; i< candidates[ context ].length(); i++ )
        {
            c = candidates[ context ][i];
            if( c != the_real_target )  {
                applyMuCandidateGradient(c);
            }
        }

    }


    else if( cost == COST_DISCR )  {
        applyMuTargetGradient();
        for( int u=0; u< target_cardinality; u++ )  {
            if( u != the_real_target )  {
                applyMuCandidateGradient(u);
            }
        }
    }
    else  {
        PLERROR("NnlmOutputLayer::applyMuGradient - invalid cost\n");
    }

}
////////////////////
// applyMuGradient
////////////////////
//! MUST be called after the corresponding fprop
//! 
void NnlmOutputLayer::applyMuTargetGradient() const
{

    Vec mu_gradient( input_size );
    mu_gradient << nd_gradient;
    for( int i=0; i<input_size; i++ ) {
        mu_gradient[i] = - mu_gradient[i];

        if( beta(the_real_target,i) > 0.0 ) {
            mu_gradient[i] += safeexp( 
                safelog( pi[the_real_target] ) + vec_log_p_rg_t[the_real_target] + safelog( beta(the_real_target,i) ) - log_sum_p_ru );
        } else  {
            mu_gradient[i] -= safeexp( 
                safelog( pi[the_real_target] ) + vec_log_p_rg_t[the_real_target] + safelog( -beta(the_real_target,i) ) - log_sum_p_ru );
        }

        mu(the_real_target,i) -= dl_lr * mu_gradient[i];
    }

}
////////////////////
// applyMuGradient
////////////////////
//! MUST be called after the corresponding fprop
//! 
void NnlmOutputLayer::applyMuCandidateGradient(int c) const
{
    Vec mu_gradient(input_size);

    for( int i=0; i<input_size; i++ ) {
        if( beta(c,i) > 0.0 ) {
            mu_gradient[i] = safeexp( 
                safelog( pi[c] ) + vec_log_p_rg_t[c] + safelog( beta(c,i) ) - log_sum_p_ru );
        } else  {
            mu_gradient[i] = - safeexp( 
                safelog( pi[c] ) + vec_log_p_rg_t[c] + safelog( -beta(c,i) ) - log_sum_p_ru );
        }
        mu(c,i) -= dl_lr * mu_gradient[i];
    }

}

///////////////////////
// applySigmaGradient
///////////////////////
void NnlmOutputLayer::applySigmaGradient() const
{

    Vec sigma2_gradient( input_size );


    if( cost == COST_NON_DISCR ) {

        real tmp = -0.5 * safeexp( vec_log_p_rg_t[ the_real_target ] - vec_log_p_r_t[ the_real_target ] );

        for( int i=0; i<input_size; i++ ) {
            sigma2_gradient[i] = tmp * ( beta(the_real_target,i) * beta(the_real_target,i) - 1.0/sigma2(the_real_target,i) );
            sigma2(the_real_target,i) -= dl_lr * sigma2_gradient[i];
        }

    }


    else if( cost == COST_APPROX_DISCR )  {
        applySigmaTargetGradient();

        // --- for the others ---
        int c;
        // shared candidates
        for( int i=0; i< shared_candidates.length(); i++ )
        {
            c = shared_candidates[i];
            if( c != the_real_target )  {
                applySigmaCandidateGradient(c);
            }
        }

        // context candidates 
        for( int i=0; i< candidates[ context ].length(); i++ )
        {
            c = candidates[ context ][i];
            if( c != the_real_target )  {
                applySigmaCandidateGradient(c);
            }
        }

    }


    else if( cost == COST_DISCR )  {
        applySigmaTargetGradient();
        for( int u=0; u< target_cardinality; u++ )  {
            if( u != the_real_target )  {
                applySigmaCandidateGradient(u);
            }
        }
    }
    else  {
        PLERROR("NnlmOutputLayer::applySigmaGradient - invalid cost\n");
    }


}
void NnlmOutputLayer::applySigmaTargetGradient() const
{
    Vec sigma2_gradient( input_size );

    real tmp = -0.5 * safeexp( vec_log_p_rg_t[ the_real_target ] - vec_log_p_r_t[ the_real_target ] );
    real tmp2 = 0.5 * pi[the_real_target] * safeexp( vec_log_p_rg_t[ the_real_target ] - log_sum_p_ru );
    real tmp3;

    for( int i=0; i<input_size; i++ ) {
        tmp3 = beta(the_real_target,i) * beta(the_real_target,i) - 1.0/sigma2(the_real_target,i);
        sigma2_gradient[i] = tmp * tmp3;
        sigma2_gradient[i] += tmp2 * tmp3;
        sigma2(the_real_target,i) -= dl_lr * sigma2_gradient[i];
    }
}


void NnlmOutputLayer::applySigmaCandidateGradient(int c) const
{
    Vec sigma2_gradient( input_size );

    real tmp2 = 0.5 * pi[c] * safeexp( vec_log_p_rg_t[ c ] - log_sum_p_ru );
    real tmp3;

    for( int i=0; i<input_size; i++ ) {
        tmp3 = beta(c,i) * beta(c,i) - 1.0/sigma2(c,i);
        sigma2_gradient[i] = tmp2 * tmp3;
        sigma2(c,i) -= dl_lr * sigma2_gradient[i];
    }

}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void NnlmOutputLayer::forget()
{
    cout << "NnlmOutputLayer::forget()" << endl;
    resetParameters();
}

/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void NnlmOutputLayer::finalize()
{
}
*/

/* THIS METHOD IS OPTIONAL
//! in case bpropUpdate does not do anything, make it known
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS RETURNS false;
bool NnlmOutputLayer::bpropDoesNothing()
{
}
*/


//! Similar to bpropUpdate, but adapt based also on the estimation
//! of the diagonal of the Hessian matrix, and propagates this
//! back. If these methods are defined, you can use them INSTEAD of
//! bpropUpdate(...)
//! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
//! JUST CALLS
//!     bbpropUpdate(input, output, input_gradient, output_gradient,
//!                  in_hess, out_hess)
//! AND IGNORES INPUT HESSIAN AND INPUT GRADIENT.
/*void NnlmOutputLayer::bbpropUpdate(const Vec& input, const Vec& output,
                                const Vec& output_gradient,
                                const Vec& output_diag_hessian)
{
}*/


/* THIS METHOD IS OPTIONAL
//! Similar to bpropUpdate, but adapt based also on the estimation
//! of the diagonal of the Hessian matrix, and propagates this
//! back. If these methods are defined, you can use them INSTEAD of
//! bpropUpdate(...)
//! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
//! RAISES A PLERROR.
void NnlmOutputLayer::bbpropUpdate(const Vec& input, const Vec& output,
                                Vec& input_gradient,
                                const Vec& output_gradient,
                                Vec& input_diag_hessian,
                                const Vec& output_diag_hessian)
{
}
*/



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
