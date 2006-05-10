
// -*- C++ -*-

// VMatrixFromDistribution.cc
//
// Copyright (C) 2003  Pascal Vincent
// Copyright (C) 2005  Olivier Delalleau
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
 ******************************************************* */

/*! \file VMatrixFromDistribution.cc */
#include "VMatrixFromDistribution.h"

namespace PLearn {
using namespace std;

VMatrixFromDistribution::VMatrixFromDistribution()
    :mode("sample"), generator_seed(0), nsamples(0)
    /* ### Initialize all fields to their default value */
{
    samples_per_dim = 10;
}

PLEARN_IMPLEMENT_OBJECT(VMatrixFromDistribution, "A VMatrix built from sampling a distribution",
                        "VMatrixFromDistribution implements a VMatrix whose data rows are drawn from a distribution\n"
                        "or that contains the density or log density sampled on a grid (depending on \"mode\").\n"
                        "The matrix is computed in memory at build time\n");

void VMatrixFromDistribution::declareOptions(OptionList& ol)
{
    declareOption(ol, "distr", &VMatrixFromDistribution::distr, OptionBase::buildoption,
                  "The distribution this matrix will be generated from\n");

    declareOption(ol, "mode", &VMatrixFromDistribution::mode, OptionBase::buildoption,
                  "mode can be one of:\n"
                  "   \"sample\" : will draw nsamples from the distribution initializing the generator with the generator_seed \n"
                  "   \"density\" : for 1d or 2d distributions, will report the density along a grid of samples_per_dim \n"
                  "                 gridpoints per dimension. The resulting matrix will contain rows of the form [ coordinates density ] \n"
                  "   \"log_density\" : same as density, but reports the log of the density instead. \n"
        );

    declareOption(ol, "generator_seed", &VMatrixFromDistribution::generator_seed, OptionBase::buildoption,
                  "The initial generator_seed to initialize the distribution's generator");

    declareOption(ol, "nsamples", &VMatrixFromDistribution::nsamples, OptionBase::buildoption,
                  "number of samples to draw");

    declareOption(ol, "samples_per_dim", &VMatrixFromDistribution::samples_per_dim, OptionBase::buildoption,
                  "number of samples on each dimensions of the grid");

    declareOption(ol, "mins", &VMatrixFromDistribution::mins, OptionBase::buildoption,
                  "the minimum of the grid on each dimensions");

    declareOption(ol, "maxs", &VMatrixFromDistribution::maxs, OptionBase::buildoption,
                  "the maximum of the grid on each dimensions");

    inherited::declareOptions(ol);
}

void VMatrixFromDistribution::build_()
{
    if(distr)
    {
        if(mode=="sample")
        {
            length_ = nsamples;
            width_ = distr->getNPredicted();
            inputsize_ = width_;
            targetsize_ = 0;
            weightsize_ = 0;
            data.resize(length_, width_);
            distr->resetGenerator(generator_seed);
            distr->generateN(data);
        }
        else if(mode=="density" || mode=="log_density")
        {
            length_ = (int)pow(double(samples_per_dim),double(distr->inputsize()));
            width_ = distr->inputsize()+1;
            inputsize_ = distr->inputsize();
            targetsize_ = 0;
            weightsize_ = 1;

            data.resize(length_, width_);
            Vec v(data.width());
            int k=0;
            switch(distr->inputsize())
            {
            case 1:
                for(int j=0;j<samples_per_dim;j++)
                {
                    v[0] = mins[0] + ((real)j / (samples_per_dim-1)) * (maxs[0]-mins[0]);
                    if(mode=="density")
                        v[1] = distr->density(v.subVec(0,1));
                    else // log_density
                        v[1] = distr->log_density(v.subVec(0,1));
                    data(k++)<<v;
                }
                break;
            case 2:
                for(int i=0;i<samples_per_dim;i++)
                {
                    v[0] = mins[0] + ((real)i / (samples_per_dim-1)) * (maxs[0]-mins[0]);
                    for(int j=0;j<samples_per_dim;j++)
                    {
                        v[1] = mins[1] + ((real)j / (samples_per_dim-1)) * (maxs[1]-mins[1]);
                        if(mode=="density")
                            v[2] = distr->density(v.subVec(0,2));
                        else // log_density
                            v[2] = distr->log_density(v.subVec(0,2));
                        data(k++)<<v;
                    }
                }
                break;
            default:
                PLERROR("density and log_density modes only supported for distribution of dimension 1 or 2");break;
            }
        }
        else
            PLERROR("In VMatrixFromDistribution: invalid mode: %s",mode.c_str());
    }
}

// ### Nothing to add here, simply calls build_
void VMatrixFromDistribution::build()
{
    inherited::build();
    build_();
}

void VMatrixFromDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
