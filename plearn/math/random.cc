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
 ******************************************************* */

extern "C" {
#include <ctime>
}

#include <plearn/base/general.h>
#include "random.h"

namespace PLearn {
using namespace std;


/*  
    The static data to store the seed used by the random number generators.
*/

static int32_t  the_seed=0;
static int      iset=0;
static real gset;

/*  
    Special functions.
    =================
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
    tmp -= (x+0.5)*pl_log(tmp);
    ser=1.000000000190015;
    for (j=0;j<=5;j++) ser += gamma_coeffs[j]/++y;
    return -tmp+pl_log(2.5066282746310005*ser/x);
}

real log_beta(real x, real y)
{
    return log_gamma(x) + log_gamma(y) - log_gamma(x+y);
}

real incomplete_beta_continued_fraction(real z, real x, real y)
{
    real x_minus_1 = x-1;
    real x_plus_1 = x+1;
    real x_plus_y = x+y;
    real denom = -z*x_plus_y/x_plus_1+1;
    if (fabs(denom)<1e-35) {
        denom=1e-35;
    }
    real rat1=1/denom;
    real rat2=1.0;
    real frac=rat1;
    for (int k=1;k<100;k++)
    {
        real f=z*k*(y-k)/((x+2*k)*(x_minus_1+2*k));
        rat2 = f/rat2 + 1;
        rat1 = rat1*f+1;
        if (fabs(rat1)<1e-35) {
            rat1=1e-35;
        }
        if (fabs(rat2)<1e-35) {
            rat2=1e-35;
        }
        rat1=1/rat1;
        frac *= rat1*rat2;

        f=-z*(x+k)*(x_plus_y+k)/((x_plus_1+2*k)*(x+2*k));
        rat2 = f/rat2+ 1;
        rat1 = rat1*f+1;
        if (fabs(rat1)<1e-35) {
            rat1=1e-35;
        }
        if (fabs(rat2)<1e-35) {
            rat2=1e-35;
        }
        rat1=1/rat1;

        real delta = rat1*rat2;
        frac *= delta;
        // stopping criterion
        if (fabs(1-delta) < 2e-7) {
            return frac;
        }
    }
    // If that happens, increase the number of k iterations or increase
    // the stopping criterion tolerance.
    PLWARNING("incomplete_beta_continued_fraction: insufficient precision!"); 
    return frac;
}

real incomplete_beta(real z, real x, real y)
{
    if (z>1 || z<0) PLERROR("incomplete_beta(z,x,y): z =%f must be in [0,1]",z);
    real coeff = 0;
    if (z>0 && z<1) coeff = exp(x*pl_log(z)+y*pl_log(1.-z)-log_beta(x,y));
    if (z*(x+y+2)<x+1) {
        return coeff*incomplete_beta_continued_fraction(z,x,y)/x;
    }
    return 1-coeff*incomplete_beta_continued_fraction(1-z,y,x)/y;
}

real student_t_cdf(real t, int nb_degrees_of_freedom)
{
    real p_t = 0.5*incomplete_beta(nb_degrees_of_freedom/(nb_degrees_of_freedom+t*t),0.5*nb_degrees_of_freedom,0.5);
    //real p_t = 0.5*incbet(0.5*nb_degrees_of_freedom,0.5,nb_degrees_of_freedom/(nb_degrees_of_freedom+t*t));
#ifdef BOUNDCHECK
    if (p_t < 0) {
        PLERROR("Bug in incomplete_beta : returned a negative p_t !\n- p_t = %f\n- degrees of freedom = %d\n- t = %f",
                p_t, nb_degrees_of_freedom, t);
    }
#endif
    if (t>0)
        return 1.0 - p_t;
    else
        return p_t;
}

/*   
     Utilities for random numbers generation. 
     =======================================
*/

/*  
    manual_seed(): gives a seed for random number generators.

    Rem: - The stored value is negative.
*/

void  manual_seed(int32_t x)
{
    the_seed = - labs(x);
    iset     = 0;
}

void  seed()
{
    time_t  ltime;
    struct  tm *today;
    time(&ltime);
    today = localtime(&ltime);
    manual_seed((int32_t)today->tm_sec+
                60*today->tm_min+
                60*60*today->tm_hour+
                60*60*24*today->tm_mday);
}

int32_t get_seed()
{
    int32_t seed = the_seed;
    return seed;
}

/*  
    Constants used by the 'numerical recipes' random number generators.
*/

#define NTAB 32                 /*   needs for ran1 & uniform_sample()   */
#define EPS 1.2e-7              /*   needs for ran1 & uniform_sample()   */
#define RNMX (1.0-EPS)          /*   needs for ran1 & uniform_sample()   */
#define IM1 2147483563          /*   needs for uniform_sample()          */
#define IM2 2147483399          /*   needs for uniform_sample()          */
#define AM1 (1.0/IM1)           /*   needs for uniform_sample()          */
#define IMM1 (IM1-1)            /*   needs for uniform_sample()          */
#define IA1 40014               /*   needs for uniform_sample()          */
#define IA2 40692               /*   needs for uniform_sample()          */
#define IQ1 53668               /*   needs for uniform_sample()          */
#define IQ2 52774               /*   needs for uniform_sample()          */
#define IR1 12211               /*   needs for uniform_sample()          */
#define IR2 3791                /*   needs for uniform_sample()          */
#define NDIV1 (1+IMM1/NTAB)     /*   needs for uniform_sample()          */

real uniform_sample()  
{
    int j;
    int32_t k;
    static int32_t idum2=123456789;
    static int32_t iy=0;
    static int32_t iv[NTAB];
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
    j=int(iy/NDIV1);
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
    while (fast_exact_is_equal(dum, 0.0));
    return -pl_log(dum);
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
        } while (rsq >= 1.0 || fast_exact_is_equal(rsq, 0.0));
        fac=sqrt(-2.0*pl_log(rsq)/rsq);
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

real  gaussian_mixture_mu_sigma(Vec& w, const Vec& mu, const Vec& sigma)
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

real  gamdev(int ia)
{
    int j;
    real am,e,s,v1,v2,x,y;

    if (ia < 1) PLERROR("Error in routine gamdev");
    if (ia < 6) {
        x=1.0;
        for (j=1;j<=ia;j++) x *= uniform_sample();
        x = -pl_log(x);
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
            e=(1.0+y*y)*exp(am*pl_log(x/am)-s*y);
        } while (uniform_sample() > e);
    }
    return x;
}

real  poidev(real xm)
{
    static real sq,alxm,g,oldm=(-1.0);
    real em,t,y;

    if (xm < 12.0) {
        if (!fast_exact_is_equal(xm, oldm)) {
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
        if (!fast_exact_is_equal(xm, oldm)) {
            oldm=xm;
            sq=sqrt(2.0*xm);
            alxm=pl_log(xm);
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
        } if (!fast_exact_is_equal(p, pold)) {
            pc=1.0-p;
            plog=pl_log(p);
            pclog=pl_log(pc);
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
    if (!fast_exact_is_equal(p, pp)) bnl=n-bnl;
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

int  multinomial_sample(const Vec& distribution)
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

void fill_random_uniform(const Vec& dest, real minval, real maxval)
{
    Vec::iterator it = dest.begin();
    Vec::iterator itend = dest.end();  
    double scale = maxval-minval;
    for(; it!=itend; ++it)
        *it = real(uniform_sample()*scale+minval);
}

void fill_random_discrete(const Vec& dest, const Vec& set)
{
    Vec::iterator it = dest.begin();
    Vec::iterator itend = dest.end();  
    int n=set.length();
    for(; it!=itend; ++it)
        *it = set[uniform_multinomial_sample(n)];
}

void fill_random_normal(const Vec& dest, real mean, real stdev)
{
    Vec::iterator it = dest.begin();
    Vec::iterator itend = dest.end();  
    for(; it!=itend; ++it)
        *it = real(gaussian_mu_sigma(mean,stdev));
}

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

/*							incbet.c
 *
 *	Incomplete beta integral
 *
 *
 * SYNOPSIS:
 *
 * double a, b, x, y, incbet();
 *
 * y = incbet( a, b, x );
 *
 *
 * DESCRIPTION:
 *
 * Returns incomplete beta integral of the arguments, evaluated
 * from zero to x.  The function is defined as
 *
 *                  x
 *     -            -
 *    | (a+b)      | |  a-1     b-1
 *  -----------    |   t   (1-t)   dt.
 *   -     -     | |
 *  | (a) | (b)   -
 *                 0
 *
 * The domain of definition is 0 <= x <= 1.  In this
 * implementation a and b are restricted to positive values.
 * The integral from x to 1 may be obtained by the symmetry
 * relation
 *
 *    1 - incbet( a, b, x )  =  incbet( b, a, 1-x ).
 *
 * The integral is evaluated by a continued fraction expansion
 * or, when b*x is small, by a power series.
 *
 * ACCURACY:
 *
 * Tested at uniformly distributed random points (a,b,x) with a and b
 * in "domain" and x between 0 and 1.
 *                                        Relative error
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0,5         10000       6.9e-15     4.5e-16
 *    IEEE      0,85       250000       2.2e-13     1.7e-14
 *    IEEE      0,1000      30000       5.3e-12     6.3e-13
 *    IEEE      0,10000    250000       9.3e-11     7.1e-12
 *    IEEE      0,100000    10000       8.7e-10     4.8e-11
 * Outputs smaller than the IEEE gradual underflow threshold
 * were excluded from these statistics.
 *
 * ERROR MESSAGES:
 *   message         condition      value returned
 * incbet domain      x<0, x>1          0.0
 * incbet underflow                     0.0
 */


/*
  Cephes Math Library, Release 2.8:  June, 2000
  Copyright 1984, 1995, 2000 by Stephen L. Moshier
*/

//#include "mconf.h"

#define MAXGAM 171.624376956302725

/*
  extern double MACHEP, MINLOG, MAXLOG;
  #ifdef ANSIPROT
  extern double gamma ( double );
  extern double lgam ( double );
  extern double exp ( double );
  extern double log ( double );
  extern double pow ( double, double );
  extern double fabs ( double );
  static double incbcf(double, double, double);
  static double incbd(double, double, double);
  static double pseries(double, double, double);
  #else
  double gamma(), lgam(), exp(), log(), pow(), fabs();
  static double incbcf(), incbd(), pseries();
  #endif

*/
double MAXLOG =  7.09782712893383996732E2;     /* log(MAXNUM) */
double MINLOG = -7.451332191019412076235E2;     /* log(2**-1075) */
double MACHEP =  1.11022302462515654042E-16;   /* 2**-53 */
//double pseries( double a, double b, double x );
// double incbcf( double a, double b, double x ); // Does not seem to be used anymore.
// double incbd( double a, double b, double x );  // Does not seem to be used anymore.
double big = 4.503599627370496e15;
double biginv =  2.22044604925031308085e-16;


/*
  double incbet(double aa, double bb, double xx )
  {
  double a, b, t, x, xc, w, y;
  int flag;

  if( aa <= 0.0 || bb <= 0.0 )
  goto domerr;

  if( (xx <= 0.0) || ( xx >= 1.0) )
  {
  if( xx == 0.0 )
  return(0.0);
  if( xx == 1.0 )
  return( 1.0 );
  domerr:
  PLERROR("incbet: arguments out of expected domain");
  return( 0.0 );
  }

  flag = 0;
  if( (bb * xx) <= 1.0 && xx <= 0.95)
  {
  t = pseries(aa, bb, xx);
  goto done;
  }

  w = 1.0 - xx;

// Reverse a and b if x is greater than the mean.
if( xx > (aa/(aa+bb)) )
{
flag = 1;
a = bb;
b = aa;
xc = xx;
x = w;
}
else
{
    a = aa;
    b = bb;
    xc = w;
    x = xx;
}

if( flag == 1 && (b * x) <= 1.0 && x <= 0.95)
{
    t = pseries(a, b, x);
    goto done;
}

// Choose expansion for better convergence.
y = x * (a+b-2.0) - (a-1.0);
if( y < 0.0 )
    w = incbcf( a, b, x );
else
w = incbd( a, b, x ) / xc;

// Multiply w by the factor
//   a      b   _             _     _
//  x  (1-x)   | (a+b) / ( a | (a) | (b) ) .

y = a * log(x);
t = b * log(xc);
if( (a+b) < MAXGAM && fabs(y) < MAXLOG && fabs(t) < MAXLOG )
{
    t = pow(xc,b);
    t *= pow(x,a);
    t /= a;
    t *= w;
    t *= gamma(a+b) / (gamma(a) * gamma(b));
    goto done;
}
// Resort to logarithms.
y += t + log_gamma(a+b) - log_gamma(a) - log_gamma(b);
y += log(w/a);
if( y < MINLOG )
    t = 0.0;
else
t = exp(y);

done:

if( flag == 1 )
{
    if( t <= MACHEP )
        t = 1.0 - MACHEP;
    else
        t = 1.0 - t;
}
return( t );
}
*/

/* Continued fraction expansion #1
 * for incomplete beta integral
 */

  double incbcf( double a, double b, double x )
{
    double xk, pk, pkm1, pkm2, qk, qkm1, qkm2;
    double k1, k2, k3, k4, k5, k6, k7, k8;
    double r, t, ans, thresh;
    int n;

    k1 = a;
    k2 = a + b;
    k3 = a;
    k4 = a + 1.0;
    k5 = 1.0;
    k6 = b - 1.0;
    k7 = k4;
    k8 = a + 2.0;

    pkm2 = 0.0;
    qkm2 = 1.0;
    pkm1 = 1.0;
    qkm1 = 1.0;
    ans = 1.0;
    r = 1.0;
    n = 0;
    thresh = 3.0 * MACHEP;
    do
    {
	
	xk = -( x * k1 * k2 )/( k3 * k4 );
	pk = pkm1 +  pkm2 * xk;
	qk = qkm1 +  qkm2 * xk;
	pkm2 = pkm1;
	pkm1 = pk;
	qkm2 = qkm1;
	qkm1 = qk;

	xk = ( x * k5 * k6 )/( k7 * k8 );
	pk = pkm1 +  pkm2 * xk;
	qk = qkm1 +  qkm2 * xk;
	pkm2 = pkm1;
	pkm1 = pk;
	qkm2 = qkm1;
	qkm1 = qk;

	if( !fast_exact_is_equal(qk, 0) )
            r = pk/qk;
	if( !fast_exact_is_equal(r, 0) )
        {
            t = fabs( (ans - r)/r );
            ans = r;
        }
	else
            t = 1.0;

	if( t < thresh )
            goto cdone;

	k1 += 1.0;
	k2 += 1.0;
	k3 += 2.0;
	k4 += 2.0;
	k5 += 1.0;
	k6 -= 1.0;
	k7 += 2.0;
	k8 += 2.0;

	if( (fabs(qk) + fabs(pk)) > big )
        {
            pkm2 *= biginv;
            pkm1 *= biginv;
            qkm2 *= biginv;
            qkm1 *= biginv;
        }
	if( (fabs(qk) < biginv) || (fabs(pk) < biginv) )
        {
            pkm2 *= big;
            pkm1 *= big;
            qkm2 *= big;
            qkm1 *= big;
        }
    }
    while( ++n < 300 );

 cdone:
    return(ans);
}

/* Continued fraction expansion #2
 * for incomplete beta integral
 */

/* Does not seem to be used anymore.
   double incbd( double a, double b, double x )
   {
   double xk, pk, pkm1, pkm2, qk, qkm1, qkm2;
   double k1, k2, k3, k4, k5, k6, k7, k8;
   double r, t, ans, z, thresh;
   int n;

   k1 = a;
   k2 = b - 1.0;
   k3 = a;
   k4 = a + 1.0;
   k5 = 1.0;
   k6 = a + b;
   k7 = a + 1.0;;
   k8 = a + 2.0;

   pkm2 = 0.0;
   qkm2 = 1.0;
   pkm1 = 1.0;
   qkm1 = 1.0;
   z = x / (1.0-x);
   ans = 1.0;
   r = 1.0;
   n = 0;
   thresh = 3.0 * MACHEP;
   do
   {
	
   xk = -( z * k1 * k2 )/( k3 * k4 );
   pk = pkm1 +  pkm2 * xk;
   qk = qkm1 +  qkm2 * xk;
   pkm2 = pkm1;
   pkm1 = pk;
   qkm2 = qkm1;
   qkm1 = qk;

   xk = ( z * k5 * k6 )/( k7 * k8 );
   pk = pkm1 +  pkm2 * xk;
   qk = qkm1 +  qkm2 * xk;
   pkm2 = pkm1;
   pkm1 = pk;
   qkm2 = qkm1;
   qkm1 = qk;

   if( qk != 0 )
   r = pk/qk;
   if( r != 0 )
   {
   t = fabs( (ans - r)/r );
   ans = r;
   }
   else
   t = 1.0;

   if( t < thresh )
   goto cdone;

   k1 += 1.0;
   k2 -= 1.0;
   k3 += 2.0;
   k4 += 2.0;
   k5 += 1.0;
   k6 += 1.0;
   k7 += 2.0;
   k8 += 2.0;

   if( (fabs(qk) + fabs(pk)) > big )
   {
   pkm2 *= biginv;
   pkm1 *= biginv;
   qkm2 *= biginv;
   qkm1 *= biginv;
   }
   if( (fabs(qk) < biginv) || (fabs(pk) < biginv) )
   {
   pkm2 *= big;
   pkm1 *= big;
   qkm2 *= big;
   qkm1 *= big;
   }
   }
   while( ++n < 300 );
   cdone:
   return(ans);
   }
*/

/* Power series for incomplete beta integral.
   Use when b*x is small and x not too close to 1.  */

/*
  double pseries( double a, double b, double x )
  {
  double s, t, u, v, n, t1, z, ai;

  ai = 1.0 / a;
  u = (1.0 - b) * x;
  v = u / (a + 1.0);
  t1 = v;
  t = u;
  n = 2.0;
  s = 0.0;
  z = MACHEP * ai;
  while( fabs(v) > z )
  {
  u = (n - b) * x / n;
  t *= u;
  v = t / (a + n);
  s += v; 
  n += 1.0;
  }
  s += t1;
  s += ai;

  u = a * log(x);
  if( (a+b) < MAXGAM && fabs(u) < MAXLOG )
  {
  t = gamma(a+b)/(gamma(a)*gamma(b));
  s = s * t * pow(x,a);
  }
  else
  {
  t = log_gamma(a+b) - log_gamma(a) - log_gamma(b) + u + log(s);
  if( t < MINLOG )
  s = 0.0;
  else
  s = exp(t);
  }
  return(s);
  }
*/

void random_subset_indices(const TVec<int>& dest, int n)
{
    if (dest.length()>n)
        PLERROR("random_subset_indices: 1st argument should have length (%d) <= value of 2nd argument (%d)",
                dest.length(),n);
    TVec<int> v(0, n-1, 1);
    shuffleElements(v);
    dest << v.subVec(0,dest.length());
}

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
