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
   * $Id: StatsCollector.cc,v 1.21 2003/10/14 20:39:30 ducharme Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "StatsCollector.h"
#include "TMat_maths.h"

namespace PLearn <%
using namespace std;

  PLEARN_IMPLEMENT_OBJECT(StatsCollector, "ONE LINE DESCR", "NO HELP");

  StatsCollector::StatsCollector(int the_maxnvalues)
    : maxnvalues(the_maxnvalues),
      nmissing_(0.), nnonmissing_(0.), 
      sum_(0.), sumsquare_(0.), 
      min_(MISSING_VALUE), max_(MISSING_VALUE),
      first_(MISSING_VALUE), last_(MISSING_VALUE)
  {
    if(maxnvalues>0)
      counts[FLT_MAX] = StatsCollectorCounts();
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

  // learnt options
  declareOption(ol, "nmissing_", &StatsCollector::nmissing_, OptionBase::learntoption,
                "number of missing values");
  declareOption(ol, "nnonmissing_", &StatsCollector::nnonmissing_, OptionBase::learntoption,
                "number of non missing value ");
  declareOption(ol, "sum_", &StatsCollector::sum_, OptionBase::learntoption,
                "sum of all values");
  declareOption(ol, "sumsquare_", &StatsCollector::sumsquare_, OptionBase::learntoption,
                "sum of square of all values ");
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

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void StatsCollector::build_()
{
  // make sure count.size==0. If not, the object must have been loaded, and FLT_MAX is an existing key
  // but rounded to some precision, and it would there would be 2 keys approx.=  FLT_MAX
  if(maxnvalues>0 && counts.size()==0)
    counts[FLT_MAX] = StatsCollectorCounts();
}

void StatsCollector::build()
{
  inherited::build();
  build_();
}

string StatsCollector::help()
{
  // ### Provide some useful description of what the class is ...
  return 
    "A StatsCollector allows to compute basic global statistics for a series of numbers,\n"
    "as well as statistics within automatically determined ranges.\n"
    "The first maxnvalues encountered values will be used as reference points to define\n"
    "the ranges, so to get reasonable results, your sequence should be iid, and NOT sorted!"
  + optionHelp();
}

void StatsCollector::forget()
{
    nmissing_ = 0.;
    nnonmissing_ = 0.;
    sum_ = 0.;
    sumsquare_ = 0.;
    min_ = MISSING_VALUE;
    max_ = MISSING_VALUE;
    first_ = last_ = MISSING_VALUE;
}

void StatsCollector::update(real val, real weight)
{
  if(is_missing(val))
    nmissing_ += weight;
  else
  {
    sum_ += val * weight;
    sumsquare_ += val*val * weight;
    last_ = val;
    if(nnonmissing_==0)                      // first value encountered
      min_ = max_ = first_ = last_ = val;
    else if(val<min_)
      min_ = val;
    else if(val>max_)
      max_ = val;
    nnonmissing_ += weight;
        
    if(maxnvalues>0)  // also remembering statistics inside values ranges
    {
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
          it->second.nbelow += weight;
          it->second.sum += val * weight;
          it->second.sumsquare += val*val * weight;
        }
      }
    }
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
  int nleft = counts.size()-1; // loop on all but last

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
  ProgressBar pb("Computing PseudoQ Mapping...",counts.size()-1);

  while(nleft--)
  {
    high = it->first;
    pb(counts.size()-1-nleft);
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
    PLERROR("StatsCollector::getBinMapping: no mapping were created; probably a bug");
  
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
      mapping.addMapping(RealRange(m.first.leftbracket,m.first.high,max_,']'),
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
  return mapping;
}


RealMapping StatsCollector::getAllValuesMapping(TVec<double> * fcount) const
{
  RealMapping mapping;
  int i=0;
  if(fcount)
  {
    (*fcount) = TVec<double>();
    fcount->resize(0,counts.size()+2);
    fcount->append(nmissing_);
    fcount->append(0);
  }

  double count=0;
    
  for(map<real,StatsCollectorCounts>::const_iterator it = counts.begin() ;
      it!=counts.end(); ++it, ++i)
  {
//    cout<<it->first<<" "<<FLT_MAX<<endl;

    if(it->first!=FLT_MAX)
      mapping.addMapping(RealRange('[',it->first,it->first,']'),i);
    if(fcount)
    {
      count += it->second.n;       
      fcount->append(it->second.n);
    }
  }
  if(fcount)
    (*fcount)[1] = nnonmissing_ - count;
  return mapping;
}

Mat StatsCollector::cdf(bool normalized) const
{
  int l = 2*counts.size();

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
  /*
    map<real,Counts>::const_iterator it = counts.begin();
    map<real,Counts>::const_iterator itend = counts.end();
    for(; it!=itend; ++it)
    {
    out << "value: " << it->first 
    << "  #equal:" << it->second.n
    << "  #less:" << it->second.nbelow
    << "  avg_of_less:" << it->second.sum/it->second.nbelow << endl;
    }
  */
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


//! Returns the index in the vector returned by getAllStats of the stat with the given name.
//! Currently available names are E (mean) V (variance) STDDEV MIN MAX STDERROR SHARPERATIO
//! Will call PLERROR statname is invalid
real StatsCollector::getStat(const string& statname) const
{
  typedef real (StatsCollector::*STATFUN)() const;
  static bool init = false;
  static map<string,STATFUN> statistics;
  if (!init) {
    statistics["E"]           = &StatsCollector::mean;
    statistics["V"]           = &StatsCollector::variance;
    statistics["STDDEV"]      = &StatsCollector::stddev;
    statistics["STDERROR"]    = &StatsCollector::stderror;
    statistics["MIN"]         = &StatsCollector::min;
    statistics["MAX"]         = &StatsCollector::max;
    statistics["SUM"]         = &StatsCollector::sum;
    statistics["SUMSQ"]       = &StatsCollector::sumsquare;
    statistics["FIRST"]       = &StatsCollector::first_obs;
    statistics["LAST"]        = &StatsCollector::last_obs;
    statistics["N"]           = &StatsCollector::n;
    statistics["NMISSING"]    = &StatsCollector::nmissing;
    statistics["NNONMISSING"] = &StatsCollector::nnonmissing;
    statistics["SHARPERATIO"] = &StatsCollector::sharperatio;
    init = true;
  }
  
  map<string,STATFUN>::iterator fun = statistics.find(statname);
  if (fun == statistics.end())
    PLERROR("In StatsCollector::getStat, invalid statname %s",
            statname.c_str());
  else
    return (this->*(fun->second))();
  return 0;
}

  // *********************************
  // *** ConditionalStatsCollector ***
  // *********************************

PLEARN_IMPLEMENT_OBJECT(ConditionalStatsCollector, "ONE LINE DESCR", "NO HELP");

ConditionalStatsCollector::ConditionalStatsCollector()
  :condvar(0) {}

void ConditionalStatsCollector::setBinMappingsAndCondvar(const TVec<RealMapping>& the_ranges, int the_condvar) 
{ 
  ranges = the_ranges;
  condvar = the_condvar;
  int nvars = ranges.length();
  counts.resize(nvars);
  sums.resize(nvars);
  sumsquares.resize(nvars);
  int nranges_condvar = ranges[condvar].length();
  for(int k=0; k<nvars; k++)
  {        
    int nranges_k = ranges[k].length();
    counts[k].resize(nranges_k+1, nranges_condvar+1);
    sums[k].resize(nranges_k, nranges_condvar);
    sumsquares[k].resize(nranges_k, nranges_condvar);
  }
}

int ConditionalStatsCollector::findrange(int varindex, real val) const
{
  RealMapping& r = ranges[varindex];
  if(is_missing(val))
    return r.length();
  else
    return (int) r.map(val);
}
  
void ConditionalStatsCollector::update(const Vec& v)
{
  int nvars = ranges.length();
  if(v.length()!=nvars)
    PLERROR("IN ConditionalStatsCollectos::update length of update vector and nvars differ!");
  int j = findrange(condvar, v[condvar]);
  if(j==-1)
    PLWARNING("In ConditionalStatsCollector::update value of conditioning var in none of the ranges");
  for(int k=0; k<nvars; k++)
  {
    real val = v[k];
    int i = findrange(k, val);
    if(i==-1)
      PLWARNING("In ConditionalStatsCollector::update value of variable #%d in none of the ranges",k);
    counts[k](i,j)++;
    if(!is_missing(val))
    {
      sums[k](i,j) += val;
      sumsquares[k](i,j) += val;
    }
  }
}

void ConditionalStatsCollector::write(ostream& out) const
{
  writeHeader(out,"ConditionalStatsCollector",0);
  writeField(out, "condvar", condvar);    
  writeField(out, "ranges", ranges);    
  writeField(out, "counts", counts);
  writeField(out, "sums", sums);
  writeField(out, "sumsquares", sumsquares);
  writeFooter(out,"ConditionalStatsCollector");
}

void ConditionalStatsCollector::oldread(istream& in)
{
  int version = readHeader(in,"ConditionalStatsCollector");
  if(version!=0)
    PLERROR("In ConditionalStatsCollector::oldead don't know how to read this version");
  readField(in, "condvar", condvar);    
  readField(in, "ranges", ranges);    
  readField(in, "counts", counts);
  readField(in, "sums", sums);
  readField(in, "sumsquares", sumsquares);
  readFooter(in,"ConditionalStatsCollector");
}

TVec<RealMapping> computeRanges(TVec<StatsCollector> stats, int discrete_mincount, int continuous_mincount)
{
  TVec<RealMapping> ranges;
  int n = stats.length();
  ranges.resize(n);
  for(int k=0; k<n; k++)
    ranges[k] = stats[k].getBinMapping(discrete_mincount, continuous_mincount);
  return ranges;
}

%> // end of namespace PLearn


