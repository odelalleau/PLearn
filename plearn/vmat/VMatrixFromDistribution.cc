
// -*- C++ -*-

// VMatrixFromDistribution.cc
//
// Copyright (C) 2003  Pascal Vincent 
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
 * $Id: VMatrixFromDistribution.cc,v 1.6 2004/02/20 21:14:44 chrish42 Exp $ 
 ******************************************************* */

/*! \file VMatrixFromDistribution.cc */
#include "VMatrixFromDistribution.h"

namespace PLearn {
using namespace std;

VMatrixFromDistribution::VMatrixFromDistribution() 
  :generator_seed(0), nsamples(0),density_mode(false),logdensity_mode(false)
  /* ### Initialise all fields to their default value */
{
  // ...

  samples_per_dim = 10;

  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

PLEARN_IMPLEMENT_OBJECT(VMatrixFromDistribution, "ONE LINE DESCR", "NO HELP");

void VMatrixFromDistribution::declareOptions(OptionList& ol)
{
  declareOption(ol, "distr", &VMatrixFromDistribution::distr, OptionBase::buildoption,
                "The distribution to draw from\n");

  declareOption(ol, "generator_seed", &VMatrixFromDistribution::generator_seed, OptionBase::buildoption,
                "The initial generator_seed to initialize the distribution's generator");

  declareOption(ol, "nsamples", &VMatrixFromDistribution::nsamples, OptionBase::buildoption,
                "number of samples to draw");

  declareOption(ol, "density_mode", &VMatrixFromDistribution::density_mode, OptionBase::buildoption,
                "if true, then the rows are the density sampled on a grid. Mutually exclusive to logdensity_mode");

  declareOption(ol, "logdensity_mode", &VMatrixFromDistribution::logdensity_mode, OptionBase::buildoption,
                "if true, then the rows are the logdensity sampled on a grid. Mutually exclusive to density_mode");

  declareOption(ol, "samples_per_dim", &VMatrixFromDistribution::samples_per_dim, OptionBase::buildoption,
                "number of samples on each dimensions of the grid");

  declareOption(ol, "mins", &VMatrixFromDistribution::mins, OptionBase::buildoption,
                "the minimum of the grid on each dimensions");

  declareOption(ol, "maxs", &VMatrixFromDistribution::maxs, OptionBase::buildoption,
                "the maximum of the grid on each dimensions");

  inherited::declareOptions(ol);
}

string VMatrixFromDistribution::help()
{
  return 
    "VMatrixFromDistribution implements a vmatrix whose data rows are drawn from a distribution\n"
    "The distribution is sampled at build time, and the samples cached in memory for later retrieval\n";
}

void VMatrixFromDistribution::build_()
{
  if(distr)
    {
      if(!density_mode && !logdensity_mode)
      {
        length_ = nsamples;
        width_ = distr->inputsize();
        data.resize(length_, width_);
        distr->resetGenerator(generator_seed);
        distr->generateN(data);
      }
      else
      {
        length_ = (int)pow(double(samples_per_dim),double(distr->inputsize()));
        width_ = distr->inputsize()+1;
        data.resize(length_, width_);
        Vec v(data.width());
        int k=0;
        switch(distr->inputsize())
        {
        case 1:
          for(int j=0;j<samples_per_dim;j++)
          {
            v[0] = mins[0] + ((real)j / samples_per_dim) * (maxs[0]-mins[0]);
            v[1] = distr->density(v.subVec(0,1));
            data(k++)<<v;
          }
          break;
        case 2:
          for(int i=0;i<samples_per_dim;i++)
          {
            v[0] = mins[0] + ((real)i / samples_per_dim) * (maxs[0]-mins[0]);
            for(int j=0;j<samples_per_dim;j++)
            {
              v[1] = mins[1] + ((real)j / samples_per_dim) * (maxs[1]-mins[1]);
              v[2] = distr->density(v.subVec(0,2));
              data(k++)<<v;
            }
          }
          break;
        default : PLERROR("only distribution of dimension 1 nd 2 implemented");break;
        }
      }
    }
}

// ### Nothing to add here, simply calls build_
void VMatrixFromDistribution::build()
{
  inherited::build();
  build_();
}

void VMatrixFromDistribution::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

real VMatrixFromDistribution::get(int i, int j) const
{ return data(i,j); }

void VMatrixFromDistribution::getColumn(int i, Vec v) const
{ v << data.column(i); }

void VMatrixFromDistribution::getSubRow(int i, int j, Vec v) const
{ v << data(i).subVec(j,v.length()); }

void VMatrixFromDistribution::getRow(int i, Vec v) const
{ v << data(i); }

void VMatrixFromDistribution::getMat(int i, int j, Mat m) const
{ m << data.subMat(i,j,m.length(),m.width()); }

Mat VMatrixFromDistribution::toMat() const
{ return data; }

} // end of namespace PLearn
