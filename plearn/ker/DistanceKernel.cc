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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "DistanceKernel.h"
#include "SelectedOutputCostFunction.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    DistanceKernel,
    "Implements an Ln distance (defaults to L2 i.e. euclidean distance).",
    "Output is as follows:\n"
    "- If option 'pow_distance' = 1,\n"
    "     k(x1,x2) = \\sum_i |x1[i]-x2[i]|^n\n"
    "- If option 'pow_distance' = 0,\n"
    "     k(x1,x2) = (\\sum_i |x1[i]-x2[i]|^2)^(1/n)");

////////////////////
// DistanceKernel //
////////////////////
DistanceKernel::DistanceKernel(real the_Ln, bool pd)
    : n(the_Ln),
      optimized(false),
      pow_distance(pd),
      ignore_missing(false)
{}

////////////////////
// declareOptions //
////////////////////
void DistanceKernel::declareOptions(OptionList& ol)
{

    declareOption(ol, "n", &DistanceKernel::n, OptionBase::buildoption, 
                  "This class implements a Ln distance (L2, the default is the usual euclidean distance).");

    declareOption(ol, "pow_distance", &DistanceKernel::pow_distance, OptionBase::buildoption, 
                  "If set to 1, the distance computed will be elevated to power n.");

    declareOption(ol, "optimized", &DistanceKernel::optimized, OptionBase::buildoption, 
                  "If set to 1, the evaluate_i_j method will be faster, at the cost of potential\n"
                  "approximations in the result.");

    declareOption(ol, "ignore_missing", &DistanceKernel::ignore_missing, OptionBase::buildoption, 
                  "If set to false, nan will be propagated.\n"
                  "If set to true, if a value is missing in the matrix of some examples, we will ignore this value for the distance\n"
                  "If set to true, work only if pow_distance is set to 1.");

    inherited::declareOptions(ol);
}

//////////////
// evaluate //
//////////////
real DistanceKernel::evaluate(const Vec& x1, const Vec& x2) const {
    if (ignore_missing && !pow_distance)
        PLERROR("In DistanceKernel::evaluate(int i, int j) - 'ignore_missing' "
                "implemented only if pow_distance is set");

    if (pow_distance) {
        return powdistance(x1, x2, n, ignore_missing);
    } else {
        return dist(x1, x2, n);
    }
}

//////////////////
// evaluate_i_j //
//////////////////
real DistanceKernel::evaluate_i_j(int i, int j) const {
    static real d;
    if (ignore_missing)
        PLERROR("DistanceKernel::evaluate_i_j(int i, int j) not implemented for ignore_missing");

    if (optimized && fast_exact_is_equal(n, 2.0)) {
        if (i == j)
            // The case 'i == j' can cause precision issues because of the optimized
            // formula below. Thus we make sure we always return 0.
            return 0;
        d = squarednorms[i] + squarednorms[j] - 2 * data->dot(i, j, data_inputsize);
        if (d < 0) {
            // This can happen (especially when compiled in -opt) if the two points
            // are the same, and the distance should be zero.
            if (d < -1e-2)
                // That should not happen.
                PLERROR("In DistanceKernel::evaluate_i_j - Found a (significantly) negative distance (%f), "
                        "i = %d, j = %d, squarednorms[i] = %f, squarednorms[j] = %f, dot = %f",
                        d, i, j, squarednorms[i], squarednorms[j], data->dot(i, j, data_inputsize));
            d = 0;
        }
        if (pow_distance)
            return d;
        else
            return sqrt(d);
    } else {
        return inherited::evaluate_i_j(i,j);
    }
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void DistanceKernel::setDataForKernelMatrix(VMat the_data)
{
    if (ignore_missing)
        PLWARNING("DistanceKernel::setDataForKernelMatrix(VMat the_data) not tested for ignore_missing");

    inherited::setDataForKernelMatrix(the_data);
    if (fast_exact_is_equal(n, 2.0)) {
        squarednorms.resize(data.length());
        for(int index=0; index<data.length(); index++) {
            squarednorms[index] = data->dot(index, index, data_inputsize);
        }
    }
}

////////////////////////
// absolute_deviation //
////////////////////////
CostFunc absolute_deviation(int singleoutputindex)
{ 
    if(singleoutputindex>=0)
        return new SelectedOutputCostFunction(new DistanceKernel(1.0),singleoutputindex); 
    else
        return new DistanceKernel(1.0); 
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
