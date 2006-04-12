// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of
//                         Montreal, all rights reserved
// Copyright (C) 2006 Olivier Delalleau
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

//
// Cholesky_utils.cc
//
//
// Authors: Yoshua Bengio
//

/*! \file Cholesky_utils.cc */

#include <plearn/math/Cholesky_utils.h>
#include <plearn/math/TMat_maths.h>


namespace PLearn {
using namespace std;

/////////////////////////////
// choleskyAppendDimension //
/////////////////////////////
void choleskyAppendDimension(Mat& L, const Vec& new_row)
{
    static Vec last_row;
    int n = L.length();
    assert( L.width() == n);
    assert( new_row.length() == n + 1 );
    assert( new_row.last() >= 0 );

    if (n == 0) {
        // Simpler version for this specific case.
        L.resize(1, 1);
        L(0, 0) = sqrt(new_row[0]);
        assert( !L.hasMissing() );
        return;
    }

    last_row.resize(n);
    choleskyLeftSolve(L, new_row.subVec(0, n), last_row);
    last_row.append(sqrt(new_row.last() - pownorm(last_row)));
    assert( !last_row.hasMissing() );
    L.resize(n + 1, n + 1, 0, true);
    L(n) << last_row;
}

/////////////////////////////
// choleskyRemoveDimension //
/////////////////////////////
void choleskyRemoveDimension(Mat& L, int i)
{
    int p = L.length();
    /* Old version, will be removed when the fast version below is tested

    // Note that in order to use the exact algorithms from the matrix
    // algorithms book, we need to transpose L (since R = L' in the QR
    // decomposition). There may be a more efficient way to do the same
    // operations.
    L.transpose();
    for (int j = i; j < p - 1; j++)
        chol_dxch(L, j, j + 1);
    L = L.subMat(0, 0, p - 1, p - 1);
    L.transpose();
    */
    for (int j = i; j < p - 1; j++)
        chol_dxch_tr(L, j, j + 1);
    L = L.subMat(0, 0, p - 1, p - 1);
}

// L (n_active x n_active) is lower-diagonal and is such that A = L L' = lambda I + sum_t phi_t(x_t) phi_t(x_t)'
// where the sum runs until 'now' and phi_t is the 'current' active basis set. 
// In this function we update L so as to incorporate a new basis, i.e. n_active is incremented,
// using a new basis whose outputs on all the examples up to 'now' are given in new_basis_outputs.
// The dimensions of L are changed as a result. Note that L must have been pre-allocated
// with dimensions max_n_active x max_n_active. The matrix active_bases_outputs (n_active x n_examples)
// contains the outputs of the active bases that are in phi up to now.
// Computational cost is O(n_active * n_examples).
void choleskyInsertBasis(Mat& L, Mat active_bases_outputs, Vec new_basis_outputs, real lambda, real min_Lii)
{
    // Let us write f = new_basis_outputs.
    // Let L* be the new L (we will do it in-place, though). 
    // It is easy to show that  L*(1:n_active,1:n_active) = L, so we only need to find
    // the new row of L* and its last element d=L*(n_active+1,n_active+1).
    // Let l be the vector with the first (current) n_active elements of that new row
    // and l*=L*(n_active+1,n_active+1).
    // 
    // The elements of the last row of L* L*' should be y_i = sum_t phi_{ti} f_t = f . phi_{:,i}.
    // The last element of the new row (lower right corner) of L* L*' should be lambda + f . f.
    // But the elements of the last row of L* L*' are L l. Hence we must choose l s.t. L l = y.
    // This can be obtained by a simple back-substitution. 
    // In the corner element of L* L*' we will get l*^2 + l'l but we want f'f+lambda.
    // Hence we set the corner element of L*, l* = sqrt(f'f+lambda-l'l).
    // 
    
    int n_active = L.length();
    static Vec y,l;
    if (n_active==0) { y.resize(1); l.resize(1); } // to avoid error when asking for data()
    y.resize(n_active);
    real* yp=y.data();
    l.resize(n_active);
    real* lp=l.data();

    // O(n_bases * n_examples)
    for (int i=0;i<n_active;i++)
        yp[i] = dot(new_basis_outputs, active_bases_outputs(i));
    // O(n_bases^2)
    choleskyLeftSolve(L,y,l);
    
    L.resize(n_active+1,n_active+1,n_active*n_active,true);
    real* Lplast=L[n_active];
    real ll=0;
    // O(n_bases)
    for (int i=0;i<n_active;i++)
    {
        Lplast[i] = lp[i];
        ll+=lp[i]*lp[i];
    }
    real arg = lambda + pownorm(new_basis_outputs) - ll;
    if (arg>0)
        Lplast[n_active]=sqrt(arg);
    else
        Lplast[n_active]=min_Lii;
}

// Given a current Cholesky decomposition of A = L L' = sum_t v_t v_t'
// add an extra v_t v_t' term to the matrix A (i.e. upgrade lower-diagonal L accordingly).
// computational cost: O(n^2)
void choleskyUpgrade(Mat& L, Vec v)
{
    // Algorithm: See tech report "Low Rank Updates for the Cholesky Decomposition" by Matthias Seeger, UC Berkeley, 2005.
    // Denote n=dim(L)
    // - find vector p s.t. L p = v (back-substitution)
    // - compute elements of vectors b and d as follows:
    //     u=1
    //     for i=1 to n-1
    //       a = u + p_i^2
    //       d_i = a/u
    //       b_i = p_i/a
    //       u = a 
    //     d_n = 1 + p_n^2 / u
    // - update other elements of L as follows:
    //     for i=1 to n
    //       s=0
    //       new L_ii = L_ii * sqrt(d_i)
    //       if i>1 r = L_ii p_i
    //       for j=i-1 down to 1
    //          s = s + r; r = L_ij p_j
    //          new L_ij = (L_ij + s b_j) * sqrt(d_j)
    //
    int n=L.length();
    static Vec p, b, d;
    p.resize(n);
    b.resize(n);
    d.resize(n);
    real* p_=p.data();
    real* b_=b.data();
    real* d_=d.data();
    choleskyLeftSolve(L,v,p);
    real u=1;
    for (int i=0;i<n-1;i++)
    {
        real pi = p_[i];
        real a = u + pi*pi;
        d_[i] = a/u;
        b_[i] = pi/a;
        u=a;
    }
    real lp = p_[n-1];
    d_[n-1]=1+lp*lp/u;
    for (int i=0;i<n;i++)
        d_[i] = sqrt(d_[i]);
    real r;
    for (int i=0;i<n;i++)
    {
        real s=0;
        real* Li = L[i];
        real Lii = Li[i];
        Li[i] *= d_[i];
        if (i>0) {
            r = Lii*p_[i];
            for (int j=i-1;j>=0;j--)
            {
                s=s+r;
                r=Li[j]*p_[j];
                Li[j] = (Li[j] + s*b_[j])*d_[j];
            }
        }
    }
}

///////////////
// chol_dxch //
///////////////
void chol_dxch(Mat& R, int l, int m)
{
    if (l == m)
        return;
    if (l > m) {
        int tmp = l;
        l = m;
        m = tmp;
    }
    //static Vec tmp;
    int n = R.length();
    int p = n;
    //Mat first_m_rows = R.subMatRows(0, m + 1);
    R.subMatRows(0, m + 1).swapColumns(l, m);
    real c, s;
    for (int k = m - 1; k >= l + 1; k--) {
        chol_rotgen(R(k, l), R(k + 1, l), c, s);
        chol_rotapp(c, s, R(k).subVec(k, p - k), R(k + 1).subVec(k, p - k));
    }
    for (int k = l; k < m; k++) {
        chol_rotgen(R(k, k), R(k+1, k), c, s);
        chol_rotapp(c, s, R(k).subVec(k + 1, p - k - 1),
                          R(k + 1).subVec(k + 1, p - k - 1));
    }
}

//////////////////
// chol_dxch_tr //
//////////////////
void chol_dxch_tr(Mat& R, int l, int m)
{
    if (l == m)
        return;
    if (l > m) {
        int tmp = l;
        l = m;
        m = tmp;
    }
    int n = R.width();
    int p = n;
    R.subMatColumns(0, m + 1).swapRows(l, m);
    real c, s;
    for (int k = m - 1; k >= l + 1; k--) {
        chol_rotgen(R(l, k), R(l, k + 1), c, s);
        chol_rotapp_tr(c, s, R.subMat(k, k, p - k, 1),
                             R.subMat(k, k + 1, p - k, 1));
    }
    for (int k = l; k < m; k++) {
        chol_rotgen(R(k, k), R(k, k + 1), c, s);
        chol_rotapp_tr(c, s, R.subMat(k + 1, k, p - k - 1, 1),
                             R.subMat(k + 1, k + 1, p - k - 1, 1));
    }
}

/////////////////
// chol_rotapp //
/////////////////
void chol_rotapp(real c, real s, const Vec& x, const Vec& y)
{
    static Vec t;
    assert( x.length() == y.length() );
    t.resize(x.length());
    t << x;
    multiplyScaledAdd(y, c, s, t);
    multiplyScaledAdd(x, c, -s, y);
    x << t;
}

////////////////////
// chol_rotapp_tr //
////////////////////
void chol_rotapp_tr(real c, real s, const Mat& x, const Mat& y)
{
    static Vec t;
    assert( x.length() == y.length() );
    int n = x.length();
    int x_mod = x.mod();
    int y_mod = y.mod();
    t.resize(n);
    // multiplyScaledAdd(y, c, s, t);
    real* y_ = y.data();
    real* t_ = t.data();
    real* x_ = x.data();
    for (int i = 0; i < n; i++) {
        *t_ = *x_; // Backup of old value of x.
        *x_ = c * (*x_) + s * (*y_);
        t_++;
        x_ += x_mod;
        y_ += y_mod;
    }
    // multiplyScaledAdd(x, c, -s, y);
    y_ = y.data();
    t_ = t.data();
    for (int i = 0; i < n; i++) {
        *y_ = c * (*y_) - s * (*t_);
        y_ += y_mod;
        t_++;
    }
}

/////////////////
// chol_rotgen //
/////////////////
void chol_rotgen(real& a, real& b, real& c, real& s)
{
    real t = fabs(a) + fabs(b);
    if (fast_exact_is_equal(t, 0)) {
        c = 1;
        s = 0;
        return;
    }
    real a_over_t = a / t;
    real b_over_t = b / t;
    t *= sqrt( a_over_t * a_over_t + b_over_t * b_over_t);
    c = a / t;
    s = b / t;
    a = t;
    b = 0;
}

} // end namespace PLearn


#include <plearn/math/random.h>

namespace PLearn {

void testCholeskyRoutines()
{
    int n=5,l=10;
    real lambda=0.1;
    Mat Xp(l,n+1);
    Mat X=Xp.subMatColumns(0,n);
    Mat Mp(n+1,n+1);
    Mat M=Mp.subMat(0,0,n,n);
    Mat L(n+1,n+1), testL(n,n), Lp(n+1,n+1), testLp(n+1,n+1);
    L.resize(n,n);
    fill_random_uniform(Xp,-1.,1.);

    identityMatrix(Mp);
    Mp*=lambda;
    choleskyDecomposition(M,testL);
    for (int t=0;t<l;t++)
    {
        //externalProductAcc(M,X(t),X(t));
        externalProductAcc(Mp,Xp(t),Xp(t));
        choleskyUpgrade(testL,X(t));
    }
    choleskyDecomposition(M,L);
    Mat testM(n,n);
    product(testM, L,transpose(L));
    testM -= M;
    real average_error = sumsquare(testM)/(n*n);
    real max_error = max(testM);
    cout << "Cholesky decomposition average error = " << average_error << ", max error = " << max_error << endl;

    // *** test choleskyUpgrade ***
    // compare with the batch method:
    testL -=L;
    average_error = sumsquare(testL)/(n*(n+1)/2);
    max_error = max(testL);
    cout << "average error in choleskyUpgrade for " << l << " upgrades of a " << n << " x " << n << " matrix = " << average_error <<  ", max error = " << max_error << endl;

    // *** test choleskyInsertBasis ***
    Mat bases_outputs = transpose(X);
    choleskyInsertBasis(L,bases_outputs, Xp.column(n).toVecCopy(), lambda, 1e-10);
    // compare with the batch method:
    choleskyDecomposition(Mp,testLp);
    testLp -=L;
    average_error = sumsquare(testLp)/((n+1)*(n+2)/2);
    max_error = max(testLp);
    cout << "average error in choleskyInsertBasis = " << average_error <<  ", max error = " << max_error << endl;
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
