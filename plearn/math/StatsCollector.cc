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
   * $Id: StatsCollector.cc,v 1.47 2005/01/28 02:45:43 chapados Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include <plearn/base/stringutils.h>
#include "StatsCollector.h"
#include "TMat_maths.h"
#include "pl_erf.h"
#include <assert.h>


namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
  StatsCollector,
  "Collects basic statistics",
  "A StatsCollector allows to compute basic global statistics for a series of numbers,\n"
  "as well as statistics within automatically determined ranges.\n"
  "The first maxnvalues encountered values will be used as reference points to define\n"
  "the ranges, so to get reasonable results, your sequence should be iid, and NOT sorted!\n"
  "\n"
  "The following statistics are available:\n"
  "  - E              Sample mean\n"
  "  - V              Sample variance\n"
  "  - STDDEV         Sample standard deviation\n"
  "  - STDERROR       Standard error of the sample mean\n"
  "  - SKEW           Skewness == E(X-mu)^3 / sigma^3\n"
  "  - KURT           Excess Kurtosis == E(X-mu)^4 / sigma^4 - 3\n"
  "  - MIN            Minimum value\n"
  "  - MAX            Maximum value\n"
  "  - RANGE          The range, i.e. MAX - MIN\n"
  "  - SUM            Sum of observations \n"
  "  - SUMSQ          Sum of squares\n"
  "  - FIRST          First observation\n"
  "  - LAST           Last observation\n"
  "  - N              Total number of observations\n"
  "  - NMISSING       Number of missing observations\n"
  "  - NNONMISSING    Number of non-missing observations\n"
  "  - SHARPERATIO    Mean divided by standard deviation\n"
  "  - ZSTAT          Z-statistic of the sample mean estimator\n"
  "  - PZ1t           One-tailed probability of the Z-Statistic\n"
  "  - PZ2t           Two-tailed probability of the Z-Statistic\n"
  "  - PSEUDOQ(q)     Return the location of the pseudo-quantile q, where 0 < q < 1\n"
  "                   NOTE that bin counting must be enabled, i.e. maxnvalues > 0\n"
  "  - IQR            The interquartile range, i.e. PSEUDOQ(0.75) - PSEUDOQ(0.25)\n"
  "  - PRR            The pseudo robust range, i.e. PSEUDOQ(0.99) - PSEUDOQ(0.01)\n"
  "  - LIFT(f)        Lift computed at fraction f (0 <= f <= 1)\n"
  "  - NIPS_LIFT      Lift cost as computed in NIPS'2004 challenge\n"
  "\n"
  "Notes:\n"
  "  - When computing LIFT-related statistics, all values encountered need to be stored\n"
  "    which means that 'maxnavalues' should be set to a high value. Also, a value should\n"
  "    be positive when the real target is the class of interest (positive example), and\n"
  "    negative otherwise, the magnitude being the estimated likelihood of the example.\n"
  "  - Formulas to compute LIFT-related statistics. Let n+ = number of positive examples,\n"
  "    n = total number of examples, v_i the value assigned to example i, and assume\n"
  "    examples are sorted by order of magnitude |v_i|:\n"
  "    LIFT(f) = sum_{k=1}^{fn} 1_{v_i > 0} / (f * n+)\n"
  "    NIPS_LIFT = (A_I - A) / A_I, with\n"
  "      A = sum_{k=1}^n LIFT(k/n) / n\n"
  "      A_I = (n / n+ - 1) / 2 * (n+ / n + 1) + 1\n"
  "      See http://predict.kyb.tuebingen.mpg.de/pages/evaluation.php for details (note\n"
  "      that the formulas on the web site and in the python script are different).\n"
  "  - LIFT(f) actually returns - 100 * LIFT(f), so that lower means better, and it is\n"
  "    scaled by 100, as it is common practice.\n"
  "  - The skewness and kurtosis are computed in terms of UNCENTERED ACCUMULATORS,\n"
  "    i.e. sum{(x-a)^n}, where a is the first observation encountered, and n is some integer\n"
  "  - For the skewness, defined as skewness == E(X-mu)^3 / sigma^3, we compute the top\n"
  "    term as\n"
  "\n"
  "        (x-a)^3+(3(x-a)^2+(a-mu)(3(x-a)+a-mu))(a-mu)\n"
  "\n"
  "  - Likewise for the kurtosis, defined as kurtosis == E(x-mu)^4 / sigma^4 - 3, \n"
  "    (note that this is the EXCESS kurtosis, whose value is 0 for a \n"
  "    normal distribution), we compute the top term as\n"
  "\n"
  "        (x-a)^4+(4(x-a)^3+(6(x-a)^2+(a-mu)(4(x-a)+a-mu))(a-mu))(a-mu)\n"
  "\n"
  "  - (Nicolas remercie Dieu et Wolfram pour Mathematica)."
  );
  

StatsCollector::StatsCollector(int the_maxnvalues)
  : maxnvalues(the_maxnvalues), no_removal_warnings(false),
    nmissing_(0.), nnonmissing_(0.), 
    sum_(0.), sumsquare_(0.),
    sumcube_(0.), sumfourth_(0.),
    min_(MISSING_VALUE), max_(MISSING_VALUE),
    first_(MISSING_VALUE), last_(MISSING_VALUE),
    more_than_maxnvalues(false),
    sorted(false)
{
  build_();
}

int sortIdComparator(const void* i1, const void* i2)
{
  double d = ((PairRealSCCType*)i1)->first - ((PairRealSCCType*)i2)->first;
  return (d<0)?-1:(d==0?0:1);
}

//! fix 'id' attribute of all StatCollectorCounts so that increasing ids correspond to increasing real values
//! *** NOT TESTED YET (Julien)
void StatsCollector::sortIds()
{
  PairRealSCCType* allreals= new PairRealSCCType[counts.size()];
  int i=0;
  for(map<real,StatsCollectorCounts>::iterator it = counts.begin();it!=counts.end();++it,i++)
    allreals[i]=make_pair(it->first,&(it->second));
  qsort(allreals,counts.size(),sizeof(PairRealSCCType),sortIdComparator);
  for(int i=0;i<(int)counts.size();i++)
    allreals[i].second->id=i;
  delete allreals;
}

void StatsCollector::declareOptions(OptionList& ol)
{
  // buid options

  declareOption(ol, "maxnvalues", &StatsCollector::maxnvalues, OptionBase::buildoption,
                "maximum number of different values defining ranges to keep track of in counts\n"
                "(if 0, we will only keep track of global statistics)");

  declareOption( ol, "no_removal_warnings", &StatsCollector::no_removal_warnings,
                 OptionBase::buildoption,
                 "If the remove_observation mecanism is used and the removed\n"
                 "value is equal to one of last_, min_ or max_, the default\n"
                 "behavior is to warn the user.\n"
                 "\n"
                 "If one want to disable this feature, he may set\n"
                 "no_removal_warnings to true.\n"
                 "\n"
                 "Default: false (0)." );


  // learnt options
  declareOption(ol, "nmissing_", &StatsCollector::nmissing_, OptionBase::learntoption,
                "number of missing values");
  declareOption(ol, "nnonmissing_", &StatsCollector::nnonmissing_, OptionBase::learntoption,
                "number of non missing value ");
  declareOption(ol, "sum_", &StatsCollector::sum_, OptionBase::learntoption,
                "sum of all (values-first_observation)");
  declareOption(ol, "sumsquare_", &StatsCollector::sumsquare_, OptionBase::learntoption,
                "sum of square of all (values-first_observation)");
  declareOption(ol, "sumcube_", &StatsCollector::sumcube_, OptionBase::learntoption,
                "sum of cube of all (values-first_observation)");
  declareOption(ol, "sumfourth_", &StatsCollector::sumfourth_, OptionBase::learntoption,
                "sum of fourth power of all (values-first_observation)");
  declareOption(ol, "min_", &StatsCollector::min_, OptionBase::learntoption,
                "the min");
  declareOption(ol, "max_", &StatsCollector::max_, OptionBase::learntoption,
                "the max");
  declareOption(ol, "first_", &StatsCollector::first_, OptionBase::learntoption,
                "first encountered observation");
  declareOption(ol, "last_", &StatsCollector::last_, OptionBase::learntoption,
                "last encountered observation");
  declareOption(ol, "counts", &StatsCollector::counts, OptionBase::learntoption,
                "will contain up to maxnvalues values and associated Counts\n"
                "as well as a last element which maps FLT_MAX, so that we don't miss anything\n"
                "(remains empty if maxnvalues=0)");
  declareOption(ol, "more_than_maxnvalues", &StatsCollector::more_than_maxnvalues, OptionBase::learntoption,
      "Set to 1 when more than 'maxnvalues' are seen. This is to warn the user when computing\n"
      "statistics that may be inaccurate when not all values are kept (e.g., LIFT).");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void StatsCollector::build_()
{
  // make sure counts.size==0. If not, the object must have been loaded, and FLT_MAX is an existing key
  // but rounded to some precision, and there would be 2 keys approx.=  FLT_MAX
  if(maxnvalues>0 && counts.size()==0)
    counts[FLT_MAX] = StatsCollectorCounts();
  // If no values are kept, then we always see more than 0 values.
  if (maxnvalues == 0)
    more_than_maxnvalues = true;
}

///////////
// build //
///////////
void StatsCollector::build()
{
  inherited::build();
  build_();
}

////////////
// forget //
////////////
void StatsCollector::forget()
{
    nmissing_ = 0.;
    nnonmissing_ = 0.;
    sum_ = 0.;
    sumsquare_ = 0.;
    sumcube_ = 0.;
    sumfourth_ = 0.;
    min_ = MISSING_VALUE;
    max_ = MISSING_VALUE;
    first_ = last_ = MISSING_VALUE;
    more_than_maxnvalues = (maxnvalues == 0);
    sorted = false;
    counts.clear();
    build_();
}

////////////
// update //
////////////
void StatsCollector::update(real val, real weight)
{
  if(is_missing(val))
    nmissing_ += weight;
  else
  {
    //sum_ += val * weight;
    //sumsquare_ += val*val * weight;
    last_ = val;
    if(nnonmissing_==0)                      // first value encountered
      min_ = max_ = first_ = last_ = val;
    else if(val<min_)
      min_ = val;
    else if(val>max_)
      max_ = val;
    nnonmissing_ += weight;
    double sqval = (val-first_)*(val-first_);
    sum_       += (val-first_)       * weight;
    sumsquare_ += sqval              * weight;
    sumcube_   += sqval*(val-first_) * weight;
    sumfourth_ += sqval*sqval        * weight;
    
    if(maxnvalues>0)  // also remembering statistics inside values ranges
    {
      sorted = false;
      map<real,StatsCollectorCounts>::iterator it;        
      if(int(counts.size())<=maxnvalues) // Still remembering new unseen values
      {
        it = counts.find(val);
        if(it==counts.end())
          counts[val].id=counts.size()-1;
        counts[val].n += weight;
      }
      else // We've filled up counts already
      {
        it = counts.lower_bound(val);
        if(it->first==val) // found the exact value
          it->second.n += weight;
        else // found the value just above val (possibly FLT_MAX)
        {
          more_than_maxnvalues = true;
          it->second.nbelow += weight;
          it->second.sum += val * weight;
          it->second.sumsquare += val*val * weight;
        }
      }
    }
  }
}                           

void StatsCollector::remove_observation(real val, real weight)
{
  if(is_missing(val))
  {
    nmissing_ -= weight;
    assert( nmissing_ >= 0 );
  }
  else
  {
    sorted = false;
    nnonmissing_ -= weight;
    assert( nnonmissing_ >= 0 );

    if( !no_removal_warnings )
    {
      if(val == first_)
        PLWARNING( "Removed value is equal to the first value encountered.\n"
                   "StatsCollector::first() may not be valid anymore." );
      if(val == last_)
        PLWARNING( "Removed value is equal to the last value encountered.\n"
                   "StatsCollector::last() may not be valid anymore." );
      if(val == min_)
        PLWARNING( "Removed value is equal to the min value encountered.\n"
                   "StatsCollector::min() may not be valid anymore." );
      if(val == max_)
        PLWARNING( "Removed value is equal to the max value encountered.\n"
                   "StatsCollector::max() may not be valid anymore." );
    }

    double sqval = (val-first_)*(val-first_);
    sum_       -= (val-first_)       * weight;
    sumsquare_ -= sqval              * weight;
    sumcube_   -= sqval*(val-first_) * weight;
    sumfourth_ -= sqval*sqval        * weight;

    if(nnonmissing_==0) {
      // first value encountered
      min_ = max_ = first_ = last_ = MISSING_VALUE;
      sum_ = sumsquare_ = sumcube_ = sumfourth_ = 0.0;
    }

    // assertion is after previous check for nnonmissing_, since the last
    // subtraction of sumsquare might have left sumsquare very slightly
    // negative due to roundoff errors
    assert( sumsquare_ >= 0 );
    
    if(maxnvalues>0)
      PLERROR("The remove observation mecanism is incompatible with maxnvalues.");
  }
}                           

RealMapping StatsCollector::getBinMapping(double discrete_mincount,
                                          double continuous_mincount,
                                          real tolerance,
                                          TVec<double> * fcount) const
{
  real mapto=0.;
  RealMapping mapping;
  mapping.setMappingForOther(-1);
  map<real,StatsCollectorCounts>::const_iterator it = counts.begin();
  int nleft = (int)counts.size()-1; // loop on all but last

  if(fcount)
  {
    (*fcount) = TVec<double>();
    // ouch, assume discrete_mincount == continuous_mincount
    fcount->resize(0, int(2.*nnonmissing_ / discrete_mincount));
    fcount->append(nmissing_);
    fcount->append(0);
  }

  double count = 0, count2 = 0;
  real low = min_;
  real high = min_;
  bool low_has_been_appended = false;
  // ProgressBar pb("Computing PseudoQ Mapping...",counts.size()-1);

  while(nleft--)
  {
    high = it->first;
    // pb(counts.size()-1-nleft);
    count += it->second.nbelow;
    count2 += it->second.nbelow;
    // cerr << "it->first:"<<it->first<<" nbelow:"<<it->second.nbelow<<" n:"<<it->second.n<<endl;
    if(count>=continuous_mincount)
    {
      // append continuous range
      mapping.addMapping(
        RealRange(low_has_been_appended?']':'[',low, high, '['),
        mapto++);
      if(fcount)
        fcount->append(count);
      low = high;
      low_has_been_appended = false;
      count = 0;

    }

    if(it->second.n >= discrete_mincount)
    {
      if(count>0) // then append the previous continuous range
      {
        mapping.addMapping(RealRange(low_has_been_appended?']':'[',low, high, '['), mapto++);
        if(fcount)
          fcount->append(count);
        count = 0;
      }
      // append discrete point
      mapping.addMapping(RealRange('[',high,high,']'), mapto++);
      if(fcount)
        fcount->append(it->second.n + count);
      count2+=it->second.n;
      count=0;
      low = high;
      low_has_been_appended = true;
    }
    else
    {
      count2+=it->second.n;      
      count += it->second.n;
    }
    ++it;
  }

  if(it->first<=max_)
    PLERROR("Bug in StatsCollector::getBinMapping expected last element of mapping to be FLT_MAX...");

  if (mapping.size() == 0)
    {
      PLWARNING("StatsCollector::getBinMapping: no mapping were created; probably a bug");
      mapping.addMapping(RealRange('[',min_,max_,']'), 0);
      return mapping;
    }

  // make sure we include max_
  pair<RealRange, real> m = mapping.lastMapping();

  // cnt is the number of elements that would be in the last bin
  double cnt = nnonmissing_ - count2 + count;
    
  // If the bin we're about to add is short of less then tolerance*100% of continuous_mincount elements, 
  // OR if the last we added is a discrete point AND the max is not already in the last range, we append it 
  if(m.first.high<max_)
  {
    if( ((real)cnt/(real)continuous_mincount)>(1.-tolerance) ||
        (m.first.low == m.first.high))
    {
      // don't join last bin with last-but-one bin
      mapping.addMapping(RealRange(m.first.rightbracket=='[' ? '[' : ']',m.first.high,max_,']'),
                         mapto++);
      if(fcount)
        fcount->append(cnt);
    }
    else
    {
      // otherwise, we can join it with the previous
      mapping.removeMapping(m.first);
      mapping.addMapping(RealRange(m.first.leftbracket, m.first.low, max_, ']'),
                         m.second);
      if(fcount)
      {
        double v = fcount->back();
        fcount->pop_back();
        fcount->append(v+cnt);
      }
    }   
  }
  else if(m.first.high==max_)  // make sure we have a closing bracket on the max_
    {
      mapping.removeMapping(m.first);
      mapping.addMapping(RealRange(m.first.leftbracket, m.first.low, max_, ']'),
                         m.second);
    }
  return mapping;
}


RealMapping StatsCollector::getAllValuesMapping(TVec<double> * fcount) const
{
  return getAllValuesMapping(0,fcount);
}

RealMapping StatsCollector::getAllValuesMapping(TVec<bool>* to_be_included,
                                                TVec<double>* fcount, bool ignore_other,
                                                real tolerance) const {
  RealMapping mapping;
  if (ignore_other) {
    mapping.keep_other_as_is = false;
    mapping.other_mapsto = -1;
  }
  int i = 0;
  int k = 0;
  if(fcount)
  {
    (*fcount) = TVec<double>();
    fcount->resize(0,counts.size()+2);
    fcount->append(nmissing_);
    fcount->append(0);
  }

  double count=0;
    
  real epsilon = 0;
  if (tolerance > 0) {
    // Compute the expansion coefficient 'epsilon'.
    StatsCollector values_diff;
    for (map<real,StatsCollectorCounts>::const_iterator it = counts.begin();
         size_t(i) < counts.size() - 2; i++) {
      real val1 = it->first;
      it++;
      real val2 = it->first;
      values_diff.update(val2 - val1);
    }
    // Mean of the difference between two consecutive values.
    real mean = values_diff.mean();
    epsilon = tolerance * mean;
    if (epsilon < 0) {
      PLERROR("In StatsCollector::getAllValuesMapping - epsilon < 0, there must be something wrong");
    }
  }

  i = 0;

  for(map<real,StatsCollectorCounts>::const_iterator it = counts.begin() ;
      size_t(i) < counts.size() - 1; ++it)
  {
    real low_val = it->first - epsilon;
    real up_val = it->first + epsilon;
    map<real,StatsCollectorCounts>::const_iterator itup = it;
    itup++;
    int j = i + 1;
    bool to_include = true;
    if (to_be_included) {
      to_include = (*to_be_included)[i];
    }
    real count_in_range = it->second.n;
    if (tolerance > 0) {
      for (; itup != counts.end(); itup++) {
        if (itup->first - epsilon <= up_val) {
          // The next mapping needs to be merged with the current one.
          if (fcount) {
            PLWARNING("In StatsCollector::getAllValuesMapping - You are using fcount and some ranges are merged. "
                "This case has not been tested yet. Please remove this warning if it works fine.");
          }
          up_val = itup->first + epsilon;
          count_in_range += itup->second.n;
          if (to_be_included) {
            // As long as one of the merged mappings needs to be included,
            // we include the result of the merge.
            to_include = to_include || (*to_be_included)[j];
          }
          j++;
        } else {
          // No merging.
          break;
        }
      }
    }
    // Because the last one won't be merged (even if all are merged, the one
    // with FLT_MAX won't).
    itup--;
    it = itup;
    i = j - 1;

    if (to_include) {
      mapping.addMapping(RealRange('[',low_val,up_val,']'),k);
      k++;
      if(fcount)
      {
        count += count_in_range;
        fcount->append(count_in_range);
      }
    }
    i++;
  }

  if(fcount)
    (*fcount)[1] = nnonmissing_ - count;
  return mapping;
}

/////////
// cdf //
/////////
Mat StatsCollector::cdf(bool normalized) const
{
  int l = 2*(int)counts.size();

  Mat xy(l+1,2);
  int i=0;
  double currentcount = 0;
  xy(i,0) = min_;
  xy(i++,1) = 0;    
  map<real,StatsCollectorCounts>::const_iterator it = counts.begin();
  map<real,StatsCollectorCounts>::const_iterator itend = counts.end();    
  for(; it!=itend; ++it)
  {
    real val = it->first;
    if(val>max_)
      val = max_;

    currentcount += it->second.nbelow;
    xy(i,0) = val;
    xy(i++,1) = currentcount;

    currentcount += it->second.n;
    xy(i,0) = val;
    xy(i++,1) = currentcount;        
  }
  if(normalized)
    xy.column(1) /= real(nnonmissing_);

  return xy;
}

real StatsCollector::pseudo_quantile(real q) const
{
  // Basic strategy is to iterate over the bins and stop when the fraction
  // of total observations crosses q.  Then we linearly interpolate between
  // the previous bin and the current one.
  map<real,StatsCollectorCounts>::const_iterator
    it = counts.begin(), end = counts.end();
  real previous_total = 0.0;
  real current_total = MISSING_VALUE;
  real previous_position = MISSING_VALUE;
  if (nnonmissing_ == 0)
    return MISSING_VALUE;
  
  for ( ; it != end ; ++it ) {
    current_total = previous_total + it->second.n + it->second.nbelow;
    if (is_missing(current_total) ||
        current_total / nnonmissing_ >= q)
      break;
    previous_total = current_total;
    previous_position = it->first;
  }

  // Boudary case if we did not collect any count statistics
  if (is_missing(current_total))
    return MISSING_VALUE;

  // If we stopped at the first bin, do not interpolate with previous bin
  assert( it != end );
  if (is_missing(previous_position))
    return it->first;

  // If we stopped at last bin, do not interpolate with current bin which
  // should be equal to FLT_MAX
  if (it->first == FLT_MAX)
    return previous_position;

  // Otherwise, interpolate linearly between previous_position and
  // current_position
  real current_position = it->first;
  real slope = (current_position - previous_position) /
                  (current_total - previous_total);
  return slope * (q * nnonmissing_ - previous_total) + previous_position;
}

void StatsCollector::print(ostream& out) const
{
  out << "# samples: " << n() << "\n";
  out << "# missing: " << nmissing() << "\n";
  out << "mean: " << mean() << "\n";
  out << "stddev: " << stddev() << "\n";
  out << "stderr: " << stderror() << "\n";
  out << "min: " << min() << "\n";
  out << "max: " << max() << "\n\n";
  out << "first: " << first_obs() << "\n";
  out << "last:  " << last_obs()  << "\n\n";
  out << "counts size: " << counts.size() << "\n";
  map<real,StatsCollectorCounts>::const_iterator it = counts.begin();
  map<real,StatsCollectorCounts>::const_iterator itend = counts.end();
  for(; it!=itend; ++it)
    {
      out << "value: " << it->first 
          << "  #equal:" << it->second.n
          << "  #less:" << it->second.nbelow
          << "  avg_of_less:" << it->second.sum/it->second.nbelow << endl;
    }
}

void StatsCollector::oldwrite(ostream& out) const
{
  writeHeader(out,"StatsCollector",0);
  writeField(out, "nmissing_", nmissing_);    
  writeField(out, "nnonmissing_", nnonmissing_);    
  writeField(out, "sum_", sum_);
  writeField(out, "sumsquare_", sumsquare_);
  writeField(out, "min_", min_);
  writeField(out, "max_", max_);
  writeField(out, "maxnvalues", maxnvalues);

  writeFieldName(out, "counts");
  PLearn::write(out, (int)counts.size());
  writeNewline(out);
  map<real,StatsCollectorCounts>::const_iterator it = counts.begin();
  map<real,StatsCollectorCounts>::const_iterator itend = counts.end();
  for(; it!=itend; ++it)
  {
    PLearn::write(out, it->first);
    PLearn::write(out, it->second.n);
    PLearn::write(out, it->second.nbelow);
    PLearn::write(out, it->second.sum);
    PLearn::write(out, it->second.sumsquare);
    writeNewline(out);
  }
  writeFooter(out,"StatsCollector");
}

void StatsCollector::oldread(istream& in)
{
  int version = readHeader(in,"StatsCollector");
  if(version!=0)
    PLERROR("In StatsCollector::oldead don't know how to read this version");
  readField(in, "nmissing_", nmissing_);    
  readField(in, "nnonmissing_", nnonmissing_);    
  readField(in, "sum_", sum_);
  readField(in, "sumsquare_", sumsquare_);
  readField(in, "min_", min_);
  readField(in, "max_", max_);
  readField(in, "maxnvalues", maxnvalues);

  readFieldName(in, "counts", true);
  counts.clear();
  int ncounts;
  PLearn::read(in, ncounts);
  readNewline(in);
  for(int i=0; i<ncounts; i++)
  {
    real value;
    StatsCollectorCounts c;
    PLearn::read(in, value);
    PLearn::read(in, c.n);
    PLearn::read(in, c.nbelow);
    PLearn::read(in, c.sum);
    PLearn::read(in, c.sumsquare);
    readNewline(in);
    counts[value] = c;
  }
  readFooter(in,"StatsCollector");
}


/////////////
// getStat //
/////////////
//! Returns the index in the vector returned by getAllStats of the stat with the given name.
//! Currently available names are E (mean) V (variance) STDDEV MIN MAX STDERROR SHARPERATIO
//! Will call PLERROR statname is invalid
real StatsCollector::getStat(const string& statname) const
{
  typedef real (StatsCollector::*STATFUN)() const;
  static bool init = false;
  static map<string,STATFUN> statistics;
  if (!init) {
    statistics["E"]           = STATFUN(&StatsCollector::mean);
    statistics["V"]           = STATFUN(&StatsCollector::variance);
    statistics["STDDEV"]      = STATFUN(&StatsCollector::stddev);
    statistics["STDERROR"]    = STATFUN(&StatsCollector::stderror);
    statistics["SKEW"]        = STATFUN(&StatsCollector::skewness);
    statistics["KURT"]        = STATFUN(&StatsCollector::kurtosis);
    statistics["MIN"]         = STATFUN(&StatsCollector::min);
    statistics["MAX"]         = STATFUN(&StatsCollector::max);
    statistics["RANGE"]       = STATFUN(&StatsCollector::range);
    statistics["SUM"]         = STATFUN(&StatsCollector::sum);
    statistics["SUMSQ"]       = STATFUN(&StatsCollector::sumsquare);
    statistics["FIRST"]       = STATFUN(&StatsCollector::first_obs);
    statistics["LAST"]        = STATFUN(&StatsCollector::last_obs);
    statistics["N"]           = STATFUN(&StatsCollector::n);
    statistics["NMISSING"]    = STATFUN(&StatsCollector::nmissing);
    statistics["NNONMISSING"] = STATFUN(&StatsCollector::nnonmissing);
    statistics["SHARPERATIO"] = STATFUN(&StatsCollector::sharperatio);
    statistics["ZSTAT"]       = STATFUN(&StatsCollector::zstat);
    statistics["PZ1t"]        = STATFUN(&StatsCollector::zpr1t);
    statistics["PZ2t"]        = STATFUN(&StatsCollector::zpr2t);
    statistics["IQR"]         = STATFUN(&StatsCollector::iqr);
    statistics["PRR"]         = STATFUN(&StatsCollector::prr);
    statistics["NIPS_LIFT"]   = STATFUN(&StatsCollector::nips_lift);
    init = true;
  }

  // Special case :: interpret the PSEUDOQ(xx) and LIFT(xxx) forms
  if (statname.substr(0,7) == "PSEUDOQ") {
    PIStringStream in(statname);
    string dummy;
    in.smartReadUntilNext("(", dummy);
    string quantile_str;
    in.smartReadUntilNext(")", quantile_str);
    real q = toreal(quantile_str);
    return pseudo_quantile(q);
  } else if (statname.substr(0, 5) == "LIFT(") {
    PIStringStream in(statname);
    string dummy;
    in.smartReadUntilNext("(", dummy);
    string fraction_str;
    in.smartReadUntilNext(")", fraction_str);
    real fraction = toreal(fraction_str);
    int dummy_int;
    return -100 * lift(int(floor(fraction * nnonmissing())), dummy_int);
  }
  
  map<string,STATFUN>::iterator fun = statistics.find(statname);
  if (fun == statistics.end())
    PLERROR("In StatsCollector::getStat, invalid statname %s",
            statname.c_str());
  else
    return (this->*(fun->second))();
  return 0;
}

//////////////
// skewness //
//////////////
real StatsCollector::skewness() const
{
  // numerator
  double diff = first_ - mean();
  double numerator = sumcube_/nnonmissing_ +
    (3*sumsquare_/nnonmissing_ + diff*(3*(sum_/nnonmissing_) + diff))*diff;

  // denominator
  double denominator = stddev();
  denominator *= denominator * denominator;
  return numerator / denominator;
}

//////////////
// kurtosis //
//////////////
real StatsCollector::kurtosis() const
{
  // numerator
  double diff = first_ - mean();
  double numerator = sumfourth_/nnonmissing_ +
    (4*sumcube_/nnonmissing_ +
     (6*sumsquare_/nnonmissing_ + diff*(4*sum_/nnonmissing_+diff)) * diff)
    * diff;

  // denominator
  double denominator = stddev();
  denominator *= denominator;
  denominator *= denominator;
  return numerator / denominator - 3.0;
}

//////////
// lift //
//////////
real StatsCollector::lift(int k, int& n_pos_in_k, int n_pos_in_k_minus_1, real pos_fraction) const
{
  if (more_than_maxnvalues)
    PLWARNING("In StatsCollector::lift - You need to increase 'maxnvalues' to get an accurate statistic");
  if (k <= 0)
    PLERROR("In StatsCollector::lift - It makes no sense to compute a lift with k <= 0");
  if (!sorted)
    sort_values_by_magnitude();
  if (n_pos_in_k_minus_1 < 0)
    // We are not given the number of positive examples in the first (k-1)
    // examples, thus we need to compute it ourselves.
    n_pos_in_k = int(floor(PLearn::sum(sorted_values.subMat(0, 1, k, 1))));
  else
    n_pos_in_k = n_pos_in_k_minus_1 + int(sorted_values(k - 1, 1));
  if (pos_fraction < 0)
    // We are not given the fraction of positive examples.
    pos_fraction = int(floor(PLearn::sum(sorted_values.column(1)))) / real(sorted_values.length());
  return real(n_pos_in_k) / (k * pos_fraction);
}

///////////////
// nips_lift //
///////////////
real StatsCollector::nips_lift() const
{
  if (more_than_maxnvalues)
    PLWARNING("In StatsCollector::nips_lift - You need to increase 'maxnvalues' to get an accurate statistic");
  if (!sorted)
    sort_values_by_magnitude();
  real n_total = real(sorted_values.length());
  real pos_fraction = int(floor(PLearn::sum(sorted_values.column(1)))) / n_total;
  int n_pos_in_k_minus_1 = -1;
  real result = 0;
  for (int k = 0; k < sorted_values.length(); k++)
    result += lift(k + 1, n_pos_in_k_minus_1, n_pos_in_k_minus_1, pos_fraction);
  result /= n_total;
  real max_performance = 0.5 * (1 / pos_fraction - 1) * (pos_fraction + 1) + 1;
  result = (max_performance - result) / max_performance;
  return result;
}

//////////////////////////////
// sort_values_by_magnitude //
//////////////////////////////
void StatsCollector::sort_values_by_magnitude() const
{
  sorted_values.resize(0, 2);
  Vec to_add(2);
  real val;
  for (map<real,StatsCollectorCounts>::const_iterator it = counts.begin();
       it != counts.end(); it++) {
    val = it->first;
    to_add[0] = fabs(val);
    to_add[1] = val > 0 ? 1 : 0;
    for (int i = 0; i < it->second.n; i++)
      sorted_values.appendRow(to_add);
  }
  sortRows(sorted_values, 0, false); // Sort by decreasing order of first column.
  sorted = true;
}

///////////////////
// computeRanges //
///////////////////
TVec<RealMapping> computeRanges(TVec<StatsCollector> stats, int discrete_mincount, int continuous_mincount)
{
  TVec<RealMapping> ranges;
  int n = stats.length();
  ranges.resize(n);
  for(int k=0; k<n; k++)
    ranges[k] = stats[k].getBinMapping(discrete_mincount, continuous_mincount);
  return ranges;
}

real StatsCollector::zpr1t() const
{
  real m = mean(), v = variance();
  if (is_missing(m) || is_missing(v))
    return MISSING_VALUE;
  else
    return p_value(mean(), variance()/nnonmissing());
}

real StatsCollector::zpr2t() const
{
  return 2 * zpr1t();
}

} // end of namespace PLearn


