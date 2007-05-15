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

// bool save_G=false;

PLEARN_IMPLEMENT_OBJECT(
    NatGradEstimator,
    "Convert a sequence of gradients into covariance-corrected (natural gradient) directions.\n",
    "The algorithm used for converting a sequence of n-dimensional gradients g_t\n"
    "into covariance-corrected update directions v_t is the following:\n\n"
    "operator(int t, Vec g, Vec v): (reads g and writes v)\n"
    "    i = t%b   /* denoting b = cov_minibatch_size */\n"
    "    extend X by a (k+i)-th column gamma^{\frac{-i}{2}} g\n"
    "    extend G by a (k+i)-th column and row, with G_{k+i,.}=X'_{k+1,.} X\n"
    "      and idem for the symmetric sub-column\n"
    "    extend vectors r and a by (k+i)-th element, r_{k+i-1}=0, r_{k+i}=gamma^{\frac{-i}{2}}\n"
    "    Solve linear system (G + gamma^{-k} lambda I) a = r in a\n"
    "    v = X a (1 - gamma)/(1 - gamma^t)\n"
    "    if i+1==b\n"
    "       (V,D) = leading_eigendecomposition(G,k)\n"
    "       U = gamma^{b/2} X V\n"
    "\n\n"
    "See technical report 'A new insight on the natural gradient' for justifications\n"
    );

NatGradEstimator::NatGradEstimator()
    /* ### Initialize all fields to their default value */
    : cov_minibatch_size(10),
      lambda(1),
      n_eigen(10),
      gamma(0.99),
      n_dim(-1),
      verbosity(0),
      renormalize(true),
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
    deepCopyField(D, copies);
    deepCopyField(Xt, copies);
    deepCopyField(G, copies);
    deepCopyField(r, copies);
    deepCopyField(Vt, copies);
    deepCopyField(Vkt, copies);
    deepCopyField(A, copies);
    deepCopyField(pivots, copies);
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
    declareOption(ol, "regularizer", &NatGradEstimator::lambda,
                  OptionBase::buildoption,
                  "Proxy for option lambda (different name to avoid python problems).\n");
    declareOption(ol, "n_eigen", &NatGradEstimator::n_eigen,
                  OptionBase::buildoption,
                  "Number of principal eigenvectors of the covariance matrix\n"
                  "that are kept in its approximation.\n");
    declareOption(ol, "gamma", &NatGradEstimator::gamma,
                  OptionBase::buildoption,
                  "Forgetting factor in moving average estimator of covariance. 0<gamma<1.\n");
    declareOption(ol, "amari_version", &NatGradEstimator::amari_version,
                  OptionBase::buildoption,
                  "Instead of our tricks, use the formula Ginv <-- (1+eps) Ginv - eps Ginv g g' Ginv\n"
                  "to estimate the inverse of the covariance matrix, and multiply it with g at each step.\n");
    declareOption(ol, "verbosity", &NatGradEstimator::verbosity,
                  OptionBase::buildoption,
                  "Verbosity level\n");
    declareOption(ol, "renormalize", &NatGradEstimator::renormalize,
                  OptionBase::buildoption,
                  "Whether to renormalize z wrt scaling that gamma produces\n");

    declareOption(ol, "n_dim", &NatGradEstimator::n_dim,
                  OptionBase::learntoption,
                  "Number of dimensions of the gradient vectors\n");
    declareOption(ol, "Ut", &NatGradEstimator::Ut,
                  OptionBase::learntoption,
                  "Estimated scaled principal eigenvectors of the gradients covariance matrix\n"
                  "(stored in the rows of Ut)\n");
    declareOption(ol, "G", &NatGradEstimator::G,
                  OptionBase::learntoption,
                  "Gram matrix growing during a minibatch\n");
    declareOption(ol, "previous_t", &NatGradEstimator::previous_t,
                  OptionBase::learntoption,
                  "Value of t at previous call of operator()\n");
    declareOption(ol, "Xt", &NatGradEstimator::Xt,
                  OptionBase::learntoption,
                  "contains in its rows the scaled eigenvectors and g's\n"
                  "seen since the beginning of the minibatch.\n");

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
        Vkt = Vt.subMatRows(0,n_eigen);
        D.resize(n_eigen+1);
        G.resize(n_eigen + cov_minibatch_size, n_eigen + cov_minibatch_size);
        A.resize(n_eigen + cov_minibatch_size, n_eigen + cov_minibatch_size);
        G.clear();
        Xt.resize(n_eigen+cov_minibatch_size, n_dim);
        Xt.clear();
        r.resize(n_eigen);
        for (int j=0;j<n_eigen;j++)
            G(j,j) = lambda;
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
    int n = n_eigen+i;
    Xt.resize(n+1,n_dim);
    Vec newX = Xt(n);
    real rn = pow(gamma,real(-0.5*(i+1)));
    multiply(g,rn,newX);
    G.resize(n+1,n+1);
    Vec newG=G(n);
    product(newG,Xt,newX);
    G.column(n) << newG;
    r.resize(n+1);
    r.clear();
    r[n] = rn;
    // solve linear system (G + \gamma^{-k} \lambda I) a = r
    pivots.resize(n);
    A.resize(n+1,n+1);
    A << G;
    real rn2 = rn*rn;
    real coef = rn2*lambda;
    for (int j=0;j<=n;j++)
        A(j,j) += coef;
    Mat r_row = r.toMat(1,n+1);
    int status = lapackSolveLinearSystem(A,r_row,pivots);
    if (status!=0)
        PLWARNING("NatGradEstimator: lapackSolveLinearSystem returned %d\n:",status);
    if (verbosity>1 && i%(cov_minibatch_size/3)==0)
        cout << "solution r = " << r << endl;
    // solution is in r
    transposeProduct(v, Xt, r);
    if (renormalize)
        v*=(1 - pow(gamma,real(t+1)))/(1 - gamma);
        //v/=(1 - pow(gamma,real(t+1)))/(1 - gamma);
/*    {
        real gnorm = dot(g,g);
        real vnorm = dot(v,v);
        g*=sqrt(vnorm/gnorm);
    }
*/
    if (verbosity>0 && i%(cov_minibatch_size/2)==0)
    {
        real gnorm = dot(g,g);
        real vnorm = dot(v,v);
        real angle = acos(dot(v,g)/sqrt(gnorm*vnorm))*360/(2*3.14159);
        cout << "angle(g,v)="<<angle<<", norm ratio="<<vnorm/gnorm<<endl;
    }

    // recompute the eigen-decomposition
    if (i+1==cov_minibatch_size)
    {
        // get eigen-decomposition, with one more eigen-x than necessary to check if coherent with lambda
        //if (save_G)
        //    saveAscii("G.amat",G);
        eigenVecOfSymmMat(G,n_eigen+1,D,Vt);
        
        // convert eigenvectors Vt of G into eigenvectors U of C
        product(Ut,Vkt,Xt);
        Ut *= 1.0/rn;
        D *= 1.0/rn2;
        for (int j=0;j<n_eigen;j++) 
            if (D[j]<1e-10)
                PLWARNING("NatGradEstimator: very small eigenvalue %d = %g\n",j,D[j]);
        if (verbosity>0) // verifier Ut U = D/
        {
            static Mat Dmat;
            cout << "eigenvalues = " << D << endl;
            if (verbosity>2)
            {
                Dmat.resize(n_eigen,n_eigen);
                productTranspose(Dmat,Ut,Ut);
                for (int j=0;j<n_eigen;j++) 
                    Dmat(j,j)-=D[j];
                cout << "norm(U' U - D)/(n_eigen*n_eigen) = " << sumsquare(Dmat.toVec())/n_eigen << endl;
            }
        }
        // prepare for next minibatch
        Xt.resize(n_eigen,n_dim);
        Xt << Ut;
        G.resize(n_eigen,n_eigen);
        G.clear();
        for (int j=0;j<n_eigen;j++)
            G(j,j) = D[j];
    }
    previous_t = t;
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
