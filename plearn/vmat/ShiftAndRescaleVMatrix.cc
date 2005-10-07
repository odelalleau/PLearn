// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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
 * $Id$
 ******************************************************* */

#include "ShiftAndRescaleVMatrix.h"
#include "VMat_computeStats.h"
#include "VMat_basic_stats.h"
#include <plearn/math/TMat_maths_impl.h>

namespace PLearn {
using namespace std;

/** ShiftAndRescaleVMatrix **/

PLEARN_IMPLEMENT_OBJECT(ShiftAndRescaleVMatrix,
                        "Applies a linear transformation to columns of an underlying VMatrix.",
                        "The default behavior is to shift and scale only the input columns in order\n"
                        "to set their mean to 0 and their standard deviation to 1, but the various\n"
                        "options can be used for finer control.\n"
    );

ShiftAndRescaleVMatrix::
ShiftAndRescaleVMatrix(VMat underlying_vm, Vec the_shift, Vec the_scale)
    : shift(the_shift), scale(the_scale), automatic(0), n_train(0), n_inputs(-1),
      negate_shift(false),
      no_scale(false),
      ignore_missing(false),
      verbosity(1)
{
    vm = underlying_vm;
    build_();
}


ShiftAndRescaleVMatrix::ShiftAndRescaleVMatrix()
    : automatic(1),n_train(0), n_inputs(-1),
      negate_shift(false),
      no_scale(false),
      ignore_missing(false),
      verbosity(1)
{}

ShiftAndRescaleVMatrix::ShiftAndRescaleVMatrix(VMat underlying_vm, int n_inputs_)
    : shift(underlying_vm->width()),
      scale(underlying_vm->width()), automatic(1),n_train(0), n_inputs(n_inputs_),
      negate_shift(false),
      no_scale(false),
      ignore_missing(false),
      verbosity(1)
{
    vm = underlying_vm;
    build_();
}

ShiftAndRescaleVMatrix::
ShiftAndRescaleVMatrix(VMat underlying_vm, int n_inputs_, int n_train_, bool the_ignore_missing, bool the_verbosity)
    : shift(underlying_vm->width()), scale(underlying_vm->width()), automatic(1), n_train(n_train_), n_inputs(n_inputs_),
      negate_shift(false),
      no_scale(false),
      ignore_missing(the_ignore_missing),
      verbosity(the_verbosity)
{
    vm = underlying_vm;
    build_();  
}

void ShiftAndRescaleVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "underlying_vmat", &ShiftAndRescaleVMatrix::vm, OptionBase::buildoption,
                  "This is the source vmatrix that will be shifted and rescaled");
    declareOption(ol, "shift", &ShiftAndRescaleVMatrix::shift, OptionBase::buildoption,
                  "This is the quantity added to each INPUT element of a row of the source vmatrix.");
    declareOption(ol, "scale", &ShiftAndRescaleVMatrix::scale, OptionBase::buildoption,
                  "This is the quantity multiplied to each shifted element of a row of the source vmatrix.");
    declareOption(ol, "automatic", &ShiftAndRescaleVMatrix::automatic, OptionBase::buildoption,
                  "Whether shift and scale are determined from the mean and stdev of the source vmatrix, or user-provided.");
    declareOption(ol, "n_train", &ShiftAndRescaleVMatrix::n_train, OptionBase::buildoption,
                  "when automatic, use only the n_train first examples to estimate shift and scale, if n_train>0.");
    declareOption(ol, "n_inputs", &ShiftAndRescaleVMatrix::n_inputs, OptionBase::buildoption,
                  "when automatic, shift and scale only the first n_inputs columns (If n_inputs<0, set n_inputs from underlying_vmat->inputsize()).");
    declareOption(ol, "negate_shift", &ShiftAndRescaleVMatrix::negate_shift, OptionBase::buildoption,
                  "If set to 1, the shift will be removed instead of added.");
    declareOption(ol, "no_scale", &ShiftAndRescaleVMatrix::no_scale, OptionBase::buildoption,
                  "If set to 1, no scaling will be performed (only a shift will be applied).");
    declareOption(ol, "ignore_missing", &ShiftAndRescaleVMatrix::ignore_missing, OptionBase::buildoption,
                  "If set to 1, then missing values will be ignored when computed mean and standard deviation.");
    declareOption(ol, "verbosity", &ShiftAndRescaleVMatrix::verbosity, OptionBase::buildoption,
                  "Controls the amount of output.");
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ShiftAndRescaleVMatrix::build_()
{
    if (vm) {
        writable = vm->isWritable();
        length_ = vm->length();
        width_  = vm->width();
        if (vm->hasMetaDataDir())
            setMetaDataDir(vm->getMetaDataDir());
        setFieldInfos(TVec<VMField>()); // Ensure we get the fieldinfos from vm.
        setMetaInfoFrom(vm);
        if (automatic)
        {
            if (n_inputs<0)
            {
                n_inputs = vm->inputsize();
                if (n_inputs<0)
                    PLERROR("ShiftAndRescaleVMatrix: either n_inputs should be provided explicitly "
                            "or the underlying VMatrix should have a set value of inputsize");
            }
            if (ignore_missing) {
                VMat vm_to_normalize;
                if (n_train>0)
                    vm_to_normalize = vm.subMat(0, 0, n_train, n_inputs);
                else
                    vm_to_normalize = vm.subMatColumns(0, n_inputs);
                TVec<StatsCollector> stats = PLearn::computeStats(vm_to_normalize, 1, false);
                shift.resize(n_inputs);
                if (!no_scale)
                    scale.resize(n_inputs);
                for (int i = 0; i < n_inputs; i++) {
                    shift[i] = stats[i].mean();
                    if (!no_scale)
                        scale[i] = stats[i].stddev();
                }
            } else {
                if (n_train>0)
                    computeMeanAndStddev(vm.subMatRows(0,n_train), shift, scale);
                else
                    computeMeanAndStddev(vm, shift, scale);
            }
            if (!negate_shift)
                negateElements(shift);
            if (!no_scale) {
                for (int i=0;i<scale.length();i++) 
                    if (fast_exact_is_equal(scale[i], 0))
                    {
                        if (verbosity >= 1)
                            PLWARNING("ShiftAndRescale: data column number %d is constant",i);
                        scale[i]=1;
                    }
                invertElements(scale);
                scale.resize(vm->width());
                scale.subVec(n_inputs, scale.length()-n_inputs).fill(1);
            }
            shift.resize(vm->width());
            shift.subVec(n_inputs, shift.length()-n_inputs).fill(0);
        }
        reset_dimensions();
    }
}

                                         
real ShiftAndRescaleVMatrix::get(int i, int j) const
{
    if (negate_shift) {
        if (no_scale) {
            return vm->get(i,j) - shift[j];
        } else {
            return (vm->get(i,j) - shift[j]) * scale[j];
        }
    } else {
        if (no_scale) {
            return vm->get(i,j) + shift[j];
        } else {
            return (vm->get(i,j) + shift[j]) * scale[j];
        }
    }
}

void ShiftAndRescaleVMatrix::getSubRow(int i, int j, Vec v) const
{
    vm->getSubRow(i,j,v);
    if (negate_shift) {
        if (no_scale) {
            v -= shift;
        } else {
            for(int jj=0; jj<v.length(); jj++)
                v[jj] = (v[jj] - shift[j+jj]) * scale[j+jj];
        }
    } else {
        if (no_scale) {
            v += shift;
        } else {
            for(int jj=0; jj<v.length(); jj++)
                v[jj] = (v[jj] + shift[j+jj]) * scale[j+jj];
        }
    }
}

void ShiftAndRescaleVMatrix::build()
{
    inherited::build();
    build_();
}

} // end of namespcae PLearn


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
