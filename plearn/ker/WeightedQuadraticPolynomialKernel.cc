// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
 * $Id: WeightedQuadraticPolynomialKernel.cc 6802 2007-03-29 15:19:55Z tihocan $
 * This file is part of the PLearn library.
 ******************************************************* */

#include "WeightedQuadraticPolynomialKernel.h"

namespace PLearn {
using namespace std;



PLEARN_IMPLEMENT_OBJECT(
    WeightedQuadraticPolynomialKernel,
    "Polynomial kernel of degree two, with coefficient correction.",
    "Computes K(x,y) = 0.5 * ( alpha (1 + <x,y>)^2 - alpha \n"
    "                          + 2*(1 - alpha) * \\sum_i x_i y_i\n"
    "                          + (2*beta - alpha) * \\sum_i x_i^2 y_i^2 )\n"
    "This implies the following features:\n"
    "  - the first degree features x_i\n"
    "  - the correlation features x_i x_j (i \neq j) weighted by alpha\n"
    "  - the second degree features x_i^2 weighted by beta\n"
);

//////////////////////
// WeightedQuadraticPolynomialKernel //
//////////////////////

WeightedQuadraticPolynomialKernel::WeightedQuadraticPolynomialKernel(
    real the_alpha, real the_beta,
    bool call_build_):
    inherited(true, call_build_),
    alpha(the_alpha),
    beta(the_beta)
{
    if (call_build_)
        build_();
}

////////////////////
// declareOptions //
////////////////////
void WeightedQuadraticPolynomialKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "alpha", &WeightedQuadraticPolynomialKernel::alpha, 
                  OptionBase::buildoption,
                  "Weight on correlation features.");

    declareOption(ol, "beta", &WeightedQuadraticPolynomialKernel::beta, 
                  OptionBase::buildoption,
                  "Weight on second degree features.");

    // Declare options inherited from parent class.
    inherited::declareOptions(ol);
}


///////////
// build //
///////////
void WeightedQuadraticPolynomialKernel::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void WeightedQuadraticPolynomialKernel::build_()
{}

//////////////
// evaluate //
//////////////
real WeightedQuadraticPolynomialKernel::evaluate(const Vec& x1, const Vec& x2) const
{ 
#ifdef BOUNDCHECK
    if(x1.length()!=x2.length())
        PLERROR("In WeightedQuadraticPolynomialKernel::evaluate(): "
            "x1 and x2 have different lengths.");
#endif
    real res = 0;
    real corr = 0;
    if (x1.size() > 0 && x2.size() > 0) {
        real* v1 = x1.data();
        real* v2 = x2.data();
        real v1i = 0;
        real v2i = 0;
        for(int i=0; i<x1.length(); i++)
        {
            v1i = v1[i];
            v2i = v2[i];
            res += v1i*v2i;
            corr += v1i*v2i*v1i*v2i;
        }
    }
    
    //Computes K(x,y) = 0.5 * ( alpha (1 + <x,y>)^2 - alpha \n"
    //                          + 2*(1 - alpha) * \sum_i x_i y_i\n"
    //                          + (2*beta - alpha) * \sum_i x_i^2 y_i^2 )\n"
    return 0.5 * (alpha * ipow(res + real(1.0), 2) - alpha + 2*(1 - alpha)*res + 
                  (2*beta - alpha)*corr);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void WeightedQuadraticPolynomialKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies) {
    inherited::makeDeepCopyFromShallowCopy(copies);
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
