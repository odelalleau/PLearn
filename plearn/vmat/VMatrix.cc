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


 
/*
* $Id: VMatrix.cc,v 1.3 2002/10/21 01:21:53 plearner Exp $
******************************************************* */

#include "VMatrix.h"
#include "SubVMatrix.h"
#include "FileVMatrix.h"
#include "DiskVMatrix.h"
#include "VMat_maths.h"

#include "Kernel.h"
#include "Func.h"
#include "TopNI.h"
#include "BottomNI.h"

//#include "VMat.h"
//#include "TMat_maths.h"
//#include "Array.h"
//#include "random.h"
//#include "Kernel.h"
//#include "Func.h"
//#include "TmpFilenames.h"
//#include "fileutils.h"
//#include <vector>
//#include "TopNI.h"
//#include "BottomNI.h"
//#include "VVMatrix.h"
// #include "DisplayUtils.h"


namespace PLearn <%
using namespace std;

/** VMatrix **/

IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY(VMatrix);

void VMatrix::declareOptions(OptionList & ol)
{
//  declareOption(ol, "writable", &VMatrix::writable, OptionBase::buildoption, "Are write operations permitted?");
  declareOption(ol, "length", &VMatrix::length_, OptionBase::buildoption, "length of the matrix");
  declareOption(ol, "width", &VMatrix::width_, OptionBase::buildoption, "width of the matrix");
}

void VMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  deepCopyField(fieldinfos, copies);
  deepCopyField(fieldstats, copies);
}

Array<VMField>& VMatrix::getFieldInfos() const
{
  int w = width();
  int ninfos = fieldinfos.size();
  if(ninfos!=w)
    {
      fieldinfos.resize(w);
      for(int j=ninfos; j<w; j++)
        fieldinfos[j] = VMField(tostring(j));
    }
  return fieldinfos;
}

void VMatrix::unduplicateFieldNames()
{
  map<string,vector<int> > mp;
  for(int i=0;i<width();i++)
    mp[getFieldInfos(i).name].push_back(i);
  map<string,vector<int> >::iterator it;
  for(it=mp.begin();it!=mp.end();++it)
    if(it->second.size()!=1)
      {
        vector<int> v=it->second;
        for(unsigned int j=0;j<v.size();j++)
          fieldinfos[v[j]].name+="."+tostring(j);
      }
}

int VMatrix::fieldIndex(const string& fieldname) const
{
  Array<VMField>& infos = getFieldInfos(); 
  for(int i=0; i<width(); i++)
    if(infos[i].name==fieldname)
      return i;
  return -1;
}

void VMatrix::printFields(ostream& out) const
{ 
  for(int j=0; j<width(); j++)
  {
    out << "Field #" << j << ":  " << getFieldInfos(j);
    if(fieldstats.size()>0)
    {
      const VMFieldStat& s = fieldStat(j);
      out << "nmissing: " << s.nmissing() << '\n';
      out << "nnonmissing: " << s.nnonmissing() << '\n';
      out << "npositive: " << s.npositive() << '\n';
      out << "nzero: " << s.nzero() << '\n';
      out << "nnegative: " << s.nnegative() << '\n';
      out << "mean: " << s.mean() << '\n';
      out << "stddev: " << s.stddev() << '\n';
      out << "min: " << s.min() << '\n';
      out << "max: " << s.max() << '\n';
      if(!s.counts.empty())
      {
        out << "value:counts :   ";
        map<real,int>::const_iterator it = s.counts.begin();
        map<real,int>::const_iterator countsend = s.counts.end();
        while(it!=countsend)
        {
          out << it->first << ':' << it->second << "  "; 
          ++it;
        }
      }
      out << endl << endl;
    }
  }
}

void VMatrix::computeStats()
{
  fieldstats = Array<VMFieldStat>(width());
  Vec row(width());
  for(int i=0; i<length(); i++)
    {
      getRow(i,row);
      for(int j=0; j<width(); j++)
        fieldstats[j].update(row[j]);
    }
}

void VMatrix::writeStats(ostream& out) const
{
  out << fieldstats.size() << endl;
  for(int j=0; j<fieldstats.size(); j++)
  {
    fieldstats[j].write(out);
    out << endl;
  }
}

void VMatrix::readStats(istream& in)
{
  int nfields;
  in >> nfields;
  if(nfields!=width())
    PLWARNING("In VMatrix::loadStats nfields differes from VMat width");

  fieldstats.resize(nfields);
  for(int j=0; j<fieldstats.size(); j++)
    fieldstats[j].read(in);
}

void VMatrix::loadStats(const string& filename)
{
  ifstream in(filename.c_str());
  if(!in)
    PLERROR("In VMatrix::loadStats Couldn't open file %s for reading",filename.c_str());
  readStats(in);
}

void VMatrix::saveStats(const string& filename) const
{
  ofstream out(filename.c_str());
  if(!out)
    PLERROR("In VMatrix::saveStats Couldn't open file %s for writing",filename.c_str());
  writeStats(out);
}


string VMatrix::fieldheader(int elementcharwidth)
{
  // Implementation not done yet

  return "VMatrix::fieldheader NOT YET IMPLEMENTED";
}

void VMatrix::saveFieldInfos(const string& filename) const
{
  ofstream f(filename.c_str());
  if(!f)
    PLERROR("In VMatrix::saveFieldInfos Couldn't open file %s for writing",filename.c_str());    
  writeFieldInfos(f);
}
void VMatrix::loadFieldInfos(const string& filename)
{
  ifstream f(filename.c_str());
  if(!f)
    PLERROR("In VMatrix::loadFieldInfos Couldn't open file %s for reading",filename.c_str());    
  readFieldInfos(f);
}
void VMatrix::writeFieldInfos(ostream& out) const
{
  for(int i= 0; i < fieldinfos.length(); ++i)
    out << fieldinfos[i].name << '\t' << fieldinfos[i].fieldtype << endl;
}
void VMatrix::readFieldInfos(istream& in)
{
  for(int i= 0; i < width(); ++i)
    {
      vector<string> v(split(pgetline(in)));
      switch(v.size())
	{
	case 1: declareField(i, v[0]); break;
	case 2: declareField(i, v[0], VMField::FieldType(toint(v[1]))); break;
	default: PLERROR("In VMatrix::readFieldInfos Format not recognized.  Each line should be '<name> {<type>}'.");
	}
    }
}

string VMatrix::getValString(int col, real val) const
{ return ""; }

real VMatrix::getStringVal(int col,const string & str) const
{
  return MISSING_VALUE;
}

string VMatrix::getString(int row,int col) const
{
  return tostring(get(row,col));
}

void VMatrix::setMetaDataDir(const string& the_metadatadir) 
{ 
  metadatadir = the_metadatadir; 
  if(!force_mkdir(metadatadir))
    PLERROR("In VMatrix::setMetadataDir could not create directory %s",metadatadir.c_str());
  metadatadir = abspath(metadatadir);
}

//! returns the unconditonal statistics for the given field
TVec<StatsCollector> VMatrix::getStats()
{
  if(!field_stats)
  {
    string statsfile = getMetaDataDir() + "/stats.psave";
    if (isfile(statsfile) && getMtime()<mtime(statsfile))
    {
      PLearn::load(statsfile, field_stats);
    }
    else
    {
      field_stats = PLearn::computeStats(this, 2000);
      PLearn::save(statsfile, field_stats);
    }
  }
  return field_stats;
}

TVec<RealMapping> VMatrix::getRanges()
{
  TVec<RealMapping> ranges;
  string rangefile = getMetaDataDir() + "/ranges.psave";
  if(isfile(rangefile))
    PLearn::load(rangefile, ranges);
  else
  {
    ranges = computeRanges(getStats(),std::max(10,length()/200),std::max(10,length()/100) );
    PLearn::save(rangefile, ranges);
  }
  return ranges;
}

//! returns the cooccurence statistics conditioned on the given field
PP<ConditionalStatsCollector> VMatrix::getConditionalStats(int condfield)
{
  PP<ConditionalStatsCollector> condst;
  TVec<RealMapping> ranges = getRanges();
  string condstatfile = getMetaDataDir() + "/stats" + tostring(condfield) + ".psave";      
  string rangefile = getMetaDataDir() + "/ranges.psave";
  cerr <<  "rangefile: " << mtime(rangefile) << " condstatfile: " << mtime(condstatfile) << endl;
  if(mtime(rangefile)>mtime(condstatfile))
    {
      cerr << ">> Computing conditional stats conditioned on field " << condfield << endl;
      cerr << "   (because file " << rangefile << " was more recent than cache file " << condstatfile << ")" << endl; 
      condst = computeConditionalStats(this, condfield, ranges);
      PLearn::save(condstatfile, *condst);
    }
  else
    PLearn::load(condstatfile, *condst);      
  return condst;
}

// Eventually to be changed to pure virtual, once get has been implemented in all subclasses
// calls to sample can then be replaced by getRow everywhere
real VMatrix::get(int i, int j) const
{
  PLERROR("get(i,j) method not implemented for this VMat (name=%s), please implement.",classname().c_str());
  return 0.0;
}

void VMatrix::put(int i, int j, real value)
{
  PLERROR("put(i,j,value) method not implemented for this VMat, please implement.");
}

void VMatrix::getColumn(int j, Vec v) const
{
#ifdef BOUNDCHECK
  if(v.length() != length())
    PLERROR("In VMatrix::getColumn v must have the same length as the VMatrix");
#endif
  for(int i=0; i<v.length(); i++)
    v[i] = get(i,j);
}

void VMatrix::getSubRow(int i, int j, Vec v) const
{
  for(int k=0; k<v.length(); k++)
    v[k] = get(i,j+k);
}

void VMatrix::putSubRow(int i, int j, Vec v)
{
  for(int k=0; k<v.length(); k++)
    put(i, j+k, v[k]);
}

void VMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if(v.length() != width())
    PLERROR("In VMatrix::getRow(i,v) length of v and width of VMatrix differ");
#endif
  getSubRow(i,0,v);
}

void VMatrix::putRow(int i, Vec v)
{
#ifdef BOUNDCHECK
  if(v.length() != width())
    PLERROR("In VMatrix::putRow(i,v) length of v and width of VMatrix differ");
#endif
  putSubRow(i,0,v);
}

void VMatrix::fill(real value)
{
  Vec v(width(), value);
  for (int i=0; i<length(); i++) putRow(i,v);
}

void VMatrix::appendRow(Vec v)
{
  PLERROR("This method (appendRow) not implemented by VMatrix subclass!");
}

void VMatrix::getMat(int i, int j, Mat m) const
{
#ifdef BOUNDCHECK
  if(i<0 || j<0 || i+m.length()>length() || j+m.width()>width())
    PLERROR("In VMatrix::getMat(i,j,m) OUT OF BOUNDS");
#endif
  for(int ii=0; ii<m.length(); ii++)
    {
      Vec v = m(ii);
      getSubRow(i+ii, j, v);
    }
}

void VMatrix::putMat(int i, int j, Mat m)
{
#ifdef BOUNDCHECK
  if(i<0 || j<0 || i+m.length()>length() || j+m.width()>width())
    PLERROR("In VMatrix::putMat(i,j,m) OUT OF BOUNDS");
#endif
  for(int ii=0; ii<m.length(); ii++)
    {
      Vec v = m(ii);
      putSubRow(i+ii, j, v);
    }
}

void VMatrix::compacify() {}


Mat VMatrix::toMat() const
{
  Mat m(length(),width());
  getMat(0,0,m);
  return m;
}

VMat VMatrix::subMat(int i, int j, int l, int w)
{ return new SubVMatrix(this,i,j,l,w); }

real VMatrix::dot(int i1, int i2, int inputsize) const
{ 
  real res = 0.;
  for(int k=0; k<inputsize; k++)
    res += get(i1,k)*get(i2,k);
  return res;
}

real VMatrix::dot(int i, const Vec& v) const
{
  real res = 0.;
  for(int k=0; k<v.length(); k++)
    res += get(i,k) * v[k];
  return res;
}

void VMatrix::getRow(int i, VarArray& inputs) const
{ 
  Vec v(width());
  getRow(i,v);
  inputs << v; 
}

void VMatrix::print(ostream& out) const
{
  Vec v(width());
  for(int i=0; i<length(); i++)
    {
      getRow(i,v);
      out << v << endl;
    }
}

void VMatrix::oldwrite(ostream& out) const
{
  writeHeader(out,"VMatrix");
  writeField(out,"length_", length_);
  writeField(out,"width_", width_);
  //writeField(out,"fieldinfos", fieldinfos);
  //writeField(out,"fieldstats", fieldstats);
  writeFooter(out,"VMatrix");
}

void VMatrix::oldread(istream& in)
{
  readHeader(in,"VMatrix");
  readField(in,"length_", length_);
  readField(in,"width_", width_);
  //readField(in,"fieldinfos", fieldinfos);
  //readField(in,"fieldstats", fieldstats);
  readFooter(in,"VMatrix");
}

VMatrix::~VMatrix()
{}

void VMatrix::save(const string& filename) const
{ savePMAT(filename); }

void VMatrix::savePMAT(const string& pmatfile) const
{
  if (width() == -1)
    PLERROR("In VMat::save Saving in a pmat file is only possible for constant width Distributions (where width()!=-1)");

  int nsamples = length();

  FileVMatrix m(pmatfile,nsamples,width());
  Vec tmpvec(width());

  ProgressBar pb(cout, "Saving to pmat", nsamples);

  for(int i=0; i<nsamples; i++)
    {
      getRow(i,tmpvec);
      m.putRow(i,tmpvec);
      pb(i);
    }
  string fieldinfosfname = pmatfile+".fieldnames";
  saveFieldInfos(fieldinfosfname);  
}

void VMatrix::saveDMAT(const string& dmatdir) const
{
  force_rmdir(dmatdir);  
  DiskVMatrix vm(dmatdir,width());
  Vec v(width());

  ProgressBar pb(cout, "Saving to dmat", length());

  for(int i=0;i<length();i++)
    {
      getRow(i,v);
      vm.appendRow(v);
      pb(i);
      //cerr<<i<<" "<<flush;
    }

  //save field names
  string fieldinfosfname = dmatdir+"/fieldnames";
  saveFieldInfos(fieldinfosfname);  
}

void VMatrix::saveAMAT(const string& amatfile) const
{
  ofstream out(amatfile.c_str());
  if (!out)
   PLERROR("In saveAscii could not open file %s for writing",amatfile.c_str());

  out << "#size: "<< length() << ' ' << width() << endl;
  out.precision(15);
  if(width()>0)
    {
      out << "#: ";
      for(int k=0; k<width(); k++)
	//there must not be any space in a field name...
        out << space_to_underscore(fieldName(k)) << ' ';
      out << "\n";
    }

  Vec v(width());

  ProgressBar pb(cout, "Saving to amat", length());

  for(int i=0;i<length();i++)
    {
      getRow(i,v);
      out << v << "\n";
      pb(i);
    }
}

    // This will compute for this vmat m a result vector (whose length must be tha same as m's)
    // s.t. result[i] = ker( m(i).subVec(v1_startcol,v1_ncols) , v2) 
    // i.e. the kernel value betweeen each (sub)row of m and v2
void VMatrix::evaluateKernel(Ker ker, int v1_startcol, int v1_ncols, 
                             const Vec& v2, const Vec& result, int startrow, int nrows) const
{
  int endrow = (nrows>0) ?startrow+nrows :length_;
  if(result.length() != endrow-startrow)
    PLERROR("In VMatrix::evaluateKernel length of result vector does not match the row range");

  Vec v1(v1_ncols);
  for(int i=startrow; i<endrow; i++)
  {
    getSubRow(i,v1_startcol,v1);
    result[i] = ker(v1,v2);
  }
}

    //  returns sum_i [ ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
real VMatrix::evaluateKernelSum(Ker ker, int v1_startcol, int v1_ncols, 
                                const Vec& v2, int startrow, int nrows, int ignore_this_row) const
{
  int endrow = (nrows>0) ?startrow+nrows :length_;
  double result = 0.;
  Vec v1(v1_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,v1_startcol,v1);
      result += ker(v1,v2);
    }
  return (real)result;
}
    
    // targetsum := sum_i [ m(i).subVec(t_startcol,t_ncols) * ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
    // and returns sum_i [ ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
real VMatrix::evaluateKernelWeightedTargetSum(Ker ker, int v1_startcol, int v1_ncols, const Vec& v2, 
                                                 int t_startcol, int t_ncols, Vec& targetsum, int startrow, int nrows, int ignore_this_row) const
{
  int endrow = (nrows>0) ?startrow+nrows :length_;
  targetsum.clear();
  double result = 0.;
  Vec v1(v1_ncols);
  Vec target(t_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,v1_startcol,v1);
      getSubRow(i,t_startcol,target);
      real kerval = ker(v1,v2);
      result += kerval;
      multiplyAcc(targetsum, target,kerval);
    }
  return (real)result;
}
  
TVec< pair<real,int> > VMatrix::evaluateKernelTopN(int N, Ker ker, int v1_startcol, int v1_ncols, 
                                                   const Vec& v2, int startrow, int nrows, int ignore_this_row) const
{
  int endrow = (nrows>0) ?startrow+nrows :length_;
  TopNI<real> extrema(N);
  Vec v1(v1_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,v1_startcol,v1);
      real kerval = ker(v1,v2);
      extrema.update(kerval,i);
    }
  extrema.sort();
  return extrema.getTopN();
}

TVec< pair<real,int> > VMatrix::evaluateKernelBottomN(int N, Ker ker, int v1_startcol, int v1_ncols, 
                                                      const Vec& v2, int startrow, int nrows, int ignore_this_row) const
{
  int endrow = (nrows>0) ?startrow+nrows :length_;
  BottomNI<real> extrema(N);
  Vec v1(v1_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,v1_startcol,v1);
      real kerval = ker(v1,v2);
      extrema.update(kerval,i);
    }
  extrema.sort();
  return extrema.getBottomN();
}

// result += transpose(X).Y
// Where X = this->subMatColumns(X_startcol,X_ncols)
// and   Y =  this->subMatColumns(Y_startcol,Y_ncols);
void VMatrix::accumulateXtY(int X_startcol, int X_ncols, int Y_startcol, int Y_ncols, 
                            Mat& result, int startrow, int nrows, int ignore_this_row) const
{
  int endrow = (nrows>0) ?startrow+nrows :length_;
  Vec x(X_ncols);
  Vec y(Y_ncols);
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,X_startcol,x);
      getSubRow(i,Y_startcol,y);
      externalProductAcc(result, x,y);
    }
}

// result += transpose(X).Y
// Where X = this->subMatColumns(X_startcol,X_ncols)
void VMatrix::accumulateXtX(int X_startcol, int X_ncols, 
                            Mat& result, int startrow, int nrows, int ignore_this_row) const
{
  Vec x(X_ncols);
  int endrow = (nrows>0) ?startrow+nrows :length_;
  for(int i=startrow; i<endrow; i++)
    if(i!=ignore_this_row)
    {
      getSubRow(i,X_startcol,x);
      externalProductAcc(result, x,x);
    }
}

void VMatrix::evaluateSumOfFprop(Func f, Vec& output_result, int nsamples)
{
  //if (f->outputs.size()!=1)
  //  PLERROR("In evaluateSumOfFprop: function must have a single variable output (maybe you can concat the vars into a single one, if this is really what you want)");
 
  static int curpos = 0;
  if (nsamples == -1) nsamples = length();
  Vec input_value(width());
  Vec output_value(output_result.length());

  f->recomputeParents();
  output_result.clear();
 
  for(int i=0; i<nsamples; i++)
  {
    getRow(curpos++, input_value);
    f->fprop(input_value, output_value);
    output_result += output_value;
    if(curpos == length()) curpos = 0;
  }
}

void VMatrix::evaluateSumOfFbprop(Func f, Vec& output_result, Vec& output_gradient, int nsamples)
{
//  if(f->outputs.size()!=1)
 //   PLERROR("In evaluateSumOfFprop: function must have a single variable output (maybe you can concat the vars into a single one, if this is really what you want)");
 
  static int curpos = 0;
  if (nsamples == -1) nsamples = length();
  Vec input_value(width());
  Vec input_gradient(width());
  Vec output_value(output_result.length());

  f->recomputeParents();
  output_result.clear();
 
  for(int i=0; i<nsamples; i++)
  {
    getRow(curpos++, input_value);
    f->fbprop(input_value, output_value, input_gradient, output_gradient);
    //displayFunction(f, true);
    output_result += output_value;
    if(curpos == length()) curpos = 0;
  }
}

%> // end of namespace PLearn
