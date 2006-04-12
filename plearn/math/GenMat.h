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


 

/* *******************************************************      
 * $Id$
 * Generic Matrix (template) operations
 * AUTHORS: Yoshua Bengio
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/Mat_maths.h */

#ifndef GenMat_INC
#define GenMat_INC

#include <plearn/math/TMat_maths.h>
#include <plearn/math/random.h>
#include <plearn/math/parpack.h>

namespace PLearn {
using namespace std;


/*!   represents A*A procedurally, where A is a square matrix 
  represented by any "standard" matrix type, i.e., which has the
  the following operations:
  void product(const Vec& x, Vec& y); <  y = A * x
  void diagonalOfSquare(Vec& d); <  d[i] = |A[i]|^2 where A[i] could either be i-th row or col
  int length();
  int width();
  This class also assumes that A is symmetric
  in its implementation of the diag method.
*/
template<class MatT>
class SquaredSymmMatT 
{
protected:
    MatT& A_;
    Vec Ax;
public:
    SquaredSymmMatT(MatT& A) : A_(A), Ax(A.length()) 
    { 
        if (A.length() != A.width()) 
            PLERROR("SquaredSymmMatT expects a square symmetric matrix"); 
    }
    int length() const { return A_.length(); }
    int width() const { return A_.width(); }
    //!  y = A * A * x
    void product(const Vec& x, Vec& y)
    {
        product(A_, x,Ax);
        product(A_, Ax,y);
    }

    void diag(Vec& d) { diagonalOfSquare(A_, d); }
    void diagonalOfSquare(Vec& d) 
    { PLERROR("SquaredSymmMatT::diagonalOfSquare not implemented"); }

};

/*!   represents (alpha*I-A) procedurally, where A is a square matrix 
  represented by any "standard" matrix type, i.e., which has the
  the following operations:
  void product(const Vec& x, Vec& y); <  y = A * x
  void diagonalOfSquare(Vec& d); <  d[i] = |A[i]|^2 where A[i] could either be i-th row or col
  void diag(Vec& d); <  d[i] = A[i,i]
  int length();
  int width();
*/
template<class MatT>
class ReverseMatT 
{
protected:
    MatT& A_;
    real alpha_;
public:
    ReverseMatT(MatT& A, real alpha) : A_(A), alpha_(alpha)
    { 
        if (A.length() != A.width()) 
            PLERROR("ReverseMatT expects a square symmetric matrix"); 
    }
    int length() const { return A_.length(); }
    int width() const { return A_.width(); }
    //!  y = alpha * x - A * x
    void product(const Vec& x, Vec& y)
    {
        product(A_, x,y);
        y*=-1;
        multiplyAcc(y, x,alpha_);
    }

    void diag(Vec& d) 
    { 
        diag(A_, d); 
        diag*=-1;
        diag+=alpha_;
    }
    void diagonalOfSquare(Vec& d) 
    {
        Vec diag_A(A_.length());
        diag(A_, diag_A);
        diagonalOfSquare(A_, d);
        multiplyAcc(d, diag_A,-2*alpha_);
        d+=alpha_*alpha_;
    }

};

/*!   Represents A + sum_{t=1}^T x_t x_t' procedurally
  keep a generic matrix A and a set of vectors x_t,
  and performing the matrix vector product with A v + sum_t x_t (x_t . v)
  The generic matrix A must have the following operations:
  
  void product(const Vec& x, Vec& y); <  y = A * x
  void diagonalOfSquare(Vec& d); <  d[i] = |A[i]|^2 where A[i] could either be i-th row or col
  void diag(Vec& d); <  d[i] = A[i,i]
  int length();
  int width();
*/
template<class MatT>
class MatTPlusSumSquaredVec
{
public:
    MatT& A_;
    Mat X_; //!<  the rows of X are the above x_t's
public:
    MatTPlusSumSquaredVec(MatT& A, int alloc_n_vectors) : A_(A), X_(alloc_n_vectors,A.length())
    { X_.resize(0,A.length()); }
    MatTPlusSumSquaredVec(MatT& A, Mat& X) : A_(A), X_(X) 
    { 
        if (X.width()!=A.length() || A.length()!=A.width()) 
            PLERROR("MatTPlusSumSquaredVec(MatT,Mat) arguments don't match"); 
    }
    void squaredVecAcc(Vec& x)
    {
        X_.resize(X_.length()+1,X_.width());
        X_(X_.length()-1) << x;
    }
    int length() const { return A_.length(); }
    int width() const { return A_.width(); }
    //!  y = A + sum_t x_t x_t' x
    void product(const Vec& x, Vec& y)
    {
        product(A_, x,y);
        for (int t=0;t<X_.length();t++)
        {
            Vec x_t = X_(t);
            multiplyAcc(y, x_t,dot(x_t,x));
        }
    }

    void diag(Vec& d) 
    { 
        diag(A_, d); 
        for (int t=0;t<X_.length();t++)
            squareAcc(d, X_(t));
    }
    void diagonalOfSquare(Vec& d) 
    { PLERROR("MatTPlusSumSquaredVec::diagonalOfSquare not implemented"); }
};


//!  for debugging
#if 0
// for debugging
#define MatT Mat
inline bool SolveLinearSymmSystemByCG(MatT A, Vec x, const Vec& b, int& max_iter, real& tol, real lambda)
{
    real resid;
    int n=A.nrows();
    Vec p(n), z(n), q(n), invdiag(n);
    real alpha, beta, rho, previous_rho;
    real normb = norm(b);

    // inverse diagonal of A, for preconditionning
    diag(A, invdiag);
    if (lambda>0)
        invdiag+=lambda;
    invertElements(invdiag);

    Vec r(n);
    // r = b - (A+lambda*I)*x;
    product(A, x,r);
    if (lambda>0)
        multiplyAcc(r, x,lambda);
    r*=-1;
    r+=b;

    if (normb == 0.0) 
        normb = 1;
  
    resid = norm(r);
    //cout << "at 0, |r| = " << resid << endl;
    if ((resid / normb) <= tol) {
        tol = resid;
        max_iter = 0;
        return true;
    }

    for (int i = 1; i <= max_iter; i++) {
        // z = M.solve(r); i.e. solve M z = r, i.e. diag(A+lambda I) z = r
        // i.e. z_i = r_i / (A_{i,i} + lambda)
        multiply(r,invdiag,z);

        rho = dot(r, z);
    
        if (i == 1)
            p << z;
        else {
            beta = rho / previous_rho;
            // p = z + beta * p;
            multiplyAdd(z,p,beta,p);
        }
    
        // q = (A+lambda I)*p;
        product(A, p,q);
        multiplyAcc(q, p,lambda);

        alpha = rho / dot(p, q);

        // x += alpha * p;
        multiplyAcc(x, p,alpha);
        // r -= alpha * q;
        multiplyAcc(r, q,-alpha);
        resid = norm(r);
        // cout << "at " << i << ", |r| = " << resid << endl;
        if ((resid / normb) <= tol) {
            tol = resid;
            max_iter = i;
            return true;     
        }

        previous_rho = rho;
    }
  
    tol = resid;
    return false;
}
#undef MatT
#else
//!  general purpose but non-debuggable with current gdb...
template <class MatT>
bool SolveLinearSymmSystemByCG(MatT A, Vec x, const Vec& b, int& max_iter, real& tol, real lambda)
{
    real resid;
    int n=A.length();
    Vec p(n), z(n), q(n), invdiag(n);
    real alpha, beta, rho, previous_rho;
    real normb = norm(b);

    //!  inverse diagonal of A, for preconditionning
    diag(A, invdiag);
    if (lambda>0)
        invdiag+=lambda;
    invertElements(invdiag);

    Vec r(n);
    //!  r = b - (A+lambda*I)*x;
    product(A, x,r);
    if (lambda>0)
        multiplyAcc(r, x,lambda);
    r*=-1;
    r+=b;

    if (normb == 0.0) 
        normb = 1;
  
    resid = norm(r);
    //cout << "at 0, |r| = " << resid << endl;
    if ((resid / normb) <= tol) {
        tol = resid;
        max_iter = 0;
        return true;
    }

    for (int i = 1; i <= max_iter; i++) {
        //!  z = M.solve(r); i.e. solve M z = r, i.e. diag(A+lambda I) z = r
        //!  i.e. z_i = r_i / (A_{i,i} + lambda)
        multiply(r,invdiag,z);

        rho = dot(r, z);
    
        if (i == 1)
            p << z;
        else {
            beta = rho / previous_rho;
            //!  p = z + beta * p;
            multiplyAdd(z,p,beta,p);
        }
    
        //!  q = (A+lambda I)*p;
        product(A, p,q);
        multiplyAcc(q, p,lambda);

        alpha = rho / dot(p, q);

        //!  x += alpha * p;
        multiplyAcc(x, p,alpha);
        //!  r -= alpha * q;
        multiplyAcc(r, q,-alpha);
        resid = norm(r);
        //!  cout << "at " << i << ", |r| = " << resid << endl;
        if ((resid / normb) <= tol) {
            tol = resid;
            max_iter = i;
            return true;     
        }

        previous_rho = rho;
    }
  
    tol = resid;
    return false;
}
#endif

/*!   Perform the "power iteration" algorithm to find the maximum eigen-pair
  of a generic matrix A, until convergence of the
  "eigenvalue" as estimated by the Rayleigh quotient x'Ax/(x'x).
  x_t = A x_{t-1} / norm(A x_{t-1})
  The Rayleigh quotient tolerance is the required fraction of increase of x_t'Ax_t.
  The algorithm would also stop if the given number of iterations is reached.
  Upon return the actual number of iterations is set in n_iterations.
  The function returns the final value of |Ax|/|x| (which is an estimator
  of the largest eigenvalue (associated to x, if x is an eigenvector), and copies N 
  last x_t's in the rows of final_vectors matrix (of length N). N should be at least 3.
  The row of the final vector is given in final_offset (it is the one
  which should have the largest Rayleigh quotient).
*/
#if 0
//!  for debugging
#define MatT Mat
inline real PowerIteration(MatT& A, Vec x0, int& n_iterations, 
                           real RayleighQuotientTolerance, Mat final_vectors, 
                           int& final_offset)
{
    int N=final_vectors.length();
    if (N<3) PLERROR("PowerIteration: final_vectors.length_ = %d should be >= 3",N);
    Vec previous=final_vectors(0);
    Vec current=final_vectors(1);
    Vec next=final_vectors(2);
    previous << x0; 
    product(A, previous,current);
    real current_norm=norm(current), next_norm, current_Rq, previous_Rq=0;
    current/=current_norm;
    for (int it=1;it<=n_iterations;it++)
    {
        product(A, current,next);
        // check Rayleigh quotient (note that norm(current)=1)
        current_Rq = dot(current,next);
        // normalize
        next_norm = norm(next);
        next/=next_norm;
        cout << "at iteration " << it << ", R(A,x) = " << current_Rq << ", |Ax|/|x| = " 
             << next_norm << endl;
        if (current_Rq < previous_Rq)
            PLWARNING("PowerIteration: something strange, x'Ax/x'x is decreasing %g->%g",
                      previous_Rq, current_Rq);
        if (it>=N && current_Rq / previous_Rq - 1 < RayleighQuotientTolerance)
        {
            // stop
            n_iterations = it;
            final_offset = it%N;
            return current_Rq;
        }
        // iterate
        previous_Rq = current_Rq;
        current_norm = next_norm;
        previous = current;
        current = next;
        next = final_vectors((it+2)%N);
    }
}
#undef MatT
#else
//!  do it with templates
template<class MatT>
real PowerIteration(MatT& A, Vec x0, int& n_iterations, 
                    real RayleighQuotientTolerance, Mat final_vectors, 
                    int& final_offset)
{
    int N=final_vectors.length();
    if (N<3) PLERROR("PowerIteration: final_vectors.length_ = %d should be >= 3",N);
    Vec previous=final_vectors(0);
    Vec current=final_vectors(1);
    Vec next=final_vectors(2);
    previous << x0; 
    product(A, previous,current);
    real current_norm=norm(current), next_norm, current_Rq, previous_Rq=0;
    current/=current_norm;
    for (int it=1;it<=n_iterations;it++)
    {
        product(A, current,next);
        //!  check Rayleigh quotient (note that norm(current)=1)
        current_Rq = dot(current,next);
        //!  normalize
        next_norm = norm(next);
        next/=next_norm;
        //cout << "at iteration " << it << ", R(A,x) = " << current_Rq << ", |Ax|/|x| = " 
        //!      << next_norm << endl;
        if (current_Rq < previous_Rq)
            PLWARNING("PowerIteration: something strange, x'Ax/x'x is decreasing %g->%g",
                      previous_Rq, current_Rq);
        if (it>=N && current_Rq / previous_Rq - 1 < RayleighQuotientTolerance)
        {
            //!  stop
            n_iterations = it;
            final_offset = it%N;
            return next_norm;
        }
        //!  iterate
        previous_Rq = current_Rq;
        current_norm = next_norm;
        previous = current;
        current = next;
        next = final_vectors((it+2)%N);
    }
}
#endif

/*!     Orthonormalize in-place the rows of the given matrix, using successive
  projections on the orthogonal subspace of the previously found
  basis. The resulting matrix has the following properties:
  - its rows spans the same space as A
  - its rows are orthogonal (dot product = 0)
  - its rows are of norm 1
  However, it may happen that the original rows of A were not linearly
  independent. In that case the, algorithm returns the number of rows
  that were successfully obtained (and the user should probably 
  then do A = A.subMatRows(0,result) to obtain the basis).
  The tolerance argument is the minimum value of the norm
  of a row when projected orthogonal to the previous ones for this row
  to contribute to the basis.
*/
int GramSchmidtOrthogonalization(Mat A, real tolerance=1e-6);

/*!   Perform a power iteration to find the largest eigen-pairs of a
  generic (square) matrix A (i.e. quasi-eigenvectors whose eigenvalues
  are the largest in magnitude). The algorithm essentially iterates
  x_t = A x_{t-1}
  where x0 is provided in argument. The user specifies the
  maximum number of iterations in n_iterations (and upon return
  this variable contains the actual number of iterations taken).
  The iterations also stop if the Rayleigh quotient does not
  improve by more than the fraction RayleighQuotientTolerance.
  The last N set of x_t's that were visited will be in the
  rows of final_vectors (where N is specified by the length of this
  matrix, and N must be at least 3 for temporary storage).
  Since the algorithm uses final_vectors as a shift register for
  the x_t's the final x_T will be at row final_offset.
  Returns the estimated eigenvalue of x_T, i.e. |A x_T|.
  Note also that the final_vectors all have norm 1 but are
  not generally orthogonal.
*/
template <class MatT>
real PowerIteration(MatT A, Vec x0, int& n_iterations, 
                    real RayleighQuotientTolerance, Mat final_vectors, 
                    int& final_offset, bool verbose=false)
{
    int N=final_vectors.length();
    if (N<3) PLERROR("PowerIteration: final_vectors.length_ = %d should be >= 3",N);
    Vec previous=final_vectors(0);
    Vec current=final_vectors(1);
    Vec next=final_vectors(2);
    previous << x0; 
    real max_x = max(previous);
    if (max_x<0) previous*=-1;
    product(A, previous,current);
    real current_norm=norm(current), next_norm, current_Rq, previous_Rq=0;
    max_x = max(current);
    if (max_x<0) current*=-1;
    current/=current_norm;
    int it=1;
    for (;it<=n_iterations;it++)
    {
        product(A, current,next);
        //!  check Rayleigh quotient (note that norm(current)=1)
        current_Rq = dot(current,next);
        //!  normalize
        next_norm = norm(next);
        next/=next_norm;
        max_x = max(next);
        if (max_x<0) next*=-1;
        if (verbose)
        {
            cout << "at iteration " << it << ", R(A,x) = " << current_Rq << ", |Ax|/|x| = " 
                 << next_norm << endl;
        }
        if (current_Rq < previous_Rq)
            PLWARNING("PowerIteration: something strange, x'Ax/x'x is decreasing %g->%g",
                      previous_Rq, current_Rq);
        if (it>=N && current_Rq / previous_Rq - 1 < RayleighQuotientTolerance)
        {
            //!  stop
            n_iterations = it;
            final_offset = it%N;
            if (verbose)
                cout << "power iteration finishes with |Ax|/|x| = " << next_norm << endl;
            return next_norm;
        }
        //!  iterate
        previous_Rq = current_Rq;
        current_norm = next_norm;
        previous = current;
        current = next;
        next = final_vectors((it+2)%N);
    }
    final_offset = it%N;
    if (verbose)
        cout << "power iteration finishes FAILING with |Ax|/|x| = " << next_norm << endl;
    return next_norm;
}

/*!   Perform an inverse power iteration to find the smallest eigen-pairs of a
  generic (square and symmetric positive semi-definite) matrix A 
  (i.e. quasi-eigenvectors whose eigenvalues
  are the smallest in magnitude). The algorithm essentially iterates
  SOLVE x_t FOR A x_t = x_{t-1}
  using conjugate gradients (the SolveLinearSymmSystemByCG function) to solve,
  and where x0 is provided in argument. The user specifies the
  maximum number of iterations in n_iterations (and upon return
  this variable contains the actual number of iterations taken).
  The iterations also stop if the Rayleigh quotient does not
  improve (decrease) by more than the fraction RTolerance.
  The last N set of x_t's that were visited will be in the
  rows of final_vectors (where N is specified by the length of this
  matrix, and N must be at least 3 for temporary storage).
  Since the algorithm uses final_vectors as a shift register for
  the x_t's the final x_T will be at row final_offset.
  The conjugate gradients procedure needs a regularizer:
  it actually solves the system (A + regularizer I) x_t = x_{t-1}
  and if A is ill-conditionned it is important to provide one.
  Returns the "error", i.e. | A x_T| (which should be as small as possible)
  and is an estimator of the associated eigen-value.
  Note also that the final_vectors all have norm 1 but are
  not generally orthogonal.
*/
template<class MatT>
real InversePowerIteration(MatT A, Vec x0, int& n_iterations, 
                           int max_n_cg_iter,
			   real RTolerance, Mat final_vectors, 
			   int& final_offset, real regularizer, bool verbose=false)
{
    int N=final_vectors.length();
    if (N<2) PLERROR("PowerIteration: final_vectors.length_ = %d should be >= 2",N);
    Vec previous=final_vectors(0);
    Vec current=final_vectors(1);
    previous << x0; 
    real max_x = max(previous);
    if (max_x<0) previous*=-1;
    real current_Rq, previous_Rq=0;
    max_x = max(current);
    if (max_x<0) current*=-1;
    normalize(previous);
    int it=1;
    Vec Ax = x0; //!<  save memory but destroy argument
    real current_error =0;
    for (;it<=n_iterations;it++)
    {
        int CGniter = max_n_cg_iter;
        real residue = RTolerance;
        current << previous;
        bool success=SolveLinearSymmSystemByCG(A, current, previous, CGniter, residue, regularizer);
        if (verbose)
        {
            if (success)
                cout << "done CG in " << CGniter << " iterations with residue = " << residue << endl;
            else
                cout << "done incomplete CG in " << CGniter << " iterations with residue = " << residue << endl;
        }
        max_x = max(current);
        if (max_x<0) current*=-1;
        normalize(current);
        //!  check Rayleigh quotient (note that norm(current)=1)
        product(A, current,Ax);
        current_Rq = dot(current,Ax);
        current_error = norm(Ax);
        if (verbose)
            cout << "at iteration " << it << ", R(A,x) = " << current_Rq << ", |Ax|/|x| = " 
                 << current_error << endl;
        if (verbose && current_Rq > (1+RTolerance)*previous_Rq)
            PLWARNING("InversePowerIteration: something strange, x'Ax/x'x is decreasing %g->%g",
                      previous_Rq, current_Rq);
        if (it>=N && 1 - current_Rq / previous_Rq  < RTolerance)
        {
            //!  stop
            n_iterations = it;
            final_offset = it%N;
            if (verbose)
                cout << "inverse power iteration finishes with |Ax|/|x| = " << current_error << endl;
            return current_error;
        }
        //!  iterate
        previous_Rq = current_Rq;
        previous = current;
        current = final_vectors((it+1)%N);
    }
    final_offset = it%N;
    if (verbose)
        cout << "power iteration finishes FAILING with |Ax|/|x| = " << current_error << endl;
    return current_error;
}

/*!   Tries to find the smallest magnitude eigen-value / eigen-vector pair
  of a generic symmetric matrix A, i.e., the smallest lambda
  and corresponding x such that
  A x = lambda x
  |x|=1
  The argument x is the initial tentative solution (and also where
  the solution is stored upon return). The algorithm proceeds
  iteratively with a call to InversePowerIteration in the inner loop,
  keeping the last 5 points visited in the inverse power iteration,
  diagonalizing them, and restarting the inverse power iteration with
  the vector corresponding to the smallest eigenvalue of the 
  diagonalized system. The user must provide an error tolerance 
  (to stop if |A x| < error_tolerance), an improvement_tolerance
  (to stop if x'Ax does not get down by this fraction in one iteration,
  the maximum number of conjugate gradients iteration during the
  InversePowerIteration, the maximum number of inverse power iterations,
  and wether to output verbose messages on cout.
*/
template<class MatT>
real findSmallestEigenPairOfSymmMat(MatT& A, Vec x, 
                                    real error_tolerance=1e-3,
                                    real improvement_tolerance=1e-4, 
                                    int max_n_cg_iter=0, 
                                    int max_n_power_iter=0, bool verbose=false)
{
    int n=A.length();
    int n_try=5;

    if (max_n_cg_iter==0)
        max_n_cg_iter = 5+int(pow(double(n),0.3));
    if (max_n_power_iter==0)
        max_n_power_iter = 5+int(pl_log(n));

    Mat try_solutions(n_try,n);
    Mat kernel_solutions = x.toMat(1,n);
    Vec Ax(n);

    int max_iter = int(sqrt(max_n_power_iter));
    real err=1e30, prev_err=1e30;
    int nrepeat=0;
    do {
        int n_iter=max_iter;
        int offs=0;
        real l0 = InversePowerIteration(A,x,n_iter,max_n_cg_iter,
                                        improvement_tolerance,
                                        try_solutions,offs,error_tolerance,verbose);
        if (verbose)
            cout << "got smallest eigenvalue = " << l0
                 << " in " << n_iter << " iterations" << endl;

        if (verbose)
        {
            //!  let's see if this is really a good solution
            //!  Vec y = try_solutions(offs);
            n_try = try_solutions.length();
            for (int i=0;i<n_try;i++)
            {
                cout << "for solution " << i << endl;
                Vec y = try_solutions(i);
                normalize(y);
                product(A, y,Ax);
                prev_err=err;
                err = norm(Ax);
                cout << "|A y| = " << err << endl;
                cout << "y. A y = " << dot(y,Ax) << endl;
            }
        }
        //!  diagonalize the subspace found by the trial solutions

        Vec evalues(n_try);
        Mat evectors(n_try,n_try);
        diagonalizeSubspace(A, try_solutions, Ax, kernel_solutions, evalues, evectors);
        product(A, x,Ax);
        err = norm(Ax);
        if (verbose)
            cout << "after diagonalization, err = " << err << endl;
        nrepeat++;
    } while (err > error_tolerance && n_try>=2 && 
             prev_err/err-1>improvement_tolerance && nrepeat<max_iter);
    if (verbose)
        cout << "return from findSmallestEigenPairOfSymmMat with err=" << err << endl;
    return err;
}


/*!   Try to find the null space of a symmetric semi-definite positive
  generic nxn matrix A, based on the inverse power iteration. 
  The algorithm looks for the m smallest-magnitude
  eigen-pairs of A. The iterations to find a null space vector x
  stop when |A x| < error_tolerance, or when x'Ax does not improve
  (decrease) by a fraction more than improvement_tolerance. 
  This method uses an inverse power iteration, which iterates
  at most max_n_power_iter times through the solution of a
  linear system by conjugate gradients (which itself takes
  at most max_n_cg_iter iterations). If the values of max_n_cg_iter
  or max_n_power_iter are left to the default of 0, then a
  value is automatically chosen. The function sets the
  results in the mxn matrix 'solutions' (an orthormal basis
  for the resulting sub-space), and sets normsAx[i] = |A x_i|
  and xAX[i] = x_i'A x_i for each row x_i of the solutions matrix
  (i=0 to m-1). It is possible that the algorithm finds less than m solutions:
  in that case they lie in the first rows of the solutions matrix;
  the number of obtained solutions is returned.
*/
template<class MatT>
int SymmMatNullSpaceByInversePowerIteration(MatT &A, Mat solutions,
                                            Vec normsAx, Vec xAx,  
                                            real error_tolerance=1e-3,
                                            real improvement_tolerance=1e-4, 
                                            int max_n_cg_iter=0, 
                                            int max_n_power_iter=0, bool verbose=false)

{  
    int n=A.length();
    int n_soln=normsAx.length();
    solutions.resize(n_soln,n);
    if (max_n_cg_iter==0)
        max_n_cg_iter = 5+int(pow(double(n),0.3));
    if (max_n_power_iter==0)
        max_n_power_iter = 5+int(pl_log(n));
    Vec Ax(n);

    MatTPlusSumSquaredVec<MatT> B(A,n_soln);
    Vec sy(n);
    fill_random_uniform(sy);
    Mat largest_evecs(3,n);
    int offs;
    int n_iter = MIN(max_n_power_iter,20);
    real largest_evalue = PowerIteration(A, sy, n_iter ,1e-3,
                                         largest_evecs, offs,verbose);
    if (verbose)
        cout << "largest evalue(B) = " << largest_evalue << endl;
    for (int i=0;i<n_soln;i++)
    {
        Vec y = solutions(i);
        if (i==0)
            y.fill(1);
        else
            fill_random_uniform(y,0.1,0.5);
        real residue=findSmallestEigenPairOfSymmMat(B, y, 
                                                    error_tolerance,
                                                    improvement_tolerance,
                                                    max_n_cg_iter,
                                                    max_n_power_iter,verbose);
        if (verbose)
        {
            cout << "found vector with |B y| = " << residue << endl;
            cout << "|y| = " << norm(y) << endl;
            product(A, y,Ax);
            cout << "****** |A y| = " << norm(Ax) << endl;
        }
        multiply(y,largest_evalue,sy);
        B.squaredVecAcc(sy);
    }

    int n_s=GramSchmidtOrthogonalization(solutions);
    solutions = solutions.subMatRows(0,n_s);
    if (n_s<n_soln && verbose)
        cout << "found only " << n_s << " independent solutions out of " 
             << n_soln << " requested" << endl;
    for (int i=0;i<n_s;i++)
    {
        Vec xi = solutions(i);
        //!  check x'Ax and norm of |Ax|
        product(A, xi,Ax);
        real err = dot(xi,Ax);
        real normAx = norm(Ax);
        normsAx[i]=normAx;
        xAx[i]=err;
        if (verbose)
            cout << "for " << i << "-th solution x: x'Ax = " << err << ", |Ax|/|x|= "
                 << normAx << endl;
    }
    return n_s;
}

/*!   Find small (ideally the smallest) eigen-pairs of a positive
  semi-definite generalized nxn matrix A, using stochastic 
  gradient descent on the cost function (sum_i x_i'A x_i)/m 
  with parameters X (whose rows are x_i).
  The algorithm looks for the m smallest-magnitude
  eigen-pairs of A (the longer you iterate, the smaller the magnitude). 
  The function sets the results in the mxn matrix X, with in its rows an 
  (optionally orthormal) basis for the resulting sub-space.
  If the optional argument diagonalize_in_the_end is set to true
  then the resulting vectors (rows of X) are orthonormalized and 
  diagonalized  in the last step (in increasing order of eigen-value),
  so the "best" eigenpairs are the first ones.
  The user controls the computational resources with either/or of
  the n_epochs (number of iterations through A, each costing mxN operations
  if the generalized matrix A has N non-zero elements) and tolerance.
  For the "default" value for n_epochs (0) the program chooses 1000+10*sqrt(n).
  The tolerance represents the tolerable value of the cost function to stop
  the iterations (when the x_i's are normalized).
  For the default value of the learning_rate (0), the maximum eigenvalue
  of A is computed. In that case tolerance represents a fraction: the actual 
  error tolerance is set to tolerance * max_evalue. Otherwise the tolerance
  argument gives the actual value of the error tolerance.
  The other arguments are also optional and in general
  do not need to be set. The normalize_every argument specifies
  how often (every how many epochs) to renormalize the rows of X. 
  The default value of 0 (i.e. never) works well in most cases. The 
  learning rate is set by default to 2/largest_eigenvalue_of_A (when
  the default value of 0 is given). If the maximum eigen-value is 
  known or a better value is known, the computation
  of that eigenvalue (through the PowerIteration method)
  can be skipped by providing a learning_rate > 0 (don't forget
  that it changes the semantic of the tolerance argument). 
  The decrease_factor is the rate of decrease of the
  learning rate through the iterations, and the default
  value of 0 has been found to be generally optimal.
  The actual number of iterations taken is returned.
*/
template<class MatT>
int GDFindSmallEigenPairs(MatT& A,Mat X,
                          bool diagonalize_in_the_end=true,
                          real tolerance=1e-6,
                          int n_epochs=0,
                          real learning_rate=0,
                          int normalize_every=0,
                          real decrease_factor=0,
                          bool verbose=false)
{
    int n=A.length();
    int m=X.length();
    if (n_epochs==0)
        n_epochs = 1000+10*int(sqrt(n));
    Vec Ax(n);
    real err_tolerance, sum_norms, actual_err;
    if (learning_rate==0)
    {
        //!  initialize learning rate using a power iteration
        fill_random_uniform(Ax,-1,1);
        Mat large_vectors(3,n);
        int offs;
        int n_iter = 10+int(sqrt(pl_log(double(n))));
        real max_eigen_value = 
            PowerIteration(A, Ax, n_iter,1e-3,large_vectors,offs,verbose);
        learning_rate = 2.0/max_eigen_value;
        if (verbose)
        {
            cout << "setting initial learning rate = 2/max_eigen_value = 2/" 
                 << max_eigen_value << " = " << learning_rate << endl;
        }
        err_tolerance = tolerance * max_eigen_value;
    }
    else err_tolerance = tolerance;
    real prev_err=1e30;
    int it=0;
    for (;it<n_epochs;it++)
    {
        real learning_rate = learning_rate / (1+it*decrease_factor);
        real err=0;
        for (int i=0;i<n;i++)
        {
            real* xi = &X(0,i);
            for (int d=0;d<m;d++, xi+=n)
            {
                //!  stochastic gradient descent step
                real gradient = matRowDotVec(A, i,X(d));
                *xi -= learning_rate * gradient;
                err += gradient * *xi;
            }
        }
        if (verbose)
        {
            cout << "at iteration " << it << " of gradient descent, est. err.= " << err << endl;
            cout << "lrate = " << learning_rate << endl;
        }
        if (tolerance>0)
        {
            sum_norms = 0;
            for (int d=0;d<m;d++)
                sum_norms += norm(X(d));
            actual_err = err / (m*sum_norms);
            if (actual_err<err_tolerance) break;
        }
        if (err>prev_err)
            cout << "at iteration " << it << " of gradient descent, est. err.= " << err 
                 << " > previous error = " << prev_err << " ; lrate = " << learning_rate << endl;
        //!  cout << normalize_every << " " << normalize_every>0?:it%normalize_every:0 << endl;
        if (verbose)
            for (int d=0;d<m;d++)
                cout << "norm(x[" << d << "])=" << norm(X(d)) << endl;
        if (normalize_every!=0 && it%normalize_every==0)
        {
            int new_m=GramSchmidtOrthogonalization(X,1e-9);
            for (int e=new_m;e<m;e++)
                fill_random_uniform(X(e)); //!<  RESET LINEARLY DEPENDENT FEATURES RANDOMLY
            if (verbose)
            {
                //!  debug
                real C=0;
                for (int d=0;d<m;d++)
                {
                    Vec xd = X(d);
                    product(A, xd,Ax);
                    C += dot(xd,Ax);
                    cout << "for " << d << ", |Ax|/|x| = " << norm(Ax) << endl;
                }
                cout << "actual cost = " << 0.5*C << endl;
            }
        }
    }
    if (diagonalize_in_the_end)
    {
        Mat diagonalized_solutions(m,n);
        Mat subspace_evectors(m,m);
        Vec subspace_evalues(m);
        diagonalizeSubspace(A,X,Ax,diagonalized_solutions,subspace_evalues,subspace_evectors);
        X << diagonalized_solutions;
    }
    return it;
}

/*! Perform kernel PCA on a set of objects for which the dot product between
    each pair of objects is provided. The dot-product matrix can be sparse, for objects
    which are "far" from each other. An embedding for each object is returned,
    which attempts to respect these dot products. The dimension of the embedding
    is specified by the user-set dimensions of the embedding matrix (n_objects x embedding_dimension).
*/
template<class MatT>
int kernelPCAfromDotProducts(MatT& dot_products,Mat embedding, int max_n_eigen_iter=300, real ncv2nev_ratio=1.5, Vec* eval=0, Mat* evec=0)
{
    int n=dot_products.length();
    FORTRAN_Integer m=embedding.width();
    if (embedding.length()!=n)
        PLERROR("kernelPCAfromDotProducts: expected embedding.length()==dot_products.length(), got %d!=%d",
                embedding.length(),n);
    if (dot_products.width()!=n)
        PLERROR("kernelPCAfromDotProducts: expected dot_products a square matrix, got %d x %d",
                n,dot_products.width());

    static Vec e_values;
    e_values.resize(m);
    static Mat e_vectors;
    e_vectors.resize(m,n);
    
    if (evec) *evec = e_vectors;
    if (eval) *eval = e_values;

    int err=eigenSparseSymmMat(dot_products, e_values, 
                               e_vectors, m, max_n_eigen_iter,
                               true, true, false, false, ncv2nev_ratio);
    // change the order so that the largest e-value comes first
    e_values.swap();
    e_vectors.swapUpsideDown();

    if (!(err==0 || err==1))
        return err;
    //!  extract the embedding:
    //!    embedding(object i, feature j) = e_vectors(j,i)*sqrt(e_values[j])
    static Vec feature;
    feature.resize(n);
    for (int j=0;j<m;j++)
    {
        real eval_j = e_values[j];
        if (eval_j<0)
        {
            PLWARNING("metricMultiDimensionalScaling::the matrix of dot-products is not positive-definite!, evalue=%g",eval_j);
            eval_j = -eval_j*0.2; // HEURISTIC TRICK, keep negative e-values, but smaller
        }
        real scale = sqrt(eval_j);
        feature << e_vectors(j);
        feature *= scale;
        embedding.column(j) << feature;
    }
    return 0;
}

/*!   Apply the metric multi-dimensional scaling (MDS) algorithm to a possibly sparse
  generalized matrix (nxn) of pairwise SQUARE distances between n objects. This yields
  an embedding of the objects in m-dimensional space, in the nxm embedding matrix.
  The distances matrix should be symmetric. It will be "destroyed" upon return
  (it will contain pseudo dot-products between the objects). The distances
  that are not specified are assumed to be NOT ZERO but "large", yielding
  a pseudo dot-product of zero.
*/
template<class MatT>
int metricMultiDimensionalScaling(MatT& square_distances,Mat embedding, int max_n_eigen_iter=300)
{
    int n=square_distances.length();
    FORTRAN_Integer m=embedding.width();
    if (embedding.length()!=n)
        PLERROR("MetricMultiDimensionalScaling: expected embedding.length()==square_distances.length(), got %d!=%d",
                embedding.length(),n);
    if (square_distances.width()!=n)
        PLERROR("MetricMultiDimensionalScaling: expected square_distances a square matrix, got %d x %d",
                n,square_distances.width());
    if (square_distances.size()!=n*n)
        PLERROR("MetricMultiDimensionalScaling: only works on a full, non-sparse matrix\n");

    //!  double-centering of the distances to get dot products
    static Vec avg_across_rows;
    avg_across_rows.resize(n);
    //!  average the non-zero elements across rows and columns
    columnSum(square_distances, avg_across_rows);
    avg_across_rows *= 1.0/n;
    doubleCentering(square_distances,avg_across_rows,square_distances,-0.5);
    //!  now "square_distances" are actually pseudo dot-products

    //!  do a partial SVD (which is the same as an eigen-decomposition since 
    //!  the matrix is symmetric) to find the largest eigen-pairs
    static Vec e_values;
    e_values.resize(m);
    static Mat e_vectors;
    e_vectors.resize(m,n);
    int err=eigenSparseSymmMat(square_distances, e_values, 
                               e_vectors, m, max_n_eigen_iter);
    if (!(err==0 || err==1))
        return err;
    //!  extract the embedding:
    //!    embedding(object i, feature j) = e_vectors(j,i)*sqrt(e_values[j])
    for (int j=0;j<m;j++)
    {
        real eval_j = e_values[j];
        if (eval_j<0)
            PLERROR("metricMultiDimensionalScaling::the matrix of dot-products is not positive-definite!, evalue=%g",eval_j);
        real scale = sqrt(eval_j);
        Vec feature_j = e_vectors(j);
        feature_j *= scale;
        embedding.column(j) << feature_j;
    }
    return 0;
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
