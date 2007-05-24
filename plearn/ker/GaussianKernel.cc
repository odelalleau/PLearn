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

#include "GaussianKernel.h"
#include <plearn/math/TMat_maths.h>

//#define GK_DEBUG

namespace PLearn {
using namespace std;

// ** GaussianKernel **

PLEARN_IMPLEMENT_OBJECT(GaussianKernel,
                        "The good old Gaussian kernel: K(x,y) = exp(-||x-y||^2 / sigma^2).",
                        "Note that this is not the proper normal density (but has the same 'shape')\n"
                        "In particular it's not properly normalized. If you want the usual, properly\n"
                        "normalized Gaussian density, consider using GaussianDensityKernel instead");

////////////////////
// GaussianKernel //
////////////////////
GaussianKernel::GaussianKernel()
    : scale_by_sigma(false),
      sigma(1)
{
    build_();
}

GaussianKernel::GaussianKernel(real the_sigma)
    : scale_by_sigma(false),
      sigma(the_sigma)
{
    build_();
}

////////////////////
// declareOptions //
////////////////////
void GaussianKernel::declareOptions(OptionList& ol)
{
    declareOption(ol, "sigma", &GaussianKernel::sigma, OptionBase::buildoption,
                  "The width of the Gaussian.");

    declareOption(ol, "scale_by_sigma", &GaussianKernel::scale_by_sigma, OptionBase::buildoption,
                  "If set to 1, the kernel will be scaled by sigma^2 / 2");

    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void GaussianKernel::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void GaussianKernel::build_()
{
    minus_one_over_sigmasquare = -1.0/square(sigma);
    sigmasquare_over_two = square(sigma) / 2.0;
}


void GaussianKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(squarednorms,copies);
}


void GaussianKernel::addDataForKernelMatrix(const Vec& newRow)
{
    inherited::addDataForKernelMatrix(newRow);

    int dlen  = data.length();
    int sqlen = squarednorms.length();
    if(sqlen == dlen-1)
        squarednorms.resize(dlen);
    else if(sqlen == dlen)
        for(int s=1; s < sqlen; s++)
            squarednorms[s-1] = squarednorms[s];  
    else
        PLERROR("Only two scenarios are managed:\n"
                "Either the data matrix was only appended the new row or, under the windowed settings,\n"
                "newRow is the new last row and other rows were moved backward.\n"
                "However, sqlen = %d and dlen = %d excludes both!", sqlen, dlen);
  
    squarednorms.lastElement() = pownorm(newRow, 2); 
}

/////////////////////////////////////////
// evaluateFromSquaredNormOfDifference //
/////////////////////////////////////////
real GaussianKernel::evaluateFromSquaredNormOfDifference(real sqnorm_of_diff) const
{
    if (sqnorm_of_diff < 0) {
        if (sqnorm_of_diff * minus_one_over_sigmasquare < 1e-10 )
            // This can still happen when computing K(x,x), because of numerical
            // approximations.
            sqnorm_of_diff = 0;
        else {
            // This should not happen (anymore) with the isUnsafe check.
            // You may comment out the PLERROR below if you want to continue your
            // computations, but then you should investigate why this happens.
            PLERROR("In GaussianKernel::evaluateFromSquaredNormOfDifference - The given "
                    "'sqnorm_of_diff' is negative (%f)", sqnorm_of_diff);
            sqnorm_of_diff = 0;
        }
    }
    if (scale_by_sigma) {
        return exp(sqnorm_of_diff*minus_one_over_sigmasquare) * sigmasquare_over_two;
    } else {
        return exp(sqnorm_of_diff*minus_one_over_sigmasquare);
    }
}

real GaussianKernel::evaluateFromDotAndSquaredNorm(real sqnorm_x1, real dot_x1_x2, real sqnorm_x2) const
{
    return evaluateFromSquaredNormOfDifference((sqnorm_x1+sqnorm_x2)-(dot_x1_x2+dot_x1_x2));
}



//////////////
// evaluate //
//////////////
real GaussianKernel::evaluate(const Vec& x1, const Vec& x2) const
{
#ifdef BOUNDCHECK
    if(x1.length()!=x2.length())
        PLERROR("IN GaussianKernel::evaluate x1 and x2 must have the same length");
#endif
    int l = x1.length();
    real* px1 = x1.data();
    real* px2 = x2.data();
    real sqnorm_of_diff = 0.;
    for(int i=0; i<l; i++)
    {
        real val = px1[i]-px2[i];
        sqnorm_of_diff += val*val;
    }
    return evaluateFromSquaredNormOfDifference(sqnorm_of_diff);
}


//////////////////
// evaluate_i_j //
//////////////////
real GaussianKernel::evaluate_i_j(int i, int j) const
{ 
#ifdef GK_DEBUG 
    if(i==0 && j==1){
        cout << "*** i==0 && j==1 ***" << endl;
        cout << "data(" << i << "): " << data(i) << endl << endl;
        cout << "data(" << j << "): " << data(j) << endl << endl;  
    
        real sqnorm_i = pownorm((Vec)data(i), 2);
        if(sqnorm_i != squarednorms[i])
            PLERROR("%f = sqnorm_i != squarednorms[%d] = %f", sqnorm_i, i, squarednorms[i]);
    
        real sqnorm_j = pownorm((Vec)data(j), 2);
        if(sqnorm_j != squarednorms[j])
            PLERROR("%f = sqnorm_j != squarednorms[%d] = %f", sqnorm_j, j, squarednorms[j]);
    }
#endif
    real sqn_i = squarednorms[i];
    real sqn_j = squarednorms[j];
    if (isUnsafe(sqn_i, sqn_j))
        return inherited::evaluate_i_j(i,j);
    else
        return evaluateFromDotAndSquaredNorm(sqn_i, data->dot(i,j,data_inputsize), sqn_j); 
}

//////////////////
// evaluate_i_x //
//////////////////
real GaussianKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const 
{ 
    if(squared_norm_of_x<0.)
        squared_norm_of_x = pownorm(x);

#ifdef GK_DEBUG 
//   real dot_x1_x2 = data->dot(i,x);
//   cout << "data.row(" << i << "): " << data.row(i) << endl 
//        << "squarednorms[" << i << "]: " << squarednorms[i] << endl
//        << "data->dot(i,x): " << dot_x1_x2 << endl
//        << "x: " << x << endl
//        << "squared_norm_of_x: " << squared_norm_of_x << endl;
//   real sqnorm_of_diff = (squarednorms[i]+squared_norm_of_x)-(dot_x1_x2+dot_x1_x2);
//   cout << "a-> sqnorm_of_diff: " << sqnorm_of_diff << endl
//        << "b-> minus_one_over_sigmasquare: " << minus_one_over_sigmasquare << endl
//        << "a*b: " << sqnorm_of_diff*minus_one_over_sigmasquare << endl
//        << "res: " << exp(sqnorm_of_diff*minus_one_over_sigmasquare) << endl; 
#endif
    real sqn_i = squarednorms[i];
    if (isUnsafe(sqn_i, squared_norm_of_x))
        return inherited::evaluate_i_x(i, x, squared_norm_of_x);
    else
        return evaluateFromDotAndSquaredNorm(sqn_i, data->dot(i,x), squared_norm_of_x); 
}


//////////////////
// evaluate_x_i //
//////////////////
real GaussianKernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const
{ 
    if(squared_norm_of_x<0.)
        squared_norm_of_x = pownorm(x);
    real sqn_i = squarednorms[i];
    if (isUnsafe(sqn_i, squared_norm_of_x))
        return inherited::evaluate_x_i(x, i, squared_norm_of_x);
    else
        return evaluateFromDotAndSquaredNorm(squared_norm_of_x, data->dot(i,x), sqn_i); 
}

//////////////
// isUnsafe //
//////////////
bool GaussianKernel::isUnsafe(real sqn_1, real sqn_2) const {
    return (sqn_1 > 1e6 && fabs(sqn_2 / sqn_1 - 1.0) < 1e-2);
}
 
////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void GaussianKernel::setDataForKernelMatrix(VMat the_data)
{ 
    inherited::setDataForKernelMatrix(the_data);
    build_();                                // Update sigma computation cache
    squarednorms.resize(data.length());
    for(int index=0; index<data.length(); index++)
        squarednorms[index] = data->dot(index,index, data_inputsize);
}

///////////////////
// setParameters //
///////////////////
void GaussianKernel::setParameters(Vec paramvec)
{ 
    PLWARNING("In GaussianKernel: setParameters is deprecated, use setOption instead");
    sigma = paramvec[0]; 
    build_();                                // Update sigma computation cache
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
