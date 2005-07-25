// -*- C++ -*-
// VecStatsCollector.h
// 
// Copyright (C) 2002 Pascal Vincent
// Copyright (C) 2005 Université de Montréal
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

/*! \file VecStatsCollector.h */
#ifndef VecStatsCollector_INC
#define VecStatsCollector_INC

// From C++ stdlib
#include <map>

// From PLearn
#include <plearn/base/Object.h>
#include "StatsCollector.h"

namespace PLearn {
using namespace std;

class VecStatsCollector: public Object
{    
private:
  typedef Object inherited;

protected:
  //! Map from fieldnames to fieldnumbers, to really speed up getFieldNum
  //! which can be a speed bottleneck in some experiments
  map<string,int> fieldnames_num;
  
  //! Names of the fields of the update vector;
  //! now protected: use setFieldNames to set them!
  TVec<string> fieldnames;

public:
  // ************************
  // * public build options *
  // ************************

  //! maximum number of different values to keep track of for each element
  //! (default: 0, meaning we only keep track of global statistics)
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

  //! Should we compute and keep X'.X ?  (default false)
  bool compute_covariance;

  //! See .cc for help.
  double epsilon;

  // ******************
  // * learnt options *
  // ******************

  //! the stats for each element
  TVec<StatsCollector> stats;

  //! See .cc for help.
  Mat cov; 
  
  // ****************
  // * Constructors *
  // ****************

  VecStatsCollector();

  
  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  //! Declares this class' options
  static void declareOptions(OptionList& ol);

public:
  
  int length() const { return stats.length(); }
  int size() const { return length(); }

  //! simply calls inherited::build() then build_()
  virtual void build();

  //! clears all previously accumulated statistics
  virtual void forget();

  //! updates the statistics when seeing x
  //! The weight applies to all elements of x
  virtual void update(const Vec& x, real weight = 1.0);

  /*! 
   * Update statistics as if the vectorial observation x
   * was removed of the observation sequence.
   */
  virtual void remove_observation(const Vec& x, real weight = 1.0);
  
  //! Declares names for the columns of the vector passed to update
  void setFieldNames(TVec<string> the_fieldnames);
  
  //! Returns the declared names
  TVec<string> getFieldNames() const
  { return fieldnames; }

  //! Returns the index corresponding to a fieldname or to the fieldnum passed as a string.
  //! returns -1 if not found
  int getFieldNum(const string& fieldname_or_num) const;

  //! Returns a particular statistic.
  /*! Standard statistics specifications are of the form ex: STAT[fieldname]
    or STAT[fieldnum] where STAT is one of the statistics names understood by
    StatsCollector::getStat. fieldnum start at 0, and fieldnames must have been
    registered with setFieldNames.
  Subclasses may overload this to handle more exotic statistics than the few 
  standard ones. */
  virtual double getStat(const string& statspec);

  //! calls update on all rows of m; weight assumed to be 1.0 for all roes
  void update(const Mat& m);

  //! calls update on all rows of m;
  //! vector of weights given, weighting each row
  void update(const Mat& m, const Vec& weights);

  //! finishes whatever computation are needed after all updates have been made
  virtual void finalize();

  //! returns statistics for element i
  const StatsCollector& getStats(int i) const 
  { return stats[i]; }

  //! returns the empirical mean (sample average) vec
  Vec getMean() const {
    Vec mean;
    getMean(mean);
    return mean;
  }
  
  //! Store the empirical mean in the given vec (which is resized)
  void getMean(Vec& mean) const;
  
  //! returns the empirical variance vec
  Vec getVariance() const;

  //! returns the empirical standard deviation vec
  Vec getStdDev() const;

  //! returns the empirical standard deviation vec
  Vec getStdError() const;

  //! Return X'X.
  const Mat& getXtX() const
  { return cov; }

  //! Covariance matrix computation.
  //! Note that the covariance is computed in order to give an unbiased
  //! estimator (under the i.i.d. assumption), so that the normalization
  //! coefficient is not exactly the sum of weights.
  void getCovariance(Mat& covar) const;
  Mat getCovariance() const;
  
  //! returns correlation matrix
  Mat getCorrelation() const;

  //! Fills vector st with [mean, variance, stddev, min, max] (after resizing it if it had a size of 0)
  //! However the order and number may change in future versions, so it's better to
  //! first call getIndexInAllStats to get the index of a given stat.
  Vec getAllStats(Vec& st) const;

  //! Call getStat() with the given statname on all the statscollectors
  Vec getAllStats(const string& statname) const;

  //! Call getStat() with the given statname on all the statscollectors,
  //! and put result in given Vec.  The vector is resized as necessary.
  void getAllStats(const string& statname, Vec& result) const;
  
  //! Returns the index in the vector returned by getAllStats of the stat with the given name.
  //! Currently available names are E (mean) V (variance) STDDEV MIN MAX
  //! Will throw an exception if statname is invalid
  int getIndexInAllStats(int fieldindex, const string& statname) const;

  //! A little magic function that appends all the StatsCollectors of an
  //! existing VecStatsCollector into this one.  A fieldname prefix can
  //! be specified, in which case the prefix is contatenated to the
  //! existing fieldnames.  Otherwise, a vector of new fieldnames can be
  //! specified (overrides the prefix).  If compute_covariance=true,
  //! a block-diagonal covariance matrix is computed.
  void append(const VecStatsCollector& vsc, const string fieldname_prefix="",
              const TVec<string>& new_fieldnames = TVec<string>() );
  
  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(VecStatsCollector);
};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(VecStatsCollector);

  
} // end of namespace PLearn

#endif
