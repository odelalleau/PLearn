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
   * $Id: SDBWithStats.cc,v 1.2 2004/02/20 21:11:43 chrish42 Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SDBWithStats.h"

namespace PLearn {
using namespace std;

  int FieldStat::max_nsymbols = 400;

  void FieldStat::updateString(const string& sym)
  {
    ++nonmissing_;
    if(nsymbols()<max_nsymbols)
      symbolcount[sym]++;
  }

  void FieldStat::updateNumber(double d)
  {
    ++nonmissing_;
    if(nsymbols()<max_nsymbols)
      symbolcount[tostring(d)]++;
    sum_ += d;
    sumsquare_ += d*d;
    if(d<min_)
      min_ = d;
    if(d>max_)
      max_ = d;
  }

  void FieldStat::clear()
  {
    nonmissing_ = 0;
    missing_ = 0; 
    sum_ = 0;
    sumsquare_ = 0;
    min_ = FLT_MAX;
    max_ = -FLT_MAX;
    mean_ = 0;
    stddev_ = 0;
    symbolcount.clear();
    symbolid.clear();
  }

  void FieldStat::finalize()
  {
    mean_ = sum_/nonmissing_;
    double meansquare_ = sumsquare_/nonmissing_;
    stddev_ = sqrt( meansquare_ - square(mean_) );
    if(nsymbols()>=max_nsymbols) // too many different values: ignore it (not really symbolic)
    {
      symbolcount.clear();
      symbolid.clear();
    }
  }

  SDBWithStats::SDBWithStats(string basename, string path, AccessType access, bool verbose)
    :SDB(basename,path,access,verbose)
  { 
    fieldstat.resize(getSchema().size());
    if(hasStats())
      loadStats();
  }

  void SDBWithStats::forgetStats()
  {
    for(int j=0; j<width(); j++)
      getStat(j).clear();
  }

  void SDBWithStats::computeStats(unsigned int nrows)
  {
    forgetStats();
    const Schema& sc = getSchema();
    Row row(&sc);

    for(SDB::RowNumber i=0; i<nrows;i++) 
    {
      getInRow(i, row);
      Row::const_iterator it = row.begin();
      if (nrows>100000 && i%100000==0) 
        cout << "SDBWithStats::computeStats processing row " << i << " of " << nrows << endl;

      for (int j=0; j<nfields(); ++j, ++it) 
      {
        if(it.isMissing())
          fieldstat[j].updateMissing();
        else
        {
          switch(it.getFieldType())
          {
            case StringType: 
            case CharacterType:
              fieldstat[j].updateString(tostring(it));
              break;
            case SignedCharType:
            case ShortType:
            case IntType:
            case FloatType:
            case DoubleType:
            case DateType:
              fieldstat[j].updateNumber(todouble(it));
              break;
            default: 
              PLERROR("Unknown field type");
          }
        }
      }
    }    
    cout << "boucle terminee" << endl;
    for (int j=0; j<nfields(); ++j)
      fieldstat[j].finalize();
    cout << "fini computestats" << endl;
  }

  bool SDBWithStats::hasStats()
  {
    string numstatsfile = getPath()+getName()+".stats"; 
    string symstatsfile = getPath()+getName()+".symbols";
    return file_exists(numstatsfile.c_str()) && file_exists(symstatsfile.c_str());
  }

  void SDBWithStats::saveStats()
  {
    string numstatsfile = getPath()+getName()+".stats"; 
    string symstatsfile = getPath()+getName()+".symbols";
    
    ofstream numstats(numstatsfile.c_str());
    if(!numstats)
      PLERROR("could not open file %s for writing",numstatsfile.c_str());

    ofstream symstats(symstatsfile.c_str());
    if(!symstats)
      PLERROR("could not open file %s for writing",symstatsfile.c_str());
      
    numstats.precision(8);
    symstats.precision(8);

    for(unsigned int j=0; j<fieldstat.size(); j++)
      {
        FieldStat& s = fieldstat[j];
        numstats << fieldname(j) << ' ' << s.nonmissing() << ' ' << s.missing() << ' ' 
                 << s.mean() << ' ' << s.stddev() << ' ' << s.min() << ' ' << s.max() << endl;

        symstats << fieldname(j) << ' ';
        symstats << s.nsymbols() << "     ";
        map<string,int>::iterator it;
        for(it = s.symbolcount.begin(); it!= s.symbolcount.end(); ++it)
          symstats << it->first << ' ' << it->second << "   ";
        symstats << endl;
      }
  }

  void SDBWithStats::loadStats()
  {
    forgetStats();
    string numstatsfile = getPath()+getName()+".stats"; 
    string symstatsfile = getPath()+getName()+".symbols";
    ifstream numstats(numstatsfile.c_str());
    if(!numstats)
      PLERROR("could not open file %s for reading",numstatsfile.c_str());

    ifstream symstats(symstatsfile.c_str());
    if(!symstats)
      PLERROR("could not open file %s for reading",symstatsfile.c_str());
      
    for(unsigned int j=0; j<fieldstat.size(); j++)
    {
        string str_nonmissing_,str_missing_,str_mean_,str_stddev_,str_min_,str_max_;
        FieldStat& s = fieldstat[j];
        string name;
        numstats >> name;

        //cout<<"field : "<<j<<" name:"<<name<<" fieldname(j):"<<fieldname(j)<<endl;
        if(name!=fieldname(j))
          PLERROR("Row number %d of file %s does not correpond to field number %d: %s",j,numstatsfile.c_str(),j,fieldname(j).c_str()); 

        // **** we use strings as intermediate type to handle nans (julien)
        numstats >> str_nonmissing_ >> str_missing_ >> str_mean_ >> str_stddev_ >> str_min_ >> str_max_;
        
        if(str_mean_=="nan")str_mean_="";
        if(str_stddev_=="nan")str_stddev_="";
        if(str_min_=="nan")str_min_="";        
        if(str_max_=="nan")str_max_="";
        
        s.nonmissing_ = toint(str_nonmissing_);
        s.missing_ = toint(str_missing_);
        s.mean_ = tofloat(str_mean_);
        s.stddev_ = tofloat(str_stddev_);
        s.min_ = tofloat(str_min_);
        s.max_ = tofloat(str_max_);

        //cout <<" "<< s.nonmissing_ <<" "<< s.missing_ <<" "<< s.mean_ <<" "<< s.stddev_ <<" "<< s.min_ <<" "<< s.max_<<endl;
                
        symstats >> name;
        if(name!=fieldname(j))
          PLERROR("Row number %d of file %s does not correpond to field number %d: %s",j,symstatsfile.c_str(),j,fieldname(j).c_str()); 

        int nsymbols;
        symstats >> nsymbols;
        string sym;
        int symcount;
        for(int k=0; k<nsymbols; k++)
          {
            symstats >> sym >> symcount;
            s.symbolcount[sym] = symcount;
            s.symbolid[sym]=k;
          }
      }
    
  }

  FieldStat& SDBWithStats::getStat(int i) 
  {
    if(i<0 || i>=width())
      PLERROR("Out of bounds");
    return fieldstat[i]; 
  }

  const FieldStat& SDBWithStats::getStat(int i) const
  {
    if(i<0 || i>=width())
      PLERROR("Out of bounds");
    return fieldstat[i]; 
  }

  FieldStat& SDBWithStats::getStat(const string& fieldname) 
  { 
    int pos = indexOfField(fieldname);
    if(pos<0)
      PLERROR("No field named %s",fieldname.c_str());
    return getStat(pos); 
  }

  const FieldStat& SDBWithStats::getStat(const string& fieldname) const
  { 
    int pos = indexOfField(fieldname);
    if(pos<0)
      PLERROR("No field named %s",fieldname.c_str());
    return getStat(pos); 
  }

  
} // end of namespace PLearn
