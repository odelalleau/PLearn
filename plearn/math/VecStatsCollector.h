// -*- C++ -*-
// VecStatsCollector.h
// 
// Copyright (C) 2002 Pascal Vincent
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
   * $Id: VecStatsCollector.h,v 1.11 2003/10/10 21:10:08 ducharme Exp $ 
   ******************************************************* */

/*! \file VecStatsCollector.h */
#ifndef VecStatsCollector_INC
#define VecStatsCollector_INC

#include "Object.h"
#include "StatsCollector.h"

namespace PLearn <%
using namespace std;

class VecStatsCollector: public Object
{    
public:

  typedef Object inherited;

  // ************************
  // * public build options *
  // ************************

  //! maximum number of different values to keep track of for each element
  int maxnvalues; //! (default: 0, meaning we only keep track of global statistics)

  //! Should we compute and keep X'.X ?
  bool compute_covariance; //! (default false)

  // * "learnt" options *
  TVec<StatsCollector> stats; // the stats for each element
  Mat cov; // the uncentered covariance matrix (mean not subtracted: X'.X)

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
  void forget();

  //! updates the statistics when seeing x
  //! The weight applies to all elements of x
  void update(const Vec& x, real weight = 1.0);
  
  //! calls update on all rows of m; weight assumed to be 1.0 for all roes
  void update(const Mat& m);

  //! calls update on all rows of m;
  //! vector of weights given, weighting each row
  void update(const Mat& m, const Vec& weights);

  //! finishes whatever computation are needed after all updates have been made
  void finalize();

  //! returns statistics for element i
  const StatsCollector& getStats(int i) const 
  { return stats[i]; }

  //! returns the empirical mean (sample average) vec
  Vec getMean() const;

  //! returns the empirical variance vec
  Vec getVariance() const;

  //! returns the empirical standard deviation vec
  Vec getStdDev() const;

  //! returns the empirical standard deviation vec
  Vec getStdError() const;

  //! returns uncentered covariance matrix (mean not subtracted X'.X)
  const Mat& getXtX() const
  { return cov; }

  //! returns centered covariance matrix (mean subtracted)
  Mat getCovariance() const;
  
  //! returns correlation matrix
  Mat getCorrelation() const;

  //! Fills vector st with [mean, variance, stddev, min, max] (after resizing it if it had a size of 0)
  //! However the order and number may change in future versions, so it's better to
  //! first call getIndexInAllStats to get the index of a given stat.
  Vec getAllStats(Vec& st) const;

  //! Returns the index in the vector returned by getAllStats of the stat with the given name.
  //! Currently available names are E (mean) V (variance) STDDEV MIN MAX
  //! Will throw an exception if statname is invalid
  int getIndexInAllStats(int fieldindex, const string& statname) const;

  //! Provides a help message describing this class
  static string help();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(VecStatsCollector);
};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(VecStatsCollector);

  
%> // end of namespace PLearn

#endif
