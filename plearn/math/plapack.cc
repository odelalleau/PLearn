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
        char JOBZ;
        if (compute_vectors)
            JOBZ = 'V';
        else
            JOBZ = 'N';
        char UPLO = 'U';
        int N = in.length();
        real* A = in.data();
        int LDA = N;
        real* W = new real[N];
        int LWORK = 3*N;
        real* WORK = new real[LWORK];

        // we now call the LAPACK Fortran function <ssyev>
#ifdef USEFLOAT
        ssyev_(&JOBZ, &UPLO, &N, A, &LDA, W, WORK, &LWORK, &INFO);
#endif
#ifdef USEDOUBLE
        dsyev_(&JOBZ, &UPLO, &N, A, &LDA, W, WORK, &LWORK, &INFO);
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
        char JOBZ;
        if (compute_vectors)
            JOBZ = 'V';
        else
            JOBZ = 'N';
        char RANGE = 'I';
        char UPLO = 'U';
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
        lapack_Xsyevx_(&JOBZ, &RANGE, &UPLO, &N, A, &LDA, &VL, &VU, &IL, &IU, &ABSTOL, &M, W, Z, &LDZ, WORK, &LWORK, IWORK, IFAIL, &INFO);

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

///////////////
// matInvert //
///////////////
int matInvert(Mat& in, Mat& inverse)
{
    // If the matrix is empty, just do nothing instead of crashing.
    if (in.isEmpty()) {
        PLASSERT( inverse.isEmpty() );
        return 0;
    }

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

    real* p_A = A;
    for (int i=0; i<N; i++) {
        real* p_inverse = inverse[i];
        for (int j=0; j<M; j++, p_inverse++, p_A++)
            *p_inverse = *p_A;
    }

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


//#####  lapackCholeskyDecompositionInPlace  ##################################

void lapackCholeskyDecompositionInPlace(Mat& A, char uplo)
{
    if (A.width() == 0 || A.length() == 0)
        return;
    if (A.mod() != A.width())
        PLERROR("lapackCholeskyDecompositionInPlace: matrix mod (%d) must equal "
                "its width (%d)", A.mod(), A.width());
    if (A.width() != A.length())
        PLERROR("lapackCholeskyDecompositionInPlace: matrix width (%d) and height (%d) "
                "must be equal", A.width(), A.length());

    char lapack_uplo;
    switch (uplo) {
    case 'L':
    case 'l':
        lapack_uplo = 'U';
        break;

    case 'U':
    case 'u':
        lapack_uplo = 'L';
        break;

    default:
        PLERROR("lapackCholeskyDecompositionInPlace: unrecognized character '%c' for "
                "argument 'uplo'; valid characters are 'U' and 'L'", uplo);
    }

    real* data = A.data();
    int N = A.width();
    int INFO;

    // call LAPACK
    lapack_Xpotrf_(&lapack_uplo, &N, data, &N, &INFO);

    if (INFO == 0)
        return;                              // all successful
    else if (INFO < 0)
        PLERROR("lapackCholeskyDecompositionInPlace: implementation error; argument %d "
                "to xPOTRF had an illegal value", -INFO);
    else
        PLERROR("lapackCholeskyDecompositionInPlace: error in decomposition; "
                "leading minor of order %d is not positive definite, "
                "and the factorization could not be completed.", INFO);
}


//#####  lapackCholeskySolveInPlace  ##########################################

void lapackCholeskySolveInPlace(Mat& A, Mat& B, bool B_is_column_major, char uplo)
{
    if (A.width() == 0 || A.length() == 0 || B.width() == 0 || B.length() == 0)
        return;
    if (A.mod() != A.width())
        PLERROR("lapackCholeskySolveInPlace: matrix A mod (%d) must equal "
                "its width (%d)", A.mod(), A.width());
    if (B.mod() != B.width())
        PLERROR("lapackCholeskySolveInPlace: matrix B mod (%d) must equal "
                "its width (%d)", B.mod(), B.width());
    if (A.width() != A.length())
        PLERROR("lapackCholeskySolveInPlace: matrix width (%d) and height (%d) "
                "must be equal", A.width(), A.length());
    if ((! B_is_column_major && B.length() != A.length()) ||
        (  B_is_column_major && B.width()  != A.length()) )
        PLERROR("lapackCholeskySolveInPlace: matrix B length (%d) is "
                "incompatible with the dimensions of A (%d)",
                (B_is_column_major? B.width() : B.length()), A.length());

    char lapack_uplo;
    switch (uplo) {
    case 'L':
    case 'l':
        lapack_uplo = 'U';
        break;

    case 'U':
    case 'u':
        lapack_uplo = 'L';
        break;

    default:
        PLERROR("lapackCholeskySolveInPlace: unrecognized character '%c' for "
                "argument 'uplo'; valid characters are 'U' and 'L'", uplo);
    }

    // If B is not column-major, transpose it
    Mat lapack_B;
    if (! B_is_column_major)
        lapack_B = transpose(B);
    else
        lapack_B = B;

    // Prepare for call to LAPACK
    int N    = A.width();
    int NRHS = lapack_B.length();   // Don't forget it's transposed for lapack
    int LDA  = A.length();
    int LDB  = lapack_B.width();
    int INFO;
    real* A_data = A.data();
    real* B_data = lapack_B.data();

    // Call LAPACK
    lapack_Xpotrs_(&lapack_uplo, &N, &NRHS, A_data, &LDA, B_data, &LDB, &INFO);

    if (INFO < 0)
        PLERROR("lapackCholeskySolvePlace: implementation error; argument %d "
                "to xPOTRS had an illegal value", -INFO);
    PLASSERT( INFO == 0 );

    // If B was not originally column-major, transpose back result from LAPACK
    if (! B_is_column_major)
        transpose(lapack_B, B);
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

// COMMENTED OUT BECAUSE INCORRECT COMPUTATION OF GCV
#if 0
//! Find weight decay which minimizes the leave-one-out cross-validation
//! mean squared error (LOOMSE) of the linear regression with (inputs, targets) pair.
//! Set the resulting weights matrix accordingly and return this weight decay value lambda,
//! i.e. weights = argmin_w ||targets - inputs*w'||^2 + lambda*sum_i ||w_i||^2.
//!   inputs: n_examples x n_inputs (if you want a bias term, include it in the inputs!)
//!   targets: n_examples x n_outputs
//!   weights: n_outputs x n_inputs
//! 
//! This work is achieved by taking advantage of the following formula:
//!    LOOMSE =    (sum of squared errors with the chosen weight decay) / (n_inputs - sum_i e_i/(e_i+lambda))
//! where e_i is an eigenvalue of  matrix inputs' * inputs. The explored values of lambda are based on an SVD
//! of the inputs matrix (whose squared singular values are the eigenvalues of inputs' * inputs.
//! We know that lambda should be between the smallest and the largest eigenvalue. We do a search
//! within the eigenvalue spectrum to select lambda.
//! If best_predictions is provided then a copy of the predictions obtained with the best weight decay is made. Similarly for best_weights.
//! If a positve initial_weight_decay_guess is provided, then instead of trying all the eigenvalues
//! the algorithm searches from this initial guess, never going more than explore_threshold steps
//! from the best weight decay found up to now. The weight decays tried are intermediate values
//! (geometric average) between consecutive eigenvalues
real generalizedCVRidgeRegression(Mat inputs, Mat targets,  real& best_LOOMSE, Mat* best_weights, Mat* best_predictions, bool inputs_are_transposed, real initial_weight_decay_guess, int explore_threshold)
{
    static Mat inputs_copy, U, Vt, predictions, RHS_matrix, weights, XY;
    static Vec singular_values, eigen_values, LOOMSE;
    int n_examples = inputs_are_transposed?inputs.width():inputs.length();
    int n_inputs = inputs_are_transposed?inputs.length():inputs.width();
    int n_outputs = targets.width();
    if (targets.length()!=n_examples)
        PLERROR("generalizedCVRidgeRegression(Mat inputs, Mat targets, Mat weights): targets length (%d) incompatible with inputs length (%d)\n",
                targets.length(),n_examples);
    if (best_weights && (best_weights->length()!=n_outputs || best_weights->width()!=n_inputs))
        PLERROR("generalizedCVRidgeRegression(Mat inputs, Mat targets, Mat weights): weights matrix dimensions was (%d,%d), expected (%d,%d)\n",
                best_weights->length(),best_weights->width(),n_outputs,n_inputs);

    inputs_copy.resize(n_examples,n_inputs);
    predictions.resize(n_examples,n_outputs);
    weights.resize(n_outputs,n_inputs);
    int rank = min(n_examples,n_inputs);
    U.resize(n_examples,rank);
    Vt.resize(rank,n_inputs);
    XY.resize(n_inputs,n_outputs);
    RHS_matrix.resize(rank,n_outputs);
    singular_values.resize(rank);
    eigen_values.resize(rank);
    LOOMSE.resize(rank);
    LOOMSE.fill(-1.);
    if (inputs_are_transposed)
        transpose(inputs, inputs_copy);
    else
        inputs_copy << inputs;

    transposeProduct(XY,inputs_copy,targets);

    // the computational cost of the SVD is O(rank^3)
    SVD(inputs_copy,U,singular_values,Vt,'S');

    product(RHS_matrix,Vt,XY);

    real trace_of_design_matrix = 0;
    for (int i=0;i<rank;i++)
    {
        eigen_values[i] = singular_values[i]*singular_values[i];
        trace_of_design_matrix += eigen_values[i];
    }

    // search among cut-off eigen-values
    best_LOOMSE = 1e38;
    real best_weight_decay = 0;
    if (initial_weight_decay_guess<0) // TRY ALL EIGENVALUES
        for (int i=1;i<rank;i++)
        {
            real weight_decay = exp(0.5*(pl_log(eigen_values[i-1])+pl_log(eigen_values[i])));
            LOOMSE[i] = LOOMSEofRidgeRegression(inputs,targets,weights,weight_decay,eigen_values,Vt, 
                                                predictions, RHS_matrix,inputs_are_transposed);
            if (LOOMSE[i]<best_LOOMSE)
            {
                best_LOOMSE=LOOMSE[i];
                best_weight_decay = weight_decay;
                if (best_predictions)
                    *best_predictions << predictions;
                if (best_weights)
                    *best_weights << weights;
            }
        }
    else // BE MORE GREEDY: DO A SEARCH FROM INITIAL GUESS
    {
        // first find eigenvalue closest to initial guess
        Vec weight_decays(rank);
        weight_decays[0] = eigen_values[0];
        for (int i=1;i<rank;i++)
            weight_decays[i] = exp(0.5*(pl_log(eigen_values[i-1])+pl_log(eigen_values[i])));
        int closest = 0;
        real eval_dist = fabs(weight_decays[0]-initial_weight_decay_guess);
        for (int i=1;i<rank;i++)
        {
            real dist = fabs(weight_decays[i]-initial_weight_decay_guess);
            if (dist < eval_dist)
            {
                eval_dist = dist;
                closest = i;
            }
        }
        // how well are we doing there?
        best_weight_decay = weight_decays[closest];
        int best_i = closest;
        best_LOOMSE = LOOMSE[closest] = LOOMSEofRidgeRegression(inputs,targets,weights,weight_decays[closest],eigen_values,Vt, 
                                                                predictions, RHS_matrix,inputs_are_transposed);
        if (best_predictions)
            *best_predictions << predictions;
        if (best_weights)
            *best_weights << weights;
        // then explore around it, first one way, then the other, until it looks like we can't get better
        int left=closest;
        int right=closest;
        if (right<rank-1)
            right++;
        else
            left--;
        while (left>=0 || right<rank)
        {
            bool improved = false;
            if (LOOMSE[left]<0)
            {
                LOOMSE[left] = LOOMSEofRidgeRegression(inputs,targets,weights,weight_decays[left],eigen_values,Vt, 
                                                       predictions, RHS_matrix,inputs_are_transposed);
                if (LOOMSE[left]<best_LOOMSE)
                {
                    best_LOOMSE=LOOMSE[left];
                    best_weight_decay = weight_decays[left];
                    best_i = left;
                    if (best_predictions)
                        *best_predictions << predictions;
                    if (best_weights)
                        *best_weights << weights;
                    if (left>0)
                    {
                        left--;
                        improved = true;
                    }
                }
            }   
            if (LOOMSE[right]<0)
            {
                LOOMSE[right] = LOOMSEofRidgeRegression(inputs,targets,weights,weight_decays[right],eigen_values,Vt, 
                                                        predictions, RHS_matrix,inputs_are_transposed);
                if (LOOMSE[right]<best_LOOMSE)
                {
                    best_LOOMSE=LOOMSE[right];
                    best_weight_decay = weight_decays[right];
                    best_i = right;
                    if (best_predictions)
                        *best_predictions << predictions;
                    if (best_weights)
                        *best_weights << weights;
                    if (right<rank-1)
                    {
                        right++;
                        improved = true;
                    }
                }
            }
            if (!improved)
            {
                if (best_i - left < right - best_i)
                {
                    if (best_i - left < explore_threshold)
                    {
                        if (left>0)
                            left--;
                        else if (right - best_i < explore_threshold && right<rank-1)
                            right++;
                        else break;
                    }
                    else break;
                }
                else
                {
                    if (right - best_i < explore_threshold)
                    {
                        if (right<rank-1)
                            right++;
                        else if (best_i - left < explore_threshold && left>0)
                            left--;
                        else break;
                    }
                    else break;
                }
            }
        }
    }
    return best_weight_decay;
}
#endif

//! Compute the generalization error estimator called Generalized Cross-Validation (Craven & Wahba 1979),
//! and the corresponding ridge regression weights in
//!    min ||Y - X*W'||^2 + weight_decay ||W||^2.
//! where Y is nxm, X is nxp, W is mxp.
//! The GCV is obtained by performing and SVD of X = U D V' and using the formula from
//! (Bates, Lindstrom, Wahba, Yandell 1986) [tech report at http://www.stat.wisc.edu/~wahba/ftp1/oldie/775r.pdf]
//! (here for m=1):
//!          n ( ||Y||^2 - ||Z||^2 + sum_{j=1}^p z_j^2 (weight_decay / (d_j^2 + weight_decay))^2 )
//!    GCV = ------------------------------------------------------------------------------------
//!                   ( n - p + sum_{j=1}^p (weight_decay  / (d_j^2 + weight_decay)) )^2
//! where Z = U' Y, z_j is the j-th element of Z and d_j is the j-th singular value of X.
//! This formula can be efficiently re-computed for different values of weight decay.
//! For this purpose, pre-compute the SVD can call GCVfromSVD. Once a weight decay
//! has been selected, the SVD can also be used (optionally) to obtain the minimizing weights:
//!    W = V inv(D^2 + weight_decay I) D Z
//!
real GCV(Mat X, Mat Y, real weight_decay, bool X_is_transposed, Mat* W)
{
    int n = Y.length();
    int m = Y.width();
    int p, nx;
    if (X_is_transposed)
    { 
        nx=X.width();
        p=X.length();
    } else {
        nx=X.length();
        p=X.width();
    }
    if (nx!=n)
        PLERROR("GCV: incompatible arguments X and Y don't have same number of examples: %d and %d\n",nx,n);
    if (W && W->length()!=m)
        PLERROR("GCV: incompatible arguments W and Y don't have compatible dimensions: %d and %d\n",W->length(),m);
    if (W && W->width()!=p)
        PLERROR("GCV: incompatible arguments W and X don't have compatible dimensions: %d and %d\n",W->width(),p);
    static Mat Xcopy, U, Vt, Z;
    static Vec singular_values, eigen_values, squaredZ, s;
    Xcopy.resize(n,p);
    if (X_is_transposed)
        transpose(X, Xcopy);
    else
        Xcopy << X;
    int rank = min(n,p);
    U.resize(n,rank);
    Vt.resize(rank,p);
    singular_values.resize(rank);
    eigen_values.resize(rank);
    Z.resize(rank,1);
    squaredZ.resize(rank);
    s.resize(rank);
    Vec z=Z.toVec();
    
    SVD(Xcopy, U, singular_values, Vt, 'S');
    for (int i=0;i<rank;i++)
    {
        eigen_values[i] = singular_values[i]*singular_values[i];
        s[i] = weight_decay / (weight_decay + eigen_values[i]);
    }

    real sum_GCV=0;
    for (int j=0;j<m;j++)
    {
        Mat yj = Y.column(j);
        real y2 = sumsquare(yj);
        transposeProduct(U,yj,Z);
        real z2 = pownorm(z);
        sum_GCV += GCVfromSVD(n, y2-z2, z, s);
        if (W)
        {
            for (int i=0;i<rank;i++)
                z[i] *= s[i]*singular_values[i]/weight_decay;
            transposeProduct((*W)(j),Vt,z);
        }
    }
    return sum_GCV;
}

real GCVfromSVD(real n, real Y2minusZ2, Vec Z, Vec s)
{
    int p = s.length();
    real numerator=Y2minusZ2, denominator=n-p;
    for (int i=0;i<p;i++)
    {
        real si_zi = s[i]*Z[i];
        numerator += si_zi*si_zi;
        denominator += s[i];
    }
    real GCV = n*numerator / (denominator*denominator);
    return GCV;
}

real ridgeRegressionByGCV(Mat X, Mat Y, Mat W, real& best_gcv, bool X_is_transposed, 
                          real initial_weight_decay_guess, int explore_threshold, real min_weight_decay)
{
    int n = Y.length();
    int m = Y.width();
    int p, nx;
    if (X_is_transposed)
    { 
        nx=X.width();
        p=X.length();
    } else {
        nx=X.length();
        p=X.width();
    }
    if (nx!=n)
        PLERROR("ridgeRegressionByGCV: incompatible arguments X and Y don't have same number of examples: %d and %d\n",nx,n);
    if (W.length()!=m)
        PLERROR("ridgeRegressionByGCV: incompatible arguments W and Y don't have compatible dimensions: %d and %d\n",W.length(),m);
    if (W.width()!=p)
        PLERROR("ridgeRegressionByGCV: incompatible arguments W and X don't have compatible dimensions: %d and %d\n",W.width(),p);
    Mat Xcopy, U, Vt, Z, squaredZ;
    Vec singular_values, eigen_values, s, y2, z2, best_s;
    Xcopy.resize(n,p);
    if (X_is_transposed)
        transpose(X, Xcopy);
    else
        Xcopy << X;
    int rank = min(n,p);
    U.resize(n,rank);
    Vt.resize(rank,p);
    singular_values.resize(rank);
    eigen_values.resize(rank);
    Z.resize(m,rank);
    squaredZ.resize(m,rank);
    s.resize(rank);
    best_s.resize(rank);
    y2.resize(m);
    z2.resize(m);
    PLASSERT( !Xcopy.hasMissing() );
    SVD(Xcopy, U, singular_values, Vt, 'S', 2);
    for (int i=0;i<rank;i++)
        eigen_values[i] = singular_values[i]*singular_values[i];

    for (int j=0;j<m;j++)
    {
        Mat Yj = Y.column(j);
        Vec Zj = Z(j);
        y2[j] = sumsquare(Yj);
        transposeProduct(Zj.toMat(rank,1),U,Yj);
        z2[j] = pownorm(Zj);
    }

    Vec gcv;
    gcv.resize(rank);
    gcv.fill(-1.);
    best_gcv = 1e38;
    real best_weight_decay = min_weight_decay;
    if (initial_weight_decay_guess<0) // TRY ALL EIGENVALUES
        // for (int i=1;i<=rank;i++)
        for (int i=1;i<rank;i++)
        {
            bool stop=false;
            real weight_decay = 0;
            /*
            if(i==rank)
                weight_decay = min_weight_decay;
            else
            */
            weight_decay = exp(0.5*(pl_log(eigen_values[i-1])+pl_log(eigen_values[i])));
            // perr << "Trying weight_decay = " << weight_decay;
            if (weight_decay < min_weight_decay)
            {
                weight_decay = min_weight_decay;
                stop = true;
            }
            for (int j=0;j<rank;j++)
                s[j] = weight_decay / (weight_decay + eigen_values[j]);
            real gcv_i = 0;
            for (int j=0;j<m;j++)
                gcv_i += GCVfromSVD(n,y2[j]-z2[j], Z(j), s);
            // perr << " -> gcv =  " << gcv_i << endl;
            if (gcv_i<best_gcv)
            {
                best_gcv=gcv_i;
                best_weight_decay = weight_decay;
                best_s << s;
            }
            if (stop)
                break;
        }
    else // BE MORE GREEDY: DO A SEARCH FROM INITIAL GUESS
    {
        // first find eigenvalue closest to initial guess
        Vec weight_decays(rank+1);
        weight_decays[0] = max(min_weight_decay,eigen_values[0]);
        int stop = rank;
        for (int i=1;i<rank;i++)
        {
            if (i<stop)
            {
                weight_decays[i] = exp(0.5*(pl_log(eigen_values[i-1])+pl_log(eigen_values[i])));
                if (weight_decays[i] < min_weight_decay)
                {
                    stop = i;
                    weight_decays[i] = min_weight_decay;
                }
            }
            else weight_decays[i] = min_weight_decay;
        }
        int closest = 0;
        real eval_dist = fabs(weight_decays[0]-initial_weight_decay_guess);
        for (int i=1;i<stop;i++)
        {
            real dist = fabs(weight_decays[i]-initial_weight_decay_guess);
            if (dist < eval_dist)
            {
                eval_dist = dist;
                closest = i;
            }
        }
        // how well are we doing there?
        best_weight_decay = weight_decays[closest];

        int best_i = closest;
        for (int i=0;i<rank;i++)
            s[i] =  best_weight_decay / (best_weight_decay + eigen_values[i]);
        gcv[closest] = 0;
        for (int j=0;j<m;j++)
            gcv[closest] += GCVfromSVD(n,y2[j]-z2[j], Z(j), s);
        best_gcv = gcv[closest];
        best_s << s;

        // then explore around it, first one way, then the other, until it looks like we can't get better
        int left=closest;
        int right=closest;
        if (right<stop-1)
            right++;
        else
            left--;
        while (left>=0 || right<stop-1)
        {
            bool improved = false;
            if (gcv[left]<0)
            {
                for (int i=0;i<rank;i++)
                    s[i] = weight_decays[left] / (weight_decays[left] + eigen_values[i]);
                gcv[left] = 0;
                for (int j=0;j<m;j++)
                    gcv[left] += GCVfromSVD(n,y2[j]-z2[j], Z(j), s);
                if (gcv[left]<best_gcv)
                {
                    best_gcv=gcv[left];
                    best_weight_decay = weight_decays[left];
                    best_i = left;
                    best_s << s;

                    if (left>0)
                    {
                        left--;
                        improved = true;
                    }
                }
            }   
            if (gcv[right]<0)
            {
                for (int i=0;i<rank;i++)
                    s[i] = weight_decays[right] / (weight_decays[right] + eigen_values[i]);
                gcv[right] = 0;
                for (int j=0;j<m;j++)
                    gcv[right] += GCVfromSVD(n,y2[j]-z2[j], Z(j), s);
                if (gcv[right]<best_gcv)
                {
                    best_gcv=gcv[right];
                    best_weight_decay = weight_decays[right];
                    best_i = right;
                    best_s << s;

                    if (right<stop-1)
                    {
                        right++;
                        improved = true;
                    }
                }
            }
            if (!improved)
            {
                if (best_i - left < right - best_i)
                {
                    if (best_i - left < explore_threshold)
                    {
                        if (left>0)
                            left--;
                        else if (right - best_i < explore_threshold && right<stop-1)
                            right++;
                        else break;
                    }
                    else break;
                }
                else
                {
                    if (right - best_i < explore_threshold)
                    {
                        if (right<stop-1)
                            right++;
                        else if (best_i - left < explore_threshold && left>0)
                            left--;
                        else break;
                    }
                    else break;
                }
            }
        }
    }

    // compute weights for selected weight decay
    for (int j=0;j<m;j++)
    {
        Vec zj = Z(j);
        for (int i=0;i<rank;i++)
            zj[i] *= best_s[i]*singular_values[i]/best_weight_decay;
        transposeProduct(W(j),Vt,zj);
    }
    return best_weight_decay;
}



real weightedRidgeRegressionByGCV(Mat X, Mat Y, Vec gamma, Mat W, real& best_gcv, real min_weight_decay)
{
    int l = X.length();

    real gamma_sum = 0;
    if(gamma.length()==0)
        gamma_sum = l;    
    else
    {
        gamma_sum = sum(gamma);
        for(int i=0; i<l; i++)
        {
            real s = sqrt(gamma[i]);
            X(i) *= s;
            Y(i) *= s;
        }
    }
    
    int n = Y.length();
    int m = Y.width();
    int p = X.width();
    int nx = X.length();

    if (nx!=n)
        PLERROR("ridgeRegressionByGCV: incompatible arguments X and Y don't have same number of examples: %d and %d\n",nx,n);
    if (W.length()!=m)
        PLERROR("ridgeRegressionByGCV: incompatible arguments W and Y don't have compatible dimensions: %d and %d\n",W.length(),m);
    if (W.width()!=p)
        PLERROR("ridgeRegressionByGCV: incompatible arguments W and X don't have compatible dimensions: %d and %d\n",W.width(),p);
    Mat U, Vt, Z, squaredZ;
    Vec singular_values, eigen_values, s, y2, z2, best_s;
    int rank = min(n,p);
    U.resize(n,rank);
    Vt.resize(rank,p);
    singular_values.resize(rank);
    eigen_values.resize(rank);
    Z.resize(m,rank);
    squaredZ.resize(m,rank);
    s.resize(rank);
    best_s.resize(rank);
    y2.resize(m);
    z2.resize(m);
    SVD(X, U, singular_values, Vt, 'S', 2);
    // perr << "Singular values: " << singular_values << endl;
    for (int i=0;i<rank;i++)
        eigen_values[i] = singular_values[i]*singular_values[i];
    // perr << "Eigen values: " << eigen_values << endl;

    for (int j=0;j<m;j++)
    {
        Mat Yj = Y.column(j);
        Vec Zj = Z(j);
        y2[j] = sumsquare(Yj);
        transposeProduct(Zj.toMat(rank,1),U,Yj);
        z2[j] = pownorm(Zj);
    }

    Vec gcv;
    gcv.resize(rank);
    gcv.fill(-1.);
    best_gcv = 1e38;
    real best_weight_decay = min_weight_decay;

    for (int i=1;i<=rank;i++)
    {
        bool stop=false;
        real weight_decay = 0;
        if(i==rank)
            weight_decay = min_weight_decay;
        else
            weight_decay = exp(0.5*(pl_log(eigen_values[i-1])+pl_log(eigen_values[i])));
        // perr << "Trying weight_decay = " << weight_decay;
        if (weight_decay < min_weight_decay)
        {
            weight_decay = min_weight_decay;
            stop = true;
        }
        for (int j=0;j<rank;j++)
            s[j] = weight_decay / (weight_decay + eigen_values[j]);
        real gcv_i = 0;
        for (int j=0;j<m;j++)
            gcv_i += GCVfromSVD(gamma_sum,y2[j]-z2[j], Z(j), s);
        // perr << " -> gcv =  " << gcv_i << endl;
        if (gcv_i<best_gcv)
        {
            best_gcv=gcv_i;
            best_weight_decay = weight_decay;
            best_s << s;
        }
        if (stop)
            break;
    }

    // compute weights for selected weight decay
    for (int j=0;j<m;j++)
    {
        Vec zj = Z(j);
        for (int i=0;i<rank;i++)
            zj[i] *= best_s[i]*singular_values[i]/best_weight_decay;
        transposeProduct(W(j),Vt,zj);
    }
    return best_weight_decay;
}

#if 0
real LOOMSEofRidgeRegression(Mat inputs, Mat targets, Mat weights, real weight_decay, Vec eigenvalues, Mat eigenvectors, Mat predictions, 
                             Mat RHS_matrix, bool inputs_are_transposed)
{
    int n_inputs = weights.width();
    int n_outputs = targets.width();
    int n_examples = targets.length();
    int rank = eigenvalues.length();
    // weights' = eigenvectors' * inv(diag(eigenvalues) + weight_decay*I) * eigenvectors' * inputs' * targets 
    //          = eigenvectors' * inv(diag(eigenvalues) + weight_decay*I) * RHS_matrix
    weights.clear();
    real s=0;
    for (int k=0;k<rank;k++)
    {
        real* vk = eigenvectors[k];
        real* RHSk = RHS_matrix[k];
        real coeff = 1.0/(eigenvalues[k] + weight_decay);
        s += eigenvalues[k]*coeff;
        for (int i=0;i<n_outputs;i++)
        {
            real *wi = weights[i];
            for (int j=0;j<n_inputs;j++)
                wi[j] += vk[j] * coeff * RHSk[i];
        }
    }
    
    if (inputs_are_transposed)
        transposeTransposeProduct(predictions, inputs, weights);
    else
        productTranspose(predictions, inputs, weights);
    real SSE = 0;
    for (int i=0;i<targets.length();i++)
    {
        real *ti = targets[i];
        real *pi = predictions[i];
        for (int j=0;j<targets.width();j++)
        {
            real diff = ti[j]-pi[j];
            SSE += diff*diff;
        }
    }
    real denom = n_examples - s;
    if (denom<0)
        PLERROR("LOOMSEofRidgeRegression: mathematical error: should not get negative trace!");
    if (denom==0) return 1e34; // some really large error...
    real LOOMSE = SSE / denom;
    return LOOMSE;
}
#endif

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
