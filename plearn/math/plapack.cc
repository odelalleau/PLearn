// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
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
 * This file is part of the PLearn library.
 ******************************************************* */

#include <cstdlib>
#include "plapack.h"
#include <algorithm>
#include "random.h"

namespace PLearn {
using namespace std;

int eigen_SymmMat(Mat& in, Vec& e_value, Mat& e_vector, int& n_evalues_found,
                  bool compute_all, int nb_eigen, bool compute_vectors, bool largest_evalues)
{
    PLWARNING("eigen_SymmMat is deprecated: use eigenVecOfSymmMat or lapackEIGEN instead");

#ifndef USE_BLAS_SPECIALISATIONS
    PLERROR("eigen_SymmMat: LAPACK not available on this system!");
    return 0;
#else
    if (!in.isSymmetric())
        PLERROR("eigen_SymmMat: Your input matrix is not symmetric\n");

    // some check
    if (nb_eigen < 1  ||  nb_eigen > in.length())
        PLERROR("The number of desired eigenvalues (%d) must be in range [1,%d]", nb_eigen, in.length());

    if (compute_all)
    {
        if (e_vector.length() != in.length()  ||  e_vector.width() != in.width())
            e_vector.resize(in.length(), in.width());
        if (in.length() != e_value.length())
            e_value.resize(in.length());
    }
    else
    {
        if (e_vector.length() != nb_eigen  ||  e_vector.width() != in.width())
            e_vector.resize(nb_eigen, in.width());
        if (nb_eigen != e_value.length())
            e_value.resize(nb_eigen);
    }

    // for the moment, we do not accept sub-matrices...
    if (in.mod() != in.width())
        PLERROR("The input matrix cannot be a sub-matrix...");

    // we set the parameters to call the LAPACK Fortran function
    // if compute_all==true,  we call <ssyev>
    // if compute_all==false, we call <ssyevx>

    int INFO = 1;
    if (compute_all)
    {
        char* JOBZ;
        if (compute_vectors)
            JOBZ = "V";
        else
            JOBZ = "N";
        char* UPLO = "U";
        int N = in.length();
        real* A = in.data();
        int LDA = N;
        real* W = new real[N];
        int LWORK = 3*N;
        real* WORK = new real[LWORK];

        // we now call the LAPACK Fortran function <ssyev>
#ifdef USEFLOAT
        ssyev_(JOBZ, UPLO, &N, A, &LDA, W, WORK, &LWORK, &INFO);
#endif
#ifdef USEDOUBLE
        dsyev_(JOBZ, UPLO, &N, A, &LDA, W, WORK, &LWORK, &INFO);
#endif

        if (INFO != 0)
        {
            PLWARNING("eigen_SymmMat: something in ssyev got wrong.  Error code %d",INFO);
            n_evalues_found = 0;
        }
        else
        {
            n_evalues_found = N;
            for (int i=0; i<N; i++)
                e_value[i] = W[i];

            if (compute_vectors)
            {
                real* p_evector = e_vector.data();
                real* p_a = A;
                for (int i=0; i<N; i++)
                    for (int j=0; j<N; j++, p_evector++, p_a++)
                        *p_evector = *p_a;
            }
        }
        delete[] W;
        delete[] WORK;
    }
    else
    {
        char* JOBZ;
        if (compute_vectors)
            JOBZ = "V";
        else
            JOBZ = "N";
        char* RANGE = "I";
        char* UPLO = "U";
        int N = in.length();
        real* A = in.data();
        int LDA = N;
        real VL, VU;  // not referenced
        int IL,IU;
        if (largest_evalues)
        {
            IL = N - nb_eigen + 1;
            IU = N;
        }
        else
        {
            IL = 1;
            IU = nb_eigen;
        }
        real ABSTOL = 1e-10;
        int M;
        real* W= new real[N];
        int LDZ = N;
        real* Z = new real[LDZ*nb_eigen];
        int LWORK = 8*N;
        real* WORK = new real[LWORK];
        int* IWORK = new int[5*N];
        int* IFAIL = new int[N];

        // we now call the LAPACK Fortran function <ssyevx>
        lapack_Xsyevx_(JOBZ, RANGE, UPLO, &N, A, &LDA, &VL, &VU, &IL, &IU, &ABSTOL, &M, W, Z, &LDZ, WORK, &LWORK, IWORK, IFAIL, &INFO);

        n_evalues_found = M;
        if (M != nb_eigen)
            cout << "eigen_SymmMat: something in ssyevx got wrong." << endl
                 << "The number of eigenvalues found (" << M
                 << ") is different from what we asked (" << nb_eigen << ")." << endl;

        if (INFO != 0)
        {
            //      cout << "eigen_SymmMat: something in ssyevx got wrong.  Error code "
            //           << INFO << endl << "See the man page of ssyevx for more details"
            //           << endl;
        }
        else
        {
            for (int i=0; i<M; i++)
                e_value[i] = W[i];

            if (compute_vectors)
            {
                real* p_evector = e_vector.data();
                real* p_z = Z;
                for (int i=0; i<M; i++)
                    for (int j=0; j<N; j++, p_evector++, p_z++)
                        *p_evector = *p_z;
            }
        }
        delete[] W;
        delete[] WORK;
        delete[] IWORK;
        delete[] IFAIL;
    }
    return INFO;
#endif
}

int eigen_SymmMat_decreasing(Mat& in, Vec& e_value, Mat& e_vector, int& n_evalues_found,
                             bool compute_all, int nb_eigen, bool compute_vectors, bool largest_evalues)
{
    PLWARNING("eigen_SymmMat_decreasing is deprecated: use eigenVecOfSymmMat or lapackEIGEN instead");

    int res = eigen_SymmMat(in, e_value, e_vector, n_evalues_found,
                            compute_all, nb_eigen, compute_vectors, largest_evalues);
    e_value.swap();
    e_vector.swapUpsideDown();
    return res;
}

int matInvert(Mat& in, Mat& inverse)
{
#ifndef USE_BLAS_SPECIALISATIONS
    PLERROR("eigen_SymmMat: LAPACK not available on this system!");
    return 0;
#else
    // PLWARNING("matInvert: Your input matrix will be over-written!");

    // some check
    if (in.length() != in.width())
        PLERROR("The input matrix [%dx%d] must be square!", in.length(), in.width());
    // for the moment, we do not accept sub-matrices...
    if (in.mod() != in.width())
        PLERROR("The input matrix cannot be a sub-matrix...");

    int M = in.length();
    int N = in.length();
    real* A = in.data();
    int LDA = N;
    int* IPIV = new int[N];
    int INFO;

#ifdef USEFLOAT
    sgetrf_(&M, &N, A, &LDA, IPIV, &INFO);
#endif
#ifdef USEDOUBLE
    dgetrf_(&M, &N, A, &LDA, IPIV, &INFO);
#endif

    if (INFO != 0)
    {
        cout << "In matInvert: Error doing the inversion." << endl
             << "Check the man page of <sgetrf> with error code " << INFO
             << " for more details." << endl;

        delete[] IPIV;
        return INFO;
    }

    int LWORK = N;
    real* WORK = new real[LWORK];

#ifdef USEFLOAT
    sgetri_(&N, A, &LDA, IPIV, WORK, &LWORK, &INFO);
#endif
#ifdef USEDOUBLE
    dgetri_(&N, A, &LDA, IPIV, WORK, &LWORK, &INFO);
#endif

    if (INFO != 0)
    {
        cout << "In matInvert: Error doing the inversion." << endl
             << "Check the man page of <sgetri> with error code " << INFO
             << " for more details." << endl;

        delete[] IPIV;
        delete[] WORK;
        return INFO;
    }

    delete[] IPIV;
    delete[] WORK;

    real* p_inverse = inverse.data();
    real* p_A = A;
    for (int i=0; i<N; i++)
        for (int j=0; j<M; j++, p_inverse++, p_A++)
            *p_inverse = *p_A;

    return INFO;
#endif
}


int lapackSolveLinearSystem(Mat& At, Mat& Bt, TVec<int>& pivots)
{
#ifdef BOUNDCHECK
    if(At.width() != Bt.width())
        PLERROR("In lapackSolveLinearSystem: Incompatible dimensions");
#endif

    int INFO;
#ifndef USE_BLAS_SPECIALISATIONS
    PLERROR("lapackSolveLinearSystem: can't be called unless PLearn linked with LAPACK");
#else
    int N = At.width();
    int NRHS = Bt.length();
    real* Aptr = At.data();
    int LDA = At.mod();
    if(pivots.length()!=N)
        pivots.resize(N);
    int* IPIVptr = pivots.data();
    real* Bptr = Bt.data();
    int LDB = Bt.mod();
#ifdef USEFLOAT
    sgesv_(&N, &NRHS, Aptr, &LDA, IPIVptr, Bptr, &LDB, &INFO);
#endif
#ifdef USEDOUBLE
    dgesv_(&N, &NRHS, Aptr, &LDA, IPIVptr, Bptr, &LDB, &INFO);
#endif
#endif
    return INFO;
}

// for matrices A such that A.length() <= A.width(),
// find X s.t. A X = Y
void solveLinearSystem(const Mat& A, const Mat& Y, Mat& X)
{
    PLERROR("solveLinearSystem: not implemented yet");
}

// for matrices A such that A.length() >= A.width(),
// find X s.t. X A = Y
void solveTransposeLinearSystem(const Mat& A, const Mat& Y, Mat& X)
{
    PLERROR("solveTransposeLinearSystem: not implemented yet");
}

Mat solveLinearSystem(const Mat& A, const Mat& B)
{
    Mat Bt = transpose(B);
    Mat At = transpose(A);
    TVec<int> pivots(A.length());
    int status = lapackSolveLinearSystem(At,Bt,pivots);
    if(status<0)
        PLERROR("Illegal value in argument of lapackSolveLinearSystem");
    else if(status>0)
        PLERROR("In solveLinearSystem: The factorization has been completed, but the factor U is exactly singular, so the solution could not be computed.");
    return transpose(Bt); // return X
}

Vec solveLinearSystem(const Mat& A, const Vec& b)
{ return solveLinearSystem(A,b.toMat(b.length(),1)).toVec(); }


/*
  real hyperplane_distance(Vec x, Mat points)
  {
  if(x.length()!=points.width())
  PLERROR("In hyperplane_distance, incompatible dimensions");
  Vec ref = points(0);
  Mat tangentvecs = points.subMatRows(1,points.length()-1).copy();
  tangentvecs -= ref;
  Mat A = productTranspose(tangentvecs,tangentvecs);
  Vec b = product(tangentvecs,x-ref);
  Vec alpha(tangentvecs.length());
  Mat alphamat(alpha.length(),1,alpha);
  solveLinearSystemByCholesky(A,Mat(b.length(),1,b),alphamat);
  return norm(ref + transposeProduct(tangentvecs,alpha) - x);
  }
*/

// Returns w that minimizes ||X.w - Y||^2 + lambda.||w||^2
// under constraint \sum w_i = 1
// Xt is the transposed of the input matrix X; Y is the target vector.
// This doesn't include any bias term.
Vec constrainedLinearRegression(const Mat& Xt, const Vec& Y, real lambda)
{
    if(Y.length()!=Xt.width())
        PLERROR("In hyperplane_distance, incompatible dimensions");

    int n = Xt.length();
    Mat A(n+1,n+1);
    Vec b(n+1);

    for(int i=0; i<n; i++)
    {
        A(n,i) = 0.5;
        A(i,n) = 0.5;
        b[i] = dot(Y,Xt(i));
        for(int j=0; j<n; j++)
        {
            real dotprod = dot(Xt(i),Xt(j));
            if(i!=j)
                A(i,j) = dotprod;
            else
                A(i,j) = dotprod + lambda;
        }
    }
    A(n,n) = 0.;
    b[n] = 0.5;
    
    // cerr << "A = " << A << endl;
    // cerr << "b = " << b << endl;
    // cerr << "b\\A = " << solveLinearSystem(A,b) << endl;

    Vec w_and_l = solveLinearSystem(A,b);
    return w_and_l.subVec(0,n); // return w
}



Mat multivariate_normal(const Vec& mu, const Mat& A, int N)
{
    Vec e_values;
    Mat e_vectors;
    Mat A_copy = A.copy(); 
    int nb_evalues_found;
    eigen_SymmMat(A_copy, e_values, e_vectors, nb_evalues_found, true, mu.length(), true);
    Mat samples(0,mu.length());
    for (int i = 0; i < N; i++)
        samples.appendRow(multivariate_normal(mu, e_values, e_vectors));
    return samples;
}

Vec multivariate_normal(const Vec& mu, const Mat& A)
{
    return multivariate_normal(mu, A, 1).toVec(); 
}

Vec multivariate_normal(const Vec& mu, const Vec& e_values, const Mat& e_vectors)
{
    int n = mu.length(); // the number of dimension
    Vec z(n), x(n);
    for (int i = 0; i < n; i++)
        z[i] = gaussian_01();
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
            x[i] += e_vectors[j][i] * sqrt(e_values[j]) * z[j]; 
        x[i] += mu[i];
    }
    return x;
}

void multivariate_normal(Vec& x, const Vec& mu, const Vec& e_values, const Mat& e_vectors, Vec& z)
{
    int n = mu.length(); // the number of dimension
    z.resize(n);
    x.resize(n);
    x.clear();
    for (int i = 0; i < n; i++)
        z[i] = gaussian_01();
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
            x[i] += e_vectors[j][i] * sqrt(e_values[j]) * z[j]; 
        x[i] += mu[i];
    }
}

void affineNormalization(Mat data, Mat W, Vec bias, real regularizer)
{
    int d=data.width();
    Vec& mu = bias;
    Mat covar(d,d);
    computeMeanAndCovar(data,mu,covar);
    Vec evalues(d);
    if (!fast_exact_is_equal(regularizer, 0))
        for (int i=0;i<d;i++)
            covar(i,i) += regularizer; 
    int nev=0;
    eigen_SymmMat(covar,evalues,W,nev,true,d,true,true);
    for (int i=0;i<d;i++)
        W(i) *= real(1.0 / sqrt(evalues[i]));
    mu *= - real(1.0); // bias = -mu
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
