// -*- C++ -*-

// CubicSpline.cc
//
// Copyright (C) 2007 Christian Dorion
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

// Authors: Christian Dorion

/*! \file CubicSpline.cc */


#include "CubicSpline.h"
//#include "algo.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    CubicSpline,
    "Unidimensional cubic spline learner.",
    "This learner fits a unidimensional cubic spline to a given set of\n"
    "points. That is, inputsize() must be one and the inputs are considered to be\n"
    "the x values, while targetsize() must also be one and the targets are\n"
    "considered to be the y values. The spline is fitted to the (x,y)-pairs so\n"
    "formed. X values don't need to be ordered; the ordering is ensured within\n"
    "the train method.\n");

CubicSpline::CubicSpline()
    : m_low_slope(MISSING_VALUE),
      m_high_slope(MISSING_VALUE)
{
    // Nothing to do here
} 

void CubicSpline::declareOptions(OptionList& ol)
{
    declareOption(ol, "low_slope", &CubicSpline::m_low_slope,
                  OptionBase::buildoption,
                  "The slope to enforce at the leftmost node -- Default: NaN [None]");
    
    declareOption(ol, "high_slope", &CubicSpline::m_high_slope,
                  OptionBase::buildoption,
                  "The slope to enforce at the rightmost node -- Default: NaN [None]");
    
    // Learnt options
    declareOption(ol, "inputs", &CubicSpline::m_inputs,
                  OptionBase::learntoption,
                  "A buffer containing the last training set inputs");

    declareOption(ol, "targets", &CubicSpline::m_targets,
                  OptionBase::learntoption,
                  "A buffer containing the last training set targets");

    declareOption(ol, "coefficients", &CubicSpline::m_coefficients,
                  OptionBase::learntoption,
                  "The learnt coefficients");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void CubicSpline::build_()
{
    // Nothing to do here    
}

// ### Nothing to add here, simply calls build_
void CubicSpline::build()
{
    inherited::build();
    build_();
}


void CubicSpline::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(m_inputs, copies);
    deepCopyField(m_targets, copies);
    deepCopyField(m_coefficients, copies);
}


int CubicSpline::outputsize() const
{
    return inputsize();
}

void CubicSpline::forget()
{
    m_coefficients->resize(0);
    inherited::forget();
}

void CubicSpline::train()
{
    // This learner fits unidimensional inputs/targets
    PLASSERT( inputsize() == 1 );
    PLASSERT( targetsize() == 1 );
    
    // Train set is a member of PLearner; set through setTrainingSet()
    int n = train_set->length();
    m_inputs = train_set.getColumn(0);
    m_targets = train_set.getColumn(1);
    PLASSERT( n >= 2 && train_set->width() == 2 );

    // Sort the inputs and targets along the inputs values
    TVec<int> indices = m_inputs.sortingPermutation();
    m_inputs          = m_inputs(indices);
    m_targets         = m_targets(indices);
    
    Vec u(n-1, 0.0);
    m_coefficients.resize(n);
    m_coefficients.fill(0.0);

    // Low slope hack
    if ( !is_missing(m_low_slope) ) {
        u[0] = (3.0/(m_inputs[1]-m_inputs[0])) *
            ( (m_targets[1]-m_targets[0])/
              (m_inputs[1] - m_inputs[0]) - m_low_slope );
        m_coefficients[0] = -0.5;
    }

    // Forward pass on coefficients
    for (int i=1; i < (n-1); i++) {
        real sig = (m_inputs[i]-m_inputs[i-1]) / (m_inputs[i+1]-m_inputs[i-1]);        
        real p   = sig*m_coefficients[i-1]+2.0;
        
        m_coefficients[i] = (sig-1.0)/p;
        
        u[i] = (m_targets[i+1]-m_targets[i]) / (m_inputs[i+1]-m_inputs[i])
            - (m_targets[i]-m_targets[i-1])  / (m_inputs[i]-m_inputs[i-1]);
        
        u[i] = (6.0*u[i]/(m_inputs[i+1]-m_inputs[i-1]) - sig*u[i-1]) / p;
    }

    // High slope hack
    real un=0, qn=0;
    if ( !is_missing(m_high_slope) ) {
        qn = 0.5;
        un = (3.0/(m_inputs[n-1]-m_inputs[n-2])) *
            (m_high_slope  -  (m_targets[n-1]-m_targets[n-2]) / (m_inputs[n-1]-m_inputs[n-2]));
    }

    // Compute the last coefficient
    m_coefficients[n-1] = (un-qn*u[n-2])/(qn*m_coefficients[n-1]+1.0);

    // Backsubstitution step
    for (int k=n-2; k >= 0; k--)
        m_coefficients[k] = m_coefficients[k]*m_coefficients[k+1]+u[k];
}


void CubicSpline::computeOutput(const Vec& input, Vec& output) const
{
    PLASSERT( input.length() == 1 );
    output.resize(1);
 
    int n = m_inputs.length();
    real x = input[0];
    int pos = min( max(1, m_inputs.findSorted(x)),  n-1 );

    real h = m_inputs[pos] - m_inputs[pos-1];
    PLASSERT( h > 0.0 );

    real a = (m_inputs[pos] - x) / h;
    real b = (x - m_inputs[pos-1]) / h;
    output[0] = a*m_targets[pos-1] + b*m_targets[pos]
        + ((a*a*a - a)*m_coefficients[pos-1]
           + (b*b*b - b)*m_coefficients[pos]) * h*h/6.0;
}

void CubicSpline::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    // No costs...
}

TVec<string> CubicSpline::getTestCostNames() const
{
    // None for now
    return TVec<string>();
}

TVec<string> CubicSpline::getTrainCostNames() const
{
    // None for now
    return TVec<string>();
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
