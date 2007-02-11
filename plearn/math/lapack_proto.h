// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Pascal Vincent
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
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXRESS OR
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


#ifndef lapack_proto_INC
#define lapack_proto_INC

namespace PLearn {
using namespace std;

extern "C" {

    void ssyevx_(char* JOBZ, char* RANGE, char* UPLO, int* N, float* A, int* LDA, float* VL, float* VU, int* IL, int* IU, float* ABSTOL, int* M, float* W, float* Z, int* LDZ, float* WORK, int* LWORK, int* IWORK, int* IFAIL, int* INFO);
    void dsyevx_(char* JOBZ, char* RANGE, char* UPLO, int* N, double* A, int* LDA, double* VL, double* VU, int* IL, int* IU, double* ABSTOL, int* M, double* W, double* Z, int* LDZ, double* WORK, int* LWORK, int* IWORK, int* IFAIL, int* INFO);
  
    void ssyev_(char* JOBZ, char* UPLO, int* N, float* A, int* LDA, float* W, float* WORK, int* LWORK, int* INFO);
    void dsyev_(char* JOBZ, char* UPLO, int* N, double* A, int* LDA, double* W, double* WORK, int* LWORK, int* INFO);

    void sgetri_(int* N, float* A, int* LDA, int* IPIV, float* WORK, int* LWORK, int* INFO);
    void dgetri_(int* N, double* A, int* LDA, int* IPIV, double* WORK, int* LWORK, int* INFO);

    void sgetrf_(int* M, int* N, float* A, int* LDA, int* IPIV, int* INFO);
    void dgetrf_(int* M, int* N, double* A, int* LDA, int* IPIV, int* INFO);

    void sgesv_(int* N, int* NRHS, float* A, int* LDA, int* IPIV, float* B, int* LDB, int* INFO);
    void dgesv_(int* N, int* NRHS, double* A, int* LDA, int* IPIV, double* B, int* LDB, int* INFO);

    void sgesdd_(char* JOBZ, int* M, int* N, float* A, int* LDA, 
                 float* S, float* U, int* LDU, float* VT, int* LDVT,
                 float* WORK, int* LWORK, int* IWORK, int* INFO);
    void dgesdd_(char* JOBZ, int* M, int* N, double* A, int* LDA, 
                 double* S, double* U, int* LDU, double* VT, int* LDVT,
                 double* WORK, int* LWORK, int* IWORK, int* INFO);

    void ssyevr_(char* JOBZ, char* RANGE, char* UPLO, int* N, 
                 float* A, int* LDA, float* VL, float* VU, 
                 int* IL, int* IU, float* ABSTOL, int* M, float* W, 
                 float* Z, int* LDZ, int* ISUPPZ, float* WORK, 
                 int* LWORK, int* IWORK, int* LIWORK, int* INFO);
    void dsyevr_(char* JOBZ, char* RANGE, char* UPLO, int* N, 
                 double* A, int* LDA, double* VL, double* VU, 
                 int* IL, int* IU, double* ABSTOL, int* M, double* W, 
                 double* Z, int* LDZ, int* ISUPPZ, double* WORK, 
                 int* LWORK, int* IWORK, int* LIWORK, int* INFO);
  
    void ssygvx_(int* ITYPE, char* JOBZ, char* RANGE, char* UPLO, int* N, float* A, int* LDA, float* B, int* LDB, float* VL, float* VU, int* IL, int* IU, float* ABSTOL, int* M, float* W, float* Z, int* LDZ, float* WORK, int* LWORK, int* IWORK, int* IFAIL, int* INFO);
    void dsygvx_(int* ITYPE, char* JOBZ, char* RANGE, char* UPLO, int* N, double* A, int* LDA, double* B, int* LDB, double* VL, double* VU, int* IL, int* IU, double* ABSTOL, int* M, double* W, double* Z, int* LDZ, double* WORK, int* LWORK, int* IWORK, int* IFAIL, int* INFO);

    // Cholesky Decomposition
    void spotrf_(char* UPLO, int* N, float*  A, int* LDA, int* INFO);
    void dpotrf_(char* UPLO, int* N, double* A, int* LDA, int* INFO);

    // Solve linear system given Cholesky Decomposition
    void spotrs_(char* UPLO, int* N, int* NRHS, float*  A, int* LDA, float*  B, int* LDB, int* INFO);
    void dpotrs_(char* UPLO, int* N, int* NRHS, double* A, int* LDA, double* B, int* LDB, int* INFO);
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
