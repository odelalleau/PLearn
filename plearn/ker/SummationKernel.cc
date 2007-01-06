// -*- C++ -*-

// SummationKernel.cc
//
// Copyright (C) 2007 Nicolas Chapados
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

// Authors: Nicolas Chapados

/*! \file SummationKernel.cc */


#include "SummationKernel.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    SummationKernel,
    "Kernel computing the sum of other kernels",
    "This kernel computes the summation of several subkernel objects.  It can\n"
    "also chop up parts of its input vector and send it to each kernel (so that\n"
    "each kernel can operate on a subset of the variables).\n"
    );


//#####  Constructor  #########################################################

SummationKernel::SummationKernel()
{ }


//#####  declareOptions  ######################################################

void SummationKernel::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "terms", &SummationKernel::m_terms, OptionBase::buildoption,
        "Individual kernels to add to produce the final result.  The\n"
        "hyperparameters of kernel i can be accesed under the option names\n"
        "'terms[i].hyperparam' for, e.g. GaussianProcessRegressor.\n");

    declareOption(
        ol, "input_indexes", &SummationKernel::m_input_indexes,
        OptionBase::buildoption,
        "Optionally, one can specify which of individual input variables should\n"
        "be routed to each kernel.  The format is as a vector of vectors: for\n"
        "each kernel in 'terms', one must list the INDEXES in the original input\n"
        "vector(zero-based) that should be passed to that kernel.  If a list of\n"
        "indexes is empty for a given kernel, it means that the COMPLETE input\n"
        "vector should be passed to the kernel.\n");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


//#####  build  ###############################################################

void SummationKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}


//#####  build_  ##############################################################

void SummationKernel::build_()
{
    // Preallocate buffers for kernel evaluation
    const int N = m_input_indexes.size();
    m_input_buf1.resize(N);
    m_input_buf2.resize(N);
    for (int i=0 ; i<N ; ++i) {
        const int M = m_input_indexes[i].size();
        m_input_buf1[i].resize(M);
        m_input_buf2[i].resize(M);
    }

    for (int i=0, n=m_terms.size() ; i<n ; ++i)
        if (! m_terms[i])
            PLERROR("SummationKernel::build_: kernel for term[%d] is not specified",i);

    if (m_input_indexes.size() > 0 && m_terms.size() != m_input_indexes.size())
        PLERROR("SummationKernel::build_: if 'input_indexes' is specified "
                "it must have the same size (%d) as 'terms'; found %d elements",
                m_terms.size(), m_input_indexes.size());
}


//#####  evaluate  ############################################################

real SummationKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    real kernel_value = 0.0;
    bool split_inputs = m_input_indexes.size() > 0;
    for (int i=0, n=m_terms.size() ; i<n ; ++i) {
        if (split_inputs && m_input_indexes[i].size() > 0) {
            selectElements(x1, m_input_indexes[i], m_input_buf1[i]);
            selectElements(x2, m_input_indexes[i], m_input_buf2[i]);
            kernel_value += m_terms[i]->evaluate(m_input_buf1[i],
                                                 m_input_buf2[i]);
        }
        else
            kernel_value += m_terms[i]->evaluate(x1,x2);
    }
    return kernel_value;
}


//#####  makeDeepCopyFromShallowCopy  #########################################

void SummationKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(m_terms,          copies);
    deepCopyField(m_input_indexes,  copies);
    deepCopyField(m_input_buf1,     copies);
    deepCopyField(m_input_buf2,     copies);
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
