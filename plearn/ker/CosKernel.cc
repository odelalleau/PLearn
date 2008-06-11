// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2007 Jerome Louradour

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
 * $Id: CosKernel.cc 7675 2007-06-29 19:50:49Z tihocan $
 * This file is part of the PLearn library.
 ******************************************************* */

#include "CosKernel.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    CosKernel,
    "Implements a distance based on a cosinus.",
    "Output k(x,y)=(0.5 - 0.5*cos(x,y)) (between 0 and 1)\n");

////////////////////
// CosKernel //
////////////////////
CosKernel::CosKernel()
{
    n=2.0;
    optimized=false;
    pow_distance=false;
    ignore_missing=false;
}

////////////////////
// declareOptions //
////////////////////
void CosKernel::declareOptions(OptionList& ol)
{
    inherited::declareOptions(ol);

    redeclareOption(ol, "n", &CosKernel::n, OptionBase::nosave, 
                  "Obsolete option for cosinus kernel.");

    redeclareOption(ol, "pow_distance", &CosKernel::pow_distance, OptionBase::nosave, 
                  "Obsolete option for cosinus kernel.");

    redeclareOption(ol, "optimized", &CosKernel::optimized, OptionBase::nosave, 
                  "Obsolete option for cosinus kernel.");

    redeclareOption(ol, "ignore_missing", &CosKernel::ignore_missing, OptionBase::nosave, 
                  "Obsolete option for cosinus kernel.");
}

//////////////
// evaluate //
//////////////
real CosKernel::evaluate(const Vec& x1, const Vec& x2) const {
    if (ignore_missing && !pow_distance)
        PLERROR("In CosKernel::evaluate(int i, int j) - 'ignore_missing' "
                "implemented only if pow_distance is set");
    static real cosinus;
    cosinus = dot(x1, x2) / ( norm(x1) * norm(x2) );
    return (1.-cosinus)*.5;
}

//////////////////
// evaluate_i_j //
//////////////////
real CosKernel::evaluate_i_j(int i, int j) const {
    static real cosinus;
    if (i == j)
        // The case 'i == j' can cause precision issues because of the optimized
        // formula below. Thus we make sure we always return 0.
        return 0;
    cosinus = data->dot(i, j, data_inputsize) / sqrt(squarednorms[i] * squarednorms[j]);
    return (1.-cosinus)*.5;
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
