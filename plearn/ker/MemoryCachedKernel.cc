// -*- C++ -*-

// MemoryCachedKernel.cc
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

/*! \file MemoryCachedKernel.cc */


#include "MemoryCachedKernel.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    MemoryCachedKernel,
    "Provide some memory-management utilities for kernels.",
    "This class is intended as a base class to provide some memory management\n"
    "utilities for the data-matrix set with setDataForKernelMatrix function.  In\n"
    "particular, it provides a single (inline, non-virtual) function to access a\n"
    "given input vector of the data matrix.  If the data VMatrix passed to\n"
    "setDataForKernelMatrix is within a certain size threshold, the VMatrix is\n"
    "converted to a Mat and cached to memory (without requiring additional space\n"
    "if the VMatrix is actually a MemoryVMatrix), and all further element access\n"
    "are done without requiring virtual function calls.\n"
    "\n"
    "IMPORTANT NOTE: the 'cache_gram_matrix' option is enabled automatically by\n"
    "default for this class.  This makes the computation of the Gram matrix\n"
    "derivatives (with respect to kernel hyperparameters) quite faster in many\n"
    "cases.  If you really don't want this caching to occur, just set it\n"
    "explicitly to false.\n"
    "\n"
    "This class also provides utility functions to derived classes to compute\n"
    "the Gram matrix and its derivative (with respect to kernel hyperparameters)\n"
    "without requiring virtual function calls in data access or evaluation\n"
    "function.\n"
    );


//#####  MemoryCachedKernel::MemoryCachedKernel  ##############################

MemoryCachedKernel::MemoryCachedKernel()
    : m_cache_threshold(1000000)
{
    cache_gram_matrix = true;
}


//#####  declareOptions  ######################################################

void MemoryCachedKernel::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "cache_threshold", &MemoryCachedKernel::m_cache_threshold,
        OptionBase::buildoption,
        "Threshold on the number of elements to cache the data VMatrix into a\n"
        "real matrix.  Above this threshold, the VMatrix is left as-is, and\n"
        "element access remains virtual.  (Default value = 1000000)\n");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


//#####  build  ###############################################################

void MemoryCachedKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}


//#####  build_  ##############################################################

void MemoryCachedKernel::build_()
{ }


//#####  makeDeepCopyFromShallowCopy  #########################################

void MemoryCachedKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(m_data_cache, copies);
}


//#####  setDataForKernelMatrix  ##############################################

void MemoryCachedKernel::setDataForKernelMatrix(VMat the_data)
{
    inherited::setDataForKernelMatrix(the_data);

    if (the_data.width() * the_data.length() <= m_cache_threshold &&
        the_data.isNotNull())
    {
        m_data_cache = the_data.toMat();

        // Update row cache
        const int N = m_data_cache.length();
        m_row_cache.resize(N);
        for (int i=0 ; i<N ; ++i)
            dataRow(i, m_row_cache[i]);
    }
    else {
        m_data_cache = Mat();
        m_row_cache.resize(0);
    }
}


//#####  addDataForKernelMatrix  ##############################################

void MemoryCachedKernel::addDataForKernelMatrix(const Vec& newrow)
{
    inherited::addDataForKernelMatrix(newrow);

    if (m_data_cache.isNotNull()) {
        const int OLD_N = m_data_cache.length();
        PLASSERT( m_data_cache.length() == m_row_cache.size() );
        m_data_cache.appendRow(newrow);

        // Update row cache
        m_row_cache.push_back(Vec());
        dataRow(OLD_N, m_row_cache[OLD_N]);
    }
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
