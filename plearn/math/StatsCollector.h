// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2001,2002 Pascal Vincent
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
   * $Id: StatsCollector.h,v 1.17 2003/08/13 08:13:17 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef StatsCollector_INC
#define StatsCollector_INC

#include "general.h"
#include "Object.h"
#include "TMat.h"
#include "RealMapping.h"

namespace PLearn <%
using namespace std;

class StatsCollectorCounts
{
public:
  int n; //!<  counts the number of occurences of the corresponding value
  int nbelow; //!<  counts the number of occurences of values below this counter's value but above the previous one
  double sum; //!<  sum of the values below this counter's but above the previous one
  double sumsquare; //!<  sum of squares of the values below this counter's but above the previous one
  int id; //!< a unique int identifier corresponding to this value (ids will span from 0 to # of known values) also, take a look at StatsCollector::sortIds()
  
  StatsCollectorCounts(): 
    n(0), nbelow(0),
    sum(0.), sumsquare(0.),id(0) {}          
};

typedef pair<real,StatsCollectorCounts*> PairRealSCCType;

//!  this class holds simple statistics about a field

inline PStream& operator>>(PStream& in, StatsCollectorCounts& c)
{ in >> c.n >> c.nbelow >> c.sum >> c.sumsquare >> c.id; return in; }

inline PStream& operator<<(PStream& out, const StatsCollectorCounts& c)
{ out << c.n << c.nbelow << c.sum << c.sumsquare << c.id; return out; }

/*!
    "A StatsCollector allows to compute basic global statistics for a series of numbers,\n"
    "as well as statistics within automatically determined ranges.\n"
    "The first maxnvalues encountered values will be used as points to define\n"
    "the ranges, so to get reasonable results, your sequence should be iid, and NOT sorted!"
*/
  class StatsCollector: public Object
  {
  public:
    typedef Object inherited;
    PLEARN_DECLARE_OBJECT(StatsCollector);
      
  public:

    typedef Object inherited;

    // ** Build options **

    //! maximum number of different values to keep track of in counts
    //! (if 0, we will only keep track of global statistics)
    int maxnvalues; 


    // ** Learnt options **

    int nmissing_;  //!<  number of missing values
    int nnonmissing_; //!<  number of non missing value 
    double sum_; //!<  sum of all values
    double sumsquare_; //!<  sum of square of all values
    real min_; //!< the min
    real max_; //!< the max

    //! will contain up to maxnvalues values and associated Counts
    //! as well as a last element which maps FLT_MAX, so that we don't miss anything
    //! (empty if maxnvalues=0)
    map<real,StatsCollectorCounts> counts; 
      
  private:
  //! This does the actual building.
  // (Please implement in .cc)
  void build_();

  protected: 
    //! Declares this class' options
    static void declareOptions(OptionList& ol);

  public:

    StatsCollector(int the_maxnvalues=0);
      
    int n() const { return nmissing_ + nnonmissing_; } //!< number of samples seen with update (length of VMat for ex.)
    int nmissing() const { return nmissing_; }
    int nnonmissing() const { return nnonmissing_; }
    real sum() const { return real(sum_); }
    real sumsquare() const { return real(sumsquare_); }
    real min() const { return min_; }
    real max() const { return max_; }
    real mean() const { return real(sum_/nnonmissing_); }
    real variance() const { return real((sumsquare_ - square(sum_)/nnonmissing_)/(nnonmissing_-1)); }
    real stddev() const { return sqrt(variance()); }
    real stderror() const { return sqrt(variance()/nnonmissing()); }
          

    //! currently understood statnames are E (mean), V (variance), STDDEV, MIN, MAX, STDERROR
    real getStat(const string& statname) const;

    //! simply calls inherited::build() then build_()
    virtual void build();

    //! clears all statistics, allowing to restart collecting them
    void forget();

    //! update statistics with next value val of sequence 
    void update(real val);

    //! finishes whatever computation are needed after all updates have been made
    void finalize() {}

    map<real,StatsCollectorCounts> * getCounts(){return &counts;}
    int getMaxNValues(){return maxnvalues;}

    //! returns a Mat with x,y coordinates for plotting the cdf
    //! only if normalized will the cdf go to 1, otherwise it will go to nsamples
    Mat cdf(bool normalized=true) const;

    //! fix 'id' attribute of all StatCollectorCounts so that increasing ids correspond to increasing real values
    //! *** NOT TESTED YET
    void sortIds();

    //! returns a mapping that maps values to a bin number (from 0 to mapping.length()-1)
    //! The mapping will leave missing values as MISSING_VALUE
    //! And values outside the [min, max] range will be mapped to -1
    //! Tolerance is used to test wheter we join the two last bins or not. If last be is short of more then tolerance*100%
    //! of continuous_mincount elements, we join it with the previous bin.
    RealMapping getBinMapping(int discrete_mincount, int continuous_mincount, real tolerance=.1, TVec<int>* fcount=NULL) const;

    RealMapping getAllValuesMapping(TVec<int> * fcount=NULL) const;

    virtual void oldwrite(ostream& out) const;
    virtual void oldread(istream& in);
    virtual void print(ostream& out) const;

    //! Provides a help message describing this class
    static string help();
  };

  DECLARE_OBJECT_PTR(StatsCollector);

  //!  this class holds counts of co-occurences of variables within specific ranges
  class ConditionalStatsCollector: public Object
  {
  protected:

    //++++++++++++
    // Parameters


    //! index of conditioning variable
    int condvar; 

    //+++++++++++++++++
    // Computed results

  public:    

    //! ranges[k] must contain bin-mappings for variable k, which maps it to an integer ( 0 to mappings[k].size()-1 )
    TVec<RealMapping> ranges; 

    //! counts[k](i,j) is the number of times the variable k fell in range i while variable condvar was in range j
    //! counts[k] has one more row and column than there are mapping ranges: the last ones counting "MISSING_VALUE" occurences.
    TVec< TMat<int> > counts;

    //! sums[k](i,j) contains the sum of variable k's values that fell in range i while condvar was in range j
    //! (unlike counts, these do not have an extra row and column for misisng value)
    TVec< TMat<double> > sums; 

    //! sums[k](i,j) contains the sum of squares of variable k's values that fell in range i while condvar was in range j
    //! (unlike counts, these do not have an extra row and column for misisng value)
    TVec< TMat<double> > sumsquares;

    PLEARN_DECLARE_OBJECT(ConditionalStatsCollector);

    ConditionalStatsCollector();

    //! Sets the ranges of interest for each variable, and the index of the conditioning variable. Ranges for a given variable should not overlap.
    void setBinMappingsAndCondvar(const TVec<RealMapping>& the_ranges, int the_condvar);

    //! Updates the counts for an observation v
    void update(const Vec& v);

    virtual void write(ostream& out) const;
    virtual void oldread(istream& in);

  protected:

    //! Returns the first index of the range containing the given value for that variable
    //! Returns ranges[varindex].length() if val==missing
    //! Returns -1 if no range containing val was found
    int findrange(int varindex, real val) const;

  };

  DECLARE_OBJECT_PTR(ConditionalStatsCollector);

TVec<RealMapping> computeRanges(TVec<StatsCollector> stats, int discrete_mincount, int continuous_mincount);

%> // end of namespace PLearn

#endif
