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
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

/** ShiftAndRescaleVMatrix **/

PLEARN_IMPLEMENT_OBJECT(ShiftAndRescaleVMatrix,
                        "Applies a linear transformation to columns of a"
                        " source VMatrix.",
                        "The default behavior is to shift and scale only the"
                        " input columns in order\n"
                        "to set their mean to 0 and their standard deviation"
                        " to 1, but the various\n"
                        "options can be used for finer control.\n"
                       );

ShiftAndRescaleVMatrix::ShiftAndRescaleVMatrix(bool call_build_)
    : inherited(call_build_),
      automatic(1),
      n_train(0),
      n_inputs(-1),
      negate_shift(false),
      no_scale(false),
      ignore_missing(false),
      verbosity(1)
{
    if( call_build_ )
        build_();
}

ShiftAndRescaleVMatrix::ShiftAndRescaleVMatrix(VMat the_source,
                                               bool call_build_)
    : inherited(the_source,
                the_source->length(),
                the_source->width(),
                call_build_),
      automatic(1),
      n_train(0),
      n_inputs(-1),
      negate_shift(false),
      no_scale(false),
      ignore_missing(false),
      verbosity(1)
{
    if( call_build_ )
        build_();
}

ShiftAndRescaleVMatrix::ShiftAndRescaleVMatrix(VMat the_source,
                                               Vec the_shift, Vec the_scale,
                                               bool call_build_)
    : inherited(the_source,
                the_source->length(),
                the_source->width(),
                call_build_),
      shift(the_shift),
      scale(the_scale),
      automatic(0),
      n_train(0),
      n_inputs(-1),
      negate_shift(false),
      no_scale(false),
      ignore_missing(false),
      verbosity(1)
{
    if( call_build_ )
        build_();
}


ShiftAndRescaleVMatrix::ShiftAndRescaleVMatrix(VMat the_source,
                                               int the_n_inputs,
                                               bool call_build_)
    : inherited(the_source,
                the_source->length(),
                the_source->width(),
                call_build_),
      shift(the_source->width()),
      scale(the_source->width()),
      automatic(1),
      n_train(0),
      n_inputs(the_n_inputs),
      negate_shift(false),
      no_scale(false),
      ignore_missing(false),
      verbosity(1)
{
    if( call_build_ )
        build_();
}

ShiftAndRescaleVMatrix::ShiftAndRescaleVMatrix(VMat the_source,
                                               int the_n_inputs,
                                               int the_n_train,
                                               bool the_ignore_missing,
                                               bool the_verbosity,
                                               bool call_build_)
    : inherited(the_source,
                the_source->length(),
                the_source->width(),
                call_build_),
      shift(the_source->width()),
      scale(the_source->width()),
      automatic(1),
      n_train(the_n_train),
      n_inputs(the_n_inputs),
      negate_shift(false),
      no_scale(false),
      ignore_missing(the_ignore_missing),
      verbosity(the_verbosity)
{
    if( call_build_ )
        build_();
}

void ShiftAndRescaleVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "underlying_vmat", &ShiftAndRescaleVMatrix::source,
                  (OptionBase::learntoption | OptionBase::nosave),
                  "DEPRECATED - Use 'source' instead.");

    declareOption(ol, "shift", &ShiftAndRescaleVMatrix::shift,
                  OptionBase::buildoption,
                  "Quantity added to each INPUT element of a row of the source"
                  " vmatrix.");

    declareOption(ol, "scale", &ShiftAndRescaleVMatrix::scale,
                  OptionBase::buildoption,
                  "Quantity multiplied to each shifted element of a row of the"
                  " source vmatrix.");

    declareOption(ol, "automatic", &ShiftAndRescaleVMatrix::automatic,
                  OptionBase::buildoption,
                  "Whether shift and scale are determined from the mean and"
                  " stdev\n"
                  "of the source vmatrix, or user-provided.\n");

    declareOption(ol, "n_train", &ShiftAndRescaleVMatrix::n_train,
                  OptionBase::buildoption,
                  "when automatic, use only the n_train first examples to"
                  " estimate\n"
                  "shift and scale, if n_train>0.\n");

    declareOption(ol, "n_inputs", &ShiftAndRescaleVMatrix::n_inputs,
                  OptionBase::buildoption,
                  "when automatic, shift and scale only the first n_inputs"
                  " columns\n"
                  "(If n_inputs<0, set n_inputs from"
                  " underlying_vmat->inputsize()).\n");

    declareOption(ol, "negate_shift", &ShiftAndRescaleVMatrix::negate_shift,
                  OptionBase::buildoption,
                  "If set to 1, the shift will be removed instead of added.");

    declareOption(ol, "no_scale", &ShiftAndRescaleVMatrix::no_scale,
                  OptionBase::buildoption,
                  "If set to 1, no scaling will be performed (only a shift"
                  " will be applied).");

    declareOption(ol, "ignore_missing",
                  &ShiftAndRescaleVMatrix::ignore_missing,
                  OptionBase::buildoption,
                  "If set to 1, then missing values will be ignored when"
                  " computing\n"
                  "mean and standard deviation.\n");

    declareOption(ol, "verbosity", &ShiftAndRescaleVMatrix::verbosity,
                  OptionBase::buildoption,
                  "Controls the amount of output.");

    declareOption(ol, "min_max", &ShiftAndRescaleVMatrix::min_max,            
                  OptionBase::buildoption,
        "A vector of size 2 [min,max]. For each column, the elements will be\n"
        "shifted and rescaled to be in [min,max]. If set, it will override\n"
        "the value of the 'automatic' option.");        
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void ShiftAndRescaleVMatrix::build_()
{
    if( source )
    {
        if (automatic && min_max.isEmpty())
        {
            if (n_inputs<0)
            {
                n_inputs = source->inputsize();
                if (n_inputs<0)
                    PLERROR("ShiftAndRescaleVMatrix: either n_inputs should be"
                            " provided explicitly\n"
                            "or the source VMatrix should have a set value of"
                            " inputsize.\n");
            }
            if (ignore_missing)
            {
                VMat src_to_normalize;
                if (n_train>0)
                    src_to_normalize = source.subMat(0, 0, n_train, n_inputs);
                else
                    src_to_normalize = source.subMatColumns(0, n_inputs);
                TVec<StatsCollector> stats =
                    PLearn::computeStats(src_to_normalize, 1, false);
                shift.resize(n_inputs);
                if (!no_scale)
                    scale.resize(n_inputs);
                for (int i = 0; i < n_inputs; i++) {
                    shift[i] = stats[i].mean();
                    if (!no_scale)
                        scale[i] = stats[i].stddev();
                }
            }
            else
            {
                if (n_train>0)
                    computeMeanAndStddev(source.subMatRows(0,n_train),
                                         shift, scale);
                else
                    computeMeanAndStddev(source, shift, scale);
            }
            if (!negate_shift)
                negateElements(shift);
            if (!no_scale) {
                for (int i=0;i<scale.length();i++)
                    if (fast_exact_is_equal(scale[i], 0))
                    {
                        if (verbosity >= 1)
                            PLWARNING("ShiftAndRescale: data column number %d"
                                      " is constant",i);
                        scale[i]=1;
                    }
                invertElements(scale);
                scale.resize(source->width());
                scale.subVec(n_inputs, scale.length()-n_inputs).fill(1);
            }
            shift.resize(source->width());
            shift.subVec(n_inputs, shift.length()-n_inputs).fill(0);
        }
        else {
            if (!min_max.isEmpty()) {
                
                if ( min_max.length()!=2 )
                    PLERROR("ShiftAndRescale: min_max should have exactly 2"
                            "elements(if set)") ; 
                
                if ( min_max[0] >= min_max[1])
                    PLERROR("ShiftAndRescale: min_max[0] should be smaller than"
                            "min_max[1]") ; 
            if (n_inputs<0)
            {
                n_inputs = source->inputsize();
                if (n_inputs<0)
                    PLERROR("ShiftAndRescaleVMatrix: either n_inputs should be"
                            " provided explicitly\n"
                            "or the source VMatrix should have a set value of"
                            " inputsize.\n");
            }
                
                Vec min_col(n_inputs) , max_col(n_inputs) ; 
                Mat data = transpose(source.subMatColumns(0,n_inputs).toMat());
                     
                rowMin(data , min_col) ; 
                rowMax(data , max_col) ; 
                
                shift.resize(source->width()) ; 
                shift.subVec(n_inputs, shift.length()-n_inputs).fill(0);
                scale.resize(source->width()) ; 
                scale.subVec(n_inputs, scale.length()-n_inputs).fill(1);

                for(int i=0 ; i<n_inputs ; ++i) { 
                    shift[i] = (max_col[i] - min_col[i]) / (min_max[1] - min_max[0]) * 
                                (min_max[0] - (min_max[1] - min_max[0])*min_col[i] /
                                              (max_col[i] - min_col[i]) ); 
                    scale[i] = (min_max[1] - min_max[0]) / (max_col[i] - min_col[i]) ; 
                }

            }
        }
        reset_dimensions();
        setMetaInfoFromSource();
    }
}

///////////////
// getNewRow //
///////////////
void ShiftAndRescaleVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getRow(i, v);

    if( negate_shift )
        v -= shift;
    else
        v += shift;

    if( !no_scale )
        v *= scale;
}

///////////
// build //
///////////
void ShiftAndRescaleVMatrix::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ShiftAndRescaleVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(shift,    copies);
    deepCopyField(scale,    copies);
    deepCopyField(min_max,  copies);
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
