// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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


#ifndef parpack_INC
#define parpack_INC

#include "arpack_proto.h"
#include "Mat.h"

namespace PLearn {
using namespace std;


/*!   Compute some eigenvalues, and optionally eigenvectors, of a symmetric, possibly 
  sparse, generalized matrix A. The only operation that will be performed
  on A (repetitively) is the matrix-vector product, i.e. A.product(Vec x, Vec y), 
  yielding y = A x.
  
  This uses the ARPACK library.
  
  Returns 0 if all went well. Otherwise, see the INFO values
  set by [ds][eu]pd in the ARPACK/SRC files.
  
  It is possible that only a subset of the eigenvalues are found
  (as given by n_evalues upon return, and the new size of e_vectors/e_values).
  Note also that e_vectors might be internally and temporarily 
  re-allocated to a larger size, with at most 1.5 times more rows.

  If you want the eigen values and eigen vectors to be returned in the same order 
  as in plapack's eigenVecOfSymmMat, "according_to_magnitude" must be set to false
  and you must swap the eigen values and the eigen vectors. 
  i.e. do something like:
 .............................................................
  Mat evectors(nb_principal_components,train_set.length());
  Vec evalues(nb_principal_components);
  int status;
  long int n_ev=nb_principal_components;

  status = eigenSparseSymmMat(A, evalues, 
                              evectors, n_ev, 300, 
                              true, true, false);
  if (status<0 || status>1)
    PLERROR("MyClass: eigenSparseSymmMat return error code number %d (see ARPACK dsaupd INFO variable)", status);
  if (status==1 || n_ev != nb_principal_components)
    PLERROR("MyClass: eigenSparseSymmMat computed only %d e-vectors rather than the required %d",
            n_ev, nb_principal_components);

  evalues.swap();
  evectors.swapUpsideDown();
 .............................................................

*/

template<class MatT>
int eigenSparseSymmMat(MatT& A, Vec e_values, Mat e_vectors, long int& n_evalues,
		       int max_n_iter=300, bool compute_vectors=true, bool largest_evalues=true,
		       bool according_to_magnitude=true, bool both_ends=false)
{
#ifdef NOARPACK
  PLERROR("eigenSparseSymmMat: ARPACK not available on this system!");
  return 0;
#else
  long int ido=0;
  char bmat[1];
  bmat[0] = 'I';
  char which[2];
  long int n=A.length();
  if (e_vectors.length()!=n_evalues || e_vectors.width()!=n)
    PLERROR("eigenSparseSymmMat: expected e_vectors.width(%d)=A.length(%d), e_vectors.length(%d)=e_values.length(%d)",
	  e_vectors.width(),n,e_vectors.length(),n_evalues);
  if (both_ends)
    strncpy(which,"BE",2); //!<  half of the e-values from each end of the spectrum
  else
  {
    which[0]= largest_evalues? 'L' : 'S';
    which[1]= according_to_magnitude? 'M' : 'A';
  }
  real tol=0;
  long int ncv=MIN(3+int(n_evalues*1.5),n-1);
  e_vectors.resize(ncv,n); //!<  we need some extra space...
  long int iparam[11];
  iparam[0]=1;
  iparam[2]=max_n_iter;
  iparam[6]=1;
  long int ipntr[11];
  Vec workd(3*n);
  long int lworkl = (ncv * ncv) + (ncv * 8);
  Vec workl(lworkl);
  Vec resid(n);
  long int info=0;
  for (;;) {
#ifdef USEDOUBLE
    dsaupd_(&ido, bmat, &n, which, &n_evalues, &tol, resid.data(), &ncv, e_vectors.data(), &n,
            iparam, ipntr, workd.data(), workl.data(), &lworkl, &info, 1, 2);
#endif
#ifdef USEFLOAT
    ssaupd_(&ido, bmat, &n, which, &n_evalues, &tol, resid.data(), &ncv, e_vectors.data(), &n,
            iparam, ipntr, workd.data(), workl.data(), &lworkl, &info, 1, 2);
#endif
    if (ido == -1 || ido == 1) {
      Vec x=workd.subVec(ipntr[0]-1,n);
      Vec z=workd.subVec(ipntr[1]-1,n);
      product(A, x, z);
    } else break;
  }
  if (info != 0 && info != 1)
  {
    PLWARNING("eigenSparseSymmMat: saupd returning error %ld",info);
    return info;
  }
  if (info > 0)
  {
    n_evalues = iparam[4];
    e_values.resize(n_evalues);
  }
  e_vectors.resize(n_evalues,n);
  if (n_evalues>0)
  {
    long int rvec = compute_vectors;
    TVec<long int> select(ncv);
    long int ierr;
    real sigma =0;
#ifdef USEDOUBLE
    dseupd_(&rvec, "All", select.data(), e_values.data(), e_vectors.data(), &n, &sigma, 
            bmat, &n, which, &n_evalues, &tol, resid.data(), &ncv, e_vectors.data(), &n, 
            iparam, ipntr, workd.data(), workl.data(), &lworkl, &ierr, 3, 1, 2);
#endif
#ifdef USEFLOAT
    sseupd_(&rvec, "All", select.data(), e_values.data(), e_vectors.data(), &n, &sigma, 
            bmat, &n, which, &n_evalues, &tol, resid.data(), &ncv, e_vectors.data(), &n, 
            iparam, ipntr, workd.data(), workl.data(), &lworkl, &ierr, 3, 1, 2);
#endif
    if (ierr != 0)
    {
      PLWARNING("eigenSparseSymmMat: seupd returning error %ld",ierr);
      return ierr;
    }
  }
#endif
  return info;
}


/*! Same arguments as eigenSparseSymmMat except that A is not symmetric. 
    We ignore the imaginary part if there is one.
    See ARPACK/SRC files for more details. 
    To get the eigen pairs in the same order as in plapack's eigenVecOfSymmMat, 
    do the same thing as above, but you don't have to swap the eigen vectors
    and eigen values.
*/
template<class MatT>
int eigenSparseNonSymmMat(MatT& A, Vec e_values, Mat e_vectors, long int& n_evalues,
		       int max_n_iter=300, bool compute_vectors=true, bool largest_evalues=true,
		       bool according_to_magnitude=true, bool both_ends=false)
{
#ifdef NOARPACK
  PLERROR("eigenSparseNonSymmMat: ARPACK not available on this system!");
  return 0;
#else
  long int ido=0;
  char bmat[1];
  bmat[0] = 'I';
  char which[2];
  long int n=A.length();
  if (e_vectors.length()!=n_evalues || e_vectors.width()!=n)
    PLERROR("eigenSparseNonSymmMat: expected e_vectors.width(%d)=A.length(%d), e_vectors.length(%d)=e_values.length(%d)",
	  e_vectors.width(),n,e_vectors.length(),n_evalues);
  if (both_ends)
    strncpy(which,"BE",2); //!<  half of the e-values from each end of the spectrum
  else
  {
    which[0]= largest_evalues? 'L' : 'S';
    which[1]= according_to_magnitude? 'M' : 'R';//according to magnitude or according to real part
  }
  real tol=0;
  long int ncv=MIN(3+int(n_evalues*1.5),n-1);
  e_vectors.resize(ncv,n); //!<  we need some extra space...
  long int iparam[11];
  iparam[0]=1;
  iparam[2]=max_n_iter;
  iparam[6]=1;
  long int ipntr[11];
  Vec workd(3*n);
  long int lworkl = 3*(ncv * ncv) + (ncv * 6);
  Vec workl(lworkl);
  Vec resid(n);
  long int info=0;
  for (;;) {
#ifdef USEDOUBLE
    dnaupd_(&ido, bmat, &n, which, &n_evalues, &tol, resid.data(), &ncv, e_vectors.data(), &n,
            iparam, ipntr, workd.data(), workl.data(), &lworkl, &info, 1, 2);
#endif
#ifdef USEFLOAT
    snaupd_(&ido, bmat, &n, which, &n_evalues, &tol, resid.data(), &ncv, e_vectors.data(), &n,
            iparam, ipntr, workd.data(), workl.data(), &lworkl, &info, 1, 2);
#endif
    if (ido == -1 || ido == 1) {
      Vec x=workd.subVec(ipntr[0]-1,n);
      Vec z=workd.subVec(ipntr[1]-1,n);
      product(A, x, z);
    } else break;
  }
  if (info != 0 && info != 1)
  {
    PLWARNING("eigenSparseNonSymmMat: naupd returning error %ld",info);
    return info;
  }
  Vec e_valuesIm(n_evalues+1);
  Vec workev(3*ncv);
  if (info > 0)
  {
    n_evalues = iparam[4];
    e_values.resize(n_evalues+1);
  }
  e_vectors.resize(n_evalues+1,n);
  if (n_evalues>0)
  {
    long int rvec = compute_vectors;
    TVec<long int> select(ncv);
    long int ierr;
    real sigmai =0;
    real sigmar =0;
#ifdef USEDOUBLE
    dneupd_(&rvec, "A", select.data(), e_values.data(), e_valuesIm.data(), e_vectors.data(), &n,
            &sigmar, &sigmai, workev.data(), bmat, &n, which, &n_evalues, &tol, 
            resid.data(), &ncv, e_vectors.data(), &n, iparam, ipntr, workd.data(), 
            workl.data(), &lworkl, &ierr, 3, 1, 2);
#endif
#ifdef USEFLOAT
    sneupd_(&rvec, "A", select.data(), e_values.data(), e_valuesIm.data(), e_vectors.data(), &n,
            &sigmar, &sigmai, workev.data(), bmat, &n, which, &n_evalues, &tol, 
            resid.data(), &ncv, e_vectors.data(), &n, iparam, ipntr, workd.data(), 
            workl.data(), &lworkl, &ierr, 3, 1, 2);
#endif
    if (ierr != 0)
    {
      PLWARNING("eigenSparseNonSymmMat: neupd returning error %ld",ierr);
      return ierr;
    }
  }
#endif
  return info;
}


} // end of namespace PLearn


#endif
