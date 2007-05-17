// -*- C++ -*-

// MatrixModule.cc
//
// Copyright (C) 2007 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file MatrixModule.cc */



#include "MatrixModule.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    MatrixModule,
    "Module that sees a single matrix.",
    ""
);

//////////////////
// MatrixModule //
//////////////////
MatrixModule::MatrixModule(const string& the_name, bool call_build_):
    inherited(the_name.empty() && call_build_ ? classname() : the_name,
              call_build_)
{
    if (call_build_)
        build_();
}

////////////////////
// declareOptions //
////////////////////
void MatrixModule::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "data", &MatrixModule::data,
                  OptionBase::buildoption,
        "The matrix seen by this module.");

    declareOption(ol, "data_gradient", &MatrixModule::data_gradient,
                  OptionBase::buildoption,
        "The gradient w.r.t. 'data'. If not provided, is assumed to be 0.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void MatrixModule::build_()
{}

///////////
// build //
///////////
void MatrixModule::build()
{
    inherited::build();
    build_();
}

////////////////////
// bpropAccUpdate //
////////////////////
void MatrixModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                  const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_gradient.length() == 1 );
    Mat* grad = ports_gradient[0];
    if (!grad)
        return;
    if (grad->isEmpty()) {
        // Accumulate 'data_gradient' into gradient (if there actually is a
        // gradient).
        grad->resize(data.length(), data.width());
        if (!data_gradient.isEmpty()) {
            PLASSERT( data.length() == data_gradient.length() &&
                      data.width()  == data_gradient.width() );
            *grad += data_gradient;
        }
    } else {
        data_gradient.resize(grad->length(), grad->width());
        data_gradient << *grad;
        PLERROR("In MatrixModule::bpropAccUpdate - Update of the underlying "
                "data matrix is not yet implemented");
    }
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void MatrixModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("MatrixModule::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

///////////
// fprop //
///////////
void MatrixModule::fprop(const Vec& input, Vec& output) const
{
    PLERROR("In MatrixModule::fprop - Not implemented");
}

void MatrixModule::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( ports_value.length() == 1 );
    Mat* mat = ports_value[0];
    if (!mat)
        return;
    if (mat->isEmpty()) {
        // We want to query the value of the matrix.
        mat->resize(data.length(), data.width());
        *mat << data;
    } else {
        // We want to store the value of the matrix.
        data.resize(mat->length(), mat->width());
        data << *mat;
    }
}

/////////////////
// bpropUpdate //
/////////////////
/* THIS METHOD IS OPTIONAL
void MatrixModule::bpropUpdate(const Vec& input, const Vec& output,
                               Vec& input_gradient,
                               const Vec& output_gradient,
                               bool accumulate)
{
}
*/

/* THIS METHOD IS OPTIONAL
void MatrixModule::bpropUpdate(const Vec& input, const Vec& output,
                               const Vec& output_gradient)
{
}
*/

//////////////////
// bbpropUpdate //
//////////////////
/* THIS METHOD IS OPTIONAL
void MatrixModule::bbpropUpdate(const Vec& input, const Vec& output,
                                Vec& input_gradient,
                                const Vec& output_gradient,
                                Vec& input_diag_hessian,
                                const Vec& output_diag_hessian,
                                bool accumulate)
{
}
*/

/* THIS METHOD IS OPTIONAL
void MatrixModule::bbpropUpdate(const Vec& input, const Vec& output,
                                const Vec& output_gradient,
                                const Vec& output_diag_hessian)
{
}
*/

////////////
// forget //
////////////
void MatrixModule::forget()
{
    // Nothing to forget.
}

//////////////
// getPorts //
//////////////
const TVec<string>& MatrixModule::getPorts()
{
    static TVec<string> ports;
    if (ports.isEmpty())
        ports.append("data");
    return ports;
}

//////////////////
// getPortSizes //
//////////////////
const TMat<int>& MatrixModule::getPortSizes() {
    port_sizes.resize(1, 2);
    port_sizes(0, 0) = data.length();
    port_sizes(0, 1) = data.width();
    return port_sizes;
}

//////////////
// finalize //
//////////////
/* THIS METHOD IS OPTIONAL
void MatrixModule::finalize()
{
}
*/

/////////////
// getData //
/////////////
Mat& MatrixModule::getData()
{
    return this->data;
}

//////////////////////
// bpropDoesNothing //
//////////////////////
/* THIS METHOD IS OPTIONAL
bool MatrixModule::bpropDoesNothing()
{
}
*/

///////////////////
// setGradientTo //
///////////////////
void MatrixModule::setGradientTo(real g)
{
    data_gradient.resize(data.length(), data.width());
    data_gradient.fill(g);
}

/////////////////////
// setLearningRate //
/////////////////////
/* OPTIONAL
void MatrixModule::setLearningRate(real dynamic_learning_rate)
{
}
*/

/////////////
// setData //
/////////////
void MatrixModule::setData(const Mat& the_data)
{
    this->data = the_data;
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
