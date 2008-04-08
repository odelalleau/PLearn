// -*- C++ -*-

// HistogramDistribution.cc
//
// Copyright (C) 2002 Yoshua Bengio, Pascal Vincent, Xavier Saint-Mleux
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

/*! \file HistogramDistribution.cc */
#include "HistogramDistribution.h"
//#include <algorithm>
//#include <cmath>

namespace PLearn {
using namespace std;

HistogramDistribution::HistogramDistribution() {}

HistogramDistribution::HistogramDistribution(VMat data, PP<Binner> binner_,
                                             PP<Smoother> smoother_)
    :bin_positions(data.length()+1), bin_density(data.length()), survival_values(data.length()),
     binner(binner_), smoother(smoother_)
{
    setTrainingSet(data);
    train();
}

PLEARN_IMPLEMENT_OBJECT(HistogramDistribution,
                        "Represents and possibly learns (using a smoother) a univariate distribution as a histogram.",
                        "This class represents a univariate distribution with a set of bins and their densities\n"
                        "The bins can be fixed or learned by a Binner object, and the densities\n"
                        "can be learned from a training set. The empirical densities in the bins can also\n"
                        "be smoothed with a Smoother (which is a general purpose univariate function\n"
                        "smoothing mechanism. If the data is not univariate, then only the LAST column\n"
                        "is considered. The smoother can either smooth the density or the survival fn.\n");

void HistogramDistribution::declareOptions(OptionList& ol)
{
    declareOption(ol, "bin_positions", &HistogramDistribution::bin_positions, OptionBase::learntoption,
                  "The n+1 positions that define n bins. There is one more bin position "
                  "than number of bins, all the bins are supposed adjacent.");

    declareOption(ol, "bin_density", &HistogramDistribution::bin_density, OptionBase::learntoption,
                  "Density of the distribution for each bin.  The density is supposed "
                  "constant within each bin:\n"
                  "\t p(x) = bin_density[i] if bin_positions[i] < x <= bin_positions[i+1].");

    declareOption(ol, "survival_values", &HistogramDistribution::survival_values, OptionBase::learntoption,
                  "Redundant with density is the pre-computed survival function.");

    declareOption(ol, "binner", &HistogramDistribution::binner, OptionBase::buildoption,
                  "Used to do binning at training time (although a fixed binning scheme can be\n"
                  "obtained by using a ManualBinner.B)");

    declareOption(ol, "smoother", &HistogramDistribution::smoother, OptionBase::buildoption,
                  "Used to smooth learned density (or survival) at train time, after the empirical\n"
                  "frequencies of each bin have been collected\n");

    declareOption(ol, "smooth_density_instead_of_survival_fn",
                  &HistogramDistribution::smooth_density_instead_of_survival_fn, OptionBase::buildoption,
                  "whether to smooth the density or the survival function, with the smoother\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void HistogramDistribution::build_()
{
}

// ### Nothing to add here, simply calls build_
void HistogramDistribution::build()
{
    inherited::build();
    build_();
}

void HistogramDistribution::train()
{

    /*
      - prend la distri empirique
      | trie les points
      | merge les bins (possiblement sous contraintes)
      |     - points de coupure predefinis (option include_cutpoints) ManualBinner
      |     - largeur des bins > a une valeur minimale
      |     - bins contenir un minimum de points
      Binner

      Smoother
      (recalcule la densite)

      calculer survival_values
    */

    if(train_set->width() != inputsize()+targetsize())
        PLERROR("In HistogramDistribution::train(VMat training_set) training_set->width() != inputsize()+targetsize()");
    if(train_set->width() != 1)
        PLERROR("In HistogramDistribution::train() train_set->width() must be 1 (column vec.)");
    if(binner == 0)
        PLERROR("In HistogramDistribution::train() Can't train without a Binner.");

    Vec data(train_set.length());
    data << train_set.getColumn(train_set.width()-1);

    PP<RealMapping> binning= binner->getBinning(train_set);
    binning->setMappingForOther(0.0);
    binning->transform(data);

    bin_positions= binning->getCutPoints();
    bin_density.resize(bin_positions.length()-1);
    survival_values.resize(bin_positions.length()-1);

    for(int i= 0; i < data.length(); ++i)
        ++survival_values[static_cast<int>(data[i])];
    for(int i= survival_values.length()-2; i >= 0; --i)
        survival_values[i]+= survival_values[i+1];
    for(int i= survival_values.length()-1; i >= 0; --i)
        survival_values[i]/= survival_values[0];

    if(smoother)
    {
        if (smooth_density_instead_of_survival_fn)
        {
            calc_density_from_survival();
            Vec df(bin_density.length());
            df << bin_density;
            smoother->smooth(df, bin_density, bin_positions, bin_positions);
            calc_survival_from_density();
        }
        else
        {
            Vec sv(survival_values.length());
            sv << survival_values;
            smoother->smooth(sv, survival_values, bin_positions, bin_positions);
            calc_density_from_survival();
        }
    }
    else
        calc_density_from_survival();
}

void HistogramDistribution::computeOutput(const Vec& input, Vec& output)
{
    if(input.size() != 1 || output.size() != 1)
        PLERROR("In HistogramDistribution::use  implemented only for reals; i.e. input.size()=output.size()=1.  "
                "Got input.size()=%d and output.size()=%d", input.size(), output.size());
    // outputs_def: 'l'->log_density, 'd' -> density, 'c' -> cdf, 's' -> survival_fn, 'e' -> expectation, 'v' -> variance
    if(outputs_def == "l") output[0]= log_density(input);
    else if(outputs_def == "d") output[0]= density(input);
    else if(outputs_def == "c") output[0]= cdf(input);
    else if(outputs_def == "s") output[0]= survival_fn(input);
    else if(outputs_def == "e") { Vec mu(1); expectation(mu); output[0]= mu[0]; }
    else if(outputs_def == "v") { Mat m(1,1); variance(m); output[0]= m(0,0); }
    else PLERROR("In HistogramDistribution::use  unknown value for outputs_def= \"%s\"", outputs_def.c_str());
}

void HistogramDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(bin_positions, copies);
    deepCopyField(bin_density, copies);
    deepCopyField(survival_values, copies);
    deepCopyField(binner, copies);
    deepCopyField(smoother, copies);
}

real HistogramDistribution::log_density(const Vec& x) const
{
    return pl_log(density(x));
}


real HistogramDistribution::density(const Vec& x) const
{
    if(x.size() != 1)
        PLERROR("HistogramDistribution::density implemented only for univariate data (vec size == 1).");
    return bin_density[find_bin(x[0])];
}


real HistogramDistribution::survival_fn(const Vec& x) const
{
    if(x.size() != 1)
        PLERROR("HistogramDistribution::survival_fn implemented only for univariate data (vec size == 1).");
    int bin= find_bin(x[0]);
    if(bin < 0)
        if(x[0] < bin_positions[0])
            return 1.0;
        else
            return 0.0;

    if(x[0] < bin_positions[bin] && bin >= 1)
        return survival_values[bin-1] + (x[0] - bin_positions[bin-1]) *
            (survival_values[bin] - survival_values[bin-1]) / (bin_positions[bin] - bin_positions[bin-1]);

    return survival_values[bin];
}

real HistogramDistribution::cdf(const Vec& x) const
{
    return 1.0-survival_fn(x);
}

void HistogramDistribution::expectation(Vec& mu) const
{
    if(mu.size() != 1)
        PLERROR("HistogramDistribution::expectation implemented only for univariate data (vec size == 1).");
    real sum= 0.0;
    for(int i= 0; i < bin_density.size(); ++i)
        sum+= bin_density[i] * (bin_positions[i+1]-bin_positions[i]) * (bin_positions[i]+bin_positions[i+1])/2;
    //    sum+= bin_density[i] * bin_positions[i+1];
    mu[0]=sum;
}

void HistogramDistribution::variance(Mat& cov) const
{
    if(cov.size() != 1)
        PLERROR("HistogramDistribution::variance implemented only for univariate data");
    real sumsq= 0.0, sum= 0.0, s;
    int n= bin_density.size();
    for(int i= 0; i < n; ++i)
    {
        s= bin_density[i] * (bin_positions[i+1]-bin_positions[i]) * (bin_positions[i]+bin_positions[i+1])/2;
        sum+= s;
        sumsq+= s*s;
    }
    cov(0,0) = abs(sumsq-(sum*sum)/n)/n;
}

real HistogramDistribution::prob_in_range(const Vec& x0, const Vec& x1) const
{
    return survival_fn(x0) - survival_fn(x1);
}


int HistogramDistribution::find_bin(real x) const
{
    int b= 0, e= bin_positions.length()-2, p= b+(e-b)/2;

    if(x < bin_positions[b] || x >= bin_positions[e+1])
        return -1;

    while(b < e)
    {
        if(bin_positions[p] == x)
            return p;
        if(bin_positions[p] > x)
            e= p-1;
        else
            b= p+1;
        p= b+(e-b)/2;
    }
    return p;
}

void HistogramDistribution::calc_density_from_survival()
{
    calc_density_from_survival(survival_values, bin_density, bin_positions);
    /*
      int n= bin_positions.length()-1;
      bin_density.resize(n);
      real sum= 0.0;
      for(int i= 0; i < n; ++i)
      if(bin_positions[i+1] != bin_positions[i])
      if(i == n-1)
      sum+= (bin_density[i]= survival_values[i] / (bin_positions[i+1]-bin_positions[i]));
      else
      sum+= (bin_density[i]= (survival_values[i] - survival_values[i+1]) / (bin_positions[i+1]-bin_positions[i]));
      else
      bin_density[i]= 0.0;
    */
}


void HistogramDistribution::calc_survival_from_density()
{
    calc_survival_from_density(bin_density, survival_values, bin_positions);
    /*
      int n= bin_positions.length()-1;
      survival_values.resize(n);
      real prec= 0.0;
      for(int i= n-1; i >= 0; --i)
      prec= survival_values[i]= bin_density[i]*(bin_positions[i+1]-bin_positions[i]) + prec;
      for(int i= 0; i < n; ++i)
      survival_values[i]/= prec;
    */
}

void HistogramDistribution::calc_density_from_survival(const Vec& survival, Vec& density_, const Vec& positions)
{
    int n= positions.length()-1;
    density_.resize(n);
    real sum= 0.0;
    for(int i= 0; i < n; ++i)
        if(positions[i+1] != positions[i])
            if(i == n-1)
                sum+= (density_[i]= survival[i] / (positions[i+1]-positions[i]));
            else
                sum+= (density_[i]= (survival[i] - survival[i+1]) / (positions[i+1]-positions[i]));
        else
            density_[i]= 0.0;
}

void HistogramDistribution::calc_survival_from_density(const Vec& density_, Vec& survival, const Vec& positions)
{
    int n= positions.length()-1;
    survival.resize(n);
    real prec= 0.0;
    for(int i= n-1; i >= 0; --i)
        prec= survival[i]= density_[i]*(positions[i+1]-positions[i]) + prec;
    for(int i= 0; i < n; ++i)
        survival[i]/= prec;
}


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
