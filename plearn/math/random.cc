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
   * $Id: random.cc,v 1.1 2002/07/30 09:01:27 plearner Exp $
   ******************************************************* */

extern "C" {
#include <ctime>
}

#include "general.h"
#include "random.h"

namespace PLearn <%
using namespace std;


/*  
  The static data to store the seed used by the random number generators.
  */

static long  the_seed=0;
static int   iset=0;
static real gset;

/*  
  Special functions.
  =================
  */

/*  
  log_gamma(): 
  returns the natural logarithm of the gamma function
  */

real  log_gamma(real xx)
{
  double x,y,tmp,ser;
  static double gamma_coeffs[6]={ 76.18009172947146     ,
                        -86.50532032941677     ,
	                 24.01409824083091     ,
                         -1.231739572450155    ,
		          0.1208650973866179e-2,
                         -0.5395239384953e-5   };
  int j;

  y=x=xx;
  tmp=x+5.5;
  tmp -= (x+0.5)*log(tmp);
  ser=1.000000000190015;
  for (j=0;j<=5;j++) ser += gamma_coeffs[j]/++y;
  return -tmp+log(2.5066282746310005*ser/x);
}

/*   
  Utilities for random numbers generation. 
  =======================================
  */

/*  
  manual_seed(): gives a seed for random number generators.

  Rem: - The stored value is negative.
  */

void  manual_seed(long x)
{
  the_seed = - labs(x);
  iset     = 0;
}

/*  
  seed(): generates a seed for random number generators, using the cpu time.
  */

void  seed()
{
  time_t  ltime;
  struct  tm *today;
  time(&ltime);
  today = localtime(&ltime);
  manual_seed((long)today->tm_sec+
                    60*today->tm_min+
                    60*60*today->tm_hour+
                    60*60*24*today->tm_mday);
}

/*  
  get_seed(): returns the current value of the 'seed'.
  */

long  get_seed()
{
  long seed = the_seed;
  return seed;
}

/*  
  Constants used by the 'numerical recipes' random number generators.
  */

#define NTAB 32                 /*   needs for ran1 & ran2   */
#define EPS 1.2e-7              /*   needs for ran1 & ran2   */
#define RNMX (1.0-EPS)          /*   needs for ran1 & ran2   */
#define IM1 2147483563          /*   needs for ran2          */
#define IM2 2147483399          /*   needs for ran2          */
#define AM1 (1.0/IM1)           /*   needs for ran2          */
#define IMM1 (IM1-1)            /*   needs for ran2          */
#define IA1 40014               /*   needs for ran2          */
#define IA2 40692               /*   needs for ran2          */
#define IQ1 53668               /*   needs for ran2          */
#define IQ2 52774               /*   needs for ran2          */
#define IR1 12211               /*   needs for ran2          */
#define IR2 3791                /*   needs for ran2          */
#define NDIV1 (1+IMM1/NTAB)     /*   needs for ran2          */

/*  
  ran2(): long period ramdom number generator from the 'numerical recipes'.

  Rem: - It is a long period (> 2 x 10^18) random number generator of L'Ecuyer
         with Bays-Durham shuffle and added safeguards.

       - Returns a uniform random deviate between 0.0 and 1.0
         (exclusive of the endpoint values).

       - Initilized with a negative seed.
  */

real uniform_sample()  
{
  int j;
  long k;
  static long idum2=123456789;
  static long iy=0;
  static long iv[NTAB];
  real temp;

  if (the_seed <= 0) {
    if (-the_seed < 1) the_seed=1;
    else the_seed = -the_seed;
    idum2=the_seed;
    for (j=NTAB+7;j>=0;j--) {
      k=the_seed/IQ1;
      the_seed=IA1*(the_seed-k*IQ1)-k*IR1;
      if (the_seed < 0) the_seed += IM1;
      if (j < NTAB) iv[j] = the_seed;
    }
    iy=iv[0];
  }
  k=the_seed/IQ1;
  the_seed=IA1*(the_seed-k*IQ1)-k*IR1;
  if (the_seed < 0) the_seed += IM1;
  k=idum2/IQ2;
  idum2=IA2*(idum2-k*IQ2)-k*IR2;
  if (idum2 < 0) idum2 += IM2;
  j=iy/NDIV1;
  iy=iv[j]-idum2;
  iv[j] = the_seed;
  if (iy < 1) iy += IMM1;
  if ((temp=AM1*iy) > RNMX) return RNMX;
  else return temp;
}

/*  
  bounded_uniform(): return an uniform random generator in [a,b].
  */

real  bounded_uniform(real a,real b)
{
  real res = uniform_sample()*(b-a) + a;
  if (res >= b) return b*RNMX;
  else return res;
}

#undef NDIV
#undef EPS
#undef RNMX
#undef IM1
#undef IM2
#undef AM1
#undef IMM1
#undef IA1
#undef IA2
#undef IQ1
#undef IQ2
#undef IR1
#undef IR2
#undef NDIV1

/*  
  TRANSFORMATION METHOD:
  ---------------------
  */

/*  
  expdev(): exponential deviate random number from the 'numerical recipes'.
  */

real  expdev()
{
  real dum;

  do
    dum=uniform_sample();
  while (dum == 0.0);
  return -log(dum);
}

real gaussian_01() 
{
  real fac,rsq,v1,v2;

  if(the_seed < 0) iset=0;
  if (iset == 0) {
    do {
      v1=2.0*uniform_sample()-1.0;
      v2=2.0*uniform_sample()-1.0;
      rsq=v1*v1+v2*v2;
    } while (rsq >= 1.0 || rsq == 0.0);
    fac=sqrt(-2.0*log(rsq)/rsq);
    gset=v1*fac;
    iset=1;
    return v2*fac;
  } else {
    iset=0;
    return gset;
  }
}

/*  
  gaussian_mu_sigma(): returns a gaussian distributed random number
                       with mean 'mu' and standard deviation 'sigma'.

  Rem: - i.e. N(mu,sigma).
  */

real  gaussian_mu_sigma(real mu, real sigma)
{
  return gaussian_01() * sigma + mu;
}


/*  
  gaussian_misture_mu_sigma(): returns a random number with mixture of gaussians,
                               'w' is the mixture (must be positive summing to 1).

  Rem: - i.e. SUM w[i] * N(mu[i],sigma[i])
  */

real  gaussian_mixture_mu_sigma(Vec& w, Vec& mu, Vec& sigma)
{
  int    i;
  int    n = w.length();
  real *p_mu = mu.data();
  real *p_sigma = sigma.data();
  real *p_w = w.data();
  real  res = 0.0;

  for (i=0; i<n; i++, p_mu++, p_sigma++, p_w++)
    res += *p_w * gaussian_mu_sigma(*p_mu,*p_sigma);

  return res;
}

/*  
  REJECTION METHOD:
  ----------------
  */

/*  
  gamdev(): returns a deviate distributed as a gamma distribution from the 'numerical recipes'.
  */

real  gamdev(int ia)
{
  int j;
  real am,e,s,v1,v2,x,y;

  if (ia < 1) PLERROR("Error in routine gamdev");
  if (ia < 6) {
    x=1.0;
    for (j=1;j<=ia;j++) x *= uniform_sample();
    x = -log(x);
  } else {
    do {
      do {
        do {
	  v1=uniform_sample();
	  v2=2.0*uniform_sample()-1.0;
	} while (v1*v1+v2*v2 > 1.0);
	y=v2/v1;
	am=ia-1;
	s=sqrt(2.0*am+1.0);
	x=s*y+am;
      } while (x <= 0.0);
      e=(1.0+y*y)*exp(am*log(x/am)-s*y);
    } while (uniform_sample() > e);
  }
  return x;
}

/*  
  poidev(): returns a deviate distributed as a poisson distribution of mean (lambda) 'xm'
            from the 'numerical recipes'.
  */

real  poidev(real xm)
{
  static real sq,alxm,g,oldm=(-1.0);
  real em,t,y;

  if (xm < 12.0) {
    if (xm != oldm) {
      oldm=xm;
      g=exp(-xm);
    }
    em = -1;
    t=1.0;
    do {
      ++em;
      t *= uniform_sample();
    } while (t > g);
  } else {
    if (xm != oldm) {
      oldm=xm;
      sq=sqrt(2.0*xm);
      alxm=log(xm);
      g=xm*alxm-log_gamma(xm+1.0);
    }
    do {
      do {
        y=tan(Pi*uniform_sample());
	em=sq*y+xm;
      } while (em < 0.0);
      em=floor(em);
      t=0.9*(1.0+y*y)*exp(em*alxm-log_gamma(em+1.0)-g);
    } while (uniform_sample() > t);
  }
  return em;
}

/*  
  bnldev(): return a random deviate drawn from a binomial distribution of 'n' trials
            each of probability 'pp', from 'numerical recipes'.

  Rem: - The returned type is an real although a binomial random variable is an integer.
  */

real  bnldev(real pp, int n)
{
  int j;
  static int nold=(-1);
  real am,em,g,angle,p,bnl,sq,t,y;
  static real pold=(-1.0),pc,plog,pclog,en,oldg;

  p=(pp <= 0.5 ? pp : 1.0-pp);
  am=n*p;
  if (n < 25) {
    bnl=0.0;
    for (j=1;j<=n;j++)
      if (uniform_sample() < p) ++bnl;
  } else if (am < 1.0) {
    g=exp(-am);
    t=1.0;
    for (j=0;j<=n;j++) {
      t *= uniform_sample();
      if (t < g) break;
    }
    bnl=(j <= n ? j : n);
  } else {
    if (n != nold) {
      en=n;
      oldg=log_gamma(en+1.0);
      nold=n;
    } if (p != pold) {
      pc=1.0-p;
      plog=log(p);
      pclog=log(pc);
      pold=p;
    }
    sq=sqrt(2.0*am*pc);
    do {
      do {
        angle=Pi*uniform_sample();
	y=tan(angle);
	em=sq*y+am;
      } while (em < 0.0 || em >= (en+1.0));
      em=floor(em);
      t=1.2*sq*(1.0+y*y)*exp(oldg-log_gamma(em+1.0)
        -log_gamma(en-em+1.0)+em*plog+(en-em)*pclog);
    } while (uniform_sample() > t);
    bnl=em;
  }
  if (p != pp) bnl=n-bnl;
  return bnl;
}

/*  
  SOME KIND OF DISCRETE DISTRIBUTIONS:
  -----------------------------------
  */

/*  
  multinomial_sample(): returns a random deviate from a discrete distribution
                       given explicitely by 'distribution'.

  Rem: - So, the vector elements of 'distribution' are probabilities summing to 1.

       - The returned value is a index value of 'distribution' (i.e. in range
         [0 .. (distribution->lenght)-1] ).

       - The graphical representation of vectors 'distribution' is histogram.

       - This random deviate is computed by the transformation method.
  */

int  multinomial_sample(Vec& distribution)
{
  real  u  = uniform_sample();
  real* pi = distribution.data();
  real  s  = *pi;
  int    n  = distribution.length();
  int    i  = 0;
  while ((i<n) && (s<u)) {
    i++;
    pi++;
    s += *pi;
  }
  if (i==n)
    i = n - 1; /*   improbable but...   */
  return i;
}

int uniform_multinomial_sample(int N)
{
  // N.B. uniform_sample() cannot return 1.0
  return int(N*uniform_sample());
}

//!  sample each element from uniform distribution U[minval,maxval]
void fill_random_uniform(const Vec& dest, real minval, real maxval)
{
  Vec::iterator it = dest.begin();
  Vec::iterator itend = dest.end();  
  double scale = maxval-minval;
  for(; it!=itend; ++it)
    *it = real(uniform_sample()*scale+minval);
}

//!  sample each element from Normal(mean,sdev^2) distribution
void fill_random_normal(const Vec& dest, real mean, real stdev)
{
  Vec::iterator it = dest.begin();
  Vec::iterator itend = dest.end();  
  for(; it!=itend; ++it)
    *it = real(gaussian_mu_sigma(mean,stdev));
}

//!  sample each element from multivariate Normal(mean,diag(sdev^2)) distribution
void fill_random_normal(const Vec& dest, const Vec& mean, const Vec& stdev)
{
#ifdef BOUNDCHECK
  if(mean.length()!=dest.length() || stdev.length()!=dest.length())
    PLERROR("In fill_random_normal: dest, mean and stdev must have the same length");
#endif
  Vec::iterator it_mean = mean.begin();
  Vec::iterator it_stdev = stdev.begin();
  Vec::iterator it = dest.begin();
  Vec::iterator itend = dest.end();  
  for(; it!=itend; ++it, ++it_mean, ++it_stdev)
    *it = real(gaussian_mu_sigma(*it_mean,*it_stdev));
}


void fill_random_uniform(const Mat& dest, real minval, real maxval)
{ 
  double scale = maxval-minval;
  Mat::iterator it = dest.begin();
  Mat::iterator itend = dest.end();
  for(; it!=itend; ++it)
    *it = real(uniform_sample()*scale+minval); 
}

void fill_random_normal(const Mat& dest, real mean, real sdev)
{ 
  Mat::iterator it = dest.begin();
  Mat::iterator itend = dest.end();
  for(; it!=itend; ++it)
    *it = real(gaussian_mu_sigma(mean,sdev));
}


%> // end of namespace PLearn
