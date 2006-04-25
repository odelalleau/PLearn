// -*- C++ -*-

// TMatTest.cc
//
// Copyright (C) 2005-2006 Olivier Delalleau 
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

/*! \file TMatTest.cc */


#include "TMatTest.h"
#include <plearn/math/PRandom.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    TMatTest,
    "Test TMat mathematical functions.",
    ""
);

//////////////
// TMatTest //
//////////////
TMatTest::TMatTest():
    bound(10),
    mat_length(2),
    mat_width(3),
    vec_length(10)
{}

///////////
// build //
///////////
void TMatTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void TMatTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("TMatTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void TMatTest::declareOptions(OptionList& ol)
{
    declareOption(ol, "vec_length", &TMatTest::vec_length,
        OptionBase::buildoption,
        "Length of the vector on which the TMat functions are to be applied.");

    declareOption(ol, "mat_length", &TMatTest::mat_length,
        OptionBase::buildoption,
        "Length of the matrix on which the TMat functions are to be applied.");

    declareOption(ol, "mat_width", &TMatTest::mat_width,
        OptionBase::buildoption,
        "Width of the matrix on which the TMat functions are to be applied.");

    declareOption(ol, "bound", &TMatTest::bound,
        OptionBase::buildoption,
        "Bound for the (real) values sampled in the vector.");

    declareOption(ol, "mat_options",  &TMatTest::mat_options,
        OptionBase::learntoption,
        "Matrices.");
        
    declareOption(ol, "real_options", &TMatTest::real_options,
        OptionBase::learntoption,
        "Real numbers");

    declareOption(ol, "vec_options",  &TMatTest::vec_options,
        OptionBase::learntoption,
        "Vectors");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void TMatTest::build_()
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
void TMatTest::perform()
{
    assert( vec_length > 0 ); // TODO Make it work with vec_length == 0
    assert( bound > 0 );
    Vec vec( vec_length );
    PRandom::common(false)->fill_random_uniform(vec, -bound, bound);
    Mat mat(mat_length, mat_width);
    PRandom::common(false)->fill_random_uniform(mat, -bound, bound);

    pout << "Starting TMatTest with vector of length " << vec_length
         << " and matrix of size (" << mat_length << " x " << mat_width
         << ")" << flush;

    this->vec_options["vector"]                 = vec;
    
    this->vec_options ["sign"]                  = sign          ( vec );

    this->real_options["sum"]                   = sum           ( vec );
    this->real_options["sum (ignore missing)"]  = sum           ( vec, true );

    this->real_options["sumabs"]                = sumabs        ( vec );
    this->real_options["sumsquare"]             = sumsquare     ( vec );
    this->real_options["sum_of_log"]            = sum_of_log    ( vec );
    this->real_options["product"]               = product       ( vec );

    this->real_options["mean"]                  = mean          ( vec );
    this->real_options["mean (ignore missing)"] = mean          ( vec, true );

    this->real_options["harmonic_mean"]         = harmonic_mean ( vec );
    this->real_options["harmonic_mean (ignore_missing)"]
                                                = harmonic_mean ( vec, true );

    this->real_options["min"]                   = min           ( vec );
    this->real_options["argmin"]                = argmin        ( vec );

    this->real_options["max"]                   = max           ( vec );
    this->real_options["argmax"]                = argmax        ( vec );

    this->real_options["norm"]                  = norm          ( vec );
    this->vec_options ["log"]                   = log           ( vec );
    this->vec_options ["sqrt"]                  = sqrt          ( vec );
    this->vec_options ["tanh"]                  = tanh          ( vec );
    this->vec_options ["fasttanh"]              = fasttanh      ( vec );

    this->vec_options ["inverted"]              = inverted      ( vec );
    this->vec_options ["square"]                = square        ( vec );
    this->vec_options ["squareroot"]            = squareroot    ( vec );
    this->vec_options ["remove_missing"]        = remove_missing( vec );
    this->vec_options ["softmax"]               = softmax       ( vec );
    this->vec_options ["exp"]                   = exp           ( vec );
    this->vec_options ["nonZeroIndices"]        = nonZeroIndices( vec );
    this->real_options["logadd"]                = logadd        ( vec );
    this->real_options["median"]                = median        ( vec );

    Vec scaled_vec = vec.copy();
    scaled_vec *= scaled_vec[0];
    this->vec_options["operator*=_vec"]         = scaled_vec;

    Vec multiply_acc_vec = vec.copy();
    multiplyAcc(multiply_acc_vec, vec, -scaled_vec[0]);
    this->vec_options["multiplyAcc_vec"]        = multiply_acc_vec;

    Mat scaled_mat = mat.copy();
    scaled_mat *= scaled_mat(0, 0);
    this->mat_options["operator*=_mat"]         = scaled_mat;

    Mat multiply_acc_mat = mat.copy();
    multiplyAcc(multiply_acc_mat, mat, -scaled_mat(0, 0));
    this->mat_options["multiplyAcc_mat"]        = multiply_acc_mat;

    pout << "... DONE!" << endl;
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
