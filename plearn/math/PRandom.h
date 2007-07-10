// -*- C++ -*-

// PRandom.h
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

/*! \file PRandom.h */


#ifndef PRandom_INC
#define PRandom_INC

#include <plearn/base/Object.h>
#include <plearn/math/TMat_maths.h>          // sum()
#include <boost/random/exponential_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/uniform_01.hpp>

namespace PLearn {

//! Important note: the pointers used for some Boost distribution objects
//! are meant to save memory. They should not be set outside of this class,
//! nor should they be used by any other object (as they will be destroyed
//! with this object).
//!
//! Note also that any field added to this object should be properly copied
//! (and possibly deep-copied) in the copy constructor.
class PRandom: public Object
{

private:
  
    typedef Object inherited;

protected:

    //! The underlying Boost random number generator used.
    boost::mt19937 rgen;

    //! The underlying Boost distribution for exponential sampling.
    boost::exponential_distribution<>* exponential_distribution;

    //! The underlying Boost distribution for normal sampling.
    boost::normal_distribution<>* normal_distribution;

    //! The underlying Boost distribution for uniform sampling.
    boost::uniform_01<boost::mt19937>* uniform_01;

    //! The actual seed used by the random number generator.
    uint32_t the_seed;

    // *********************
    // * protected options *
    // *********************

    int32_t fixed_seed;

public:

    // ************************
    // * public build options *
    // ************************

    int32_t seed_;

    // ****************
    // * Constructors *
    // ****************

    //! Constructor from a given seed.
    PRandom(int32_t seed = -1);

    //! Copy constructor.
    //! This constructor ensures that no deep-copy is needed. All fields are
    //! properly [deep-]copied in this constructor.
    PRandom(const PRandom& rhs);

    //! Operator= .
    //! Do the same thing as copy constructor.
    PRandom operator=(const PRandom& rhs);

    //! Destructor to free memory.
    virtual ~PRandom();

    // ******************
    // * Object methods *
    // ******************

    //! Accessors.
    const boost::mt19937*                     get_rgen()                     const
    { return &rgen; }
    boost::exponential_distribution<>*        get_exponential_distribution() const
    { return exponential_distribution; }
    boost::normal_distribution<>*             get_normal_distribution()      const
    { return normal_distribution; }
    boost::uniform_01<boost::mt19937>*        get_uniform_01()               const
    { return uniform_01; }

    uint32_t get_the_seed()   const { return the_seed; }
    int32_t  get_fixed_seed() const { return fixed_seed; }
    int32_t  get_seed()       const { return seed_; }

private:

    //! This does the actual building. 
    void build_();

protected:

    //! Declares this class' options.
    static void declareOptions(OptionList& ol);

    //! Initialize the random number generator with the CPU time.
    //! This is an internal method that does not update the 'seed' option.
    void time_seed_();

    //! Initialize the random number generator with the given int32_t 'x'.
    //! This is an internal method that does not update the 'seed' option.
    void manual_seed_(int32_t x);

    //! Ensure the 'uniform_01' member is correctly initialized.
    //! This method is called in build(), so it should not be needed to call it
    //! from anywhere else.
    inline void ensure_uniform_01() {
        if (!uniform_01)
            uniform_01 = new boost::uniform_01<boost::mt19937>(rgen);
    }

    //! Ensure the 'normal_distribution' member is correctly initialized.
    //! This method should be called before using the 'normal_distribution'
    //! member, as its presence is not always guaranteed.
    inline void ensure_normal_distribution() {
        if (!normal_distribution)
            normal_distribution = new boost::normal_distribution<>();
    }

    //! Ensure the 'exponential_distribution' member is correctly initialized.
    //! This method should be called before using the 'exponential_distribution'
    //! member, as its presence is not always guaranteed.
    inline void ensure_exponential_distribution() {
        if (!exponential_distribution)
            exponential_distribution = new boost::exponential_distribution<>();
    }

public:

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(PRandom);

    //! Build object.
    //! Note: one should not have to call build() on a PRandom object, as it is
    //! called directly in the constructor and in the time_seed() and manual_seed()
    //! methods. Don't forget that calling build() will re-initialize the random
    //! number generator.
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Set the 'seed' option to 'x' and reset the random number generator
    //! accordingly.
    //! 'x' may be -1 to initialize from the current CPU time, or 0 to make
    //! no initialization.
    void manual_seed(int32_t x);

    //! Initialize the random number generator with the CPU time.
    inline void time_seed() { manual_seed(-1); }

    //! Return a random number uniformly distributed between 0 and 1.
    real uniform_sample();
    //! Return a random number uniformly distributed between a and b.
    real bounded_uniform(real a, real b);

    //! Return a random number generated from a Gaussian with mean 0 and stddev 1.
    real gaussian_01();
    inline real normal_sample() { return this->gaussian_01(); }
    //! Return a random number generated from a Gaussian with mean mu and stddev sigma.
    real gaussian_mu_sigma(real mu, real sigma);

    //! Fill vector 'dest' with samples taken randomly from the given 'set'.
    void fill_random_discrete(const Vec& dest, const Vec& set);

    //! Fill vector 'dest' with samples generated from a normal distribution
    //! with mean 'mean' and standard deviation 'stddev'.
    void fill_random_normal(const Vec& dest, real mean = 0, real stddev = 1);

    //! Fill matrix 'dest' with samples generated from a normal distribution
    //! with mean 'mean' and standard deviation 'stddev'.
    void fill_random_normal(const Mat& dest, real mean = 0, real stddev = 1);

    //! Fill vector 'dest' with samples generated from a uniform distribution
    //! between 'minval' and 'maxval'.
    void fill_random_uniform(const Vec& dest, real min = 0, real max = 1);

    //! Fill matrix 'dest' with samples generated from a uniform distribution
    //! between 'minval' and 'maxval'.
    void fill_random_uniform(const Mat& dest, real min = 0, real max = 1);

    //! Return a random number generated from an exponential distribution with
    //! parameter lambda = 1.
    real exp_sample();

    /* TODO Implement.
    //! Return a random number generated from a gamma distribution.
    real gamma_sample(int ia);
    //! Return a random number generated from a Poisson distribution.
    real poisson_sample(real xm);
    //! Return a random number generated from a binomial distribution with
    //! probability 'pp' and 'n' trials.
    real binom_sample(real pp, int n = 1);
    */

    /**
     *  Return a random deviate from a discrete distribution given explicitely
     *  in the 'distribution' vector. The returned value is an index in
     *  'distribution'.  Elements of 'distribution' must sum to 1.
     */
    int multinomial_sample(const Vec& distribution);

    //! Return an integer between 0 and n-1 with equal probabilities.
    inline int uniform_multinomial_sample(int n)
    { return int(n * this->uniform_sample()); }

    //! Return a sample from a binomial, given the expectation 'pp'
    int binomial_sample(real pp);

    //! Randomly shuffle the entries of a vector (random permutation)
    template<class T>
    void shuffleElements(const TVec<T>& vec);

    /**
     *  Return a weighted random permutation of the entries of a vector.  The
     *  'weights' cause some elements to be picked first.  It is not necessary
     *  for them to be normalized (the function takes care of it).  If desired,
     *  only the first N elements of the permutation may be returned (faster).
     */
    template <class T>
    TVec<T> weightedShuffleElements(const TVec<T>& vec, const Vec& weights,
                                    int number_to_keep = -1 /* all by default */);

    //! Randomly shuffle the rows of a matrix.
    template<class T>
    void shuffleRows(const TMat<T>& mat);

    //! Return a newly allocated PRandom from this one;
    //! The new PRandom is seeded with this one's output.
    PP<PRandom> split();

    /*** Static methods ***/

    /**
     *  Return a pointer to a common PRandom object accessible from any PLearn
     *  code.  There are two common PRandom objects: one whose seed was given
     *  by the CPU time (no argument, or 'random_seed' set to true), and
     *  another whose seed is fixed at compilation time ('random_seed' set to
     *  false).  The latter can be useful to get reproducible results.  For
     *  safety, it is not possible to change their seed.
     */
    static PP<PRandom> common(bool random_seed = true);
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PRandom);


//#####  shuffleElements Implementation  ######################################

template<class T>
void PRandom::shuffleElements(const TVec<T>& vec)
{
    if (vec.isEmpty())
        return;
    T* v = vec.data();
    T tmp;
    int n = vec.length();
    for (int i = 0; i < vec.length(); i++) {
        int j = i + this->uniform_multinomial_sample(n - i);
        tmp = v[i];
        v[i] = v[j];
        v[j] = tmp;
    }
}


//#####  weightedShuffleElements Implementation  ##############################

template <class T>
TVec<T> PRandom::weightedShuffleElements(const TVec<T>& vec, const Vec& weights,
                                         int number_to_keep)
{
    PLASSERT( vec.size() == weights.size() );
    if (number_to_keep < 0)
        number_to_keep = vec.size();

    Vec w = weights.copy();
    TVec<T> r(number_to_keep);

    // Normalize the weights
    real s = sum(w);
    w /= s;

    // The algorithm is fairly simple-minded and is O(N^2), so be careful: we
    // pick the first element according to a multinomial distribution given by
    // the weights.  Then we set the probability of that element to 0 and
    // renormalize the weights.  We pick the second element according to the
    // new distribution, and so forth.
    for (int i=0 ; i<number_to_keep ; ++i) {
        int j = this->multinomial_sample(w);
        r[i] = vec[j];

        // Set its weight to zero and renormalize.  To keep numerical
        // precision, every 20 time-steps we do a full renormalization (and not
        // only an incremental one).
        if (i % 20 == 0) {
            w[j] = 0;
            real s = sum(w);
            w /= s;
        }
        else {
            real old_w = w[j];
            w[j] = 0;
            w /= (1. - old_w); // New sum of weights is 1 minus what we took out
        }
    }
    
    return r;
}


//#####  shuffleRows Implementation  ##########################################

template<class T>
void PRandom::shuffleRows(const TMat<T>& mat)
{
    int n = mat.length();
    for (int i = 0; i < n; i++) {
        int j = i + int(this->uniform_sample() * (n - i));
        mat.swapRows(i,j);
    }
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
