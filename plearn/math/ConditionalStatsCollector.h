// -*- C++ -*-

// ConditionalStatsCollector.h
//
// Copyright (C) 2003 Pascal Vincent 
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
   * $Id: ConditionalStatsCollector.h,v 1.2 2004/01/08 18:55:27 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file ConditionalStatsCollector.h */


#ifndef ConditionalStatsCollector_INC
#define ConditionalStatsCollector_INC

#include "Object.h"
#include "TMat.h"
#include "RealMapping.h"

namespace PLearn <%
using namespace std;

class ConditionalStatsCollector: public Object
{
public:
  typedef Object inherited;

protected:
  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...
    
public:

  // ************************
  // * options *
  // ************************

    //! index of conditioning variable
    int condvar; 

    //! ranges[k] must contain bin-mappings for variable k, which maps it to an integer ( 0 to mappings[k].size()-1 )
    TVec<RealMapping> ranges; 

    //! counts[k](i,j) is the number of times the variable k fell in range i while variable condvar was in range j
    //! counts[k] has one more row and column than there are mapping ranges: the last ones counting "MISSING_VALUE" occurences.
    //! Actually counts is the "number of times" only when update is called without a weight. Otherwise it's really the sum of 
    //! the sample weights.
    TVec< TMat<double> > counts;

    //! sums[k](i,j) contains the (possibly weighted) sum of variable k's values that fell in range i while condvar was in range j
    //! (unlike counts, these do not have an extra row and column for misisng value)
    TVec< TMat<double> > sums; 

    //! sumsquares[k](i,j) contains the (possibly weighted) sum of squares of variable k's values that fell in range i while condvar was in range j
    //! (unlike counts, these do not have an extra row and column for misisng value)
    TVec< TMat<double> > sumsquares;

    TVec< TMat<double> > minima; 
    TVec< TMat<double> > maxima; 


  // ****************
  // * Methods
  // ****************

public:

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  ConditionalStatsCollector();

  //! Sets the ranges of interest for each variable, and the index of the conditioning variable. Ranges for a given variable should not overlap.
  void setBinMappingsAndCondvar(const TVec<RealMapping>& the_ranges, int the_condvar);

  //! clears all statistics, allowing to restart collecting them
  void forget();

  //! Updates the counts for an observation v
  void update(const Vec& v, real weight = 1.0);

protected:
    //! Returns the first index of the range containing the given value for that variable
    //! Returns ranges[varindex].length() if val==missing
    //! Returns -1 if no range containing val was found
    int findrange(int varindex, real val) const;

  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  // (PLEASE IMPLEMENT IN .cc)
  void build_();

protected: 
  //! Declares this class' options
  // (PLEASE IMPLEMENT IN .cc)
  static void declareOptions(OptionList& ol);

public:
  // Declares other standard object methods
  //  If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
  PLEARN_DECLARE_OBJECT(ConditionalStatsCollector);

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  // (PLEASE IMPLEMENT IN .cc)
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(ConditionalStatsCollector);
  
%> // end of namespace PLearn

#endif
