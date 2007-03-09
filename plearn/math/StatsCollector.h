// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
//
// Copyright (C) 2001,2002 Pascal Vincent
// Copyright (C) 2005 University of Montreal
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef StatsCollector_INC
#define StatsCollector_INC

#include <string>
#include <plearn/base/general.h>
#include <plearn/base/Object.h>
#include <plearn/base/RealMapping.h>
#include "TMat.h"

namespace PLearn {
using namespace std;

//! Forward declarations.
class PLearnDiff;

class StatsCollectorCounts
{
public:
    double n; //!<  counts the number of occurences of the corresponding value
    double nbelow; //!<  counts the number of occurences of values below this counter's value but above the previous one
    double sum; //!<  sum of the values below this counter's but above the previous one
    double sumsquare; //!<  sum of squares of the values below this counter's but above the previous one
    int id; //!< a unique int identifier corresponding to this value (ids will span from 0 to # of known values) also, take a look at StatsCollector::sortIds()
  
    StatsCollectorCounts(): 
        n(0), nbelow(0),
        sum(0.), sumsquare(0.),id(0) 
    {}          

    /**
     * merge: Merge counts from another StatsCollectorCounts
     *        into this one.
     */
    virtual void merge(const StatsCollectorCounts& other)
    {
        //don't change id
        n+= other.n;
        nbelow+= other.nbelow;
        sum+= other.sum;
        sumsquare+= other.sumsquare;
    }
};

typedef pair<real,StatsCollectorCounts*> PairRealSCCType;

//!  this class holds simple statistics about a field

inline PStream& operator>>(PStream& in, StatsCollectorCounts& c)
{ in >> c.n >> c.nbelow >> c.sum >> c.sumsquare >> c.id; return in; }

inline PStream& operator<<(PStream& out, const StatsCollectorCounts& c)
{ out << c.n << c.nbelow << c.sum << c.sumsquare << c.id; return out; }

template<class ObjectType>
int diff(const string& refer, const string& other,
         const Option<ObjectType, StatsCollectorCounts>* opt,
         PLearnDiff* diffs)
{
    StatsCollectorCounts refer_sc, other_sc;
    PStream in = openString(refer, PStream::plearn_ascii);
    in >> refer_sc;
    in = openString(other, PStream::plearn_ascii);
    in >> other_sc;
    int n_diffs = 0;
    PP<OptionBase> opt_double = new Option<ObjectType, double>
        ("", 0, 0, TypeTraits<double>::name(), "", "");
    PP<OptionBase> opt_int = new Option<ObjectType, int>
        ("", 0, 0, TypeTraits<int>::name(), "", "");
    opt_double->setOptionName(opt->optionname() + ".n");
    n_diffs +=  opt_double->diff(tostring(refer_sc.n),
                                 tostring(other_sc.n), diffs);
    opt_double->setOptionName(opt->optionname() + ".nbelow");
    n_diffs += opt_double->diff(tostring(refer_sc.nbelow),
                                tostring(other_sc.nbelow), diffs);
    opt_double->setOptionName(opt->optionname() + ".sum");
    n_diffs += opt_double->diff(tostring(refer_sc.sum),
                                tostring(other_sc.sum), diffs);
    opt_double->setOptionName(opt->optionname() + ".sumsquare");
    n_diffs += opt_double->diff(tostring(refer_sc.sumsquare),
                                tostring(other_sc.sumsquare), diffs);
    opt_int->setOptionName(opt->optionname() + ".id");
    n_diffs += opt_int->diff(tostring(refer_sc.id),
                                tostring(other_sc.id), diffs);
    return n_diffs;
}

DECLARE_SPECIALIZED_DIFF_CLASS(StatsCollectorCounts)

class StatsCollector: public Object
{
    typedef Object inherited;

public:

    PLEARN_DECLARE_OBJECT(StatsCollector);
      
public:

    // ** Build options **

    int maxnvalues; 

    /**
     * If the remove_observation mecanism is used and the removed
     * value is equal to one of first_, last_, min_ or max_, the default
     * behavior is to warn the user.
     * 
     * If one want to disable this feature, he may set
     * no_removal_warnings to true.
     *
     * Default: false (0).
     */
    bool no_removal_warnings;

    // ** Learnt options **

    double nmissing_;      //!< (weighted) number of missing values
    double nnonmissing_;   //!< (weighted) number of non missing value 
    double sumsquarew_;    //!< sum of square of all weights
    double sum_;           //!< sum of all (values-first_)
    double sumsquare_;     //!< sum of square of all (values-first_)
    double sumcube_;       //!< sum of cube of all (values-first_)
    double sumfourth_;     //!< sum of fourth-power of all (values-first_)
    real min_;             //!< the min
    real max_;             //!< the max
    real first_;           //!< first encountered nonmissing observation
    real last_;            //!< last encountered nonmissing observation
    bool more_than_maxnvalues;

    map<real, StatsCollectorCounts> counts; 
    map<int, real> count_ids;

protected:

    //! This map is only created when getApproximateCounts() is called.
    map<real, StatsCollectorCounts> approximate_counts; 

    //! Used to store the sorted values (after taking their absolute value),
    //! with their target value (1 or 0) in the second column.
    mutable Mat sorted_values;

    //! Set to 1 when the values stored in 'counts' are sorted and stored in 'sorted_values'.
    mutable bool sorted;
      
private:

    //! This does the actual building.
    void build_();

protected: 

    //! Declares this class' options
    static void declareOptions(OptionList& ol);

    //! Sort values stored in 'counts' by magnitude, so as to fill 'sorted_values'.
    void sort_values_by_magnitude() const;

    //! Return 'true' iff this StatsCollector needs to fill the 'counts' map,
    //! i.e. iff maxnvalues is set to either -1 or a strictly positive value.
    bool storeCounts() { return (maxnvalues == -1 || maxnvalues > 0); }

public:

    StatsCollector(int the_maxnvalues=0);
      
    real n() const { return nmissing_ + nnonmissing_; } //!< number of samples seen with update (length of VMat for ex.)
    real nmissing() const               { return nmissing_; }
    real nnonmissing() const            { return nnonmissing_; }
    real sumsquarew() const             { return sumsquarew_; }
    real sum() const                    { return real(sum_+nnonmissing_*first_); }
    //real sumsquare() const { return real(sumsquare_); }
    real sumsquare() const              { return real(sumsquare_+2*first_*sum()-first_*first_*nnonmissing_); }
    real min() const                    { return min_; }
    real max() const                    { return max_; }
    real range() const                  { return max_ - min_; }
    real mean() const                   { return real(sum()/nnonmissing_); }
    //! The normalization for variance (nnonmissing_ - sumsquarew_/nnonmissing_)
    //! is defined so that the estimator is unbiased. When all weights are equal
    //! to 1, it reduces to the traditional (n-1) coefficient.
    //! The estimator is unbiased under the assumption that the weights are fixed
    //! and the samples are i.i.d. according to a Gaussian distribution.
    real variance() const               { return real((sumsquare_ - square(sum_)/nnonmissing_)/(nnonmissing_ - sumsquarew_/nnonmissing_)); }
    real stddev() const                 { return sqrt(variance()); }
    real skewness() const;
    real kurtosis() const;
    real stderror() const               { return sqrt(variance()/nnonmissing()); }
    real first_obs() const              { return first_; }
    real last_obs() const               { return last_; }
    real sharperatio() const;
    real mean_over_skewness() const     { return mean()/skewness(); }
    real mean_over_skewness_ms() const;        //!< Special version for model selection
    real mean_over_kurtosis() const     { return mean()/kurtosis(); }
    real zstat() const                  { return mean()/stderror(); }
    real zpr1t() const;                        //!< one-tailed P(zstat())
    real zpr2t() const;                        //!< two-tailed P(zstat())
    real iqr() const                    { return pseudo_quantile(0.75) - pseudo_quantile(0.25); }
    real prr() const                    { return pseudo_quantile(0.99) - pseudo_quantile(0.01); }
    //! Return LIFT(k/n). 'n_pos_in_k' is filled with the number of positive examples
    //! in the first k examples. If provided, 'n_pos_in_k_minus_1' must be the number
    //! of positive examples in the first (k-1) examples. If provided, pos_fraction
    //! must be the average fraction of positive examples in the dataset (= n+ / n).
    real lift(int k, int& n_pos_in_k, int n_pos_in_k_minus_1 = -1, real pos_fraction = -1) const;
    real nips_lift() const;   //!< NIPS_LIFT statistic (see help).
    real prbp() const;        //!< PRBP statistic (see help).
    //! discrete distribution mode
    real dmode() const;
    Vec dmodes() const;

    //! currently understood statnames are :
    //!   - E (mean)
    //!   - V (variance)
    //!   - STDDEV, STDERROR
    //!   - MIN, MAX
    //!   - SUM, SUMSQ
    //!   - FIRST, LAST
    //!   - N, NMISSING, NNONMISING
    //!   - SHARPERATIO
    //!   - EoverSKEW
    //!   - EoverKURT
    //!   - ZSTAT, PZ
    //!   - PSEUDOQ(q)    (q is a fraction between 0 and 1)
    //!   - IQR           (inter-quartile range)
    //!   - PRR           (pseudo robust range)
    //!   - SKEW          (skewness == E(X-mu)^3 / sigma^3)
    //!   - KURT          (kurtosis == E(X-mu)^4 / sigma^4 - 3)
    //!   - DMODE         (discrete distribution mode)
    real getStat(const string& statname) const;

    //! simply calls inherited::build() then build_()
    virtual void build();

    //! clears all statistics, allowing to restart collecting them
    void forget();

    //! update statistics with next value val of sequence 
    void update(real val, real weight = 1.0);

    /** 
     * update statistics as if an observation of value val was removed
     * of the observation sequence.
     */
    void remove_observation(real val, real weight = 1.0);

    //! finishes whatever computation are needed after all updates have been made
    void finalize() {}

    //! Return the mapping from encountered real values to
    //! StatsCollectorCounts.
    map<real, StatsCollectorCounts>* getCounts(){return &counts;}

    //! Same as getCounts(), except that the map that is returned has been
    //! transformed so that no two keys are equal, where equality is defined
    //! as the result of the PLearn function 'is_equal'. This means some keys
    //! may be merged when they are found to be almost equal.
    map<real, StatsCollectorCounts>* getApproximateCounts();
    
    int getMaxNValues(){return maxnvalues;}

    //! returns a Mat with x,y coordinates for plotting the cdf
    //! only if normalized will the cdf go to 1, otherwise it will go to nsamples
    Mat cdf(bool normalized=true) const;

    /**
     * Return the position of the pseudo-quantile Q.  This is derived from
     * the bin-mapping, so maxnvalues must not be zero for this function
     * to return something meaningful.
     */
    real pseudo_quantile(real q) const;

    /**
     * DEPRECATED: DO NOT SORT IDs -xsm
    //! fix 'id' attribute of all StatCollectorCounts so that increasing ids correspond to increasing real values
    //! *** NOT TESTED YET
    void sortIds();
    */

    //! returns a mapping that maps values to a bin number
    //! (from 0 to mapping.length()-1)
    //! The mapping will leave missing values as MISSING_VALUE
    //! And values outside the [min, max] range will be mapped to -1
    //! Tolerance is used to test wheter we join the two last bins or not.
    //! If last be is short of more then tolerance*100%
    //! of continuous_mincount elements, we join it with the previous bin.
    RealMapping getBinMapping(double discrete_mincount,
                              double continuous_mincount,
                              real tolerance=.1,
                              TVec<double>* fcount=0) const;

    RealMapping getAllValuesMapping(TVec<double>* fcount=0) const;
    //! Same as getAllValuesMapping, except we can specify a bool vector,
    //! that indicates whether the k-th range should be included or not.
    //! The boolean 'ignore_other' indicates whether a value not appearing
    //! in the mapping should be mapped to itself (false), or to -1 (true).
    //! We can also give a 'tolerance': in this case, each mapping will
    //! be expanded by '-epsilon' below and '+epsilon' above, with
    //! epsilon = tolerance * mean(difference between two consecutive values).
    //! If two consecutive mappings have a non-empty intersection after
    //! the expansion, they will be merged.
    RealMapping getAllValuesMapping(TVec<bool>* to_be_included, TVec<double>* fcount=0, bool ignore_other = false, real tolerance = 0) const;

    virtual void oldwrite(ostream& out) const;
    /* TODO Remove (deprecated)
       virtual void oldread(istream& in);
    */

    //! Overridden to have a fancy output for raw_ascii and pretty_ascii modes.
    virtual void newwrite(PStream& out) const;

    //! merge another StatsCollector into this one
    virtual void merge(const StatsCollector& other);

};

DECLARE_OBJECT_PTR(StatsCollector);

//! Apparently needed to specialize this method, otherwise it was the generic
//! deepCopyField from CopiesMap.h that was called when deep copying a
//! TVec<StatsCollector>.
template <>
inline void deepCopyField(StatsCollector& field, CopiesMap& copies)
{
    field.makeDeepCopyFromShallowCopy(copies);
}

TVec<RealMapping> computeRanges(TVec<StatsCollector> stats, int discrete_mincount, int continuous_mincount);

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
