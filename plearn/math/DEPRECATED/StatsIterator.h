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
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearn/plearn/math/DEPRECATED/StatsIterator.h */

#ifndef StatsIterator_INC
#define StatsIterator_INC

#include <plearn/base/stringutils.h>
#include <plearn/base/Object.h>
#include "Mat.h"
#include <plearn/vmat/VMat.h>
#include "TMat_maths.h"

namespace PLearn {
using namespace std;


class StatsIterator: public Object
{
    typedef Object inherited;
    
protected:
    Vec result;

public:

    //!  Should return true if several passes are required
    //!  (default version returns false)
    virtual bool requiresMultiplePasses();

    //!  Call this method once with the correct inputsize
    virtual void init(int inputsize)=0;

    //!  Then iterate over the data set and call this method for each row
    virtual void update(const Vec& input)=0;
    virtual void update(const Mat& inputs) 
    { 
        for (int i=0;inputs.length();i++)
        {
            Vec input = inputs(i);
            update(input);
        }
    }

/*!     Call this method when all the data has been shown (through update)
  If the method returns false, then a further pass through the data
  is required.
*/
    virtual bool finish()=0;

    //!  You can call this method after finish has returned true
    virtual Vec getResult();
    
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
    PLEARN_DECLARE_ABSTRACT_OBJECT(StatsIterator);
    
    virtual void build() {}
    virtual void oldwrite(ostream& out) const;
    /* TODO Remove (deprecate)
       virtual void oldread(istream& in);
    */

protected:
    static void declareOptions(OptionList& ol);

};

DECLARE_OBJECT_PTR(StatsIterator);

class MeanStatsIterator: public StatsIterator
{
    typedef StatsIterator inherited;

protected:
    TVec<int> nsamples;

public:
    virtual string info() const { return "mean"; }
    virtual void init(int inputsize);
    virtual void update(const Vec& input);
    virtual bool finish();
    PLEARN_DECLARE_OBJECT(MeanStatsIterator);

    virtual void oldwrite(ostream& out) const;
    /* TODO Remove (deprecated)
       virtual void oldread(istream& in);
    */

protected:
    static void declareOptions(OptionList& ol);

};

DECLARE_OBJECT_PTR(MeanStatsIterator);

class ExpMeanStatsIterator: public StatsIterator
{
    typedef StatsIterator inherited;

protected:
    TVec<int> nsamples;

public:
    virtual string info() const { return "exp_mean"; }

    virtual void init(int inputsize);
    virtual void update(const Vec& input);
    virtual bool finish();
    PLEARN_DECLARE_OBJECT(ExpMeanStatsIterator);

    virtual void oldwrite(ostream& out) const;
    /* TODO Remove (deprecated)
       virtual void oldread(istream& in);
    */

protected:
    static void declareOptions(OptionList& ol);

};

DECLARE_OBJECT_PTR(ExpMeanStatsIterator);

class StddevStatsIterator: public StatsIterator
{
    typedef StatsIterator inherited;
    
protected:
    Vec meansquared;
    Vec mean;
    TVec<int> nsamples;

public:
    virtual string info() const { return "std_dev"; }
    virtual void init(int inputsize);
    virtual void update(const Vec& input);
    virtual bool finish();
    PLEARN_DECLARE_OBJECT(StddevStatsIterator);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void oldwrite(ostream& out) const;
    /* TODO Remove (deprecated)
       virtual void oldread(istream& in);
    */

protected:
    static void declareOptions(OptionList& ol);

};

DECLARE_OBJECT_PTR(StddevStatsIterator);

class StderrStatsIterator: public StatsIterator
{
    typedef StatsIterator inherited;
    
protected:
    Vec meansquared;
    Vec mean;
    TVec<int> nsamples;

public:
    virtual string info() const { return "std_err"; }
    virtual void init(int inputsize);
    virtual void update(const Vec& input);
    virtual bool finish();
    PLEARN_DECLARE_OBJECT(StderrStatsIterator);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void oldwrite(ostream& out) const;
    /* TODO Remove (deprecated)
       virtual void oldread(istream& in);
    */

protected:
    static void declareOptions(OptionList& ol);

};

DECLARE_OBJECT_PTR(StderrStatsIterator);

/*!   Compute the Sharpe ratio = mean(profit) / stdev(profit)
  where profit is assumed to be the "cost" given in input.
  Note that the mean and stdev are only computed over the
  instances where profit!=0 (because these represent the
  actual transactions that occured).
*/
class SharpeRatioStatsIterator: public StatsIterator
{
    typedef StatsIterator inherited;
    
protected:
    Vec nnonzero;
    Vec meansquared;
    Vec mean;

public:
    virtual string info() const { return "sharpe_ratio"; }
    virtual void init(int inputsize);
    virtual void update(const Vec& input);
    virtual bool finish();
    PLEARN_DECLARE_OBJECT(SharpeRatioStatsIterator);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void oldwrite(ostream& out) const;
    /* TODO Remove (deprecated)
       virtual void oldread(istream& in);
    */

protected:
    static void declareOptions(OptionList& ol);

};

DECLARE_OBJECT_PTR(SharpeRatioStatsIterator);

class MinStatsIterator: public StatsIterator
{
    typedef StatsIterator inherited;
    
public:
    virtual string info() const { return "min"; }
    virtual void init(int inputsize);
    virtual void update(const Vec& input);
    virtual bool finish();
    PLEARN_DECLARE_OBJECT(MinStatsIterator);

    virtual void oldwrite(ostream& out) const;
    /* TODO Remove (deprecated)
       virtual void oldread(istream& in);
    */

protected:
    static void declareOptions(OptionList& ol);

};

DECLARE_OBJECT_PTR(MinStatsIterator);

class MaxStatsIterator: public StatsIterator
{
    typedef StatsIterator inherited;
    
public:
    virtual string info() const { return "max"; }
    virtual void init(int inputsize);
    virtual void update(const Vec& input);
    virtual bool finish();
    PLEARN_DECLARE_OBJECT(MaxStatsIterator);

    virtual void oldwrite(ostream& out) const;
    /* TODO Remove (deprecated)
       virtual void oldread(istream& in);
    */

protected:
    static void declareOptions(OptionList& ol);

};

DECLARE_OBJECT_PTR(MaxStatsIterator);

/*!   result Vec has size 2:
  result[0] = lift
  result[1] = lift/lift_maximum
  
*/
class LiftStatsIterator: public StatsIterator
{
    typedef StatsIterator inherited;
    
protected:
    int nsamples;
    int lift_index;
    real lift_fraction;
    Mat output_and_pos;
    Vec targets;

public:
    virtual string info() const { return "lift"; }
    virtual void init(int inputsize);
    virtual void update(const Vec& input);
    virtual bool finish();

    LiftStatsIterator(int the_index=0, real the_fraction=0.1);
    PLEARN_DECLARE_OBJECT(LiftStatsIterator);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void oldwrite(ostream& out) const;
    /* TODO Remove (deprecated)
       virtual void oldread(istream& in);
    */

protected:
    static void declareOptions(OptionList& ol);

};

DECLARE_OBJECT_PTR(LiftStatsIterator);

/*!  result Vec has size N: the value of the observed variable
  at the N given quantiles.
*/

class QuantilesStatsIterator: public StatsIterator
{
    typedef StatsIterator inherited;
    
protected:
    int nsamples;
    Vec quantiles;
    Array<Vec> data;

public:
    virtual string info() const { return "quantiles(" + tostring(quantiles) + ")"; }
    virtual void init(int inputsize);
    virtual void update(const Vec& input);
    virtual bool finish();
    QuantilesStatsIterator(){}
    QuantilesStatsIterator(Vec quantiles, int n_data=1000);
    PLEARN_DECLARE_OBJECT(QuantilesStatsIterator);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void oldwrite(ostream& out) const;
    /* TODO Remove (deprecated)
       virtual void oldread(istream& in);
    */

protected:
    static void declareOptions(OptionList& ol);
};

DECLARE_OBJECT_PTR(QuantilesStatsIterator);

typedef PP<StatsIterator> StatsIt;



class StatsItArray: public Array<StatsIt>
{
public:
    StatsItArray();
    StatsItArray(const StatsIt& statsit);
    StatsItArray(const StatsIt& statsit1, const StatsIt& statsit2);

    StatsItArray(const Array<StatsIt>& va): Array<StatsIt>(va) {}
    StatsItArray(Array<StatsIt>& va): Array<StatsIt>(va) {}
    StatsItArray(const StatsItArray& va): Array<StatsIt>(va) {}
    StatsItArray& operator&=(const StatsIt& v) 
    { PLearn::operator&=(*this,v); return *this;}
    StatsItArray& operator&=(const StatsItArray& va) 
    { PLearn::operator&=(*this,va); return *this; }
    StatsItArray operator&(const StatsIt& v) const 
    { return PLearn::operator&(*this,v); }
    StatsItArray operator&(const StatsItArray& va) const 
    { return PLearn::operator&(*this,va); }

    void init(int inputsize);
    void update(const Vec& input);
    void update(const Mat& inputs);

    //!  returns true if any of the StatsIterator in the array requires more than one pass through the data
    bool requiresMultiplePasses();

    //!  returns an array of those that are not yet finished
    StatsItArray finish();

    Array<Vec> getResults();

    Array<Vec> computeStats(VMat data);
};

DECLARE_TYPE_TRAITS(StatsItArray);

inline PStream &operator>>(PStream &in, StatsItArray &o)
{ in >> static_cast<Array<StatsIt> &>(o); return in; }

inline PStream &operator<<(PStream &out, const StatsItArray &o)
{ out << static_cast<const Array<StatsIt> &>(o); return out; }


template <>
inline void deepCopyField(StatsItArray& field, CopiesMap& copies)
{ field.makeDeepCopyFromShallowCopy(copies); }

inline StatsItArray operator&(const StatsIt& statsit1, const StatsIt& statsit2)
{ return StatsItArray(statsit1,statsit2); }

inline StatsIt mean_stats() { return new MeanStatsIterator(); }
inline StatsIt stddev_stats() { return new StddevStatsIterator(); }
inline StatsIt stderr_stats() { return new StderrStatsIterator(); }
inline StatsIt min_stats() { return new MinStatsIterator(); }
inline StatsIt max_stats() { return new MaxStatsIterator(); }
inline StatsIt quantiles_stats(Vec quantiles, int n_data=1000)
{ return new QuantilesStatsIterator(quantiles,n_data); }
inline StatsIt lift_stats(int the_index=0, real the_fraction=0.1) { return new LiftStatsIterator(the_index, the_fraction); }
inline StatsIt sharpe_ratio_stats() { return new SharpeRatioStatsIterator(); }

//!< exponential of the mean
inline StatsIt exp_mean_stats() { return new ExpMeanStatsIterator(); }

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
