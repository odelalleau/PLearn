// -*- C++ -*-

// PartsDistanceKernel.cc
// Copyright (C) 2008 Pascal Vincent
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

/*! \file PartsDistanceKernel.h */

/* *******************************************************      
 * $Id: PartsDistanceKernel.cc 7675 2007-06-29 19:50:49Z tihocan $
 * This file is part of the PLearn library.
 ******************************************************* */

#include "PartsDistanceKernel.h"

#include <plearn/vmat/VMat_basic_stats.h>

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    PartsDistanceKernel,
    "Implements a parts distance",
    "");

////////////////////
// PartsDistanceKernel //
////////////////////
PartsDistanceKernel::PartsDistanceKernel()
{
    partsize = 0.9;
    n = 2;
    standardize = true;
    min_stddev = 1e-8;
    epsilon = 1e-8;
    pow_distance = false;
}

////////////////////
// declareOptions //
////////////////////
void PartsDistanceKernel::declareOptions(OptionList& ol)
{
    declareOption(ol, "n", &PartsDistanceKernel::n, OptionBase::buildoption, 
                  "This class implements a Ln parts distance (L2 is the default).");

    declareOption(ol, "pow_distance", &PartsDistanceKernel::pow_distance, OptionBase::buildoption, 
                  "If set to 1 (true), the distance computed will be elevated to power n.");

    declareOption(ol, "standardize", &PartsDistanceKernel::standardize, OptionBase::buildoption, 
                  "If set to 1 (true), the inverse standard deviation inv_stddev will be used to scale the vector differences.");

    declareOption(ol, "min_stddev", &PartsDistanceKernel::min_stddev, OptionBase::buildoption, 
                  "When method train computes inv_stddev, it will set it to FLT_MAX for \n"
                  "any component for which the standard deviation is below min_stddev");

    declareOption(ol, "epsilon", &PartsDistanceKernel::epsilon, OptionBase::buildoption, 
                  "This is added to the absolute value of the elementwise difference between the 2 vectors.\n" 
                  "It's especially important for this to be non-zero when standardizing.");

    declareOption(ol, "partsize", &PartsDistanceKernel::partsize, OptionBase::buildoption, 
                  "This determines the number of elements (the size of the part) that will be used to compute the distance.\n"
                  "If >=1 it's interpreted as anabsolute number of elements. If it's <1 it's taken to mean a fraction of all the elements.\n");

    declareOption(ol, "inv_stddev", &PartsDistanceKernel::inv_stddev, OptionBase::learntoption, 
                  "This is computed when calling method train, and will be used when evaluating distances only if standardize is true.\n"
                  "It crresponds to the inverse of the standard deviation of inputs in the dataset passed to train. But see also min_stddev.\n");

    inherited::declareOptions(ol);
}

void PartsDistanceKernel::train(VMat data)
{    
    if(standardize)
    {
        Vec meanvec;
        Vec stddev;    
        computeInputMeanAndStddev(data, meanvec, stddev, 0.0);
        int l = stddev.length();
        inv_stddev.resize(l);
        for(int i=0; i<l; i++)
        {
            if(stddev[i]<min_stddev)
                inv_stddev[i] = FLT_MAX;
            else
                inv_stddev[i] = 1/stddev[i]; 
        }
    }
}


//////////////
// evaluate //
//////////////
real PartsDistanceKernel::evaluate(const Vec& x1, const Vec& x2) const {
    int l = x1.length();
    if(x2.length() != l)
        PLERROR("vectors x1 and x2 must have the same size");
    
    if(standardize && l!=inv_stddev.length())
        PLERROR("In PartsDistanceKernel::evaluate, size of vectors (%d) does not match size of inv_stddev (%d). Make sure you called train on the kernel with appropriate dataset",l,inv_stddev.length());
        
    elementdist.resize(l);
    for(int i=0; i<l; i++)
    {
        real d = FLT_MAX;
        if( !(standardize && inv_stddev[i]>=FLT_MAX) )
        {
            d = abs(x1[i]-x2[i])+epsilon;
            if(standardize)
                d *= inv_stddev[i];
            
            if(fast_exact_is_equal(n,2))
                d *= d;
            else if(!fast_exact_is_equal(n,1))
                d = mypow(d,n);
        }
        elementdist[i] = d;     
    }
    
    sortElements(elementdist);
    int ps = (partsize>=1 ? (int)partsize :(int)(partsize*l+0.5));
    if(ps>l)
        ps = l;
    real res = 0;
    for(int i=0; i<ps; i++)
    {
        real d = elementdist[i];
        if(d<FLT_MAX)
            res += d;
    }

    if(!pow_distance)
        res = mypow(res, 1/n);

    return res;
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
