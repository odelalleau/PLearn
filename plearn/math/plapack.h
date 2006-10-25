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

/*! \file PLearnLibrary/PLearnCore/plapack.h */

#ifndef plapack_h
#define plapack_h

#include "Mat.h"
#include "TMat_maths.h"

#include "lapack_proto.h"

namespace PLearn {
using namespace std;

// Direct lapack calls, type independent
inline void lapack_Xsyevx_(char* JOBZ, char* RANGE, char* UPLO, int* N, double* A, int* LDA, double* VL, double* VU, int* IL, int* IU, double* ABSTOL, int* M, double* W, double* Z, int* LDZ, double* WORK, int* LWORK, int* IWORK, int* IFAIL, int* INFO)
{ dsyevx_(JOBZ, RANGE, UPLO, N, A, LDA, VL, VU, IL, IU, ABSTOL, M, W, Z, LDZ, WORK, LWORK, IWORK, IFAIL, INFO); }

inline void lapack_Xsyevx_(char* JOBZ, char* RANGE, char* UPLO, int* N, float* A, int* LDA, float* VL, float* VU, int* IL, int* IU, float* ABSTOL, int* M, float* W, float* Z, int* LDZ, float* WORK, int* LWORK, int* IWORK, int* IFAIL, int* INFO)
{ ssyevx_(JOBZ, RANGE, UPLO, N, A, LDA, VL, VU, IL, IU, ABSTOL, M, W, Z, LDZ, WORK, LWORK, IWORK, IFAIL, INFO); }

inline void lapack_Xgesdd_(char* JOBZ, int* M, int* N, double* A, int* LDA, double* S, double* U, int* LDU, double* VT, int* LDVT, double* WORK, int* LWORK, int* IWORK, int* INFO)
{ dgesdd_(JOBZ, M, N, A, LDA, S, U, LDU, VT, LDVT, WORK, LWORK, IWORK, INFO); }

inline void lapack_Xgesdd_(char* JOBZ, int* M, int* N, float* A, int* LDA, float* S, float* U, int* LDU, float* VT, int* LDVT, float* WORK, int* LWORK, int* IWORK, int* INFO)
{ sgesdd_(JOBZ, M, N, A, LDA, S, U, LDU, VT, LDVT, WORK, LWORK, IWORK, INFO); }

inline void lapack_Xsyevr_(char* JOBZ, char* RANGE, char* UPLO, int* N, float* A, int* LDA, float* VL, float* VU, int* IL, int* IU, float* ABSTOL, int* M, float* W, float* Z, int* LDZ, int* ISUPPZ, float* WORK, int* LWORK, int* IWORK, int* LIWORK, int* INFO)
{ ssyevr_(JOBZ, RANGE, UPLO, N, A, LDA, VL, VU, IL, IU, ABSTOL, M, W, Z, LDZ, ISUPPZ, WORK, LWORK, IWORK, LIWORK, INFO);}

inline void lapack_Xsyevr_(char* JOBZ, char* RANGE, char* UPLO, int* N, double* A, int* LDA, double* VL, double* VU, int* IL, int* IU, double* ABSTOL, int* M, double* W, double* Z, int* LDZ, int* ISUPPZ, double* WORK, int* LWORK, int* IWORK, int* LIWORK, int* INFO)
{ dsyevr_(JOBZ, RANGE, UPLO, N, A, LDA, VL, VU, IL, IU, ABSTOL, M, W, Z, LDZ, ISUPPZ, WORK, LWORK, IWORK, LIWORK, INFO);}

inline void lapack_Xsygvx_(int* ITYPE, char* JOBZ, char* RANGE, char* UPLO, int* N, double* A, int* LDA, double* B, int* LDB, double* VL, double* VU, int* IL, int* IU, double* ABSTOL, int* M, double* W, double* Z, int* LDZ, double* WORK, int* LWORK, int* IWORK, int* IFAIL, int* INFO)
{ dsygvx_(ITYPE, JOBZ, RANGE, UPLO, N, A, LDA, B, LDB, VL, VU, IL, IU, ABSTOL, M, W, Z, LDZ, WORK, LWORK, IWORK, IFAIL, INFO); }

inline void lapack_Xsygvx_(int* ITYPE, char* JOBZ, char* RANGE, char* UPLO, int* N, float* A, int* LDA, float* B, int* LDB, float* VL, float* VU, int* IL, int* IU, float* ABSTOL, int* M, float* W, float* Z, int* LDZ, float* WORK, int* LWORK, int* IWORK, int* IFAIL, int* INFO)
{ ssygvx_(ITYPE, JOBZ, RANGE, UPLO, N, A, LDA, B, LDB, VL, VU, IL, IU, ABSTOL, M, W, Z, LDZ, WORK, LWORK, IWORK, IFAIL, INFO); }


//!  Computes the eigenvalues and eigenvectors of a symmetric (NxN) matrix A.
//!  BEWARE: The content of A is destroyed by the call.
//!  NOTE: you may wish to use the simpler call eigenVecOfSymmMat
/*! 
  Meaning of RANGE: 
  'A': all eigenvalues will be found.
  'V': all eigenvalues in the half-open interval (low,high] will be found. 
  'I': will find eigenvals with indexes int(low) to int(high) included (smallest eigenval having index 0)

  ABSTOL is the tolerance (see lapack doc for call dsyevx_ )

  If you do not wish to compute eigenvectors, provide a null (empty) 'eigenvecs'.

  Upon return, eigenvals will contain the M eigenvalues found 
  in increasing order (it will be resized to M).
  And eigenvecs (unless initially null) will be resized to an MxN matrix 
  containing the corresponding M eigenvectors in its *rows*.
*/

// (will work for float and double)
template<class num_t>
void lapackEIGEN(const TMat<num_t>& A, TVec<num_t>& eigenvals, TMat<num_t>& eigenvecs, char RANGE='A', num_t low=0, num_t high=0, num_t ABSTOL=0)
{

#ifdef BOUNDCHECK
    if(A.length()!=A.width())
        PLERROR("In lapackEIGEN length and width of A differ, it should be symmetric!");
#endif

    char JOBZ = eigenvecs.isEmpty() ?'N' :'V';
    char UPLO = 'U';  
    int N = A.length();
    int LDA = A.mod();

    int IL=0, IU=0;
    num_t VL=0, VU=0;

    eigenvals.resize(N);
    int M; // The number of eigenvalues returned

    switch(RANGE)
    {
    case 'A':
        if(JOBZ=='V')
            eigenvecs.resize(N, N);
        break;
    case 'I': 
        IL = int(low)+1; // +1 because fortran indexes start at 1
        IU = int(high)+1;
        if(JOBZ=='V')
            eigenvecs.resize(IU-IL+1, N);
        break;
    case 'V':
        VL = low;
        VU = high;
        if(JOBZ=='V')
            eigenvecs.resize(N, N);
        break;
    default:
        PLERROR("In lapackEIGEN: invalid RANGE character: %c",RANGE);
    }
  
    num_t* Z = 0;
    int LDZ = 1;
    if(eigenvecs.isNotEmpty())
    {
        Z = eigenvecs.data();
        LDZ = eigenvecs.mod();
    }

    // temporary work vectors
    static TVec<num_t> WORK;
    static TVec<int> IWORK;
    static TVec<int> IFAIL;

    WORK.resize(1);
    IWORK.resize(5*N);
    IFAIL.resize(N);

    int LWORK = -1;
    int INFO;


    // first call to find optimal work size
    //  cerr << '(';
    lapack_Xsyevx_( &JOBZ, &RANGE, &UPLO, &N, A.data(), &LDA,  &VL,  &VU,
                    &IL,  &IU,  &ABSTOL,  &M,  eigenvals.data(), Z, &LDZ, 
                    WORK.data(), &LWORK, IWORK.data(), IFAIL.data(), &INFO );
    // cerr << ')';

    if(INFO!=0)
        PLERROR("In lapackEIGEN, problem in first call to sgesdd_ to get optimal work size, returned INFO = %d",INFO); 
  
    // make sure we have enough space
    LWORK = (int) WORK[0]; // optimal size
    WORK.resize(LWORK);

    // second call to do the computation
    // cerr << '{';
    lapack_Xsyevx_( &JOBZ, &RANGE, &UPLO, &N, A.data(), &LDA,  &VL,  &VU,
                    &IL,  &IU,  &ABSTOL,  &M,  eigenvals.data(), Z, &LDZ, 
                    WORK.data(), &LWORK, IWORK.data(), IFAIL.data(), &INFO );
    // cerr << '}';

    if(INFO!=0)
        PLERROR("In lapackEIGEN, problem when calling sgesdd_ to perform computation, returned INFO = %d",INFO); 

    eigenvals.resize(M);
    if(JOBZ=='V')
        eigenvecs.resize(M, N);
}

//!  Computes the eigenvalues and eigenvectors of a real generalized 
//!  symmetric-definite eigenproblem, of the form
//!  A*x=(lambda)*B*x,  A*Bx=(lambda)*x, or B*A*x=(lambda)*x
//!  A and B are assumed to be symmetric and B is also positive definite.
//!  BEWARE: The content of A and B is destroyed by the call.
//!  NOTE: you may wish to use the simpler call generalizedEigenVecOfSymmMat
/*! 
  Meaning of ITYPE
  Specifies the problem type to be solved:
  = 1:  A*x = (lambda)*B*x
  = 2:  A*B*x = (lambda)*x
  = 3:  B*A*x = (lambda)*x

  Meaning of RANGE: 
  'A': all eigenvalues will be found.
  'V': all eigenvalues in the half-open interval (low,high] will be found. 
  'I': will find eigenvals with indexes int(low) to int(high) included (smallest eigenval having index 0)

  ABSTOL is the tolerance (see lapack doc for call dsygvx_ )

  If you do not wish to compute eigenvectors, provide a null (empty) 'eigenvecs'.

  Upon return, eigenvals will contain the M eigenvalues found 
  in increasing order (it will be resized to M).
  And eigenvecs (unless initially null) will be resized to an MxN matrix 
  containing the corresponding M eigenvectors in its *rows*.
*/

// (will work for float and double)
template<class num_t>
void lapackGeneralizedEIGEN(const TMat<num_t>& A, const TMat<num_t>& B, int ITYPE, TVec<num_t>& eigenvals, TMat<num_t>& eigenvecs, char RANGE='A', num_t low=0, num_t high=0, num_t ABSTOL=0)
{

#ifdef BOUNDCHECK
    if(A.length()!=A.width())
        PLERROR("In lapackGeneralizedEIGEN length and width of A differ, it should be symmetric!");
#endif

    char JOBZ = eigenvecs.isEmpty() ?'N' :'V';
    char UPLO = 'U';  
    int N = A.length();//The order of the matrix pencil (A,B)
    int LDA = A.mod();
    int LDB = B.mod();

    int IL=0, IU=0;
    num_t VL=0, VU=0;

    eigenvals.resize(N);
    int M; // The number of eigenvalues returned

    switch(RANGE)
    {
    case 'A':
        if(JOBZ=='V')
            eigenvecs.resize(N, N);
        break;
    case 'I': 
        IL = int(low)+1; // +1 because fortran indexes start at 1
        IU = int(high)+1;
        if(JOBZ=='V')
            eigenvecs.resize(IU-IL+1, N);
        break;
    case 'V':
        VL = low;
        VU = high;
        if(JOBZ=='V')
            eigenvecs.resize(N, N);
        break;
    default:
        PLERROR("In lapackGeneralizedEIGEN: invalid RANGE character: %c",RANGE);
    }
  
    num_t* Z = 0;
    int LDZ = 1;
    if(eigenvecs.isNotEmpty())
    {
        Z = eigenvecs.data();
        LDZ = eigenvecs.mod();
    }

    // temporary work vectors
    static TVec<num_t> WORK;
    static TVec<int> IWORK;
    static TVec<int> IFAIL;

    WORK.resize(1);
    IWORK.resize(5*N);
    IFAIL.resize(N);

    int LWORK = -1;
    int INFO;


    // first call to find optimal work size
    //  cerr << '(';
    lapack_Xsygvx_( &ITYPE, &JOBZ, &RANGE, &UPLO, &N, A.data(), &LDA, B.data(), &LDB,  &VL,  &VU,
                    &IL,  &IU,  &ABSTOL,  &M,  eigenvals.data(), Z, &LDZ, 
                    WORK.data(), &LWORK, IWORK.data(), IFAIL.data(), &INFO );
    // cerr << ')';

    if(INFO!=0)
        PLERROR("In lapackGeneralizedEIGEN, problem in first call to sgesdd_ to get optimal work size, returned INFO = %d",INFO); 
  
    // make sure we have enough space
    LWORK = (int) WORK[0]; // optimal size
    WORK.resize(LWORK);

    // second call to do the computation
    // cerr << '{';
    lapack_Xsygvx_( &ITYPE, &JOBZ, &RANGE, &UPLO, &N, A.data(), &LDA, B.data(), &LDB,  &VL,  &VU,
                    &IL,  &IU,  &ABSTOL,  &M,  eigenvals.data(), Z, &LDZ, 
                    WORK.data(), &LWORK, IWORK.data(), IFAIL.data(), &INFO );
    // cerr << '}';

    if(INFO!=0)
        PLERROR("In lapackGeneralizedEIGEN, problem when calling sgesdd_ to perform computation, returned INFO = %d",INFO); 

    eigenvals.resize(M);
    if(JOBZ=='V')
        eigenvecs.resize(M, N);
}

//! Computes up to k largest eigen_values and corresponding eigen_vectors of symmetric matrix m. 
//! Parameters eigen_values and eigen_vectors are resized accordingly and filled by the call.
//! The eigenvalues are returned in decreasing order (largest first).
//! The corresponding eigenvectors are in the *ROWS* of eigen_vectors
//! WARNING: m is destroyed during the operation.

// (will work for float and double)
template<class num_t>
void eigenVecOfSymmMat(TMat<num_t>& m, int k, TVec<num_t>& eigen_values, TMat<num_t>& eigen_vectors, bool verbose=true)
{
    if (m.isEmpty()) {
        // Empty matrix: we just need to do some resizing.
        eigen_values.resize(0);
        eigen_vectors.resize(m.length(), m.width());
        return;
    }
    if (!m.isSymmetric()) {
        if (m.isSymmetric(false))
        {
            // Almost symmetric.
            if (verbose)
                PLWARNING("In eigenVecOfSymmMat - The matrix is only 'almost' symmetric, "
                          "it will be forced to be exactly symmetric");
        }
        else
            PLWARNING("In eigenVecOfSymmMat - The matrix is not symmetric, it will "
                      "be forced to be exactly symmetric by copying the top "
                      "right part to the bottom left part");
        fillItSymmetric(m);
    }
    eigen_vectors.resize(k,m.width());
    eigen_values.resize(k);
    // FASTER
    if(k>= m.width())
        lapackEIGEN(m, eigen_values, eigen_vectors, 'A',num_t(0),num_t(0));
    else
        lapackEIGEN(m, eigen_values, eigen_vectors, 'I', num_t(m.width()-k), num_t(m.width()-1));

    // put largest (rather than smallest) first!
    eigen_values.swap();
    eigen_vectors.swapUpsideDown();
}

//! Computes up to k largest eigen_values and corresponding eigen_vectors of a real 
//! generalized symmetric-definite eigenproblem, of the form 
//! m1*x=(lambda)*m2*x (itype = 1),
//! m1*m2*x=(lambda)*x (itype = 2) or 
//! m2*m1*x=(lambda)*x (itype = 3) 
//! m1 and m2 are assumed to be symmetric and m2 is also positive definite. 
//! Parameters eigen_values and eigen_vectors are resized accordingly and filled by the call.
//! The eigenvalues are returned in decreasing order (largest first).
//! The corresponding eigenvectors are in the *ROWS* of eigen_vectors
//! WARNING: m1 and m2 are destroyed during the operation.

// (will work for float and double)
template<class num_t>
void generalizedEigenVecOfSymmMat(TMat<num_t>& m1, TMat<num_t>& m2, int itype, int k, TVec<num_t>& eigen_values, TMat<num_t>& eigen_vectors)
{
    if(m1.length() != m2.length() || m1.width() != m2.width())
        PLERROR("In generalizedEigenVecOfSymmMat, m1 and m2 must have the same size"); 

    eigen_vectors.resize(k,m1.width());
    eigen_values.resize(k);
    // FASTER
    if(k>= m1.width())
        lapackGeneralizedEIGEN(m1, m2, itype, eigen_values, eigen_vectors, 'A',num_t(0),num_t(0));
    else
        lapackGeneralizedEIGEN(m1, m2, itype, eigen_values, eigen_vectors, 'I', num_t(m1.width()-k), num_t(m1.width()-1));

    // put largest (rather than smallest) first!
    eigen_values.swap();
    eigen_vectors.swapUpsideDown();
}



/*! Performs the SVD decomposition A = U.S.Vt
  See SVD(...) for more details.

  CAREFUL: the 'At' matrix argument is changed in the process!

  This is a straight forward call to the lapack function.
  As fortran uses column-major matrices, and we use row-major matrices,
  it's really as if we had to pass the transpose of A (denoted At) and 
  were getting back the transpose of U (Ut) and V.

  If you want a version without the funny transposes, look at SVD (which
  simply calls this one with a different order of parameters...)  
*/

// (will work for float and double)
template<class num_t>
void lapackSVD(const TMat<num_t>& At, TMat<num_t>& Ut, TVec<num_t>& S, TMat<num_t>& V, char JOBZ='A', real safeguard = 1)
{            
    int M = At.width();
    int N = At.length();
    int LDA = At.mod();
    int min_M_N = min(M,N);
    S.resize(min_M_N);

    switch(JOBZ)
    {
    case 'A':
        Ut.resize(M,M);
        V.resize(N,N);
        break;
    case 'S':
        Ut.resize(min_M_N, M);
        V.resize(N, min_M_N);
        break;
    case 'O':
        if(M<N)
            Ut.resize(M,M); // and V is not used      
        else
            V.resize(N,N); // and Ut is not used
        break;
    case 'N':
        break;
    default:
        PLERROR("In lapackSVD, bad JOBZ argument : %c",JOBZ);
    }


    int LDU = 1;
    int LDVT = 1;
    num_t* U = 0;
    num_t* VT = 0;

    if(V.isNotEmpty())
    {
        LDVT = V.mod();
        VT = V.data();
    }
    if(Ut.isNotEmpty())
    {
        LDU = Ut.mod();
        U = Ut.data();
    }

    static TVec<num_t> WORK;
    WORK.resize(1);
    int LWORK = -1;

    static TVec<int> IWORK;
    IWORK.resize(8*min_M_N);

    int INFO;

    // first call to find optimal work size
    lapack_Xgesdd_(&JOBZ, &M, &N, At.data(), &LDA, S.data(), U, &LDU, VT, &LDVT, WORK.data(), &LWORK, IWORK.data(), &INFO);

    if(INFO!=0)
        PLERROR("In lapackSVD, problem in first call to sgesdd_ to get optimal work size, returned INFO = %d",INFO); 
  
    // make sure we have enough space
    LWORK = int(WORK[0] * safeguard + 0.5); // optimal size (safeguard may be used to make sure it doesn't crash in some rare occasions).
    WORK.resize(LWORK);
    // cerr << "Optimal WORK size: " << LWORK << endl;

    // second call to do the computation
    lapack_Xgesdd_(&JOBZ, &M, &N, At.data(), &LDA, S.data(), U, &LDU, VT, &LDVT, WORK.data(), &LWORK, IWORK.data(), &INFO );

    if(INFO!=0)
    {      
        // cerr << At << endl;
        // cerr << "In lapackSVD, failed with INFO = " << INFO << endl;
        PLERROR("In lapackSVD, problem when calling sgesdd_ to perform computation, returned INFO = %d",INFO); 
    }
}

//! Performs the SVD decomposition A = U.S.Vt
//! Where U and Vt are orthonormal matrices.
/*! 

A is an MxN matrix whose content is destroyed by the call.

S in the above formula is also an MxN matrix, with only its 
first min(M,N) diagonal elements are non-zero. The call fills a 
vector S with those elements: the singular values, in decreasing order.

JOBZ has the following meaning:

'A': all M columns of U and all N rows of Vt are returned in the
arrays U and VT;
  
'S': the first min(M,N) columns of U and the first min(M,N) rows of
Vt are returned in the arrays U and Vt;

'O': If M >= N, the first N columns of U are overwritten on the
array A and all rows of Vt are returned in the array VT; otherwise,
all columns of U are returned in the array U and the first M rows of
Vt are overwritten in the array VT; = 'N': no columns of U or rows
of Vt are computed.

'N': compute only the singular values (U and V are not computed)

The optional value 'safeguard' may be used with a value > 1 if there is
a crash in the SVD (typically, saying that parameter 12 has an illegal
value).

Relationships between SVD(A) and eigendecomposition of At.A and A.At
-> square(singular values) = eigenvalues
-> columns of V (rows of Vt) are the eigenvectors of At.A
-> columns of U are the eigenvectors of A.At
*/

// (will work for float and double)
template<class num_t>
inline void SVD(const TMat<num_t>& A, TMat<num_t>& U, TVec<num_t>& S, TMat<num_t>& Vt, char JOBZ='A', real safeguard = 1)
{
    // A = U.S.Vt -> At = V.S.Ut
    PLASSERT( !A.hasMissing() );
    lapackSVD(A,Vt,S,U,JOBZ, safeguard);
}


//  DO  NOT USE! 
// eigen_SymmMat is deprecated: use eigenVecOfSymmMat or lapackEIGEN instead");
/*!   This function compute some or all eigenvalues (and optionnaly the
  corresponding eigenvectors) of a symmetric matrix. The eigenvectors
  are returned in the ROWS of e_vector.
  
  Note1: If compute_all==true, then the field nb_eigen will not be used.
  
  Note2: Your input matrix `in' will be over-written
  
  Note3: If compute_all=false only some eigen-values (and optionally e-vectors)
  are computed. This flag allows to select whether those largest in magnitude
  (the default) or smallest in magnitude are selected.
  
  Note4: This function is slightly modified. Now, you do not have to check 
  if your input matrix is symmetric or not. 
  Note5: The vectors and eigenvalues seem to be sorted in increasing order (
*/
int eigen_SymmMat(Mat& in, Vec& e_value, Mat& e_vector, int& n_evalues_found, 
                  bool compute_all, int nb_eigen, bool compute_vectors = true,
                  bool largest_evalues=true);

//  DO  NOT USE! 
// eigen_SymmMat_decreasing is deprecated: use eigenVecOfSymmMat or lapackEIGEN instead");
//!  same as the previous call, but eigenvalues/vectors are sorted by largest firat (in decreasing order)
int eigen_SymmMat_decreasing(Mat& in, Vec& e_value, Mat& e_vector, int& n_evalues_found, 
                             bool compute_all, int nb_eigen, bool compute_vectors = true,
                             bool largest_evalues=true);

//! This function compute the inverse of a matrix.
//! WARNING: the input matrix 'in' is overwritten in the process.
int matInvert(Mat& in, Mat& inverse);

//!  generate N vectors sampled from the normal with mean vector mu
//!  and covariance matrix A 
Mat multivariate_normal(const Vec& mu, const Mat& A, int N);

//!  generate a vector sampled from the normal with mean vector mu 
//!  and covariance matrix A
Vec multivariate_normal(const Vec& mu, const Mat& A);

//!  generate 1 vector sampled from the normal with mean mu 
//!  and covariance matrix A = evectors * diagonal(e_values) * evectors' 
Vec multivariate_normal(const Vec& mu, const Vec& e_values, const Mat& e_vectors);

//!  generate a vector x sampled from the normal with mean mu 
//!  and covariance matrix A = evectors * diagonal(e_values) * evectors' 
//! (the normal(0,I) originally sampled to obtain x is stored in z).
//! Unlike the other variants of this function, this one does not allocate anything.
void multivariate_normal(Vec& x, const Vec& mu, const Vec& e_values, const Mat& e_vectors, Vec& z);

/*!   Solves AX = B
  This is a simple wrapper over the lapack routine. It expects At and Bt (transposes of A and B) as input, 
  as well as storage for resulting pivots vector of ints of same length as A.
  The call overwrites Bt, putting the transposed solution Xt in there,
  and At is also overwritten to contain the factors L and U from the factorization A = P*L*U; 
  (the unit diagonal elements of L  are  not stored).
  The lapack status is returned:
  = 0:  successful exit
  < 0:  if INFO = -i, the i-th argument had an illegal value
  > 0:  if INFO = i, U(i,i) is  exactly  zero.   The factorization has been completed, 
  but the factor U is exactly singular, so the solution could not be computed.
*/
int lapackSolveLinearSystem(Mat& At, Mat& Bt, TVec<int>& pivots);

/*!   Returns the solution X of AX = B
  A and B are left intact, and the solution is returned.
  This call does memory allocations/deallocations and transposed copies of matrices (contrary to the lower 
  level lapackSolveLinearSystem call that you may consider using if efficiency is a concern).
*/
Mat solveLinearSystem(const Mat& A, const Mat& B);

//!  Returns solution x of Ax = b
//!  (same as above, except b and x are vectors)
Vec solveLinearSystem(const Mat& A, const Vec& b);

//!  for matrices A such that A.length() <= A.width(),
//!  find X s.t. A X = Y
void solveLinearSystem(const Mat& A, const Mat& Y, Mat& X);

//!  for matrices A such that A.length() >= A.width(),
//!  find X s.t. X A = Y
void solveTransposeLinearSystem(const Mat& A, const Mat& Y, Mat& X);

/*!   Returns w that minimizes ||X.w - Y||^2 + lambda.||w||^2
  under constraint \sum w_i = 1
  Xt is the transposed of the input matrix X; Y is the target vector.
  This doesn't include any bias term.
*/
Vec constrainedLinearRegression(const Mat& Xt, const Vec& Y, real lambda=0.);


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
real GCV(Mat X, Mat Y, real weight_decay, bool X_is_transposed=false, Mat* W=0);

//! Estimator of generalization error estimator called Generalized Cross-Validation (Craven & Wahba 1979),
//! computed from the SVD of the input matrix X in the ridge regression. See the comments
//! for GCV. This function implements the formula:
//!          n ( ||Y||^2 - ||Z||^2 + sum_{j=1}^p z_j^2 (weight_decay / (d_j^2 + weight_decay))^2 )
//!    GCV = ------------------------------------------------------------------------------------
//!                   ( n - p + sum_{j=1}^p (weight_decay  / (d_j^2 + weight_decay)) )^2
//! where Z = U' Y, z_j is the j-th element of Z and d_j is the j-th singular value of X, with X = U D V' the SVD.
//! The vector s with s_i = (weight_decay  / (d_j^2 + weight_decay)) must also be pre-computed.
real GCVfromSVD(real n, real Y2minusZ2, Vec Z, Vec s);

//! Perform ridge regression WITH model selection (i.e. choosing the weight decay).
//! The selection of weight decay is done in order to minimize the Generalized Cross Validation
//! (GCV) criterion(Craven & Wahba 1979). The ridge regression weights minimize
//!    min ||Y - X*W'||^2 + weight_decay ||W||^2.
//! where Y is nxm, X is nxp, W is mxp, and this procedure ALSO selects a weight_decay value.
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
//! If a positve initial_weight_decay_guess is provided, then instead of trying all the eigenvalues
//! the algorithm searches from this initial guess, never going more than explore_threshold steps
//! from the best weight decay found up to now. The weight decays tried are intermediate values
//! (geometric average) between consecutive eigenvalues.
//! Set best_GCV to the GCV of the selected weight decay and return that selected weight decay.
real ridgeRegressionByGCV(Mat X, Mat Y, Mat W, real& best_GCV, bool X_is_transposed=false, 
                          real initial_weight_decay_guess=-1, int explore_threshold=5, real min_weight_decay=0);


//! Similar to ridgeRegressionByGCV, but with support form sample weights gamma.
//! If gamma is empty, then this will perform an unweighted ridge regression based on GCV.
//! WARNING: contrary to the unweighted version, here X and Y are modified by the call
//! (this is to be more memory efficient), if you do not want this, call it with X.copy() and Y.copy().
/*! Note: I'm not 100% sure that this weighted version is really correct. 
  All it does is multiply the rows of X and Y by sqrt(gamma[i]) and then essentially the code
  is a slightly modified copy-paste of the unweighted version, where the number of samples is replaced
  by the sum of the weights in one or two spots. Somebody should more carefully check the maths!
 */
real weightedRidgeRegressionByGCV(Mat X, Mat Y, Vec gamma, Mat W, real& best_gcv, real min_weight_decay=0);


// COMMENTED OUT BECAUSE INCORRECT COMPUTATION OF GCV: CORRECT COMPUTATION DONE WITH ABOVE ROUTINES
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
//!    LOOSSE =    (sum of squared errors with the chosen weight decay) / (n_inputs - sum_i e_i/(e_i+lambda))^2
//! where e_i is an eigenvalue of  matrix inputs' * inputs. The explored values of lambda are based on an SVD
//! of the inputs matrix (whose squared singular values are the eigenvalues of the squared design matrix):
//! We know that lambda should be between the smallest and the largest eigenvalue. We do a search
//! within the eigenvalue spectrum to select lambda.
//! If best_predictions is provided then a copy of the predictions obtained with the best weight decay is made. Similarly for best_weights.
//! If a positve initial_weight_decay_guess is provided, then instead of trying all the eigenvalues
//! the algorithm searches from this initial guess, never going more than explore_threshold steps
//! from the best weight decay found up to now. The weight decays tried are intermediate values
//! (geometric average) between consecutive eigenvalues
real generalizedCVRidgeRegression(Mat inputs, Mat targets,  real& best_LOOMSE, Mat* best_weights=0, Mat* best_predictions=0, bool inputs_are_transposed=false, real initial_weight_decay_guess=-1., int explore_threshold = 5);

//! Auxiliary function used by generalizedCFRidgeRegression in order to compute the estimated generalization error
//! associated with a given choice of weight decay. The eigenvalues and eigenvectors are those of the squared design matrix.
//! The eigenvectors are in the ROWS of the matrix. 
//! The RHS_matrix is eigenvectors*inputs'*targets, pre-computed.
real LOOMSEofRidgeRegression(Mat inputs, Mat targets, Mat weights, real weight_decay, Vec eigenvalues, Mat eigenvectors, Mat predictions, Mat RHS_matrix, bool inputs_are_transposed);
#endif

// Return the affine transformation that
// is such that the transformed data has
// zero mean and identity covariance.
// The input data matrix is (n x d).
// The results are the matrix W and the bias vector 
// such that the
//   transformed_row = W row + bias
// have the desired properties.
// Note that W is obtained by doing an eigen-decomposition
// of the data covariance matrix. In case this matrix
// is ill-conditionned, the user can add a regularization
// term on its diagonal before the inversion, by providing
// a regularizer>0 as argument.
void affineNormalization(Mat data, Mat W, Vec bias, real regularizer=0);

//!  closest point to x on hyperplane that passes through all points (with weight decay)
inline Vec closestPointOnHyperplane(const Vec& x, const Mat& points, real weight_decay = 0.)
{ return transposeProduct(points, constrainedLinearRegression(points,x,weight_decay)); }

//!  Distance between point x and closest point on hyperplane that passes through all points
inline real hyperplaneDistance(const Vec& x, const Mat& points, real weight_decay = 0.)
{ return L2distance(x, closestPointOnHyperplane(x,points,weight_decay)); }

/*!   Diagonalize the sub-space spanned by the rows of X(mxn)
  with respect to symmetric matrix A(nxn), m<=n. The eigenpairs will be put
  in the evalues/evectors arguments (expressed in the basis of X),
  and the corresponding basis in R^n will be put in the solutions(kxn) matrix.
  
  The function proceeds as follows: 
  
  GramSchmid orthornormalize X, so that X X' = I(mxm)
  C(mxm) = X A X'
  solve small eigensystem C = V' S V            (V = evectors, S = evalues)
  solutions = V X
  
  Thus in the end we have
  
  solutions solutions' = V X X' V' = I if X was orthonormal to start with
  solutions A solutions' = V X A X' V' = V C V' = S
  
*/
template<class MatT>
void diagonalizeSubspace(MatT& A, Mat& X, Vec& Ax, 
                         Mat& solutions, Vec& evalues, Mat& evectors)
{
    int n_try=X.length();

    n_try=GramSchmidtOrthogonalization(X);
    X = X.subMatRows(0,n_try);

    int n_soln=solutions.length();
    //!  first collect C = X A X'
    Mat C(n_try,n_try);
    for (int i=0;i<n_try;i++)
    {
        real* Ci = C[i];
        Vec x_i=X(i);
        A.product(x_i,Ax);
        for (int j=0;j<=i;j++)
            Ci[j] = dot(X(j),Ax);
    }
    //!  symmetric part
    for (int i=0;i<n_try;i++)
    {
        real* Ci = C[i];
        for (int j=i+1;j<n_try;j++)
            Ci[j] = C(j,i);
    }

    //!  then diagonalize C = evectors' * diag(evalues) * evectors
    int n_evalues_found=0;
    Mat CC=C.copy();
    eigen_SymmMat(CC,evalues,evectors,n_evalues_found,true,n_try,true,true);
    //!  the eigen-values should be in increasing order
#if 0
    //!  check that the eigen-decomposition has worked:
    Vec Cv(n_try);
    for (int i=0;i<n_try;i++)
    {
        Vec vi=evectors(i);
        if (fabs(norm(vi)-1)>1e-5)
            cout << "norm v[" << i << "] = " << norm(vi) << endl;
        product(C, vi,Cv);
        real ncv=norm(Cv);
        if (fabs(ncv-evalues[i])>1e-5)
            cout << "C v[" << i << "] = " << ncv << " but evalue = " << evalues[i] << endl;
        real vcv = dot(vi,Cv);
        if (fabs(vcv-evalues[i])>1e-5)
            cout << "v' C v[" << i << "] = " << vcv << " but evalue = " << evalues[i] << endl;
        for (int j=0;j<i;j++)
        {
            Vec vj=evectors(j);
            real dij = dot(vi,vj);
            if (fabs(dij)>1e-5)
                cout << "v[" << i << "] . v[" << j << "] = " << dij << endl;
        }
    }
#endif

    //!  convert the eigenvectors corresponding to the smallest eigenvalues
    for (int i=0;i<n_soln;i++)
    {
        Vec xi=solutions(i);
        xi.clear(); //!<  already 0
        for (int j=0;j<n_try;j++)
            multiplyAcc(xi, X(j),evectors(i,j));
    }
#if 0
    //!  CHECK RESULT
    for (int i=0;i<n_soln;i++)
    {
        Vec xi=solutions(i);
        real normxi=norm(xi);
        if (fabs(normxi-1)>1e-5)
            cout << "norm x[" << i << "]=" << normxi << endl;
        product(A, xi,Ax); 
        cout << "Ax[" << i << "]=" << norm(Ax) << endl;
        real xAx = dot(Ax,xi);
        real err=fabs(xAx-evalues[i]);
        if (err>1e-5)
            cout << "xAx [" << i << "]=" << xAx << " but evalue = " << evalues[i] << endl;
        for (int j=0;j<i;j++)
        {
            Vec xj=solutions(j);
            err = fabs(dot(xi,xj));
            if (err>1e-5)
                cout << "|x[" << i << "] . x[" << j << "]| = " << err << endl;
        }
    }
#endif
}

} // end of namespace PLearn


#endif //!<  plapack_h


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
