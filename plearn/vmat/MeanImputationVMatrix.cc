// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
// Copyright (C) 2003, 2006 Olivier Delalleau
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


/* *******************************************************************
 * $Id: MeanImputationVMatrix.cc 3658 2005-07-06 20:30:15  Godbout $
 ******************************************************************* */


#include "MeanImputationVMatrix.h"
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/vmat/VMat_basic_stats.h>

namespace PLearn {
using namespace std;

/** MeanImputationVMatrix **/

PLEARN_IMPLEMENT_OBJECT(
    MeanImputationVMatrix,
    "Imputes the observed variable mean to replace missing values in source.",
    "This class will replace missing values in the underlying dataset with\n"
    "the observed mean of a variable.\n"
    "If the 'number_of_train_samples' option is different than zero, the\n"
    "mean is computed only on that first portion of the underlying VMat.\n"
    "Otherwise, the mean is computed on the entire dataset.\n"
    "Optionally, if one wants to obtain the variable means from another\n"
    "dataset than the underlying source VMatrix, the 'mean_source' option\n"
    "can be specified.\n"
);

///////////////////////////
// MeanImputationVMatrix //
///////////////////////////
MeanImputationVMatrix::MeanImputationVMatrix():
    obtained_inputsize_from_source(false),
    obtained_targetsize_from_source(false),
    obtained_weightsize_from_source(false),
    number_of_train_samples(0.0)
{}

MeanImputationVMatrix::MeanImputationVMatrix(VMat the_source,
                                             real the_number_of_train_samples,
                                             bool call_build_):
    inherited(the_source, call_build_),
    obtained_inputsize_from_source(false),
    obtained_targetsize_from_source(false),
    obtained_weightsize_from_source(false),
    number_of_train_samples(the_number_of_train_samples)
{
    if (call_build_)
        build_();
}

////////////////////
// declareOptions //
////////////////////
void MeanImputationVMatrix::declareOptions(OptionList &ol)
{

    declareOption(ol, "number_of_train_samples",
                  &MeanImputationVMatrix::number_of_train_samples,
                  OptionBase::buildoption,
        "If equal to zero, all the underlying dataset samples are used to\n"
        "compute the variable means. If it is a fraction between 0 and 1,\n"
        "this proportion of the samples will be used. If greater than or\n"
        "equal to 1, the integer portion will be interpreted as the number\n"
        "of samples to use.");

    declareOption(ol, "mean_source",
                  &MeanImputationVMatrix::mean_source,
                  OptionBase::buildoption,
        "If specified, this VMat will be used to compute the means instead\n"
        "of the 'source' option.");

    declareOption(ol, "variable_mean", &MeanImputationVMatrix::variable_mean,
                                       OptionBase::learntoption,
        "The vector of variable means observed from the source matrix.");

    declareOption(ol, "obtained_inputsize_from_source",
                  &MeanImputationVMatrix::obtained_inputsize_from_source,
                  OptionBase::learntoption,
        "Set to 1 when the inputsize was obtained from the source matrix.");

    declareOption(ol, "obtained_targetsize_from_source",
                  &MeanImputationVMatrix::obtained_targetsize_from_source,
                  OptionBase::learntoption,
        "Set to 1 when the targetsize was obtained from the source matrix.");

    declareOption(ol, "obtained_weightsize_from_source",
                  &MeanImputationVMatrix::obtained_weightsize_from_source,
                  OptionBase::learntoption,
        "Set to 1 when the weightsize was obtained from the source matrix.");

    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void MeanImputationVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void MeanImputationVMatrix::build_()
{
    if (source) {
        string error_msg =
            "In MeanImputationVMatrix::build_ - For safety reasons, it is forbidden to "
            "re-use sizes obtained from a previous source VMatrix with a new source "
            "VMatrix having different sizes";
        length_ = source->length();
        width_ = source->width();
        if(inputsize_<0) {
            inputsize_ = source->inputsize();
            obtained_inputsize_from_source = true;
        } else if (obtained_inputsize_from_source && inputsize_ != source->inputsize())
            PLERROR(error_msg.c_str());
        if(targetsize_<0) {
            targetsize_ = source->targetsize();
            obtained_targetsize_from_source = true;
        } else if (obtained_targetsize_from_source && targetsize_ != source->targetsize())
            PLERROR(error_msg.c_str());
        if(weightsize_<0) {
            weightsize_ = source->weightsize();
            obtained_weightsize_from_source = true;
        } else if (obtained_weightsize_from_source && weightsize_ != source->weightsize())
            PLERROR(error_msg.c_str());
        setMetaInfoFromSource();
        computeMeanVector();
    } else {
        // Restore the original undefined sizes if the current one had been obtained
        // from the source VMatrix.
        if (obtained_inputsize_from_source) {
            inputsize_ = -1;
            obtained_inputsize_from_source = false;
        }
        if (obtained_targetsize_from_source) {
            targetsize_ = -1;
            obtained_targetsize_from_source = false;
        }
        if (obtained_weightsize_from_source) {
            weightsize_ = -1;
            obtained_weightsize_from_source = false;
        }
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void MeanImputationVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(variable_mean, copies);
    deepCopyField(mean_source,   copies);
}


///////////////
// getNewRow //
///////////////
void MeanImputationVMatrix::getNewRow(int i, const Vec& v) const
{
    assert( source );
    source->getRow(i, v);
    for (int j = 0; j < v.length(); j++)
        if (is_missing(v[j]))
            v[j] = variable_mean[j];
}

///////////////////
// getMeanVector //
///////////////////
Vec MeanImputationVMatrix::getMeanVector()
{
    return variable_mean;
}

///////////////////////
// computeMeanVector //
///////////////////////
void MeanImputationVMatrix::computeMeanVector()
{
    VMat the_mean_source;
    if (mean_source) {
        assert( mean_source->width() == source->width() );
        the_mean_source = mean_source;
    } else
        the_mean_source = source;
   
    assert( the_mean_source );

    int length = the_mean_source->length();
    int width = width_;
    assert( width = the_mean_source->width() );
    variable_mean.resize(width);
    if (number_of_train_samples > 0.0)
    {
        if (number_of_train_samples >= 1.0)
            length = (int) number_of_train_samples;
        else
            length = (int) ((double) length * number_of_train_samples);
        if (length < 1)
            length = 1;
        if (length > the_mean_source->length())
            length = the_mean_source->length();
    }
    VMat sub_source = the_mean_source;
    if (length != the_mean_source->length())
        sub_source = new SubVMatrix(sub_source, 0, 0,
                                    length, sub_source->width());
    computeMean(sub_source, variable_mean);
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
