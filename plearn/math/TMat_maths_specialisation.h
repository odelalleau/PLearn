// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Pascal Vincent

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
 * AUTHORS: Pascal Vincent & Yoshua Bengio & Rejean Ducharme
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file TMat_maths_specialisation.h */

#ifndef TMat_maths_specialisation_INC
#define TMat_maths_specialisation_INC

#include "TMat.h"
#include <plearn/sys/Profiler.h>

namespace PLearn {
using namespace std;

//#define USE_BLAS_SPECIALISATIONS

#ifdef USE_BLAS_SPECIALISATIONS
#include "blas_proto.h"

#ifdef USEDOUBLE
#define BLAS_COPY dcopy_
#define BLAS_MULT_ACC daxpy_
#define BLAS_SCALE dscal_
#define BLAS_SWAP dswap_
#else
#define BLAS_COPY scopy_
#define BLAS_MULT_ACC saxpy_
#define BLAS_SCALE sscal_
#define BLAS_SWAP sswap_
#endif

/* Commented out. It is not clear exactly (1) if it works, (2) where it would
 * be called, and (3) if it is more efficient.
//////////
// copy //
//////////
inline real* copy(real* first, real* last, real* dest)
{
    int n = last - first;
    int one = 1;
    BLAS_COPY(&n, first, &one, dest, &one);
    return dest + n;
}
*/

/////////////////
// multiplyAcc //
/////////////////
//! vec += x * scale
inline void multiplyAcc(const Vec& vec, const Vec& x, real scale)
{
    int n = vec.length();
    PLASSERT( vec.length() == x.length() );
    int one = 1;
    BLAS_MULT_ACC(&n, &scale, x.data(), &one, vec.data(), &one);
}

inline void multiplyAcc(const Mat& mat, const Mat& x, real scale)
{
    PLASSERT( mat.length() == x.length() && mat.width() == x.width() );

    int one = 1;
    int w = mat.width(); // == x.width()
    int mod_mat = mat.mod();
    real* data_mat = mat.data();
    int mod_x = x.mod();
    real* data_x = x.data();

    if( mat.isEmpty() ) // x.isEmpty() too
        return;

    if( w == mod_mat && w == mod_x )
    {
        // The two matrices have contiguous rows, we do it in one call
        int n = mat.size(); // == x.size()
        BLAS_MULT_ACC(&n, &scale, data_x, &one, data_mat, &one);
    }
    else if( w == 1 )
    {
        // There is only one column, we do it in one call
        int l = mat.length(); // == x.length(), == mat.size()
        BLAS_MULT_ACC(&l, &scale, data_x, &mod_x, data_mat, &mod_mat);
    }
    else
    {
        // We iterate over the rows
        int l = mat.length(); // == x.length()
        for( int i=0 ; i<l ; i++, data_mat += mod_mat, data_x += mod_x )
            BLAS_MULT_ACC(&w, &scale, data_x, &one, data_mat, &one);
    }
}

/*
  inline void operator+=(const TVec<double>& vec, const TVec<double>& x)
  { multiplyAcc(vec,x,1.); }
*/

////////////////
// operator*= //
////////////////
inline void operator*=(const Vec& vec, real factor)
{
    int mod = 1;
    int n = vec.length();
    if ( n != 0 )
        BLAS_SCALE(&n, &factor, vec.data(), &mod);
}

inline void operator*=(const Mat& mat, real factor)
{
    if (mat.isEmpty())
        return;

    int one = 1;
    int w = mat.width();
    int mod = mat.mod();
    real* data = mat.data();
    if (w == mod)
    {
        // The rows are contiguous, so we can do it in one call
        int n = mat.size();
        BLAS_SCALE(&n, &factor, data, &one);
    }
    else
    {
        // We iterate over the rows
        int l = mat.length();
        for( int i=0 ; i<l ; i++, data += mod )
            BLAS_SCALE(&w, &factor, data, &one);
    }
}

//////////////
// swapRows //
//////////////
inline void swapRows(const Mat& mat, int i, int j)
{
    if (i == j)
        return;
    real* mat_row_i = mat[i];
    real* mat_row_j = mat[j];
    int one = 1;
    int n = mat.width();
    BLAS_SWAP(&n, mat_row_i, &one, mat_row_j, &one);
}

/////////////////////
// productScaleAcc //
/////////////////////
//! C = alpha A.B + beta C
// (Will use the transpose of A and/or B instead, if you set the corresponding
// flags to true)
inline void productScaleAcc(const TMat<double>& C,
                            const TMat<double>& A, bool transposeA,
                            const TMat<double>& B, bool transposeB,
                            double alpha, double beta)
{
    Profiler::pl_profile_start("productScaleAcc(dgemm) specialisation");
#ifdef BOUNDCHECK
    int l2;
#endif
    int l1, w1, w2;
    char transa, transb;
    if(transposeA)
    {
        l1 = A.width();
        w1 = A.length();
        transa = 'T';
    }
    else
    {
        l1 = A.length();
        w1 = A.width();
        transa = 'N';
    }
    if(transposeB)
    {
#ifdef BOUNDCHECK
        l2 = B.width();
#endif
        w2 = B.length();
        transb = 'T';
    }
    else
    {
#ifdef BOUNDCHECK
        l2 = B.length();
#endif
        w2 = B.width();
        transb = 'N';
    }

#ifdef BOUNDCHECK
    if (w1!=l2 || C.length()!=l1 || C.width()!=w2)
        PLERROR("productScaleAcc, incompatible arguments:\n"
                "(%dx%d) <- %s(%dx%d) . %s(%dx%d)",
                C.length(), C.width(),
                transposeA?"transpose":"", A.length(), A.width(),
                transposeB?"transpose":"", B.length(), B.width());
#endif

    int lda = A.mod();
    int ldb = B.mod();
    int ldc = C.mod();

    if (A.isEmpty() || B.isEmpty() || C.isEmpty()) {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-matrix multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!C.isEmpty())
            C *= beta;
        return;
    }

    dgemm_(&transb, &transa, &w2, &l1, &w1, &alpha, B.data(), &ldb, A.data(),
           &lda, &beta, C.data(), &ldc);
    Profiler::pl_profile_end("productScaleAcc(dgemm) specialisation");
}

//! y <- alpha A.x + beta y 
// (will use the transposed of A instead if tranposeA is true)
inline void productScaleAcc(const TVec<double>& y,
                            const TMat<double>& A, bool transposeA,
                            const TVec<double>& x, double alpha, double beta)
{
    Profiler::pl_profile_start("productScaleAcc(dgemv_) specialisation");
#ifdef BOUNDCHECK
    if(!transposeA)
    {
        if(A.length()!=y.length() || A.width()!=x.length())
            PLERROR("productScaleAcc, incompatible arguments:\n"
                    "Vec(%d) <- Mat(%d,%d) . Vec(%d)",
                    y.length(), A.length(), A.width(), x.length());
    }
    else
    {
        if(A.length()!=x.length() || A.width()!=y.length())
            PLERROR("productScaleAcc, incompatible arguments:\n"
                    "Vec(%d) <- Mat(%d,%d)' . Vec(%d)",
                    y.length(), A.length(), A.width(), x.length());
    }
#endif

    int one = 1;
    char trans = transposeA ?'N' :'T';
    int lda = A.mod();
    int m = A.width();
    int n = A.length();

    if (A.isEmpty() || x.isEmpty() || y.isEmpty()) {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-vector multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!y.isEmpty())
            y *= beta;
        return;
    }

    dgemv_(&trans, &m, &n, &alpha, A.data(), &lda, x.data(), &one, &beta,
           y.data(), &one);
    Profiler::pl_profile_end("productScaleAcc(dgemv_) specialisation");
}

//! A <- A + alpha x.y'
inline void externalProductScaleAcc(const TMat<double>& A,
                                    const TVec<double>& x,
                                    const TVec<double>& y, double alpha)
{
    Profiler::pl_profile_start("externalProductScaleAcc(dger_) double specialisation");

#ifdef BOUNDCHECK
    if(A.length()!=x.length() || A.width()!=y.length())
        PLERROR("In externalProductScaleAcc, incompatible dimensions:\n"
                "Mat(%d,%d) <- Vec(%d).Vec(%d)'",
                A.length(), A.width(), x.length(), y.length());
    if(A.mod()<=0 || A.width()<=0)
        PLERROR("In externalProductScaleAcc, destination matrix has a width "
                "(%d) or a mod (%d) <= 0", A.width(), A.mod());
#endif
    int one = 1;
    int lda = A.mod();
    int m = A.width();
    int n = A.length();

    if (A.isNull() || x.isNull() || y.isNull()  // Size zero ; don't bother
        || m == 0 || n == 0)                    // with actual calculation
        return;

    dger_(&m, &n, &alpha, y.data(), &one, x.data(), &one, A.data(), &lda);
    Profiler::pl_profile_end("externalProductScaleAcc(dger_) double specialisation");
}

inline void externalProductAcc(const TMat<double>& A,
                               const TVec<double>& x,
                               const TVec<double>& y)
{ externalProductScaleAcc(A, x, y, 1.); }

inline void product(const TVec<double>& vec, const TMat<double>& m,
                    const TVec<double>& v)
{ productScaleAcc(vec, m, false, v, 1., 0.); }

inline void productAcc(const TVec<double>& vec, const TMat<double>& m,
                       const TVec<double>& v)
{ productScaleAcc(vec, m, false, v, 1., 1.); }

inline void productScaleAcc(const TVec<double>& vec, const TMat<double>& m,
                            const TVec<double>& v, double alpha, double beta)
{ productScaleAcc(vec, m, false, v, alpha, beta); }

inline void transposeProduct(const TVec<double>& vec, const TMat<double>& m,
                             const TVec<double>& v)
{ productScaleAcc(vec, m, true, v, 1., 0.); }

inline void transposeProductAcc(const TVec<double>& vec, const TMat<double>& m,
                                const TVec<double>& v)
{ productScaleAcc(vec, m, true, v, 1., 1.); }

inline void transposeProductScaleAcc(const TVec<double>& vec,
                                     const TMat<double>& m,
                                     const TVec<double>& v,
                                     double alpha, double beta)
{ productScaleAcc(vec, m, true, v, alpha, beta); }

inline void product(const TMat<double>& mat, const TMat<double>& m1,
                    const TMat<double>& m2)
{ productScaleAcc(mat, m1, false, m2, false, 1., 0.); }

inline void transposeTransposeProduct(const TMat<double>& mat,
                                      const TMat<double>& m1,
                                      const TMat<double>& m2)
{ productScaleAcc(mat, m1, true, m2, true, 1., 0.); }

inline void transposeProduct(const TMat<double>& mat, const TMat<double>& m1,
                             const TMat<double>& m2)
{ productScaleAcc(mat, m1, true, m2, false, 1., 0.); }

inline void productTranspose(const TMat<double>& mat, const TMat<double>& m1,
                             const TMat<double>& m2)
{ productScaleAcc(mat, m1, false, m2, true, 1., 0.); }

inline void productAcc(const TMat<double>& mat, const TMat<double>& m1,
                       const TMat<double>& m2)
{ productScaleAcc(mat, m1, false, m2, false, 1., 1.); }

inline void productScaleAcc(const TMat<double>& mat,
                            const TMat<double>& m1, const TMat<double>& m2,
                            double alpha, double beta)
{ productScaleAcc(mat, m1, false, m2, false, alpha, beta); }

inline void transposeTransposeProductAcc(const TMat<double>& mat,
                                         const TMat<double>& m1,
                                         const TMat<double>& m2)
{ productScaleAcc(mat, m1, true, m2, true, 1., 1.); }

inline void transposeTransposeProductScaleAcc(const TMat<double>& mat,
                                              const TMat<double>& m1,
                                              const TMat<double>& m2,
                                              double alpha, double beta)
{ productScaleAcc(mat, m1, true, m2, true, alpha, beta); }

inline void transposeProductAcc(const TMat<double>& mat,
                                const TMat<double>& m1,
                                const TMat<double>& m2)
{ productScaleAcc(mat, m1, true, m2, false, 1., 1.); }

inline void transposeProductScaleAcc(const TMat<double>& mat,
                                     const TMat<double>& m1,
                                     const TMat<double>& m2,
                                     double alpha, double beta)
{ productScaleAcc(mat, m1, true, m2, false, alpha, beta); }

inline void productTransposeAcc(const TMat<double>& mat,
                                const TMat<double>& m1,
                                const TMat<double>& m2)
{ productScaleAcc(mat, m1, false, m2, true, 1., 1.); }

inline void productTransposeScaleAcc(const TMat<double>& mat,
                                     const TMat<double>& m1,
                                     const TMat<double>& m2,
                                     double alpha, double beta)
{ productScaleAcc(mat, m1, false, m2, true, alpha, beta); }



// float


//! C = alpha A.B + beta C
// (Will use the transpose of A and/or B instead, if you set the corresponding
// flags to true)
inline void productScaleAcc(const TMat<float>& C,
                            const TMat<float>& A, bool transposeA,
                            const TMat<float>& B, bool transposeB,
                            float alpha, float beta)
{
    Profiler::pl_profile_start("productScaleAcc(sgemm) specialisation");

#ifdef BOUNDCHECK
    int l2;
#endif
    int l1, w1, w2;
    char transa, transb;
    if(transposeA)
    {
        l1 = A.width();
        w1 = A.length();
        transa = 'T';
    }
    else
    {
        l1 = A.length();
        w1 = A.width();
        transa = 'N';
    }
    if(transposeB)
    {
#ifdef BOUNDCHECK
        l2 = B.width();
#endif
        w2 = B.length();
        transb = 'T';
    }
    else
    {
#ifdef BOUNDCHECK
        l2 = B.length();
#endif
        w2 = B.width();
        transb = 'N';
    }

#ifdef BOUNDCHECK
    if (w1!=l2 || C.length()!=l1 || C.width()!=w2)
        PLERROR("productScaleAcc, incompatible arguments:\n"
                "(%dx%d) <- %s(%dx%d) . %s(%dx%d)",
                C.length(), C.width(),
                transposeA?"transpose":"", A.length(), A.width(),
                transposeB?"transpose":"", B.length(), B.width());
#endif

    int lda = A.mod();
    int ldb = B.mod();
    int ldc = C.mod();

    if (A.isNull() || B.isNull() || C.isNull()) // Size zero ; don't bother
        return;                                 // with actual calculation

    sgemm_(&transb, &transa, &w2, &l1, &w1, &alpha, B.data(), &ldb, A.data(),
           &lda, &beta, C.data(), &ldc);
    Profiler::pl_profile_end("productScaleAcc(sgemm) specialisation");
}

//! y <- alpha A.x + beta y
// (will use the transposed of A instead if tranposeA is true)
inline void productScaleAcc(const TVec<float>& y,
                            const TMat<float>& A, bool transposeA,
                            const TVec<float>& x, float alpha, float beta)
{
    Profiler::pl_profile_start("productScaleAcc(sger_) specialisation");
#ifdef BOUNDCHECK
    if(!transposeA)
    {
        if(A.length()!=y.length() || A.width()!=x.length())
            PLERROR("productScaleAcc, incompatible arguments:\n"
                    "Vec(%d) <- Mat(%d,%d) . Vec(%d)",
                    y.length(), A.length(), A.width(), x.length());
    }
    else
    {
        if(A.length()!=x.length() || A.width()!=y.length())
            PLERROR("productScaleAcc, incompatible arguments:\n"
                    "Vec(%d) <- Mat(%d,%d)' . Vec(%d)",
                    y.length(), A.length(), A.width(), x.length());
    }
#endif

      int one = 1;
      char trans = transposeA ?'N' :'T';
      int lda = A.mod();
      int m = A.width();
      int n = A.length();

      sgemv_(&trans, &m, &n, &alpha, A.data(), &lda, x.data(), &one, &beta,
             y.data(), &one);
    Profiler::pl_profile_end("productScaleAcc(sger_) specialisation");
}

//! A <- A + alpha x.y'
inline void externalProductScaleAcc(const TMat<float>& A, const TVec<float>& x,
                                    const TVec<float>& y, float alpha)
{
    Profiler::pl_profile_start("externalProductScaleAcc(sger_) float specialisation");
#ifdef BOUNDCHECK
    if(A.length()!=x.length() || A.width()!=y.length())
        PLERROR("In externalProductScaleAcc, incompatible dimensions:\n"
                "Mat(%d,%d) <- Vec(%d).Vec(%d)'",
                A.length(), A.width(), x.length(), y.length());
#endif
    int one = 1;
    int lda = A.mod();
    int m = A.width();
    int n = A.length();

    if (A.isNull() || x.isNull() || y.isNull()) // Size zero ; don't bother
        return;                                 // with actual calculation

    sger_(&m, &n, &alpha, y.data(), &one, x.data(), &one, A.data(), &lda);
    Profiler::pl_profile_end("externalProductScaleAcc(sger_) float specialisation");
}

inline void externalProductAcc(const TMat<float>& A, const TVec<float>& x,
                               const TVec<float>& y)
{ externalProductScaleAcc(A, x, y, 1.); }

inline void product(const TVec<float>& vec, const TMat<float>& m,
                    const TVec<float>& v)
{ productScaleAcc(vec, m, false, v, 1., 0.); }

inline void productAcc(const TVec<float>& vec, const TMat<float>& m,
                       const TVec<float>& v)
{ productScaleAcc(vec, m, false, v, 1., 1.); }

inline void productScaleAcc(const TVec<float>& vec, const TMat<float>& m,
                            const TVec<float>& v, float alpha, float beta)
{ productScaleAcc(vec, m, false, v, alpha, beta); }

inline void transposeProduct(const TVec<float>& vec, const TMat<float>& m,
                             const TVec<float>& v)
{ productScaleAcc(vec, m, true, v, 1., 0.); }

inline void transposeProductAcc(const TVec<float>& vec, const TMat<float>& m,
                                const TVec<float>& v)
{ productScaleAcc(vec, m, true, v, 1., 1.); }

inline void transposeProductScaleAcc(const TVec<float>& vec,
                                     const TMat<float>& m,
                                     const TVec<float>& v,
                                     float alpha, float beta)
{ productScaleAcc(vec, m, true, v, alpha, beta); }

inline void product(const TMat<float>& mat, const TMat<float>& m1,
                    const TMat<float>& m2)
{ productScaleAcc(mat, m1, false, m2, false, 1., 0.); }

inline void transposeTransposeProduct(const TMat<float>& mat,
                                      const TMat<float>& m1,
                                      const TMat<float>& m2)
{ productScaleAcc(mat, m1, true, m2, true, 1., 0.); }

inline void transposeProduct(const TMat<float>& mat, const TMat<float>& m1,
                             const TMat<float>& m2)
{ productScaleAcc(mat, m1, true, m2, false, 1., 0.); }

inline void productTranspose(const TMat<float>& mat, const TMat<float>& m1,
                             const TMat<float>& m2)
{ productScaleAcc(mat, m1, false, m2, true, 1., 0.); }

inline void productAcc(const TMat<float>& mat, const TMat<float>& m1,
                       const TMat<float>& m2)
{ productScaleAcc(mat, m1, false, m2, false, 1., 1.); }

inline void productScaleAcc(const TMat<float>& mat,
                            const TMat<float>& m1, const TMat<float>& m2,
                            float alpha, float beta)
{ productScaleAcc(mat, m1, false, m2, false, alpha, beta); }

inline void transposeTransposeProductAcc(const TMat<float>& mat,
                                         const TMat<float>& m1,
                                         const TMat<float>& m2)
{ productScaleAcc(mat, m1, true, m2, true, 1., 1.); }

inline void transposeTransposeProductScaleAcc(const TMat<float>& mat,
                                              const TMat<float>& m1,
                                              const TMat<float>& m2,
                                              float alpha, float beta)
{ productScaleAcc(mat, m1, true, m2, true, alpha, beta); }

inline void transposeProductAcc(const TMat<float>& mat, const TMat<float>& m1,
                                const TMat<float>& m2)
{ productScaleAcc(mat, m1, true, m2, false, 1., 1.); }

inline void transposeProductScaleAcc(const TMat<float>& mat,
                                     const TMat<float>& m1,
                                     const TMat<float>& m2,
                                     float alpha, float beta)
{ productScaleAcc(mat, m1, true, m2, false, alpha, beta); }

inline void productTransposeAcc(const TMat<float>& mat, const TMat<float>& m1,
                                const TMat<float>& m2)
{ productScaleAcc(mat, m1, false, m2, true, 1., 1.); }

inline void productTransposeScaleAcc(const TMat<float>& mat,
                                     const TMat<float>& m1,
                                     const TMat<float>& m2,
                                     float alpha, float beta)
{ productScaleAcc(mat, m1, false, m2, true, alpha, beta); }


#endif // USE_BLAS_SPECIALISATIONS

// strange little functions of Yoshua for optimized computations in neural nets

#define UNFOLD

// dot product, assumes that s is already initialized
//  return s + sum_{i=0}^{n-1} x[i]*y[i]
inline real dot_product(real s,real* x,real* y,int n)
{
#ifdef UNFOLD
    int n4 = (n >> 2) << 2;
    int i=0;
    for (;i<n4;i+=4)
    {
        real s1 = x[i] * y[i];
        real s2 = x[i+1] * y[i+1];
        real s3 = x[i+2] * y[i+2];
        real s4 = x[i+3] * y[i+3];
        s += s1+s2+s3+s4;
    }
    for (;i<n;i++)
        s += x[i] * y[i];
#else
    for (int i=0;i<n;i++)
        s += *x++ * *y++;
#endif
    return s;
}

// norman: sse is not supported in WIN32
#if defined(SGI) && !defined(WIN32)
#include <plearn/sys/sse.h>
#endif //ndef SGI

//#define BUNDLE
// dx[j] += sum_i dy[i]*w(i,j)
// w(i,j) -= learning_rate*(dy[i]*x[j] + weight_decay*w(i,j))
inline void bprop_update_layer(real* dy, real* x, real* dx, real* w,
                               int n_y, int n_x, real learning_rate,
                               real weight_decay)
{
#ifdef BUNDLE
    int nx8 = (n_x >> 3) << 3;
    int j8=0;
    real* xj = x;
    real* dxj = dx;
    int delta_w1 = n_x - 8;
    int delta_w2 = n_y*n_x - 8;
    real* w_ij = w;
    for (;j8<nx8;j8+=8,xj+=8,dxj+=8,w_ij-=delta_w2)
    {
        real* dy_ = dy;
        for (int i=0;i<n_y;i++)
        {
            real* next_w = w_ij + delta_w1;
            prefetchnta(*next_w);
            real* x_j = xj;
            real* dx_j = dxj;
            real d_y = dy_[i];
            *dx_j   += d_y * *w_ij;
            *w_ij   -= learning_rate*(d_y * *x_j + weight_decay * *w_ij);
            dx_j[1] += d_y * w_ij[1];
            w_ij[1] -= learning_rate*(d_y * x_j[1] + weight_decay * w_ij[1]);
            dx_j[2] += d_y * w_ij[2];
            w_ij[2] -= learning_rate*(d_y * x_j[2] + weight_decay * w_ij[2]);
            dx_j[3] += d_y * w_ij[3];
            w_ij[3] -= learning_rate*(d_y * x_j[3] + weight_decay * w_ij[3]);
            dx_j[4] += d_y * w_ij[4];
            w_ij[4] -= learning_rate*(d_y * x_j[4] + weight_decay * w_ij[4]);
            dx_j[5] += d_y * w_ij[5];
            w_ij[5] -= learning_rate*(d_y * x_j[5] + weight_decay * w_ij[5]);
            dx_j[6] += d_y * w_ij[6];
            w_ij[6] -= learning_rate*(d_y * x_j[6] + weight_decay * w_ij[6]);
            dx_j[7] += d_y * w_ij[7];
            w_ij[7] -= learning_rate*(d_y * x_j[7] + weight_decay * w_ij[7]);
            w_ij = next_w;
        }
    }
    for (int i=0;i<n_y;i++)
    {
        real dy_i = dy[i];
        real *dx_j = dx;
        real *x_j = x;
        for (int j=j8;j<n_x;j++)
        {
            *dx_j++ += dy_i * *w;
            *w++ -= learning_rate*(dy_i * *x_j++ + weight_decay * *w);
        }
    }

#else
#ifdef UNFOLD
    int nx4 = (n_x >> 2) << 2;
    real *w_i = w;
    for (int i=0;i<n_y;i++,w_i+=n_x)
    {
        real dy_i = dy[i];
        real *dx_j = dx;
        real *x_j = x;
        int j=0;
        for (;j<nx4;j+=4)
        {
            real w_ij0 = w_i[j];
            real w_ij1 = w_i[j+1];
            real w_ij2 = w_i[j+2];
            real w_ij3 = w_i[j+3];
            dx_j[j] += dy_i * w_ij0;
            dx_j[j+1] += dy_i * w_ij1;
            dx_j[j+2] += dy_i * w_ij2;
            dx_j[j+3] += dy_i * w_ij3;
            w_i[j] -= learning_rate*(dy_i * x_j[j] + weight_decay * w_ij0);
            w_i[j+1] -= learning_rate*(dy_i * x_j[j+1] + weight_decay * w_ij1);
            w_i[j+2] -= learning_rate*(dy_i * x_j[j+2] + weight_decay * w_ij2);
            w_i[j+3] -= learning_rate*(dy_i * x_j[j+3] + weight_decay * w_ij3);
        }
        for (;j<n_x;j++)
        {
            real w_ij = w_i[j];
            dx_j[j] += dy_i * w_ij;
            w_i[j] -= learning_rate*(dy_i * x_j[j] + weight_decay * w_ij);
        }
    }
#else
    for (int i=0;i<n_y;i++)
    {
        real dy_i = dy[i];
        real *dx_j = dx;
        real *x_j = x;
        for (int j=0;j<n_x;j++)
        {
            *dx_j++ += dy_i * *w;
            *w++ -= learning_rate*(dy_i * *x_j++ + weight_decay * *w);
        }
    }
#endif
#endif
}

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
