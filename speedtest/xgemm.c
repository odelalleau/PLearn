
/* Includes, system */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef NVIDIA
  /* Includes, cuda */
  #include <cublas.h>
#else
#include <plearn/math/blas_proto.h>
#endif
#ifdef USEDOUBLE
//  typedef double real;
#define real double
  #define xgemm_ dgemm_
#elif USEFLOAT
//  typedef float real;
#define real float
  #define xgemm_ sgemm_
#else
  #error "USEDOUBLE or USEFLOAT must be defined"
#endif
//#include <iostream>
//using namespace std;

#if defined(CXGEMM) || defined(COMPARE)
/* Host implementation of a simple version of sgemm */
static void c_xgemm(int M, int N, int K, const real alpha, const real *A, 
		  const real *B, const real beta, real *C)
{
  int i;
  int j;
  int k;
  for (i = 0; i < M; ++i) {
    for (j = 0; j < N; ++j) {
      real prod = 0;
      for (k = 0; k < K; ++k) {
	prod += A[i * K + k] * B[k * N + j];
      }
      C[i * N + j] = alpha * prod + beta * C[i * N + j];
    }
  }
}
#endif
/* Main */
int main(int argc, char** argv)
{    
  if (argc!=5){ 
    fprintf (stderr, "Usage: %s <sizeM> <sizeN> <sizeK> <Nb iter>\n",argv[0]); 
    exit(0); 
  } 
  const int M=strtol(argv[1],0,0);
  const int N=strtol(argv[2],0,0);
  const int K=strtol(argv[3],0,0);
  const int NBITER=strtol(argv[4],0,0);
  const int NA= M * K;
  const int NB= K * N;
  const int NC= M * N;
  real* h_A;
  real* h_B;
  real* h_C;
  const real alpha = 1.0f;
  const real beta = 0.0f;
#ifdef NVIDIA
  cublasStatus status;
  real* d_A = 0;
  real* d_B = 0;
  real* d_C = 0;
#endif

#ifdef COMPARE
  real* h_C_ref;
  real error_norm;
  real ref_norm;
  real diff;
#endif

    /* Allocate host memory for the matrices */
    h_A = (real*)malloc(NA * sizeof(h_A[0]));
    if (h_A == 0) {
        fprintf (stderr, "!!!! host memory allocation error (A)\n");
        return EXIT_FAILURE;
    }
    h_B = (real*)malloc(NB * sizeof(h_B[0]));
    if (h_B == 0) {
        fprintf (stderr, "!!!! host memory allocation error (B)\n");
        return EXIT_FAILURE;
    }
    h_C = (real*)malloc(NC * sizeof(h_C[0]));
    if (h_C == 0) {
        fprintf (stderr, "!!!! host memory allocation error (C)\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < NA; ++i) h_A[i] = M_PI+(real)i;
    for (int i = 0; i < NB; ++i) h_B[i] = M_PI+(real)i;

#ifdef NVIDIA
    /* Initialize CUBLAS */
    status = cublasInit();
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! CUBLAS initialization error\n");
        return EXIT_FAILURE;
    }
    /* Allocate device memory for the matrices */
    status = cublasAlloc(NA, sizeof(d_A[0]), (void**)&d_A);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! device memory allocation error (A)\n");
        return EXIT_FAILURE;
    }
    status = cublasAlloc(NB, sizeof(d_B[0]), (void**)&d_B);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! device memory allocation error (B)\n");
        return EXIT_FAILURE;
    }
    status = cublasAlloc(NC, sizeof(d_C[0]), (void**)&d_C);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! device memory allocation error (C)\n");
        return EXIT_FAILURE;
    }

    /* Initialize the device matrices with the host matrices */
    status = cublasSetVector(NA, sizeof(h_A[0]), h_A, 1, d_A, 1);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! device access error (write A)\n");
        return EXIT_FAILURE;
    }
    status = cublasSetVector(NB, sizeof(h_B[0]), h_B, 1, d_B, 1);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! device access error (write B)\n");
        return EXIT_FAILURE;
    }
    status = cublasSetVector(NC, sizeof(h_C[0]), h_C, 1, d_C, 1);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! device access error (write C)\n");
        return EXIT_FAILURE;
    }

    /* Clear last error */
    cublasGetError();
#endif
#ifdef COMPARE
    /* Performs operation using plain C code */
    for (int i=0;i<NBITER;i++)
      c_xgemm(M,N,K, alpha, h_A, h_B, beta, h_C);
    h_C_ref = h_C;
    /* Allocate host memory for reading back the result from device memory */
    h_C = (real*)malloc(NC * sizeof(h_C[0]));
    if (h_C == 0) {
        fprintf (stderr, "!!!! host memory allocation error (C)\n");
        return EXIT_FAILURE;
    }
#endif
#ifdef NVIDIA
    /* Performs operation using cublas */
    for (int i=0;i<NBITER;i++)
      //We must Change the order of the parameter as cublas take
      //matrix as colomn major and C matrix is row major
      cublasSgemm('n', 'n', N, M, K, alpha, d_B, N, d_A, K, beta, d_C, N);

    status = cublasGetError();
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! kernel execution error.\n");
        return EXIT_FAILURE;
    }
    /* Read the result back */
    status = cublasGetVector(NC, sizeof(h_C[0]), d_C, 1, h_C, 1);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! device access error (read C)\n");
        return EXIT_FAILURE;
    }
#elif defined( CXGEMM )
    for (int i=0;i<NBITER;i++)
      c_xgemm(M,N,K, alpha, h_A, h_B, beta, h_C);
#else
    char transa='N', transb='N';
    for (int i=0;i<NBITER;i++)
      sgemm_(&transb, &transa, &N, &M, &K, &alpha, h_B, &N, h_A, &K, &beta, h_C, &N);

#endif
#ifdef COMPARE
    /* Check result against reference */
    error_norm = 0;
    ref_norm = 0;
    for (int i = 0; i < NC; ++i) {
        diff = h_C_ref[i] - h_C[i];
        error_norm += diff * diff;
        ref_norm += h_C_ref[i] * h_C_ref[i];
    }
    error_norm = (float)sqrt((double)error_norm);
    ref_norm = (float)sqrt((double)ref_norm);
    if (fabs(ref_norm) < 1e-7) {
        fprintf (stderr, "!!!! reference norm is 0\n");
        return EXIT_FAILURE;
    }
    printf( "Test %s\n", (error_norm / ref_norm < 1e-6f) ? "PASSED" : "FAILED");
#endif

    /* Memory clean up */
    free(h_A);
    free(h_B);
    free(h_C);

#ifdef NVIDIA
    status = cublasFree(d_A);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! memory free error (A)\n");
        return EXIT_FAILURE;
    }
    status = cublasFree(d_B);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! memory free error (B)\n");
        return EXIT_FAILURE;
    }
    status = cublasFree(d_C);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! memory free error (C)\n");
        return EXIT_FAILURE;
    }

    /* Shutdown */
    status = cublasShutdown();
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! shutdown error (A)\n");
        return EXIT_FAILURE;
    }
#endif
    //    if (argc <= 1 || strcmp(argv[1], "-noprompt")) {
    //        printf("\nPress ENTER to exit...\n");
    //        getchar();
    //    }
    return EXIT_SUCCESS;
}
