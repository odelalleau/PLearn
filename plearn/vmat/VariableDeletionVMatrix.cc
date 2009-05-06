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


/* *********************************************************************
 * $Id: VariableDeletionVMatrix.cc 3658 2005-07-06 20:30:15  Godbout $
 ********************************************************************* */

#include "VariableDeletionVMatrix.h"
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/vmat/VMat_computeStats.h>
#include <plearn/io/fileutils.h>
#include <plearn/io/load_and_save.h>
#define PL_LOG_MODULE_NAME "VariableDeletionVMatrix"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    VariableDeletionVMatrix,
    "Deletes its source's inputs with most missing / constant values.",
    "The columns that are removed are those that:\n"
    "- contain a proportion of missing values higher than a threshold\n"
    "  equal to 1 - min_non_missing_threshold, or\n"
    "- contain a single value whose proportion in the source dataset is\n"
    "  higher than (or equal to) max_constant_threshold (if >0).\n"
    "\n"
    "Note that this selection is performed only on input variables (the\n"
    "inputsize is modified accordingly), while the target, weight and extra\n"
    "columns are always preserved.\n"
);

/////////////////////////////
// VariableDeletionVMatrix //
/////////////////////////////
VariableDeletionVMatrix::VariableDeletionVMatrix():
    min_non_missing_threshold(0),
    max_constant_threshold(0),
    number_of_train_samples(0),
    warn_removed_var(false),
    info_var_with_missing(false),
    deletion_threshold(-1),
    remove_columns_with_constant_value(-1)
{}

VariableDeletionVMatrix::VariableDeletionVMatrix(
        VMat the_source, real the_min_non_missing_threshold, 
        bool the_remove_columns_with_constant_value,
        int  the_number_of_train_samples,
        bool call_build_):
    inherited(the_source, TVec<int>(), call_build_),
    min_non_missing_threshold(the_min_non_missing_threshold),
    number_of_train_samples(the_number_of_train_samples),
    remove_columns_with_constant_value(the_remove_columns_with_constant_value)
{
    if (call_build_)
        build_();
    PLDEPRECATED("In VariableDeletionVMatrix::VariableDeletionVMatrix - You "
                 "are using a deprecated constructor");
    // Note: this constructor should take as argument the new non-deprecated
    // options.
}

////////////////////
// declareOptions //
////////////////////
void VariableDeletionVMatrix::declareOptions(OptionList &ol)
{

    declareOption(ol, "min_non_missing_threshold",
                  &VariableDeletionVMatrix::min_non_missing_threshold,
                  OptionBase::buildoption,
        "Minimum proportion of non-missing values for a variable to be kept\n"
        "(a 1 means only variables with no missing values are kept).\n"
        "if >0, we will always remove columns with all missing value.");

    declareOption(ol, "max_constant_threshold",
                  &VariableDeletionVMatrix::max_constant_threshold,
                  OptionBase::buildoption,
        "Maximum proportion of a unique value for a variable to be kept\n"
        "(contrary to 'min_non_missing_threshold', a variable will be\n"
        "removed if this proportion is attained, so that a 1 means only\n"
        "constant variables are removed, while 0 is a special value meaning\n"
        "that none is removed).\n"
        "Note also that this proportion is computed over the non-missing\n"
        "values only.");

    declareOption(ol, "number_of_train_samples",
                  &VariableDeletionVMatrix::number_of_train_samples,
                  OptionBase::buildoption,
        "If equal to zero, all the underlying dataset samples are used to\n"
        "compute the percentages and constant values.\n"
        "If it is a fraction between 0 and 1, only this proportion of the\n"
        "samples will be used.\n"
        "If greater than or equal to 1, the integer portion will be\n"
        "interpreted as the number of samples to use.");

    declareOption(ol, "warn_removed_var",
                  &VariableDeletionVMatrix::warn_removed_var,
                  OptionBase::buildoption,
                  "If true, will print a warning about variable that are removed");

    declareOption(ol, "info_var_with_missing",
                  &VariableDeletionVMatrix::info_var_with_missing,
                  OptionBase::buildoption,
                  "If true, will print the variable that have some missing"
                  " that we keep.");

    declareOption(ol, "save_deleted_columns",
                  &VariableDeletionVMatrix::save_deleted_columns,
                  OptionBase::buildoption,
                  "If not empty will save the deleted culumns in this file."
                  "If present, will verify that it have the same content then"
                  " the calculated data.");

    declareOption(ol, "complete_dataset",
                  &VariableDeletionVMatrix::complete_dataset,
                  OptionBase::learntoption,
        "DEPRECATED (use 'source' instead) - The data set with all variables\n"
        "to select the columns from.");

    declareOption(ol, "train_set", &VariableDeletionVMatrix::train_set, OptionBase::buildoption,
                  "The train set in which to compute the percentage of missing values.\n"
                  "If null, will use the source to compute the percentage of missing values.");

    declareOption(ol, "deletion_threshold",
                  &VariableDeletionVMatrix::deletion_threshold,
                  OptionBase::learntoption,
        "DEPRECATED (use 'min_non_missing_threshold' instead) - The\n"
        "percentage of non-missing values for a variable above which\n"
        "the variable will be selected.");

    declareOption(ol, "remove_columns_with_constant_value",
                  &VariableDeletionVMatrix::remove_columns_with_constant_value,
                  OptionBase::learntoption,
        "DEPRECATED (use 'max_constant_threshold' instead) - If set to 1,\n"
        "the columns with constant non-missing values will be removed.");

    inherited::declareOptions(ol);

    // Hide unused parent class' options.

    redeclareOption(ol, "fields", &VariableDeletionVMatrix::fields,
                                  OptionBase::nosave,
        "Not used in VariableDeletionVMatrix.");

    redeclareOption(ol, "fields_partial_match",
                    &VariableDeletionVMatrix::fields_partial_match,
                    OptionBase::nosave,
        "Not used in VariableDeletionVMatrix.");

    redeclareOption(ol, "indices", &VariableDeletionVMatrix::indices,
                                   OptionBase::nosave,
        "Not used in VariableDeletionVMatrix.");

    redeclareOption(ol, "extend_with_missing",
                    &VariableDeletionVMatrix::extend_with_missing,
                    OptionBase::nosave,
        "Not used in VariableDeletionVMatrix.");

    redeclareOption(ol, "inputsize", &VariableDeletionVMatrix::inputsize_,
                                     OptionBase::nosave,
        "Not used in VariableDeletionVMatrix.");

    redeclareOption(ol, "targetsize", &VariableDeletionVMatrix::targetsize_,
                                     OptionBase::nosave,
        "Not used in VariableDeletionVMatrix.");
                
    redeclareOption(ol, "weightsize", &VariableDeletionVMatrix::weightsize_,
                                     OptionBase::nosave,
        "Not used in VariableDeletionVMatrix.");
    
    redeclareOption(ol, "extrasize", &VariableDeletionVMatrix::extrasize_,
                                     OptionBase::nosave,
        "Not used in VariableDeletionVMatrix.");
}

///////////
// build //
///////////
void VariableDeletionVMatrix::build()
{
    //must be done even if we will call it later to have the
    //source metadatadir set correctly.
    bool saved_warn_non_selected_field=warn_non_selected_field;
    warn_non_selected_field=false;
    inherited::build();
    warn_non_selected_field=saved_warn_non_selected_field;
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void VariableDeletionVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(complete_dataset, copies);
    deepCopyField(train_set, copies);
}

////////////
// build_ //
////////////
void VariableDeletionVMatrix::build_()
{
    if (complete_dataset) {
        PLDEPRECATED("In VariableDeletionVMatrix::build_ - The option "
                     "'complete_dataset' is deprecated, use source instead");
        if (source && source != complete_dataset)
            PLERROR("In VariableDeletionVMatrix::build_ - A source was also "
                    "specified, but it is different from 'complete_dataset'");
        source = complete_dataset;
    }
    if (!is_equal(deletion_threshold, -1)) {
        PLDEPRECATED("In VariableDeletionVMatrix::build_ - You are using the "
                     "deprecated option 'deletion_threshold', you should "
                     "instead use 'min_non_missing_threshold'");
        min_non_missing_threshold = deletion_threshold;
    }
    if (remove_columns_with_constant_value != -1) {
        PLDEPRECATED("In VariableDeletionVMatrix::build_ - You are using the "
                     "deprecated option 'remove_columns_with_constant_value', "
                     "you should instead use 'max_constant_threshold'");
        max_constant_threshold = remove_columns_with_constant_value == 0 ? 0:1;
    }
    if(!source)
        PLERROR("In VariableDeletionVMatrix::build_ - The source VMat do not exist!");

    updateMtime(source);
    updateMtime(train_set);

    int is = source->inputsize();
    if (is < 0)
        PLERROR("In VariableDeletionVMatrix::build_ - The source VMat must "
                "have an inputsize defined");

    VMat the_train_source = train_set ? train_set : source;
    PLCHECK( the_train_source.width() == source->width() );

    if (number_of_train_samples > 0 &&
        number_of_train_samples < the_train_source->length())
        the_train_source = new SubVMatrix(the_train_source, 0, 0,
                                          number_of_train_samples,
                                          the_train_source->width());
    TVec<StatsCollector> stats;
    if(min_non_missing_threshold > 0 || max_constant_threshold > 0){
        int maxnvalues = -1;
        if(is_equal(max_constant_threshold,1))
//We don't need all the value, if (min==max && non_missing_value>0) it is constant value.
            maxnvalues = 0;
        if(!the_train_source->hasMetaDataDir() && hasMetaDataDir())
            the_train_source->setMetaDataDir(getMetaDataDir()+"/source");
        stats = the_train_source->
            getPrecomputedStatsFromFile("stats_variableDeletionVMatrix_"+
                                        tostring(maxnvalues)+".psave",
                                        maxnvalues, true);
        PLCHECK( stats.length() == source->width() );
    }

    indices.resize(0);

    // First remove columns that have too many missing values.
    if (min_non_missing_threshold > 0){
        int min_non_missing =
            int(round(min_non_missing_threshold * the_train_source->length()));
        TVec<int> have_missing;
        for (int i = 0; i < is; i++){
            if (stats[i].nnonmissing() >= min_non_missing 
                && stats[i].nnonmissing() > 0)
                indices.append(i);
            else if (warn_removed_var)
                PLWARNING("In VariableDeletionVMatrix::build_() var '%s'"
                          " have too many missing (%d/%d). We remove it.",
                          source->fieldName(i).c_str(),
                          int(stats[i].nmissing()),
                          int(stats[i].n()));
            if (info_var_with_missing && stats[i].nmissing() > 0){
                have_missing.append(i);
            }
        }
        if(have_missing.length()>0){
            string s="INFO: In build_() variable with missing value (var,nb_missing/nb_value): ";
            for(int k=0;k<have_missing.length();k++){
                int i = have_missing[k];
                s+=" ("+source->fieldName(i)
                    +","+tostring(stats[i].nmissing())
                    +"/"+ tostring(stats[i].n())+")";

            }
            MODULE_LOG<<s<<endl;
        }
                
    } else
        for (int i = 0; i < is; i++)
            indices.append(i);
    // Then remove columns that are too constant.
    TVec<int> final_indices;
    if (is_equal(max_constant_threshold,1)){
        TVec<int> const_indices;
        for (int k = 0; k < indices.length(); k++) {
            int i = indices[k];
            StatsCollector stat = stats[i];
            if(!(stat.min()==stat.max() && stat.nnonmissing()>0))
                final_indices.append(i);
            else if (warn_removed_var)
                const_indices.append(i);
        }
        if(warn_removed_var && const_indices.length()>0){
            string s = " WARNING: In VariableDeletionVMatrix::build_() - The following tuple (variable, constant value) indicate variable that are removed because they are constant: \n";
            for(int k=0;k<const_indices.length();k++){
                int i = const_indices[k];
                StatsCollector stat = stats[i];
                s+=" ("+source->fieldName(i)+","+tostring(stat.min())+")";
            }
            NORMAL_LOG<<s<<endl;
        }
        indices.resize(final_indices.length());
        indices << final_indices; 
    }else if (max_constant_threshold > 0){
        for (int k = 0; k < indices.length(); k++) {
            int i = indices[k];
            int max_constant_absolute =
                int(round(max_constant_threshold * stats[i].nnonmissing()));
            map<real, StatsCollectorCounts>* counts = stats[i].getCounts();
            map<real, StatsCollectorCounts>::const_iterator it;
            bool ok = true;
            int n;
            for (it = counts->begin(); ok && it != counts->end(); it++) {
                n = int(round(it->second.n));
                if (n >= max_constant_absolute)
                    ok = false;
            }
            if (ok)
                final_indices.append(i);
            else if (warn_removed_var)
                PLWARNING("In VariableDeletionVMatrix::build_() var '%s'"
                          " is too constant. Value %f happen %d/%f",
                          source->fieldName(i).c_str(), it->first, 
                          n, stats[i].nnonmissing());
        }
        indices.resize(final_indices.length());
        indices << final_indices;
    }
    // Define sizes.
    inputsize_  = indices.length();
    targetsize_ = source->targetsize();
    weightsize_ = source->weightsize();
    extrasize_  = source->extrasize();

    // Add target, weight and extra columns.
    for (int i = 0; i < source->targetsize(); i++)
        indices.append(is + i);
    if (source->targetsize() > 0)
        is += source->targetsize();
    for (int i = 0; i < source->weightsize(); i++)
        indices.append(is + i);
    if (source->weightsize() > 0)
        is += source->weightsize();
    for (int i = 0; i < source->extrasize(); i++)
        indices.append(is + i);

    // We have modified the selected columns, so the parent class must be
    // re-built.
    inherited::build();

    if(!save_deleted_columns.empty()){
        if(isfile(save_deleted_columns)){
            TVec<int> indices2;
            PLearn::load(save_deleted_columns, indices2);
            if(indices!=indices2){
                PLWARNING("In VariableDeletionVMatrix::build_() - the calculated"
                          " indices(%d) differ from the saved indices(%d) in file '%s'."
                          " We overwrite it.",
                        indices2.length(), indices.length(), save_deleted_columns.c_str());
                PLearn::save(save_deleted_columns,indices);
            }
        }else{
            PLearn::save(save_deleted_columns,indices);
        }
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
