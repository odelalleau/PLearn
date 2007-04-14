// -*- C++ -*-

// KroneckerBaseKernel.cc
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

/*! \file KroneckerBaseKernel.cc */

#include <plearn/base/lexical_cast.h>
#include <plearn/math/TMat_maths.h>
#include "KroneckerBaseKernel.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    KroneckerBaseKernel,
    "Base class for kernels that make use of Kronecker terms",
    "This kernel allows the specification of product a of Kronecker delta terms\n"
    "when there is a match of VALUE in ONE DIMENSION.  (This may be generalized\n"
    "in the future to allow match according to a subset of the input variables,\n"
    "but is not currently done for performance reasons).  With these terms, the\n"
    "kernel function takes the form:\n"
    "\n"
    "  k(x,y) = \\product_i delta_x[kr(i)],y[kr(i)]\n"
    "\n"
    "where kr(i) is the i-th element of 'kronecker_indexes' (representing an\n"
    "index into the input vectors)  Derived classes can either integrate these\n"
    "terms additively (e.g. KroneckerBaseKernel) or multiplicatively\n"
    "(e.g. ARDBaseKernel and derived classes).  Note that this class does not\n"
    "provide any hyperparameter associated with this product; an hyperparameter\n"
    "may be included by derived classes as required.  (Currently, only\n"
    "IIDNoiseKernel needs one; in other kernels, this is absorbed by the global\n"
    "function noise hyperparameter)\n"
    "\n"
    "The basic idea for Kronecker terms is to selectively build in parts of a\n"
    "covariance function based on matches in the value of some input variables.\n"
    "They are useful in conjunction with a \"covariance function builder\" such as\n"
    "SummationKernel.\n"
    );


KroneckerBaseKernel::KroneckerBaseKernel()
    : m_default_value(0.)
{ }


//#####  declareOptions  ######################################################

void KroneckerBaseKernel::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "kronecker_indexes", &KroneckerBaseKernel::m_kronecker_indexes,
        OptionBase::buildoption,
        "Element index in the input vectors that should be subject to additional\n"
        "Kronecker delta terms");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


//#####  build  ###############################################################

void KroneckerBaseKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}


//#####  build_  ##############################################################

void KroneckerBaseKernel::build_()
{ }


//#####  evaluate  ############################################################

real KroneckerBaseKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    const int n = m_kronecker_indexes.size();
    if (n > 0) {
        int*  cur_index = m_kronecker_indexes.data();
        for (int i=0 ; i<n ; ++i, ++cur_index)
            if (! fast_is_equal(x1[*cur_index], x2[*cur_index]))
                return 0.0;
        return 1.0;
    }
    return m_default_value;
}


//#####  computeGramMatrix  ###################################################

void KroneckerBaseKernel::computeGramMatrix(Mat K) const
{
    if (!data)
        PLERROR("Kernel::computeGramMatrix: setDataForKernelMatrix not yet called");
    if (!is_symmetric)
        PLERROR("Kernel::computeGramMatrix: not supported for non-symmetric kernels");
    if (K.length() != data.length() || K.width() != data.length())
        PLERROR("Kernel::computeGramMatrix: the argument matrix K should be\n"
                "of size %d x %d (currently of size %d x %d)",
                data.length(), data.length(), K.length(), K.width());
                
    PLASSERT( K.size() == 0 || m_data_cache.size() > 0 );  // Ensure data cached OK

    // Prepare kronecker iteration
    int   kronecker_num     = m_kronecker_indexes.size();
    int*  kronecker_indexes = ( kronecker_num > 0?
                                m_kronecker_indexes.data() : 0 );

    // Compute Gram Matrix
    int  l = data->length();
    int  m = K.mod();
    int  cache_mod = m_data_cache.mod();

    real *data_start = &m_data_cache(0,0);
    real Kij = m_default_value;
    real *Ki, *Kji;
    real *xi = data_start;
    
    for (int i=0 ; i<l ; ++i, xi += cache_mod) {
        Ki  = K[i];
        Kji = &K[0][i];
        real *xj = data_start;

        for (int j=0; j<=i; ++j, Kji += m, xj += cache_mod) {
            if (kronecker_num > 0) {
                real  product = 1.0;
                int*  cur_index = kronecker_indexes;

                // Go over Kronecker terms, skipping over an eventual omitted term
                for (int k=0 ; k<kronecker_num ; ++k, ++cur_index)
                    if (! fast_is_equal(xi[*cur_index], xj[*cur_index])) {
                        product = 0.0;
                        break;
                    }

                Kij = product;
            }
            *Ki++ = Kij;
        }
    }
}


//#####  softplusFloor  #######################################################

real KroneckerBaseKernel::softplusFloor(real& value, real floor)
{
    real sp = softplus(value);
    if (sp < floor) {
        value = pl_log(exp(floor)-1);           // inverse soft-plus
        return floor;
    }
    return sp;
}


//#####  makeDeepCopyFromShallowCopy  #########################################

void KroneckerBaseKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(m_kronecker_indexes,   copies);
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
