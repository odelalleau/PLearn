

// -*- C++ -*-

// ScaledConditionalCDFSmoother.cc
// 
// Copyright (C) *YEAR* *AUTHOR(S)* 
// ...
// Copyright (C) *YEAR* *AUTHOR(S)* 
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

/*! \file ScaledConditionalCDFSmoother.cc */

#include "ScaledConditionalCDFSmoother.h"
//#include "HistogramDistribution.h" //to get static fns. to calc survival <--> density  // already inc. from ConditionalCDFSmoother

namespace PLearn {
using namespace std;

ScaledConditionalCDFSmoother::ScaledConditionalCDFSmoother() 
    :ConditionalCDFSmoother(), preserve_relative_density(true)
{
}

PLEARN_IMPLEMENT_OBJECT(ScaledConditionalCDFSmoother, 
                        "This smoothes a low-resolution histogram using as prior a high-resolution one.", 
                        "This class takes as 'prior_cdf' a detailed histogram (usually derived from\n"
                        "an unconditional distribution) and uses it to smooth a given survival\n"
                        "function and provide extra detail (high resolution).\n"
                        "Two smoothing formula are provided, both of which guarantee that the smoothed\n"
                        "survival function takes the same value as the raw one at or near original bin\n"
                        "positions. In between the original bin positions, the smoothed survival\n"
                        "is obtained by applying one of two possible formula, according to the\n"
                        "preserve_relative_density option.\n");

void ScaledConditionalCDFSmoother::declareOptions(OptionList& ol)
{
    declareOption(ol, "preserve_relative_density", &ScaledConditionalCDFSmoother::preserve_relative_density, 
                  OptionBase::buildoption,
                  "If true then the following formula is used inside each of the large intervals (t_0,t_1):\n"
                  "  S(y_t) = S(y_{t_0})+(PS(y_t)-PS(y_{t_0}))(RS(y_{t_0})-RS(y_{t_1}))/(PS(y_{t_1})-PS(y_{t_0})\n"
                  "where S(y_t) is the smoothed survival function at position y_t, PS(y_t) is the prior\n"
                  "survival function at y_t, and RS(y_t) is the rough survival function (which is to be\n"
                  "smoothed) at y_t. Note that RS is only known at the extremes of the interval, y_{t_0}\n"
                  "and y_{t_1}. Note that this formula has the property that within the interval, the\n"
                  "density is the prior density, scaled by the ratio of the total density in the interval\n"
                  "for the target rough curve with respect to the prior curve\n"
                  "If false, then the following formula is used instead, using the same notation:\n"
                  "  S(y_t) = PS(y_t)(RS(y_{t_0})/PS(y_{t_0}) + (y_t - y_{t_0})(RS(y_{t_1})-RS(y_{t_0}))/(PS(y_{t_1}) (t_1 - t_0)))\n"
                  "What is the justification for this second formula?\n"
        );
                

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ScaledConditionalCDFSmoother::build_()
{
}

// ### Nothing to add here, simply calls build_
void ScaledConditionalCDFSmoother::build()
{
    inherited::build();
    build_();
}


void ScaledConditionalCDFSmoother::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


// To obtain each bin of the smoothed_function, scale multiplicatively each bin
// density of prior_cdf by a fixed factor per source_function bin, that
// makes it match the probability under the corresponding source_function
// bin.
real ScaledConditionalCDFSmoother::smooth(const Vec& source_function, Vec& smoothed_function, 
                                          Vec bin_positions, Vec dest_bin_positions) const
{
    // put in 'survival_fn' the multiplicatively adjusted unconditional_survival_fn
    // such that the estimatedS values at yvalues match. In each segment
    // between prev_y and next_y. The adjustment ratio varies linearly from
    // estimatedS[prev_y]/unconditionalS[prev_y] to estimatedS[next_y]/unconditionalS[next_y]):
    //  prev_ratio = estimatedS[prev_y]/unconditionalS[prev_y]
    //  next_ratio = estimatedS[next_y]/unconditionalS[next_y]
    //  adjustment = prev_ratio + (y-prev_y)*next_ratio/(next_y-prev_y)
    //  s(y) = unconditional_s(y)*adjustment

    if (!prior_cdf)
        PLERROR("in ScaledConditionalCDFSmoother::smooth  you need to supply a prior_cdf");
    //assume source_function is a survival fn.
    if(bin_positions.size() != source_function.size()+1)
        PLERROR("in ScaledConditionalCDFSmoother::smooth  you need to supply bin_positions");
    if(dest_bin_positions.size() == 0)
        PLERROR("in ScaledConditionalCDFSmoother::smooth  you need to supply dest_bin_positions");
    smoothed_function.resize(dest_bin_positions.size()-1);
 

    int j= 0;
    for(int i= 0; i < source_function.size(); ++i)
    {
        Vec v0(1), v1(1);//prev_y, next_y
        v0[0]= bin_positions[i];
        v1[0]= bin_positions[i+1];
      
        real prev_ratio=  source_function[i]/prior_cdf->survival_fn(v0);
        real next_ratio;
        if(i == source_function.size()-1)
            next_ratio= 0.0;
        else
            next_ratio=  source_function[i+1]/prior_cdf->survival_fn(v1);
      
        cout  << source_function[i] << '\t'  << prev_ratio  << '\t' << next_ratio << '\t' << v0[0] << '\t' << v1[0] << endl;
        real slope = !preserve_relative_density? 0 : 
            ((source_function[i+1]-source_function[i])/(prior_cdf->survival_fn(v1)-prior_cdf->survival_fn(v0)));
        real absisse = !preserve_relative_density? 0 : 
            (source_function[i] - slope * prior_cdf->survival_fn(v0));
        while(j < smoothed_function.size() && dest_bin_positions[j+1] <= bin_positions[i+1])
        {
            Vec v(1);
            v[0]= dest_bin_positions[j];
            // the line below seems wrong, so I have fixed it -- YB
            // the reason it seems wrong is that smoothed_function[j_final] should be equal
            // to source_function[i+1], but it is not, currently.
            // smoothed_function[j]= prior_cdf->survival_fn(v) * (prev_ratio + (v[0]-v0[0])*next_ratio/(v1[0]-v0[0]));
            if (!preserve_relative_density)
                smoothed_function[j]= prior_cdf->survival_fn(v) * 
                    (prev_ratio + (v[0]-v0[0])*(next_ratio-prev_ratio)/(v1[0]-v0[0]));
            else // scale with bin number, i.e. warped with density
                smoothed_function[j]= absisse + slope * prior_cdf->survival_fn(v);
            cout  << '\t' << v[0] << '\t' << prior_cdf->survival_fn(v) << '\t' << smoothed_function[j] << endl;
            ++j;
        }
    }
  
  












    /*
    //assume source_function is a survival fn.
    if(bin_positions.size() != source_function.size()+1)
    PLERROR("in ScaledConditionalCDFSmoother::smooth  you need to supply bin_positions");
    if(dest_bin_positions.size() == 0)
    PLERROR("in ScaledConditionalCDFSmoother::smooth  you need to supply dest_bin_positions");
    smoothed_function.resize(dest_bin_positions.size()-1);
    Vec f0(dest_bin_positions.size()-1); //new density


    int j= 0;
    real factor= 1.0;
    for(int i= 0; i < source_function.size(); ++i)
    {
    Vec v0(1), v1(1);
    v0[0]= bin_positions[i];
    v1[0]= bin_positions[i+1];
    real prior_prob= prior_cdf->survival_fn(v0) - prior_cdf->survival_fn(v1);
    real prob;
    if(i < source_function.size()-1)
    prob= (source_function[i]-source_function[i+1]);
    else
    prob= source_function[i];

    if(0 < prior_prob && prob != 0.0)
    factor= prob / prior_prob;
    // else: use prev. factor

    //dummy-temp  
    cout << v0[0] << '-' << v1[0] << ":\t" << prob << '/' <<  prior_prob << '=' << factor << endl;


    while(j < smoothed_function.size() && dest_bin_positions[j+1] <= bin_positions[i+1])
    {
    Vec v(1);
    v[0]= (dest_bin_positions[j]+dest_bin_positions[j+1])/2;
    //	  smoothed_function[j]= factor * prior_cdf->survival_fn(v);
    f0[j]= factor * prior_cdf->density(v);
    //dummy-temp  
    cout << '\t' << smoothed_function[j] << "= " <<  factor  << " * " << prior_cdf->survival_fn(v) << endl;

    ++j;
    }
    }
  

    HistogramDistribution::calc_survival_from_density(f0, smoothed_function, dest_bin_positions);

    */

    /*
      int j= 0;
      real factor= 1.0;
      for(int i= 0; i < source_function.size(); ++i)
      {
      Vec v0(1), v1(1);
      v0[0]= bin_positions[i];
      v1[0]= bin_positions[i+1];
      real prior_prob= prior_cdf->survival_fn(v0) - prior_cdf->survival_fn(v1);
      real prob;
      if(i < source_function.size()-1)
      prob= (source_function[i]-source_function[i+1]);
      else
      prob= source_function[i];

      if(0 < prior_prob && prob != 0.0)
      factor= prob / prior_prob;
      // else: use prev. factor

      //dummy-temp  
      cout << v0[0] << '-' << v1[0] << ":\t" << prob << '/' <<  prior_prob << '=' << factor << endl;


      while(j < smoothed_function.size() && dest_bin_positions[j+1] <= bin_positions[i+1])
      {
      Vec v(1);
      v[0]= (dest_bin_positions[j]+dest_bin_positions[j+1])/2;
      smoothed_function[j]= factor * prior_cdf->survival_fn(v);
      //dummy-temp  
      cout << '\t' << smoothed_function[j] << "= " <<  factor  << " * " << prior_cdf->survival_fn(v) << endl;

      ++j;
      }
      }
    */

    return 0.0; //dummy - FIXME - xsm
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
