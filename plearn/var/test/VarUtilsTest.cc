// -*- C++ -*-

// VarUtilsTest.cc
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

/*! \file VarUtilsTest.cc */


#include "VarUtilsTest.h"
#include <plearn/math/PRandom.h>
#include <plearn/var/VarArray.h>
#include <plearn/var/Var_utils.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    VarUtilsTest,
    "Test various functions in Var_utils",
    ""
);

//////////////////
// VarUtilsTest //
//////////////////
VarUtilsTest::VarUtilsTest() 
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
void VarUtilsTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void VarUtilsTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("VarUtilsTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void VarUtilsTest::declareOptions(OptionList& ol)
{
    declareOption(ol, "mat_results", &VarUtilsTest::mat_results,
        OptionBase::buildoption,
        "Test Mat results.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void VarUtilsTest::build_()
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
void VarUtilsTest::perform()
{
    int var_length = 10;
    int var_width = 5;
    int bound = 10;
    Var tmp;
    VarArray prop_path;
    Var mat_var(var_length, var_width);
    Var vec_var(var_length, 1);
    Var prob_mat_var(var_length, var_width);
    Var prob_vec_var(var_length, 1);
    Var scalar_var(1, 1);
    Var multi_index_var(var_width, 1);
    multi_index_var->matValue << "1 3 5 2 9";
    Var single_index_var(1, 1);
    single_index_var->matValue(0, 0) = 3;
    PRandom::common(false)->fill_random_uniform(mat_var->matValue, -bound, bound);
    PRandom::common(false)->fill_random_uniform(vec_var->matValue, -bound, bound);
    PRandom::common(false)->fill_random_uniform(prob_mat_var->matValue, 0, 1);
    PRandom::common(false)->fill_random_uniform(prob_vec_var->matValue, 0, 1);
    PRandom::common(false)->fill_random_uniform(scalar_var->matValue, -bound, bound);

    tmp = mean(mat_var);
    prop_path = propagationPath(tmp);
    prop_path.fprop();
    mat_results["mean"] = tmp->matValue;

    tmp = neg_log_pi(prob_vec_var, single_index_var);
    prop_path = propagationPath(tmp);
    prop_path.fprop();
    mat_results["neg_log_pi_vec"] = tmp->matValue;

    tmp = neg_log_pi(prob_mat_var, multi_index_var);
    prop_path = propagationPath(tmp);
    prop_path.fprop();
    mat_results["neg_log_pi_mat"] = tmp->matValue;

    tmp = softmax(prob_vec_var, 2);
    prop_path = propagationPath(tmp);
    prop_path.fprop();
    mat_results["softmax"] = tmp->matValue;

    tmp = pownorm(vec_var, 1.5);
    prop_path = propagationPath(tmp);
    prop_path.fprop();
    mat_results["pownorm"] = tmp->matValue;

    tmp = norm(vec_var, 1.5);
    prop_path = propagationPath(tmp);
    prop_path.fprop();
    mat_results["norm"] = tmp->matValue;

    tmp = entropy(vec_var);
    prop_path = propagationPath(tmp);
    prop_path.fprop();
    mat_results["entropy"] = tmp->matValue;

    tmp = distance(vec_var, prob_vec_var, 2);
    prop_path = propagationPath(tmp);
    prop_path.fprop();
    mat_results["distance"] = tmp->matValue;

    tmp = powdistance(vec_var, prob_vec_var, 2);
    prop_path = propagationPath(tmp);
    prop_path.fprop();
    mat_results["powdistance"] = tmp->matValue;
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
