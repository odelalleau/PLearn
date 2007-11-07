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
 * AUTHORS: Pascal Vincent
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearn/plearn/db/SDBWithStats.h */

#ifndef SDBWithStats_INC
#define SDBWithStats_INC

//#include "general.h"
#include "SimpleDB.h"
#include <map>

namespace PLearn {
using namespace std;

class FieldStat
{
    friend class SDBWithStats;

protected:

    //!  For all values:
    int nonmissing_; //!<  number of entries with non missing value
    int missing_;    //!<  number of entries with missing value

    //!  For numeric values:
    double sum_;
    double sumsquare_;

    double min_;
    double max_;
    double mean_;
    double stddev_;

public:

    //!  For symbolic values
    map<string,int> symbolcount;
    mutable map<string,int> symbolid; //!<  an int between 1 and nsymbols associated to each symbol
    int nsymbols() { return (int)symbolcount.size(); }
    static int max_nsymbols; //!<  stop remembering symbols above this number...

    FieldStat()
        :nonmissing_(0), missing_(0), 
         sum_(0.), sumsquare_(0), min_(FLT_MAX), max_(-FLT_MAX)
    {}

    int ntotal() const { return missing_+nonmissing_; }
    int missing() const { return missing_; }
    int nonmissing() const { return nonmissing_; }
    real mean() const { return real(mean_); }
    real stddev() const { return real(stddev_); }
    real min() const { return real(min_); }
    real max() const { return real(max_); }

    void updateString(const string& sym);
    void updateNumber(double d);
    void updateMissing()  { ++missing_; }

    void clear(); //!<  clear everything 
    void finalize(); //!<  computes final mean and stddev
};

class SDBWithStats: public SDB
{
public:
    vector<FieldStat> fieldstat;
    int nfields() { return (int)getSchema().size(); }
    string fieldname(int i) { return getSchema()[i].name; }

public:
    SDBWithStats(string basename, string path=".", AccessType access = readwrite,
		 bool verbose=true);

    void forgetStats();
    void computeStats(unsigned int nrows);
    void computeStats() { computeStats(size()); }

    bool hasStats(); //!<  returns true if the stats files exist
    void saveStats();
    void loadStats();

    FieldStat& getStat(int i);
    const FieldStat& getStat(int i) const;
    FieldStat& getStat(const string& fieldname);
    const FieldStat& getStat(const string& fieldname) const;
};

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
