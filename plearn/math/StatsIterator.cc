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
   * $Id: StatsIterator.cc,v 1.6 2004/03/29 16:23:03 ducharme Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "StatsIterator.h"
#include "TMat_maths.h"

namespace PLearn {
using namespace std;

// *******************
// ** StatsIterator ** 
// *******************

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(StatsIterator, "ONE LINE DESCR", "NO HELP");
void StatsIterator::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(result, copies);
}

bool StatsIterator::requiresMultiplePasses() { return false; }

Vec StatsIterator::getResult() { return result; }

void StatsIterator::declareOptions(OptionList& ol)
{
  declareOption(ol, "result", &StatsIterator::result, OptionBase::learntoption, 
                "    result\n");

  inherited::declareOptions(ol);
}

void StatsIterator::oldwrite(ostream& out) const
{
  writeHeader(out,"StatsIterator");
  //inherited::write(out);
  writeField(out,"result",result);
  writeFooter(out,"StatsIterator");
}

void StatsIterator::oldread(istream& in)
{
  readHeader(in,"StatsIterator");
  //inherited::read(int);
  readField(in,"result",result);
  readFooter(in,"StatsIterator");
}

// ***********************
// ** MeanStatsIterator ** 
// ***********************

PLEARN_IMPLEMENT_OBJECT(MeanStatsIterator, "ONE LINE DESCR", "NO HELP");

void MeanStatsIterator::init(int inputsize)
{ 
  // We do not use resize on purpose, so 
  // that the previous result Vec does not get overwritten
  result = Vec(inputsize);
  nsamples.resize(inputsize);
  nsamples.clear();
} 

void MeanStatsIterator::update(const Vec& input)
{ 
  addIfNonMissing(input,nsamples,result);
}

bool MeanStatsIterator::finish()
{
  for (int i=0;i<result.length();i++)
    result[i] /= nsamples[i];
  return true;
}

void MeanStatsIterator::declareOptions(OptionList& ol)
{
  declareOption(ol, "nsamples", &MeanStatsIterator::nsamples, OptionBase::learntoption, 
                "    nsamples\n");

  inherited::declareOptions(ol);
}


void MeanStatsIterator::oldwrite(ostream& out) const
{
  writeHeader(out,"MeanStatsIterator");
  inherited::write(out);
  writeField(out,"nsamples",nsamples);
  writeFooter(out,"MeanStatsIterator");
}

void MeanStatsIterator::oldread(istream& in)
{
  readHeader(in,"MeanStatsIterator");
  inherited::oldread(in);
  readField(in,"nsamples",nsamples);
  readFooter(in,"MeanStatsIterator");
}

// ***********************
// ** ExpMeanStatsIterator ** 
// ***********************

PLEARN_IMPLEMENT_OBJECT(ExpMeanStatsIterator, "ONE LINE DESCR", "NO HELP");

void ExpMeanStatsIterator::init(int inputsize)
{ 
  // We do not use resize on purpose, so 
  // that the previous result Vec does not get overwritten
  result = Vec(inputsize);
  nsamples.resize(inputsize);
  nsamples.clear();
} 

void ExpMeanStatsIterator::update(const Vec& input)
{ 
  addIfNonMissing(input,nsamples,result);
}

bool ExpMeanStatsIterator::finish()
{
  for (int i=0;i<result.length();i++)
    result[i] = exp(result[i]/nsamples[i]);
  return true;
}

void ExpMeanStatsIterator::declareOptions(OptionList& ol)
{
  declareOption(ol, "nsamples", &ExpMeanStatsIterator::nsamples, OptionBase::learntoption, 
                "    nsamples\n");

  inherited::declareOptions(ol);
}

void ExpMeanStatsIterator::oldwrite(ostream& out) const
{
  writeHeader(out,"ExpMeanStatsIterator");
  inherited::write(out);
  writeField(out,"nsamples",nsamples);
  writeFooter(out,"ExpMeanStatsIterator");
}

void ExpMeanStatsIterator::oldread(istream& in)
{
  readHeader(in,"ExpMeanStatsIterator");
  inherited::oldread(in);
  readField(in,"nsamples",nsamples);
  readFooter(in,"ExpMeanStatsIterator");
}

// *************************
// ** StddevStatsIterator ** 
// *************************

PLEARN_IMPLEMENT_OBJECT(StddevStatsIterator, "ONE LINE DESCR", "NO HELP");

void StddevStatsIterator::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(mean, copies);
  deepCopyField(meansquared, copies);
}

void StddevStatsIterator::init(int inputsize)
{ 
  // We do not use resize on purpose, so 
  // that the previous result Vec does not get overwritten
  meansquared = Vec(inputsize);
  mean = Vec(inputsize);
  nsamples.resize(inputsize);
  nsamples.clear();
} 

void StddevStatsIterator::update(const Vec& input)
{ 
  addXandX2IfNonMissing(input,nsamples,mean,meansquared);
}

bool StddevStatsIterator::finish()
{
  Vec square_mean(mean.length());
  for (int i=0;i<mean.length();i++)
  {
    //mean[i] /= nsamples[i];
    real n = nsamples[i];
    square_mean[i] = mean[i]*mean[i]/(n*(n-1.0));
    meansquared[i] /= n-1.0;
  }
  //squareSubtract(meansquared, mean);
  meansquared -= square_mean;
  result = sqrt(meansquared);
  return true;
}

void StddevStatsIterator::declareOptions(OptionList& ol)
{
  declareOption(ol, "mean", &StddevStatsIterator::mean, OptionBase::learntoption, 
                "    mean\n");

  declareOption(ol, "meansquared", &StddevStatsIterator::meansquared, OptionBase::learntoption, 
                "    meansquared\n");

  declareOption(ol, "nsamples", &StddevStatsIterator::nsamples, OptionBase::learntoption, 
                "    nsamples\n");

  inherited::declareOptions(ol);
}

void StddevStatsIterator::oldwrite(ostream& out) const
{
  writeHeader(out,"StddevStatsIterator");
  inherited::write(out);
  writeField(out,"mean",mean);
  writeField(out,"meansquared",meansquared);
  writeField(out,"nsamples",nsamples);
  writeFooter(out,"StddevStatsIterator");
}

void StddevStatsIterator::oldread(istream& in)
{
  readHeader(in,"StddevStatsIterator");
  inherited::oldread(in);
  readField(in,"mean",mean);
  readField(in,"meansquared",meansquared);
  readField(in,"nsamples",nsamples);
  readFooter(in,"StddevStatsIterator");
}

// *************************
// ** StderrStatsIterator ** 
// *************************

PLEARN_IMPLEMENT_OBJECT(StderrStatsIterator, "ONE LINE DESCR", "NO HELP");

void StderrStatsIterator::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(mean, copies);
  deepCopyField(meansquared, copies);
}

void StderrStatsIterator::init(int inputsize)
{ 
  // We do not use resize on purpose, so 
  // that the previous result Vec does not get overwritten
  meansquared = Vec(inputsize);
  mean = Vec(inputsize);
  nsamples.resize(inputsize);
  nsamples.clear();
} 

void StderrStatsIterator::update(const Vec& input)
{ 
  addXandX2IfNonMissing(input,nsamples,mean,meansquared);
}

bool StderrStatsIterator::finish()
{
  for (int i=0;i<mean.length();i++)
    {
      mean[i] /= nsamples[i];
      meansquared[i] /= nsamples[i]-1;
    }
  squareSubtract(meansquared, mean);
  result = sqrt(meansquared/nsamples);
  return true;
}

void StderrStatsIterator::declareOptions(OptionList& ol)
{
  declareOption(ol, "mean", &StderrStatsIterator::mean, OptionBase::learntoption, 
                "    mean\n");

  declareOption(ol, "meansquared", &StderrStatsIterator::meansquared, OptionBase::learntoption, 
                "    meansquared\n");

  declareOption(ol, "nsamples", &StderrStatsIterator::nsamples, OptionBase::learntoption, 
                "    nsamples\n");

  inherited::declareOptions(ol);
}

void StderrStatsIterator::oldwrite(ostream& out) const
{
  writeHeader(out,"StderrStatsIterator");
  inherited::write(out);
  writeField(out,"mean",mean);
  writeField(out,"meansquared",meansquared);
  writeField(out,"nsamples",nsamples);
  writeFooter(out,"StderrStatsIterator");
}

void StderrStatsIterator::oldread(istream& in)
{
  readHeader(in,"StderrStatsIterator");
  inherited::oldread(in);
  readField(in,"mean",mean);
  readField(in,"meansquared",meansquared);
  readField(in,"nsamples",nsamples);
  readFooter(in,"StderrStatsIterator");
}

// *************************
// ** SharpeRatioStatsIterator ** 
// *************************

PLEARN_IMPLEMENT_OBJECT(SharpeRatioStatsIterator, "ONE LINE DESCR", "NO HELP");

void SharpeRatioStatsIterator::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(mean, copies);
  deepCopyField(meansquared, copies);
}

void SharpeRatioStatsIterator::init(int inputsize)
{ 
  // We do not use resize on purpose, so 
  // that the previous result Vec does not get overwritten
  meansquared = Vec(inputsize);
  mean = Vec(inputsize);
  nnonzero = Vec(inputsize);
} 

void SharpeRatioStatsIterator::update(const Vec& input)
{ 
  int n=input.length();
  for (int i=0;i<n;i++)
  {
    real in=input[i];
    if (in!=0) nnonzero[i]++;
    mean[i] += in;
    meansquared[i] += in*in;
  }
}

bool SharpeRatioStatsIterator::finish()
{
  mean /= nnonzero;
  meansquared /= nnonzero;
  squareSubtract(meansquared, mean);
  result = mean/sqrt(meansquared);
  return true;
}

void SharpeRatioStatsIterator::declareOptions(OptionList& ol)
{
  declareOption(ol, "mean", &SharpeRatioStatsIterator::mean, OptionBase::learntoption, 
                "    mean\n");

  declareOption(ol, "meansquared", &SharpeRatioStatsIterator::meansquared, OptionBase::learntoption, 
                "    meansquared\n");

  declareOption(ol, "nnonzero", &SharpeRatioStatsIterator::nnonzero, OptionBase::learntoption, 
                "    nnonzero\n");

  inherited::declareOptions(ol);
}

void SharpeRatioStatsIterator::oldwrite(ostream& out) const
{
  writeHeader(out,"SharpeRatioStatsIterator");
  inherited::write(out);
  writeField(out,"mean",mean);
  writeField(out,"meansquared",meansquared);
  writeField(out,"nnonzero",nnonzero);
  writeFooter(out,"SharpeRatioStatsIterator");
}

void SharpeRatioStatsIterator::oldread(istream& in)
{
  readHeader(in,"SharpeRatioStatsIterator");
  inherited::oldread(in);
  readField(in,"mean",mean);
  readField(in,"meansquared",meansquared);
  readField(in,"nnonzero",nnonzero);
  readFooter(in,"SharpeRatioStatsIterator");
}

// ***********************
// ** MinStatsIterator ** 
// ***********************

PLEARN_IMPLEMENT_OBJECT(MinStatsIterator, "ONE LINE DESCR", "NO HELP");

void MinStatsIterator::init(int inputsize)
{ 
  result = Vec(inputsize,FLT_MAX);
} 

void MinStatsIterator::update(const Vec& input)
{ 
  real* inputdata = input.data();
  real* resultdata = result.data();
  for(int i=0; i<input.length(); i++)
    if(inputdata[i]<resultdata[i])
      resultdata[i] = inputdata[i];
}

bool MinStatsIterator::finish()
{ return true; }

void MinStatsIterator::declareOptions(OptionList& ol)
{
  inherited::declareOptions(ol);
}

void MinStatsIterator::oldwrite(ostream& out) const
{
  writeHeader(out,"MinStatsIterator");
  inherited::write(out);
  writeFooter(out,"MinStatsIterator");
}

void MinStatsIterator::oldread(istream& in)
{
  readHeader(in,"MinStatsIterator");
  inherited::oldread(in);
  readFooter(in,"MinStatsIterator");
}

// ***********************
// ** MaxStatsIterator ** 
// ***********************

PLEARN_IMPLEMENT_OBJECT(MaxStatsIterator, "ONE LINE DESCR", "NO HELP");

void MaxStatsIterator::init(int inputsize)
{ 
  // We do not use resize on purpose, so 
  // that the previous result Vec does not get overwritten
  result = Vec(inputsize,-FLT_MAX);
} 

void MaxStatsIterator::update(const Vec& input)
{ 
  real* inputdata = input.data();
  real* resultdata = result.data();
  for(int i=0; i<input.length(); i++)
    if(inputdata[i]>resultdata[i])
      resultdata[i] = inputdata[i];
}

bool MaxStatsIterator::finish()
{ return true; }

void MaxStatsIterator::declareOptions(OptionList& ol)
{
  inherited::declareOptions(ol);
}

void MaxStatsIterator::oldwrite(ostream& out) const
{
  writeHeader(out,"MaxStatsIterator");
  inherited::write(out);
  writeFooter(out,"MaxStatsIterator");
}

void MaxStatsIterator::oldread(istream& in)
{
  readHeader(in,"MaxStatsIterator");
  inherited::oldread(in);
  readFooter(in,"MaxStatsIterator");
}


// ***********************
// ** LiftStatsIterator **
// ***********************
 
PLEARN_IMPLEMENT_OBJECT(LiftStatsIterator, "ONE LINE DESCR", "NO HELP");
void LiftStatsIterator::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  StatsIterator::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(output_and_pos, copies);
  deepCopyField(targets, copies);
}
 
LiftStatsIterator::LiftStatsIterator(int the_index, real the_fraction)
  : lift_index(the_index), lift_fraction(the_fraction)
{}

void LiftStatsIterator::init(int inputsize)
{
  // We do not use resize on purpose, so
  // that the previous result Vec does not get overwritten
  result = Vec(2);

  const int initial_length = 1000;
  output_and_pos.resize(initial_length, 2);  // 1 output + 1 pos
  targets.resize(initial_length);
  nsamples = 0;
}
 
void LiftStatsIterator::update(const Vec& input)
{
  if (nsamples == output_and_pos.length())
  {
    output_and_pos.resize(10*output_and_pos.length(), 2);
    targets.resize(10*output_and_pos.length());
  }

  output_and_pos(nsamples, 0) = FABS(input[lift_index]);
  output_and_pos(nsamples, 1) = nsamples;
  targets[nsamples] = (input[lift_index]>0) ? 1 : 0;
  nsamples++;
}
 
bool LiftStatsIterator::finish()
{
  output_and_pos.resize(nsamples,2);
  targets.resize(nsamples);

  const int n_first_samples = int(lift_fraction*nsamples);
  const int n_last_samples = nsamples - n_first_samples;
  selectAndOrder(output_and_pos, n_last_samples);
  /*
  Vec first_samples_index =
    output_and_pos.subMat(n_last_samples,1,n_first_samples,1).toVecCopy();
  */
  TVec<int> first_samples_index(n_first_samples);
  first_samples_index << output_and_pos.subMat(n_last_samples,1,n_first_samples,1);

  Vec first_samples_targets = targets(first_samples_index);
  real first_samples_perf = sum(first_samples_targets)/n_first_samples;
  real targets_perf = sum(targets)/nsamples;
  real lift = first_samples_perf/targets_perf*100.0;
  result[0] = lift;
  real nones = sum(targets);
  real max_first_samples_perf = MIN(nones,(real)n_first_samples)/n_first_samples;
  real max_lift = max_first_samples_perf/targets_perf*100.0;
  result[1] = lift/max_lift;

  return true;
}
 
void LiftStatsIterator::declareOptions(OptionList& ol)
{
  declareOption(ol, "nsamples", &LiftStatsIterator::nsamples, OptionBase::learntoption, 
                "    nsamples\n");

  declareOption(ol, "lift_index", &LiftStatsIterator::lift_index, OptionBase::buildoption, 
                "    lift_index\n");

  declareOption(ol, "lift_fraction", &LiftStatsIterator::lift_fraction, OptionBase::buildoption, 
                "    lift_fraction\n");

  declareOption(ol, "output_and_pos", &LiftStatsIterator::output_and_pos, OptionBase::learntoption, 
                "    output_and_pos\n");

  declareOption(ol, "targets", &LiftStatsIterator::targets, OptionBase::learntoption, 
                "    targets\n");

  inherited::declareOptions(ol);
}

void LiftStatsIterator::oldwrite(ostream& out) const
{
  writeHeader(out,"LiftStatsIterator");
  inherited::write(out);
  writeField(out,"nsamples",nsamples);
  writeField(out,"lift_index",lift_index);
  writeField(out,"lift_fraction",lift_fraction);
  writeField(out,"output_and_pos",output_and_pos);
  writeField(out,"targets",targets);
  writeFooter(out,"LiftStatsIterator");
}

void LiftStatsIterator::oldread(istream& in)
{
  readHeader(in,"LiftStatsIterator");
  inherited::oldread(in);
  readField(in,"nsamples",nsamples);
  readField(in,"lift_index",lift_index);
  readField(in,"lift_fraction",lift_fraction);
  readField(in,"output_and_pos",output_and_pos);
  readField(in,"targets",targets);
  readFooter(in,"LiftStatsIterator");
}

// ****************************
// ** QuantilesStatsIterator **
// ****************************

PLEARN_IMPLEMENT_OBJECT(QuantilesStatsIterator, "ONE LINE DESCR", "NO HELP");

void QuantilesStatsIterator::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  StatsIterator::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(quantiles, copies);
  deepCopyField(data, copies);
}
 
QuantilesStatsIterator::QuantilesStatsIterator(Vec quantiles_,int n_data)
  : nsamples(n_data), quantiles(quantiles_)
{ 
}

void QuantilesStatsIterator::init(int inputsize)
{
  data.resize(inputsize);
  for (int i=0;i<inputsize;i++)
  {
    data[i].resize(nsamples);
    data[i].resize(0);
  }
  nsamples=0;
  // We do not use resize on purpose, so
  // that the previous result Vec does not get overwritten
  result = Vec(quantiles.length()*data.length());
}
 
void QuantilesStatsIterator::update(const Vec& input)
{
  if (nsamples == data[0].length())
  {
    for (int i=0;i<data.length();i++)
    {
      data[i].resize(10*(nsamples+1));
      data[i].resize(nsamples);
    }
  }

  for (int i=0;i<input.length();i++)
    data[i].push_back(input[i]);
  nsamples++;
}
 
bool QuantilesStatsIterator::finish()
{
  for (int i=0;i<data.length();i++)
    sortElements(data[i]);
  real dq =real(1.0)/nsamples;
  real hdq = dq*real(0.5);
  Mat results = result.toMat(data.length(),quantiles.length());
  results.fill(MISSING_VALUE);
  for (int i=0;i<data.length();i++) // loop over "variables"
  {
    real q=0;
    for (int t=0;t<nsamples;t++,q+=dq)
    {
      for (int j=0;j<quantiles.length();j++)
        if (quantiles[j]>q-hdq && quantiles[j]<=q+hdq)
          results(i,j) = data[i][t];
    }
  }
  return true;
}
 
void QuantilesStatsIterator::declareOptions(OptionList& ol)
{
  declareOption(ol, "nsamples", &QuantilesStatsIterator::nsamples, OptionBase::buildoption, 
                "    nsamples\n");

  declareOption(ol, "quantiles", &QuantilesStatsIterator::quantiles, OptionBase::buildoption, 
                "    quantiles\n");

  inherited::declareOptions(ol);
}

void QuantilesStatsIterator::oldwrite(ostream& out) const
{
  writeHeader(out,"QuantilesStatsIterator");
  inherited::write(out);
  writeField(out,"nsamples",nsamples);
  writeField(out,"quantiles",quantiles);
  writeFooter(out,"QuantilesStatsIterator");
}

void QuantilesStatsIterator::oldread(istream& in)
{
  readHeader(in,"QuantilesStatsIterator");
  inherited::oldread(in);
  readField(in,"nsamples",nsamples);
  readField(in,"quantiles",quantiles);
  readFooter(in,"QuantilesStatsIterator");
}

// ******************
// ** StatsItArray **
// ******************

StatsItArray::StatsItArray() 
  : Array<StatsIt>(0,5)
{}

StatsItArray::StatsItArray(const StatsIt& statsit)
  : Array<StatsIt>(1,5)
{ (*this)[0] = statsit; }

StatsItArray::StatsItArray(const StatsIt& statsit1, const StatsIt& statsit2)
  : Array<StatsIt>(2,5)
{
  (*this)[0] = statsit1;
  (*this)[1] = statsit2;
}

void StatsItArray::init(int inputsize)
{
  for(int k=0; k<size(); k++)
    (*this)[k]->init(inputsize);
}

void StatsItArray::update(const Vec& input)
{
  for(int k=0; k<size(); k++)
    (*this)[k]->update(input);
}
void StatsItArray::update(const Mat& inputs)
{
  for (int i=0;i<inputs.length();i++)
  {
    Vec input = inputs(i);
    update(input);
  }
}

bool StatsItArray::requiresMultiplePasses()
{
  for(int k=0; k<size(); k++)
    if ( (*this)[k]->requiresMultiplePasses() )
      return true;
  return false;
}

// returns an array of those that are not yet finished
StatsItArray StatsItArray::finish()
{
  StatsItArray unfinished;
  for(int k=0; k<size(); k++)
    if ( ! (*this)[k]->finish() )
      unfinished.append((*this)[k]);
  return unfinished;
}

Array<Vec> StatsItArray::getResults()
{
  Array<Vec> results(size());
  for(int k=0; k<size(); k++)
    results[k] = (*this)[k]->getResult();
  return results;
}

Array<Vec> StatsItArray::computeStats(VMat data)
{
  int inputsize = data.width();
  init(inputsize);

  Vec input(inputsize);
  StatsItArray unfinished = *this;

  while(unfinished.size()>0)
    {
      for(int i=0; i<data.length(); i++)
        {
          data->getRow(i,input);
          unfinished.update(input);
        }
      unfinished = finish();
    }

  return getResults();
}

} // end of namespace PLearn
