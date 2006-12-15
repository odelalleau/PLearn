// -*- C++ -*-

// PLMathTest.cc
//
// Copyright (C) 2005 Olivier Delalleau 
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

/* *******************************************************      
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file PLMathTest.cc */


#include "PLMathTest.h"
#include <plearn/math/pl_math.h>
#include <plearn/math/PRandom.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PLMathTest,
    "Test various mathematical functions defined in pl_math.",
    ""
);

////////////////
// PLMathTest //
////////////////
PLMathTest::PLMathTest() 
    /* ### Initialize all fields to their default value */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

///////////
// build //
///////////
void PLMathTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PLMathTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("PLMathTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void PLMathTest::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "results", &PLMathTest::results, OptionBase::buildoption,
        "Map storing all test results.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void PLMathTest::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

/////////////
// perform //
/////////////
void PLMathTest::perform()
{
    int n = 20;
    TVec<int> bounds;
    string bounds_str = "[ 1 10 50 1000 ]";
    PStream read_bounds = openString(bounds_str, PStream::plearn_ascii);
    read_bounds >> bounds;
    read_bounds.flush();
    Vec samples;
    Vec vec(n);
    for (int i = 0; i < bounds.length(); i++) {
        PRandom::common(false)->fill_random_uniform(vec,-bounds[i],bounds[i]);
        samples.append(vec);
    }
    samples.append(0);
    samples.append(1);
    samples.append(-1);
    samples.append(MISSING_VALUE);
    samples.append(INFINITY);
    samples.append(-INFINITY);
    PP<ProgressBar> pb = new ProgressBar("Performing tests", samples.length());
    for (int i = 0; i < samples.length(); i++) {
        int j;
        real x = samples[i];
        results["data"].append(x);
        DOUBLE_TO_INT(x, j);
        results["DOUBLE_TO_INT"].append(j);
        results["sign"].append(sign(x));
        results["positive"].append(positive(x));
        results["negative"].append(negative(x));
        results["is_equal_1.00000001"].append(is_equal(x, x * 1.00000001));
        results["is_equal_1.001"].append(is_equal(x, x * 1.001));
        results["square"].append(square(x));
        results["hinge_loss_1"].append(hinge_loss(x, 1));
        results["d_hinge_loss_1"].append(d_hinge_loss(x, 1));
        results["is_missing"].append(is_missing(x));
        results["is_integer"].append(is_integer(x));
        results["FABS"].append(FABS(x));
        results["mypow"].append(mypow(x,x));
        if (int(x) >= 0) {
            results["ipow_real"].append(ipow(x,int(x)));
            results["ipow_int"].append(ipow(int(x),int(x)));
        } else {
            results["ipow_real"].append(0);
            results["ipow_int"].append(0);
        }
        results["sigmoid"].append(sigmoid(x));
        results["is_positive"].append(is_positive(x));
        if (x >= 0 && x <= 1)
            results["inverse_sigmoid"].append(inverse_sigmoid(x));
        else
            results["inverse_sigmoid"].append(0);
        results["softplus"].append(softplus(x));
        results["inverse_softplus"].append(inverse_softplus(x));
        results["hard_slope"].append(hard_slope(x));
        results["soft_slope"].append(soft_slope(x));
        results["d_soft_slope"].append(d_soft_slope(x));
        results["n_choose"].append(n_choose(int(x), int(x) + 2));
        if (x >= 0) {
            results["safelog"].append(safelog(x));
            results["safeflog"].append(safeflog(x));
            results["safeflog"].append(safeflog(x, x + 3));
            results["safeflog2"].append(safeflog2(x));
        } else {
            results["safelog"].append(0);
            results["safeflog"].append(0);
            results["safeflog"].append(0);
            results["safeflog2"].append(0);
        }
        results["sqrt"].append(sqrt(x));
        results["tanh"].append(tanh(x));
        results["exp"].append(exp(x));
        results["safeexp"].append(safeexp(x));
        results["pl_log"].append(pl_log(x));
        results["log_a_b"].append(log(x, x + 3));
        results["logtwo"].append(logtwo(x));
        results["logadd"].append(logadd(x, x + 2));
        if (!is_missing(x) && !isinf(x)) {
            results["logsub"].append(logsub(x, x - 1));
            results["dilogarithm"].append(dilogarithm(x));
            results["fasttanh"].append(fasttanh(x));
            results["fastsigmoid"].append(fastsigmoid(x));
            results["ultrafasttanh"].append(ultrafasttanh(x));
            results["ultrafastsigmoid"].append(ultrafastsigmoid(x));
            results["tabulated_softplus"].append(tabulated_softplus(x));
            results["tabulated_soft_slope"].append(tabulated_soft_slope(x));
        } else {
            results["logsub"].append(0);
            results["dilogarithm"].append(0);
            results["fasttanh"].append(0);
            results["fastsigmoid"].append(0);
            results["ultrafasttanh"].append(0);
            results["ultrafastsigmoid"].append(0);
            results["tabulated_softplus"].append(0);
            results["tabulated_soft_slope"].append(0);
        }
        if (FABS(x) < 100) {
            results["softplus_primitive"].append(softplus_primitive(x));
            results["tabulated_softplus_primitive"].append(tabulated_softplus_primitive(x));
            results["hard_slope_integral"].append(hard_slope_integral(x));
            results["soft_slope_integral"].append(soft_slope_integral(x));
            results["tabulated_soft_slope_integral"].append(tabulated_soft_slope_integral(x));
        } else {
            results["softplus_primitive"].append(0);
            results["tabulated_softplus_primitive"].append(0);
            results["hard_slope_integral"].append(0);
            results["soft_slope_integral"].append(0);
            results["tabulated_soft_slope_integral"].append(0);
        }
        if (pb)
            pb->update(i + 1);
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
