// -*- C++ -*-

// OnlineGramNaturalGradientOptimizer.cc
//
// Copyright (C) 2007 Pierre-Antoine Manzagol
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

/*! \file OnlineGramNaturalGradientOptimizer.cc */


#define PL_LOG_MODULE_NAME "OnlineGramNaturalGradientOptimizer"

#include "OnlineGramNaturalGradientOptimizer.h"
#include <plearn/io/pl_log.h>
#include <plearn/math/TMat_maths.h>
#include <plearn/display/DisplayUtils.h>
#include <plearn/var/SumOfVariable.h>

#include <plearn/math/plapack.h>



namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    OnlineGramNaturalGradientOptimizer,
    "Optimization by Schraudolph's stochastic meta descent (SMD).", 
    "OnlineGramNaturalGradientOptimizer is \n"
    "blabla \n"
    "\n"
);

OnlineGramNaturalGradientOptimizer::OnlineGramNaturalGradientOptimizer():
    learning_rate(0.01),
    gamma(1.0),
    reg(1e-6),
    opt_batch_size(1),
    n_eigen(6)
{}


void OnlineGramNaturalGradientOptimizer::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "learning_rate", &OnlineGramNaturalGradientOptimizer::learning_rate,
        OptionBase::buildoption, 
        "Learning rate used in the natural gradient descent.\n");
    declareOption(
        ol, "gamma", &OnlineGramNaturalGradientOptimizer::gamma,
        OptionBase::buildoption, 
        "Discount factor used in the update of the estimate of the gradient covariance.\n");
    declareOption(
        ol, "reg", &OnlineGramNaturalGradientOptimizer::reg,
        OptionBase::buildoption, 
        "Regularizer used in computing the natural gradient, C^{-1} mu. Added to C^{-1} diagonal.\n");
    declareOption(
        ol, "opt_batch_size", &OnlineGramNaturalGradientOptimizer::opt_batch_size,
        OptionBase::buildoption, 
        "Size of the optimizer's batches (examples before parameter and gradient covariance updates).\n");
    declareOption(
        ol, "n_eigen", &OnlineGramNaturalGradientOptimizer::n_eigen,
        OptionBase::buildoption, 
        "The number of eigen vectors to model the gradient covariance matrix\n");

    inherited::declareOptions(ol);
}

void OnlineGramNaturalGradientOptimizer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{ 
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(gradients, copies);
    deepCopyField(mu, copies);
    deepCopyField(gram, copies);
    deepCopyField(U, copies);
    deepCopyField(D, copies);
    deepCopyField(cov_eigen_vec, copies);
    deepCopyField(cov_eigen_val, copies);
    deepCopyField(cov_norm_eigen_vec, copies);
    deepCopyField(dot_prod, copies);
    deepCopyField(scaled_dot_prod, copies);
    deepCopyField(naturalg, copies);

}

void OnlineGramNaturalGradientOptimizer::build_()
{
    n_optimizeN_calls=0;
    n_eigen_cur = 0;
    n_eigen_old = 0;

    total_variance = 0.0;
    variance_percentage = 0.;

    int n = params.nelems();

    cout << "Number of parameters: " << n << endl;

    if (n > 0) {
        gradients.resize( opt_batch_size, n );
        gradients.clear();
        mu.resize(n);
        mu.clear();
        naturalg.resize(n);
        naturalg.clear();
        // other variables will have different lengths
        // depending on the current number of eigen vectors
    }
}

// 'stage' is to be interpreted as "the number of examples to use
// in batches of size 'batch_size' "
// Note that a batch could be spread over two epochs
bool OnlineGramNaturalGradientOptimizer::optimizeN(VecStatsCollector& stats_coll) 
{
    n_optimizeN_calls++;

    if( nstages%opt_batch_size != 0 )   {
        PLWARNING("OnlineGramNaturalGradientOptimizer::optimizeN(...) - nstages%opt_batch_size != 0");
    }

    int stage_max = stage + nstages; // the stage to reach

    PP<ProgressBar> pb;
    pb = new ProgressBar("Training " + classname() + " from stage " 
                + tostring(stage) + " to " + tostring(stage_max), (int)(stage_max-stage)/opt_batch_size );

    int initial_stage = stage;
    while( stage < stage_max )    {

        /*if( bi == 0 )
            t0 = clock();*/

        // Get the new gradient and append it
        params.clearGradient();
        proppath.clearGradient();
        cost->gradient[0] = -1.0;
        proppath.fbprop();
        params.copyGradientTo( gradients(bi) );

        // End of batch. Compute natural gradient and update parameters.
        bi++;
        if( bi == opt_batch_size )  {
            //t1 = clock();

            bi = 0;
            gramEigenNaturalGradient();

            //t2 = clock();

            // set params += -learning_rate * params.gradient
            naturalg *= learning_rate;
            params.copyGradientFrom( naturalg );
            params.updateAndClear();

            //t3 = clock();

            //cout << double(t1-t0) << " " << double(t2-t1) << " " << double(t3-t2) << endl;

            if(pb)
                pb->update((stage-initial_stage)/opt_batch_size);

        }

        stats_coll.update(cost->value);
        stage++;
    }

    return false;
}


void OnlineGramNaturalGradientOptimizer::gramEigenNaturalGradient()
{
    // We don't have any eigen vectors yet
    if( n_eigen_cur == 0 )  {

        // The number of eigen vectors we will have after incorporating the new data
        // (the gram matrix of gradients might have a rank smaller than n_eigen)
        n_eigen_cur = min( gradients.length(), n_eigen);

        // Compute the total variance - to do this, compute the trace of the covariance matrix
        // could also use the trace of the gram matrix since we compute it, ie sum(diag(gram))
/*        for( int i=0; i<gradients.length(); i++)   {
            Vec v = gradients(i);
            total_variance += sumsquare(v);
        }
        total_variance /= gradients.length();*/

        // Compute the gram matrix - TODO does this recognize gram is symetric? (and save the computations?)
        gram.resize( gradients.length(), gradients.length() );
        productTranspose(gram, gradients, gradients);
        gram /= gradients.length();

        // Extract eigenvectors/eigenvalues - destroys the content of gram, D and U are resized
        // gram = U D U' (if we took all values)
        eigenVecOfSymmMat(gram, n_eigen_cur, D, U);

        // Percentage of the variance we keep is the sum of the kept eigenvalues divided
        // by the total variance.
        //variance_percentage = sum(D)/total_variance;

	// The eigenvectors V of C are deduced from the eigenvectors U of G by the
	// formula V = AUD^{-1/2} (D the eigenvalues of G).  The nonzero eigenvalues of
	// C and D are the same.

	// The true eigenvalues are norm_eigen_vec. However, we shall keep in memory
	// the eigenvectors of C rescaled by the square root of their associated
	// eigenvalues, so that C can be written VV' instead of VDV'. Thus, the "new" V
	// is equal to VD^{1/2} = AU.
        // We have row vectors so AU = (U'A')'

        cov_eigen_vec.resize(n_eigen_cur, gradients.width() );
        product( cov_eigen_vec, U, gradients );
        cov_eigen_vec /= sqrt( gradients.length() );
        cov_eigen_val.resize( D.length() );
        cov_eigen_val << D;

        ofstream fd_eigval("eigen_vals.txt", ios_base::app);
        fd_eigval << cov_eigen_val << endl;
        fd_eigval.close();

        cov_norm_eigen_vec.resize( n_eigen_cur, gradients.width() );
        for( int i=0; i<n_eigen_cur; i++)   {
            Vec v = cov_norm_eigen_vec(i);
            divide( cov_eigen_vec(i), sqrt(D[i]), v );
        }

    }

    // We already have some eigen vectors, so it's an update
    else    {

        // The number of eigen vectors we will have after incorporating the new data
        n_eigen_old = cov_eigen_vec.length();
        n_eigen_cur = min( cov_eigen_vec.length() + gradients.length(), n_eigen);

        // Update the total variance, by computing that of the covariance matrix
        // total_variance = gamma*total_variance + (1-gamma)*sum(sum(A.^2))/n_new_vec
        /*total_variance *= gamma;
        for( int i=0; i<gradients.length(); i++)   {
            Vec v = gradients(i);
            total_variance += (1.-gamma) * sumsquare(v) / gradients.length();
        }*/

        // Compute the gram matrix
	// To find the equivalence between the covariance matrix and the Gram matrix,
	// we need to have the covariance matrix under the form C = UU' + AA'. However,
	// what we have is C = gamma UU' + (1-gamma)AA'/n_new_vec. Thus, we will
	// rescale U and A using U = sqrt(gamma) U and A = sqrt((1 - gamma)/n_new_vec)
	// A. Now, the Gram matrix is of the form [U'U U'A;A'U A'A] using the new U and
	// A.

        gram.resize( n_eigen_old + gradients.length(), n_eigen_old + gradients.length() );

        Mat m = gram.subMat(0, 0, n_eigen_old, n_eigen_old);
        m.clear();
        addToDiagonal(m, gamma*D);

        // Nicolas says "use C_{n+1} = gamma C_n + gg'" so no (1.-gamma)
        m = gram.subMat(n_eigen_old, n_eigen_old, gradients.length(), gradients.length());
        productTranspose(m, gradients, gradients);
        //m *= (1.-gamma) / gradients.length();
        m /= gradients.length();

        m = gram.subMat(n_eigen_old, 0, gradients.length(), n_eigen_old );
        productTranspose(m, gradients, cov_eigen_vec);
        //m *= sqrt(gamma*(1.-gamma)/gradients.length());
        m *= sqrt(gamma/gradients.length());

        Mat m2 = gram.subMat( 0, n_eigen_old, n_eigen_old, gradients.length() );
        transpose( m, m2 );

        //G = (G + G')/2; % Solving numerical mistakes

//cout << "--" << endl << gram << endl;

        // Extract eigenvectors/eigenvalues - destroys the content of gram, D and U are resized
        // gram = U D U' (if we took all values)
        eigenVecOfSymmMat(gram, n_eigen_cur, D, U);

        // Percentage of the variance we keep is the sum of the kept eigenvalues divided
        // by the total variance.
        //variance_percentage = sum(D)/total_variance;

	// The new (rescaled) eigenvectors are of the form [U A]*V where V is the
	// eigenvector of G. Rewriting V = [V1;V2], we have [U A]*V = UV1 + AV2.
        // for us cov_eigen_vec = U1 eigen_vec + U2 gradients

        swap = old_cov_eigen_vec;
        old_cov_eigen_vec = cov_eigen_vec;
        cov_eigen_vec = swap;

        cov_eigen_vec.resize(n_eigen_cur, gradients.width());
        product( cov_eigen_vec, U.subMatColumns(0, n_eigen_old), old_cov_eigen_vec );

//  C = alpha A.B + beta C
productScaleAcc(cov_eigen_vec, U.subMatColumns(n_eigen_old, gradients.length()), false, gradients, false,
                   sqrt((1.-gamma)/gradients.length()), sqrt(gamma));

        cov_eigen_val.resize( D.length() );
        cov_eigen_val << D;

        cov_norm_eigen_vec.resize( n_eigen_cur, gradients.width() );
        for( int i=0; i<n_eigen_cur; i++)   {
            Vec v = cov_norm_eigen_vec(i);
            divide( cov_eigen_vec(i), sqrt(D[i]), v );
        }

    }

    // ### Determine reg - Should be set automaticaly.
    //reg = cov_eigen_val[n_eigen_cur-1];
    for( int i=0; i<n_eigen_cur; i++)   {
        if( cov_eigen_val[i] < reg )  {
            PLWARNING("cov_eigen_val[i] < reg. Setting to reg.");
            cov_eigen_val[i] = reg;
        }
    }


    // *** Compute C^{-1} mu, where mu is the mean of gradients ***

    // Compute mu
    columnMean( gradients, mu );


/*    cout << "mu  " << mu << endl;
    cout << "norm(mu) " << norm(mu) << endl;
    cout << "cov_eigen_val " << cov_eigen_val << endl;
    cout << "cov_eigen_vec " << cov_eigen_vec << endl;
    cout << "cov_norm_eigen_vec " << cov_norm_eigen_vec << endl;*/

    // Compute the dot product with the eigenvectors
    dot_prod.resize(n_eigen_cur);
    product( dot_prod, cov_norm_eigen_vec, mu);

//    cout << "dot_prod " << dot_prod << endl;

    // Rescale according to the eigenvectors. Since the regularization constant will
    // be added to all the eigenvalues (and not only the ones we didn't keep), we
    // have to remove it from the ones we kept.
    scaled_dot_prod.resize(n_eigen_cur);

    divide( dot_prod, cov_eigen_val, scaled_dot_prod);
    scaled_dot_prod -= dot_prod/reg;

    transposeProduct(naturalg, cov_norm_eigen_vec, scaled_dot_prod);

    naturalg += mu / reg;


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
