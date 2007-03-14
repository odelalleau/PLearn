// -*- C++ -*-

// NatGradEstimator.cc
//
// Copyright (C) 2007 yoshua Bengio
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

// Authors: yoshua Bengio

/*! \file NatGradEstimator.cc */


#include "NatGradEstimator.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/math/plapack.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NatGradEstimator,
    "Convert a sequence of gradients into covariance-corrected (natural gradient) directions.\n",
    "The algorithm used for converting a sequence of n-dimensional gradients g_t\n"
    "into covariance-corrected update directions v_t is the following:\n\n"
    "init():\n"
    "  initialize U = Id(k,n)\n"
    "  initialize D = lambda Id(k,k), diag matrix stored as a vector\n"
    "  initialize sigma = 0\n"
    "\n"
    "operator(int t, Vec g, Vec v): (reads g and writes v)\n"
    "    i = t%b   /* denoting b = cov_minibatch_size */\n"
    "    G_{.i} = g /* = gradient for example t = i-th column of matrix G */\n"
    "    if t<b \n"
    "        v0_i = g / (lambda + ||g||^2)\n"
    "        v = v0_i\n"
    "    else /* denoting k = n_eigen */ \n"
    "        v0_i = (g gamma/sigma + sum_{j=1}^k (1/D_j - gamma/sigma) U_{.j} U_{.j}' g)  /* = inv(C) g */ \n"
    "        u0_i = v0_i / ( gamma + v0_i' g / (i+1))\n"
    "        v = u0_i  - (1/(i+1)) sum_{j=1}^{i-1} v0_j G_{.j}' u0_i / (gamma + v0_j'G_{.j}/(i+1)) \n"
    "    for j = 1 to inversion_n_iterations\n"
    "       v = (1 - gamma alpha) v + alpha v0_i - (alpha/i) sum_{r=0}^i v0_r G_{.r}' v\n"
    "    v *= (1 - gamma^{t/b})/(1 - gamma)\n"
    "    if i+1==b  /* recompute eigen-decomposition: */\n"
    "       M = [gamma D    (gamma/b)^{1/2} sqrt(D) U' G;  (gamma/b)^{1/2} G' U sqrt(D)    G'G/b] /* = Gram matrix */\n"
    "       (V,E) = leading_eigendecomposition(M,k)\n"
    "       U = ([U sqrt(D)   G] V E^{-1/2} /* = k-principal e-vec of C */\n"
    "       D = E /* = k principal e-val of C */\n"
    "       sigma = {(k+1)th e-value of M}/gamma \n"
    "               /* = heuristic value for lower e-values of C */\n"
    "\n\n"
    "This is derived from the following considerations:\n"
    "  - let the covariance estimator at the beginning of minibatch t/b be C. We have its\n"
    "    eigen-decomposition in principal e-vectors U, principal e-values D, and lower e-values=sigma.\n"
    "  - at the end of the minibatch it is B + GG'/b\n"
    "    where B is C with the upper eigenvalues reduced by a factor gamma.\n"
    "  - this introduces a scaling factor (1-gamma)/(1-gamma^{t/b}) which is scaled out of\n"
    "    the v's on last line of above pseudo-code\n"
    "  - to obtain the eigen-decomposition efficiently, we rewrite B* + GG' in Gram matrix form\n"
    "    where B* ignores the lower eigenvalues of B, i.e. B* = gamma U D U'. Hence\n"
    "    B* + GG' = [sqrt(gamma) U sqrt(D)    G]' [sqrt(gamma) U sqrt(D)   G],\n"
    "    but this matrix has the same eigenvalues as M = [sqrt(gamma) U sqrt(D)    G] [sqrt(gamma) U sqrt(D)    G]'\n"
    "    and the eigenvectors of B*  + GG' can be recovered from above formula.\n"
    "  - To regularize B* + GG', we threshold the lower eigenvalues and set them to the (k+1)-th eigenvalue.\n"
    "  - on the i-th gradient g_i of the minibatch we would like to solve\n"
    "           (B + (1/i)sum_{k=1}^i g_k g_k') v_i = g_i\n"
    "  - we do this iteratively using as initial estimator of v_i: v_i^0 = inv(F) g_i\n"
    "    where F is C with the lower eigenvalues boosted by a factor 1/gamma, and \n"
    "    each iteration has the form:\n"
    "            v_i <-- v_i + alpha inv(F) (g_i - (B + (1/i)sum_{k=1}^i g_k g_k') v_i)\n"
    "    which can be simplified into\n"
    "            v_i <-- (1 - alpha gamma) v_i + alpha v_i^0 - alpha/i sum_{k=1}^i v_k^0 g_k' v_i \n"
    );

NatGradEstimator::NatGradEstimator()
    /* ### Initialize all fields to their default value */
    : cov_minibatch_size(10),
      lambda(1),
      n_eigen(10),
      alpha(0.1),
      gamma(0.9),
      inversion_n_iterations(5),
      n_dim(-1),
      use_double_init(true),
      verbosity(0),
      sigma(0),
      previous_t(-1)
{
    build();
}

// ### Nothing to add here, simply calls build_
void NatGradEstimator::build()
{
    inherited::build();
    build_();
}

void NatGradEstimator::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    deepCopyField(Ut, copies);
    deepCopyField(E, copies);
    deepCopyField(D, copies);
    deepCopyField(Gt, copies);
    deepCopyField(initial_v, copies);
    deepCopyField(tmp_v, copies);
    deepCopyField(M, copies);
    deepCopyField(M11, copies);
    deepCopyField(M12, copies);
    deepCopyField(M21, copies);
    deepCopyField(M22, copies);
    deepCopyField(Vt, copies);
    deepCopyField(Vkt, copies);
    deepCopyField(Vbt, copies);
    deepCopyField(newUt, copies);
    deepCopyField(vg, copies);
}

void NatGradEstimator::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    declareOption(ol, "cov_minibatch_size", &NatGradEstimator::cov_minibatch_size,
                  OptionBase::buildoption,
                  "Covariance estimator minibatch size, i.e. number of calls\n"
                  "to operator() before re-estimating the principal\n"
                  "eigenvectors/values. Note that each such re-computation will\n"
                  "cost O(n_eigen * n)");
    declareOption(ol, "lambda", &NatGradEstimator::lambda,
                  OptionBase::buildoption,
                  "Initial variance. The first covariance is assumed to be\n"
                  "lambda times the identity. Default = 1.\n");
    declareOption(ol, "n_eigen", &NatGradEstimator::n_eigen,
                  OptionBase::buildoption,
                  "Number of principal eigenvectors of the covariance matrix\n"
                  "that are kept in its approximation.\n");
    declareOption(ol, "alpha", &NatGradEstimator::alpha,
                  OptionBase::buildoption,
                  "Learning rate of the inversion iterations.\n");
    declareOption(ol, "inversion_n_iterations", &NatGradEstimator::inversion_n_iterations,
                  OptionBase::buildoption,
                  "Number of iterations of numerical approximation algorithm for\n"
                  "solving the system inverse(cov) v = g\n");
    declareOption(ol, "use_double_init", &NatGradEstimator::use_double_init,
                  OptionBase::buildoption,
                  "wether to use the u0 and its correction for initialization the inversion iteration\n");
    declareOption(ol, "verbosity", &NatGradEstimator::verbosity,
                  OptionBase::buildoption,
                  "Verbosity level\n");

    declareOption(ol, "n_dim", &NatGradEstimator::n_dim,
                  OptionBase::learntoption,
                  "Number of dimensions of the gradient vectors\n");
    declareOption(ol, "Ut", &NatGradEstimator::Ut,
                  OptionBase::learntoption,
                  "Estimated principal eigenvectors of the gradients covariance matrix\n"
                  "(stored in the rows of Ut)\n");
    declareOption(ol, "E", &NatGradEstimator::E,
                  OptionBase::learntoption,
                  "Estimated principal eigenvalues of the gradients covariance matrix\n");
    declareOption(ol, "sigma", &NatGradEstimator::sigma,
                  OptionBase::learntoption,
                  "Estimated value for the minor eigenvalues of the gradients covariance matrix\n");
    declareOption(ol, "gamma", &NatGradEstimator::gamma,
                  OptionBase::learntoption,
                  "Forgetting factor in moving average estimator of covariance. 0<gamma<1.\n");
    declareOption(ol, "Gt", &NatGradEstimator::Gt,
                  OptionBase::learntoption,
                  "Collected gradients during a minibatch\n");
    declareOption(ol, "previous_t", &NatGradEstimator::previous_t,
                  OptionBase::learntoption,
                  "Value of t at previous call of operator()\n");
    declareOption(ol, "initial_v", &NatGradEstimator::initial_v,
                  OptionBase::learntoption,
                  "Initial v for the g's of the current minibatch\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void NatGradEstimator::build_()
{
    init();
}

void NatGradEstimator::init()
{
    if (n_dim>=0)
    {
        PLASSERT_MSG(n_dim>0, "NatGradEstimator::init(), n_dim should be > 0");
        PLASSERT_MSG(gamma<1 && gamma>0, "NatGradEstimator::init(), gamma should be < 1 and >0");
        Ut.resize(n_eigen,n_dim);
        Vt.resize(n_eigen+1,n_eigen+cov_minibatch_size);
        Vkt = Vt.subMat(0,n_eigen,0,n_eigen);
        Vbt = Vt.subMat(0,n_eigen,n_eigen,cov_minibatch_size);
        E.resize(n_eigen+1);
        D = E.subVec(0,n_eigen);
        M.resize(n_eigen + cov_minibatch_size, n_eigen + cov_minibatch_size);
        M11=M.subMat(0,n_eigen,0,n_eigen);
        M12=M.subMat(0,n_eigen,n_eigen,cov_minibatch_size);
        M21=M.subMat(n_eigen,cov_minibatch_size,0,n_eigen);
        M22=M.subMat(n_eigen,cov_minibatch_size,n_eigen,cov_minibatch_size);
        Gt.resize(cov_minibatch_size, n_dim);
        initial_v.resize(cov_minibatch_size, n_dim);
        tmp_v.resize(n_dim);
        newUt.resize(n_eigen,n_dim);
        vg.resize(cov_minibatch_size);
    }
}

void NatGradEstimator::operator()(int t, const Vec& g, Vec v)
{
    if (t!=0)
        PLASSERT_MSG(t==previous_t+1, "NatGradEstimator() should be called sequentially!");
    if  (n_dim<0) 
    {
        PLASSERT_MSG(t==0, "The first call to NatGradEstimator() should be with t=0\n");
        n_dim = g.length();
        v.resize(n_dim);
        init();
    }
    int i = t % cov_minibatch_size;
    Vec v0 = initial_v(i);
    Gt(i) << g;

    // initialize v0
    v0 << g;
    if (t<cov_minibatch_size)
    {
        v0 *= 1.0/(lambda + pownorm(g));
        v << v0;
    }
    else
    {
        real oos = gamma/sigma;
        real ooip1 = 1.0/(i+1.0);
        v0 *= oos;
        // v0 = g*gamma/sigma + sum_j (1/D_j - gamma/sigma Uj Uj' g
        for (int j=0;j<n_eigen;j++)
        {
            Vec Uj = Ut(j);
            multiplyAcc(v0, Uj, (1/D[j] - oos) * dot(Uj,g));
        }
        if (use_double_init)
        {
            vg[i] = dot(v0,g);
            multiply(v0,1.0/(gamma + vg[i]*ooip1),tmp_v); // tmp_v == u0_i here
            v << tmp_v;
            for (int j=0;j<i;j++)
                multiplyAcc(v, initial_v(j), -ooip1*dot(Gt(j),tmp_v)/(gamma + vg[j]*ooip1));
        }
        else
            v << v0;
    }

    // iterate on v to solve linear system
    for (int j=0;j<inversion_n_iterations;j++)
    {
        multiply(v, (1 - gamma*alpha),tmp_v);
        multiplyAcc(tmp_v, v0, alpha);
        for (int r=0;r<=i;r++)
            multiplyAcc(tmp_v, initial_v(r), -alpha/(i+1)*dot(Gt(r),v));
        v << tmp_v;
        // verify that we get an improvement
        if (verbosity>0)
        {
            // compute (B + (1/i)sum_{k=1}^i g_k g_k') v_i            
            //        =(U (gamma D -sigma I) U' + sigma I + (1/i)sum_{k=1}^i g_k g_k') v_i            
            multiply(v,sigma,tmp_v);
            for (int j=0;j<n_eigen;j++)
            {
                Vec Uj = Ut(j);
                multiplyAcc(tmp_v,Uj,(gamma*D[j]-sigma)*dot(Uj,v));
            }
            for (int j=0;j<=i;j++)
            {
                Vec Gj = Gt(j);
                multiplyAcc(tmp_v,Gj,dot(Gj,v)/(i+1));
            }
            // result is in tmp_v. Compare with g_i
            real gnorm = dot(g,g);
            real enorm = dot(tmp_v,tmp_v);
            real angle = acos(dot(tmp_v,g)/sqrt(gnorm*enorm))*360/(2*3.14159);
            real err = L2distance(g,tmp_v);
            cout << "linear system distance=" << err << ", angle="<<angle<<", norm ratio="<<enorm/gnorm<<endl;
        }
    }
    
    // normalize back v, to take into account scaling up of C due to gamma iteration
    v *= (1 - pow(gamma,real(t/cov_minibatch_size)))/(1 - gamma);
    // recompute the eigen-decomposition
    if (i+1==cov_minibatch_size)
    {
        // build Gram matrix M, by blocks [M11 M12; M21 M22]
        M11.clear();
        for (int j=0;j<n_eigen;j++)
            M11(j,j) = gamma*D[j];
        productTranspose(M12,Ut,Gt);
        real gob=gamma/cov_minibatch_size;
        for (int j=0;j<n_eigen;j++)
            M12(j) *= sqrt(D[j]*gob);
        transpose(M12,M21);
        productTranspose(M22,Gt,Gt);
        M22 *= 1.0/cov_minibatch_size;

        // get eigen-decomposition, with one more eigen-x than necessary to set sigma
        eigenVecOfSymmMat(M,n_eigen+1,E,Vt);
        
        // convert eigenvectors Vt of M into eigenvectors U of C
        product(newUt,Vbt,Gt);
        Vec sqrtD = tmp_v.subVec(0,n_eigen);
        compute_sqrt(D,sqrtD);
        diagonalizedFactorsProduct(newUt,Vkt,sqrtD,Ut,true);
        Ut << newUt;
    }
    
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
