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
   * $Id: Kernel.cc,v 1.5 2003/08/13 08:13:17 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include <cmath>
#include "stringutils.h"
#include "Kernel.h"
#include "TMat_maths.h"
#include "PLMPI.h"

namespace PLearn <%
using namespace std;
  
  PLEARN_IMPLEMENT_ABSTRACT_OBJECT(Kernel, "ONE LINE DESCR", "NO HELP");
  Kernel::~Kernel() {}

  void Kernel::setDataForKernelMatrix(VMat the_data)
  { data = the_data; }

  real Kernel::evaluate_i_j(int i, int j) const
  { return evaluate(data(i),data(j)); }

  real Kernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const 
  { return evaluate(data(i),x); }

  real Kernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const
  { 
    if(is_symmetric)
      return evaluate_i_x(i,x,squared_norm_of_x);
    else
      return evaluate(x,data(i));
  }

void Kernel::computeGramMatrix(Mat K) const
{
  if (!data) PLERROR("Kernel::computeGramMatrix should be called only after setDataForKernelMatrix");
  int l=data->length();
  int m=K.mod();
  for (int i=0;i<l;i++)
  {
    real* Ki = K[i];
    real* Kji_ = &K[0][i];
    for (int j=0;j<=i;j++,Kji_+=m)
    {
      real Kij = evaluate_i_j(i,j);
      Ki[j]=Kij;
      if (j<i)
        *Kji_ =Kij;
    }
  }
}
    
void Kernel::setParameters(Vec paramvec)
{ PLERROR("setParameters(Vec paramvec) not implemented for this kernel"); }

Vec Kernel::getParameters() const
{ return Vec(); }

void 
Kernel::apply(VMat m1, VMat m2, Mat& result) const
{
  result.resize(m1->length(), m2->length());
  Vec m1_i(m1->width());
  Vec m2_j(m2->width());
  if(is_symmetric && m1==m2)
    {
      for(int i=0; i<m1->length(); i++)
        {
          m1->getRow(i,m1_i);
          for(int j=0; j<=i; j++)
            {
              m2->getRow(j,m2_j);
              real val = evaluate(m1_i,m2_j);
              result(i,j) = val;
              result(j,i) = val;
            }
        }
    }
  else
    {
      for(int i=0; i<m1->length(); i++)
        {
          m1->getRow(i,m1_i);
          for(int j=0; j<m2->length(); j++)
            {
              m2->getRow(j,m2_j);
              result(i,j) = evaluate(m1_i,m2_j);
            }
        }
    }
}

void 
Kernel::apply(VMat m, const Vec& x, Vec& result) const
{
  result.resize(m->length());
  Vec m_i(m->width());
  for(int i=0; i<m->length(); i++)
    {
      m->getRow(i,m_i);
      result[i] = evaluate(m_i,x);
    }
}

void 
Kernel::apply(Vec x, VMat m, Vec& result) const
{
  result.resize(m->length());
  Vec m_i(m->width());
  for(int i=0; i<m->length(); i++)
    {
      m->getRow(i,m_i);
      result[i] = evaluate(x,m_i);
    }
}

Mat 
Kernel::apply(VMat m1, VMat m2) const
{
  Mat result;
  apply(m1,m2,result);
  return result;
}

real 
Kernel::test(VMat d, real threshold, real sameness_below_threshold, real sameness_above_threshold) const
{
  int nerrors = 0;
  int inputsize = (d->width()-1)/2;
  for(int i=0; i<d->length(); i++)
    {
      Vec inputs = d(i);
      Vec input1 = inputs.subVec(0,inputsize);
      Vec input2 = inputs.subVec(inputsize,inputsize);
      real sameness = inputs[inputs.length()-1];
      real kernelvalue = evaluate(input1,input2);
      cerr << "[" << kernelvalue << " " << sameness << "]\n";
      if(kernelvalue<threshold)
        {
          if(sameness==sameness_above_threshold)
            nerrors++;
        }
      else // kernelvalue>=threshold
        {
          if(sameness==sameness_below_threshold)
            nerrors++;
        }
    }
  return real(nerrors)/d->length();
}


Mat 
Kernel::computeNeighbourMatrixFromDistanceMatrix(const Mat& D, bool insure_self_first_neighbour)
{
  int npoints = D.length();
  Mat neighbours(npoints, npoints);  
  Mat tmpsort(npoints,2);
  
    //for(int i=0; i<2; i++)
  for(int i=0; i<npoints; i++)
    {
      for(int j=0; j<npoints; j++)
        {
          tmpsort(j,0) = D(i,j);
          tmpsort(j,1) = j;
        }
      if(insure_self_first_neighbour)
        tmpsort(i,0) = -FLT_MAX;

      sortRows(tmpsort);
      neighbours(i) << tmpsort.column(1);
    }
  return neighbours;
}

Mat 
Kernel::estimateHistograms(VMat d, real sameness_threshold, real minval, real maxval, int nbins) const
{
  real binwidth = (maxval-minval)/nbins;
  int inputsize = (d->width()-1)/2;
  Mat histo(2,nbins);
  Vec histo_below = histo(0);
  Vec histo_above = histo(1);
  int nbelow=0;
  int nabove=0;
  for(int i=0; i<d->length(); i++)
    {
      Vec inputs = d(i);
      Vec input1 = inputs.subVec(0,inputsize);
      Vec input2 = inputs.subVec(inputsize,inputsize);
      real sameness = inputs[inputs.length()-1];
      real kernelvalue = evaluate(input1,input2);
      if(kernelvalue>=minval && kernelvalue<maxval)
        {
          int binindex = int((kernelvalue-minval)/binwidth);
          if(sameness<sameness_threshold)
            {
              histo_below[binindex]++;
              nbelow++;
            }
          else
            {
              histo_above[binindex]++;
              nabove++;
            }
        }
    }
  histo_below /= real(nbelow);
  histo_above /= real(nabove);
  return histo;
}

Mat Kernel::estimateHistograms(Mat input_and_class, real minval, real maxval, int nbins) const
{
  real binwidth = (maxval-minval)/nbins;
  int inputsize = input_and_class.width()-1;
  Mat inputs = input_and_class.subMatColumns(0,inputsize);
  Mat classes = input_and_class.column(inputsize);
  Mat histo(4,nbins);
  Vec histo_mean_same = histo(0);
  Vec histo_mean_other = histo(1);
  Vec histo_min_same = histo(2);
  Vec histo_min_other = histo(3);

  for(int i=0; i<inputs.length(); i++)
    {
      Vec input = inputs(i);
      real input_class = classes(i,0);
      real sameclass_meandist = 0.0;
      real otherclass_meandist = 0.0;
      real sameclass_mindist = FLT_MAX;
      real otherclass_mindist = FLT_MAX;
      for(int j=0; j<inputs.length(); j++)
        if(j!=i)
          {
            real dist = evaluate(input, inputs(j));
            if(classes(j,0)==input_class)
              {
                sameclass_meandist += dist;
                if(dist<sameclass_mindist)
                  sameclass_mindist = dist;
              }
            else
              {
                otherclass_meandist += dist;
                if(dist<otherclass_mindist)
                  otherclass_mindist = dist;
              }
          }
      sameclass_meandist /= (inputs.length()-1);
      otherclass_meandist /= (inputs.length()-1);      
      if(sameclass_meandist>=minval && sameclass_meandist<maxval)
        histo_mean_same[int((sameclass_meandist-minval)/binwidth)]++;
      if(sameclass_mindist>=minval && sameclass_mindist<maxval)
        histo_min_same[int((sameclass_mindist-minval)/binwidth)]++;
      if(otherclass_meandist>=minval && otherclass_meandist<maxval)
        histo_mean_other[int((otherclass_meandist-minval)/binwidth)]++;
      if(otherclass_mindist>=minval && otherclass_mindist<maxval)
        histo_min_other[int((otherclass_mindist-minval)/binwidth)]++;
    }
  histo_mean_same /= sum(histo_mean_same);
  histo_min_same /= sum(histo_min_same);
  histo_mean_other /= sum(histo_mean_other);
  histo_min_other /= sum(histo_min_other);
  return histo;
}

void
Kernel::oldwrite(ostream& out) const
{
	writeHeader(out,"Kernel");
	writeField(out,"is_symmetric",is_symmetric);
	writeFooter(out,"Kernel");
}

void
Kernel::oldread(istream& in)
{
	readHeader(in,"Kernel");
	readField(in,"is_symmetric",is_symmetric);
	readFooter(in,"Kernel");
}

// ** PowDistanceKernel **

PLEARN_IMPLEMENT_OBJECT(PowDistanceKernel, "ONE LINE DESCR", "NO HELP");
string PowDistanceKernel::info() const { return "(L"+tostring(n)+")^"+tostring(n); }
real PowDistanceKernel::evaluate(const Vec& x1, const Vec& x2) const
{ return powdistance(x1, x2, n); }

void PowDistanceKernel::write(ostream& out) const
{
  writeHeader(out,"PowDistanceKernel");
  inherited::oldwrite(out);
	writeField(out,"n",n);
  writeFooter(out,"PowDistanceKernel");
}

void PowDistanceKernel::oldread(istream& in)
{
  readHeader(in,"PowDistanceKernel");
  inherited::oldread(in);
  readField(in,"n",n);
  readFooter(in,"PowDistanceKernel");
}

/*
void PowDistanceKernel::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="n")
    PLearn::read(in, n);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void PowDistanceKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "n", &PowDistanceKernel::n, OptionBase::buildoption,
                  "TODO: Give some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(DistanceKernel, "ONE LINE DESCR", "NO HELP");
string DistanceKernel::info() const { return "L"+tostring(n); }
real DistanceKernel::evaluate(const Vec& x1, const Vec& x2) const
{ return dist(x1, x2, n); }

void DistanceKernel::oldread(istream& in)
{
  readHeader(in,"DistanceKernel");
  inherited::oldread(in);
  readField(in,"n",n);
  readFooter(in,"DistanceKernel");
}

void DistanceKernel::declareOptions(OptionList& ol)
{
  declareOption(ol, "n", &DistanceKernel::n, OptionBase::buildoption, 
                "This class implements a Ln distance (L2, the default is the usual euclidean distance)");
  inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(DotProductKernel, "ONE LINE DESCR", "NO HELP");
real DotProductKernel::evaluate(const Vec& x1, const Vec& x2) const
{ return dot(x1,x2); }

real DotProductKernel::evaluate_i_j(int i, int j) const
{ return data->dot(i,j); }

real DotProductKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const 
{ return data->dot(i,x); } 

real DotProductKernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const
{ return data->dot(i,x); } 

void DotProductKernel::write(ostream& out) const
{
  writeHeader(out,"DotProductKernel");
  inherited::oldwrite(out);
	writeFooter(out,"DotProductKernel");
}
void DotProductKernel::oldread(istream& in)
{
  readHeader(in,"DotProductKernel");
  inherited::oldread(in);
  readFooter(in,"DotProductKernel");
}


PLEARN_IMPLEMENT_OBJECT(PolynomialKernel, "ONE LINE DESCR", "NO HELP");
real PolynomialKernel::evaluate(const Vec& x1, const Vec& x2) const
{ return evaluateFromDot(dot(x1,x2)); }
real PolynomialKernel::evaluate_i_j(int i, int j) const
{ return evaluateFromDot(data->dot(i,j)); }
real PolynomialKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const 
{ return evaluateFromDot(data->dot(i,x)); } 
real PolynomialKernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const
{ return evaluateFromDot(data->dot(i,x)); } 

void PolynomialKernel::write(ostream& out) const
{
  writeHeader(out,"PolynomialKernel");
  inherited::oldwrite(out);
	writeField(out,"n",n);
  writeField(out,"beta",beta);
  writeFooter(out,"PolynomialKernel");
}
void PolynomialKernel::oldread(istream& in)
{
  readHeader(in,"PolynomialKernel");
  inherited::oldread(in);
  readField(in,"n",n);
  readField(in,"beta",beta);
  readFooter(in,"PolynomialKernel");
}
// recognized options are "n" and "beta"
/*
void PolynomialKernel::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="n")
    PLearn::read(in,n);
  if (optionname=="beta")
    PLearn::read(in,beta);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void PolynomialKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "n", &PolynomialKernel::n, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "beta", &PolynomialKernel::beta, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(SigmoidalKernel, "ONE LINE DESCR", "NO HELP");
real SigmoidalKernel::evaluate(const Vec& x1, const Vec& x2) const
{ return sigmoid(c*dot(x1,x2)); }

void SigmoidalKernel::write(ostream& out) const
{
  writeHeader(out,"SigmoidalKernel");
  inherited::oldwrite(out);
	writeField(out,"c",c);
  writeFooter(out,"SigmoidalKernel");
}
void SigmoidalKernel::oldread(istream& in)
{
  readHeader(in,"SigmoidalKernel");
  inherited::oldread(in);
  readField(in,"c",c);
  readFooter(in,"SigmoidalKernel");
}
// recognized option is "c"
/*
void SigmoidalKernel::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="c")
    PLearn::read(in,c);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void SigmoidalKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "c", &SigmoidalKernel::c, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(SigmoidPrimitiveKernel, "ONE LINE DESCR", "NO HELP");
real SigmoidPrimitiveKernel::evaluate(const Vec& x1, const Vec& x2) const
{ return log(1.0+exp(c*dot(x1,x2))); }

void SigmoidPrimitiveKernel::write(ostream& out) const
{
  writeHeader(out,"SigmoidPrimitiveKernel");
  inherited::oldwrite(out);
	writeField(out,"c",c);
  writeFooter(out,"SigmoidPrimitiveKernel");
}
void SigmoidPrimitiveKernel::oldread(istream& in)
{
  readHeader(in,"SigmoidPrimitiveKernel");
  inherited::oldread(in);
  readField(in,"c",c);
  readFooter(in,"SigmoidPrimitiveKernel");
}
// recognized option is "c"
/*
void SigmoidPrimitiveKernel::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="c")
    PLearn::read(in,c);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void SigmoidPrimitiveKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "c", &SigmoidPrimitiveKernel::c, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}


PLEARN_IMPLEMENT_OBJECT(ConvexBasisKernel, "ONE LINE DESCR", "NO HELP");
real ConvexBasisKernel::evaluate(const Vec& x1, const Vec& x2) const
{ 
  real p=1;
  real* x1i=x1.data();
  real* x2i=x2.data();
  int n=x1.length();
  for (int i=0;i<n;i++)
    p *= log(1+exp(c*(x1i[i]-x2i[i])));
  return p;
}

void ConvexBasisKernel::write(ostream& out) const
{
  writeHeader(out,"ConvexBasisKernel");
  inherited::oldwrite(out);
  writeField(out,"c",c);
  writeFooter(out,"ConvexBasisKernel");
}
void ConvexBasisKernel::oldread(istream& in)
{
  readHeader(in,"ConvexBasisKernel");
  inherited::oldread(in);
  readField(in,"c",c);
  readFooter(in,"ConvexBasisKernel");
}
// recognized option is "c"
/*
void ConvexBasisKernel::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="c")
    PLearn::read(in,c);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void ConvexBasisKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "c", &ConvexBasisKernel::c, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}

// ** GaussianKernel **

PLEARN_IMPLEMENT_OBJECT(GaussianKernel, "ONE LINE DESCR", "NO HELP");

void GaussianKernel::declareOptions(OptionList& ol)
{
  declareOption(ol, "sigma", &GaussianKernel::sigma, OptionBase::buildoption,
                "The width of the Gaussian");
  inherited::declareOptions(ol);
}

void GaussianKernel::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Kernel::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(squarednorms,copies);
}

void GaussianKernel::setDataForKernelMatrix(VMat the_data)
{ 
  Kernel::setDataForKernelMatrix(the_data);
  squarednorms.resize(data.length());
  for(int i=0; i<data.length(); i++)
    squarednorms[i] = data->dot(i,i);
}

real GaussianKernel::evaluate(const Vec& x1, const Vec& x2) const
{
#ifdef BOUNDCHECK
  if(x1.length()!=x2.length())
    PLERROR("IN GaussianKernel::evaluate x1 and x2 must have the same length");
#endif
  int l = x1.length();
  real* px1 = x1.data();
  real* px2 = x2.data();
  real sqnorm_of_diff = 0.;
  for(int i=0; i<l; i++)
    {
      real val = px1[i]-px2[i];
      sqnorm_of_diff += val*val;
    }
  return evaluateFromSquaredNormOfDifference(sqnorm_of_diff);
}

real GaussianKernel::evaluate_i_j(int i, int j) const
{ return evaluateFromDotAndSquaredNorm(squarednorms[i],data->dot(i,j),squarednorms[j]); }

real GaussianKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const 
{ 
  if(squared_norm_of_x<0.)
    squared_norm_of_x = pownorm(x);
  return evaluateFromDotAndSquaredNorm(squarednorms[i],data->dot(i,x),squared_norm_of_x); 
}

real GaussianKernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const
{ 
  if(squared_norm_of_x<0.)
    squared_norm_of_x = pownorm(x);
  return evaluateFromDotAndSquaredNorm(squared_norm_of_x,data->dot(i,x),squarednorms[i]); 
}

void GaussianKernel::setParameters(Vec paramvec)
{ 
  PLWARNING("In GaussianKernel: setParameters is deprecated, use setOption instead");
  sigma = paramvec[0]; 
  minus_one_over_sigmasquare = -1.0/(sigma*sigma);
}

void GaussianKernel::oldread(istream& in)
{
  readHeader(in,"GaussianKernel");
  inherited::oldread(in);
  readField(in,"sigma",sigma);
  readFooter(in,"GaussianKernel");
  minus_one_over_sigmasquare = -1.0/(sigma*sigma);
}

void GaussianKernel::build_()
{
  minus_one_over_sigmasquare = -1.0/square(sigma);
}

void GaussianKernel::build()
{
  inherited::build();
  build_();
}


// ** PrecomputedKernel **

PLEARN_IMPLEMENT_OBJECT(PrecomputedKernel, "ONE LINE DESCR", "NO HELP");

PrecomputedKernel::~PrecomputedKernel()
{
  if(precomputedK)
    delete[] precomputedK;
}

void PrecomputedKernel::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Kernel::makeDeepCopyFromShallowCopy(copies);
  precomputedK = 0; // not a real deep copy, but this will do for now...
}

void PrecomputedKernel::setDataForKernelMatrix(VMat the_data)
{ 
  Kernel::setDataForKernelMatrix(the_data);
  ker->setDataForKernelMatrix(the_data);
  if(precomputedK)
    delete[] precomputedK;
  int l = data.length();
  precomputedK = new float(l*l);
  float* Kdata = precomputedK;
  for(int i=0; i<l; i++)
    {
      cerr << "Precomputing Kernel Matrix Row " << i << " of " << l << " ..." << endl;
      for(int j=0; j<l; j++)
        Kdata[j] = (float)ker->evaluate_i_j(i,j);
      Kdata += l;
    }
}

real PrecomputedKernel::evaluate(const Vec& x1, const Vec& x2) const
{ return ker->evaluate(x1,x2); }

real PrecomputedKernel::evaluate_i_j(int i, int j) const
{ 
#ifdef BOUNDCHECK
  if(!precomputedK)
    PLERROR("In PrecomputedKernel::evaluate_i_j data must first be set with setDataForKernelMatrix");
  else if(i<0 || j<0 || i>=data.length() || j>=data.length())
    PLERROR("In PrecomputedKernel::evaluate_i_j i and j must be between 0 and data.length()");
#endif
  return precomputedK[i*data.length()+j];
}

real PrecomputedKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const 
{ return ker->evaluate_i_x(i,x,squared_norm_of_x); }

real PrecomputedKernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const
{ return ker->evaluate_x_i(x,i,squared_norm_of_x); }

void PrecomputedKernel::write(ostream& out) const
{
  writeHeader(out,"PrecomputedKernel");
  inherited::oldwrite(out);
	ker->write(out);
  writeFooter(out,"PrecomputedKernel");
}

void PrecomputedKernel::oldread(istream& in)
{
  readHeader(in,"PrecomputedKernel");
  inherited::oldread(in);
  ker = dynamic_cast<Kernel*>(readObject(in));
  readFooter(in,"PrecomputedKernel");
}
/*
void PrecomputedKernel::readOptionVal(istream& in, const string& optionname)
{ 
  if(optionname.length()>4 && optionname.substr(0,4)=="ker.")
    ker->readOptionVal(in, optionname.substr(4));
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void PrecomputedKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "ker", &PrecomputedKernel::ker, OptionBase::buildoption,
                   "TODO: Some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(ScaledGaussianKernel, "ONE LINE DESCR", "NO HELP");
void ScaledGaussianKernel::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Kernel::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(phi, copies);
}
real ScaledGaussianKernel::evaluate(const Vec& x1, const Vec& x2) const
{ return exp(-weighted_powdistance(x1, x2, real(2.0), phi)/square(sigma)); }

void ScaledGaussianKernel::write(ostream& out) const
{
  writeHeader(out,"ScaledGaussianKernel");
  inherited::oldwrite(out);
  writeField(out,"sigma",sigma);
  writeField(out,"phi",phi);
  writeFooter(out,"ScaledGaussianKernel");
}
void ScaledGaussianKernel::oldread(istream& in)
{
  readHeader(in,"ScaledGaussianKernel");
  inherited::oldread(in);
  readField(in,"sigma",sigma);
  readField(in,"phi",phi);
  readFooter(in,"ScaledGaussianKernel");
}
// recognized option is "sigma"
/*
void ScaledGaussianKernel::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="sigma")
    PLearn::read(in, sigma);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void ScaledGaussianKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "sigma", &ScaledGaussianKernel::sigma, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(GaussianDensityKernel, "ONE LINE DESCR", "NO HELP");
real GaussianDensityKernel::evaluate(const Vec& x1, const Vec& x2) const
{ return exp(-real(0.5)*powdistance(x1, x2, real(2.0))/(sigma*sigma) - x1.length()*(0.5*Log2Pi + log(sigma))); }

void GaussianDensityKernel::declareOptions(OptionList& ol)
{
  declareOption(ol, "sigma", &GaussianDensityKernel::sigma, OptionBase::buildoption,
                "The width of the Gaussian");
  inherited::declareOptions(ol);
}

void GaussianDensityKernel::oldread(istream& in)
{
  readHeader(in,"GaussianDensityKernel");
  inherited::oldread(in);
  readField(in,"sigma",sigma);
  readFooter(in,"GaussianDensityKernel");
}

PLEARN_IMPLEMENT_OBJECT(LogOfGaussianDensityKernel, "ONE LINE DESCR", "NO HELP");
real LogOfGaussianDensityKernel::evaluate(const Vec& x1, const Vec& x2) const
{ 
  // cerr << "LogOfGaussKernel mu: " << x1 << endl; 
  double sigmasq = sigma*sigma;
  // cerr << "sqnorm_xmu " << powdistance(x1, x2, real(2.0)) << endl;
  // cerr << "sigmasq " << sigmasq << endl;
  double q = powdistance(x1, x2, real(2.0))/sigmasq;
  // cerr << "log of gauss kernel q = " << q << endl; 
  //double logp = -0.5 * ( q + x1.length()*( log(2*M_PI) + log(sigmasq)) ); 
  double logp = -0.5 * ( q + x1.length()*( log(2*Pi) + log(sigmasq)) ); 
  // cerr << "logp = " << logp << endl;
  // exit(0);
  return real(logp);
}

void LogOfGaussianDensityKernel::declareOptions(OptionList& ol)
{
  declareOption(ol, "sigma", &LogOfGaussianDensityKernel::sigma, OptionBase::buildoption,
                "The width of the Gaussian");
  inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(GeneralizedDistanceRBFKernel, "ONE LINE DESCR", "NO HELP");
real GeneralizedDistanceRBFKernel::evaluate(const Vec& x1, const Vec& x2) const
{ 
#ifdef BOUNDCHECK
  if(x1.length()!=x2.length())
    PLERROR("IN GeneralizedDistanceRBFKernel::evaluate x1 and x2 must have the same length");
#endif

  real summ = 0.0;
  for(int i=0; i<x1.length(); i++)
    summ += pow(fabs(pow(x1[i],(real)a)-pow(x2[i],(real)a)), (real)b);
  return exp(-phi*pow(summ,c));
}


void GeneralizedDistanceRBFKernel::write(ostream& out) const
{
  writeHeader(out,"GeneralizedDistanceRBFKernel");
  inherited::oldwrite(out);
  writeField(out,"phi",phi);
  writeField(out,"a",a);
  writeField(out,"b",b);
  writeField(out,"c",c);
  writeFooter(out,"GeneralizedDistanceRBFKernel");
}
void GeneralizedDistanceRBFKernel::oldread(istream& in)
{
  readHeader(in,"GeneralizedDistanceRBFKernel");
  inherited::oldread(in);
  readField(in,"phi",phi);
  readField(in,"a",a);
  readField(in,"b",b);
  readField(in,"c",c);
  readFooter(in,"GeneralizedDistanceRBFKernel");
}

// recognized options are "phi", "a", "b" and "c"
/*
void GeneralizedDistanceRBFKernel::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="phi")
    PLearn::read(in, phi); 
  if (optionname=="a")
    PLearn::read(in, a); 
  if (optionname=="b")
    PLearn::read(in, b); 
  if (optionname=="c")
    PLearn::read(in, c); 
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void GeneralizedDistanceRBFKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "phi", &GeneralizedDistanceRBFKernel::phi, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "a", &GeneralizedDistanceRBFKernel::a, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "b", &GeneralizedDistanceRBFKernel::b, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "c", &GeneralizedDistanceRBFKernel::c, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(ScaledGeneralizedDistanceRBFKernel, "ONE LINE DESCR", "NO HELP");
void ScaledGeneralizedDistanceRBFKernel::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Kernel::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(phi, copies);
  deepCopyField(a, copies);
}
real ScaledGeneralizedDistanceRBFKernel::evaluate(const Vec& x1, const Vec& x2) const
{ 
#ifdef BOUNDCHECK
  if(x1.length()!=x2.length())
    PLERROR("IN ScaledGeneralizedDistanceRBFKernel::evaluate x1 and x2 must have the same length");
#endif

  real summ = 0.0;
  real* ph=phi.data();
  real* aa=a.data();
  for(int i=0; i<x1.length(); i++)
    summ += ph[i]*pow(fabs(pow(x1[i],aa[i])-pow(x2[i],aa[i])), (real)b);
  return exp(-pow(summ,c));
}


void ScaledGeneralizedDistanceRBFKernel::write(ostream& out) const
{
  writeHeader(out,"ScaledGeneralizedDistanceRBFKernel");
  inherited::oldwrite(out);
  writeField(out,"phi", phi);
  writeField(out,"a", a);
  writeField(out,"b",b);
  writeField(out,"c",c);
  writeFooter(out,"ScaledGeneralizedDistanceRBFKernel");
}
void ScaledGeneralizedDistanceRBFKernel::oldread(istream& in)
{
  readHeader(in,"ScaledGeneralizedDistanceRBFKernel");
  inherited::oldread(in);
  readField(in,"phi", phi);
  readField(in,"a", a);
  readField(in,"b",b);
  readField(in,"c",c);
  readFooter(in,"ScaledGeneralizedDistanceRBFKernel");
}
// recognized options are "b" and "c"
/*
void ScaledGeneralizedDistanceRBFKernel::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="b")
    PLearn::read(in, b); 
  if (optionname=="c")
    PLearn::read(in, c); 
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void ScaledGeneralizedDistanceRBFKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "phi", &ScaledGeneralizedDistanceRBFKernel::phi, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "a", &ScaledGeneralizedDistanceRBFKernel::phi, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "b", &ScaledGeneralizedDistanceRBFKernel::phi, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "c", &ScaledGeneralizedDistanceRBFKernel::phi, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(LaplacianKernel, "ONE LINE DESCR", "NO HELP");
real LaplacianKernel::evaluate(const Vec& x1, const Vec& x2) const
{ 
#ifdef BOUNDCHECK
  if(x1.length()!=x2.length())
    PLERROR("IN LaplacianKernel::evaluate x1 and x2 must have the same length");
#endif

  real summ = 0.0;
  real* v1=x1.data();
  real* v2=x2.data();
  int n=x1.length();
  for(int i=0; i<n; i++)
    summ += fabs(v1[i]-v2[i]);
  return exp(-phi*summ);
}

void LaplacianKernel::write(ostream& out) const
{
  writeHeader(out,"LaplacianKernel");
  inherited::oldwrite(out);
  writeField(out,"phi",phi);
  writeFooter(out,"LaplacianKernel");
}
void LaplacianKernel::oldread(istream& in)
{
  readHeader(in,"LaplacianKernel");
  inherited::oldread(in);
  readField(in,"phi",phi);
  readFooter(in,"LaplacianKernel");
}
// recognized option is "phi"
/*
void LaplacianKernel::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="phi")
    PLearn::read(in, phi); 
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void LaplacianKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "phi", &LaplacianKernel::phi, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(ScaledLaplacianKernel, "ONE LINE DESCR", "NO HELP");
void ScaledLaplacianKernel::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Kernel::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(phi, copies);
}
void ScaledLaplacianKernel::write(ostream& out) const
{
  writeHeader(out,"ScaledLaplacianKernel");
  inherited::oldwrite(out);
  writeField(out,"phi",phi);
  writeFooter(out,"ScaledLaplacianKernel");
}
void ScaledLaplacianKernel::oldread(istream& in)
{
  readHeader(in,"ScaledLaplacianKernel");
  inherited::oldread(in);
  readField(in,"phi",phi);
  readFooter(in,"ScaledLaplacianKernel");
}

real ScaledLaplacianKernel::evaluate(const Vec& x1, const Vec& x2) const
{ 
#ifdef BOUNDCHECK
  if(x1.length()!=x2.length() || x1.length()!=phi.length())
    PLERROR("IN ScaledLaplacianKernel::evaluate x1 and x2 and phi must have the same length");
#endif

  real summ = 0.0;
  real* v1=x1.data();
  real* v2=x2.data();
  real* ph=phi.data();
  int n=x1.length();
  for(int i=0; i<n; i++)
    summ += fabs(v1[i]-v2[i])*ph[i];
  return exp(-summ);
}

PLEARN_IMPLEMENT_OBJECT(DifferenceKernel, "ONE LINE DESCR", "NO HELP");
real DifferenceKernel::evaluate(const Vec& x1, const Vec& x2) const
{ 
  real result = 0.0;
  for(int i=0; i<x1.length(); i++)
    result += x1[i]-x2[i];
  return result;
}
void DifferenceKernel::write(ostream& out) const
{
  writeHeader(out,"DifferenceKernel"); 
  inherited::oldwrite(out);
  writeFooter(out,"DifferenceKernel");
}
void DifferenceKernel::oldread(istream& in)
{
  readHeader(in,"DifferenceKernel");
  inherited::oldread(in);
  readFooter(in,"DifferenceKernel");
}

PLEARN_IMPLEMENT_OBJECT(NegKernel, "ONE LINE DESCR", "NO HELP");
void NegKernel::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Kernel::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(ker, copies);
}
real NegKernel::evaluate(const Vec& x1, const Vec& x2) const
{ return -ker->evaluate(x1, x2); }

void NegKernel::write(ostream& out) const
{
  writeHeader(out,"NegKernel");
  inherited::oldwrite(out);
  writeField(out,"ker", ker);
  writeFooter(out,"NegKernel");
}
void NegKernel::oldread(istream& in)
{
  readHeader(in,"NegKernel");
  inherited::oldread(in);
  readField(in,"ker", ker);
  readFooter(in,"NegKernel");
}

PLEARN_IMPLEMENT_OBJECT(NormalizedDotProductKernel, "ONE LINE DESCR", "NO HELP");
real NormalizedDotProductKernel::evaluate(const Vec& x1, const Vec& x2) const
{ return dot(x1,x2)/(norm(x1,norm_to_use)*norm(x2,norm_to_use)); }

void NormalizedDotProductKernel::write(ostream& out) const
{
  writeHeader(out,"NormalizedDotProductKernel");
  inherited::oldwrite(out);
  writeField(out,"norm_to_use",norm_to_use);
  writeFooter(out,"NormalizedDotProductKernel");
}
void NormalizedDotProductKernel::oldread(istream& in)
{
  readHeader(in,"NormalizedDotProductKernel");
  inherited::oldread(in);
  readField(in,"norm_to_use",norm_to_use);
  readFooter(in,"NormalizedDotProductKernel");
}
// recognized option is "norm_to_use"
/*
void NormalizedDotProductKernel::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="norm_to_use")
    PLearn::read(in, norm_to_use); 
  else
    inherited::readOptionVal(in, optionname);  
}
*/

void NormalizedDotProductKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "norm_to_use", &NormalizedDotProductKernel::norm_to_use, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(SquaredErrorCostFunction, "ONE LINE DESCR", "NO HELP");
string SquaredErrorCostFunction::info() const { return "squared_error"; }
real SquaredErrorCostFunction::evaluate(const Vec& output, const Vec& target) const
{
#ifdef BOUNDCHECK
  if(target.length()!=output.length() && classification==false)
    PLERROR("In SquaredErrorCostFunction::evaluate target.length() %d should be equal to output.length() %d",target.length(),output.length());
#endif

  real result = 0.0;
  if (targetindex>=0)
    result = square(output[targetindex]-target[targetindex]);
  else
    {
      real* outputdata = output.data();
      real* targetdata = target.data();
      if (classification) {
	if (target.length() != 1)
	  PLERROR("In SquaredErrorCostFunction::evaluate target.length() %s should be 1", target.length());

	for (int i = 0; i < output.length(); ++i)
	  if (i == targetdata[0])
	    result += square(outputdata[i] - hotvalue);
	  else
	    result += square(outputdata[i] - coldvalue);
      } else {
	for(int i=0; i<output.length(); i++)
	  result += square(outputdata[i]-targetdata[i]);
      }
    }
  return result;
}

void SquaredErrorCostFunction::write(ostream& out) const
{
  writeHeader(out,"SquaredErrorCostFunction");
  writeField(out,"targetindex",targetindex);
  writeField(out, "classification", classification);
  writeField(out, "coldvalue", coldvalue);
  writeField(out, "hotvalue", hotvalue);
  writeFooter(out,"SquaredErrorCostFunction");
}

void SquaredErrorCostFunction::oldread(istream& in)
{
  readHeader(in,"SquaredErrorCostFunction");
  readField(in,"targetindex",targetindex);
  if (in.peek() != '<') {
    // Newly added stuff, hack
    readField(in, "classification", classification); 
    readField(in, "coldvalue", coldvalue);
    readField(in, "hotvalue", hotvalue);
  }
  readFooter(in,"SquaredErrorCostFunction");

  if (hotvalue == -1)
    classification = true;
}
/*
// recognized option is "norm_to_use"
void SquaredErrorCostFunction::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="targetindex")
    PLearn::read(in,targetindex);
  if (optionname == "hotvalue")
    PLearn::read(in,hotvalue);
  if (optionname == "coldvalue")
    PLearn::read(in,coldvalue);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void SquaredErrorCostFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "targetindex", &SquaredErrorCostFunction::targetindex, OptionBase::buildoption, "Index of target");
    declareOption(ol, "hotvalue", &SquaredErrorCostFunction::hotvalue, OptionBase::buildoption, "Hot value");
    declareOption(ol, "coldvalue", &SquaredErrorCostFunction::coldvalue, OptionBase::buildoption, "Cold value");
    declareOption(ol, "classification", &SquaredErrorCostFunction::classification, OptionBase::buildoption, "Used as classification cost");
    inherited::declareOptions(ol);
}


// *** NegOutputCostFunction ***

PLEARN_IMPLEMENT_OBJECT(NegOutputCostFunction, "ONE LINE DESCR", "NO HELP");

real NegOutputCostFunction::evaluate(const Vec& output, const Vec& target) const
{ return -output[0]; }

void NegOutputCostFunction::write(ostream& out) const
{
  writeHeader(out,"NegOutputCostFunction");
  writeFooter(out,"NegOutputCostFunction");
}

void NegOutputCostFunction::oldread(istream& in)
{
  readHeader(in,"NegOutputCostFunction");
  readFooter(in,"NegOutputCostFunction");
}

// **** ClassErrorCostFunction ****

PLEARN_IMPLEMENT_OBJECT(ClassErrorCostFunction, "ONE LINE DESCR", "NO HELP");
string ClassErrorCostFunction::info() const { return "class_error"; }
real ClassErrorCostFunction::evaluate(const Vec& output, const Vec& target) const
{
  if(output_is_classnum)
  {
    if(is_integer(output[0]))
      return output[0]==target[0] ?0 :1;
    else if(target[0]==1.)
      return output[0]>0.5 ?0 :1;
    else // target[0]==0 or -1
      return output[0]<=0.5 ?0 :1;
  }

  if(output.length()==1) // we assume the sign of output indicates the chosen class 
  {
    if(target[0]>0)
      return output[0]>0 ?0. :1.;
    else
      return output[0]<0 ?0. :1.;
  }
  else // we assume output gives a score for each class
  {
    int trueclass;
    if(target.length()==1)
      trueclass = int(target[0]);
    else
      trueclass = argmax(target);
    return argmax(output)==trueclass ?0. :1.;
  }
  return 1.; // to make the compiler happy
}

void ClassErrorCostFunction::write(ostream& out) const
{
  writeHeader(out,"ClassErrorCostFunction");
  writeField(out,"output_is_classnum",output_is_classnum);
  writeFooter(out,"ClassErrorCostFunction");
}
void ClassErrorCostFunction::oldread(istream& in)
{
  readHeader(in,"ClassErrorCostFunction");
  readField(in,"output_is_classnum",output_is_classnum);
  readFooter(in,"ClassErrorCostFunction");
}
// recognized option is "norm_to_use"
/*
void ClassErrorCostFunction::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="output_is_classnum")
    PLearn::read(in,output_is_classnum);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void
ClassErrorCostFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "output_is_classnum", &ClassErrorCostFunction::output_is_classnum, OptionBase::buildoption, "Output of learner is class number");
    inherited::declareOptions(ol);
}

// **** MulticlassErrorCostFunction ****

PLEARN_IMPLEMENT_OBJECT(MulticlassErrorCostFunction, "ONE LINE DESCR", "NO HELP");
string MulticlassErrorCostFunction::info() const { return "multiclass_error"; }
real MulticlassErrorCostFunction::evaluate(const Vec& output, const Vec& target) const
{
  if (output.length() != target.length())
    PLERROR("In MulticlassErrorCostFunction::evaluate: Output vec and target vec must have the same length (%d!=%d", output.length(),target.length());

  real cost = 0.0;
  for (int i=0; i<output.length(); i++)
  {
    real output_i = output[i];
    int target_i = (int)target[i];
    cost += (target_i==1) ? output_i<0.5 : output_i>0.5;
  }
  return cost;
}

void MulticlassErrorCostFunction::write(ostream& out) const
{
  writeHeader(out,"MulticlassErrorCostFunction");
  writeFooter(out,"MulticlassErrorCostFunction");
}
void MulticlassErrorCostFunction::oldread(istream& in)
{
  readHeader(in,"MulticlassErrorCostFunction");
  readFooter(in,"MulticlassErrorCostFunction");
}

PLEARN_IMPLEMENT_OBJECT(ClassMarginCostFunction, "ONE LINE DESCR", "NO HELP");
string ClassMarginCostFunction::info() const { return "class_margin"; }
real ClassMarginCostFunction::evaluate(const Vec& output, const Vec& target) const
{
  real margin;
  if (output.length()==1)
  {
    real out = output[0];
    if (output_is_positive)
      out = 2*out-1;
    margin = binary_target_is_01 ? out*(target[0]-0.5)*4.0 : out*target[0];
  }
  else // we assume output gives a score for each class
  {
    int trueclass;
    if (target.length()==1)
    {
      trueclass = int(target[0]);
      if (!binary_target_is_01)
        trueclass = (trueclass+1)/2;
    }
    else
      trueclass = argmax(target);
    
    real trueclass_score = output[trueclass];
    output[trueclass] = -FLT_MAX;
    real otherclass_score = max(output);
    output[trueclass] = trueclass_score;
    margin = trueclass_score-otherclass_score;
  }
  return -margin;
}

void ClassMarginCostFunction::write(ostream& out) const
{
  writeHeader(out,"ClassMarginCostFunction");
  writeField(out,"binary_target_is_01",binary_target_is_01);
  writeField(out,"output_is_positive",output_is_positive);
  writeFooter(out,"ClassMarginCostFunction");
}
void ClassMarginCostFunction::oldread(istream& in)
{
  readHeader(in,"ClassMarginCostFunction");
  readField(in,"binary_target_is_01",binary_target_is_01);
  readField(in,"output_is_positive",output_is_positive);
  readFooter(in,"ClassMarginCostFunction");
}
// recognized option is "norm_to_use"
/*
void ClassMarginCostFunction::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="binary_target_is_01")
    PLearn::read(in,binary_target_is_01);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void ClassMarginCostFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "binary_target_is_01", &ClassMarginCostFunction::binary_target_is_01, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(ClassDistanceProportionCostFunction, "ONE LINE DESCR", "NO HELP");
real ClassDistanceProportionCostFunction::evaluate(const Vec& output, const Vec& target) const
{
  if (output.length()==1)
    PLERROR("In ClassDistanceProportionCostFunction::evaluate, output should be multiclass and contain negative distances to each class");

  int trueclass;
  if (target.length()==1)
    trueclass = int(target[0]);
  else
    trueclass = argmax(target);
    
  real trueclass_score = output[trueclass];
  output[trueclass] = -FLT_MAX;
  real otherclass_score = max(output);
  output[trueclass] = trueclass_score;
  return trueclass_score/(trueclass_score+otherclass_score);
}

void ClassDistanceProportionCostFunction::write(ostream& out) const
{
  writeHeader(out,"ClassDistanceProportionCostFunction"); 
  writeFooter(out,"ClassDistanceProportionCostFunction");
}
void ClassDistanceProportionCostFunction::oldread(istream& in)
{
  readHeader(in,"ClassDistanceProportionCostFunction");
  readFooter(in,"ClassDistanceProportionCostFunction");
}


PLEARN_IMPLEMENT_OBJECT(NegLogProbCostFunction, "ONE LINE DESCR", "NO HELP");
string NegLogProbCostFunction::info() const { return "negative_log_probability"; }

#define smoothmap sigmoid
real NegLogProbCostFunction::evaluate(const Vec& output, const Vec& target) const
{
  real prob = 0.;
  int desired_class = int(target[0]);
  if (desired_class == -1) desired_class=0;
  if(output.length()==1) // we assume output[0] gives the probability of having class 1 
  {
    prob = output[0];
    if(smooth_map_outputs)
      prob = smoothmap(prob);
    if(desired_class==0)
      prob = 1-prob;
  }
  else 
  {
    if(!normalize) // we assume output gives a real probability for each class
#if USING_MPI      
#define SEND_PROB_TAG 981
    {
      // EACH CPU ONLY CARRIES THE CORRECT outputs IN THE INTERVAL
      // GIVEN BY [out_start,out_end).
      // THE RESULT WILL BE THAT CPU 0 WILL HAVE
      // THE PROBABILITY OF desired_class IN prob
      // (and the other CPUs will have some dummy value)
      if (PLMPI::size>1 && out_end>=0)
      {
        if (desired_class>=out_start && desired_class<out_end)
        {
          prob = output[desired_class];
          if (PLMPI::rank>0) // send it to CPU 0
          {
            MPI_Send(&prob,1,PLMPI_REAL,0,SEND_PROB_TAG,MPI_COMM_WORLD);
          }
        }
        else
        {
          if (PLMPI::rank==0)
          {
            MPI_Status status;
            MPI_Recv(&prob,1,PLMPI_REAL,MPI_ANY_SOURCE,SEND_PROB_TAG,MPI_COMM_WORLD,&status);
          }
          else 
          {
            prob = 1; // dummy value (whose log exists)
          }
        }
      }
      else
        prob = output[desired_class];
    }
#else
      prob = output[desired_class];
#endif
    else // outputs may not sum to 1, so we'll normalize them
    {
#if USING_MPI      
      if (PLMPI::size>1 && out_end>=0)
        PLERROR("condprob used in parallel mode: normalize not implemented");
#endif
      real* outputdata = output.data();
      if(smooth_map_outputs) // outputs may be slightly smaller than 0 or slightly larger than 1
      {                      // so we'll smooth-map them to fit in range [0,1] before normalizing 
        real outputsum = 0.0;
        for(int i=0; i<output.length(); i++)
          outputsum += smoothmap(outputdata[i]);
        prob = smoothmap(outputdata[desired_class])/outputsum;
      }
      else
        prob = output[desired_class]/sum(output);
    }
  }
  return -safeflog(prob);
}

void NegLogProbCostFunction::write(ostream& out) const
{
  writeHeader(out,"NegLogProbCostFunction");
  writeField(out,"normalize",normalize);
  writeField(out,"smooth_map_outputs",smooth_map_outputs);
  writeFooter(out,"NegLogProbCostFunction");
}
void NegLogProbCostFunction::oldread(istream& in)
{
  readHeader(in,"NegLogProbCostFunction");
  readField(in,"normalize",normalize);
  readField(in,"smooth_map_outputs",smooth_map_outputs);
  readFooter(in,"NegLogProbCostFunction");
}
// recognized options are "normalize" and "smooth_map_outputs"
/*
void NegLogProbCostFunction::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="normalize")
    PLearn::read(in,normalize);
  if (optionname=="smooth_map_outputs")
    PLearn::read(in,smooth_map_outputs);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void NegLogProbCostFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "normalize", &NegLogProbCostFunction::normalize, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "smooth_map_outputs", &NegLogProbCostFunction::smooth_map_outputs, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}   

PLEARN_IMPLEMENT_OBJECT(WeightedCostFunction, "ONE LINE DESCR", "NO HELP");
void WeightedCostFunction::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Kernel::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(costfunc, copies);
}
string WeightedCostFunction::info() const { return "weighted "+costfunc->info(); }
real WeightedCostFunction::evaluate(const Vec& output, const Vec& target) const
{ return target[target.length()-1] * costfunc(output,target.subVec(0,target.length()-1)); }

void WeightedCostFunction::write(ostream& out) const
{
  writeHeader(out,"WeightedCostFunction"); 
  writeField(out,"costfunc",costfunc);
  writeFooter(out,"WeightedCostFunction");
}
void WeightedCostFunction::oldread(istream& in)
{
  readHeader(in,"WeightedCostFunction");
  readField(in,"costfunc",costfunc);
  readFooter(in,"WeightedCostFunction");
}


PLEARN_IMPLEMENT_OBJECT(SelectedOutputCostFunction, "ONE LINE DESCR", "NO HELP");
void SelectedOutputCostFunction::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Kernel::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(costfunc, copies);
}
string SelectedOutputCostFunction::info() const { return "selected_output[" + tostring(outputindex) + "] "+costfunc->info(); }
real SelectedOutputCostFunction::evaluate(const Vec& output, const Vec& target) const
{ return costfunc(output.subVec(outputindex,1),target.subVec(outputindex,1)); }

void SelectedOutputCostFunction::write(ostream& out) const
{
  writeHeader(out,"SelectedOutputCostFunction");
  writeField(out,"outputindex",outputindex);
  writeField(out,"costfunc",costfunc);
  writeFooter(out,"SelectedOutputCostFunction");
}
void SelectedOutputCostFunction::oldread(istream& in)
{
  readHeader(in,"SelectedOutputCostFunction");
  readField(in,"outputindex",outputindex);
  readField(in,"costfunc",costfunc);
  readFooter(in,"SelectedOutputCostFunction");
}
// recognized option is "norm_to_use"
/*
void SelectedOutputCostFunction::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="outputindex")
    PLearn::read(in,outputindex);
  else
    inherited::readOptionVal(in, optionname);  
}
*/

void SelectedOutputCostFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "outputindex", &SelectedOutputCostFunction::outputindex, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(LiftBinaryCostFunction, "ONE LINE DESCR", "NO HELP");
string LiftBinaryCostFunction::info() const { return "lift_binary_function"; }
real LiftBinaryCostFunction::evaluate(const Vec& output, const Vec& target) const
{
  int len = output.length();
  if ((target.length()!=1 && target.length()!=2) || len>2)
    PLERROR("For binary problems, the output and binary vectors must have length 1 or 2 (%d)", len);

  if (len == 1)
  {
    real out0 = output[0];
    if (make_positive_output) out0 = sigmoid(out0);
    return (target[0]>0.5) ? out0 : -out0;
  }
  else
  {
    real out1 = output[1];
    if (make_positive_output) out1 = sigmoid(out1);
    if (target.length()==2)
      return (target[1]>target[0]) ? out1 : -out1;
    else
      return (target[0]>0.5) ? out1 : -out1;
  }

  return 0.0;  // to make the compiler happy...
}

void LiftBinaryCostFunction::write(ostream& out) const
{
  writeHeader(out,"LiftBinaryCostFunction");
  writeField(out,"make_positive_output",make_positive_output);
  writeFooter(out,"LiftBinaryCostFunction");
}
void LiftBinaryCostFunction::oldread(istream& in)
{
  readHeader(in,"LiftBinaryCostFunction");
  readField(in,"make_positive_output",make_positive_output);
  readFooter(in,"LiftBinaryCostFunction");
}
// recognized option is "norm_to_use"
/*
void LiftBinaryCostFunction::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="make_positive_output")
    PLearn::read(in,make_positive_output);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void LiftBinaryCostFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "make_positive_output", &LiftBinaryCostFunction::make_positive_output, OptionBase::buildoption,
                   "TODO: Some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(DirectNegativeCostFunction, "ONE LINE DESCR", "NO HELP");

string DirectNegativeCostFunction::info() const { return "direct_negative_cost_function"; }

real DirectNegativeCostFunction::evaluate(const Vec& output, const Vec& target) const
{
  if(output.length() != 1)
    PLERROR("Output should be a scalar");
  return -output[0];
}

void DirectNegativeCostFunction::write(ostream& out) const
{
  writeHeader(out,"DirectNegativeCostFunction");
  writeFooter(out,"DirectNegativeCostFunction");
}
void DirectNegativeCostFunction::oldread(istream& in)
{
  readHeader(in,"DirectNegativeCostFunction");
  readFooter(in,"DirectNegativeCostFunction");
}

string QuadraticUtilityCostFunction::info() const { return "quadratic_risk"; }

real QuadraticUtilityCostFunction::evaluate(const Vec& output, const Vec& target) const
{
  real profit = profit_function(output,target);
  return  -profit + risk_aversion*profit*profit;
}

void QuadraticUtilityCostFunction::write(ostream& out) const
{
  writeHeader(out,"QuadraticUtilityCostFunction");
  writeField(out,"risk_aversion",risk_aversion);
  writeField(out,"profit_function",profit_function);
  writeFooter(out,"QuadraticUtilityCostFunction");
}
void QuadraticUtilityCostFunction::oldread(istream& in)
{
  readHeader(in,"QuadraticUtilityCostFunction");
  readField(in,"risk_aversion",risk_aversion);
  readField(in,"profit_function",profit_function);
  readFooter(in,"QuadraticUtilityCostFunction");
}

// recognized option is "risk_aversion"
/*
void QuadraticUtilityCostFunction::readOptionVal(istream& in, const string& optionname)
{
  if (optionname=="risk_aversion")
    PLearn::read(in,risk_aversion);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void QuadraticUtilityCostFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "risk_aversion", &QuadraticUtilityCostFunction::risk_aversion, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(QuadraticUtilityCostFunction, "ONE LINE DESCR", "NO HELP");

string PricingTransactionPairProfitFunction::info() const { return "pricing_pair_profit"; }

real PricingTransactionPairProfitFunction::evaluate(const Vec& output, const Vec& target) const
{
  real nb_units_transaction = output[0];
  real cash_earned_at_t1 = output[1];
  real price_t2 = target[0];
  real transaction_loss_t2 = nb_units_transaction>0 ? additive_cost + 
    fabs(nb_units_transaction) * (price_t2 * multiplicative_cost + per_unit_cost) : 0;
  real profit = cash_earned_at_t1 + nb_units_transaction * price_t2 - transaction_loss_t2;
  return profit;
}

void PricingTransactionPairProfitFunction::write(ostream& out) const
{
  writeHeader(out,"PricingTransactionPairProfitFunction");
  writeField(out,"multiplicative_cost",multiplicative_cost);
  writeField(out,"addititive_cost",additive_cost);
  writeField(out,"per_unit_cost",per_unit_cost);
  writeFooter(out,"PricingTransactionPairProfitFunction");
}
void PricingTransactionPairProfitFunction::oldread(istream& in)
{
  readHeader(in,"PricingTransactionPairProfitFunction");
  readField(in,"multiplicative_cost",multiplicative_cost);
  readField(in,"addititive_cost",additive_cost);
  readField(in,"per_unit_cost",per_unit_cost);
  readFooter(in,"PricingTransactionPairProfitFunction");
}

/*
void PricingTransactionPairProfitFunction::readOptionVal(istream& in, const string& optionname)
{
  if (optionname == "multiplicative_cost")
    PLearn::read(in,multiplicative_cost);
  else if (optionname == "additive_cost")
    PLearn::read(in,additive_cost);
  else if (optionname == "per_unit_cost")
    PLearn::read(in,per_unit_cost);
  else
    inherited::readOptionVal(in, optionname);  
}
*/
void PricingTransactionPairProfitFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "multiplicative_cost", &PricingTransactionPairProfitFunction::multiplicative_cost, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "additive_cost", &PricingTransactionPairProfitFunction::additive_cost, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "per_unit_cost", &PricingTransactionPairProfitFunction::per_unit_cost, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}

PLEARN_IMPLEMENT_OBJECT(PricingTransactionPairProfitFunction, "ONE LINE DESCR", "NO HELP");


// *****************
// * KernelVMatrix *
// *****************

KernelVMatrix::KernelVMatrix(VMat data1, VMat data2, Ker the_ker)
  : VMatrix(data1->length(), data2->length()), 
    d1(data1), d2(data2), ker(the_ker), 
    input1(data1->width()), input2(data2->width())
{}

/*
IMPLEMENT_NAME_AND_DEEPCOPY(KernelVMatrix);
void KernelVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Kernel::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(d1, copies);
  deepCopyField(d2, copies);
  deepCopyField(ker, copies);
  deepCopyField(input1, copies);
  deepCopyField(input2, copies);
}
*/

real KernelVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j>=width())
    PLERROR("In KernelVMatrix::get OUT OF BOUNDS");
#endif

  d1->getRow(i,input1);
  d2->getRow(j,input2);
  return ker(input1,input2);
}

void KernelVMatrix::getSubRow(int i, int j, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j+v.length()>width())
    PLERROR("In KernelVMatrix::getRow OUT OF BOUNDS");
#endif

  d1->getRow(i,input1);
  for(int jj=0; jj<v.length(); jj++)
    {
      d2->getRow(j+jj,input2);
      v[jj] = ker(input1,input2);
    }
}


// last column of data is supposed to be a classnum
// returns a matrix of (index1, index2, distance)
Mat findClosestPairsOfDifferentClass(int k, VMat data, Ker dist)
{
  Mat result(k,3);
  real maxdistinlist = -FLT_MAX;
  int posofmaxdistinlist = -1;
  int kk=0; // number of pairs already in list
  Vec rowi(data.width());
  Vec inputi = rowi.subVec(0,rowi.length()-1);
  real& targeti = rowi[rowi.length()-1];
  Vec rowj(data.width());
  Vec inputj = rowj.subVec(0,rowj.length()-1);
  real& targetj = rowj[rowj.length()-1];
  for(int i=0; i<data.length(); i++)
  {
    data->getRow(i,rowi);
    for(int j=0; j<data.length(); j++)
    {
      data->getRow(j,rowj);
      if(targeti!=targetj)
      {
        real d = dist(inputi,inputj);
        if(kk<k)
        {
          result(kk,0) = i;
          result(kk,1) = j;
          result(kk,2) = d;
          if(d>maxdistinlist)
          {
            maxdistinlist = d;
            posofmaxdistinlist = kk;
          }
          kk++;
        }
        else if(d<maxdistinlist)
        {
          result(posofmaxdistinlist,0) = i;
          result(posofmaxdistinlist,1) = j;
          result(posofmaxdistinlist,2) = d;
          posofmaxdistinlist = argmax(result.column(2));
          maxdistinlist = result(posofmaxdistinlist,2);
        }
      }
    }
  }
  sortRows(result, 2);
  return result;
}

template <>
void deepCopyField(Ker& field, CopiesMap& copies)
{
  if (field)
    field = static_cast<Kernel*>(field->deepCopy(copies));
}

%> // end of namespace PLearn

