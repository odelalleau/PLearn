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
 * $Id: WeightedDistance.cc 4253 2005-10-18 19:02:25Z tihocan $
 * This file is part of the PLearn library.
 ******************************************************* */

#include "WeightedDistance.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    WeightedDistance,
    "Implements an weighted distance.",
    "Output is as follows:\n"
    "- k(x1,x2) = \\sum_i w[i]*(x1[i]-x2[i])^2\n"
);

////////////////////
// WeightedDistance //
////////////////////
WeightedDistance::WeightedDistance()
{
}

WeightedDistance::WeightedDistance(Vec the_weights)
 : weights(the_weights)
{
}

////////////////////
// declareOptions //
////////////////////
void WeightedDistance::declareOptions(OptionList& ol)
{

    declareOption(ol, "weights", &WeightedDistance::weights, OptionBase::buildoption, 
                  "The vector of weights to apply to the distance computation.");

    inherited::declareOptions(ol);
}

//////////////
// evaluate //
//////////////
real WeightedDistance::evaluate(const Vec& x1, const Vec& x2) const
{
    if (weights.length() != x1.length()) PLERROR("In WeightedDistance: inconsistent length between weigths and x1");
    if (weights.length() != x2.length()) PLERROR("In WeightedDistance: inconsistent length between weigths and x2");
    real return_value = 0.0;
    for (int i = 0; i < weights.length(); i++)
    {
        return_value += weights[i] * pow(x1[i] - x2[i], 2.0);
    }
    return return_value;
}

//////////////////
// evaluate_i_j //
//////////////////
real WeightedDistance::evaluate_i_j(int i, int j) const
{
    PLERROR("In WeightedDistance: evaluate_i_j not implemented");
    return 0.0;
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void WeightedDistance::setDataForKernelMatrix(VMat the_data)
{
    PLERROR("In WeightedDistance: setDataForKernelMatrix not implemented");
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
