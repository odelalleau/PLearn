// -*- C++ -*-

// BetaKernel.cc
//
// Copyright (C) 2008 Dumitru Erhan
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

// Authors: Dumitru Erhan

/*! \file BetaKernel.cc */


#include "BetaKernel.h"
#include <plearn/math/distr_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    BetaKernel,
    "Several implementatins of the Beta kernel, for distributions that have support in the [0;1] range",
    "Useful for performing Parzen Windows-style density estimation. Need to specify the type of the kernel\n"
     "- simple: the basic Beta kernel \n"
     "- alternative: the alternative, faster converging version \n"
     "output_type should be set to either log_density or density"  );

//////////////////
// BetaKernel //
//////////////////
BetaKernel::BetaKernel()
    : width(1.),
     kernel_type("simple"),
     output_type("log_density")
{
}

////////////////////
// declareOptions //
////////////////////
void BetaKernel::declareOptions(OptionList& ol)
{

    declareOption(ol, "width", &BetaKernel::width,
                   OptionBase::buildoption,
                  "The (positive, real-valued) smoothing parameter of the kernel (note that this does not quite correspond to the variance). If you use the Beta Kernel to do density estimation, this should go towards zero as the number of samples goes to infinity");
 
    declareOption(ol, "kernel_type", &BetaKernel::kernel_type,
                   OptionBase::buildoption,
                  "A string containing the type of Beta kernel. The \"simple\" kernel has a particularly simple mathematical form and is easily shown to integrate to 1 (thus is a density). The \"alternative\" kernel is slightly more complicated, but is better suited for those distributions that have a lot of mass near the boundaries (0 or 1). It will converge faster, but asymptotically both kernels are boundary bias free. Also, the \"alternative\" kernel is not yet shown by us to integrate to 1, thus we don't know for sure whether it's a valid density. Default is simple");
    
    declareOption(ol, "output_type", &BetaKernel::output_type,
                  OptionBase::buildoption,
                  "A string specifying whether we want log densities as outputs (\"log_density\"; default) or just densities (\"density\")");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void BetaKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void BetaKernel::build_()
{
}

//////////////
// evaluate //
//////////////
real BetaKernel::evaluate(const Vec& x1, const Vec& x2) const {
#ifdef BOUNDCHECK
    if(x1.length()!=x2.length())
        PLERROR("In BetaKernel::evaluate x1 and x2 must have the same length");
#endif
    int l = x1.length();
    real* px1 = x1.data();
    real* px2 = x2.data();
    real kvalue = 0.;

    //kernel_type = lowerstring(kernel_type);

    // check http://www.sta.nus.edu.sg/documents/publication_chen2.pdf for an
    // explanation of the estimators ("Beta kernel estimators for
    // density functions" is the paper title)
    if (kernel_type=="simple")
        for(int i=0; i<l; i++)
        {
            real a = px1[i] / width + 1.0;
            real b = (1.0 - px1[i]) / width + 1.0;
            real val = log_beta_density(px2[i],a,b);
            kvalue += val;
        } 
    else if (kernel_type=="alternative")
        for(int i=0; i<l; i++)
        {
            real x = px1[i];
            real a, b;

            if (x<0 || x >1 || px2[i] < 0 || px2[i] > 1)
                 PLERROR("In BetaKernel::evaluate x1 and x2 must contain values in the (closed) interval [0;1]");
            
            real p_xb = 2*pow(width,2.) + 2.5 - sqrt(4*pow(width,4.) + 6*pow(width,2.) + 2.25 - x*x - x / width);
            real y = 1-x;
            real p_1xb = 2*pow(width,2.) + 2.5 - sqrt(4*pow(width,4.)+ 6*pow(width,2.) + 2.25 - y*y - y / width);

            if ((x >= 2*width) && (x <= 1 - 2*width)) {
                a = x / width; 
                b = (1 - x) / width;
            }
            else if ((x >= 0) & (x <= 2*width)) {
                a = p_xb;
                b = (1 - x) / width;   
            }
            else {
                a = x / width;
                b = p_1xb;
            }
                
            real val = log_beta_density(px2[i],a,b);
            kvalue += val;
        } 
    else
        PLERROR("In BetaKernel::evaluate kernel_type must be either \"simple\" or \"alternative\"");
   
    real retval;
 
    if (output_type=="log_density")
        retval = kvalue;
    else if (output_type=="density")
        retval = exp(kvalue);
    else
        PLERROR("In BetaKernel::evaluate output_type must be either \"log_density\" or \"density\"");

    return retval;
}

/* ### This method will very often be overridden.
//////////////////
// evaluate_i_j //
//////////////////
real BetaKernel::evaluate_i_j(int i, int j) const {
// ### Evaluate the kernel on a pair of training points.
}
*/

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void BetaKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

/* ### This method will be overridden if computations need to be done,
   ### or to forward the call to another object.
   ### In this case, be careful that it may be called BEFORE the build_()
   ### method has been called, if the 'specify_dataset' option is used.
////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void BetaKernel::setDataForKernelMatrix(VMat the_data) {
}
*/

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
