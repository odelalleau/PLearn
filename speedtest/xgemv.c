
/* Includes, system */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef NVIDIA
  /* Includes, cuda */
  #include <cublas.h>
#else
  /* Includes, cblas */
  #include <gsl/gsl_cblas.h>
#endif
#ifdef DOUBLE
  typedef double real;
  #define cblas_xgemv cblas_dgemv
#else
  typedef float real;
  #define cblas_xgemv cblas_sgemv
#endif
//#include <iostream>
//using namespace std;

#if defined(CXGEMV) || defined(COMPARE)
/* Host implementation of a simple version of sgemm */
static void c_xgemv(int M_, int N_, const real alpha, const real *A, 
		  const real *X, const real beta, real *Y)
{
  int M=M_,N=1,K=N_;
  int i;
  int j;
  int k;
  for (i = 0; i < M; ++i) {
    for (j = 0; j < N; ++j) {
      float prod = 0;
      for (k = 0; k < K; ++k) {
	prod += A[i * K + k] * X[k * N + j];
      }
      Y[i * N + j] = alpha * prod + beta * Y[i * N + j];
    }
  }
}
#endif
/* Main */
int main(int argc, char** argv)
{    
  if (argc!=4){ 
    fprintf (stderr, "Usage: %s <sizeM> <sizeN> <Nb iter>\n",argv[0]); 
    exit(0); 
  } 
  const int M=strtol(argv[1],0,0);
  const int N=strtol(argv[2],0,0);
  const int NBITER=strtol(argv[3],0,0);
  const int NA= M * N;
  const int NX= N;
  const int NY= N;
  real* h_A;
  real* h_X;
  real* h_Y;
  const real alpha = 1.0f;
  const real beta = 0.0f;
#ifdef NVIDIA
  cublasStatus status;
  real* d_A = 0;
  real* d_X = 0;
  real* d_Y = 0;
#endif

#ifdef COMPARE
  real* h_Y_ref;
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
    h_X = (real*)malloc(NX * sizeof(h_X[0]));
    if (h_X == 0) {
        fprintf (stderr, "!!!! host memory allocation error (X)\n");
        return EXIT_FAILURE;
    }
    h_Y = (real*)malloc(NY * sizeof(h_Y[0]));
    if (h_Y == 0) {
        fprintf (stderr, "!!!! host memory allocation error (Y)\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < NA; ++i) h_A[i] = M_PI+(real)i;
    for (int i = 0; i < NX; ++i) h_X[i] = M_PI+(real)i;

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
    status = cublasAlloc(NX, sizeof(d_X[0]), (void**)&d_X);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! device memory allocation error (X)\n");
        return EXIT_FAILURE;
    }
    status = cublasAlloc(NY, sizeof(d_Y[0]), (void**)&d_Y);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! device memory allocation error (Y)\n");
        return EXIT_FAILURE;
    }

    /* Initialize the device matrices with the host matrices */
    status = cublasSetVector(NA, sizeof(h_A[0]), h_A, 1, d_A, 1);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! device access error (write A)\n");
        return EXIT_FAILURE;
    }
    status = cublasSetVector(NX, sizeof(h_X[0]), h_X, 1, d_X, 1);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! device access error (write X)\n");
        return EXIT_FAILURE;
    }
    status = cublasSetVector(NY, sizeof(h_Y[0]), h_Y, 1, d_Y, 1);
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
      c_xgemv(M,N, alpha, h_A, h_X, beta, h_Y);
    h_Y_ref = h_Y;
    /* Allocate host memory for reading back the result from device memory */
    h_Y = (float*)malloc(NY * sizeof(h_Y[0]));
    if (h_Y == 0) {
        fprintf (stderr, "!!!! host memory allocation error (C)\n");
        return EXIT_FAILURE;
    }
#endif
#ifdef NVIDIA
    /* Performs operation using cublas */
    for (int i=0;i<NBITER;i++)
      //We must Change the order of the parameter as cublas take
      //matrix as colomn major and C matrix is row major?????
      cublasSgemv('n', 'n', N, M, alpha, d_X, N, d_A, K, beta, d_Y, N);

    status = cublasGetError();
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! kernel execution error.\n");
        return EXIT_FAILURE;
    }
    /* Read the result back */
    status = cublasGetVector(NY, sizeof(h_Y[0]), d_Y, 1, h_Y, 1);
    if (status != CUBLAS_STATUS_SUCCESS) {
        fprintf (stderr, "!!!! device access error (read C)\n");
        return EXIT_FAILURE;
    }
#elif defined( CXGEMV )
    for (int i=0;i<NBITER;i++)
      c_xgemv(M,N, alpha, h_A, h_X, beta, h_Y);
#else
    for (int i=0;i<NBITER;i++)
      cblas_xgemv(CblasRowMajor, CblasNoTrans, M,N, alpha, h_A, N, h_X, 1, beta, h_Y, 1);
#endif
    /*    for (int i = 0; i < NA; ++i) printf("%f,",h_A[i]);
    printf(")\n");
    for (int i = 0; i < NX; ++i) printf("%f,",h_X[i]);
    printf(")\n");
    for (int i = 0; i < NY; ++i) printf("%f,",h_Y[i]);
    printf(")\n");*/
#ifdef COMPARE
    /*    for (int i = 0; i < NY; ++i) printf("%f,",h_Y_ref[i]);
	  printf(")\n");*/
    /* Check result against reference */
    error_norm = 0;
    ref_norm = 0;
    for (int i = 0; i < NY; ++i) {
        diff = h_Y_ref[i] - h_Y[i];
        error_norm += diff * diff;
        ref_norm += h_Y_ref[i] * h_Y_ref[i];
    }
    error_norm = (float)sqrt((double)error_norm);
    ref_norm = (float)sqrt((double)ref_norm);
    if (fabs(ref_norm) < 1e-7) {
        fprintf (stderr, "!!!! reference norm is 0\n");
        return EXIT_FAILURE;
    }
    printf( "Test %s\n", (error_norm / ref_norm < 1e-6f) ? "PASSED" : "FAILED");
#endif

    /*    printf("Matrix A:\n");
    for(int i=0;i<NA;i++)
      printf("%f,",h_A[i]);
    printf("\nArray X:\n");
    for(int i=0;i<NX;i++)
      printf("%f,",h_X[i]);
    printf("\nArray Y:\n");
    for(int i=0;i<NY;i++)
      printf("%f,",h_Y[i]);
    printf("\n");
    */
    /* Memory clean up */
    free(h_A);
    free(h_X);
    free(h_Y);
#ifdef COMPARE
    free(h_Y_ref);
#endif
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
