// -*- C++ -*-

// PRandom.cc
//
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, University of Montreal
// Copyright (C) 2005 Olivier Delalleau 
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
 * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file PRandom.cc */

// Constants used for random numbers generation.
#define RAND_EPS 1.2e-7
#define RAND_RNMX (1.0 - RAND_EPS)

#include "PRandom.h"

namespace PLearn {
using namespace std;

/////////////
// PRandom //
/////////////
PRandom::PRandom(long seed):
    exponential_distribution(0),
    normal_distribution(0),
    uniform_01(0),
    the_seed(0),
    fixed_seed(0),
    seed_(seed)
{
    // For convenience, we systematically call build() in the constructor.
    build();
}

PRandom::PRandom(const PRandom& rhs):
    rgen                    (*(rhs.get_rgen())),
    the_seed                (rhs.get_the_seed()),
    fixed_seed              (rhs.get_fixed_seed()),
    seed_                   (rhs.get_seed())
{
    // Note: the extra parentheses are here to tell the compiler that the
    // assignments are meant to be used as truth values.
    if ((exponential_distribution = rhs.get_exponential_distribution()))
        exponential_distribution = new boost::exponential_distribution<>
            (*exponential_distribution);
    if ((normal_distribution      = rhs.get_normal_distribution()))
        normal_distribution      = new boost::normal_distribution<>
            (*normal_distribution);
    if ((uniform_01               = rhs.get_uniform_01()))
        uniform_01               = new boost::uniform_01<boost::mt19937>
            (*uniform_01);
}

PRandom PRandom::operator=(const PRandom& rhs)
{
    rgen =          *(rhs.get_rgen());
    the_seed =      rhs.get_the_seed();
    fixed_seed =    rhs.get_fixed_seed();
    seed_ =         rhs.get_seed();

    if ((exponential_distribution = rhs.get_exponential_distribution()))
        exponential_distribution = new boost::exponential_distribution<>
            (*exponential_distribution);
    if ((normal_distribution      = rhs.get_normal_distribution()))
        normal_distribution      = new boost::normal_distribution<>
            (*normal_distribution);
    if ((uniform_01               = rhs.get_uniform_01()))
        uniform_01               = new boost::uniform_01<boost::mt19937>
            (*uniform_01);

    return (*this);
}


PLEARN_IMPLEMENT_OBJECT(PRandom,
                        "Perform a number of random operations, including generating random numbers",
                        ""
    );

////////////////////
// declareOptions //
////////////////////
void PRandom::declareOptions(OptionList& ol)
{
    declareOption(ol, "seed", &PRandom::seed_, OptionBase::buildoption,
                  "Seed for the random number generator, set at build time:\n"
                  " - -1      : initialized with the current CPU time\n"
                  " -  0      : the current seed is left intact\n"
                  " -  x > 0  : the seed is changed to 'x'");

    // Declared as a learnt option to hide some complexity to the novice.
    declareOption(ol, "fixed_seed", &PRandom::fixed_seed, OptionBase::learntoption,
                  "If set to 0, will be ignored. If set to -2, its value will be copied from\n"
                  "the 'seed' option. If set to any other value, it must always be equal to\n"
                  "'seed' when build() is called. This allows one to prevent the seed from\n"
                  "being accidentally modified by setting 'fixed_seed' to -2. Someone modifying\n"
                  "the seed afterwards will then get an error.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

/////////////////////
// bounded_uniform //
/////////////////////
real PRandom::bounded_uniform(real a, real b) {
    real res = uniform_sample()*(b-a) + a;
    if (res >= b)
        return b*RAND_RNMX;
    else
        return res;
}

///////////
// build //
///////////
void PRandom::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void PRandom::build_()
{
    if (fixed_seed) {
        if (fixed_seed == -2)
            fixed_seed = seed_;
        else {
            if (seed_ != fixed_seed)
                PLERROR("In PRandom::build_ - You are not allowed to modify the seed of "
                        "a PRandom object whose seed has been fixed");
        }
    }
    if (seed_ == -1)
        this->time_seed_();
    else if (seed_ == 0) {}
    else if (seed_ > 0)
        this->manual_seed_(seed_);
    else
        PLERROR("In PRandom::build_ - The only value allowed for the seed are "
                "-1, 0 or a strictly positive long integer");
}

////////////
// common //
////////////
PP<PRandom> PRandom::common(bool random_seed)
{
    static PP<PRandom> gen_random = 0;
    static PP<PRandom> gen_const  = 0;
    if (random_seed) {
        if (!gen_random) {
            gen_random = new PRandom();
            gen_random->fixed_seed = -2;
            gen_random->build();
        }
        return gen_random;
    } else {
        if (!gen_const) {
            gen_const = new PRandom(12345678);
            gen_const->fixed_seed = -2;
            gen_const->build();
        }
        return gen_const;
    }
}

////////////////
// exp_sample //
////////////////
real PRandom::exp_sample() {
    ensure_exponential_distribution();
    return real((*exponential_distribution)(*uniform_01));
}

//////////////////////////
// fill_random_discrete //
//////////////////////////
void PRandom::fill_random_discrete(const Vec& dest, const Vec& set)
{
    PLASSERT( dest.isEmpty() || !set.isEmpty() );
    Vec::iterator it = dest.begin();
    Vec::iterator itend = dest.end();
    int n = set.length();
    for(; it != itend; ++it)
        *it = set[this->uniform_multinomial_sample(n)];
}

////////////////////////
// fill_random_normal //
////////////////////////
void PRandom::fill_random_normal(const Vec& dest, real mean, real stddev) {
    for (int i = 0; i < dest.length(); i++)
        dest[i] = gaussian_mu_sigma(mean, stddev);
}

void PRandom::fill_random_normal(const Mat& dest, real mean, real stddev) {
    for (int i = 0; i < dest.length(); i++)
        fill_random_normal(dest(i), mean, stddev);
}

/////////////////////////
// fill_random_uniform //
/////////////////////////
void PRandom::fill_random_uniform(const Vec& dest, real min, real max)
{
    for (int i = 0; i < dest.length(); i++)
        dest[i] = bounded_uniform(min, max);
}

void PRandom::fill_random_uniform(const Mat& dest, real min, real max)
{
    for (int i = 0; i < dest.length(); i++)
        fill_random_uniform(dest(i), min, max);
}

/////////////////
// gaussian_01 //
/////////////////
real PRandom::gaussian_01() {
    ensure_normal_distribution();
    return real((*normal_distribution)(*uniform_01));
}

///////////////////////
// gaussian_mu_sigma //
///////////////////////
real PRandom::gaussian_mu_sigma(real mu, real sigma) {
    return gaussian_01() * sigma + mu;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PRandom::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    // Nothing more should be added here: this object is meant to be properly
    // copied directly within the copy constructor (this is where you should add
    // any statement needed for a proper copy).
}

/////////////////
// manual_seed //
/////////////////
void PRandom::manual_seed(long x)
{
    if (fixed_seed)
        PLERROR("In PRandom::manual_seed - You are not allowed to change the seed "
                "of a PRandom object whose seed is fixed");
    seed_ = x;
    build();
}

//////////////////
// manual_seed_ //
//////////////////
void PRandom::manual_seed_(long x)
{
    the_seed = boost::uint32_t(x);
    rgen.seed(the_seed);
    if (uniform_01) {
        // The boost::uniform_01 object must be re-constructed from the updated
        // random number generator.
        delete uniform_01;
        uniform_01 = 0;
    }
    // Systematically construct the uniform_01 member, which is the basis for most
    // of the random operations.
    ensure_uniform_01();
}

////////////////////////
// multinomial_sample //
////////////////////////
int PRandom::multinomial_sample(const Vec& distribution) {
    real  u  = this->uniform_sample();
    real* pi = distribution.data();
    real  s  = *pi;
    int   n  = distribution.length();
    int   i  = 0;
    while ((i<n) && (s<u)) {
        i++;
        pi++;
        s += *pi;
    }
    if (i == n)
        i = n - 1; // Improbable, but...
    return i;
}

/////////////////////
// binomial_sample //
/////////////////////
int PRandom::binomial_sample(real pp) {
    if( pp < 0 || pp > 1 )
        PLERROR("In PRandom::binomial_sample, pp should be between 0 and 1, "
                "but is %f.", pp);

    real u = this->uniform_sample();
    if( pp < u )
        return 0;
    else
        return 1;
}

////////////////
// time_seed_ //
////////////////
void PRandom::time_seed_()
{
    time_t ltime;
    struct tm *today;
    time(&ltime);
    today = localtime(&ltime);
    manual_seed_((long)today->tm_sec+
                 60*today->tm_min+
                 60*60*today->tm_hour+
                 60*60*24*today->tm_mday);
}

////////////////////
// uniform_sample //
////////////////////
real PRandom::uniform_sample() {
    return real((*uniform_01)());
}

///////
// ~ //
///////
PRandom::~PRandom() {
    if (uniform_01) {
        delete uniform_01;
        uniform_01 = 0;
    }
    if (normal_distribution) {
        delete normal_distribution;
        normal_distribution = 0;
    }
    if (exponential_distribution) {
        delete exponential_distribution;
        exponential_distribution = 0;
    }
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
