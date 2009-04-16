// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2005 Yoshua Bengio, Mantas Lukosevicius

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
 * AUTHORS: Pascal Vincent & Yoshua Bengio & Rejean Ducharme & Mantas Lukosevicius
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file PLearn/plearn/math/TMat_maths_impl.h */

#ifndef TMat_maths_impl_H
#define TMat_maths_impl_H

#include <algorithm>
#include <limits>
#include <plearn/sys/Profiler.h>

namespace PLearn {
using namespace std;

template <class T>
TVec<T> sign(const TVec<T>& vec)
{
    int len = vec.length();

    TVec<T> sign_( len );
    if (len > 0) {
        T*  v   = vec.data();
        T*  s   = sign_.data();

        while(--len >= 0)
        {
            *s = sign( *v );
            v++; s++;
        }
    }
    return sign_;
}

template <class T>
void compute_sign(const TVec<T>& vec, const TVec<T>& dest)
{
    int len = vec.length();
    if (len > 0) {
        T*  v   = vec.data();
        T*  s   = dest.data();
        while(--len >= 0)
        {
            *s = sign( *v );
            v++; s++;
        }
    }
}

//! v1 and v2 have their elements in increasing order.
//! Do they have at least one element in common?
//! Use the 'merge sort' principle to find out.
template <class T>
bool sortedVectorsIntersect(const TVec<T>& v1, const TVec<T>& v2)
{
    int i1=0,i2=0;
    do
    {
        T v1i = v1[i1];
        T v2i = v2[i2];
        if (v1i==v2i) return true;
        if (v1i<v2i) i1++;
        else i2++;
    }
    while (i1<v1.size() && i2<v2.size());
    return false;
}

// target is an integer between 0 and N-1
// output is a vector of N discriminant functions
// (each of which tries to separate class i from the others)
template <class T>
real one_against_all_hinge_loss(const TVec<T>& output,
                                const int target)
{
    int N = output.length();
    T total_hinge_loss = 0;
    if (N > 0) {
        T*  o = output.data();
        while(--N >= 0)
        {
            if (N==target)
                total_hinge_loss += hinge_loss(*o,1);
            else
                total_hinge_loss += hinge_loss(*o,-1);
            o++;
        }
    }
    return total_hinge_loss;
}

// target is an integer between 0 and N-1
// output is a vector of N discriminant functions
// (each of which tries to separate class i from the others)
// compute derivative of hinge loss wrt each output, in d_output
template <class T>
void one_against_all_hinge_loss_bprop(const TVec<T>& output,
                                      const int target,
                                      TVec<T> d_output)
{
    int N = output.length();
    d_output.resize(N);
    if (N > 0) {
        T*  o = output.data();
        T*  d_o = d_output.data();
        //MNT old buggy code (opposite numbering of outputs):
        /*while(--N >= 0)
          {
          if (N==target)
          *d_o = d_hinge_loss(*o,1);
          else
          *d_o = d_hinge_loss(*o,-1);
          o++; d_o++;
          }
        */
        for( int i = 0; i < N; i++ ) {
            if ( i == target )
                *d_o = d_hinge_loss( *o, 1 );
            else
                *d_o = d_hinge_loss( *o, -1 );
            o++;
            d_o++;
        }
    }
}

//! y = softmax(x)
template <class T>
void softmax(const TVec<T>& x, const TVec<T>& y)
{
    int n = x.length();
    if (n>0)
    {
        T* yp = y.data();
        T* xp = x.data();
        T maxx = max(x);
        real s = 0;
        for (int i=0;i<n;i++)
            s += (yp[i] = safeexp(xp[i]-maxx));
        if (s == 0) PLERROR("trying to divide by 0 in softmax");
        s = 1.0 / s;
        for (int i=0;i<n;i++)
            yp[i] *= s;
    }
}

//! y = softmax(-x)
template <class T>
void softmaxMinus(const TVec<T>& x, const TVec<T>& y)
{
    int n = x.length();
    if (n>0)
    {
        T* yp = y.data();
        T* xp = x.data();
        T minx = min(x);
        real s = 0;
        for (int i=0;i<n;i++)
            s += (yp[i] = safeexp(-xp[i]+minx));
        if (s == 0) PLERROR("trying to divide by 0 in softmax");
        s = 1.0 / s;
        for (int i=0;i<n;i++)
            yp[i] *= s;
    }
}

// returns y = log(sofmax(x))
template <class T>
void log_softmax(const TVec<T> &x, TVec<T> &y)
{
    if (x.length() > 0) {
        y << x;
        y -= max(x);
        y -= logadd(y);
    }
}

//! computes y <- exp(x)
template <class T>
void exp(const TVec<T>& x, TVec<T>& y)
{
    y.resize(x.length());
    int n = x.length();
    if (!n)
        return;
    T* xp = x.data();
    T* yp = y.data();
    while(n--)
        *yp++ = exp(*xp++);
}

//! returns the sum of squared elements
template<class T>
T sumsquare(const TVec<T>& x)
{
    if (x.length() == 0)
        return T(0);
    T* v = x.data();
    T res = square(v[0]);
    int l = x.length();
    for(int i=1; i<l; i++)
        res += square(v[i]);
    return res;
}

//! returns the sum of absolute values of elements
template<class T>
T sumabs(const TVec<T>& x)
{
    if (x.length() == 0)
        return T(0);
    T* v = x.data();
    T res = (T)(fabs((real)v[0]));
    int l = x.length();
    for(int i=1; i<l; i++)
        res += (T)(fabs((real)v[i]));
    return res;
}

//! squares the elements of x in place
template<class T>
void squareElements(const TVec<T>& x)
{
    if (x.length() == 0)
        return;
    T* ptr = x.data();
    int l = x.length();
    while(l--)
    {
        *ptr *= *ptr;
        ++ptr;
    }
}

//! squares the elements of m in place
template<class T>
void squareElements(const TMat<T>& m)
{
    if (m.size()==0)
        return;
    if(m.isCompact()) {
        typename TMat<T>::compact_iterator it = m.compact_begin();
        typename TMat<T>::compact_iterator itend = m.compact_end();
        for(; it != itend; ++it)
            *it = square(*it);
    } else {
        typename TMat<T>::iterator it = m.begin();
        typename TMat<T>::iterator itend = m.end();
        for(; it != itend; ++it)
            *it = square(*it);
    }
}

//! returns the sum of squared elements
template<class T>
T sumsquare(const TMat<T>& m)
{
    if (m.size()==0)
        return T(0);
    if(m.isCompact())
    {
        typename TMat<T>::compact_iterator it = m.compact_begin();
        typename TMat<T>::compact_iterator itend = m.compact_end();
        T res = square(*it);
        ++it;
        for(; it!=itend; ++it)
            res += square(*it);
        return res;
    }
    else
    {
        typename TMat<T>::iterator it = m.begin();
        typename TMat<T>::iterator itend = m.end();
        T res = square(*it);
        ++it;
        for(; it!=itend; ++it)
            res += square(*it);
        return res;
    }
}


//! returns the sum of absolute value of the elements
template<class T>
T sumabs(const TMat<T>& m)
{
    if (m.size()==0)
        return T(0);
    if(m.isCompact())
    {
        typename TMat<T>::compact_iterator it = m.compact_begin();
        typename TMat<T>::compact_iterator itend = m.compact_end();
        T res = fabs(*it);
        ++it;
        for(; it!=itend; ++it)
            res += fabs(*it);
        return res;
    }
    else
    {
        typename TMat<T>::iterator it = m.begin();
        typename TMat<T>::iterator itend = m.end();
        T res = fabs(*it);
        ++it;
        for(; it!=itend; ++it)
            res += fabs(*it);
        return res;
    }
}

// res[i,j] = scale*(mat[i,j] - avg[i] - avg[j] + mean(avg))
template<class T>
void doubleCentering(const TMat<T>& mat, TVec<T>& avg, TMat<T>& res, T scale=T(1))
{
    T moy = mean(avg);
    int n=avg.length();
    if (!n)
        return;
    T* a = avg.data();
    if (scale==T(1))
        for (int i=0;i<n;i++)
        {
            T* Mi = mat[i];
            T* Ri = res[i];
            T term = moy-a[i];
            for (int j=0;j<n;j++)
                Ri[j] = Mi[j] - a[j] + term;
        }
    else
        for (int i=0;i<n;i++)
        {
            T* Mi = mat[i];
            T* Ri = res[i];
            T term = moy-a[i];
            for (int j=0;j<n;j++)
                Ri[j] = scale*(Mi[j] - a[j] + term);
        }
}


//! destination = source1*source2
template <class T>
inline void multiply(const TVec<T>& source1, T source2, TVec<T>& destination)
{
    int n=source1.length();
    if (n!=destination.length())
        destination.resize(n);
    if (!n)
        return;
    T* s1=source1.data();
    T* d=destination.data();
    for (int i=0;i<n;i++)
        d[i] = s1[i]*source2;
}


//------- These were previously in Vec_maths

//! Sum of elements of a vector, which handles missing values.
//! Should only be called with T = double or float.
template<class T>
T sum(const TVec<T>& vec, bool ignore_missing)
{
    double res = 0.0;
    if (vec.size() == 0)
        return res;
    T* v = vec.data();
    for(int i=0; i<vec.length(); i++)
    {
        if (!is_missing(v[i])) res += v[i];
        else if (!ignore_missing) return MISSING_VALUE;
    }
    return T(res);
}

//! Sum of elements of a vector, which assumes all elements are non-missing
//! (will return NAN if T = float or double and there is a missing value).
template<class T>
T sum(const TVec<T>& vec)
{
    T res = T(0);
    if (vec.size() == 0)
        return res;
    T* v = vec.data();
    for(int i=0; i<vec.length(); i++)
        res += v[i];
    return res;
}

//! Returns the sum of the log of the elements
//! (this is also the log of the product of the elements
//! but is more stable if you have very small elements).
template<class T>
T sum_of_log(const TVec<T>& vec)
{
    double res = 0.0;
    if (vec.size() == 0)
        return res;
    T* v = vec.data();
    for(int i=0; i<vec.length(); i++)
        res += pl_log(v[i]);
    return T(res);
}

template<class T>
T product(const TVec<T>& vec)
{
    T res(static_cast<T>(1.0));
    if (vec.size() == 0)
        return res;
    T* v = vec.data();
    for(int i=0; i<vec.length(); i++)
        res *= v[i];
    return res;
}

/*
  template<class T>
  T mean(const TVec<T>& vec)
  {
  #ifdef BOUNDCHECK
  if(vec.length()==0)
  PLERROR("IN T mean(const TVec<T>& vec) vec has zero length");
  #endif
  double res = 0.0;
  T* v = vec.data();
  for(int i=0; i<vec.length(); i++)
  res += v[i];
  return T(res/vec.length());
  }
*/

//! if ignore_missing==true, then the mean is computed by ignoring the
//! possible MISSING_VALUE in the Vec.
//! if ignore_missing==false, then MISSING_VALUE is returned if one
//! element of the Vec is MISSING_VALUE.
template<class T>
T mean(const TVec<T>& vec, bool ignore_missing=false)
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN T mean(const TVec<T>& vec) vec has zero length");
#endif
    if (vec.size() == 0)
        return MISSING_VALUE;
    double res = 0.0;
    int n = 0;
    T* v = vec.data();
    for(int i=0; i<vec.length(); i++)
    {
        if (!is_missing(v[i]))
        {
            res += v[i];
            n++;
        }
        else if (!ignore_missing)
            return MISSING_VALUE;
    }

    if (n == 0)
        return MISSING_VALUE;
    return T(res/double(n));
}

template<class T>
T harmonic_mean(const TVec<T>& vec, bool ignore_missing=false)
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN T mean(const TVec<T>& vec) vec has zero length");
#endif
    if (vec.size() == 0)
        return MISSING_VALUE;
    double res = 0.0;
    int n = 0;
    T* v = vec.data();
    for(int i=0; i<vec.length(); i++)
    {
        if (!is_missing(v[i]))
        {
            res += 1.0/v[i];
            n++;
        }
        else if (!ignore_missing)
            return MISSING_VALUE;
    }

    if (n == 0)
        return MISSING_VALUE;
    return T(double(n)/res);
}

// This one won't really work if you have missing values in the vector
// template<class T>
// T avgdev(const TVec<T>& vec, T meanval)
// {
//   #ifdef BOUNDCHECK
//   if(vec.length()==0)
//     PLERROR("IN T avgdev(const TVec<T>& vec, T meanval) vec has zero length");
//   #endif
//   double res = 0.0;
//   T* v = vec.data();
//   for(int i=0; i<vec.length(); i++)
//       res += fabs(v[i]-meanval);
//   return res/vec.length();
// }

// Does avgdev with/without missing values. ignore_missing=true ignores the missing values
// and computes the avgdev without'em
template<class T>
T avgdev(const TVec<T>& vec, T meanval, bool ignore_missing = false)
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN T avgdev(const TVec<T>& vec, T meanval) vec has zero length");
#endif
    double res = 0.0;
    int n = 0;
    if (vec.size() == 0)
        return MISSING_VALUE;
    T* v = vec.data();
    for(int i=0; i<vec.length(); i++)
        if (!is_missing(v[i]))
        {
            res += fabs(v[i]-meanval);
            n++;
        }
        else if (!ignore_missing)
            return MISSING_VALUE;
    if (n == 0)
        return MISSING_VALUE;
    else
        return T(res/n);
}

template<class T>
T geometric_mean(const TVec<T>& vec)
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN T geometric_mean(const TVec<T>& vec) vec has zero length");
#endif
    if (vec.size() == 0)
        return MISSING_VALUE;
    double res = 0.0;
    T* v = vec.data();
    for(int i=0; i<vec.length(); i++)
    {
        T vi = v[i];
        if (vi<=0)
            PLERROR("geometric_mean(TVec<T>): argument %g <=0 at position [%d]",
                    vi,i);
        res += v[i];
    }
    return T(exp(res/vec.length()));
}

template<class T>
T weighted_mean(const TVec<T>& vec, const TVec<T>& weights, bool ignore_missing=false)
{
#ifdef BOUNDCHECK
    if(vec.length()!=weights.length() || vec.length() == 0)
        PLERROR("IN T weighted_mean(const TVec<T>& vec, const TVec<T>& weights) vec and weights must have equal (non-zero) lengths");
#endif
    if (vec.size() == 0)
        return MISSING_VALUE;
    double res = 0.0;
    T sum_weights = 0.0;
    T* v = vec.data();
    T* w = weights.data();
    for(int i=0; i<vec.length(); i++)
    {
        if (!is_missing(v[i]) && !is_missing(w[i]))
        {
            res += v[i] * w[i];
            sum_weights += w[i];
        }
        else if (!ignore_missing) return MISSING_VALUE;
    }
    if (sum_weights == 0)
        PLERROR("IN T weighted_mean: sum(weights) == 0");
    return T(res/sum_weights);
}

// ignore_missing = true means that it computes the variance ignoring
// the missing values
template<class T>
T variance(const TVec<T>& vec, T meanval, bool ignore_missing=false)
{
#ifdef BOUNDCHECK
    if(vec.length()<=1)
        PLERROR("IN T variance(const TVec<T>& vec, T meanval) vec length must be more than one");
#endif
    if (vec.size() == 0)
        return MISSING_VALUE;
    double res = 0.0;
    T* v = vec.data();
    int n = 0;
    for(int i=0; i<vec.length(); i++)
    {
        if (!is_missing(v[i]))
        {
            double diff = v[i]-meanval;
            res += diff*diff;
            n++;
        }
        else if (!ignore_missing)
            return MISSING_VALUE;
    }
    if (n == 0)
        return MISSING_VALUE;
    else
        return T(res/n);
}

template<class T>
T covariance(const TVec<T>& vec1, const TVec<T>& vec2, T mean1, T mean2)
{
#ifdef BOUNDCHECK
    if(vec1.length()<=1)
        PLERROR("IN T covariance(const TVec<T>& vec1, const TVec<T>& vec2, T mean1, T mean2) vec1's length must be more than one");
    if(vec2.length()<=1)
        PLERROR("IN T covariance(const TVec<T>& vec1, const TVec<T>& vec2, T mean1, T mean2) vec2's length must be more than one");
    if(vec1.length() != vec2.length())
        PLERROR("IN T covariance(const TVec<T>& vec1, const TVec<T>& vec2, T mean1, T mean2) the lengths of vec1 and vec2 must be same");
#endif
    if (vec1.size() == 0 || vec2.size() == 0)
        return MISSING_VALUE;
    int length = vec1.length();
    double res = 0.0;
    T* v1 = vec1.data();
    T* v2 = vec2.data();
    for(int i=0; i<length; i++)
    {
        double temp = (v1[i]-mean1)*(v2[i]-mean2);
        res += temp;
    }
    return res/(length - 1);
}

template<class T>
T weighted_variance(const TVec<T>& vec, const TVec<T>& weights, T no_weighted_mean, T weighted_mean)
{
#ifdef BOUNDCHECK
    if(vec.length()!=weights.length() || vec.length()==0)
        PLERROR("IN T weighted_variance(const TVec<T>& vec, const TVec<T>& weights, T no_weighted_mean, T weighted_mean) vec and weights must have equal (non-zero) lengths");
#endif
    if (vec.size() == 0)
        return MISSING_VALUE;
    double res = 0.0;
    T* v = vec.data();
    T* w = weights.data();
    for(int i=0; i<vec.length(); i++)
        res += v[i] * v[i] * w[i];
    T sum_weights = sum(weights, false);
    if (sum_weights == 0)
        PLERROR("IN T weighted_variance(const TVec<T>& vec, const TVec<T>& weights, T no_weighted_mean, T weighted_mean) sum(weights) == 0");
    return (res/sum_weights - no_weighted_mean * (2*weighted_mean - no_weighted_mean))*vec.length()/(vec.length()-1);
}

template<class T>
TVec<T> histogram(const TVec<T>& vec, T minval, T maxval, int nbins)
{
    TVec<T> histo(nbins);
    T deltaval = maxval-minval + 1e-6;
    for(int i=0; i<vec.length(); i++)
    {
        T val = vec[i];
        int binpos = int((val-minval)/deltaval*nbins);
        if(binpos>=0 && binpos<nbins)
            histo[binpos]++;
    }
    return histo;
}


//! Returns the maximum
template <class T>
T max(const TVec<T>& vec)
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN max(const NumericVec& vec) TVec has zero length()");
#endif
    int n = vec.length();
    if (!n)
        return std::numeric_limits<T>::min();
    T* pv = vec.data();
    T maxval = *pv++;
    while(--n)
    {
        if(*pv>maxval)
            maxval = *pv;
        ++pv;
    }
    return maxval;
}

//! Returns the maximum and computes its index
template <class T>
T max(const TVec<T>& vec, int& argmax)
{
    PLASSERT(vec.length() != 0);

    int n = vec.length();
    if (n == 0)
    {
        argmax = -1;
        return std::numeric_limits<T>::min();
    }
    T* pv = vec.data();
    T maxval = *pv++;
    argmax = 0;
    for (int i=1; i<vec.length(); i++,pv++)
        if (*pv>maxval)
        {
            maxval = *pv;
            argmax = i;
        }
    return maxval;
}

//! Returns the minimum
template<class T>
T min(const TVec<T>& vec)
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN T min(const TVec<T>& vec) vec has zero length");
#endif
    if (vec.size() == 0)
        return std::numeric_limits<T>::max();
    T* v = vec.data();
    T minval = v[0];
    for(int i=1; i<vec.length(); i++)
        if(v[i]<minval)
            minval = v[i];
    return minval;
}

//! Returns the minimum and computes its index
template <class T>
T min(const TVec<T>& vec, int& argmin)
{
    PLASSERT(vec.length() != 0);

    int n = vec.length();
    if (n == 0)
    {
        argmin = -1;
        return std::numeric_limits<T>::max();
    }
    T* pv = vec.data();
    T minval = *pv++;
    argmin = 0;
    for (int i=1; i<vec.length(); i++,pv++)
        if (*pv<minval)
        {
            minval = *pv;
            argmin = i;
        }
    return minval;
}

//! Returns the maximum in absolute value
template<class T>
T maxabs(const TVec<T>& vec)
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN T maxabs(const TVec<T>& vec) vec has zero length");
#endif
    if (vec.size() == 0)
        return std::numeric_limits<T>::min();
    T* v = vec.data();
    T maxval = fabs(v[0]);
    for(int i=1; i<vec.length(); i++)
    {
        T a=fabs(v[i]);
        if(a>maxval)
            maxval = a;
    }
    return maxval;
}

//! Returns the maximum in absolute value and compute its index
template <class T>
T maxabs(const TVec<T>& vec, int& argmax)
{
    PLASSERT(vec.length() != 0);

    int n = vec.length();
    if (n == 0)
    {
        argmax = -1;
        return std::numeric_limits<T>::min();
    }
    T* pv = vec.data();
    T maxval = fabs(*pv++);
    argmax = 0;
    for (int i=1; i<vec.length(); i++,pv++)
    {
        T a = fabs(*pv);
        if (a>maxval)
        {
            maxval = a;
            argmax = i;
        }
    }
    return maxval;
}

//! Returns the minimum in absolute value
template<class T>
T minabs(const TVec<T>& vec)
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN T minabs(const TVec<T>& vec) vec has zero length");
#endif
    int n = vec.length();
    PLASSERT( n >= 1 );
    T* v = vec.data();
    T minval = fabs(v[0]);
    for(int i=1; i<n; i++)
    {
        T a=fabs(v[i]);
        if(a<minval)
            minval = a;
    }

    return minval;
}

//! Returns the minimum in absolute value and compute its index
template <class T>
T minabs(const TVec<T>& vec, int& argmin)
{
    PLASSERT(vec.length() != 0);

    int n = vec.length();
    if (n == 0)
    {
        argmin = -1;
        return std::numeric_limits<T>::max();
    }
    T* pv = vec.data();
    T minval = fabs(*pv++);
    argmin = 0;
    for (int i=1; i<n; i++,pv++)
    {
        T a = fabs(*pv);
        if (a<minval)
        {
            minval = a;
            argmin = i;
        }
    }
    return minval;
}

template<class T>
int argmax(const TVec<T>& vec)
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN int argmax(const TVec<T>& vec) vec has zero length");
#endif
    T* v = vec.data();
    int indexmax = 0;
    T maxval = v[0];
    for(int i=1; i<vec.length(); i++)
        if(v[i]>maxval)
        {
            maxval = v[i];
            indexmax = i;
        }
    return indexmax;
}

template<class T>
int argmax(const TVec<T>& vec, bool ignore_missing)
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN int argmax(const TVec<T>& vec) vec has zero length");
#endif
    T* v = vec.data();
    int indexmax = -1;
    T maxval = MISSING_VALUE;

    for(int i=0; i<vec.length(); i++)
    {
        if( is_missing(v[i]) )
        {
            if(ignore_missing) continue;
            else PLERROR("argmax(const TVec<T>& vec, bool ignore_missing) encountered a MISSING_VALUE\n"
                         "at index %d and ignore_missing is false.", i);
        }

        if( indexmax == -1 ||
            v[i] > maxval   )
        {
            maxval = v[i];
            indexmax = i;
        }
    }
    return indexmax;
}


template<class T>
int argmin(const TVec<T>& vec)
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN int argmin(const TVec<T>& vec) vec has zero length");
#endif
    T* v = vec.data();
    int indexmin = 0;
    T minval = v[0];
    for(int i=1; i<vec.length(); i++)
        if(v[i]<minval)
        {
            minval = v[i];
            indexmin = i;
        }
    return indexmin;
}

template<class T>
int argmin(const TVec<T>& vec, bool ignore_missing)
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN int argmin(const TVec<T>& vec) vec has zero length");
#endif
    T* v = vec.data();
    int indexmin = -1;
    T minval = MISSING_VALUE;

    for(int i=0; i<vec.length(); i++)
    {
        if( is_missing(v[i]) )
        {
            if(ignore_missing) continue;
            else PLERROR("argmin(const TVec<T>& vec, bool ignore_missing) encountered a MISSING_VALUE\n"
                         "at index %d and ignore_missing is false.", i);
        }

        if( indexmin == -1 ||
            v[i] < minval   )
        {
            minval = v[i];
            indexmin = i;
        }
    }
    return indexmin;
}



template<class T>
T pownorm(const TVec<T>& vec, double n)
{
    double result = 0.0;
    if (vec.size() == 0)
        return result;
    T* v = vec.data();
    if(n==1.0)
    {
        for(int i=0; i<vec.length(); i++)
        {
            T val = v[i];
            if(val>=0)
                result += val;
            else
                result -= val;
        }
    }
    else if(n==2.0)
    {
        for(int i=0; i<vec.length(); i++)
        {
            T val = v[i];
            result += val*val;
        }
    }
    else if(n==0)
    { result = vec.length(); }
    else
    {
        for(int i=0; i<vec.length(); i++)
            result += mypow(fabs(v[i]),n);
    }
    return result;
}

template<class T>
inline T pownorm(const TVec<T>& vec) { return pownorm(vec,T(2.0)); }

template<class T>
T norm(const TVec<T>& vec, double n)
{
    if(n==T(1.0))
        return pownorm(vec, T(1.0));
    else if(n==T(2.0))
        return sqrt(pownorm(vec,T(2.0)));
    else
        return mypow(pownorm(vec,n), T(1.0)/n);
}

template<class T>
inline T norm(const TVec<T>& vec) { return norm(vec,T(2.0)); }

template<class T>
void normalize(const TVec<T>& vec, double n=2)
{ vec /= norm(vec,n); }

//! Compute ||vec1 - vec2||_n^n.
//! If 'ignore_missing' is set to true, only components where both 'vec1' and
//! 'vec2' have non missing values will be taken into account. Otherwise,
//! having missing values will result in a 'nan' value being returned.
template<class T>
T powdistance(const TVec<T>& vec1, const TVec<T>& vec2, double n,
              bool ignore_missing = false)
{
#ifdef BOUNDCHECK
    if(vec1.length() != vec2.length())
        PLERROR("In weighted_powdistance: vec1, vec2 should have the same length (%d!=%d)",
                vec1.length(), vec2.length());
#endif
    int length = vec1.length();
    if (length == 0)
        return 0.0;
    T result = 0;
    T diff = 0;
    T* v1 = vec1.data();
    T* v2 = vec2.data();
    if(fast_exact_is_equal(n, 1.0)) // L1 distance
    {
        for(int i=0; i<length; i++, v1++, v2++)
            if (!ignore_missing || (!is_missing(*v1) && !is_missing(*v2))) {
                diff = *v1 - *v2;
                if(diff >= 0)
                    result += diff;
                else
                    result -= diff;
            }
    }
    else if(fast_exact_is_equal(n, 2.0))
    {
        for(int i=0; i<length; i++, v1++, v2++)
            if (!ignore_missing || (!is_missing(*v1) && !is_missing(*v2))) {
                diff = *v1 - *v2;
                result += diff*diff;
            }
    }
    else
    {
        for(int i=0; i<length; i++, v1++, v2++)
            if (!ignore_missing || (!is_missing(*v1) && !is_missing(*v2))) {
                diff = *v1 - *v2;
                if(diff<0)
                    diff = -diff;
                result += mypow(diff,n);
            }
    }
    return result;
}

template<class T>
inline T powdistance(const TVec<T>& vec1, const TVec<T>& vec2)
{ return powdistance(vec1, vec2, 2.0); }

template<class T>
T dist(const TVec<T>& vec1, const TVec<T>& vec2, double n)
{
    if(fast_exact_is_equal(n, T(1.0)))
        return powdistance(vec1, vec2, T(1.0));
    else if(fast_exact_is_equal(n, T(2.0)))
        return sqrt(powdistance(vec1, vec2, T(2.0)));
    else
        return mypow(powdistance(vec1, vec2, n), T(1.0)/n);
}

template<class T>
inline T L2distance(const TVec<T>& vec1, const TVec<T>& vec2)
{ return dist(vec1, vec2, 2.0); }

template<class T>
inline T L1distance(const TVec<T>& vec1, const TVec<T>& vec2)
{ return dist(vec1, vec2, 1.0); }


template<class T>
T weighted_powdistance(const TVec<T>& vec1, const TVec<T>& vec2, double n, const TVec<T>& weights)
{
#ifdef BOUNDCHECK
    if(vec1.length() != weights.length() || vec2.length()!=weights.length())
        PLERROR("In weighted_powdistance: vec1, vec2 and weights vector should have the same length");
#endif
    T result = 0.0;
    if (vec1.size() > 0 && vec2.size() > 0 && weights.size() > 0) {
        T* v1 = vec1.data();
        T* v2 = vec2.data();
        T* w = weights.data();
        int length = vec1.length();
        if(n==1.0) // L1 distance
        {
            for(int i=0; i<length; i++)
            {
                T diff = w[i]*(v1[i]-v2[i]);
                if(diff>=0)
                    result += diff;
                else
                    result -= diff;
            }
        }
        else if(n==2.0)
        {
            for(int i=0; i<length; i++)
            {
                T diff = w[i]*(v1[i]-v2[i]);
                result += diff*diff;
            }
        }
        else
        {
            for(int i=0; i<length; i++)
            {
                T diff = w[i]*(v1[i]-v2[i]);
                if(diff<0)
                    diff = -diff;
                result += mypow(diff,n);
            }
        }
    }
    return result;
}

template<class T>
T weighted_distance(const TVec<T>& vec1, const TVec<T>& vec2, double n, const TVec<T>& weights)
{
    if(n==1.0)
        return weighted_powdistance(vec1, vec2, 1.0, weights);
    else if(n==2.0)
        return sqrt(weighted_powdistance(vec1, vec2, 2.0, weights));
    else
        return mypow(weighted_powdistance(vec1, vec2, n, weights), 1.0/n);
}


//!  element-wise +
template<class T>
inline void operator+=(const TVec<T>& vec1, const TVec<T>& vec2)
{
#ifdef BOUNDCHECK
    if(vec1.size() != vec2.size())
        PLERROR("In operator+=, vec1 and vec2 vectors must have the same length");
#endif
    if (vec1.size() > 0 && vec2.size() > 0) {
        T* v1 = vec1.data();
        T* v2 = vec2.data();
        int l = vec1.length();
        for(int i=0; i<l; i++)
            *v1++ += *v2++;
    }
}

template<class T>
void operator+=(const TVec<T>& vec, T scalar)
{
    if (vec.size() > 0) {
        T* v = vec.data();
        for(int i=0; i<vec.length(); i++)
            v[i] += scalar;
    }
}

template<class T>
TVec<T> operator-(const TVec<T>& vec)
{
    if (vec.size() > 0) {
        TVec<T> opposite(vec.length());
        T *v=vec.data();
        T *o=opposite.data();
        for (int i=0;i<vec.length();i++)
            o[i] = - v[i];
        return opposite;
    }
    return TVec<T>();
}

template<class T>
void operator-=(const TVec<T>& vec1, const TVec<T>& vec2)
{
#ifdef BOUNDCHECK
    if(vec1.size() != vec2.size())
        PLERROR("In operator-=, vec1 and vec2 vectors must have the same length");
#endif
    if (vec1.size() > 0 && vec2.size() > 0) {
        T* v1 = vec1.data();
        T* v2 = vec2.data();
        for(int i=0; i<vec1.length(); i++)
            v1[i] -= v2[i];
    }
}

template<class T>
void operator-=(const TVec<T>& vec, T scalar)
{ vec += -scalar; }

template<class T>
void operator*=(const TVec<T>& vec1, const TVec<T>& vec2)
{
#ifdef BOUNDCHECK
    if(vec1.size() != vec2.size())
        PLERROR("In operator*=, vec1 and vec2 vectors must have the same length");
#endif
    if (vec1.size() > 0 && vec2.size() > 0) {
        T* v1 = vec1.data();
        T* v2 = vec2.data();
        for(int i=0; i<vec1.length(); i++)
            v1[i] *= v2[i];
    }
}

template<class T>
void operator*=(const TVec<T>& vec, T factor)
{
    if (vec.size() > 0) {
        T* p = vec.data();
        int l = vec.length();
        for (int i=0;i<l;i++)
            *p++ *= factor;
    }
}

template<class T>
void operator/=(const TVec<T>& vec1, const TVec<T>& vec2)
{
#ifdef BOUNDCHECK
    if(vec1.size() != vec2.size())
        PLERROR("In operator/=, vec1 and vec2 vectors must have the same length");
#endif
    if (vec1.size() > 0 && vec2.size() > 0) {
        T* v1 = vec1.data();
        T* v2 = vec2.data();
        int l=vec1.length();
        for(int i=0; i<l; i++)
            v1[i] /= v2[i];
    }
}

template<class T>
inline void operator/=(const TVec<T>& vec, T scalar)
{ vec *= T(1.0)/scalar; }

template<class T>
inline void operator/=(const TVec<T>& vec, int scalar)
{ vec /= T(scalar); }

template<class T>
void compute_log(const TVec<T>& src, const TVec<T>& dest)
{
#ifdef BOUNDCHECK
    if(src.length()!=dest.length())
        PLERROR("In log, src and dest vectors must have the same length");
#endif
    if (src.size() > 0 && dest.size() > 0) {
        T* ps = src.data();
        T* pd = dest.data();
        int n = src.length();
        for(int i=0; i<n; i++)
            *pd++ = pl_log(*ps++);
    }
}

template<class T>
inline TVec<T> log(const TVec<T>& src)
{ TVec<T> dest(src.length()); compute_log(src,dest); return dest; }

template<class T>
void compute_sqrt(const TVec<T>& src, const TVec<T>& dest)
{
#ifdef BOUNDCHECK
    if(src.length()!=dest.length())
        PLERROR("In sqrt, src and dest vectors must have the same length");
#endif
    if (src.size() > 0 && dest.size() > 0) {
        T* ps = src.data();
        T* pd = dest.data();
        int n = src.length();
        for(int i=0; i<n; i++)
            *pd++ = sqrt(*ps++);
    }
}

template<class T>
inline TVec<T> sqrt(const TVec<T>& src)
{ TVec<T> dest(src.length()); compute_sqrt(src,dest); return dest; }

template<class T>
void compute_safelog(const TVec<T>& src, const TVec<T>& dest)
{
#ifdef BOUNDCHECK
    if(src.length()!=dest.length())
        PLERROR("In safelog, src and dest vectors must have the same length");
#endif
    if (src.size() > 0 && dest.size() > 0) {
        T* ps = src.data();
        T* pd = dest.data();
        int n = src.length();
        for(int i=0; i<n; i++)
            *pd++ = safelog(*ps++);
    }
}

template<class T>
inline TVec<T> safelog(const TVec<T>& src)
{ TVec<T> dest(src.length()); compute_safelog(src,dest); return dest; }

template<class T>
void compute_tanh(const TVec<T>& src, const TVec<T>& dest)
{
#ifdef BOUNDCHECK
    if(src.length()!=dest.length())
        PLERROR("In tanh, src and dest vectors must have the same length");
#endif
    if (src.size() > 0 && dest.size() > 0) {
        T* ps = src.data();
        T* pd = dest.data();
        int n = src.length();
        for(int i=0; i<n; i++)
            *pd++ = tanh(*ps++);
    }
}

template<class T>
void bprop_tanh(const TVec<T>& tanh_x, const TVec<T>& d_tanh_x, TVec<T>& d_x)
{
#ifdef BOUNDCHECK
    if(tanh_x.length()!=d_tanh_x.length())
        PLERROR("In bprop_tanh, src and dest vectors must have the same length");
#endif
    if (tanh_x.size() > 0 && d_tanh_x.size() > 0 && d_x.size() > 0) {
        int n = tanh_x.length();
        if (n != d_x.length()) d_x.resize(n);
        T* y = tanh_x.data();
        T* dy = d_tanh_x.data();
        T* dx = d_x.data();
        for(int i=0; i<n; i++)
        {
            real yi = *y++;
            *dx++ = *dy++ * (1 - yi*yi);
        }
    }
}

template<class T>
inline TVec<T> tanh(const TVec<T>& src)
{ TVec<T> dest(src.length()); compute_tanh(src,dest); return dest; }


template<class T>
void compute_fasttanh(const TVec<T>& src, const TVec<T>& dest)
{
#ifdef BOUNDCHECK
    if(src.length()!=dest.length())
        PLERROR("In fasttanh, src and dest vectors must have the same length");
#endif
    if (src.size() > 0 && dest.size() > 0) {
        T* ps = src.data();
        T* pd = dest.data();
        int n = src.length();
        for(int i=0; i<n; i++)
            *pd++ = fasttanh(*ps++);
    }
}

template<class T>
inline TVec<T> fasttanh(const TVec<T>& src)
{ TVec<T> dest(src.length()); compute_fasttanh(src,dest); return dest; }

template<class T>
void compute_sigmoid(const TVec<T>& src, const TVec<T>& dest)
{
#ifdef BOUNDCHECK
    if(src.length()!=dest.length())
        PLERROR("In sigmoid, src and dest vectors must have the same length");
#endif
    if (src.size() > 0 && dest.size() > 0) {
        T* ps = src.data();
        T* pd = dest.data();
        int n = src.length();
        for(int i=0; i<n; i++)
            *pd++ = sigmoid(*ps++);
    }
}

template<class T>
void log_sigmoid(const TVec<T>& src, const TVec<T>& dest)
{
#ifdef BOUNDCHECK
    if(src.length()!=dest.length())
        PLERROR("In sigmoid, src and dest vectors must have the same length");
#endif
    if (src.size() > 0 && dest.size() > 0) {
        T* ps = src.data();
        T* pd = dest.data();
        int n = src.length();
        for(int i=0; i<n; i++)
            *pd++ = log_sigmoid(*ps++);
    }
}

template<class T>
inline TVec<T> sigmoid(const TVec<T>& src)
{ TVec<T> dest(src.length()); compute_sigmoid(src,dest); return dest; }


template<class T>
void compute_fastsigmoid(const TVec<T>& src, const TVec<T>& dest)
{
#ifdef BOUNDCHECK
    if(src.length()!=dest.length())
        PLERROR("In fastsigmoid, src and dest vectors must have the same length");
#endif
    if (src.size() > 0 && dest.size() > 0) {
        T* ps = src.data();
        T* pd = dest.data();
        int n = src.length();
        for(int i=0; i<n; i++)
            *pd++ = fastsigmoid(*ps++);
    }
}

template<class T>
inline TVec<T> fastsigmoid(const TVec<T>& src)
{ TVec<T> dest(src.length()); compute_fastsigmoid(src,dest); return dest; }

template<class T>
void compute_inverse_sigmoid(const TVec<T>& src, const TVec<T>& dest)
{
#ifdef BOUNDCHECK
    if(src.length()!=dest.length())
        PLERROR("In inverse_sigmoid, src and dest vectors must have the same length");
#endif
    if (src.size() > 0 && dest.size() > 0) {
        T* ps = src.data();
        T* pd = dest.data();
        int n = src.length();
        for(int i=0; i<n; i++)
            *pd++ = inverse_sigmoid(*ps++);
    }
}

template<class T>
inline TVec<T> inverse_sigmoid(const TVec<T>& src)
{ TVec<T> dest(src.length()); compute_inverse_sigmoid(src,dest); return dest; }


template<class T>
void negateElements(const TVec<T>& vec)
{
    if (vec.size() > 0) {
        T* v = vec.data();
        for(int i=0; i<vec.length(); i++)
            v[i] = -v[i];
    }
}

template<class T>
void invertElements(const TVec<T>& vec)
{
    if (vec.size() > 0) {
        T* v = vec.data();
        for(int i=0; i<vec.length(); i++)
            v[i] = 1.0/v[i];
    }
}

template<class T>
TVec<T> inverted(const TVec<T>& vec)
{
    TVec<T> ret(vec.length());
    if (vec.size() > 0) {
        T* v = vec.data();
        for(int i=0; i<vec.length(); i++)
            ret[i] = 1.0/v[i];
    }
    return ret;
}


template<class T>
T dot(const TVec<T>& vec1, const TVec<T>& vec2)
{
#ifdef BOUNDCHECK
    if(vec1.length()!=vec2.length())
        PLERROR("In T operator*(const TVec<T>& vec1, const TVec<T>& vec2) (dot product) the 2 vecs must have the same length.");
#endif
    T res = 0;
    if (vec1.size() > 0 && vec2.size() > 0) {
        T* v1 = vec1.data();
        T* v2 = vec2.data();
        for(int i=0; i<vec1.length(); i++)
            res += v1[i]*v2[i];
    }
    return res;
}

//! Special dot product that allows TVec's of different types,
//! as long as operator*(T,U) is defined.  The return type V must
//! be specified in all circumstances, e.g. :
//!   TVec<int> v1; TVec<float> v2;
//!   double result = dot<double>(v1,v2);
template<class V, class T, class U>
V dot(const TVec<T>& vec1, const TVec<U>& vec2)
{
#ifdef BOUNDCHECK
    if(vec1.length()!=vec2.length())
        PLERROR("In T operator*(const TVec<T>& vec1, const TVec<T>& vec2) (dot product) the 2 vecs must have the same length.");
#endif
    V res = 0;
    if (vec1.size() > 0 && vec2.size() > 0) {
        T* v1 = vec1.data();
        U* v2 = vec2.data();
        for(int i=0; i<vec1.length(); i++)
            res += v1[i]*v2[i];
    }
    return res;
}

template<class T>
T dot(const TMat<T>& m1, const TMat<T>& m2)
{
#ifdef BOUNDCHECK
    if(m1.size()!=m2.size())
        PLERROR("In T operator*(const TMat<T>& m1, const TVec<T>& vec2) (dot product) the 2 matrices must have the same number of elements.");
#endif

    T res = 0;
    if (m1.size() > 0 && m2.size() > 0) {
        T* v1 = m1.data();
        T* v2 = m2.data();
        if (m1.isCompact() && m2.isCompact())
            for(int i=0; i<m1.size(); i++)
                res += v1[i]*v2[i];
        else
        {
            TMatElementIterator<T> p1 = m1.begin();
            TMatElementIterator<T> p2 = m2.begin();
            for (int i=0; i<m1.size(); i++,++p1,++p2)
                res += *p1 * *p2;
        }
    }
    return res;
}

template<class T>
TVec<T> operator-(const TVec<T>& v1, const TVec<T>& v2)
{
    if (v1.length() != v2.length())
        PLERROR("TVec<T> - TVec<T>: different lengths %d and %d",
                v1.length(), v2.length());
    TVec<T> v(v1.length());
    v << v1;
    v-=v2;
    return v;
}

template<class T>
TVec<T> operator-(T v1, const TVec<T>& v2)
{
    TVec<T> v(v2.length());
    v = -v2;
    v += v1;
    return v;
}

template<class T>
TVec<T> operator-(const TVec<T>& v1, T v2)
{
    TVec<T> v(v1.length());
    substract(v1,v2,v);
    return v;
}

template<class T>
TVec<T> operator+(const TVec<T>& v1, const TVec<T>& v2)
{
    if (v1.length() != v2.length())
        PLERROR("TVec<T> + TVec<T>: different lengths %d and %d",
                v1.length(), v2.length());
    TVec<T> v(v1.length());
    v << v1;
    v+=v2;
    return v;
}

template<class T>
TVec<T> operator+(T v1, const TVec<T>& v2)
{
    TVec<T> v(v2.length());
    add(v2,v1,v);
    return v;
}

template<class T>
TVec<T> operator+(const TVec<T>& v1, T v2)
{
    TVec<T> v(v1.length());
    add(v1,v2,v);
    return v;
}

template<class T>
TVec<T> operator%(const TVec<T>& v1, const TVec<T>& v2)
{
    if (v1.length() != v2.length())
        PLERROR("TVec<T> + TVec<T>: different lengths %d and %d",
                v1.length(), v2.length());
    TVec<T> v(v1.length());
    v << v1;
    v*=v2;
    return v;
}

template<class T>
TVec<T> operator*(T scalar, const TVec<T>& v)
{
    TVec<T> result(v.length());
    multiply(v,scalar,result);
    return result;
}

template<class T>
TVec<T> operator*(const TVec<T>& v1, T v2)
{
    TVec<T> v(v1.length());
    multiply(v1,v2,v);
    return v;
}

template<class T>
TVec<T> operator/(const TVec<T>& v1, const TVec<T>& v2)
{
    if (v1.length() != v2.length())
        PLERROR("TVec<T> + TVec<T>: different lengths %d and %d",
                v1.length(), v2.length());
    TVec<T> v(v1.length());
    v << v1;
    v/=v2;
    return v;
}

template<class T>
TVec<T> operator/(T v1, const TVec<T>& v2)
{
    int n=v2.length();
    TVec<T> v(n);
    if (v2.size() > 0) {
        T* s2=v2.data();
        T* d=v.data();
        for (int i=0;i<n;i++)
            d[i] = v1/s2[i];
    }
    return v;
}

// norman: changed to unharmful declaration (see below old style)
template<class T1, class T2>
TVec<T1> operator/(const TVec<T1>& v1, T2 scalar)
{
    TVec<T1> v(v1.length());
    multiply(v1,T1(1.0)/(T1)scalar,v);
    return v;
}

// norman: harmful declarations
//         Replaced with a better declaration above
//template<class T>
//TVec<T> operator/(const TVec<T>& v1, T scalar)
//{
//  TVec<T> v(v1.length());
//  multiply(v1,T(1.0)/scalar,v);
//  return v;
//}

// norman: This will cause problems if T = int (recursive declaration)
//         Replaced with a better declaration above
//template<class T>
//TVec<T> operator/(const TVec<T>& v1, int scalar)
//{ return v1/T(scalar); }

template<class T>
T logadd(const TVec<T>& vec)
{
    int l = vec.length();
    if(l==0)
        return LOG_INIT;

    T *p_x = vec.data();
    T sum = *p_x++;
    for (int i=1; i<l; i++, p_x++)
        sum = logadd(sum, *p_x);
    return sum;
}

template<class T>
T output_margin(const TVec<T>& class_scores, int correct_class)
{
    T maxother = -FLT_MAX;
    for(int i=0; i<class_scores.length(); i++)
    {
        if(i!=correct_class && class_scores[i]>maxother)
            maxother = class_scores[i];
    }
    return class_scores[correct_class]-maxother;
}

template<class T>
void fill_one_hot(const TVec<T>& vec, int hotpos, T coldvalue, T hotvalue)
{
#ifdef BOUNDCHECK
    if(!vec)
        PLERROR("In fill_one_hot given vec must have the correct size");
    if(hotpos<0 || (vec.length()==1 && hotpos>1) || (vec.length()>1 && hotpos>=vec.length()))
        PLERROR("In fill_one_hot given hotpos out of vec range");
#endif
    if(vec.length()==1)
        vec[0] = (hotpos==0 ?coldvalue :hotvalue);
    else
    {
        vec.fill(coldvalue);
        vec[hotpos] = hotvalue;
    }
}

template<class T>
TVec<T> one_hot(int length, int hotpos, T coldvalue, T hotvalue)
{
    TVec<T> result(length);
    fill_one_hot(result, hotpos, coldvalue, hotvalue);
    return result;
}

template<class T>
TVec<T> square(const TVec<T>& vec)
{
    int n = vec.length();
    TVec<T> result(n);
    square(result,vec);
    return result;
}

template<class T>
void square(TVec<T>& result, const TVec<T>& vec)
{
#ifdef BOUNDCHECK
    if (result.size() != vec.size())
        PLERROR("In square, 'result' and 'vec' must have the same size");
#endif
    int n = vec.length();
    if (n > 0) {
        T* v = vec.data();
        T* r = result.data();
        for(int i=0; i<n; i++)
            r[i] = v[i]*v[i];
    }
}

template<class T>
TVec<T> squareroot(const TVec<T>& vec)
{
    int n = vec.length();
    TVec<T> result(n);
    if (n > 0) {
        T* v = vec.data();
        T* r = result.data();
        for(int i=0; i<n; i++)
        r[i] = sqrt(v[i]);
    }
    return result;
}

//! @ return a new array that contain only the non-missing value
//! @ see remove_missing_inplace for inplace version
template<class T>
TVec<T> remove_missing(const TVec<T>& vec)
{
    int n = vec.length();
    int n_non_missing = 0;
    TVec<T> result(n);
    if (n > 0) {
        T* v = vec.data();
        T* r = result.data();
        for(int i=0; i<n; i++) {
            if (!is_missing(v[i]))
                r[n_non_missing++] = v[i];
        }
        result.resize(n_non_missing);
    }
    return result;
}

//! remove all missing value inplace while keeping the order
template<class T>
void remove_missing_inplace(TVec<T>& v)
{   
    int n_non_missing=0;
    int next_non_missing=1;
    T* d = v.data();
    for(;;)
    {
        while(n_non_missing<v.length()&&!is_missing(d[n_non_missing]))
        {
            n_non_missing++;next_non_missing++;
        }
        if(n_non_missing>=v.length())
            return;
        while(next_non_missing<v.length()&&is_missing(d[next_non_missing]))
            next_non_missing++;
        if(next_non_missing>=v.length())
        {
            v.resize(n_non_missing);
            return;
        }
        else
        {
            pl_swap(d[n_non_missing],d[next_non_missing]);
        }
    }
}

//! Transform a vector of T into a vector of U through a unary function.
//! Note: output type need not be specified in this case
template<class T, class U, class V>
TVec<U> apply(const TVec<T>& vec, U (*func)(V))
{
    TVec<U> destination(vec.length());
    apply(vec,destination,func);
    return destination;
}

//! Transform a vector of T into a vector of U through a unary function
template<class T, class U>
void apply(const TVec<T>& source, TVec<U>& destination, U (*func)(T))
{
    int n=source.length();
    if (n!=destination.length())
        PLERROR("apply: source(%d) and destination(%d) TVec<T>'s must have same length",
                n,destination.length());
    if (n > 0) {
        T* s = source.data();
        U* d = destination.data();
        for(int i=0; i<n; i++)
            d[i]=func(s[i]);
    }
}

//! Transform a vector of T and a vector of U into a vector of V,
//! through a binary function
template<class T, class U, class V>
void apply(const TVec<T>& src1,const TVec<U>& src2, TVec<V>& dest,
           V (*func)(T,U))
{
    int n=src1.length();
    if (n!=dest.length() || n!=src2.length())
        PLERROR("apply: src1, src2 and destination TVec<T>'s must have same length");
    if (n > 0) {
        T* s1 = src1.data();
        U* s2 = src2.data();
        V* d = dest.data();
        for(int i=0; i<n; i++)
            d[i]=func(s1[i],s2[i]);
    }
}


// Efficient mathematical operations (without memory allocation)

// destination[i] = source1[i]*source2[i]
template<class T>
void multiply(const TVec<T>& source1, const TVec<T>& source2, TVec<T>& destination)
{
    int n=source1.length();
    if (n!=source2.length())
        PLERROR("multiply: two sources (l=%d and %d) must have same length",
                n,source2.length());
    if (n!=destination.length())
        destination.resize(n);
    if (n > 0) {
        T* s1=source1.data();
        T* s2=source2.data();
        T* d=destination.data();
        for (int i=0;i<n;i++)
            d[i] = s1[i]*s2[i];
    }
}

// destination[i] = source1[i] + source2[i]*source3
template<class T>
void multiplyAdd(const TVec<T>& source1, const TVec<T>& source2,
                 T source3, TVec<T>& destination)
{
    int n=source1.length();
    if (n!=source2.length())
        PLERROR("multiply: two sources (l=%d and %d) must have same length",
                n,source2.length());
    if (n!=destination.length())
        destination.resize(n);
    if (n > 0) {
        T* s1=source1.data();
        T* s2=source2.data();
        T* d=destination.data();
        for (int i=0;i<n;i++)
            d[i] = s1[i]+s2[i]*source3;
    }
}

// destination[i] = a*destination[i] + b*source[i]
template<class T>
void multiplyScaledAdd(const TVec<T>& source, T a, T b, const TVec<T>& destination)
{
    int n=source.length();
    if (n!=destination.length())
        PLERROR("multiply: source and destination (l=%d and %d) must have same length",
                n,destination.length());
    if (n > 0) {
        T* s=source.data();
        T* d=destination.data();
        for (int i=0;i<n;i++)
            d[i] = a*d[i] + b*s[i];
    }
}

// destination[i,j] = a*destination[i,j] + b*source[i,j]
template<class T>
void multiplyScaledAdd(const TMat<T>& source, T a, T b, const TMat<T>& destination)
{
    int n=source.length();
    int m=source.width();
    if (n!=destination.length() || m!=destination.width())
        PLERROR("multiply: source and destination must have same dimensions");
    if (n > 0) {
        int sm=source.mod();
        int dm=destination.mod();
        T* s=source.data();
        T* d=destination.data();
        for (int i=0;i<n;i++,s+=sm,d+=dm)
            for (int j=0;j<m;j++)
                d[j] = a*d[j] + b*s[j];
    }
}

// destination[i] = source1[i]+source2[i]
template<class T>
void add(const TVec<T>& source1, const TVec<T>& source2, TVec<T>& destination)
{
    int n=source1.length();
    if (n!=source2.length())
        PLERROR("add: two sources (l=%d and %d) must have same length",
                n,source2.length());
    if (n!=destination.length())
        destination.resize(n);
    if (n > 0) {
        T* s1=source1.data();
        T* s2=source2.data();
        T* d=destination.data();
        for (int i=0;i<n;i++)
            d[i] = s1[i]+s2[i];
    }
}

// destination[i] = source1[i]+source2
template<class T>
void add(const TVec<T>& source1, T source2, TVec<T>& destination)
{
    int n=source1.length();
    if (n!=destination.length())
        destination.resize(n);
    if (n > 0) {
        T* s1=source1.data();
        T* d=destination.data();
        for (int i=0;i<n;i++)
            d[i] = s1[i]+source2;
    }
}

template<class T>
inline void substract(const TVec<T>& source1, T source2, TVec<T>& destination)
{ add(source1,-source2,destination); }

// destination[i] = source1[i]-source2[i]
template<class T>
void substract(const TVec<T>& source1, const TVec<T>& source2, TVec<T>& destination)
{
    int n=source1.length();
    if (n!=source2.length())
        PLERROR("substract: two sources (l=%d and %d) must have same length",
                n,source2.length());
    if (n!=destination.length())
        destination.resize(n);
    if (n > 0) {
        T* s1=source1.data();
        T* s2=source2.data();
        T* d=destination.data();
        for (int i=0;i<n;i++)
            d[i] = s1[i]-s2[i];
    }
}

// destination[i] += source1[i]-source2[i]
template<class T>
void substractAcc(const TVec<T>& source1, const TVec<T>& source2, TVec<T>& destination)
{
    int n=source1.length();
    if (n!=source2.length())
        PLERROR("substract: two sources (l=%d and %d) must have same length",
                n,source2.length());
    if (n!=destination.length())
        destination.resize(n);
    if (n > 0) {
        T* s1=source1.data();
        T* s2=source2.data();
        T* d=destination.data();
        for (int i=0;i<n;i++)
            d[i] += s1[i]-s2[i];
    }
}

// destination[i] = source1-source2[i]
template<class T>
void substract(T source1, const TVec<T>& source2, TVec<T>& destination)
{
  int n=source2.length();
  if (n!=destination.length())
    destination.resize(n);
  if (n > 0) {
      T* s2=source2.data();
      T* d=destination.data();
      for (int i=0;i<n;i++)
          d[i] = source1-s2[i];
  }
}

template<class T>
inline void divide(const TVec<T>& source1, T source2, TVec<T>& destination)
{ multiply(source1,1.0/source2,destination); }

// destination[i] = source1[i]/source2[i]
template<class T>
void divide(const TVec<T>& source1, const TVec<T>& source2, TVec<T>& destination)
{
    int n=source1.length();
    if (n!=source2.length())
        PLERROR("divide: two sources (l=%d and %d) must have same length",
                n,source2.length());
    if (n!=destination.length())
        destination.resize(n);
    if (n > 0) {
        T* s1=source1.data();
        T* s2=source2.data();
        T* d=destination.data();
        for (int i=0;i<n;i++)
            d[i] = s1[i]/s2[i];
    }
}

// destination[i] = source1/source2[i]
template<class T>
void divide(T source1, const TVec<T>& source2, TVec<T>& destination)
{
    int n=source2.length();
    if (n!=destination.length())
        destination.resize(n);
    if (n > 0) {
        T* s2=source2.data();
        T* d=destination.data();
        for (int i=0;i<n;i++)
            d[i] = source1/s2[i];
    }
}

// destination[i] = max(source1[i],source2[i])
template<class T>
void max(const TVec<T>& source1, const TVec<T>& source2, TVec<T>& destination)
{
    int n=source1.length();
    if (n!=source2.length())
        PLERROR("max: two sources (l=%d and %d) must have same length",
                n,source2.length());
    if (n!=destination.length())
        destination.resize(n);
    if (n > 0) {
        T* s1=source1.data();
        T* s2=source2.data();
        T* d=destination.data();
        for (int i=0;i<n;i++)
            d[i] = MAX(s1[i],s2[i]);
    }
}

// destination[i] = max(source1[i],source2)
template<class T>
void max(const TVec<T>& source1, T source2, TVec<T>& destination)
{
    int n=source1.length();
    if (n!=destination.length())
        destination.resize(n);
    if (n > 0) {
        T* s1=source1.data();
        T* d=destination.data();
        for (int i=0;i<n;i++)
            d[i] = MAX(s1[i],source2);
    }
}


// destination[i] = min(source1[i],source2[i])
template<class T>
void min(const TVec<T>& source1, const TVec<T>& source2, TVec<T>& destination)
{
    int n=source1.length();
    if (n!=source2.length())
        PLERROR("min: two sources (l=%d and %d) must have same length",
                n,source2.length());
    if (n!=destination.length())
        destination.resize(n);
    if (n > 0) {
        T* s1=source1.data();
        T* s2=source2.data();
        T* d=destination.data();
        for (int i=0;i<n;i++)
            d[i] = MIN(s1[i],s2[i]);
    }
}

// destination[i] = min(source1[i],source2)
template<class T>
void min(const TVec<T>& source1, T source2, TVec<T>& destination)
{
    int n=source1.length();
    if (n!=destination.length())
        destination.resize(n);
    if (n > 0) {
        T* s1=source1.data();
        T* d=destination.data();
        for (int i=0;i<n;i++)
            d[i] = MIN(s1[i],source2);
    }
}


template<class T>
TVec<T> softmax(const TVec<T>& x)
{
    TVec<T> y(x.length());
    softmax(x,y);
    return y;
}

template<class T>
void tanh(const TVec<T>& x, TVec<T>& y)
{
    int n = x.length();
#ifdef BOUNDCHECK
    if (y.length()!=n)
        PLERROR("tanh(TVec<T>,TVec<T>), second argument of length %d, first of length %d, should be =",
                n,y.length());
#endif
    if (n>0)
    {
        T* yp = y.data();
        T* xp = x.data();
        for (int i=0;i<n;i++)
            yp[i] = tanh(xp[i]);
    }
}

template<class T>
TVec<T>
exp(const TVec<T>& vec)
{
    TVec<T> res( vec.length() );
    exp( vec, res );
    return res;
}

// return indices of non-zero elements
template<class T>
TVec<T> nonZeroIndices(TVec<T> v)
{
    int n=v.length();
    if (!n)
        return TVec<T>();
    TVec<T> indices(n);
    int ni=0;
    T* val = v.data();
    T* indx= indices.data();
    for (int i=0;i<n;i++)
        if (val[i]!=0)
            indx[ni++]=i;
    indices.resize(ni);
    return indices;
}

// return indices of non-zero elements
template<class T>
TVec<T> nonZeroIndices(TVec<bool> v)
{
    int n=v.length();
    if (!n)
        return TVec<T>();
    TVec<T> indices(n);
    int ni=0;
    bool* val = v.data();
    T* indx= indices.data();
    for (int i=0;i<n;i++)
        if (val[i])
            indx[ni++]=i;
    indices.resize(ni);
    return indices;
}

// Set the complement indices, i.e. if 0<=i<n is not an element
// of the indices vector it is put in the complement_indices vector.
template<class T>
void complement_indices(TVec<T>& indices, int n,
                        TVec<T>& complement_indices,
                        TVec<T>& buffer)
{
    int ni=indices.length();
    T* ind = indices.data();
    T* cind = complement_indices.data();
    buffer.resize(n);
    buffer.fill(0);
    T* buf=buffer.data();
    for (int i=0;i<ni;i++)
        buf[(int)ind[i]]=1.0;
    for (int i=0,j=0;i<n;i++)
        if (buf[i]==0.0)
            cind[j++]=i;
}

// dest[i] = 1 if src[i]==v, 0 otherwise
template<class T>
void equals(const TVec<T>& src, T v, TVec<T>& dest)
{
    int n=src.length();
#ifdef BOUNDCHECK
    if (n!=dest.length())
        PLERROR("equals(TVec<T>(%d),T,TVec<T>(%d)) args of unequal lengths",
                n,dest.length());
#endif
    if (n > 0) {
        T* s=src.data();
        T* d=dest.data();
        for (int i=0;i<n;i++)
            if (s[i]==v) d[i]=1.0; else d[i]=0.0;
    }
}

// dest[i] = 1 if first[i] > second[i], 0 otherwise
template<class T>
void isLargerThan(const TVec<T>& first, const TVec<T>& second, TVec<T>& dest)
{
    int n=first.length();
    if(n!=second.length() || n!=dest.length())
        PLERROR("isLargerThan(TVec<T>(%d), TVec<T>(%d), TVec<T>(%d)) args of unequal length",
                n, second.length(), dest.length());
    if (n > 0) {
        T* f=first.data();
        T* s=second.data();
        T* d=dest.data();
        for (int i=0; i<n; i++)
            d[i] = f[i] > s[i];
    }
}

// dest[i] = 1 if first[i] >= second[i], 0 otherwise
template<class T>
void isLargerThanOrEqualTo(const TVec<T>& first, const TVec<T>& second, TVec<T>& dest)
{
    int n=first.length();
    if(n!=second.length() || n!=dest.length())
        PLERROR("isLargerThan(TVec<T>(%d), TVec<T>(%d), TVec<T>(%d)) args of unequal length",
                n, second.length(), dest.length());
    if (n > 0) {
        T* f=first.data();
        T* s=second.data();
        T* d=dest.data();
        for (int i=0; i<n; i++)
            d[i] = f[i] >= s[i];
    }
}

// dest[i] = 1 if first[i] < second[i], 0 otherwise
template<class T>
void isSmallerThan(const TVec<T>& first, const TVec<T>& second, TVec<T>& dest)
{
    int n=first.length();
    if(n!=second.length() || n!=dest.length())
        PLERROR("isLargerThan(TVec<T>(%d), TVec<T>(%d), TVec<T>(%d)) args of unequal length",
                n, second.length(), dest.length());
    if (n > 0) {
        T* f=first.data();
        T* s=second.data();
        T* d=dest.data();
        for (int i=0; i<n; i++)
            d[i] = f[i] < s[i];
    }
}

// dest[i] = 1 if first[i] <= second[i], 0 otherwise
template<class T>
void isSmallerThanOrEqualTo(const TVec<T>& first, const TVec<T>& second, TVec<T>& dest)
{
    int n=first.length();
    if(n!=second.length() || n!=dest.length())
        PLERROR("isLargerThan(TVec<T>(%d), TVec<T>(%d), TVec<T>(%d)) args of unequal length",
                n, second.length(), dest.length());
    if (n > 0) {
        T* f=first.data();
        T* s=second.data();
        T* d=dest.data();
        for (int i=0; i<n; i++)
            d[i] = f[i] <= s[i];
    }
}

// dest[i] = if_vec[i] ? then_vec[i] : else_vec[i];
template<class T>
void ifThenElse(const TVec<T>& if_vec, const TVec<T>& then_vec,
                const TVec<T>& else_vec, TVec<T>& dest)
{
    int n=if_vec.length();
    if (n!=then_vec.length() || n!=else_vec.length() || n!=dest.length())
        PLERROR("ifThenElse(TVec<T>(%d), TVec<T>(%d), TVec<T>(%d), TVec<T>(%d)) args of unequal lengths",
                n, then_vec.length(), else_vec.length(), dest.length());
    if (n > 0) {
        T* i_=if_vec.data();
        T* t_=then_vec.data();
        T* e_=else_vec.data();
        T* d_=dest.data();
        for (int i=0;i<n;i++)
            d_[i] = i_[i] ? t_[i] : e_[i];
    }
}

// returns the number of times that src[i] == value
template<class T>
int vec_counts(const TVec<T>& src, T value)
{
    int len = src.length();
    int n = 0;
    if (len > 0) {
        T *p = src.data();
        for (int i=0; i<len; i++, p++)
            if (*p == value)
                n++;
    }
    return n;
}

// returns the position of f in src (-1 if f is not found)
template<class T>
int vec_find(const TVec<T>& src, T f)
{
    int len = src.length();
    if (len > 0) {
        T *p = src.data();
        for (int i=0; i<len; i++, p++)
            if (*p == f)
                return(i);
    }
    return -1;
}


template<class T>
T estimatedCumProb(T x, TVec<T> bins)
{
    const int nbins = bins.length()-1;
    if (nbins<1) PLERROR("estimatedCumProb:: there should be at least two elements in the bins vector");
    // +0.5 because we allocate mass 0.25 at the left and 0.25 at the right of the interval (bins(0),bins(nbins))
    const T one_over_nbins = 1.0/(T)(nbins+0.5);

    int k = binary_search(bins, x);

    if (k == -1)
        return 0.25*one_over_nbins;
    else if (k == nbins-1)
        return 1.0 - 0.25*one_over_nbins;
    else if (bins[k] != bins[k+1])
        return one_over_nbins*(0.25 + k + (x-bins[k])/(bins[k+1]-bins[k]));
    else
        return one_over_nbins*(0.75 + k);
}

// returns the index of the kth ordered element of v
// (dumb algorithm, takes time in k*n )
template<class T>
int positionOfkthOrderedElement(const TVec<T>& vec, int k)
{
#ifdef BOUNDCHECK
    if(k<0 || k>=vec.length())
        PLERROR("In positionOfkthOrderedElement, k out of bounds");
#endif

    T* v = vec.data();

    T minval = -FLT_MAX;
    int pos = -1;
    int l=0;

    while(l<=k)
    {
        int nelements_equal_to_newminval = 0;
        T newminval = FLT_MAX;
        for(int i=0; i<vec.length(); i++)
        {
            if(v[i]>minval)
            {
                if(v[i]<newminval)
                {
                    newminval = v[i];
                    nelements_equal_to_newminval = 1;
                    pos = i;
                }
                else if(v[i]==newminval)
                    nelements_equal_to_newminval++;
            }
        }
        l += nelements_equal_to_newminval;
        minval = newminval;
    }

    return pos;
}

//!  returns the value of the kth ordered element of v
//!  k can take values 0 to vec.length()-1
template<class T>
inline T kthOrderedElement(const TVec<T>& vec, int k)
{ return vec[positionOfkthOrderedElement(vec,k)]; }

//! Return the median value of vector.
template<class T>
inline T median(const TVec<T>& vec)
{
    if (vec.isEmpty())
        PLERROR("In median - Cannot compute median of an empty vector");
    return kthOrderedElement(vec, (vec.length()-1)/2);
}

//-------------- These were previouslty methods of TVec ----------------------------------


//!  find the element at position pos that would result from a sort
//!  and put all elements (not in order!) lower than v[pos] in v[i<pos].
template<class T>
T selectAndOrder(const TVec<T>& vec, int pos)
{
    if (pos<0 || pos>=vec.length()) PLERROR("Bad position (%d)", pos);

    int l=0;
    int h=vec.length()-1;
    T* v = vec.data();

    while (l<h)
    {
        T p = v[(l+h)/2];
        int x = l;
        int y = h;

        do
        {
            while (v[x]<p) x++;
            while (p<v[y]) y--;
            if (x<=y)
            {
                PLearn::swap(v[x],v[y]);
                x++;
                y--;
            }
        } while (x<=y);

        if (pos>=x) l=x;
        else h=x-1;
    }

    return v[l];
}

/*!     returns a vector of length q+1 that contains the q quantiles
  of the sorted vector v and the last value corresponds
  to the last value of the vector vec.
*/
template<class T>
TVec<T> getQuantiles(const TVec<T>& vec, int q)
{
    int l = vec.length();
    T* v = vec.data();
    TVec<T> w(q+1);
    T linvq = T(l)/q;
    for(int i=0;i<q;i++) w[i] = v[int(linvq*i)];
    w[q]=v[l-1];
    return w;
}

//!  returns a vector composed of the values of v that are different
//!  from 0;
template<class T>
TVec<T> nonZero(const TVec<T>& vec)
{
    T *v =vec.data();
    int n=0;
    for(int i=0;i<vec.length(); i++) if (v[i]!=0) n++;
    TVec<T> w(n);
    int j=0;
    for(int i=0;i<vec.length(); i++) if (v[i]!=0) w[j++]=v[i];
    return(w);
}

//!  returns a vector composed of the values of v that are greater
//!  than 0;
template<class T>
TVec<T> positiveValues(const TVec<T>& vec)
{
    T *v =vec.data();
    int n=0;
    for(int i=0;i<vec.length(); i++) if (v[i]>0) n++;
    TVec<T> w(n);
    int j=0;
    for(int i=0;i<vec.length(); i++) if (v[i]>0) w[j++]=v[i];
    return(w);
}

/*!  returns the position of the element in the vector that is closest to value
  If is_sorted_vec is true the procedure assumes the vector's elements
  are sorted in ascending order and uses a dichotomy search.
*/
template<class T>
int positionOfClosestElement(const TVec<T>& vec, const T& value, bool is_sorted_vec=false)
{
    T* v = vec.data();
    if (is_sorted_vec) // dichotomy search
    {
        int pos = binary_search(vec, value);
        if (pos == -1) return 0;
        else if (pos == vec.length()-1) return pos;
        T dist1 = fabs(v[pos]-value);
        T dist2 = fabs(v[pos+1]-value);
        if (dist1 <= dist2) return pos;
        else return pos+1;
    }
    else // linear search
    {
        int pos_of_closest = 0;
        T dist_to_closest = fabs(v[0]-value);
        for(int i=1; i<vec.length(); i++)
        {
            T dist = fabs(v[i]-value);
            if(dist<dist_to_closest)
            {
                pos_of_closest = i;
                dist_to_closest = dist;
            }
        }
        return pos_of_closest;
    }
}


/*!  project the Vec x on the linear subspace ORTHOGONAL to the
  subspace defined by the rows of the orthonormal_subspace matrix,
  which are ASSUMED to be ORTHORNORMAL. The method is
  based on substracting for each row v of the matrix
  the quantity v * x . v.
*/
template <class T>
void projectOnOrthogonalSubspace(const TVec<T>& vec, const TMat<T>& orthonormal_subspace)
{
    for (int i=0;i<orthonormal_subspace.length();i++)
    {
        TVec<T> vi = orthonormal_subspace(i);
        T dp = dot(vec,vi);
        multiplyAcc(vec, vi,-dp);
    }
}


//!  vec[i] += x[i]*scale;
template<class T>
void multiplyAcc(const TVec<T>& vec, const TVec<T>& x, T scale)
{
    int n=x.length();
    if (vec.length()!=n)
        PLERROR("TVec::multiplyAcc this has length_=%d and x has length_=%d", vec.length(),n);
    T* p=vec.data();
    T* xp=x.data();
    for (int i=0;i<n;i++)
        *p++ += scale * *xp++;
}

//!  TVec[i] = (1-alpha)*TVec[i]+x[i]*alpha;
template<class T>
void exponentialMovingAverageUpdate(const TVec<T>& vec, const TVec<T>& x, T alpha)
{
    int n=x.length();
    if (vec.length()!=n)
        PLERROR("TVec::exponentialMovingAverageUpdate length_=%d and x has length_=%d",
                vec.length(),n);
    T* p=vec.data();
    T* xp=x.data();
    T one_minus_alpha = 1-alpha;
    for (int i=0;i<n;i++)
        p[i] = one_minus_alpha*p[i] + alpha*xp[i];
}

//!  TVec[i] = (1-alpha)*TVec[i]+(x[i]-mu[i])^2*alpha;
template<class T>
void exponentialMovingVarianceUpdate(const TVec<T>& vec, const TVec<T>& x, const TVec<T>& mu, T alpha)
{
    int n=x.length();
    if (vec.length()!=n || vec.length()!=mu.length())
        PLERROR("TVec::exponentialVarianceAverageUpdate length_=%d and"
                "x has length_=%d, mu has length() %d",
                vec.length(),n,mu.length());
    T* p=vec.data();
    T* xp=x.data();
    T* mp=mu.data();
    T one_minus_alpha = 1-alpha;
    for (int i=0;i<n;i++)
    {
        T dif = (xp[i]-mp[i]);
        p[i] = one_minus_alpha*p[i] + alpha*dif*dif;
    }
}


//!  TVec[i] = (1-alpha)*TVec[i]+x[i]^2*alpha;
template<class T>
void exponentialMovingSquareUpdate(const TVec<T>& vec, const TVec<T>& x, T alpha)
{
    int n=x.length();
    if (vec.length()!=n)
        PLERROR("TVec::exponentialMovingAverageUpdate length_=%d and x has length_=%d",
                vec.length(),n);
    T* p=vec.data();
    T* xp=x.data();
    T one_minus_alpha = 1-alpha;
    for (int i=0;i<n;i++)
    {
        T xpi = xp[i];
        p[i] = one_minus_alpha*p[i] + alpha*xpi*xpi;
    }
}

//!  vec[i] += x[i]*y[i];
template<class T>
void multiplyAcc(const TVec<T>& vec, const TVec<T>& x, const TVec<T>& y)
{
    int n=x.length();
    if (vec.length()!=n || y.length()!=n)
        PLERROR("TVec::multiplyAcc, this+=x*y: length_=%d, x.length_=%d, y.length_=%d",
                vec.length(),n,y.length());
    T* p=vec.data();
    T* xp=x.data();
    T* yp=y.data();
    for (int i=0;i<n;i++)
        p[i] += xp[i] * yp[i];
}

//!  TVec[i] += x[i]*x[i]*scale;
template<class T>
void squareMultiplyAcc(const TVec<T>& vec, const TVec<T>& x, T scale)
{
    int n=x.length();
    if (vec.length()!=n)
        PLERROR("TVec::squareMultiplyAcc this has length_=%d and x has length_=%d", vec.length(),n);
    T* p=vec.data();
    T* xp=x.data();
    for (int i=0;i<n;i++)
    {
        T xpi = xp[i];
        p[i] += scale * xpi * xpi;
    }
}

//!  TVec[i] += x[i]*x[i];
template<class T>
void squareAcc(const TVec<T>& vec, const TVec<T>& x)
{
    int n=x.length();
    if (vec.length()!=n)
        PLERROR("TVec::squareAcc this has length_=%d and x has length_=%d", vec.length(),n);
    T* p=vec.data();
    T* xp=x.data();
    for (int i=0;i<n;i++)
    {
        T xpi = xp[i];
        p[i] += xpi * xpi;
    }
}

//!  Tvec[i] -= x[i]*x[i];
template<class T>
void squareSubtract(const TVec<T>& vec, const TVec<T>& x)
{
    int n=x.length();
    if (vec.length()!=n)
        PLERROR("TVec::squareDiff this has length_=%d and x has length_=%d", vec.length(),n);
    T* p=vec.data();
    T* xp=x.data();
    for (int i=0;i<n;i++)
    {
        T xpi = xp[i];
        p[i] -= xpi * xpi;
    }
}

//!  TVec[i] += (x[i]-y[i])^2*scale;
template<class T>
void diffSquareMultiplyAcc(const TVec<T>& vec, const TVec<T>& x, const TVec<T>& y, T scale)
{
    int n=x.length();
    if (vec.length()!=n || y.length()!=n)
        PLERROR("TVec::diffSquareMultiplyAcc this.length_=%d, x.length_=%d, y.length_=%d",
                vec.length(),n,y.length());
    T* p=vec.data();
    T* xp=x.data();
    T* yp=y.data();
    for (int i=0;i<n;i++)
    {
        T diff = xp[i]-yp[i];
        p[i] += scale * diff * diff;
    }
}

//!  TVec[i] = TVec[i]*fact1 + (x[i]-y[i])^2*fact2;
template<class T>
void diffSquareMultiplyScaledAcc(const TVec<T>& vec, const TVec<T>& x, const TVec<T>& y, T fact1, T fact2)
{
    int n=x.length();
    if (vec.length()!=n || y.length()!=n)
        PLERROR("TVec::diffSquareMultiplyAcc this.length_=%d, x.length_=%d, y.length_=%d",
                vec.length(),n,y.length());
    T* p=vec.data();
    T* xp=x.data();
    T* yp=y.data();
    for (int i=0;i<n;i++)
    {
        T diff = xp[i]-yp[i];
        p[i] = fact1 * p[i] + fact2 * diff * diff;
    }
}

//!  result[i] = sum_j m[i,j] * v[j]
template <class T>
void product(const TVec<T>& result, const TMat<T>& m, const TVec<T>& v)
{
    int l = m.length();
    int w = m.width();
#ifdef BOUNDCHECK
    if (l!=result.length() || w!=v.length())
        PLERROR("product(TVec, TMat, TVec), incompatible arguments:\n"
                "%d <- %dx%d times %d",
                result.length(), l, w, v.length());
#endif

    if (m.isEmpty() || v.isEmpty() || result.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-vector multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!result.isEmpty())
            result.clear();
        return;
    }

    T *rp = result.data();
    T *vp = v.data();
    for (int i=0;i<l;i++)
    {
        const T* mi = m[i];
        T s = 0;
        for (int j=0;j<w;j++)
            s += mi[j] * vp[j];
        rp[i] = s;
    }
}

//!  result[i] += sum_j m[i,j] * v[j]
template <class T>
void productAcc(const TVec<T>& result, const TMat<T>& m, const TVec<T>& v)
{
    int l = m.length();
    int w = m.width();
#ifdef BOUNDCHECK
    if (l!=result.length() || w!=v.length())
        PLERROR("productAcc(TVec, TMat, TVec), incompatible arguments:\n"
                "%d <- %dx%d times %d",
                result.length(), l, w, v.length());
#endif

    if (m.isEmpty() || v.isEmpty() || result.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-vector multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        return;
    }

    T* rp = result.data();
    T* mp = m.data();
    T* vdata = v.data();
    int deltam = m.mod()-m.width();
    for (int i=0;i<l;i++)
    {
        T *vp = vdata;
        T s = *rp;
        for (int j=0;j<w;j++)
            s += *mp++ * *vp++;
        *rp++ = s;
        mp += deltam;
    }
}

//!  result[i] = alpha * sum_j m[i,j] * v[j] + beta * v[i]
//! (Will use the transpose of m if transpose_m is true)
template <class T>
void productScaleAcc(const TVec<T>& result, const TMat<T>& m, bool transpose_m,
                     const TVec<T>& v, T alpha, T beta)
{
    if (transpose_m)
        transposeProductScaleAcc(result, m, v, alpha, beta);
    else
        productScaleAcc(result, m, v, alpha, beta);
}

//!  result[i] = alpha * sum_j m[i,j] * v[j] + beta * v[i]
template <class T>
void productScaleAcc(const TVec<T>& result, const TMat<T>& m, const TVec<T>& v,
                     T alpha, T beta)
{
    int l = m.length();
    int w = m.width();
#ifdef BOUNDCHECK
    if (l!=result.length() || w!=v.length())
        PLERROR("productScaleAcc(TVec, TMat, TVec), incompatible arguments:\n"
                "%d <- %dx%d times %d",
                result.length(), l, w, v.length());
#endif

    if (m.isEmpty() || v.isEmpty() || result.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-vector multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!result.isEmpty())
            result *= beta;
        return;
    }

    T* rp = result.data();
    T* mp = m.data();
    T* vdata = v.data();
    int deltam = m.mod()-m.width();
    for (int i=0;i<l;i++)
    {
        T *vp = vdata;
        T s = 0;
        for (int j=0;j<w;j++)
            s += *mp++ * *vp++;
        *rp = alpha * s + beta * (*rp);
        ++rp;
        mp += deltam;
    }
}

//! result[i] = sum_j m[j,i] * v[j]
//! Equivalently: rowvec(result) = rowvec(v) . m
//! Equivalently: columnvec(result) = transpose(m).columnvec(v)
template <class T>
void transposeProduct(const TVec<T>& result, const TMat<T>& m, const TVec<T>& v)
{
    Profiler::pl_profile_start("transposeProduct T");
    int l=m.length();
#ifdef BOUNDCHECK
    int w=m.width();
    if (l!=v.length() || w!=result.length())
        PLERROR("transposeProduct(TVec, TMat, TVec), incompatible arguments:\n"
                "%d <- %dx%d' times %d",
                result.length(), l, w, v.length());
#endif

    if (m.isEmpty() || v.isEmpty() || result.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-vector multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!result.isEmpty())
            result.clear();
        Profiler::pl_profile_end("transposeProduct T");
        return;
    }

    T *rp = result.data();
    T *vp = v.data();
    result.clear();
    for (int j=0;j<l;j++)
    {
        const T* mj = m[j];
        T vj = vp[j];
        for (int i=0;i<result.length();i++)
            rp[i] += mj[i] * vj;
    }
    Profiler::pl_profile_end("transposeProduct T");
}

//!  result[i] += sum_j m[j,i] * v[j]
template <class T>
void transposeProductAcc(const TVec<T>& result, const TMat<T>& m,
                         const TVec<T>& v)
{
    int l=m.length();
    int w=m.width();
#ifdef BOUNDCHECK
    if (l!=v.length() || w!=result.length())
        PLERROR("transposeProductAcc(TVec, TMat, TVec), incompatible arguments"
                ":\n"
                "%dx%d' times %d -> %d",
                result.length(), l, w, v.length());
#endif

    if (m.isEmpty() || v.isEmpty() || result.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-vector multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        return;
    }

    T* rdata = result.data();
    T* vp = v.data();
    T* mp = m.data();
    int deltam = m.mod()-m.width();
    for (int j=0;j<l;j++)
    {
        T vj = *vp++;

        /*
          T* rp = rdata;
          for (int i=0;i<w;i++)
          *rp++ += vj * *mp++;
          mp += deltam;
        */

        if(vj!=0)
        {
            if(vj==1)
            {
                T* rp = rdata;
                for (int i=0;i<w;i++)
                    *rp++ += *mp++;
                mp += deltam;
            }
            else
            {
                T* rp = rdata;
                for (int i=0;i<w;i++)
                    *rp++ += vj * *mp++;
                mp += deltam;
            }
        }
        else mp += w + deltam;
    }
}

//!  result[i] = alpha * sum_j m[j,i] * v[j] + beta * result[i]
template <class T>
void transposeProductScaleAcc(const TVec<T>& result, const TMat<T>& m,
                              const TVec<T>& v, T alpha, T beta)
{
    int l=m.length();
    int w=m.width();
#ifdef BOUNDCHECK
    if (l!=v.length() || w!=result.length())
        PLERROR("transposeProductScaleAcc(TVec, TMat, TVec), incompatible"
                " arguments:\n"
                "%d <- %dx%d' times %d",
                result.length(), l, w, v.length());
#endif

    if (m.isEmpty() || v.isEmpty() || result.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-vector multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!result.isEmpty())
            result *= beta;
        return;
    }

    T* rdata = result.data();
    T* vp = v.data();
    T* mp = m.data();
    int deltam = m.mod()-m.width();

    T* rp = rdata;
    // initial scaling
    for (int i=0;i<w;i++)
        *rp++ *= beta;

    for (int j=0;j<l;j++)
    {
        T vj = *vp++;
        rp = rdata;
        for (int i=0;i<w;i++)
            *rp++ += alpha * vj * *mp++;
        mp += deltam;
    }
}

/* Obsolete? Uncomment if needed
//!  result[i] += alpha * sum_j m[j,i] * v[j]
// For compatibility
template <class T>
void transposeProductAcc(const TVec<T>& result, const TMat<T>& m, const TVec<T>& v, T alpha)
{
    transposeProductAcc(result, m, v, alpha, 1.);
}
*/

/* Obsolete? Uncomment if needed
template <class T>
void compressedTransposeProductAcc(const TVec<T>& result, const TMat<T>& m, char* comprbufvec)
{
    cout<<"using kasjdlkadja"<<endl;
    union { double d; char c[8]; } uni;
    int l=m.length(),n, idx=0;
    unsigned char mode;
    cout<<"l="<<l<<endl;
    for(int i=0;i<l;i++)
        cout<<i<<":"<<char(comprbufvec[i])<<endl;
    while(l>0)
    {
        read_compr_mode_and_size_ptr(comprbufvec, mode, n);
        if(mode==0 || mode==1)
        {
            idx+=n;
            cout<<"0x"<<n<<" ";
            l-=n;
            if(mode==1)
            {
                --l;
                result+=m(idx++); // !!!!!!
                cout<<"1 ";
            }
        }
        else if(mode==2)
        {
            while(n--)
            {
                cout<<double(*comprbufvec)<<" "<<endl;
                result+= m(idx++) * double(*comprbufvec++); // !!!!!!

                --l;
            }
        }
        else if(mode==3)
        {
            while(n--)
            {
                memcpy(uni.c,comprbufvec,sizeof(double));
                cout<<double(uni.d)<<" "<<endl;
                comprbufvec+=8;
                result+= m(idx++) * uni.d; // !!!!!!!
                --l;
            }
        }
        else
            PLERROR("BUG IN binread_compressed: mode is only 2 bits, so how can it be other than 0,1,2,3 ?");
    }

    if(l!=0)
        PLERROR("In compressed_dot_product : l is not 0 at exit of function, wrong data?");
}
*/

//! return the matrix with elements (i,j) = sum_k U_{ik} d_k V_{kj}
template<class T>
void diagonalizedFactorsProduct(TMat<T>& result, const TMat<T>& U, const TVec<T> d, const TMat<T> V, bool accumulate=false)
{
#ifdef BOUNDCHECK
    if (result.length()!=U.length() || result.width()!=V.width() || U.width()!=d.length() || V.length()!=d.length())
        PLERROR("diagonalizedFactorsProduct: incompatible arguments: (%dx%d)*(%d)*(%dx%d) --> (%dx%d)",
                U.length(),U.width(),d.length(),V.length(),V.width(),result.length(),result.width());
#endif
    int n1=U.length();
    int n2=U.width();
    int n3=V.width();
    T *r_ij = result.data();
    if (accumulate)
        for (int i=0;i<n1;i++)
        {
            T *u_i = U[i];
            for (int j=0;j<n3;j++,r_ij++)
            {
                T* d_k = d.data();
                T res=0;
                for (int k=0;k<n2;k++,d_k++)
                    res += *d_k * u_i[k] * V(k,j);
                *r_ij += res;
            }
        }
    else
        for (int i=0;i<n1;i++)
        {
            T *u_i = U[i];
            for (int j=0;j<n3;j++,r_ij++)
            {
                T* d_k = d.data();
                T res=0;
                for (int k=0;k<n2;k++,d_k++)
                    res += *d_k * u_i[k] * V(k,j);
                *r_ij = res;
            }
        }
}

//! GIVEN that res(i,j) = sum_k U_{ik} d_k V_{kj}, and given dC/dres, U,d and V, accumulate
//! gradients on dC/dU, dC/dd and dC/dV:
//! dC/dU[i,k] += sum_j dC/dres[i,j] d_k V[k,j]
//! dC/dd[k] += sum_{ij} dC/dres[i,j] U[i,k] V[k,j]
//! dC/dV[k,j] += d_k * sum_i U[i,k] dC/dres[i,j]
template<class T>
void diagonalizedFactorsProductBprop(const TMat<T>& dCdresult, const TMat<T>& U, const TVec<T> d,
                                     const TMat<T> V, TMat<T>& dCdU, TVec<T>& dCdd, TMat<T>& dCdV)
{
#ifdef BOUNDCHECK
    if (dCdU.length()!=U.length() || dCdU.width()!=U.width() || dCdd.length()!=d.length()
        || dCdV.length()!=V.length() || dCdV.width()!=V.width() ||
        U.width()!=d.length() || V.length()!=d.length())
        PLERROR("diagonalizedFactorsProductBprop: incompatible arguments");
#endif
    int n1=U.length();
    int n2=U.width();
    int n3=V.width();
    T *dCdr_ij = dCdresult.data();
    for (int i=0;i<n1;i++)
    {
        T *u_i = U[i];
        T *dCdu_i = dCdU[i];
        for (int j=0;j<n3;j++,dCdr_ij++)
        {
            T dcdr = *dCdr_ij;
            T* d_k = d.data();
            T* dCdd_k = dCdd.data();
            for (int k=0;k<n2;k++,d_k++,dCdd_k++)
            {
                T dk = *d_k;
                T u_ik = u_i[k];
                T v_kj = V(k,j);
                dCdu_i[k] += dcdr * dk * v_kj;
                *dCdd_k += dcdr * u_ik * v_kj;
                dCdV(k,j) += dk * u_ik * dcdr;
            }
        }
    }
}

//! return the matrix with elements (i,j) = sum_k U_{ik} d_k V_{jk}
template<class T>
void diagonalizedFactorsProductTranspose(TMat<T>& result, const TMat<T>& U, const TVec<T> d, const TMat<T> V, bool accumulate=false)
{
#ifdef BOUNDCHECK
    if (result.length()!=U.length() || result.width()!=V.length() || U.width()!=d.length() || V.width()!=d.length())
        PLERROR("diagonalizedFactorsProductTranspose: incompatible arguments: (%dx%d)*(%d)*(%dx%d)' --> (%dx%d)",
                U.length(),U.width(),d.length(),V.length(),V.width(),result.length(),result.width());
#endif
    int n1=U.length();
    int n2=U.width();
    int n3=V.length();
    T *r_ij = result.data();
    for (int i=0;i<n1;i++)
    {
        T *u_i = U[i];
        for (int j=0;j<n3;j++,r_ij++)
        {
            T* d_k = d.data();
            T* v_j = V[j];
            T res=0;
            for (int k=0;k<n2;k++,d_k++)
                res += *d_k * u_i[k] * v_j[k];
            if (accumulate)
                *r_ij += res;
            else
                *r_ij = res;
        }
    }
}

// SINCE res[i,j] = sum_k U[i,k] d[k] V[j,k] ==>
// gradients on dC/dU, dC/dd and dC/dV:
// dC/dU[i,k] = sum_j dC/dres[i,j] d_k V[j,k]
// dC/dd[k] = sum_{ij} dC/dres[i,j] U[i,k] V[j,k]
// dC/dV[j,k] = sum_i dC/dres[i,j] d_k U[i,k]
template<class T>
void diagonalizedFactorsProductTransposeBprop(const TMat<T>& dCdresult, const TMat<T>& U,
                                              const TVec<T> d, const TMat<T> V, TMat<T>& dCdU,
                                              TVec<T>& dCdd, TMat<T>& dCdV)
{
#ifdef BOUNDCHECK
    if (dCdU.length()!=U.length() || dCdU.width()!=U.width() || dCdd.length()!=d.length()
        || dCdV.length()!=V.length() || dCdV.width()!=V.width() ||
        U.width()!=d.length() || V.width()!=d.length())
        PLERROR("diagonalizedFactorsProductTransposeBprop: incompatible arguments");
#endif
    int n1=U.length();
    int n2=U.width();
    int n3=V.length();
    T *dCdr_ij = dCdresult.data();
    for (int i=0;i<n1;i++)
    {
        T *u_i = U[i];
        T *dCdu_i = dCdU[i];
        for (int j=0;j<n3;j++,dCdr_ij++)
        {
            T* d_k = d.data();
            T* dCdd_k = dCdd.data();
            T* v_j = V[j];
            T* dCdv_j = dCdV[j];
            for (int k=0;k<n2;k++,d_k++,dCdd_k++)
            {
                T dcdr = *dCdr_ij;
                T dk = *d_k;
                T v_jk = v_j[k];
                T u_ik = u_i[k];
                dCdu_i[k] += dcdr * dk * v_jk;
                *dCdd_k += dcdr * u_ik * v_jk;
                dCdv_j[k] += dcdr * dk * u_ik;
            }
        }
    }
}

//! return the matrix with elements (i,j) = sum_k U_{ki} d_k V_{kj}
template<class T>
void diagonalizedFactorsTransposeProduct(TMat<T>& result, const TMat<T>& U, const TVec<T> d, const TMat<T> V, bool accumulate=false)
{
#ifdef BOUNDCHECK
    if (result.length()!=U.width() || result.width()!=V.width() || U.length()!=d.length() || V.length()!=d.length())
        PLERROR("diagonalizedFactorsTransposeProduct: incompatible arguments: (%dx%d)'*(%d)*(%dx%d) --> (%dx%d)",
                U.length(),U.width(),d.length(),V.length(),V.width(),result.length(),result.width());
#endif
    int n1=U.width();
    int n2=U.length();
    int n3=V.width();
    if (!accumulate)
        result.clear();
    T* d_k = d.data();
    for (int k=0;k<n2;k++,d_k++)
    {
        T *u_k = U[k];
        T *v_k = V[k];
        T *r_ij = result.data();
        for (int i=0;i<n1;i++)
        {
            T u_ki = u_k[i];
            for (int j=0;j<n3;j++,r_ij++)
                *r_ij += *d_k * u_ki * v_k[j];
        }
    }
}

// SINCE res[i,j] = sum_k U[k,i] d[k] V[k,j] ==>
// gradients on dC/dU, dC/dd and dC/dV:
// dC/dU[k,i] = d_k * sum_j dC/dres[i,j] V[k,j]
// dC/dd[k] = sum_{ij} dC/dres[i,j] U[k,i] V[k,j]
// dC/dV[k,j] = d_k sum_i dC/dres[i,j] U[k,i]
template<class T>
void diagonalizedFactorsTransposeProductBprop(const TMat<T>& dCdresult, const TMat<T>& U,
                                              const TVec<T> d, const TMat<T> V, TMat<T>& dCdU,
                                              TVec<T>& dCdd, TMat<T>& dCdV)
{
#ifdef BOUNDCHECK
    if (dCdU.length()!=U.length() || dCdU.width()!=U.width() || dCdd.length()!=d.length()
        || dCdV.length()!=V.length() || dCdV.width()!=V.width() ||
        U.length()!=d.length() || V.length()!=d.length())
        PLERROR("diagonalizedFactorsTransposeProductBprop: incompatible arguments");
#endif
    int n1=U.width();
    int n2=U.length();
    int n3=V.width();
    T* d_k = d.data();
    T* dCdd_k = dCdd.data();
    for (int k=0;k<n2;k++,d_k++,dCdd_k++)
    {
        T dk = *d_k;
        T *u_k = U[k];
        T *dCdu_k = dCdU[k];
        T *v_k = V[k];
        T *dCdv_k = dCdV[k];
        T *dCdr_ij = dCdresult.data();
        for (int i=0;i<n1;i++)
        {
            T u_ki = u_k[i];
            T& dCdu_ki = dCdu_k[i];
            for (int j=0;j<n3;j++,dCdr_ij++)
            {
                T dcdr = *dCdr_ij;
                T v_kj = v_k[j];
                dCdu_ki +=  dcdr * dk * v_kj;
                *dCdd_k += dcdr * u_ki * v_kj;
                dCdv_k[j] += dcdr * dk * u_ki;
            }
        }
    }
}

//! return the matrix with elements (i,j) = sum_k U_{ki} d_k V_{jk}
template<class T>
void diagonalizedFactorsTransposeProductTranspose(TMat<T>& result, const TMat<T>& U, const TVec<T> d, const TMat<T> V, bool accumulate=false)
{
#ifdef BOUNDCHECK
    if (result.length()!=U.width() || result.width()!=V.length() || U.length()!=d.length() || V.width()!=d.length())
        PLERROR("diagonalizedFactorsTransposeProductTranspose: incompatible arguments: (%dx%d)'*(%d)*(%dx%d)' --> (%dx%d)",
                U.length(),U.width(),d.length(),V.length(),V.width(),result.length(),result.width());
#endif
    int n1=U.width();
    int n2=U.length();
    int n3=V.length();
    if (!accumulate)
        result.clear();
    T* d_k = d.data();
    for (int k=0;k<n2;k++,d_k++)
    {
        T *u_k = U[k];
        T *r_ij = result.data();
        for (int i=0;i<n1;i++)
        {
            T u_ki = u_k[i];
            for (int j=0;j<n3;j++,r_ij++)
                *r_ij += *d_k * u_ki * V(j,k);
        }
    }
}

// SINCE res[i,j] = sum_k U[k,i] d[k] V[j,k] ==>
// gradients on dC/dU, dC/dd and dC/dV:
// dC/dU[k,i] = d_k * sum_j dC/dres[i,j] V[j,k]
// dC/dd[k] = sum_{ij} dC/dres[i,j] U[k,i] V[j,k]
// dC/dV[j,k] = d_k * sum_i dC/dres[i,j] U[k,i]
template<class T>
void diagonalizedFactorsTransposeProductTransposeBprop(const TMat<T>& dCdresult, const TMat<T>& U,
                                                       const TVec<T> d, const TMat<T> V, TMat<T>& dCdU,
                                                       TVec<T>& dCdd, TMat<T>& dCdV)
{
#ifdef BOUNDCHECK
    if (dCdU.length()!=U.length() || dCdU.width()!=U.width() || dCdd.length()!=d.length()
        || dCdV.length()!=V.length() || dCdV.width()!=V.width() ||
        U.length()!=d.length() || V.width()!=d.length())
        PLERROR("diagonalizedFactorsTransposeProductTransposeBprop: incompatible arguments");
#endif
    int n1=U.width();
    int n2=U.length();
    int n3=V.length();
    T* d_k = d.data();
    T* dCdd_k = dCdd.data();
    for (int k=0;k<n2;k++,d_k++,dCdd_k++)
    {
        T dk = *d_k;
        T *u_k = U[k];
        T *dCdu_k = dCdU[k];
        T *dCdr_ij = dCdresult.data();
        for (int i=0;i<n1;i++)
        {
            T u_ki = u_k[i];
            T& dCdu_ki = dCdu_k[i];
            for (int j=0;j<n3;j++,dCdr_ij++)
            {
                T dcdr = *dCdr_ij;
                T v_jk = V(j,k);
                dCdu_ki += dcdr * dk * v_jk;
                *dCdd_k += dcdr * u_ki * v_jk;
                dCdV(j,k) += dcdr * dk * u_ki;
            }
        }
    }
}

// ---------- these were previously methods of TMat ---------------

//! return dot product of i-th row with vector v
template<class T>
T matRowDotVec(const TMat<T>& mat, int i, const TVec<T> v)
{
#ifdef BOUNDCHECK
    if (v.length()!=mat.width())
        PLERROR("dotRow(%d,v), v.length_=%d != matrix width_=%d",
                i,v.length(),mat.width());
#endif
    T s = 0;
    T* rowi = mat.rowdata(i);
    T* v_=v.data();
    int w=mat.width();
    for (int j=0;j<w;j++)
        s += rowi[j] * v_[j];
    return s;
}

//! return dot product of j-th column with vector v
template<class T>
T matColumnDotVec(const TMat<T>& mat, int j, const TVec<T> v)
{
#ifdef BOUNDCHECK
    if (v.length()!=mat.length())
        PLERROR("dotColumn(%d,v), v.length_=%d != matrix length_=%d",
                j,v.length(),mat.length());
#endif
    T s = 0;
    T* colj = mat.data()+j;
    T* v_=v.data();
    int l=mat.length();
    for (int i=0;i<l;i++, colj+=mat.mod())
        s += *colj * v_[i];
    return s;
}

//! return dot products of i-th row of A with i-th row of B in vector v
template<class T>
void matRowsDots(TVec<T> v, const TMat<T>& A, const TMat<T>& B)
{
#ifdef BOUNDCHECK
    if (A.length()!=v.length())
        PLERROR("matRowDotsVec(v,A,B): v.length_=%d != A.length_=%d",
                v.length(),A.length());
    if (A.length()!=B.length())
        PLERROR("matRowDotsVec(v,A,B): A.length_=%d != B.length_=%d",
                A.length(),B.length());
    if (A.width()!=B.width())
        PLERROR("matRowDotsVec(v,A,B): A.width_=%d != B.width_=%d",
                A.width(),B.width());
#endif
    int l=A.length(), w=A.width();
    T* vi = v.data();
    for (int i=0;i<l;i++)
    {
        T s = 0;
        T* Aij = A[i];
        T* Bij = B[i];
        for (int j=0;j<w;j++)
            s += *Aij++ * *Bij++;
        *vi++ = s;
    }
}

//! return dot products of i-th row of A with i-th row of B in vector v
template<class T>
void matRowsDotsAcc(TVec<T> v, const TMat<T>& A, const TMat<T>& B)
{
#ifdef BOUNDCHECK
    if (A.length()!=v.length())
        PLERROR("matRowDotsVec(v,A,B): v.length_=%d != A.length_=%d",
                v.length(),A.length());
    if (A.length()!=B.length())
        PLERROR("matRowDotsVec(v,A,B): A.length_=%d != B.length_=%d",
                A.length(),B.length());
    if (A.width()!=B.width())
        PLERROR("matRowDotsVec(v,A,B): A.width_=%d != B.width_=%d",
                A.width(),B.width());
#endif
    int l=A.length(), w=A.width();
    T* vi = v.data();
    for (int i=0;i<l;i++)
    {
        T s = 0;
        T* Aij = A[i];
        T* Bij = B[i];
        for (int j=0;j<w;j++)
            s += *Aij++ * *Bij++;
        *vi++ += s;
    }
}

//! Fill the bottom left part of a matrix with its top right part, so that it
//! becomes symmetric.
template<class T>
void fillItSymmetric(const TMat<T>& mat) {
    int m = mat.mod();
    T* mat_data_to_fill;
    T* mat_data_to_copy;
    for (int i = 0; i < mat.length(); i++) {
        mat_data_to_fill = mat[i];
        mat_data_to_copy = &mat[0][i];
        for (int j = 0; j < i; j++) {
            *(mat_data_to_fill++) = *mat_data_to_copy;
            mat_data_to_copy += m;
        }
    }
}

template<class T>
void makeItSymmetric(const TMat<T>& mat, T max_dif)
{
    if (!mat.isSquare())
        PLERROR("at void makeItSymmetric, the matrix is not even square\n");
    T dif;
    T value;
    bool warning_flag = false;
    int w=mat.width();
    for (int i=0; i<mat.length()-1 ; i++)
        for (int j=i+1; j<w; j++)
        {
            dif = std::abs(mat[i][j] - mat[j][i]);
            if (dif > max_dif)
            {
                max_dif = dif;
                warning_flag = true;
            }
            value = (mat[i][j] + mat[j][i])/2;
            mat[i][j] = value; mat[j][i] = value;
        }
    if (warning_flag)
        PLWARNING("At void makeItSymmetric, the maximum difference %f is not affordable\n", max_dif);
}


/* DEPRECATED, use product(TVec, TMat, TVec) instead
// y[i] = sum_j A[i,j] x[j]

template<class T>
void product(const TMat<T>& mat, const TVec<T>& x, TVec<T>& y)
{
#ifdef BOUNDCHECK
    if (mat.length()!=y.length() || mat.width()!=x.length())
        PLERROR("TMat(%d,%d)::product(TVec& x(%d),TVec& y(%d)), incompatible arguments",
                mat.length(),mat.width(),x.length(),y.length());
#endif
    T* x_=x.data();
    T* y_=y.data();
    for (int i=0;i<mat.length();i++)
    {
        T* Ai = mat[i];
        T yi = 0;
        for (int j=0;j<mat.width();j++)
            yi += Ai[j] * x_[j];
        y_[i]=yi;
    }
}
*/

//! mat[i,j] = sum_k m1[i,k] * m2[k,j]
template<class T>
void product(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)
{
    int n=m1.length();
    int m=m1.width();
    int l=m2.width();
#ifdef BOUNDCHECK
    if (n!=mat.length() || m!=m2.length() || l!=mat.width())
        PLERROR("product(TMat, TMat, TMat), incompatible arguments:\n"
                "%dx%d <- %dx%d times %dx%d",
                mat.length(), mat.width(), n, m, m2.length(), l);
#endif

    if (m1.isEmpty() || m2.isEmpty() || mat.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-matrix multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!mat.isEmpty())
            mat.clear();
        return;
    }

    for (int i=0;i<n;i++)
    {
        const T* m1i = m1[i];
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            const T* m2kj = m2.data()+j;
            for (int k=0;k<m;k++,m2kj+=m2.mod())
                s += m1i[k] * (*m2kj);
            mi[j] = s;
        }
    }
}

//! mat[i,j] += sum_k m1[i,k] * m2[k,j]
template<class T>
void productAcc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)
{
    int n=m1.length();
    int m=m1.width();
    int l=m2.width();
#ifdef BOUNDCHECK
    if (n!=mat.length() || m!=m2.length() || l!=mat.width())
        PLERROR("productAcc(TMat, TMat, TMat), incompatible arguments:\n"
                "%dx%d <- %dx%d times %dx%d",
                mat.length(), mat.width(), n, m, m2.length(), l);
#endif

    if (m1.isEmpty() || m2.isEmpty() || mat.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-matrix multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        return;
    }

    for (int i=0;i<n;i++)
    {
        const T* m1i = m1[i];
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            T* m2kj = m2.data()+j;
            for (int k=0;k<m;k++,m2kj+=m2.mod())
                s += m1i[k] * (*m2kj);
            mi[j] += s;
        }
    }
}

//! mat[i,j] = alpha sum_k m1[i,k] * m2[k,j] + beta mat[i,j]
// (Will use the transpose of m1 and/or m2 instead,
// if you set the corresponding flags to true)
template<class T>
void productScaleAcc(const TMat<T>& mat,
                     const TMat<T>& m1, bool transpose_m1,
                     const TMat<T>& m2, bool transpose_m2,
                     T alpha, T beta)
{
    // Boundary checking is done in called functions
    if (transpose_m1)
        if (transpose_m2) // transpose_m1 && transpose_m2
           transposeTransposeProductScaleAcc(mat, m1, m2, alpha, beta);
        else // transpose_m1 && !transpose_m2
            transposeProductScaleAcc(mat, m1, m2, alpha, beta);
    else
        if (transpose_m2) // !transpose_m1 && transpose_m2
            productTransposeScaleAcc(mat, m1, m2, alpha, beta);
        else // !transpose_m1 && !transpose_m2
            productScaleAcc(mat, m1, m2, alpha, beta);
}

//! mat[i,j] = alpha * sum_k m1[i,k] * m2[k,j] + beta * mat[i,j]
template<class T>
void productScaleAcc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2,
                     T alpha, T beta)
{
    int n=m1.length();
    int m=m1.width();
    int l=m2.width();
#ifdef BOUNDCHECK
    if (n!=mat.length() || m!=m2.length() || l!=mat.width())
        PLERROR("productScaleAcc(TMat, TMat, TMat), incompatible arguments:\n"
                "%dx%d <- %dx%d times %dx%d",
                mat.length(), mat.width(), n, m, m2.length(), l);
#endif

    if (m1.isEmpty() || m2.isEmpty() || mat.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-matrix multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!mat.isEmpty())
            mat *= beta;
        return;
    }

    for (int i=0;i<n;i++)
    {
        const T* m1i = m1[i];
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            T* m2kj = m2.data()+j;
            for (int k=0;k<m;k++,m2kj+=m2.mod())
                s += m1i[k] * (*m2kj);
            mi[j] = alpha * s + beta * mi[j];
        }
    }
}

// result[i,j] += sum_k m1[i,k] * m2[k,j]^2
template<class T>
void product2Acc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)
{
#ifdef BOUNDCHECK
    if (m1.width()!=m2.length() || mat.length()!=m1.length() || mat.width()!=m2.width())
        PLERROR("product2Acc(Mat,Mat), incompatible arguments %dx%d= %dx%d times %dx%d",
                mat.length(),mat.width(),m1.length(),m1.width(), m2.length(),m2.width());
#endif
    int n=m1.length();
    int m=m1.width();
    int l=m2.width();
    for (int i=0;i<n;i++)
    {
        const T* m1i = m1[i];
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            T* m2kj = m2.data()+j;
            for (int k=0;k<m;k++,m2kj+=m2.mod())
                s += m1i[k] * (*m2kj) * (*m2kj);
            mi[j] += s;
        }
    }
}

// result[i,j] += sum_k m1[i,k]^2 * m2[k,j]
template<class T>
void squareProductAcc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)
{
#ifdef BOUNDCHECK
    if (m1.width()!=m2.length() || mat.length()!=m1.length() || mat.width()!=m2.width())
        PLERROR("squareProductAcc(Mat,Mat), incompatible arguments %dx%d= %dx%d times %dx%d",
                mat.length(),mat.width(),m1.length(),m1.width(), m2.length(),m2.width());
#endif
    int n=m1.length();
    int m=m1.width();
    int l=m2.width();
    for (int i=0;i<n;i++)
    {
        const T* m1i = m1[i];
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            T* m2kj = m2.data()+j;
            for (int k=0;k<m;k++,m2kj+=m2.mod())
            {
                T m1ik=m1i[k];
                s += m1ik*m1ik * (*m2kj);
            }
            mi[j] += s;
        }
    }
}

// result[i][j] = v1[i] * v2[j]

template<class T>
void externalProduct(const TMat<T>& mat, const TVec<T>& v1, const TVec<T>& v2)
{
#ifdef BOUNDCHECK
    if (v1.length()!=mat.length() || mat.width()!=v2.length())
        PLERROR("externalProduct(Vec,Vec), incompatible arguments %dx%d= %d times %d",
                mat.length(),mat.width(),v1.length(), v2.length());
#endif
    const T* v_1=v1.data();
    const T* v_2=v2.data();
    int w=mat.width();
    for (int i=0;i<mat.length();i++)
    {
        T* mi = mat[i];
        T v1i = v_1[i];
        for (int j=0;j<w;j++)
            mi[j] = v1i * v_2[j];
    }
}

// mat[i][j] += v1[i] * v2[j]
template<class T>
void externalProductAcc(const TMat<T>& mat, const TVec<T>& v1, const TVec<T>& v2)
{
#ifdef BOUNDCHECK
    if (v1.length()!=mat.length() || mat.width()!=v2.length())
        PLERROR("externalProductAcc(Vec,Vec), incompatible arguments %dx%d= %d times %d",
                mat.length(),mat.width(),v1.length(), v2.length());
#endif

    T* v_1=v1.data();
    T* v_2=v2.data();
    T* mp = mat.data();
    int l = mat.length();
    int w = mat.width();

    if(mat.isCompact())
    {
        T* pv1 = v_1;
        for(int i=0; i<l; i++)
        {
            T* pv2 = v_2;
            T val = *pv1++;
            for(int j=0; j<w; j++)
                *mp++ += val * *pv2++;
        }
    }
    else
    {
        cerr << "!";
        for (int i=0;i<l;i++)
        {
            T* mi = mat[i];
            T v1i = v_1[i];
            for (int j=0;j<w;j++)
                mi[j] += v1i * v_2[j];
        }
    }
}

// mat[i][j] += gamma * v1[i] * v2[j]
template<class T>
void externalProductScaleAcc(const TMat<T>& mat, const TVec<T>& v1, const TVec<T>& v2, T gamma)
{
    Profiler::pl_profile_start("externalProductScaleAcc T");

#ifdef BOUNDCHECK
    if (v1.length()!=mat.length() || mat.width()!=v2.length())
        PLERROR("externalProductScaleAcc(Vec,Vec), incompatible arguments %dx%d= %d times %d",
                mat.length(),mat.width(),v1.length(), v2.length());
#endif
    const T* v_1=v1.data();
    const T* v_2=v2.data();
    int w=mat.width();
    for (int i=0;i<mat.length();i++)
    {
        T* mi = mat[i];
        T v1i = v_1[i];
        for (int j=0;j<w;j++)
            mi[j] += gamma * v1i * v_2[j];
    }
    Profiler::pl_profile_end("externalProductScaleAcc T");
}

// mat[i][j] = alpha * mat[i][j] + gamma * v1[i] * v2[j]
template<class T>
void externalProductScaleAcc(const TMat<T>& mat, const TVec<T>& v1, const TVec<T>& v2, T gamma, T alpha)
{
    Profiler::pl_profile_start("externalProductScaleAcc T");

#ifdef BOUNDCHECK
    if (v1.length()!=mat.length() || mat.width()!=v2.length())
        PLERROR("externalProductScaleAcc(Vec,Vec), incompatible arguments %dx%d= %d times %d",
                mat.length(),mat.width(),v1.length(), v2.length());
#endif
    const T* v_1=v1.data();
    const T* v_2=v2.data();
    int w=mat.width();
    for (int i=0;i<mat.length();i++)
    {
        T* mi = mat[i];
        T v1i = v_1[i];
        for (int j=0;j<w;j++)
            mi[j] = alpha*mi[j] + gamma * v1i * v_2[j];
    }
    Profiler::pl_profile_end("externalProductScaleAcc T");
}

// mat[i][j] *= v1[i] * v2[j]
template<class T>
void externalProductMultUpdate(const TMat<T>& mat, const TVec<T>& v1, const TVec<T>& v2)
{
#ifdef BOUNDCHECK
    if (v1.length()!=mat.length() || mat.width()!=v2.length())
        PLERROR("externalProductMultUpdate(mat,v1,v2), incompatible arguments %dx%d= %d times %d",
                mat.length(),mat.width(),v1.length(), v2.length());
#endif
    const T* v_1=v1.data();
    const T* v_2=v2.data();
    const int N = mat.length();
    const int M = mat.width();
    for (int i=0 ; i<N ; ++i) {
        T* mi = mat[i];
        T v1i = v_1[i];
        for (int j=0; j<M ; ++j)
            mi[j] *= v1i * v_2[j];
    }
}


// mat[i][j] /= v1[i] * v2[j]
template<class T>
void externalProductDivUpdate(const TMat<T>& mat, const TVec<T>& v1, const TVec<T>& v2)
{
#ifdef BOUNDCHECK
    if (v1.length()!=mat.length() || mat.width()!=v2.length())
        PLERROR("externalProductDivUpdate(mat,v1,v2), incompatible arguments %dx%d= %d times %d",
                mat.length(),mat.width(),v1.length(), v2.length());
#endif
    const T* v_1=v1.data();
    const T* v_2=v2.data();
    const int N = mat.length();
    const int M = mat.width();
    for (int i=0 ; i<N ; ++i) {
        T* mi = mat[i];
        T v1i = v_1[i];
        for (int j=0; j<M ; ++j)
            mi[j] /= v1i * v_2[j];
    }
}


//! mat[i,j] = sum_k m1[i,k] * m2[j,k]
template<class T>
void productTranspose(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)
{
    int n=m1.length();
    int m=m1.width();
    int l=m2.length();
#ifdef BOUNDCHECK
    if (n!=mat.length() || m!=m2.width() || l!=mat.width())
        PLERROR("productTranspose(TMat, TMat, TMat), incompatible arguments:\n"
                "%dx%d <- %dx%d times %dx%d'",
                mat.length(), mat.width(), n, m, l, m2.width());
#endif

    if (m1.isEmpty() || m2.isEmpty() || mat.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-matrix multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!mat.isEmpty())
            mat.clear();
        return;
    }

    for (int i=0;i<n;i++)
    {
        const T* m1i = m1[i];
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            const T* m2j = m2[j];
            for (int k=0;k<m;k++)
                s += m1i[k] * m2j[k];
            mi[j] = s;
        }
    }
}

// result[i,j] = sum_k m1[i,k]^2 * m2[j,k]
template<class T>
void squareProductTranspose(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)
{
#ifdef BOUNDCHECK
    if (m1.width()!=m2.width() || mat.length()!=m1.length() || mat.width()!=m2.length())
        PLERROR("squareProductTranspose(Mat,Mat), incompatible arguments %dx%d= %dx%d times %dx%d'",
                mat.length(),mat.width(),m1.length(),m1.width(), m2.length(),m2.width());
#endif
    int n=m1.length();
    int m=m1.width();
    int l=m2.length();
    for (int i=0;i<n;i++)
    {
        const T* m1i = m1[i];
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            const T* m2j = m2[j];
            for (int k=0;k<m;k++)
            {
                T m1ik=m1i[k];
                s += m1ik*m1ik * m2j[k];
            }
            mi[j] = s;
        }
    }
}

// result[i,j] = sum_k m1[i,k] * m2[j,k]^2
template<class T>
void product2Transpose(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)
{
#ifdef BOUNDCHECK
    if (m1.width()!=m2.width() || mat.length()!=m1.length() || mat.width()!=m2.length())
        PLERROR("product2Transpose(Mat,Mat), incompatible arguments %dx%d= %dx%d times %dx%d'",
                mat.length(),mat.width(),m1.length(),m1.width(), m2.length(),m2.width());
#endif
    int n=m1.length();
    int m=m1.width();
    int l=m2.length();
    for (int i=0;i<n;i++)
    {
        const T* m1i = m1[i];
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            const T* m2j = m2[j];
            for (int k=0;k<m;k++)
            {
                T m2jk=m2j[k];
                s += m1i[k] * m2jk*m2jk;
            }
            mi[j] = s;
        }
    }
}

//! mat[i,j] += sum_k m1[i,k] * m2[j,k]
template<class T>
void productTransposeAcc(const TMat<T>& mat, const TMat<T>& m1,
                         const TMat<T>& m2)
{
    int n=m1.length();
    int m=m1.width();
    int l=m2.length();
#ifdef BOUNDCHECK
    if (n!=mat.length() || m!=m2.width() || l!=mat.width())
        PLERROR("productTransposeAcc(TMat, TMat, TMat), incompatible arguments"
                ":\n"
                "%dx%d <- %dx%d times %dx%d'",
                mat.length(), mat.width(), n, m, l, m2.width());
#endif

    if (m1.isEmpty() || m2.isEmpty() || mat.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-matrix multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        return;
    }

    for (int i=0;i<n;i++)
    {
        const T* m1i = m1[i];
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            const T* m2j = m2[j];
            for (int k=0;k<m;k++)
                s += m1i[k] * m2j[k];
            mi[j] += s;
        }
    }
}

//! mat[i,j] = alpha * sum_k m1[i,k] * m2[j,k] + beta * mat[i,j]
template<class T>
void productTransposeScaleAcc(const TMat<T>& mat, const TMat<T>& m1,
                              const TMat<T>& m2, T alpha, T beta)
{
    int n=m1.length();
    int m=m1.width();
    int l=m2.length();
#ifdef BOUNDCHECK
    if (n!=mat.length() || m!=m2.width() || l!=mat.width())
        PLERROR("productTransposeScaleAcc(TMat, TMat, TMat), incompatible"
                " arguments:\n"
                "%dx%d <- %dx%d times %dx%d'",
                mat.length(), mat.width(), n, m, l, m2.width());
#endif

    if (m1.isEmpty() || m2.isEmpty() || mat.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-matrix multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!mat.isEmpty())
            mat *= beta;
        return;
    }

    for (int i=0;i<n;i++)
    {
        const T* m1i = m1[i];
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            const T* m2j = m2[j];
            for (int k=0;k<m;k++)
                s += m1i[k] * m2j[k];
            mi[j] = alpha * s + beta * mi[j];
        }
    }
}

// result[i,j] += sum_k m1[i,k] * m2[j,k]^2
template<class T>
void product2TransposeAcc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)
{
#ifdef BOUNDCHECK
    if (m1.width()!=m2.width() || mat.length()!=m1.length() || mat.width()!=m2.length())
        PLERROR("product2TransposeAcc(Mat,Mat), incompatible arguments %dx%d= %dx%d times %dx%d'",
                mat.length(),mat.width(),m1.length(),m1.width(), m2.length(),m2.width());
#endif
    int n=m1.length();
    int m=m1.width();
    int l=m2.length();
    for (int i=0;i<n;i++)
    {
        const T* m1i = m1[i];
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            const T* m2j = m2[j];
            for (int k=0;k<m;k++)
            {
                T m2jk=m2j[k];
                s += m1i[k] * m2jk*m2jk;
            }
            mi[j] += s;
        }
    }
}

// result[i,j] += sum_k m1[i,k]^2 * m2[j,k]
template<class T>
void squareProductTransposeAcc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)
{
#ifdef BOUNDCHECK
    if (m1.width()!=m2.width() || mat.length()!=m1.length() || mat.width()!=m2.length())
        PLERROR("squareProductTransposeAcc(Mat,Mat), incompatible arguments %dx%d= %dx%d times %dx%d'",
                mat.length(),mat.width(),m1.length(),m1.width(), m2.length(),m2.width());
#endif
    int n=m1.length();
    int m=m1.width();
    int l=m2.length();
    for (int i=0;i<n;i++)
    {
        const T* m1i = m1[i];
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            const T* m2j = m2[j];
            for (int k=0;k<m;k++)
            {
                T m1ik=m1i[k];
                s += m1ik*m1ik * m2j[k];
            }
            mi[j] += s;
        }
    }
}

//! mat[i,j] = sum_k m1[k,i] * m2[k,j]
template<class T>
void transposeProduct(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)
{
    int n=m1.width();
    int m=m1.length();
    int l=m2.width();
#ifdef BOUNDCHECK
    if (m!=m2.length() || mat.length()!=n || mat.width()!=l)
        PLERROR("transposeProduct(TMat, TMat, TMat), incompatible arguments:\n"
                "%dx%d <- %dx%d' times %dx%d",
                mat.length(), mat.width(), m, n, m2.length(), l);
#endif

    if (m1.isEmpty() || m2.isEmpty() || mat.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-matrix multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!mat.isEmpty())
            mat.clear();
        return;
    }

    mat.clear();
    for (int i=0;i<n;i++)
    {
        T* m1ki = m1.data()+i;
        T* mi = mat[i];
        for (int k=0;k<m;k++,m1ki+=m1.mod())
        {
            const T* m2k = m2[k];
            T m1_ki = *m1ki;
            for (int j=0;j<l;j++)
                mi[j] += m1_ki * m2k[j];
        }
    }
}

// result[i,j] = sum_k m1[k,i] * m2[k,j]^2
template<class T>
void transposeProduct2(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)
{
    int n=m1.width();
    int m=m1.length();
    int l=m2.width();
#ifdef BOUNDCHECK
    if (m!=m2.length() || mat.length()!=n || mat.width()!=l)
        PLERROR("transposeProduct2(Mat,Mat), incompatible arguments "
                "%dx%d' times %dx%d into %dx%d",
                m1.length(),m1.width(), m2.length(),m2.width(), mat.length(), mat.width());
#endif
    mat.clear();
    for (int i=0;i<n;i++)
    {
        T* m1ki = m1.data()+i;
        T* mi = mat[i];
        for (int k=0;k<m;k++,m1ki+=m1.mod())
        {
            const T* m2k = m2[k];
            T m1_ki = *m1ki;
            for (int j=0;j<l;j++)
            {
                T m2kj=m2k[j];
                mi[j] += m1_ki * m2kj*m2kj;
            }
        }
    }
}

//! mat[i,j] += sum_k m1[k,i] * m2[k,j]
template<class T>
void transposeProductAcc(const TMat<T>& mat, const TMat<T>& m1,
                         const TMat<T>& m2)
{
    int n=m1.width();
    int m=m1.length();
    int l=m2.width();
#ifdef BOUNDCHECK
    if (m!=m2.length() || mat.length()!=n || mat.width()!=l)
        PLERROR("transposeProductAcc(TMat, TMat, TMat), incompatible"
                " arguments:\n"
                "%dx%d <- %dx%d' times %dx%d",
                mat.length(), mat.width(), m, n, m2.length(), l);
#endif

    if (m1.isEmpty() || m2.isEmpty() || mat.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-matrix multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        return;
    }

    for (int i=0;i<n;i++)
    {
        T* m1ki = m1.data()+i;
        T* mi = mat[i];
        for (int k=0;k<m;k++,m1ki+=m1.mod())
        {
            const T* m2k = m2[k];
            T m1_ki = *m1ki;
            for (int j=0;j<l;j++)
                mi[j] += m1_ki * m2k[j];
        }
    }
}

// mat[i,j] = alpha * sum_k m1[k,i] * m2[k,j] + beta * mat[i,j]
template<class T>
void transposeProductScaleAcc(const TMat<T>& mat, const TMat<T>& m1,
                              const TMat<T>& m2, T alpha, T beta)
{
    int n=m1.width();
    int m=m1.length();
    int l=m2.width();
#ifdef BOUNDCHECK
    if (m!=m2.length() || mat.length()!=n || mat.width()!=l)
        PLERROR("transposeProductScaleAcc(TMat, TMat, TMat), incompatible"
                " arguments:\n"
                "%dx%d <- %dx%d' times %dx%d",
                mat.length(), mat.width(), m, n, m2.length(), l);
#endif

    if (m1.isEmpty() || m2.isEmpty() || mat.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-matrix multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!mat.isEmpty())
            mat *= beta;
        return;
    }

    for (int i=0;i<n;i++)
    {
        T* m1ki = m1.data()+i;
        T* mi = mat[i];

        // initial scaling
        for (int j=0;j<l;j++)
            mi[j] *= beta;

        for (int k=0;k<m;k++,m1ki+=m1.mod())
        {
            const T* m2k = m2[k];
            T m1_ki = *m1ki;
            for (int j=0;j<l;j++)
                mi[j] += alpha * m1_ki * m2k[j];
        }
    }
}

// result[i,j] += sum_k m1[k,i] * m2[k,j]^2
template<class T>
void transposeProduct2Acc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)
{
    int n=m1.width();
    int m=m1.length();
    int l=m2.width();
#ifdef BOUNDCHECK
    if (m!=m2.length() || mat.length()!=n || mat.width()!=l)
        PLERROR("transposeProduct2Acc(Mat,Mat), incompatible arguments "
                "%dx%d' times %dx%d into %dx%d",
                m1.length(),m1.width(), m2.length(),m2.width(), mat.length(), mat.width());
#endif
    for (int i=0;i<n;i++)
    {
        T* m1ki = m1.data()+i;
        T* mi = mat[i];
        for (int k=0;k<m;k++,m1ki+=m1.mod())
        {
            const T* m2k = m2[k];
            T m1_ki = *m1ki;
            for (int j=0;j<l;j++)
            {
                T m2kj = m2k[j];
                mi[j] += m1_ki * m2kj * m2kj;
            }
        }
    }
}

//! mat[i,j] = sum_k m1[k,i] * m2[j,k]
template<class T>
void transposeTransposeProduct(const TMat<T>& mat, const TMat<T>& m1,
                               const TMat<T>& m2)
{
    int n=m1.width();
    int m=m1.length();
    int l=m2.length();
#ifdef BOUNDCHECK
    if (n!=mat.length() || m!=m2.width() || l!=mat.width())
        PLERROR("transposeTransposeProduct(TMat, TMat, TMat), incompatible"
                " arguments:\n"
                "%dx%d <- %dx%d' times %dx%d'",
                mat.length(), mat.width(), m, n, l, m2.width());
#endif

    if (m1.isEmpty() || m2.isEmpty() || mat.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-matrix multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!mat.isEmpty())
            mat.clear();
        return;
    }

    for (int i=0;i<n;i++)
    {
        T* m1ki0 = m1.data()+i;
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            const T* m2j = m2[j];
            T* m1ki = m1ki0;
            for (int k=0;k<m;k++,m1ki+=m1.mod())
                s += (*m1ki) * m2j[k];
            mi[j] = s;
        }
    }
}

//! mat[i,j] += sum_k m1[k,i] * m2[j,k]
template<class T>
void transposeTransposeProductAcc(const TMat<T>& mat, const TMat<T>& m1,
                                  const TMat<T>& m2)
{
    int n=m1.width();
    int m=m1.length();
    int l=m2.length();
#ifdef BOUNDCHECK
    if (n!=mat.length() || m!=m2.width() || l!=mat.width())
        PLERROR("transposeTransposeProductAcc(TMat, TMat, TMat), incompatible"
                " arguments:\n"
                "%dx%d <-  %dx%d' times %dx%d'",
                mat.length(), mat.width(), m, n, l, m2.width());
#endif

    if (m1.isEmpty() || m2.isEmpty() || mat.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-matrix multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        return;
    }

    for (int i=0;i<n;i++)
    {
        T* m1ki0 = m1.data()+i;
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            const T* m2j = m2[j];
            T* m1ki = m1ki0;
            for (int k=0;k<m;k++,m1ki+=m1.mod())
                s += (*m1ki) * m2j[k];
            mi[j] += s;
        }
    }
}

//! mat[i,j] = alpha * sum_k m1[k,i] * m2[j,k] + beta * mat[i,j]
template<class T>
void transposeTransposeProductScaleAcc(const TMat<T>& mat, const TMat<T>& m1,
                                       const TMat<T>& m2, T alpha, T beta)
{
    int n=m1.width();
    int m=m1.length();
    int l=m2.length();
#ifdef BOUNDCHECK
    if (n!=mat.length() || m!=m2.width() || l!=mat.width())
        PLERROR("transposeTransposeProductScaleAcc(TMat, TMat, TMat),"
                " incompatible arguments:\n"
                "%dx%d <- %dx%d' times %dx%d'",
                mat.length(), mat.width(), m, n, l, m2.width());
#endif

    if (m1.isEmpty() || m2.isEmpty() || mat.isEmpty())
    {
        // Size zero: no need to bother computing anything.
        // In such a case, the result of the matrix-matrix multiplication, if
        // not empty, is necessarily zero, since R^0 = {0}.
        if (!mat.isEmpty())
            mat *= beta;
        return;
    }

    for (int i=0;i<n;i++)
    {
        T* m1ki0 = m1.data()+i;
        T* mi = mat[i];
        for (int j=0;j<l;j++)
        {
            T s=0;
            const T* m2j = m2[j];
            T* m1ki = m1ki0;
            for (int k=0;k<m;k++,m1ki+=m1.mod())
                s += (*m1ki) * m2j[k];
            mi[j] = alpha * s + beta * mi[j];
        }
    }
}

template<class T>
T trace(const TMat<T>& mat)
{
    if (!mat.isSquare())
        PLERROR( "In trace()\nThe matrix must be square." );
    T tr = mat.firstElement();
    for ( int i = 1; i < mat.length(); i++ )
        tr += mat(i,i);
    return tr;
}

//!  Applies a regularizer : diag(A) += (tolerance * trace(A))
template<class T>
void regularizeMatrix(const TMat<T>& mat, T tolerance)
{
    T reg;
    T* k;
    int shift;
    reg = tolerance * trace(mat);
    k = mat.data();
    shift = mat.mod() + 1;
    for (int i = 0; i < mat.length(); i++) {
        *k += reg;
        k += shift;
    }
}


template<class T>
void makeRowsSumTo1(const TMat<T>& mat)
{
    for (int i = 0; i < mat.length(); i++)
    {
        TVec<T> row_i = mat(i);
        divide(row_i, sum(row_i), row_i);
    }
}

// result[i,j] = x[i,j]*scale;
template<class T>
void multiply(const TMat<T>& result, const TMat<T>& x, T scale)
{
#ifdef BOUNDCHECK
    if (result.length()!=x.length() || result.width()!=x.width())
        PLERROR("multiply incompatible dimensions: %dx%d <- %dx%d",
                result.length(),result.width(),x.length(),x.width());
#endif
    if(result.isCompact() && x.isCompact())
    {
        typename TMat<T>::compact_iterator itm = result.compact_begin();
        typename TMat<T>::compact_iterator itmend = result.compact_end();
        typename TMat<T>::compact_iterator itx = x.compact_begin();
        for(; itm!=itmend; ++itm, ++itx)
            *itm = *itx * scale;
    }
    else // use non-compact iterators
    {
        typename TMat<T>::iterator itm = result.begin();
        typename TMat<T>::iterator itmend = result.end();
        typename TMat<T>::iterator itx = x.begin();
        for(; itm!=itmend; ++itm, ++itx)
            *itm = *itx * scale;
    }
}

// result[i,j] = x[i,j]*y[i] or x[i,j]*y[j] (transpose case)
template<class T>
void multiply(TMat<T>& result, const TMat<T>& x, const TVec<T>& y, bool transpose=false)
{
    PLASSERT_MSG(transpose && x.width()==y.length() ||
                 !transpose && x.length()==y.length(),
                 "multiply matrix rows or columns by vector: incompatible dimensions");
    result.resize(x.length(),x.width());
    int w=x.width();
    if(result.isCompact() && x.isCompact())
    {
        typename TMat<T>::compact_iterator itm = result.compact_begin();
        typename TMat<T>::compact_iterator itx = x.compact_begin();
        typename TVec<T>::iterator ity = y.begin();
        if (transpose)
            for (int i=0;i<x.length();i++)
            {
                ity = y.begin();
                for (int j=0;j<w;j++,++itx,++itm,++ity)
                    *itm = *itx * *ity;
            }
        else
            for (int i=0;i<x.length();i++,++ity)
                for (int j=0;j<w;j++,++itx,++itm)
                    *itm = *itx * *ity;
    }
    else // use non-compact iterators
    {
        typename TMat<T>::iterator itm = result.begin();
        typename TMat<T>::iterator itx = x.begin();
        typename TVec<T>::iterator ity = y.begin();
        if (transpose)
            for (int i=0;i<x.length();i++)
            {
                ity = y.begin();
                for (int j=0;j<w;j++,++itx,++itm,++ity)
                    *itm = *itx * *ity;
            }
        else
            for (int i=0;i<x.length();i++,++ity)
                for (int j=0;j<w;j++,++itx,++itm)
                    *itm = *itx * *ity;
    }
}

template<class T>
inline TMat<T> operator*(const TMat<T>& m, const T& scalar)
{
    TMat<T> result(m.length(),m.width());
    multiply(result, m, scalar);
    return result;
}

template<class T>
inline TMat<T> operator*(const T& scalar, const TMat<T>& m)
{ return m * scalar;}

// Will not work properly for integers...
template<class T>
inline TMat<T> operator/(const TMat<T>& m, const T& scalar)
{ return m * (T(1)/scalar); }

// result[i,j] += x[i,j]*scale;
template<class T>
void multiplyAcc(const TMat<T>& mat, const TMat<T>& x, T scale)
{
#ifdef BOUNDCHECK
    if (mat.length()!=x.length() || mat.width()!=x.width())
        PLERROR("multiplyAcc incompatible dimensions: %dx%d <- %dx%d",
                mat.length(),mat.width(),x.length(),x.width());
#endif
    if(mat.isCompact() && x.isCompact())
    {
        typename TMat<T>::compact_iterator itm = mat.compact_begin();
        typename TMat<T>::compact_iterator itmend = mat.compact_end();
        typename TMat<T>::compact_iterator itx = x.compact_begin();
        for(; itm!=itmend; ++itm, ++itx)
            *itm += *itx * scale;
    }
    else // use non-compact iterators
    {
        typename TMat<T>::iterator itm = mat.begin();
        typename TMat<T>::iterator itmend = mat.end();
        typename TMat<T>::iterator itx = x.begin();
        for(; itm!=itmend; ++itm, ++itx)
            *itm += *itx * scale;
    }
}

// result[i,j] += x[i,j]*y[i,j];
template<class T>
void multiplyAcc(const TMat<T>& mat, const TMat<T>& x, const TMat<T>& y)
{
    int n=mat.length()*mat.width();
    if (mat.length()!=x.length() || mat.width()!=x.width() || y.length()!=mat.length() || y.width()!=mat.width())
        PLERROR("multiplyAcc this has size=%dx%d, x is %dx%d, y is %dx%d",
                mat.length(),mat.width(),x.length(),x.width(),y.length(),y.width());
    T* p=mat.data();
    T* xp=x.data();
    T* yp=y.data();
    for (int i=0;i<n;i++)
        p[i] += xp[i] * yp[i];
}

// result[i,j] += x[i,j]*x[i,j]*scale;
template<class T>
void squareMultiplyAcc(const TMat<T>& mat, const TMat<T>& x, T scale)
{
    int n=x.length()*x.width();
    if (mat.length()*mat.width()!=n)
        PLERROR("squareMultiplyAcc this has size=%d and x has size=%d",
                mat.width()*mat.length(),n);
    T* p=mat.data();
    T* xp=x.data();
    for (int i=0;i<n;i++)
    {
        T xpi = xp[i];
        p[i] += scale * xpi * xpi;
    }
}

// result[i,j] += (x[i,j]-y[i,j])^2*scale;
template<class T>
void diffSquareMultiplyAcc(const TMat<T>& mat, const TMat<T>& x, const TMat<T>& y, T scale)
{
    int n=x.length()*x.width();
    if (mat.length()*mat.width()!=n)
        PLERROR("diffSquareMultiplyAcc this has size=%d and x has size=%d",
                mat.width()*mat.length(),n);
    T* p=mat.data();
    T* xp=x.data();
    T* yp=y.data();
    for (int i=0;i<n;i++)
    {
        T diff = (xp[i]-yp[i]);
        p[i] += scale * diff * diff;
    }
}

//////////////
// swapRows //
//////////////
//! Swap rows i and j in matrix 'mat'.
//! It is specialized for real numbers in TMat_maths_specialisation.h, in order
//! to use the corresponding BLAS function.
template<class T>
void swapRows(const TMat<T>& mat, int i, int j)
{
    if (i == j)
        return;
    mat.swapRows(i, j);
}

////////////////////
// selectAndOrder //
////////////////////
template<class T>
TVec<T> selectAndOrder(const TMat<T>& mat, int pos, int colnum=0)
{
#ifdef BOUNDCHECK
    if (colnum<0 || colnum>=mat.width()) PLERROR("Bad column number (%d)", colnum);
    if (pos<0 || pos>=mat.length()) PLERROR("Bad position (%d)", pos);
#endif

    int l=0;
    int h=mat.length()-1;
    TMat<T> v = mat.column(colnum);

    while (l<h)
    {
        T p = v((l+h)/2,0);
        int x = l;
        int y = h;

        do
        {
            while (v(x,0)<p) x++;
            while (p<v(y,0)) y--;
            if (x<=y)
            {
                mat.swapRows(x,y);
                x++;
                y--;
            }
        } while (x<=y);

        if (pos>=x) l=x;
        else h=x-1;
    }

    return mat(l);
}


// result[i,i] += lambda
template<class T>
void addToDiagonal(const TMat<T>& mat, T lambda)
{
    T *d = mat.data();
    int l=mat.length();
    for (int i=0;i<l;i++,d+=mat.mod()+1) *d+=lambda;
}



// result[i,i] += lambda[i]

template<class T>
void addToDiagonal(const TMat<T>& mat, const TVec<T>& lambda)
{
#ifdef BOUNDCHECK
    if (lambda.length()!=mat.length())
        PLERROR("Mat(%d)::addToDiagonal(Vec(%d)) inconsistent lengths",
                mat.length(), lambda.length());
#endif
    T *l = lambda.data();
    T *d = mat.data();
    int le= mat.length();
    for (int i=0;i<le;i++,d+=mat.mod()+1,l++) *d += *l;
}

//! Fill diagonal with the specified value
template<class T>
void fillDiagonal(const TMat<T>& mat, T val)
{
    int l=mat.length();
    for (int i=0;i<l;i++)
        mat(i,i) = val;
}

//! Fill diagonal with the specified vector
template<class T>
void fillDiagonal(const TMat<T>& mat, const TVec<T>& v)
{
    int l=mat.length();
    for (int i=0;i<l;i++)
        mat(i,i) = v[i];
}


//! Copy diagonal of mat in d (which must have correct size)
template<class T>
void diag(const TMat<T>& mat, const TVec<T>& d)
{
    T* d_ = d.data();
    int l=mat.length();
    for (int i=0;i<l;i++)
        d_[i] = mat(i,i);
}

template<class T>
TVec<T> diag(const TMat<T>& mat)
{
    TVec<T> d(mat.length());
    diag(mat, d);
    return d;
}

template<class T>
void diagonalOfSquare(const TMat<T>& mat, const TVec<T>& d)
{
    T* d_=d.data();
    for (int i=0;i<mat.length();i++)
        d_[i]=pownorm(mat(i));
}


template<class T>
void projectOnOrthogonalSubspace(const TMat<T>& mat, TMat<T> orthonormal_subspace)
{
    for (int i=0;i<mat.length();i++)
    {
        TVec<T> row_i = mat(i);
        projectOnOrthogonalSubspace(row_i, orthonormal_subspace);
    }
}


template<class T>
void averageAcrossRowsAndColumns(const TMat<T>& mat, TVec<T>& avg_across_rows, TVec<T>& avg_across_columns, bool ignored)
{
    avg_across_rows.resize(mat.width());
    avg_across_columns.resize(mat.length());
    avg_across_rows.clear();
    avg_across_columns.clear();
    T* row_i=mat.data();
    int w=mat.width();
    for (int i=0;i<mat.length();i++)
    {
        T& avg_cols_i=avg_across_columns[i];
        T* avg_rows = avg_across_rows.data();
        for (int j=0;j<w;j++)
        {
            T row_ij=row_i[j];
            avg_cols_i += row_ij;
            avg_rows[j] += row_ij;
        }
        row_i+=mat.mod();
    }
    avg_across_rows /= mat.length();
    avg_across_columns /= mat.width();
}


template<class T>
void addToRows(const TMat<T>& mat, const TVec<T> row, bool ignored)
{
    int l=mat.length();
    for (int i=0;i<l;i++)
    {
        TVec<T> row_i = mat(i);
        row_i += row;
    }
}


template<class T>
void addToColumns(const TMat<T>& mat, const TVec<T> col, bool ignored)
{
    T* row_i=mat.data();
    int w=mat.width();
    for (int i=0;i<mat.length();i++)
    {
        T col_i=col[i];
        for (int j=0;j<w;j++)
            row_i[j] += col_i;
        row_i+=mat.mod();
    }
}

template<class T>
void substractFromRows(const TMat<T>& mat, const TVec<T> row, bool ignored)
{
    for (int i=0;i<mat.length();i++)
    {
        TVec<T> row_i = mat(i);
        row_i -= row;
    }
}



// Probably bugged!!!
template<class T>
void substractFromColumns(const TMat<T>& mat, const TVec<T> col, bool ignored)
{
    T* row_i=mat.data();
    int w=mat.width();
    for (int i=0;i<mat.length();i++)
    {
        T col_i=col[i];
        for (int j=0;j<w;j++)
            row_i[j] -= col_i;
        row_i+=mat.mod();
    }
}


template<class T>
void addToMat(const TMat<T>& mat, T scalar, bool ignored)
{ mat += scalar; }


// -------------- taken and adapted from Mat_maths.cc ------------------

//! Sum of elements of a matrix, which handles missing values.
//! Should only be called with T = double or float.
template<class T>
T sum(const TMat<T>& mat, bool ignore_missing)
{
    double res = 0.0;
    T* m_i = mat.data();
    int w=mat.width();
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
    {
        for(int j=0; j<w; j++)
        {
            if (!is_missing(m_i[j])) res += m_i[j];
            else if (!ignore_missing) return MISSING_VALUE;
        }
    }
    return T(res);
}

//! Sum of elements of a matrix, which assumes all elements are non-missing
//! (will return NAN if T = float or double and there is a missing value).
template<class T>
T sum(const TMat<T>& mat)
{
    T res = T(0);
    T* m_i = mat.data();
    int w=mat.width();

    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<w; j++)
            res += m_i[j];
    return res;
}

template<class T>
T product(const TMat<T>& mat)
{
    double res = 1.0;
    T* m_i = mat.data();
    int w=mat.width();

    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<w; j++)
            res *= m_i[j];
    return T(res);
}

template<class T>
T sum_of_squares(const TMat<T>& mat)
{
    double res = 0.0;
    T* m_i = mat.data();
    int w=mat.width();
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<w; j++)
        {
            T v = m_i[j];
            res += v*v;
        }
    return T(res);
}

template<class T>
T mean(const TMat<T>& mat)
{
#ifdef BOUNDCHECK
    if(mat.length()==0 || mat.width()==0)
        PLERROR("IN T mean(const TMat<T>& mat) mat has 0 size");
#endif
    double res = 0.0;
    T* m_i = mat.data();
    int w=mat.width();
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<w; j++)
            res += m_i[j];
    return T(res/(mat.length()*mat.width()));
}

template<class T>
T geometric_mean(const TMat<T>& mat)
{
#ifdef BOUNDCHECK
    if(mat.length()==0 || mat.width()==0)
        PLERROR("IN T geometric_mean(const TMat<T>& mat) mat has 0 size");
#endif
    double res = 0.0;
    T* m_i = mat.data();
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<mat.width(); j++)
        {
            T mij = m_i[j];
            if (mij<=0)
                PLERROR("geometric_mean(TMat<T>): argument %g <=0 at position (%d,%d)",
                        mij,i,j);
            res += pl_log(m_i[j]);
        }
    return T(exp(res/(mat.length()*mat.width())));
}

template<class T>
T variance(const TMat<T>& mat, T meanval)
{
#ifdef BOUNDCHECK
    if(mat.length()==0 || mat.width()==0)
        PLERROR("IN T variance(const TMat<T>& mat, T meanval) mat has 0 size");
#endif
    double res = 0.0;
    T* m_i = mat.data();
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<mat.width(); j++)
        {
            double diff = m_i[j]-meanval;
            res += diff*diff;
        }
    return res/(mat.length()*mat.width()-1);
}

template<class T>
T correlation(const TMat<T>& mat)
{
    int n = mat.length();
#ifdef BOUNDCHECK
    if(n==0 || mat.width()==0)
        PLERROR("In T correlation(const TMat<T>& mat) mat has 0 size");
#endif
    if (mat.width() != 2)
        PLERROR("In T correlation(const TMat<T>& mat), mat width (%d) must be 2", mat.width());

    double s_x=0, s_y=0, s_xy=0, s_x2=0, s_y2=0;
    for (int i=0; i<n; i++)
    {
        T x = mat(i,0);
        T y = mat(i,1);
        s_x += x;
        s_x2 += x*x;
        s_y += y;
        s_y2 += y*y;
        s_xy += x*y;
    }

    return (n*s_xy - s_x*s_y)/sqrt((n*s_x2 - s_x*s_x)*(n*s_y2 - s_y*s_y));
}

template<class T>
T correlation(const TVec<T>& x, const TVec<T>& y)
{
    int n = x.length();
#ifdef BOUNDCHECK
    if(n==0 || y.length()==0)
        PLERROR("In T correlation(const TVec<T>& x, const TVec<T>& y), one Vec has 0 size");
#endif
    if (n != y.length())
        PLERROR("In T correlation(const TVec<T>& x, const TVec<T>& y), both Vec must have same length (%d != %d)", n, y.length());

    double s_x=0, s_y=0, s_xy=0, s_x2=0, s_y2=0;
    for (int i=0; i<n; i++)
    {
        T x_val = x[i];
        T y_val = y[i];
        s_x += x_val;
        s_x2 += x_val*x_val;
        s_y += y_val;
        s_y2 += y_val*y_val;
        s_xy += x_val*y_val;
    }

    return (n*s_xy - s_x*s_y)/sqrt((n*s_x2 - s_x*s_x)*(n*s_y2 - s_y*s_y));
}

//! Returns the minimum
template<class T>
T min(const TMat<T>& mat)
{
#ifdef BOUNDCHECK
    if(mat.length()==0 || mat.width()==0)
        PLERROR("IN T min(const TMat<T>& mat) mat has 0 size");
#endif
    T* m_i = mat.data();
    double minval = m_i[0];
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<mat.width(); j++)
            if(m_i[j]<minval)
                minval = m_i[j];
    return minval;
}

//! Returns the minimum and computes its position
template<class T>
T min(const TMat<T>& mat, int& min_i, int& min_j)
{
    PLASSERT(mat.size() != 0);

    T* m_i = mat.data();
    double minval = m_i[0];
    min_i = 0;
    min_j = 0;
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<mat.width(); j++)
            if(m_i[j]<minval)
            {
                minval = m_i[j];
                min_i = i;
                min_j = j;
            }
    return minval;
}

//! Returns the maximum
template<class T>
T max(const TMat<T>& mat)
{
#ifdef BOUNDCHECK
    if(mat.length()==0 || mat.width()==0)
        PLERROR("IN T max(const TMat<T>& mat) mat has 0 size");
#endif
    T* m_i = mat.data();
    double maxval = m_i[0];
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<mat.width(); j++)
            if(m_i[j]>maxval)
                maxval = m_i[j];
    return maxval;
}

//! Returns the maximum and computes its position
template<class T>
T max(const TMat<T>& mat, int& max_i, int& max_j)
{
    PLASSERT(mat.size() != 0);

    T* m_i = mat.data();
    double maxval = m_i[0];
    max_i = 0;
    max_j = 0;
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<mat.width(); j++)
            if(m_i[j]>maxval)
            {
                maxval = m_i[j];
                max_i = i;
                max_j = j;
            }
    return maxval;
}

//! Returns the minimum in absolute value
template<class T>
T minabs(const TMat<T>& mat)
{
#ifdef BOUNDCHECK
    if(mat.length()==0 || mat.width()==0)
        PLERROR("IN T minabs(const TMat<T>& mat) mat has 0 size");
#endif
    T* m_i = mat.data();
    double minval = fabs(m_i[0]);
    int w=mat.width();
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<w; j++)
        {
            T a=fabs(m_i[j]);
            if(a<minval)
                minval = a;
        }
    return minval;
}

//! Returns the minimum in absolute value and computes its position
template<class T>
T minabs(const TMat<T>& mat, int& min_i, int& min_j)
{
    PLASSERT(mat.size() != 0);

    T* m_i = mat.data();
    double minval = fabs(m_i[0]);
    min_i = 0;
    min_j = 0;
    int w=mat.width();
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<w; j++)
        {
            T a = fabs(m_i[j]);
            if(a<minval)
            {
                minval = a;
                min_i = i;
                min_j = j;
            }
        }
    return minval;
}

//! Returns the maximum in absolute value
template<class T>
T maxabs(const TMat<T>& mat)
{
#ifdef BOUNDCHECK
    if(mat.length()==0 || mat.width()==0)
        PLERROR("IN T maxabs(const TMat<T>& mat) mat has 0 size");
#endif
    T* m_i = mat.data();
    double maxval = fabs(m_i[0]);
    int w=mat.width();
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<w; j++)
        {
            T a=fabs(m_i[j]);
            if(a>maxval)
                maxval = a;
        }
    return maxval;
}

//! Returns the maximum in absolute value and computes its position
template<class T>
T maxabs(const TMat<T>& mat, int& max_i, int& max_j)
{
    PLASSERT(mat.size() != 0);

    T* m_i = mat.data();
    double maxval = fabs(m_i[0]);
    max_i = 0;
    max_j = 0;
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<mat.width(); j++)
        {
            T a = fabs(m_i[j]);
            if(a>maxval)
            {
                maxval = a;
                max_i = i;
                max_j = j;
            }
        }
    return maxval;
}

//! Stores the position of the min in the 'mini' & 'minj' arg.
template<class T>
void argmin(const TMat<T>& mat, int& mini, int& minj)
{
#ifdef BOUNDCHECK
    if(mat.length()==0 || mat.width()==0)
        PLERROR("IN void argmin(const TMat<T>& mat, int& mini, iny& minj) mat has 0 size");
#endif
    T* m_i = mat.data();
    mini=0;
    minj=0;
    double minval = m_i[0];
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<mat.width(); j++)
            if(m_i[j]<minval)
            {
                minval = m_i[j];
                mini = i;
                minj = j;
            }
}

// Same as above with the max.
template<class T>
void argmax(const TMat<T>& mat, int& maxi, int& maxj)
{
#ifdef BOUNDCHECK
    if(mat.length()==0 || mat.width()==0)
        PLERROR("IN void argmax(const TMat<T>& mat, int& maxi, iny& maxj) mat has 0 size");
#endif
    T* m_i = mat.data();
    maxi=0;
    maxj=0;
    double maxval = m_i[0];
    for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
        for(int j=0; j<mat.width(); j++)
            if(m_i[j]>maxval)
            {
                maxval = m_i[j];
                maxi = i;
                maxj = j;
            }
}

//!  return mini*width+minj
template<class T>
int argmin(const TMat<T>& m)
{
    int imin, jmin;
    argmin(m,imin,jmin);
    return (imin*m.width()+jmin);
}

//!  return maxi*width+maxj
template<class T>
int argmax(const TMat<T>& m)
{
    int imax, jmax;
    argmax(m,imax,jmax);
    return (imax*m.width()+jmax);
}

/*!   all the operations below result in a column vector
  and are obtained by iterating (e.g. summing) over the column index,
  e.g. yielding the sum of each row in the result.
*/

// singlecolumn[i] = sum_j mat(j,i)
template<class T>
void rowSum(const TMat<T>& mat, const TMat<T>& singlecolumn)
{
#ifdef BOUNDCHECK
    if(singlecolumn.length()!=mat.length() || singlecolumn.width() != 1)
        PLERROR("IN void rowSum(const TMat<T>& mat, TMat<T>& singlecolumn) singlecolumn must be a mat.length() x 1 matrix");
#endif
    for(int i=0; i<mat.length(); i++)
        singlecolumn(i,0) = sum(mat(i));
}

// singlecolumn[i] += sum_j mat(j,i)
template<class T>
void rowSumAcc(const TMat<T>& mat, const TMat<T>& singlecolumn)
{
#ifdef BOUNDCHECK
    if(singlecolumn.length()!=mat.length() || singlecolumn.width() != 1)
        PLERROR("IN void rowSum(const TMat<T>& mat, TMat<T>& singlecolumn) singlecolumn must be a mat.length() x 1 matrix");
#endif
    for(int i=0; i<mat.length(); i++)
        singlecolumn(i,0) += sum(mat(i));
}


template<class T>
void rowSum(const TMat<T>& mat, const TVec<T>& colvec)
{
#ifdef BOUNDCHECK
    if(colvec.length()!=mat.length())
        PLERROR("IN void rowSum(const TMat<T>& mat, const TVec<T>& colvec) colvec must have same length as mat");
#endif
    for(int i=0; i<mat.length(); i++)
        colvec[i] = sum(mat(i));
}

template<class T>
void rowMean(const TMat<T>& mat, const TMat<T>& singlecolumn)
{
#ifdef BOUNDCHECK
    if(singlecolumn.length()!=mat.length() || singlecolumn.width()!=1 || mat.width()==0)
        PLERROR("IN void rowMean(const TMat<T>& mat, TMat<T>& singlecolumn) singlecolumn must be a mat.length() x 1 matrix, and mat must have non-zero width");
#endif
    for(int i=0; i<mat.length(); i++)
        singlecolumn(i,0) = mean(mat(i));
}

template<class T>
void rowVariance(const TMat<T>& mat, const TMat<T>& singlecolumn, const TMat<T>& rowmean)
{
#ifdef BOUNDCHECK
    if(singlecolumn.length()!=mat.length() || singlecolumn.width()!=1 || rowmean.length()!=mat.length() || rowmean.width()!=1 || mat.width()==0)
        PLERROR("IN void rowVariance(const TMat<T>& mat, TMat<T>& singlecolumn, const TMat<T>& rowmean) singlecolumn and rowmean must be mat.length() x 1 matrices, mat must have non-zero width");
#endif
    for(int i=0; i<mat.length(); i++)
        singlecolumn(i,0) = variance(mat(i),rowmean(i,0));
}

template<class T>
void rowSumOfSquares(const TMat<T>& mat, const TMat<T>& singlecolumn)
{
#ifdef BOUNDCHECK
    if(singlecolumn.length()!=mat.length() || singlecolumn.width()!=1)
        PLERROR("IN void rowSumOfSquares(const TMat<T>& mat, TMat<T>& singlecolumn) singlecolumn must be a mat.length() x 1 matrix");
#endif
    int w=mat.width();
    for (int i=0;i<mat.length();i++)
    {
        T ss=0;
        T* mi=mat[i];
        for (int j=0;j<w;j++) { T mij=mi[j]; ss+=mij*mij; }
        singlecolumn(i,0)=ss;
    }
}

template<class T>
void rowMax(const TMat<T>& mat, const TMat<T>& singlecolumn)
{
#ifdef BOUNDCHECK
    if(singlecolumn.length()!=mat.length() || singlecolumn.width()!=1 || mat.width()==0)
        PLERROR("IN void rowMax(const TMat<T>& mat, TMat<T>& singlecolumn) singlecolumn must be a mat.length() x 1 matrix, and mat must have non-zero width");
#endif
    for(int i=0; i<mat.length(); i++)
        singlecolumn(i,0) = max(mat(i));
}

template<class T>
void rowMax(const TMat<T>& mat, const TVec<T>& colvec)
{
#ifdef BOUNDCHECK
    if(colvec.length()!=mat.length())
        PLERROR("IN void rowSum(const TMat<T>& mat, const TVec<T>& colvec) colvec must have same length as mat");
#endif
    for(int i=0; i<mat.length(); i++)
        colvec[i] = max(mat(i));
}

template<class T>
void rowMin(const TMat<T>& mat, const TMat<T>& singlecolumn)
{
#ifdef BOUNDCHECK
    if(singlecolumn.length()!=mat.length() || singlecolumn.width()!=1 || mat.width()==0)
        PLERROR("IN void rowMin(const TMat<T>& mat, TMat<T>& singlecolumn) singlecolumn must be a mat.length() x 1 matrix, and mat must have non-zero width");
#endif
    for(int i=0; i<mat.length(); i++)
        singlecolumn(i,0) = min(mat(i));
}


template<class T>
void rowMin(const TMat<T>& mat, const TVec<T>& colvec)
{
#ifdef BOUNDCHECK
    if(colvec.length()!=mat.length())
        PLERROR("IN void rowSum(const TMat<T>& mat, const TVec<T>& colvec) colvec must have same length as mat");
#endif
    for(int i=0; i<mat.length(); i++)
        colvec[i] = min(mat(i));
}

template<class T>
void rowArgmax(const TMat<T>& mat, const TMat<T>& singlecolumn)
{
#ifdef BOUNDCHECK
    if(singlecolumn.length()!=mat.length() || singlecolumn.width()!=1 || mat.width()==0)
        PLERROR("IN void rowMax(const TMat<T>& mat, TMat<T>& singlecolumn) singlecolumn must be a mat.length() x 1 matrix, and mat must have non-zero width");
#endif
    for(int i=0; i<mat.length(); i++)
        singlecolumn(i,0) = argmax(mat(i));
}

template<class T>
void rowArgmin(const TMat<T>& mat, const TMat<T>& singlecolumn)
{
#ifdef BOUNDCHECK
    if(singlecolumn.length()!=mat.length() || singlecolumn.width()!=1 || mat.width()==0)
        PLERROR("IN void rowMax(const TMat<T>& mat, TMat<T>& singlecolumn) singlecolumn must be a mat.length() x 1 matrix, and mat must have non-zero width");
#endif
    for(int i=0; i<mat.length(); i++)
        singlecolumn(i,0) = argmin(mat(i));
}

/*!   all the operations below result in a row vector
  and are obtained by iterating (e.g. summing) over the row index,
  e.g. yielding the sum of each column in the result.
*/

template<class T>
void columnSum(const TMat<T>& mat, TVec<T>& result)
{
#ifdef BOUNDCHECK
    if(result.length()!=mat.width())
        PLERROR("IN void columnSum(const TMat<T>& mat, TVec<T>& result) the length of result must equal the width of mat");
#endif
    int l = mat.length();
    result << mat(0);
    for(int j=1; j<l; j++)
        result += mat(j);
}

template<class T>
void columnSumOfSquares(const TMat<T>& mat, TVec<T>& result)
{
#ifdef BOUNDCHECK
    if(result.length()!=mat.width())
        PLERROR("IN void columnSumOfSquares(const TMat<T>& mat, TVec<T>& result) the length of result must equal the width of mat");
#endif
    for(int j=0; j<mat.width(); j++)
        result[j] = sum_of_squares(mat.column(j));
}

template<class T>
void columnMean(const TMat<T>& mat, TVec<T>& result)
{
#ifdef BOUNDCHECK
    if(result.length()!=mat.width() || mat.length()==0)
        PLERROR("IN void columnMean(const TMat<T>& mat, TVec<T>& result) the length of result must equal the width of mat and mat must have non-zero length");
#endif
    columnSum(mat,result);
    result /= real(mat.length());
}

template<class T>
void columnWeightedMean(const TMat<T>& mat, TVec<T>& result)
{
#ifdef BOUNDCHECK
    if(result.length()!=mat.width()-1 || mat.length()<=1)
        PLERROR("IN void columnWeightedMean(const TMat<T>& mat, TVec<T>& result) the length of result must equal the width - 1 of mat and mat must have at least 1 length");
#endif
    TVec<T> column_j_vec(mat.length()), weights_vec(mat.length());
    TMat<T> column_j_mat(mat.length(), 1), weights_mat(mat.length(), 1);
    for(int j=0; j<mat.width()-1; j++){
        column_j_mat = mat.column(j);
        weights_mat = mat.column(mat.width()-1);
        column_j_vec = column_j_mat.toVecCopy();
        weights_vec = weights_mat.toVecCopy();
        result[j] = weighted_mean(column_j_vec, weights_vec);
    }
}

template<class T>
void columnVariance(const TMat<T>& mat, TVec<T>& result, const TVec<T>& columnmean)
{
#ifdef BOUNDCHECK
    if(result.length()!=mat.width() || columnmean.length()!=mat.width() || mat.length()==0)
        PLERROR("IN void columnVariance(const TMat<T>& mat, TVec<T>& result, const TVec<T>& columnmean) the length of result and columnmean must equal the width of mat and mat must have non-zero length");
#endif
    for(int j=0; j<mat.width(); j++)
        result[j] = variance(mat.column(j),columnmean[j]);
}

template<class T>
void columnWeightedVariance(const TMat<T>& mat, TVec<T>& result, const TVec<T>& column_weighted_mean)
{
#ifdef BOUNDCHECK
    if(result.length()!=mat.width()-1 || column_weighted_mean.length()!=mat.width()-1 || mat.length()<=1)
        PLERROR("IN void columnWeightedVariance(const TMat<T>& mat, TVec<T>& result, const TVec<T>& column_weighted_mean) the length of result and column_weighted_mean must equal the width - 1 of mat and mat must have at least 1 length");
#endif
    T column_no_weighted_mean_j;
    TVec<T> column_j_vec(mat.length()), weights_vec(mat.length());
    TMat<T> column_j_mat(mat.length(), 1), weights_mat(mat.length(), 1);
    for(int j=0; j<mat.width()-1; j++){
        column_j_mat = mat.column(j);
        weights_mat = mat.column(mat.width()-1);
        column_j_vec = column_j_mat.toVecCopy();
        weights_vec = weights_mat.toVecCopy();
        column_no_weighted_mean_j = mean(mat.column(j));
        result[j] = weighted_variance(column_j_vec, weights_vec, column_no_weighted_mean_j, column_weighted_mean[j]);
    }
}

template<class T>
void columnMax(const TMat<T>& mat, TVec<T>& result)
{
#ifdef BOUNDCHECK
    if(result.length()!=mat.width() || mat.length()==0)
        PLERROR("IN void columnMax(const TMat<T>& mat, TVec<T>& result) the length of result must equal the width of mat and mat must have non-zero length");
#endif
    for(int j=0; j<mat.width(); j++)
        result[j] = max(mat.column(j));
}

template<class T>
void columnMin(const TMat<T>& mat, TVec<T>& result)
{
#ifdef BOUNDCHECK
    if(result.length()!=mat.width() || mat.length()==0)
        PLERROR("IN void columnMax(const TMat<T>& mat, TVec<T>& result) the length of result must equal the width of mat and mat must have non-zero length");
#endif
    for(int j=0; j<mat.width(); j++)
        result[j] = min(mat.column(j));
}

template<class T>
void columnArgmax(const TMat<T>& mat, TVec<T>& result)
{
#ifdef BOUNDCHECK
    if(result.length()!=mat.width() || mat.length()==0)
        PLERROR("IN void columnMax(const TMat<T>& mat, TVec<T>& result) the length of result must equal the width of mat and mat must have non-zero length");
#endif
    int imax, jmax;
    for(int j=0; j<mat.width(); j++)
    {
        argmax(mat.column(j), imax, jmax);
        result[j] = imax;
    }
}

template<class T>
void columnArgmin(const TMat<T>& mat, TVec<T>& result)
{
#ifdef BOUNDCHECK
    if(result.length()!=mat.width() || mat.length()==0)
        PLERROR("IN void columnMax(const TMat<T>& mat, TVec<T>& result) the length of result must equal the width of mat and mat must have non-zero length");
#endif
    int imin, jmin;
    for(int j=0; j<mat.width(); j++)
    {
        argmin(mat.column(j), imin, jmin);
        result[j] = imin;
    }
}

template<class T>
T mahalanobis_distance(const TVec<T>& input, const TVec<T>& meanvec, const
                       TMat<T>& inversecovmat)
{
    TVec<T> diff = input-meanvec;
    return dot(diff,product(inversecovmat,diff));
}

//!  compute the mean of the rows of m (looping over columns)
template<class T>
inline void computeMean(const TMat<T>& m, TVec<T>& meanvec) { columnMean(m,meanvec); }

//!  compute the mean and variance of the rows of m (looping over columns)
template<class T>
void computeMeanAndVariance(const TMat<T>& m, TVec<T>& meanvec, TVec<T>& variancevec)
{
    columnMean(m,meanvec);
    columnVariance(m,variancevec,meanvec);
}

//! inverse_standard_deviation[i,j] =
//!    1/sqrt(mean_of_squares[i,j] - means[i,j]^2)
//! If 'min_stddev' is provided, any standard deviation less than this value
//! will be set to 'default_stddev' without any warning being issued (even when
//! a negative variance is encountered, which can happen because of numerical
//! approximation for an almost constant variable).
template<class T>
void computeInverseStandardDeviationFromMeanAndSquareMean(const TMat<T>& inverse_standard_deviation,
                                                          const TMat<T>& means,
                                                          const TMat<T>& mean_of_squares,
                                                          real default_stddev = 1,
                                                          real min_stddev = -1)
{
    int n=inverse_standard_deviation.length();
    int m=inverse_standard_deviation.width();
    int invs_mod = inverse_standard_deviation.mod();
    int mu_mod = means.mod();
    int mu2_mod = mean_of_squares.mod();
#ifdef BOUNDCHECK
    if (means.length()!=n || means.width()!=m || mean_of_squares.length()!=n
        || mean_of_squares.width()!=m)
        PLERROR("In computeInverseStandardDeviationFromMeanAndSquareMean - Arguments have incompatible sizes");
#endif
    T* invs = inverse_standard_deviation.data();
    T* mu = means.data();
    T* mu2 = mean_of_squares.data();
    for (int i=0;i<n;i++, invs += invs_mod, mu += mu_mod, mu2 += mu2_mod) {
        for (int j=0;j<m;j++)
        {
            real diff = mu2[j] - mu[j] * mu[j];
            if (diff>0) {
                real sqrt_diff = sqrt(diff);
                if (sqrt_diff < min_stddev)    // NB: Cannot happen if 'min_stddev' is -1.
                    invs[j] = real(1.0 / default_stddev);
                else
                    invs[j] = real(1.0 / sqrt_diff);
            }
            else {
                if (min_stddev < 0)
                    // No minimum standard deviation provided, this is suspect.
                    PLWARNING("In computeInverseStandardDeviationFromMeanAndSquareMean - Variance is not > 0");
                invs[j] = real(1.0 / default_stddev);
            }
        }
    }
}



template<class T>
void computeCovar(const TMat<T>& m, const TVec<T>& meanvec, TMat<T>& covarmat)
{
    int n = m.width();
    covarmat.resize(n,n);
    transposeProduct(covarmat,m,m);
    covarmat /= T(m.length());
    externalProductScaleAcc(covarmat,meanvec,meanvec,T(-1));
}

template<class T>
void computeMeanAndCovar(const TMat<T>& m, TVec<T>& meanvec, TMat<T>& covarmat)
{
    int n = m.width();
    meanvec.resize(n);
    covarmat.resize(n,n);
    columnMean(m,meanvec);

    transposeProduct(covarmat,m,m);
    covarmat /= T(m.length());
    externalProductScaleAcc(covarmat,meanvec,meanvec,T(-1));

    /*
      Mat mm = m.copy();
      mm -= meanvec;
      transposeProduct(covarmat,mm,mm);
      covarmat /= T(m.length());
    */
}

//!  compute the mean and standard deviations of the rows of m (looping over columns)
template<class T>
void computeMeanAndStddev(const TMat<T>& m, TVec<T>& meanvec, TVec<T>& stddevvec)
{
    columnMean(m,meanvec);
    columnVariance(m,stddevvec,meanvec);
    int l=stddevvec.length();
    for(int i=0; i<l; i++)
        stddevvec[i] = sqrt(stddevvec[i]);
}


//!  compute the mean and standard deviations of the colums of m (looping over s)
//!  (the result is stored in column vectors meanvec and stddevvec)
template<class T>
void computeColumnsMeanAndStddev(const TMat<T>& m, TMat<T>& meanvec, TMat<T>& stddevvec)
{
    rowMean(m,meanvec);
    rowVariance(m,stddevvec,meanvec);
    int l=stddevvec.length();
    for(int i=0; i<l; i++)
        stddevvec[i][0] = sqrt(stddevvec[i][0]);
}

//! substract mean, and divide by stddev (these are estimated globally)
template<class T>
void normalize(TMat<T>& m)
{
    TVec<T> meanvec(m.width());
    TVec<T> stddevvec(m.width());
    computeMeanAndStddev(m,meanvec,stddevvec);
    m -= meanvec;
    m /= stddevvec;
}

//! Divides each row by the sum of its elements
template<class T>
void normalizeRows(const TMat<T>& m)
{
    int l = m.length();
    for(int i=0; i<l; i++)
    {
        TVec<T> v = m(i);
        v /= sum(v);
    }
}

//! Divides each column by the sum of its elements
template<class T>
void normalizeColumns(const TMat<T>& m)
{
    int w = m.width();
    for(int j=0; j<w; j++)
    {
        TMat<T> v = m.column(j);
        v /= sum(v);
    }
}

//! divide each row by its n norm
template<class T>
void normalize(TMat<T>& m, double n)
{
    for(int i=0; i<m.length(); i++)
    {
        TVec<T> m_i = m(i);
        normalize(m_i,n);
    }
}

template<class T>
void operator+=(const TMat<T>& m, T scalar)
{
    T* m_i = m.data();
    int w = m.width();
    for(int i=0; i<m.length(); i++, m_i+=m.mod())
        for(int j=0; j<w; j++)
            m_i[j] += scalar;
}

template<class T>
void operator*=(const TMat<T>& m, T scalar)
{
    T* m_i = m.data();
    int w = m.width();
    for(int i=0; i<m.length(); i++, m_i+=m.mod())
        for(int j=0; j<w; j++)
            m_i[j] *= scalar;
}

template<class T>
inline void operator-=(const TMat<T>& m, T scalar) { m += (-scalar); }

template<class T>
inline void operator/=(const TMat<T>& m, T scalar) { m *= (T(1)/scalar); }

template<class T>
inline void operator/=(const TMat<T>& m, int scalar) { m *= (T(1)/scalar); }


//!  adds v to every row
template<class T>
void operator+=(const TMat<T>& m, const TVec<T>& v)
{
#ifdef BOUNDCHECK
    if(m.width()!=v.length())
        PLERROR("IN operator+=(const TMat<T>& m, const TVec<T>& v) v must be as long as m is wide");
#endif
    T* m_i = m.data();
    T* vv = v.data();
    int w=m.width();
    for(int i=0; i<m.length(); i++, m_i+=m.mod())
        for(int j=0; j<w; j++)
            m_i[j] += vv[j];
}

//!  subtracts v from every row
template<class T>
void operator-=(const TMat<T>& m, const TVec<T>& v)
{
#ifdef BOUNDCHECK
    if(m.width()!=v.length())
        PLERROR("IN operator-=(const TMat<T>& m, const TVec<T>& v) v must be as long as m is wide");
#endif
    T* m_i = m.data();
    T* vv = v.data();
    int w=m.width();
    for(int i=0; i<m.length(); i++, m_i+=m.mod())
        for(int j=0; j<w; j++)
            m_i[j] -= vv[j];
}

//! does an elementwise multiplication of every row by v
template<class T>
void operator*=(const TMat<T>& m, const TVec<T>& v)
{
#ifdef BOUNDCHECK
    if(m.width()!=v.length())
        PLERROR("IN operator*=(const TMat<T>& m, const TVec<T>& v) v must be as long as m is wide");
#endif
    T* m_i = m.data();
    T* vv = v.data();
    int w=m.width();
    for(int i=0; i<m.length(); i++, m_i+=m.mod())
        for(int j=0; j<w; j++)
            m_i[j] *= vv[j];
}

//!  does an elementwise multiplication
template<class T>
void operator*=(const TMat<T>& m1, const TMat<T>& m2)
{
    int n=m1.length();
    int l=m1.width();
#ifdef BOUNDCHECK
    if(l!=m2.width() || n!=m2.length())
        PLERROR("IN operator*=(const TMat<T>& m1(%d,%d), const TMat<T>& m2(%d,%d)) sizes differ",
                m1.length(),m1.width(),m2.length(),m2.width());
#endif
    T* m1_i = m1.data();
    T* m2_i = m2.data();
    for(int i=0; i<n; i++, m1_i+=m1.mod(),m2_i+=m2.mod())
        for(int j=0; j<l; j++)
            m1_i[j] *= m2_i[j];
}

//!<  does an elementwise division of every row by v
template<class T>
void operator/=(const TMat<T>& m, const TVec<T>& v)
{
#ifdef BOUNDCHECK
    if(m.width()!=v.length())
        PLERROR("IN operator/=(const TMat<T>& m, const TVec<T>& v) v must be as long as m is wide");
#endif
    T* m_i = m.data();
    T* vv = v.data();
    int w=m.width();
    for(int i=0; i<m.length(); i++, m_i+=m.mod())
        for(int j=0; j<w; j++)
            m_i[j] /= vv[j];
}

//! does an elementwise division
template<class T>
void operator/=(const TMat<T>& m1, const TMat<T>& m2)
{
    int n=m1.length();
    int l=m1.width();
#ifdef BOUNDCHECK
    if(l!=m2.width() || n!=m2.length())
        PLERROR("IN operator/=(const TMat<T>& m1(%d,%d), const TMat<T>& m2(%d,%d)) sizes differ",
                m1.length(),m1.width(),m2.length(),m2.width());
#endif
    T* m1_i = m1.data();
    T* m2_i = m2.data();
    for(int i=0; i<n; i++, m1_i+=m1.mod(),m2_i+=m2.mod())
        for(int j=0; j<l; j++)
            m1_i[j] /= m2_i[j];
}

template<class T>
void operator+=(const TMat<T>& m1, const TMat<T>& m2)
{
    int n=m1.length();
    int l=m1.width();
#ifdef BOUNDCHECK
    if(m1.width()!=m2.width() || m1.length()!=m2.length())
        PLERROR("IN operator+=(const TMat<T>& m1(%d,%d), const TMat<T>& m2(%d,%d)): m1 and m2 must have same dimensions",
                m1.length(),m1.width(),m2.length(),m2.width());
#endif
    T* m1_i = m1.data();
    T* m2_i = m2.data();
    for(int i=0; i<n; i++, m1_i+=m1.mod(),m2_i+=m2.mod())
        for(int j=0; j<l; j++)
            m1_i[j] += m2_i[j];
}

template<class T>
void operator-=(const TMat<T>& m1, const TMat<T>& m2)
{
    int n=m1.length();
    int l=m1.width();
#ifdef BOUNDCHECK
    if(m1.width()!=m2.width() || m1.length()!=m2.length())
        PLERROR("IN operator+=(const TMat<T>& m1(%d,%d), const TMat<T>& m2(%d,%d)): m1 and m2 must have same dimensions",
                m1.length(),m1.width(),m2.length(),m2.width());
#endif
    if(m1.isNotEmpty()) // calc only if some data
    {
        T* m1_i = m1.data();
        T* m2_i = m2.data();
        for(int i=0; i<n; i++, m1_i+=m1.mod(),m2_i+=m2.mod())
            for(int j=0; j<l; j++)
                m1_i[j] -= m2_i[j];
    }
}

template<class T>
TMat<T> operator-(const TMat<T>& m1, const TMat<T>& m2)
{
    TMat<T> result(m1.length(), m1.width());
    substract(m1,m2,result);
    return result;
}

template<class T>
TMat<T> operator+(const TMat<T>& m1, const TMat<T>& m2)
{
    TMat<T> result(m1.length(), m1.width());
    add(m1,m2,result);
    return result;
}

template<class T>
void substract(const TMat<T>& m1, const TMat<T>& m2, TMat<T>& destination)
{
#ifdef BOUNDCHECK
    if(m1.width()!=m2.width() || m1.length()!=m2.length()
       || m1.width()!=destination.width() || m1.length()!=destination.length())
        PLERROR("IN substract(m1(%d,%d), m2(%d,%d), dest(%d,%d)): args must have same dimensions",
                m1.length(),m1.width(),m2.length(),m2.width(),destination.length(),
                destination.width());
#endif
    T* m1_i = m1.data();
    T* m2_i = m2.data();
    T* d_i = destination.data();
    int m1_mod = m1.mod();
    int m2_mod = m2.mod();
    int d_mod = destination.mod();
    int w = m1.width();
    for (int i=0;i<m1.length();i++,m1_i+=m1_mod,m2_i+=m2_mod,d_i+=d_mod)
        for (int j=0;j<w;j++)
            d_i[j] = m1_i[j] - m2_i[j];
}

template<class T>
void add(const TMat<T>& m1, const TMat<T>& m2, TMat<T>& destination)
{
#ifdef BOUNDCHECK
    if(m1.width()!=m2.width() || m1.length()!=m2.length()
       || m1.width()!=destination.width() || m1.length()!=destination.length())
        PLERROR("IN substract(m1(%d,%d), m2(%d,%d), dest(%d,%d)): args must have same dimensions",
                m1.length(),m1.width(),m2.length(),m2.width(),destination.length(),
                destination.width());
#endif
    T* m1_i = m1.data();
    T* m2_i = m2.data();
    T* d_i = destination.data();
    int m1_mod = m1.mod();
    int m2_mod = m2.mod();
    int d_mod = destination.mod();
    int w = m1.width();
    for (int i=0;i<m1.length();i++,m1_i+=m1_mod,m2_i+=m2_mod,d_i+=d_mod)
        for (int j=0;j<w;j++)
            d_i[j] = m1_i[j] + m2_i[j];
}

//! return a negated copy of m
template<class T>
TMat<T> operator-(const TMat<T>& m)
{
    TMat<T> opposite(m.length(),m.width());
    T *m_i=m.data();
    T *o_i=opposite.data();
    int w=m.width();
    for (int i=0;i<m.length();i++,m_i+=m.mod(),o_i+=opposite.mod())
        for (int j=0;j<w;j++)
            o_i[j] = - m_i[j];
    return opposite;
}

//! x'_ij = -x_ij;
template<class T>
void negateElements(const TMat<T>& m)
{
    T* m_i = m.data();
    int w=m.width();
    for(int i=0; i<m.length(); i++, m_i+=m.mod())
        for(int j=0; j<w; j++)
            m_i[j] = -m_i[j];
}

//! x'_ij = 1.0/x_ij;
template<class T>
void invertElements(const TMat<T>& m)
{
    T* m_i = m.data();
    int w=m.width();
    for(int i=0; i<m.length(); i++, m_i+=m.mod())
        for(int j=0; j<w; j++)
            m_i[j] = 1.0/m_i[j];
}

// result * m = identity
// (works only if m.length()>=m.width())
template<class T>
TMat<T> leftPseudoInverse(TMat<T>& m)
{
    TMat<T> inv(m.width(), m.length());
    leftPseudoInverse(m,inv);
    return inv;
}

// result * m = identity
// (works only if m.length()>=m.width())
template<class T>
void leftPseudoInverse(const TMat<T>& m, TMat<T>& inv)
{
    if (m.length()==m.width())
        inverse(m,inv);
    if (m.length()<m.width())
        PLERROR("leftPseudoInverse: matrix length(%d) must be >= width(%d)",
                m.length(), m.width());
    PLERROR("SVD not implemented yet");
}

// m * result = identity
// (works only if m.length()<=m.width())
template<class T>
TMat<T> rightPseudoInverse(TMat<T>& m)
{
    TMat<T> inv(m.width(), m.length());
    rightPseudoInverse(m,inv);
    return inv;
}

// m * result = identity
// (works only if m.length()<=m.width())
template<class T>
void rightPseudoInverse(const TMat<T>& m, TMat<T>& inv)
{
    if (m.length()==m.width())
        inverse(m,inv);
    if (m.length()>m.width())
        PLERROR("rightPseudoInverse: matrix length(%d) must be <= width(%d)",
                m.length(), m.width());
    PLERROR("SVD not implemented yet");
}

// find and return inv s.t. m * inv = inv * m = I = identity
// (m must be square)
template<class T>
TMat<T> inverse(TMat<T>& m)
{
    TMat<T> inv(m.length(),m.length());
    inverse(m,inv);
    return inv;
}

// find inv s.t. m * inv = inv * m = I = identity
// (m must be square)
template<class T>
void inverse(const TMat<T>& m, TMat<T>& inv)
{
    int n=m.length();
    if (m.width()!=n)
        PLERROR("inverse(TMat<T>,TMat<T>): argument(%d,%d) must be square matrix",
                m.width(), n);
    inv.resize(n,n);
    if (n==1)
        inv.data()[0]=1.0/m.data()[0];
    else
        PLERROR("matrix inverse not implemented yet");
}

// for square positive definite symmetric matrices A,
//     find X(n,m) s.t. A(n,n) X(n,m) = B(n,m).
// This is obtained by doing a Cholesky decomposition
//  A = L L', with L lower diagonal, thus to solve
//      L L' X = B.
// We use the CholeskySolve function which solves for x_i in L L' x_i = b_i
// (on the columns x_i and b_i of X and B respectively).
// Optionally provide pointers to the temporary matrix L(n,n) and vector y(n)
// to avoid memory allocations.
template<class T>
void solveLinearSystemByCholesky(const TMat<T>& A, const TMat<T>& B, TMat<T>& X, TMat<T>* pL=0, TVec<T>* py=0)
{
    int n=A.length();
    int m=X.width();
    if (X.length()!=n || A.width()!=n || B.length()!=n || B.width()!=m)
        PLERROR("solveLinearSystemByCholesky:  A(%d,%d) * X(%d,%d) == B(%d,%d), incompatible",
                n,A.width(),X.length(),m,B.length(),B.width());
    TMat<T>* L;
    TVec<T>* y;
    if (pL) L=pL; else L = new TMat<T>(n,n);
    if (py) y=py; else y = new TVec<T>(n);
    choleskyDecomposition(A,*L);
    choleskySolve(*L,B,X,*y);
    if (!pL) delete L;
    if (!py) delete y;
}

// for square positive definite symmetric matrices A,
//     find X(n,m) s.t. X(n,m) A(m,m) = B(n,m).
// This is obtained by doing a Cholesky decomposition
//  A = L L', with L lower diagonal, thus to solve
//      X L L' = B.
// We use the CholeskySolve function which solves for x_i in L L' x_i = b_i:
//      L L' X' = B'
// is solved on the rows of X (x_i) and the columns of B (b_i).
// Optionally provide pointers to the temporary matrices L and y
// to avoid memory allocations.
template<class T>
void solveTransposeLinearSystemByCholesky(const TMat<T>& A, const TMat<T>& B, TMat<T>& X,TMat<T>* pL=0, TVec<T>* py=0)
{
    int n=X.length();
    int m=X.width();
    if (A.length()!=m || A.width()!=m || B.length()!=n || B.width()!=m)
        PLERROR("solveTransposeLinearSystemByCholesky: X(%d,%d) * A(%d,%d) == B(%d,%d), incompatible",
                n,m,A.length(),A.width(),B.length(),B.width());
    TMat<T>* L;
    TVec<T>* y;
    if (pL) L=pL; else L = new TMat<T>(m,m);
    if (py) y=py; else y = new TVec<T>(m);
    choleskyDecomposition(A,*L);
    for (int i=0;i<n;i++)
        choleskySolve(*L,B(i),X(i),*y);
    if (!pL) delete L;
    if (!py) delete y;
}

/*  Perform a Cholesky decomposition of nxn symmetric positive definite
    matrix A, i.e., decompose it into
    A = L L'
    where L is a lower diagonal matrix (with zeros above the diagonal).
    L be used to solve a linear system A x = b, i.e., LL'x=b, with choleskySolve(L,b,x).
    See choleskySolve(TMat<T>*,TVec<T>*) for an example of use.

    From the above equation, one obtains

    for i=0..n-1
    L[i][i] = sqrt(A[i][i] - sum_{k=0}^{i-1} L[i][k]^2)
    for j=i+1... n-1
    L[j][i] = (1/L[i][i]) ( A[i][j] - sum_{k=0}^{i-1} L[i][k] L[j][k] )

*/
template<class T>
void  choleskyDecomposition(const TMat<T>& A, TMat<T>& L)
{
    int n = A.length();
    if (n!=A.width())
        PLERROR("choleskyDecomposition: non-square matrix %dx%d\n",n,A.width());
    L.resize(n,n);
    int i,j,k;
    T sum;
    bool restart=false;
    do
    {
        restart=false;
        for (i=0;i<n;i++)
        {
            const T* Ai = A[i];
            T* Li = L[i];
            T Lii=0;
            for (j=i;j<n;j++)
            {
                T* Lj = L[j];
                for (sum=Ai[j],k=i-1;k>=0;k--) sum -= Li[k] * Lj[k];
                if (i==j)
                {
                    if (sum <= 0.0)
                    {
                        T eps = -1.1*sum;
                        if (fast_exact_is_equal(sum,0.0)) eps=1e-8;
                        PLWARNING("Cholesky decomposition would fail: add %g to diagonal",eps);
                        // saveAscii("A.amat",A);
                        T* Aii=A.data();
                        int addm=A.mod()+1;
                        for (int ii=0;ii<n;ii++,Aii+=addm) *Aii += eps;
                        restart=true;
                        break;
                    }
                    Lii = sqrt(sum);
                }
                else Lj[i] = sum/Lii;
            }
            if (restart) break;
            Li[i] = Lii;
        }
    }
    while (restart);

}

/*  Back-propagate through the call to choleskyDecomposition(A,L).
    The argument A holds the original symmetric positive definite
    matrix while is the lower diagonal matrix with L L' = A.
    Given the derivative of C wrt L, fill the derivative
    of C wrt A. dC_dA must have been cleared beforehand.
    We are given A, L, dC_dL, and write into dC_dA.
    Note that dC_dL is modified after the call
    because of the internal dependencies between the L's.

    for i=n-1..0
    for j=n-1..i+1
    dC_dL[i][i] -= dC_dL[j][i] L[j][i] / L[i][i]
    dC_dA[i][j] += dC_dL[j][i] / L[i][i]
    for k=0..i-1
    dC_dL[i][k] -= dC_dL[j][i] L[j][k] / L[i][i]
    dC_dL[j][k] -= dC_dL[j][i] L[i][k] / L[i][i]
    dC_dA[i][i] += 0.5 * dC_dL[i][i] / L[i][i]
    for k=0..i-1
    dC_dL[i][k] -= dC_dL[i][i] L[i][k] / L[i][i]

*/
template<class T>
void  bpropCholeskyDecomposition(const TMat<T>& A, const TMat<T>& L,
                                 TMat<T>& dC_dA, TMat<T>& dC_dL)
{
    int n = A.length();
    if (dC_dA)
        dC_dA.resize(n,n);
    int i,j,k;
    for (i=n-1;i>=0;i--)
    {
        const T* Li = L[i];
        T* dC_dLi = dC_dL[i];
        T* dC_dAi = dC_dA[i];
        T invLii = 1.0/Li[i];
        for (j=n-1;j>i;j--)
        {
            const T* Lj = L[j];
            T* dC_dLj = dC_dL[j];
            T dC_dLji = dC_dLj[i];
            dC_dLi[i] -= dC_dLji * Lj[i] * invLii;
            dC_dAi[j] += dC_dLji * invLii;
            for (k=0;k<i;k++)
            {
                dC_dLi[k] -= dC_dLji * Lj[k] * invLii;
                dC_dLj[k] -= dC_dLji * Li[k] * invLii;
            }
        }
        T dC_dLii = dC_dLi[i];
        dC_dAi[i] += 0.5 * dC_dLii * invLii;
        for (k=0;k<i;k++)
            dC_dLi[k] -= dC_dLii * Li[k] * invLii;
    }
}

// Given L lower-diagonal, solve L y = b
template<class T>
void  choleskyLeftSolve(const TMat<T>& L, const TVec<T>& b, const TVec<T>& y)
{
    int i,k;
    T sum;
    int n = L.length();
#ifdef BOUNDCHECK
    if (L.width()!=n)
        PLERROR("choleskySolve: matrix L (%d x %d) is not square!",
                n, L.width());
    if (b.length()!=n || y.length()!=n)
        PLERROR("choleskySolve: RHS vector b(%d) or unknown y(%d) incompatible with L(%d,%d)",
                b.length(),y.length(),n,n);
#endif

    if (n == 0)
        // Empty matrix, there is nothing that needs being solved.
        return;

    T* bp = b.data();
    T* yp = y.data();

    // solve L y = b (in variable x if y=0):
    // for i=0..n-1
    //   y[i] = (b[i] - sum_{k<i} L[i][k] y[k])/L[i][i]
    for (i=0;i<n;i++)
    {
        const T* Li = L[i];
        for (sum=bp[i],k=i-1;k>=0;k--) sum -= Li[k] * yp[k];
        if (Li[i]==0)
            PLERROR("choleskyLeftSolve: found zero entry in diagonal of L (%d)",i);
        yp[i] = sum / Li[i];
    }
}

// Given L lower-diagonal, solve L' x = y
template<class T>
void  choleskyRightSolve(const TMat<T>& L, TVec<T>& y, TVec<T>& x)
{
    int i,k;
    T sum;
    int n = L.length();
#ifdef BOUNDCHECK
    if (L.width()!=n)
        PLERROR("choleskySolve: matrix L (%d x %d) is not square!",
                n, L.width());
    if (x.length()!=n || y.length()!=n)
        PLERROR("choleskySolve: RHS vector y(%d) or unknown x(%d) incompatible with L(%d,%d)",
                y.length(),x.length(),n,n);
#endif

    if (n == 0)
        // Empty matrix, there is nothing that needs being solved.
        return;

    T* xp = x.data();
    T* yp = y.data();

    // for i=n-1..0
    //   x[i] = (y[i] - sum_{k>i} L[k][i] x[k])/L[i][i]
    for (i=n-1;i>=0;i--)
    {
        for (sum=yp[i],k=i+1;k<n;k++) sum -= L[k][i] * xp[k];
        xp[i] = sum / L[i][i];
    }
}

/*  Solve the linear system A x = L L' x = b using a Cholesky decomposition
    of A into L L' performed with a prior call to choleskyDecomposition(A,L)
    (which on return has the matrix L, that is lower diagonal, and A = L L').
    The solution of the linear system L L' x = b will be in x.
    See choleskySolve(TMat<T>*,TVec<T>*) for an example of use.
    The algorithm is first to solve L y = b, and then L' x = y.
    The argument y is optional and can be used to hold the intermediate
    solution to L y = b.

    The solution to L L' x = b is obtained as follows:

    * Solve L y = b by iterating once through the rows of L
    (store result in x):
    y[i] = (b[i] - sum_{k<i} L[i][k] y[k])/L[i][i]

    * Solve L' x = y by iterating once (backwards) through the rows of L.
    x[i] = (y[i] - sum_{k>i} L[k][i] x[k])/L[i][i]

*/
template<class T>
void  choleskySolve(const TMat<T>& L, TVec<T> b, TVec<T> x, TVec<T>& y)
{
    // solve L y = b
    choleskyLeftSolve(L,b,y);
    // solve L' x = y
    choleskyRightSolve(L,y,x);
}

// same as the previous choleskySolve but do it m times on the columns
// of nxm matrices X and B.
template<class T>
void  choleskySolve(const TMat<T>& L, const TMat<T>& B, TMat<T>& X, TVec<T>& y)
{
    int i,k;
    T sum;
    int n = L.length();
    int m = X.width();
    if (L.width()!=n)
        PLERROR("choleskySolve: matrix L (%d x %d) is not square!",
                n, L.width());
    if (B.length()!=n || B.width() !=m)
        PLERROR("choleskySolve: RHS matrix B(%d,%d) instead of (%d,%d) like X",
                B.length(),B.width(), n, m);
    if (X.length()!=n)
        PLERROR("choleskySolve: X(%d,%d) not compatible with L(%d,%d)",
                X.length(),m,n,n);
    if (y.length()!=n)
        PLERROR("choleskySolve: y(%d) not compatible with L(%d,%d)",
                y.length(),n,n);
    int bmod = B.mod();
    int xmod = X.mod();
    // loop over columns b and x of B and X
    for (int j=0;j<m;j++)
    {
        T* bp = B.data()+j;
        T* yp = y.data();
        // solve L y = b (in variable x if y=0):
        // for i=0..n-1
        //   y[i] = (b[i] - sum_{k<i} L[i][k] y[k])/L[i][i]
        for (i=0;i<n;i++,bp+=bmod)
        {
            const T* Li = L[i];
            for (sum = *bp,k=i-1;k>=0;k--) sum -= Li[k] * yp[k];
            yp[i] = sum / Li[i];
        }
        // solve L' x = y
        // for i=n-1..0
        //   x[i] = (y[i] - sum_{k>i} L[k][i] x[k])/L[i][i]
        for (i=n-1;i>=0;i--)
        {
            sum=yp[i];
            if (i+1<n)
            {
                T* xp = &X(i+1,j);
                for (k=i+1;k<n;k++,xp+=xmod) sum -= L[k][i] * *xp;
            }
            X(i,j) = sum / L[i][i];
        }
    }
}

/*
   Back-propagate through the CholeskySolve(L,b,x,y) operation
   (the optional argument y of this call must have been provided).

   dC_dL and dC_db must have been cleared beforehand.
   dC_dx will be modified (because of the dependencies between the x's.

   (1) back-prop through step L' x = y:
   for i=0..n-1
   dC_dy[i] = dC_dx[i] / L[i][i]
   dC_dL[i][i] -= dC_dx[i] x[i] / L[i][i]
   for k=i+1..n
   dC_dx[k]    -= dC_dx[i] L[k][i] / L[i][i]
   dC_dL[k][i] -= dC_dx[i] x[k] / L[i][i]

   (2) back-prop through step L y = b:
   for i=n-1..0
   dC_db[i] = dC_dy[i] / L[i][i]
   dC_dL[i][i] -= dC_dy[i] y[i] / L[i][i]
   for k=0..i-1
   dC_dy[k]    -= dC_dy[i] L[i][k] / L[i][i]
   dC_dL[i][k] -= dC_dy[i] * y[k] / L[i][i]
*/
template<class T>
void  bpropCholeskySolve(const TMat<T>& L, const TVec<T>& x, const TVec<T>& y,
                         TMat<T>& dC_dL, TVec<T>& dC_db, TVec<T>& dC_dx)
{
    int n = L.length();
    int i,k;
    TVec<T> dC_dy(n);
    const T *xp = x.data();
    const T *yp = y.data();
    T *dC_dbp = dC_db.data();
    T *dC_dxp = dC_dx.data();
    T* dC_dyp = dC_dy.data();

    // (1) back-prop through step L' x = y:
    for (i=0;i<n;i++)
    {
        const T* Li = L[i];
        T invLii = 1.0 / Li[i];
        dC_dyp[i] = dC_dxp[i] * invLii;
        T dC_dxi = dC_dxp[i];
        dC_dL[i][i] -= dC_dxp[i] * xp[i] * invLii;
        for (k=i+1;k<n;k++)
        {
            dC_dxp[k] -= dC_dxi * L[k][i] * invLii;
            dC_dL[k][i] -= dC_dxi * xp[k] * invLii;
        }
    }

    // (2) back-prop through step L y = b:
    for (i=n-1;i>=0;i--)
    {
        const T* Li = L[i];
        T* dC_dLi = dC_dL[i];
        T invLii = 1.0 / Li[i];
        T dC_dyi = dC_dyp[i];
        T dC_dyi_over_Lii = dC_dyi * invLii;
        dC_dbp[i] += dC_dyi_over_Lii;
        dC_dLi[i] -= dC_dyi_over_Lii * yp[i];
        for (k=0;k<i;k++)
        {
            dC_dyp[k] -= dC_dyi_over_Lii * Li[k];
            dC_dLi[k]  -= dC_dyi_over_Lii * yp[k];
        }
    };
}

/*  Use Cholesky decomposition to invert symmetric
    positive definite matrix A.
    Also returns the log of the determinant of A

    We have L L' = A, and we want to solve L L' Ainv = I.

    1) solve L Linv = I, i.e., invert L

    for j=0..n-1
    Linv[j][j] = 1 / L[j][j]
    for i=j+1..n-1
    Linv[i][j] = - sum_{j<=k<i} L[i][k] Linv[k][j] / L[i][i]
    and 0 elsewhere (Linv is lower diagonal)

    2) solve L' Ainv = Linv

    for j=0..n-1
    for i=n-1..0
    Ainv[i][j] = (Linv[i][j] - sum_{k>i} L[k][i] Ainv[k][j])/L[i][i]

*/
template<class T>
real choleskyInvert(const TMat<T>& A, TMat<T>& Ainv)
{
    int n= A.length();
    TMat<T> L(n,n);
    Ainv.resize(n,n);

    choleskyDecomposition(A,L);
    // now L L' = A

    real logdet = pl_log(fabs(L(0,0)));
    for(int i=1; i<n; i++)
        logdet += pl_log(fabs(L(i,i)));
    logdet *= 2;

    // Compute Linv and put its transpose above L's diagonal.
    // and put Linv[i][i] = 1 / L[i][i] in L's diagonal.
    int i,j;
    T *Lii = L.data();
    for (i=0;i<n;i++,Lii+=1+n)
        *Lii = 1.0 / *Lii;

    for (j=0;j<n;j++)
    {
        T *Linv_xj = L[j]; // Linv' in L's upper triangle
        for (i=j+1;i<n;i++)
        {
            T sum=0.0;
            T* Li = L[i];
            int k;
            for (k=j;k<i;k++) sum -= Li[k] * Linv_xj[k];
            Linv_xj[i] = sum * Li[i]; // * not / because inverse already done above
        }
    }
    // recall: now Linv above and on diagonal of L, L below it,

    // compute A's inverse
    for (j=0;j<n;j++)
    {
        T* Linv_xj = L[j];
        for (i=n-1;i>=j;i--)
        {
            T sum = Linv_xj[i]; // this is Linv[i][j]
            int k;
            for (k=i+1;k<n;k++)
                sum -= L[k][i] * Ainv[k][j];
            Ainv[i][j] = sum * L[i][i];
        }
        for (i=j-1;i>=0;i--) // symmetric part
            Ainv[i][j] = Ainv[j][i];
    };

    return logdet;
}

/*  Solve a linear system of equations A x = b, when A is
    symmetric positive definite. Return x.  */
template<class T>
TVec<T> choleskySolve(const TMat<T>& A, const TVec<T>& b)
{
    int n = A.length();
    TMat<T> L(n,n);
    TVec<T> x(n);
    choleskyDecomposition(A,L);
    choleskySolve(L,b,x);
    return x;
}

/*  return inverse of positive definite matrix A
    using Cholesky decomposition. No side-effect on A.  */
template<class T>
TMat<T> choleskyInvert(const TMat<T>& A)
{
    int n=A.length();
    TMat<T> Ainv(n,n);
    choleskyInvert(A,Ainv);
    return Ainv;
}

template<class T>
void LU_decomposition(TMat<T>& A, TVec<T>& Trow, int& detsign, TVec<T>* p=0)
{
    int n=A.length();
    if (n!=A.width())
        PLERROR("LU_decomposition: matrix A(%d,%d) should be square", n,A.width());
    TVec<T>* pivot = (p==0)?new TVec<T>(n):p;
    T* pv = pivot->data();
    detsign = 1;
    for (int i=0;i<n;i++)
    {
        T max_abs = maxabs(A(i));
        if (max_abs==0)
            PLERROR("LU_decomposition: %d-th row has only zeros",i);
        pv[i] = 1.0 / max_abs;
    }
    int mod = A.mod();
    for (int j=0;j<n;j++)
    {
        for (int i=0;i<j;i++)
        {
            T* Ai = A[i];
            T* Akj = A.data()+j;
            T Uij = Ai[j];
            for (int k=0;k<i;k++,Akj+=mod)
                Uij -= Ai[k] * *Akj;
            Ai[j] = Uij;
        }
        T max_abs = 0;
        int maxi = 0;
        for (int i=j;i<n;i++)
        {
            T* Ai = A[i];
            T* Akj = A.data()+j;
            T Lij = Ai[j];
            for (int k=0;k<j;k++,Akj+=mod)
                Lij -= Ai[k] * *Akj;
            Ai[j] = Lij;
            T piv = fabs(Lij) * pv[i];
            if (piv >= max_abs)
            {
                maxi = i;
                max_abs = piv;
            }
        }
        if (j!=maxi)
            // swap row j and row maxi
        {
            A.swapRows(j,maxi);
            pv[maxi]=pv[j];
            detsign = -detsign;
        }
        Trow[j] = maxi;
        T& Ajj = A(j,j);
        if (Ajj==0) Ajj=1e-20; // some regularization of singular matrices
        if (j<n-1)
        {
            T denom = 1.0/Ajj;
            T* Aij = &A(j+1,j);
            for (int i=j+1;i<n;i++, Aij+=mod)
                *Aij *= denom;
        }
    }
    if (p == 0) delete pivot;
}

//! Return the determinant of A, using LU decomposition.
//! If 'log_det' is set to true, the log determinant is returned.
template<class T>
T det(const TMat<T>& A, bool log_det = false)
{
    // Work storage.
    static TMat<T> LU;
    static TVec<T> Trow, p;

    int n = A.length();
    if (n!=A.width())
        PLERROR("det(const TMat<T>& A): A(%d,%d) is not square!",n,A.width());
    for (int i=0;i<n;i++)
    {
        T max_abs = maxabs(A(i));
        if (max_abs==0)
            return 0.0;
    }
    LU.resize(A.length(), A.width());
    LU << A;
    Trow.resize(n);
    p.resize(n);
    int detsign;
    LU_decomposition(LU, Trow, detsign, &p);
    return det(LU, detsign, log_det);
}

//! Return the determinant of A, whose LU decomposition is given ('detsign' is
//! as set by the LU_decomposition(..) function).
//! If 'log_det' is set to true, the log determinant is returned.
template<class T>
T det(const TMat<T>& LU, int detsign, bool log_det = false)
{
    T determinant = detsign;
    bool minus = false;
    if (log_det) {
        if (detsign < 0) {
            minus = !minus;
            detsign = - detsign;
        }
        determinant = pl_log(double(detsign));
    }
    int mod = LU.mod();
    int n = LU.width();
    if (n!=LU.width())
        PLERROR("det(const TMat<T>& LU, int detsign): LU(%d,%d) is not square!",n,LU.width());
    T* LUii = LU.data();
    if (log_det) {
        for (int i=0;i<n;i++, LUii+=1+mod) {
            real LUii_ = *LUii;
            if (LUii_ < 0) {
                minus = !minus;
                LUii_ = - LUii_;
            }
            determinant += pl_log(LUii_);
        }
    } else {
        for (int i=0;i<n;i++, LUii+=1+mod)
            determinant *= *LUii;
    }
    if (log_det && minus)
        // The determinant is negative: its log should be NaN.
        determinant = MISSING_VALUE;
    return determinant;
}

// dest[i,j] = 1 if src[i,j]==v, 0 otherwise
template<class T>
void equals(const TMat<T>& src, T v, TMat<T>& dest)
{
    int l=src.length();
    int w=src.width();
#ifdef BOUNDCHECK
    if (l!=dest.length() || w!=dest.width())
        PLERROR("equals(TMat<T>(%d,%d),T,TMat<T>(%d,%d)) args of unequal dimensions",
                src.length(),src.width(),dest.length(),dest.width());
#endif
    for (int i=0;i<l;i++)
    {
        const T* s=src[i];
        T* d=dest[i];
        for (int j=0;j<w;j++)
            if (s[j]==v) d[j]=1.0; else d[j]=0.0;
    }
}

// dest[i,j] = src[i,j]
template<class T>
void transpose(const TMat<T> src, TMat<T> dest)
{
    int l=src.length();
    int w=src.width();
#ifdef BOUNDCHECK
    if (w!=dest.length() || l!=dest.width())
        PLERROR("transpose(TMat<T>(%d,%d),T,TMat<T>(%d,%d)) args of unequal dimensions",
                src.length(),src.width(),dest.length(),dest.width());
#endif
    int dmod=dest.mod();
    for (int i=0;i<l;i++)
    {
        const T* si=src[i];
        T* dji= &dest(0,i);
        for (int j=0;j<w;j++,dji+=dmod)
            *dji = si[j];
    }
}

// res[i,j] = src[i,j]
template<class T>
TMat<T> transpose(const TMat<T>& src)
{
    TMat<T> res(src.width(),src.length());
    transpose(src,res);
    return res;
}

//! Transform a matrix of T into a matrix of U through a unary function
template<class T, class U>
void apply(U (*func)(T), const TMat<T>& source, TMat<U>& destination)
{
    int l=source.length();
    int w=source.width();
    if (l!=destination.length() || w!=destination.width())
        PLERROR("apply: source(%d,%d) TMat<T> and destination(%d,%d) TMat<U> must have same length and width",
                l,w,destination.length(),destination.width());
    for(int i=0; i<l; i++) {
        for(int j=0; j<w; j++)
            destination(i,j)=func(source(i,j));
    }
}

//! Transform a matrix of T into a matrix of U through a unary function
//! Same as above, for coherence with TVec<T>'s notation
template<class T, class U>
void apply(const TMat<T>& source, TMat<U>& destination, U (*func)(T))
{
    apply(func, source, destination);
}

// Apply a vector operation to each row of matrices, result in rows of a matrix
template<class T>
void apply(T (*func)(const TVec<T>&), const TMat<T>& m, TMat<T>& dest)
{
    if (dest.length()!=m.length())
        PLERROR("apply: m.length_=%d, dest.length_=%d",
                m.length(),dest.length());
    int l=m.length();
    for (int i=0;i<l;i++)
        dest(i,0)=func(m(i));
}

template<class T>
void apply(T (*func)(const TVec<T>&,const TVec<T>&), const TMat<T>& m1, const TMat<T>& m2,
           TMat<T>& dest)
{
    if (dest.length()!=m1.length() || m1.length()!=m2.length())
        PLERROR("apply: m1.length_=%d, m2.length_=%d, dest.length_=%d",
                m1.length(),m2.length(),dest.length());
    for (int i=0;i<m1.length();i++)
        dest(i,0)=func(m1(i),m2(i));
}

// Perform a traditional linear regression (but with weight decay),
// without bias term. i.e. find weights such that:
//
//   norm(weights*inputs - outputs) + weight_decay*norm(weights)
//
// is minimized,
//
// This is achieved by solving the following linear system:
//
//   (X' X + weight_decay I) * weights = X' outputs

template<class T>
void linearRegressionNoBias(TMat<T> inputs, TMat<T> outputs, T weight_decay,
                            TMat<T> weights)
{
    int inputsize = inputs.width();
    int outputsize = outputs.width();
    int l = inputs.length();
    if(outputs.length()!=l)
        PLERROR("In linearRegressionNoBias: inputs and outputs should have the same length");
    if(weights.length()!=inputsize || weights.width()!=outputsize)
        PLERROR("In linearRegressionNoBias: weights should be a (inputsize x outputsize) matrix (%d x %d)",inputsize,outputsize);
    static TMat<T> XtX;
    XtX.resize(inputsize,inputsize);
    transposeProduct(XtX, inputs,inputs);
    static TMat<T> XtY;
    XtY.resize(inputsize,outputsize);
    transposeProduct(XtY, inputs,outputs);
    for(int i=0; i<inputsize; i++)
        XtX(i,i) += weight_decay;
    solveLinearSystemByCholesky(XtX,XtY,weights);
}


// Perform a traditional linear regression (but with weight decay),
// i.e. find bias and weights such that
//
//   norm(bias + weights*inputs - outputs) + weight_decay*norm(weights)
//
// is minimized, where theta'=(biases;weights') {biases in first row}
//
// This is achieved by solving the following linear system:
//
//   (X' X + weight_decay I) * theta = X' outputs
//
// where X = augmented inputs, i.e. X(t) = (1,inputs(t))
//
template<class T>
void linearRegression(TMat<T> inputs, TMat<T> outputs, T weight_decay,
                      TMat<T> theta_t)
{
    int l = inputs.length();
    int n_inputs = inputs.width();
    int n_outputs = outputs.width();
    if (outputs.length()!=l)
        PLERROR("linearRegression: inputs.length_=%d while outputs.length_=%d",
                l,outputs.length());
    if (theta_t.length()!=n_inputs+1 || theta_t.width()!=n_outputs)
        PLERROR("linearRegression: theta_t(%d,%d) should be (n_inputs(%d)+1)xn_outputs(%d)",
                theta_t.length(),theta_t.width(),n_inputs,n_outputs);

    int n=n_inputs+1;

    static TMat<T> XtX;
    XtX.resize(n,n);
    XtX.clear();
    static TMat<T> XtY;
    XtY.resize(n,n_outputs);
    XtY.clear();
    // compute X' X and X'Y:
    // XtX(i,j) = sum_t X[t,i]*X[t,j] (with X[t,0]=1, X[t,i+1]=inputs[t,i])
    // YtY(i,j) = sum_t X[t,i]*Y[t,j]
    //
    int xmod=inputs.mod();
    int ymod=outputs.mod();
    T *xt = inputs.data();
    T *yt = outputs.data();
    XtX(0,0) = l; // we know the answer ahead of time for element (0,0)
    for (int t=0;t<l;t++,xt+=xmod,yt+=ymod)
    {
        T* xx0 = XtX.data();
        T* xy0 = XtY.data();
        for (int j=0;j<n_outputs;j++)
            xy0[j] += yt[j];
        T *xxi = xx0+n; // start the inner matrix at (1,0)
        T *xyi = xy0+n_outputs; // start xy at (1,0)
        for (int i=0;i<n_inputs;i++,xxi+=n,xyi+=n_outputs)
        {
            T xti = xt[i];
            xxi[0]+=xti;
            T *xxip=xxi+1;
            for (int j=0;j<i;j++)
                xxip[j] += xti*xt[j];
            xxip[i]+=xti*xti;
            for (int j=0;j<n_outputs;j++)
                xyi[j] += xti * yt[j];
        }
    }
    // now do the symmetric part of XtX
    T* xx = XtX.data();
    T* xxi = xx+n;
    for (int i=1;i<n;i++,xxi+=n)
    {
        T *xx_i=xx+i;
        for (int j=0;j<i;j++,xx_i+=n)
            *xx_i = xxi[j];
    }

    // add weight_decay on the diagonal of XX' (except for the bias)
    T* xxii = &XtX(1,1);
    for (int i=0;i<n_inputs;i++,xxii+=1+n)
        *xxii += weight_decay;

    // now solve by Cholesky decomposition
    solveLinearSystemByCholesky(XtX,XtY,theta_t);
}

// Compute a linear fitting of 2 dimensional data resulting
// in parameters m et b for y = mx + b
//                         1                                    1
// Cost function used: C = - Sum[t] { (m * x_t + b - y_t)^2 } + - weight_decay * m^2
//                         2                                    2

template<class T>
void linearRegression(TVec<T> inputs, TVec<T> outputs, T weight_decay, TVec<T> theta_t)
{
    int npts = inputs.length();

    if (outputs.length()!=npts)
        PLERROR("linearRegression: inputs.length_=%d while outputs.length_=%d",
                inputs.length(),outputs.length());
    if (theta_t.length()!=2)
        PLERROR("linearRegression: theta_t(%d) should be 2", theta_t.length());

    T sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0, sum2_x = 0, sum2_y = 0;

    for (int i = 0; i < npts; ++i) {
        sum_x += inputs[i];
        sum_y += outputs[i];
        sum_xy += inputs[i] * outputs[i];
        sum_x2 += inputs[i] * inputs[i];
    }
    sum2_x = sum_x * sum_x;
    sum2_y = sum_y * sum_y;

    // m
    theta_t[1] = (sum_xy - (sum_x * sum_y) / npts) / (sum_x2 + weight_decay - sum2_x / npts);
    // b
    theta_t[0] = (sum_y - theta_t[1] * sum_x) / npts;
}


template<class T>
TMat<T> smooth(TMat<T> data, int windowsize)
{
    TVec<T> sumvec(data.width());
    TMat<T> result(data.length(), data.width());
    int currentwindowsize = windowsize/2;
    for(int k=0; k<currentwindowsize; k++)
        sumvec += data(k);
    result(0) << sumvec;
    //result(0) /= (T)currentwindowsize;
    TVec<T> res0 = result(0);
    res0 /= (T)currentwindowsize;
    result(0) << res0;

    for(int i=0; i<data.length(); i++)
    {
        int lowi = i-(windowsize-1)/2; // lowest index of window rows (inclusive)
        int highi = i+windowsize/2; // highest index of window rows (inclusive)
        if(lowi-1>=0) // remove row lowi-1 if it exists
        {
            sumvec -= data(lowi-1);
            currentwindowsize--;
        }
        if(highi<data.length()) // add row highi if it exists
        {
            sumvec += data(highi);
            currentwindowsize++;
        }
        result(i) << sumvec;
        //result(i) /= (T)currentwindowsize;
        TVec<T> resi = result(i);
        resi /= (T)currentwindowsize;
        result(i) << resi;
    }


    return result;
}


template<class T>
TMat<T> square(const TMat<T>& m)
{
    TMat<T> res(m.length(), m.width());
    int w=m.width();
    for(int i=0; i<m.length(); i++)
        for(int j=0; j<w; j++)
            res(i,j) = square(m(i,j));
    return res;
}

template<class T>
TMat<T> sqrt(const TMat<T>& m)
{
    TMat<T> res(m.length(), m.width());
    int w=m.width();
    for(int i=0; i<m.length(); i++)
        for(int j=0; j<w; j++)
            res(i,j) = sqrt(m(i,j));
    return res;
}

template<class T>
inline void affineMatrixInitialize(TMat<T> W, bool output_on_columns=true, real scale=1.0)
{
    int n_inputs = output_on_columns?W.width():W.length();
    real delta = scale/n_inputs;
    fill_random_uniform(W,-delta,delta);
    W(0).clear();
}

template<class T>
TMat<T> grep(TMat<T> data, int col, TVec<T> values, bool exclude=false)
{
    TMat<T> result(data.length(),data.width());
    int length=0;

    for(int i=0; i<data.length(); i++)
    {
        bool contains = values.contains(data(i,col));
        if( (!exclude && contains) || (exclude && !contains) )
            result(length++) << data(i);
    }
    result.resize(length,result.width());
    result.compact(); // use less memory
    return result;
}


template<class T>
void convolve(TMat<T> m, TMat<T> mask, TMat<T> result)
{
    if(result.length() != m.length()-mask.length()+1 || result.width() != m.width()-mask.width()+1)
        PLERROR("In convolve(TMat<T> m, TMat<T> mask, TMat<T> result), result does not have the appropriate dimensions");
    T sum;
    for(int i=0; i<result.length(); i++)
        for(int j=0; j<result.width(); j++)
        {
            T* maskptr = mask.data();
            T* mptr = m[i]+j;
            sum = 0.0;
            int w=mask.width();

            for(int l=0; l<mask.length(); l++, maskptr += mask.mod(), mptr += m.mod())
                for(int c=0; c<w; c++)
                    sum += maskptr[c] * mptr[c];
            result(i,j) = sum;
        }
}

template<class T>
void subsample(TMat<T> m, int thesubsamplefactor, TMat<T> result)
{
    T sum;
    int norm = thesubsamplefactor * thesubsamplefactor;
    for(int i=0; i<result.length(); i++)
        for(int j=0; j<result.width(); j++)
        {
            T* mptr = m[thesubsamplefactor*i]+thesubsamplefactor*j;
            sum = 0.0;
            for(int l=0; l<thesubsamplefactor; l++, mptr += m.mod())
                for(int c=0; c<thesubsamplefactor; c++)
                    sum += mptr[c];
            result(i,j) = sum/norm;
        }
}


template<class T>
void classification_confusion_matrix(TMat<T> outputs, TMat<T> target_classes, TMat<T> confusion_matrix)
{
    int argmax, target;
    T v_max, tmp;

    for (int i=0; i<outputs.length(); i++) {
        // Find argmax(outputs)
        v_max = outputs(i,0);
        argmax = 0;
        for (int j=1; j<outputs.width(); ++j) {
            tmp = outputs(i,j);
            if (tmp > v_max) {
                argmax = j;
                v_max = tmp;
            }
        }
        // Update confusion matrix
        target = (int) target_classes(i,0);
        confusion_matrix(argmax, target) ++;
    }
}

//! Orthonormalize in-place the rows of the given matrix, using successive
//! projections on the orthogonal subspace of the previously found
//! basis. The resulting matrix has the following properties:
//!  - its rows spans the same space as A
//!  - its rows are orthogonal (dot product = 0)
//!  - its rows are of norm 1
//! However, it may happen that the original rows of A were not linearly
//! independent. In that case the, algorithm returns the number of rows
//! that were successfully obtained (and the user should probably
//! then do A = A.subMatRows(0,result) to obtain the basis).
//! The tolerance argument is the minimum value of the norm
//! of a row when projected orthogonal to the previous ones for this row
//! to contribute to the basis.
template<class T>
int GramSchmidtOrthogonalization(TMat<T> A, T tolerance=1e-6)
{
    int n_basis = 0;
    for (int i=0;i<A.length();i++)
    {
        TVec<T> Ai=A(i);
        if (n_basis!=i)
        {
            TVec<T> Ab = A(n_basis);
            Ab << Ai;
            Ai=Ab;
        }
        if (i>0)
            projectOnOrthogonalSubspace(Ai, A.subMatRows(0,n_basis));
        T normAi = norm(Ai);
        if (normAi>1e-6)
        {
            if (normAi!=1)
                Ai/=normAi;
            n_basis++;
        }
        // else ignore row i
    }
    return n_basis;
}

//!  products

//! return m x v
template<class T>
inline TVec<T> product(const TMat<T>& m, const TVec<T>& v)
{ TVec<T> res(m.length()); product(res, m,v); return res; }

//! return m' x v
template<class T>
inline TVec<T> transposeProduct(const TMat<T>& m, const TVec<T>& v)
{ TVec<T> res(m.width()); transposeProduct(res, m,v); return res; }

//! return m1 x m2
template<class T>
inline TMat<T> product(const TMat<T>& m1, const TMat<T>& m2)
{ TMat<T> res(m1.length(),m2.width()); product(res, m1,m2); return res; }

//! return m1' x m2
template<class T>
inline TMat<T> transposeProduct(const TMat<T>& m1, const TMat<T>& m2)
{ TMat<T> res(m1.width(),m2.width()); transposeProduct(res, m1,m2); return res; }

//! return m1 x m2'
template<class T>
inline TMat<T> productTranspose(const TMat<T>& m1, const TMat<T>& m2)
{ TMat<T> res(m1.length(),m2.length()); productTranspose(res, m1,m2); return res; }

//! return m + v (added to every ROW of m)
template<class T>
inline TMat<T> operator+(const TMat<T>& m, const TVec<T>& v)
{ TMat<T> res = m.copy(); res+=v; return res; }

//! return m - v (subtracted from every ROW of m)
template<class T>
inline TMat<T> operator-(const TMat<T>& m, const TVec<T>& v)
{ TMat<T> res = m.copy(); res-=v; return res; }

//!  does an elementwise multiplication of every row by v
template<class T>
inline TMat<T> operator*(const TMat<T>& m, const TVec<T>& v)
{ TMat<T> res = m.copy(); res*=v; return res; }

//!  elementwise division of every row by v
template<class T>
inline TMat<T> operator/(const TMat<T>& m, const TVec<T>& v)
{ TMat<T> res = m.copy(); res/=v; return res; }

//!  elementwise division of every row by v
template<class T>
inline TMat<T> operator/(const TMat<T>& m1, const TMat<T>& m2)
{ TMat<T> res = m1.copy(); res/=m2; return res; }

template<class T>
inline void choleskySolve(const TMat<T>& L, TVec<T> b, TVec<T> x) //!<  So that y be optional
{ TVec<T> y(b.size()); choleskySolve(L,b,x,y); }

//!  Same as above, but with a single value argument
template<class T>
inline TMat<T> grep(TMat<T> data, int col, T value, bool exclude=false)
{ return grep(data,col,TVec<T>(1,value),exclude); }

template<class T>
void addIfNonMissing(const TVec<T>& source, const TVec<int>& nnonmissing, TVec<T> destination)
{
#ifdef BOUNDCHECK
    if (source.length()!=nnonmissing.length() || source.length()!=destination.length())
        PLERROR("addIfNonMissing: all arguments should have the same length, got %d,%d,%d\n",
                source.length(),nnonmissing.length(),destination.length());
#endif
    T* s=source.data();
    T* d=destination.data();
    int* n=nnonmissing.data();
    int size=source.length();
    for (int i=0;i<size;i++)
        if (finite(s[i]))
        {
            d[i] += s[i];
            n[i]++;
        }
}

template<class T>
void addXandX2IfNonMissing(const TVec<T>& source, const TVec<int>& nnonmissing, TVec<T> somme, TVec<T> somme2)
{
#ifdef BOUNDCHECK
    if (source.length()!=nnonmissing.length() || source.length()!=somme.length() || source.length()!=somme2.length())
        PLERROR("addIfNonMissing: all arguments should have the same length, got %d,%d,%d,%d\n",
                source.length(),nnonmissing.length(),somme.length(),somme2.length());
#endif
    T* s=source.data();
    T* s1=somme.data();
    T* s2=somme.data();
    int* n=nnonmissing.data();
    int size=source.length();
    for (int i=0;i<size;i++)
        if (finite(s[i]))
        {
            s1[i] += s[i];
            s2[i] += s[i]*s[i];
            n[i]++;
        }
}

// input_gradient[j] = sum_i weights[i,j]*output_gradient[i]
// weights[i,j] -= learning_rate * output_gradient[i] * input[j]
template<class T>
void layerBpropUpdate(TVec<T> input_gradient, TMat<T> weights, const TVec<T>& input,
                      const TVec<T>& output_gradient, real learning_rate)
{
    int n_inputs = input_gradient.length();
    int n_outputs = output_gradient.length();
#ifdef BOUNDCHECK
    if (weights.length() != n_outputs || weights.width() != n_inputs
        || input.length() != n_inputs)
        PLERROR("layerBpropUpdate: arguments have incompatible sizes");
#endif
    input_gradient.clear();
    T* in_g = input_gradient.data();
    T* out_g = output_gradient.data();
    T* inp = input.data();
    for (int i=0;i<n_outputs;i++)
    {
        T* Wi = weights[i];
        T out_gi = out_g[i];
        for (int j=0;j<n_inputs;j++)
        {
            in_g[j] += Wi[j] * out_gi;
            Wi[j] -= learning_rate * out_gi * inp[j];
        }
    }
}


// input_gradient[j] = sum_i weights[i,j]*output_gradient[i]
// weights[i,j] -= learning_rate * (output_gradient[i] * input[j] + weight_decay * weights[i,j])
template<class T>
void layerL2BpropUpdate(TVec<T> input_gradient, TMat<T> weights, const TVec<T>& input,
                        const TVec<T>& output_gradient, real learning_rate, T weight_decay)
{
    int n_inputs = input_gradient.length();
    int n_outputs = output_gradient.length();
#ifdef BOUNDCHECK
    if (weights.length() != n_outputs || weights.width() != n_inputs
        || input.length() != n_inputs)
        PLERROR("layerL2BpropUpdate: arguments have incompatible sizes");
#endif
    input_gradient.clear();
    T* in_g = input_gradient.data();
    T* out_g = output_gradient.data();
    T* inp = input.data();
    for (int i=0;i<n_outputs;i++)
    {
        T* Wi = weights[i];
        T out_gi = out_g[i];
        for (int j=0;j<n_inputs;j++)
        {
            T Wij = Wi[j];
            in_g[j] += Wij * out_gi;
            Wi[j] -= learning_rate * (out_gi * inp[j] + weight_decay * Wij);
        }
    }
}

// like layerL2BpropUpdate but weights is given transposed (not reflected in the formula below).
// input_gradient[j] = sum_i weights[j,i]*output_gradient[i]
// weights[i,j] -= learning_rate * (output_gradient[i] * input[j] + weight_decay * weights[i,j])
template<class T>
void transposedLayerL2BpropUpdate(TVec<T> input_gradient, TMat<T> weights, const TVec<T>& input,
                                  const TVec<T>& output_gradient, real learning_rate, T weight_decay)
{
    int n_inputs = input_gradient.length();
    int n_outputs = output_gradient.length();
#ifdef BOUNDCHECK
    if (weights.width() != n_outputs || weights.length() != n_inputs
        || input.length() != n_inputs)
        PLERROR("layerL2BpropUpdate: arguments have incompatible sizes");
#endif
    input_gradient.clear();
    T* in_g = input_gradient.data();
    T* out_g = output_gradient.data();
    T* inp = input.data();
    for (int j=0;j<n_inputs;j++)
    {
        T* Wj = weights[j];
        T inp_j = inp[j];
        for (int i=0;i<n_outputs;i++)
        {
            T out_gi = out_g[i];
            T Wji = Wj[i];
            in_g[j] += Wji * out_gi;
            Wj[i] -= learning_rate * (out_gi * inp_j + weight_decay * Wji);
        }
    }
}

// input_gradient[j] = sum_i weights[i,j]*output_gradient[i]
// weights[i,j] -= learning_rate * (output_gradient[i] * input[j] + weight_decay * sign(weights[i,j]))
template<class T>
void layerL1BpropUpdate(TVec<T> input_gradient, TMat<T> weights, const TVec<T>& input,
                        const TVec<T>& output_gradient, real learning_rate, T weight_decay)
{
    int n_inputs = input_gradient.length();
    int n_outputs = output_gradient.length();
#ifdef BOUNDCHECK
    if (weights.length() != n_outputs || weights.width() != n_inputs
        || input.length() != n_inputs)
        PLERROR("layerL1BpropUpdate: arguments have incompatible sizes");
#endif
    input_gradient.clear();
    T* in_g = input_gradient.data();
    T* out_g = output_gradient.data();
    T* inp = input.data();
    for (int i=0;i<n_outputs;i++)
    {
        T* Wi = weights[i];
        T out_gi = out_g[i];
        for (int j=0;j<n_inputs;j++)
        {
            T Wij = Wi[j];
            in_g[j] += Wij * out_gi;
            Wi[j] -= learning_rate * (out_gi * inp[j] + weight_decay * sign(Wij));
        }
    }
}

// like layerL1BpropUpdate but weights is given transposed.
// input_gradient[j] = sum_i weights[j,i]*output_gradient[i]
// weights[i,j] -= learning_rate * (output_gradient[i] * input[j] + weight_decay * sign(weights[i,j]))
template<class T>
void transposedLayerL1BpropUpdate(TVec<T> input_gradient, TMat<T> weights, const TVec<T>& input,
                                  const TVec<T>& output_gradient, real learning_rate, T weight_decay)
{
    int n_inputs = input_gradient.length();
    int n_outputs = output_gradient.length();
#ifdef BOUNDCHECK
    if (weights.width() != n_outputs || weights.length() != n_inputs
        || input.length() != n_inputs)
        PLERROR("layerL1BpropUpdate: arguments have incompatible sizes");
#endif
    input_gradient.clear();
    T* in_g = input_gradient.data();
    T* out_g = output_gradient.data();
    T* inp = input.data();
    for (int j=0;j<n_inputs;j++)
    {
        T* Wj = weights[j];
        T inp_j = inp[j];
        for (int i=0;i<n_outputs;i++)
        {
            T out_gi = out_g[i];
            T Wji = Wj[i];
            in_g[j] += Wji * out_gi;
            Wj[i] -= learning_rate * (out_gi * inp_j + weight_decay * sign(Wji));
        }
    }
}

//! set m to the identity matrix, more precisely set m(i,j) = 1_{i==j}
//! (works also for non-square matrices)
template<class T>
void identityMatrix(TMat<T> m)
{
    int l=m.length();
    int w=m.width();
    for (int i=0;i<l;i++)
    {
        T* mi = m[i];
        for (int j=0;j<w;j++)
            if (j==i)
                mi[j]=1;
            else
                mi[j]=0;
    }
}

//! Return the identity matrix, more precisely an n x n or n x m matrix with result(i,j) = 1_{i==j}
template<class T>
TMat<T> identityMatrix(int n, int m=-1)
{
    if (m<0) m=n;
    TMat<T> result(n,m);
    identityMatrix(result);
    return result;
}


} // end of namespace PLearn


// Norman: replaced the code below with this wrapper
SET_HASH_FUNCTION(PLearn::TVec<T>, T, v, sumsquare(v))
    SET_HASH_WITH_FUNCTION(PLearn::Vec, v, sumsquare(v))

//#if __GNUC__==3 && __GNUC_MINOR__>0
//namespace __gnu_cxx {
//#else
//namespace std {
//#endif
//
//template<class T>
//struct hash<PLearn::TVec<T> >
//{
//    size_t operator()(PLearn::TVec<T> v) const { return hash<T>()(sumsquare(v));}
//};

//} // end of namespace std


#endif // TMat_maths_impl_H


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
