// -*- C++ -*-

// GaussianizeVMatrix.cc
//
// Copyright (C) 2006 Olivier Delalleau
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

/*! \file GaussianizeVMatrix.cc */


#include "GaussianizeVMatrix.h"
#include <plearn/math/pl_erf.h>
#include "VMat_computeStats.h"
#include <plearn/io/load_and_save.h>
#include <plearn/io/fileutils.h>
#include <plearn/base/RemoteDeclareMethod.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    GaussianizeVMatrix,
    "Transforms its source VMatrix so that its features look Gaussian.",

    "This VMat transforms the features of its source that are obviously non-\n"
    "Gaussian, i.e. when the difference between the maximum and minimum\n"
    "value is too large compared to the standard deviation (the meaning of\n"
    "'too large' being controlled by the 'threshold_ratio' option).\n"
    "\n"
    "When this happens, the values of a features are sorted and their rank\n"
    "is used to transform them through the inverse cumulative of a normal\n"
    "Gaussian, resulting on a distribution that actually looks Gaussian.\n"
    "Note that, unless specified otherwise through the options, only the\n"
    "input features are transformed.\n"
    "\n"
    "It is important to note that only unique values are considered when\n"
    "computing the mapping, so that there is no 'hole' in the resulting\n"
    "distribution. This means the transformation learnt does not depend on\n"
    "the number of occurences of a specific value, but only on the ordering\n"
    "of the unique values encountered. The 'uniqueness' is defined by the\n"
    "PLearn 'is_equal' function, used to approximately compare real numbers.\n"
    "\n"
    "An additional 'train_source' VMat can also be specified in order to\n"
    "transform new data (in the 'source' option) while the transformation\n"
    "parameters are learned on a fixed 'train_source' VMat (e.g. when new\n"
    "test data are obtained and need to be properly Gaussianized).\n"
);

////////////////////////
// GaussianizeVMatrix //
////////////////////////
GaussianizeVMatrix::GaussianizeVMatrix():
    gaussianize_input(true),
    gaussianize_target(false),
    gaussianize_weight(false),
    gaussianize_extra(false),
    gaussianize_binary(false),
    threshold_ratio(10),
    save_and_reuse_stats(true)
{}

////////////////////
// declareOptions //
////////////////////
void GaussianizeVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "threshold_ratio", &GaussianizeVMatrix::threshold_ratio,
                                         OptionBase::buildoption,
        "A source's feature will be Gaussianized when the following holds:\n"
        "(max - min) / stddev > threshold_ratio.");

    declareOption(ol, "gaussianize_input",
                  &GaussianizeVMatrix::gaussianize_input,
                  OptionBase::buildoption,
        "Whether or not to Gaussianize the input part.");

    declareOption(ol, "gaussianize_target",
                  &GaussianizeVMatrix::gaussianize_target,
                  OptionBase::buildoption,
        "Whether or not to Gaussianize the target part.");

    declareOption(ol, "gaussianize_weight",
                  &GaussianizeVMatrix::gaussianize_weight,
                  OptionBase::buildoption,
        "Whether or not to Gaussianize the weight part.");

    declareOption(ol, "gaussianize_extra",
                  &GaussianizeVMatrix::gaussianize_extra,
                  OptionBase::buildoption,
        "Whether or not to Gaussianize the extra part.");

    declareOption(ol, "save_and_reuse_stats",
                  &GaussianizeVMatrix::save_and_reuse_stats,
                  OptionBase::buildoption,
        "If true, will save and reuse the stats of the source.");

    declareOption(ol, "gaussianize_binary",
                  &GaussianizeVMatrix::gaussianize_binary,
                  OptionBase::buildoption,
        "Whether or not to Gaussianize binary variable.");

    declareOption(ol, "train_source", &GaussianizeVMatrix::train_source,
                                      OptionBase::buildoption,
        "An optional VMat that will be used instead of 'source' to compute\n"
        "the transformation parameters from the distribution statistics.");

    declareOption(ol, "fields_to_gaussianize",
                  &GaussianizeVMatrix::fields_to_gaussianize,
                  OptionBase::buildoption,
                  "The fields that we want to be gaussianized.");

    declareOption(ol, "stats_file_to_use",
                  &GaussianizeVMatrix::stats_file_to_use,
                  OptionBase::buildoption,
                  "The filename of the statistics to use instead of the"
                  " train_source.");

    declareOption(ol, "save_fields_gaussianized",
                  &GaussianizeVMatrix::save_fields_gaussianized,
                  OptionBase::buildoption,
                  "A path where we will save the fields selected to be gaussianized.");

    declareOption(ol, "features_to_gaussianize",
                  &GaussianizeVMatrix::features_to_gaussianize,
                  OptionBase::learntoption,
                  "The columsn that will be gaussianized.");

    declareOption(ol, "values",
                  &GaussianizeVMatrix::values,
                  OptionBase::learntoption|OptionBase::nosave,
                  "The values used to gaussinaze.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void GaussianizeVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void GaussianizeVMatrix::build_()
{
    if (!source)
        return;

    if (train_source) {
        source->compatibleSizeError(train_source,
                                    "In GaussianizeVMatrix::build_ -"
                                    " The source and the train_source"
                                    " option are not compatible.");
    }

    VMat the_source = train_source ? train_source : source;

    PLCHECK( the_source->inputsize() >= 0 && the_source->targetsize() >= 0 &&
            the_source->weightsize() >= 0 && the_source->extrasize() >= 0 );

    // We set the mtime to remove the warning of Mtime=0
    if(train_source)
        updateMtime(train_source);
    updateMtime(source);

    // Find which dimensions to Gaussianize.
    features_to_gaussianize.resize(0);
    int col = 0;
    if (gaussianize_input)
        features_to_gaussianize.append(
                TVec<int>(col, col + the_source->inputsize() - 1, 1));
    col += the_source->inputsize();
    if (gaussianize_target)
        features_to_gaussianize.append(
                TVec<int>(col, col + the_source->targetsize() - 1, 1));
    col += the_source->targetsize();
    if (gaussianize_weight)
        features_to_gaussianize.append(
                TVec<int>(col, col + the_source->weightsize() - 1, 1));
    col += the_source->weightsize();
    if (gaussianize_extra)
        features_to_gaussianize.append(
                TVec<int>(col, col + the_source->extrasize() - 1, 1));
    col += the_source->extrasize();

    // Obtain meta information from source.
    setMetaInfoFromSource();
    if((hasMetaDataDir()||!stats_file_to_use.empty()) && values.size()==0)
        setMetaDataDir(getMetaDataDir());
}

///////////////////////////////
// append_col_to_gaussianize //
///////////////////////////////
void GaussianizeVMatrix::append_col_to_gaussianize(int col, StatsCollector stat){
    values.append(Vec());
    Vec& values_j = values.lastElement();
    features_to_gaussianize.append(col);
    map<real, StatsCollectorCounts>::const_iterator it, it_dummy;
    // Note that we obtain the approximate counts, so that almost equal
    // values have been merged together already.
    map<real,StatsCollectorCounts>* count_map =
        stat.getApproximateCounts();
    values_j.resize(0,count_map->size());
    // We use a dummy iterator to get rid of the last element in the
    // map, which is the max real value.
    it_dummy = count_map->begin();
    it_dummy++;
    for (it = count_map->begin(); it_dummy != count_map->end();
         it++, it_dummy++)
    {
        values_j.append(it->first);
    }
}

////////////////////
// setMetaDataDir //
////////////////////
void GaussianizeVMatrix::setMetaDataDir(const PPath& the_metadatadir){

    if(!the_metadatadir.empty())
        inherited::setMetaDataDir(the_metadatadir);

    if(features_to_gaussianize.size()==0)
        return;

    VMat the_source = train_source ? train_source : source;
    
    if(!the_source->hasMetaDataDir() && stats_file_to_use.empty() )
        PLERROR("In GaussianizeVMatrix::setMetaDataDir() - the "
                " train_source, source or this VMatrix should have a metadata directory!");

    //to save the stats their must be a metadatadir
    if(!the_source->hasMetaDataDir() && hasMetaDataDir()){
        if (train_source)
            the_source->setMetaDataDir(getMetaDataDir()+"train_source");
        else
            the_source->setMetaDataDir(getMetaDataDir()+"source");
    }

    TVec<StatsCollector> stats;
    if(!stats_file_to_use.empty()){
        if(!isfile(stats_file_to_use))
            PLERROR("In GaussianizeVMatrix::setMetaDataDir() - "
                    "stats_file_to_use = '%s' is not a file.",
                    stats_file_to_use.c_str());
         PLearn::load(stats_file_to_use, stats);
    } else if(save_and_reuse_stats)
        stats = the_source->
            getPrecomputedStatsFromFile("stats_gaussianizeVMatrix.psave", -1, true);
    else
        stats = PLearn::computeStats(the_source, -1, true);

    if(fields_to_gaussianize.size()>0){
        for(int i=0;i<fields_to_gaussianize.size();i++){
            int field=fields_to_gaussianize[i];
            if(field>=width() || field<0)
                PLERROR("In GaussianizeVMatrix::setMetaDataDir() - "
                        "bad fields number to gaussianize(%d)!",field);
        }
        features_to_gaussianize.resize(0,fields_to_gaussianize.length());

        values.resize(0);
        int last_j=-1;
        for (int i = 0; i < fields_to_gaussianize.length(); i++) {
            int j = fields_to_gaussianize[i];
            StatsCollector& stat = stats[j];
            if(last_j+1!=j)
                for(int k=last_j+1;k<j;k++){
                    //to keep the total memory used lower faster.
                    stats[k].forget();
                }
            append_col_to_gaussianize(j,stat);
            stats[j].forget();//to keep the total memory used lower.
        }
    }else{

        // See which dimensions violate the Gaussian assumption and will be
        // actually Gaussianized, and store the corresponding list of values.
        TVec<int> candidates = features_to_gaussianize.copy();
        features_to_gaussianize.resize(0);
        values.resize(0);
        for (int i = 0; i < candidates.length(); i++) {
            int j = candidates[i];
            StatsCollector& stat = stats[j];
            if (fast_exact_is_equal(stat.stddev(), 0)){
                //we don't gaussianize
            }else if (!gaussianize_binary && stat.isbinary()) {
                //we don't gaussianize
            }else if ((stat.max() - stat.min()) > threshold_ratio * stat.stddev()) {
                append_col_to_gaussianize(j,stat);
            }

            stats[j].forget();//to keep the total memory used lower.
        }
    }

    fields_gaussianized.resize(width());
    fields_gaussianized.fill(-1);
    for(int i=0;i<features_to_gaussianize.size();i++)
        fields_gaussianized[features_to_gaussianize[i]]=i;
    if(!save_fields_gaussianized.empty()){
        PLearn::save(save_fields_gaussianized,features_to_gaussianize);
    }
    if(features_to_gaussianize.size()==0)
        PLWARNING("GaussianizeVMatrix::build_() 0 variable was gaussianized");
}

///////////////
// getNewRow //
///////////////
void GaussianizeVMatrix::getNewRow(int i, const Vec& v) const
{
    PLASSERT( source );
    source->getRow(i, v);
    for (int k = 0; k < features_to_gaussianize.length(); k++) {
        int j = features_to_gaussianize[k];
        real current_val = v[j];
        if (is_missing(current_val))
            continue;
        // Find closest values in the training data.
        Vec& values_j = values[k];
        real interpol;
        if (current_val < values_j[0]) {
            // Smaller than the minimum.
            interpol = 0;
        } else if (current_val > values_j.lastElement()) {
            // Higher than the maximum.
            interpol = 1;
        } else {
            int min = 0;
            int max = values_j.length() - 1;
            while (max - min > 1) {
                int mid = (max + min) / 2;
                real mid_val = values_j[mid];
                if (current_val < mid_val)
                    max = mid;
                else if (current_val > mid_val)
                    min = mid;
                else {
                    // Found the exact value.
                    min = max = mid;
                }
            }
            if (min == max)
                interpol = min;
            else {
                PLASSERT( max - min == 1 );
                interpol = (current_val - values_j[min]) /
                          (values_j[max] - values_j[min]) + min;
                PLASSERT( !is_missing(interpol) );
            }
        }
        interpol /= (values_j.length() - 1);
        PLASSERT( interpol >= 0 && interpol <= 1 );
        // The expectation of the minimum and maximum of n numbers taken from a
        // uniform(0,1) distribution are respectively 1/n+1 and n/n+1: we shift
        // and rescale 'interpol' to be in [1/n+1, n/n+1] before using the
        // inverse of the Gaussian cumulative function.
        real n = values_j.length();
        interpol = (n - 1) / (n + 1) * interpol + 1 / (n + 1);
        v[j] = fast_gauss_01_quantile(interpol);
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GaussianizeVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(train_source, copies);
    //features_to_gaussianize?
    //scaling_factor?
    //values?
}


/////////////
// unGauss //
/////////////
real GaussianizeVMatrix::unGauss(real input, int j) const
{
    int k=fields_gaussianized[j];
    if(k<0)
        return input;//was not gaussianized
    
    real interpol = gauss_01_cum(input);
    Vec& values_j = values[k];
    int idx=int(interpol*values_j.length());
    return values_j[idx];
}

/////////////
// unGauss //
/////////////
void GaussianizeVMatrix::unGauss(Vec& inputs, Vec& ret, int j) const
{
    int k=fields_gaussianized[j];
    if(k<0)
        ret<<inputs;//was not gaussianized
    
    for(int i=0;i<inputs.size();i++){
        real value = inputs[i];
        real interpol = gauss_01_cum(value);
        Vec& values_j = values[k];
        int idx=int(interpol*values_j.length());
        ret[i]=values_j[idx];
    }
   
}

//! Version of unGauss(vec,vec,int) that's called by RMI
real GaussianizeVMatrix::remote_unGauss(real value, int col) const
{
    return unGauss(value,col);
}

//! Version of unGauss(vec,vec,int) that's called by RMI
Vec GaussianizeVMatrix::remote_unGauss_vec(Vec values, int col) const
{
    Vec outputs(values.length());
    unGauss(values,outputs,col);
    return outputs;
}

////////////////////
// declareMethods //
////////////////////
void GaussianizeVMatrix::declareMethods(RemoteMethodMap& rmm)
{
    // Insert a backpointer to remote methods; note that this is different from
    // declareOptions().
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(
        rmm, "unGauss", &GaussianizeVMatrix::remote_unGauss,
        (BodyDoc("Revert the gaussinisation done."),
         ArgDoc ("value", "The value to revert."),
         ArgDoc ("j", "The column of the value.")));


    declareMethod(
        rmm, "unGauss2", &GaussianizeVMatrix::remote_unGauss_vec,
        (BodyDoc("Revert the gaussinisation done."),
         ArgDoc ("values", "A vector of values to revert."),
         ArgDoc ("j", "The column of the value.")));

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
