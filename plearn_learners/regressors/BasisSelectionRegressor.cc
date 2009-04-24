// -*- C++ -*-

// BasisSelectionRegressor.cc
//
// Copyright (C) 2006 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file BasisSelectionRegressor.cc */


#include "BasisSelectionRegressor.h"
#include <plearn/math/RealFunctionOfInputFeature.h>
#include <plearn/math/ShiftAndRescaleFeatureRealFunction.h>
#include <plearn/math/RealFunctionFromKernel.h>
#include <plearn/math/ConstantRealFunction.h>
#include <plearn/math/RealFunctionProduct.h>
#include <plearn/math/RealValueIndicatorFunction.h>
#include <plearn/math/RealRangeIndicatorFunction.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn/math/random.h>
#include <plearn/vmat/RealFunctionsProcessedVMatrix.h>

#include <boost/thread.hpp>


namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    BasisSelectionRegressor,
    "This learner is able to incrementally select a basis of functions to be used to produce processed features fed to the underlying learner",
    "Functions are chosen among a dictionary of functions specified through the options.\n"
    "At each stage, the next function to append to the basis is chosen to be the one most colinear with the residue.\n"
    "This learner can be used to perform simple feature selection.\n"
    "The underlying learner is typically a linear regressor (this linear case might get \n"
    "implemented directly in this learner in future versions to skip unnecessary recomputation).");

BasisSelectionRegressor::BasisSelectionRegressor()
    : consider_constant_function(false),
      consider_raw_inputs(true),
      consider_normalized_inputs(false),
      consider_input_range_indicators(false),
      fixed_min_range(false),
      indicator_desired_prob(0.05),
      indicator_min_prob(0.01),
      n_kernel_centers_to_pick(-1),
      consider_interaction_terms(false),
      max_interaction_terms(-1),
      consider_n_best_for_interaction(-1),
      interaction_max_order(-1),
      consider_sorted_encodings(false),
      max_n_vals_for_sorted_encodings(-1),
      normalize_features(false),
      precompute_features(true),
      n_threads(0),
      thread_subtrain_length(0),
      use_all_basis(false),
      residue_sum(0),
      residue_sum_sq(0)
{}

void BasisSelectionRegressor::declareOptions(OptionList& ol)
{
    //#####  Public Build Options  ############################################

    declareOption(ol, "consider_constant_function", &BasisSelectionRegressor::consider_constant_function,
                  OptionBase::buildoption,
                  "If true, the constant function is included in the dictionary");

    declareOption(ol, "explicit_functions", &BasisSelectionRegressor::explicit_functions,
                  OptionBase::buildoption,
                  "This (optional) list of explicitly given RealFunctions\n"
                  "will get included in the dictionary");

    declareOption(ol, "explicit_interaction_variables", &BasisSelectionRegressor::explicit_interaction_variables,
                  OptionBase::buildoption,
                  "This (optional) list of explicitly given variables (fieldnames)\n"
                  "will get included in the dictionary for interaction terms ONLY\n"
                  "(i.e. these interact with the other functions.)");

    declareOption(ol, "mandatory_functions", &BasisSelectionRegressor::mandatory_functions,
                  OptionBase::buildoption,
                  "This (optional) list of explicitly given RealFunctions\n"
                  "will be automatically selected.");

    declareOption(ol, "consider_raw_inputs", &BasisSelectionRegressor::consider_raw_inputs,
                  OptionBase::buildoption,
                  "If true, then functions which select one of the raw inputs\n" 
                  "will be included in the dictionary."
                  "Beware that missing values (NaN) will be left as such.");

    declareOption(ol, "consider_normalized_inputs", &BasisSelectionRegressor::consider_normalized_inputs,
                  OptionBase::buildoption,
                  "If true, then functions which select and normalize inputs\n" 
                  "will be included in the dictionary. \n"
                  "Missing values will be replaced by 0 (i.e. the mean of normalized input)\n"
                  "Inputs which have nearly zero variance will be ignored.\n");

    declareOption(ol, "consider_input_range_indicators", &BasisSelectionRegressor::consider_input_range_indicators,
                  OptionBase::buildoption,
                  "If true, then we'll include in the dictionary indicator functions\n"
                  "triggered by input ranges and input special values\n"
                  "Special values will include all symbolic values\n"
                  "(detected by the existance of a corresponding string mapping)\n"
                  "as well as MISSING_VALUE (nan) (if it's present more than \n"
                  "indicator_min_prob fraction of the training set).\n"
                  "Real ranges will be formed in accordance to indicator_desired_prob \n"
                  "and indicator_min_prob options. The necessary statistics are obtained\n"
                  "from the counts in the StatsCollector of the train_set VMatrix.\n");

    declareOption(ol, "fixed_min_range", &BasisSelectionRegressor::fixed_min_range,
                  OptionBase::buildoption,
                  "If true, the min value of all range functions will be set to -FLT_MAX.\n"
                  "This correspond to a 'thermometer' type of mapping.");

    declareOption(ol, "indicator_desired_prob", &BasisSelectionRegressor::indicator_desired_prob,
                  OptionBase::buildoption,
                  "The algo will try to build input ranges that have at least that probability of occurence in the training set.");

    declareOption(ol, "indicator_min_prob", &BasisSelectionRegressor::indicator_min_prob,
                  OptionBase::buildoption,
                  "This will be used instead of indicator_desired_prob, for missing values, \n"
                  "and ranges immediately followed by a symbolic value");

    declareOption(ol, "kernels", &BasisSelectionRegressor::kernels,
                  OptionBase::buildoption,
                  "If given then each of these kernels, centered on each of the kernel_centers \n"
                  "will be included in the dictionary");

    declareOption(ol, "kernel_centers", &BasisSelectionRegressor::kernel_centers,
                  OptionBase::buildoption,
                  "If you specified a non empty kernels, you can give a matrix of explicit \n"
                  "centers here. Alternatively you can specify n_kernel_centers_to_pick.\n");

    declareOption(ol, "n_kernel_centers_to_pick", &BasisSelectionRegressor::n_kernel_centers_to_pick,
                  OptionBase::buildoption,
                  "If >0 then kernel_centers will be generated by randomly picking \n"
                  "n_kernel_centers_to_pick data points from the training set \n"
                  "(don't forget to set the seed option)");

    declareOption(ol, "consider_interaction_terms", &BasisSelectionRegressor::consider_interaction_terms,
                  OptionBase::buildoption,
                  "If true, the dictionary will be enriched, at each stage, by the product of\n"
                  "each of the already chosen basis functions with each of the dictionary functions\n");

    declareOption(ol, "max_interaction_terms", &BasisSelectionRegressor::max_interaction_terms,
                  OptionBase::buildoption,
                  "Maximum number of interaction terms to consider.  -1 means no max.\n"
                  "If more terms are possible, some are chosen randomly at each stage.\n");

    declareOption(ol, "consider_n_best_for_interaction", &BasisSelectionRegressor::consider_n_best_for_interaction,
                  OptionBase::buildoption,
                  "Only the top best functions of single variables are considered when building interaction terms.  -1 means no max.\n");

    declareOption(ol, "interaction_max_order", &BasisSelectionRegressor::interaction_max_order,
                  OptionBase::buildoption,
                  "Maximum order of a feature in an interaction function.  -1 means no max.\n");

    declareOption(ol, "consider_sorted_encodings", &BasisSelectionRegressor::consider_sorted_encodings,
                  OptionBase::buildoption,
                  "If true, the dictionary will be enriched with encodings sorted in target order.\n"
                  "This will be done for all fields with less than max_n_vals_for_sorted_encodings different values.\n");

    declareOption(ol, "max_n_vals_for_sorted_encodings", &BasisSelectionRegressor::max_n_vals_for_sorted_encodings,
                  OptionBase::buildoption,
                  "Maximum number of different values for a field to be considered for a sorted encoding.\n");

    declareOption(ol, "normalize_features", &BasisSelectionRegressor::normalize_features,
                  OptionBase::buildoption,
                  "EXPERIMENTAL OPTION (under development)");

    declareOption(ol, "learner", &BasisSelectionRegressor::template_learner,
                  OptionBase::buildoption,
                  "The underlying template learner.");

    declareOption(ol, "precompute_features", &BasisSelectionRegressor::precompute_features,
                  OptionBase::buildoption,
                  "True if features mat should be kept in memory; false if each row should be recalculated every time it is needed.");

    declareOption(ol, "n_threads", &BasisSelectionRegressor::n_threads,
                  OptionBase::buildoption,
                  "The number of threads to use when computing residue scores.\n"
                  "NOTE: MOST OF PLEARN IS NOT THREAD-SAFE; THIS CODE ASSUMES THAT SOME PARTS ARE, BUT THESE MAY CHANGE.");

    declareOption(ol, "thread_subtrain_length", &BasisSelectionRegressor::thread_subtrain_length,
                  OptionBase::buildoption,
                  "Preload thread_subtrain_length data when using multi-threading.");

    declareOption(ol, "use_all_basis", &BasisSelectionRegressor::use_all_basis,
                  OptionBase::buildoption,
                  "If true, we use the underlying learner on all basis functions generated by the BSR.\n"
                  "In this special way, all interaction terms are shut down and only 1 stage of training is necessary");

    //#####  Public Learnt Options  ############################################

    declareOption(ol, "selected_functions", &BasisSelectionRegressor::selected_functions,
                  OptionBase::learntoption,
                  "The list of real functions selected by the incremental algorithm.");

    declareOption(ol, "alphas", &BasisSelectionRegressor::alphas,
                  OptionBase::learntoption,
                  "CURRENTLY UNUSED");

    declareOption(ol, "scores", &BasisSelectionRegressor::scores,
                  OptionBase::learntoption,
                  "Matrice of the scores for each candidate function.\n"
                  "Used only when 'consider_n_best_for_interaction' > 0.");

    declareOption(ol, "candidate_functions", &BasisSelectionRegressor::candidate_functions,
                  OptionBase::learntoption,
                  "The list of current candidate functions.");


    declareOption(ol, "explicit_interaction_functions", &BasisSelectionRegressor::explicit_interaction_functions,
                  OptionBase::learntoption,
                  "This (optional) list of explicitly given RealFunctions\n"
                  "will get included in the dictionary for interaction terms ONLY\n"
                  "(i.e. these interact with the other functions.)");

    declareOption(ol, "true_learner", &BasisSelectionRegressor::learner,
                  OptionBase::learntoption,
                  "The underlying learner to be trained with the extracted features.");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void BasisSelectionRegressor::build_()
{
    if (use_all_basis)
    {
        PLASSERT_MSG(nstages == 1, "\"nstages\" must be 1 when \"use_all_basis\" is true");
        PLASSERT_MSG(!consider_interaction_terms, "\"consider_interaction_terms\" must be false when \"use_all_basis\" is true");
    }
}


void BasisSelectionRegressor::setExperimentDirectory(const PPath& the_expdir)
{ 
    inherited::setExperimentDirectory(the_expdir);
    template_learner->setExperimentDirectory(the_expdir / "SubLearner");
}


// ### Nothing to add here, simply calls build_
void BasisSelectionRegressor::build()
{
    inherited::build();
    build_();
}


void BasisSelectionRegressor::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(explicit_functions, copies);
    deepCopyField(explicit_interaction_functions, copies);
    deepCopyField(explicit_interaction_variables, copies);
    deepCopyField(mandatory_functions, copies);
    deepCopyField(kernels, copies);
    deepCopyField(kernel_centers, copies);
    deepCopyField(learner, copies);
    deepCopyField(template_learner, copies);
    deepCopyField(selected_functions, copies);
    deepCopyField(alphas, copies);
    deepCopyField(scores, copies);

    deepCopyField(simple_candidate_functions, copies);
    deepCopyField(candidate_functions, copies);
    deepCopyField(features, copies);
    deepCopyField(residue, copies);
    deepCopyField(targets, copies);
    deepCopyField(weights, copies);

    deepCopyField(input, copies);
    deepCopyField(targ, copies);
    deepCopyField(featurevec, copies);
}


int BasisSelectionRegressor::outputsize() const
{
    //return 1;
    return template_learner->outputsize();
}

void BasisSelectionRegressor::forget()
{
    selected_functions.resize(0);
    targets.resize(0);
    residue.resize(0);
    weights.resize(0);
    features.resize(0,0);
    if(n_kernel_centers_to_pick>=0)
        kernel_centers.resize(0,0);
    if(learner.isNotNull())
        learner->forget();

    candidate_functions.resize(0);

    stage = 0;
}

void BasisSelectionRegressor::appendCandidateFunctionsOfSingleField(int fieldnum, TVec<RealFunc>& functions) const
{
    VMField field_info = train_set->getFieldInfos(fieldnum);
    string fieldname = field_info.name;
    VMField::FieldType fieldtype = field_info.fieldtype;
    StatsCollector& stats_collector = train_set->getStats(fieldnum);

    real n           = stats_collector.n();
    real nmissing    = stats_collector.nmissing();
    real nnonmissing = stats_collector.nnonmissing();
    real min_count     = indicator_min_prob * n;
    real desired_count = indicator_desired_prob * n;

    // Raw inputs for non-discrete variables
    if (consider_raw_inputs  &&  (fieldtype != VMField::DiscrGeneral))
    {
        RealFunc f = new RealFunctionOfInputFeature(fieldnum);
        f->setInfo(fieldname);
        functions.append(f);
    }

    // Normalized inputs for non-discrete variables
    if (consider_normalized_inputs  &&  (fieldtype != VMField::DiscrGeneral))
    {
        if (nnonmissing > 0)
        {
            real mean   = stats_collector.mean();
            real stddev = stats_collector.stddev();
            if (stddev > 1e-9)
            {
                RealFunc f = new ShiftAndRescaleFeatureRealFunction(fieldnum, -mean, 1./stddev, 0.);
                string info = fieldname + "-" + tostring(mean) + "/" + tostring(stddev);
                f->setInfo(info);
                functions.append(f);
            }
        }

    }

    if (consider_input_range_indicators)
    {
        const map<real,string>& smap = train_set->getRealToStringMapping(fieldnum);
        map<real,string>::const_iterator smap_it  = smap.begin();
        map<real,string>::const_iterator smap_end = smap.end();

        map<real, StatsCollectorCounts>* counts = stats_collector.getApproximateCounts();
        map<real,StatsCollectorCounts>::const_iterator count_it = counts->begin();
        map<real,StatsCollectorCounts>::const_iterator count_end = counts->end();

        // Indicator function for mapped variables
        while (smap_it != smap_end)
        {
            RealFunc f = new RealValueIndicatorFunction(fieldnum, smap_it->first);
            string info = fieldname + "=" + smap_it->second;
            f->setInfo(info);
            functions.append(f);
            ++smap_it;
        }

        // Indicator function for discrete variables not using mapping
        if (fieldtype == VMField::DiscrGeneral  ||  fieldtype == VMField::DiscrMonotonic)
        {
            while (count_it != count_end)
            {
                real val = count_it->first;
                // Make sure the variable don't use mapping for this particular value
                bool mapped_value = false;
                smap_it = smap.begin();
                while (smap_it != smap_end)
                {
                    if (smap_it->first == val)
                    {
                        mapped_value = true;
                        break;
                    }
                    ++smap_it;
                }

                if (!mapped_value)
                {
                    RealFunc f = new RealValueIndicatorFunction(fieldnum, val);
                    string info = fieldname + "=" + tostring(val);
                    f->setInfo(info);
                    functions.append(f);
                }
                ++count_it;
            }
        }

        // If enough missing values, add an indicator function for it
        if (nmissing >= min_count && nnonmissing >= min_count)
        {
            RealFunc f = new RealValueIndicatorFunction(fieldnum, MISSING_VALUE);
            string info = fieldname + "=MISSING";
            f->setInfo(info);
            functions.append(f);
        }

        // For fieldtype DiscrGeneral, it stops here.
        // A RealRangeIndicatorFunction makes no sense for DiscrGeneral
        if (fieldtype == VMField::DiscrGeneral) return;
        
        real cum_count = 0;
        real low = -FLT_MAX;
        real val = FLT_MAX;
        count_it = counts->begin();
        while (count_it != count_end)
        {
            val = count_it->first;
            cum_count += count_it->second.nbelow;
            bool in_smap = (smap.find(val) != smap_end);
            if((cum_count>=desired_count || in_smap&&cum_count>=min_count) && (n-cum_count>=desired_count || in_smap&&n-cum_count>=min_count))
            {
                RealRange range(']',low,val,'[');
                if (fixed_min_range) range.low = -FLT_MAX;
                RealFunc f = new RealRangeIndicatorFunction(fieldnum, range);
                string info = fieldname + "__" + tostring(range);
                f->setInfo(info);
                functions.append(f);
                cum_count = 0;
                low = val;
            }

            cum_count += count_it->second.n;
            if (in_smap)
            {
                cum_count = 0;
                low = val;
            }
            else if (cum_count>=desired_count && n-cum_count>=desired_count)
            {
                RealRange range(']',low,val,']');
                if (fixed_min_range) range.low = -FLT_MAX;
                RealFunc f = new RealRangeIndicatorFunction(fieldnum, range);
                string info = fieldname + "__" + tostring(range);
                f->setInfo(info);
                functions.append(f);
                cum_count = 0;
                low = val;
            }            
            ++count_it;
        }
        // last chunk
        if (cum_count > 0)
        {
            if (cum_count>=min_count && n-cum_count>=min_count)
            {
                RealRange range(']',low,val,']');
                if (fixed_min_range) range.low = -FLT_MAX;
                RealFunc f = new RealRangeIndicatorFunction(fieldnum, range);
                string info = fieldname + "__" + tostring(range);
                f->setInfo(info);
                functions.append(f);
            }
            else if (functions.length()>0) // possibly lump it together with last range
            {
                RealRangeIndicatorFunction* f = (RealRangeIndicatorFunction*)(RealFunction*)functions.lastElement();
                RealRange& range = f->range;
                if(smap.find(range.high) != smap_end) // last element does not appear to be symbolic
                {                    
                    range.high = val; // OK, change the last range to include val
                    string info = fieldname + "__" + tostring(range);
                    f->setInfo(info);
                }
            }
        }
    }
}

void BasisSelectionRegressor::appendKernelFunctions(TVec<RealFunc>& functions) const
{
    if(kernel_centers.length()<=0 && n_kernel_centers_to_pick>=0)
    {
        int nc = n_kernel_centers_to_pick;
        kernel_centers.resize(nc, inputsize());
        real weight;
        int l = train_set->length();
        if(random_gen.isNull())
            random_gen = new PRandom();
        random_gen->manual_seed(seed_);
        for(int i=0; i<nc; i++)
        {
            Vec input = kernel_centers(i);
            int rowpos = min(int(l*random_gen->uniform_sample()),l-1);
            train_set->getExample(rowpos, input, targ, weight);
        }
    }

    for(int i=0; i<kernel_centers.length(); i++)
    {
        Vec center = kernel_centers(i);
        for(int k=0; k<kernels.length(); k++)
            functions.append(new RealFunctionFromKernel(kernels[k],center));
    }
}

void BasisSelectionRegressor::appendConstantFunction(TVec<RealFunc>& functions) const
{
    functions.append(new ConstantRealFunction());
}

void BasisSelectionRegressor::buildSimpleCandidateFunctions()
{
    if(consider_constant_function)
        appendConstantFunction(simple_candidate_functions);
    
    if(explicit_functions.length()>0)
        simple_candidate_functions.append(explicit_functions);

    for(int fieldnum=0; fieldnum<inputsize(); fieldnum++)
        appendCandidateFunctionsOfSingleField(fieldnum, simple_candidate_functions);
    
    if(kernels.length()>0)
        appendKernelFunctions(simple_candidate_functions);
}

void BasisSelectionRegressor::buildAllCandidateFunctions()
{
    if(selected_functions.length()==0)
    {
        candidate_functions= mandatory_functions.copy();
        while(candidate_functions.length() > 0)
            appendFunctionToSelection(0);
    }

    if(simple_candidate_functions.length()==0)
        buildSimpleCandidateFunctions();

    candidate_functions = simple_candidate_functions.copy();
    TVec<RealFunc> interaction_candidate_functions;

    int candidate_start = consider_constant_function ? 1 : 0; // skip bias
    int ncandidates = candidate_functions.length();
    int nselected = selected_functions.length();
    if (nselected > 0  &&  consider_interaction_terms)
    {
        TVec<RealFunc> top_candidate_functions = simple_candidate_functions.copy();
        int start = candidate_start;
        if (consider_n_best_for_interaction > 0  &&  ncandidates > consider_n_best_for_interaction)
        {
            top_candidate_functions = buildTopCandidateFunctions();
            start = 0;
        }

        for (int k=0; k<nselected; k++)
        {
            for (int j=start; j<top_candidate_functions.length(); j++)
            {
                addInteractionFunction(selected_functions[k], top_candidate_functions[j], interaction_candidate_functions);
            }
        }
    }

    // Build explicit_interaction_functions from explicit_interaction_variables
    explicit_interaction_functions.resize(0);
    for(int k=0; k<explicit_interaction_variables.length(); ++k)
        appendCandidateFunctionsOfSingleField(train_set->getFieldIndex(explicit_interaction_variables[k]), explicit_interaction_functions);

    // Add interaction_candidate_functions from explicit_interaction_functions
    for(int k= 0; k < explicit_interaction_functions.length(); ++k)
    {
        for(int j=candidate_start; j<ncandidates; ++j)
        {
            addInteractionFunction(explicit_interaction_functions[k], simple_candidate_functions[j], interaction_candidate_functions);
        }
    }

    // If too many interaction_candidate_functions, we choose them at random
    if(max_interaction_terms > 0  &&  interaction_candidate_functions.length() > max_interaction_terms)
    {
        shuffleElements(interaction_candidate_functions);
        interaction_candidate_functions.resize(max_interaction_terms);
    }
    candidate_functions.append(interaction_candidate_functions);

    // If use_all_basis, append all candidate_functions to selected_functions
    if (use_all_basis)
    {
        while (candidate_functions.length() > 0)
            appendFunctionToSelection(0);
    }
}

void BasisSelectionRegressor::addInteractionFunction(RealFunc& f1, RealFunc& f2, TVec<RealFunc>& all_functions)
{
    // Check that feature in f2 don't exceed "interaction_max_order" of that feature in f1
    // Note that f2 should be a new function to be added (and thus an instance of RealFunctionOfInputFeature)
    if (interaction_max_order > 0)
    {
        int order = 0;
        computeOrder(f1, order);
        computeOrder(f2, order);
        if (order > interaction_max_order)
            return;
    }

    RealFunc f = new RealFunctionProduct(f1, f2);
    f->setInfo("(" + f1->getInfo() + "*" + f2->getInfo() + ")");
    all_functions.append(f);
}

void BasisSelectionRegressor::computeOrder(RealFunc& func, int& order)
{
    if (dynamic_cast<RealFunctionOfInputFeature*>((RealFunction*) func))
    {
        ++order;
    }
    else if (RealFunctionProduct* f = dynamic_cast<RealFunctionProduct*>((RealFunction*) func))
    {
        computeOrder(f->f1, order);
        computeOrder(f->f2, order);
    }
    else
        PLERROR("In BasisSelectionRegressor::computeOrder: bad function type.");
}

TVec<RealFunc> BasisSelectionRegressor::buildTopCandidateFunctions()
{
    // The scores matrix should match (in size) the candidate_functions matrix
    PLASSERT(scores.length() == candidate_functions.length());

    sortRows(scores, 1, false);
    TVec<RealFunc> top_best_functions;
    for (int i=0; i<consider_n_best_for_interaction; i++)
        top_best_functions.append(simple_candidate_functions[(int)scores(i,0)]);

    return top_best_functions;
}

/* Returns the index of the most correlated (or anti-correlated) feature 
among the full candidate features. 
*/
void BasisSelectionRegressor::findBestCandidateFunction(int& best_candidate_index, real& best_score) const
{
    int n_candidates = candidate_functions.size();
    Vec E_x;
    Vec E_xx;
    Vec E_xy;
    real wsum = 0;
    real E_y = 0;
    real E_yy = 0;

    computeWeightedAveragesWithResidue(candidate_functions, wsum, E_x, E_xx, E_y, E_yy, E_xy);
    
    scores.resize(simple_candidate_functions.length(), 2);

    if(verbosity>=5)
        perr << "n_candidates = " << n_candidates << endl;

    if(verbosity>=10)
        perr << "E_xy = " << E_xy << endl;
    best_candidate_index = -1;
    best_score = 0;

    for(int j=0; j<n_candidates; j++)
    {
        real score = 0;
        if(normalize_features)
            score = fabs((E_xy[j]-E_y*E_x[j])/(1e-6+sqrt(E_xx[j]-square(E_x[j]))));
        else
            score = fabs(E_xy[j]);
        if(verbosity>=10)
            perr << score << ' ';
        if(score>best_score)
        {
            best_candidate_index = j;
            best_score = score;
        }
        // we keep the score only for the simple_candidate_functions
        if (j < simple_candidate_functions.length())
        {
            scores(j, 0) = j;
            scores(j, 1) = score;
        }
    }

    if(verbosity>=10)
        perr << endl;
}


//function-object for a thread
struct BasisSelectionRegressor::thread_wawr
{
    int thread_id, n_threads;
    const TVec<RealFunc>& functions;
    real& wsum;
    Vec& E_x;
    Vec& E_xx;
    real& E_y;
    real& E_yy;
    Vec& E_xy;
    const Vec& Y;
    boost::mutex& ts_mx;
    const VMat& train_set;
    boost::mutex& pb_mx;
    PP<ProgressBar> pb;
    int thread_subtrain_length;

    thread_wawr(int thread_id_, int n_threads_,
                const TVec<RealFunc>& functions_,  
                real& wsum_,
                Vec& E_x_, Vec& E_xx_,
                real& E_y_, real& E_yy_,
                Vec& E_xy_, const Vec& Y_, boost::mutex& ts_mx_,
                const VMat& train_set_,
                boost::mutex& pb_mx_,
                PP<ProgressBar> pb_,
                int thread_subtrain_length_)
        : thread_id(thread_id_), 
          n_threads(n_threads_),
          functions(functions_),
          wsum(wsum_),
          E_x(E_x_),
          E_xx(E_xx_),
          E_y(E_y_),
          E_yy(E_yy_),
          E_xy(E_xy_),
          Y(Y_),
          ts_mx(ts_mx_),
          train_set(train_set_),
          pb_mx(pb_mx_),
          pb(pb_),
          thread_subtrain_length(thread_subtrain_length_)
    {}

    void operator()()
    {
        Vec input, targ;
        real w;
        Vec candidate_features;
        int n_candidates = functions.length();
        int train_len = train_set->length();
     
        E_x.resize(n_candidates);
        E_x.fill(0.);
        E_xx.resize(n_candidates);
        E_xx.fill(0.);
        E_y = 0.;
        E_yy = 0.;
        E_xy.resize(n_candidates);
        E_xy.fill(0.);
        wsum = 0.;

        // Used when thread_subtrain_length > 1
        Mat all_inputs;
        Vec all_w;
        int input_size = train_set->inputsize();
        if (thread_subtrain_length > 1)
        {
            // pre-allocate memory
            all_inputs.resize(thread_subtrain_length, input_size);
            all_w.resize(thread_subtrain_length);
        }
   
        for(int i=thread_id; i<train_len; i+= n_threads)
        {
            if (thread_subtrain_length > 1)
            {
                int j = (i-thread_id)/n_threads;
                int j_mod = j % thread_subtrain_length;
                if (j_mod == 0)  // on doit faire le plein de donnees
                {
                    all_inputs.resize(0, input_size);
                    all_w.resize(0);

                    boost::mutex::scoped_lock lock(ts_mx);
                    int max_train = min(train_len, i + thread_subtrain_length*n_threads);
                    for (int ii=i; ii<max_train; ii+= n_threads)
                    {
                        train_set->getExample(ii, input, targ, w);
                        all_inputs.appendRow(input);
                        all_w.append(w);
                    }
                }
                input = all_inputs(j_mod);
                w = all_w[j_mod];
            }
            else
            {
                boost::mutex::scoped_lock lock(ts_mx);
                train_set->getExample(i, input, targ, w);
            }
            evaluate_functions(functions, input, candidate_features);
            wsum += w;
            real y = Y[i];
            real wy = w*y;
            E_y  += wy;
            E_yy  += wy*y;
            for(int j=0; j<n_candidates; j++)
            {
                real x = candidate_features[j];
                real wx = w*x;
                E_x[j] += wx;
                E_xx[j] += wx*x;
                E_xy[j] += wx*y;
            }
            if(pb)
            {
                boost::mutex::scoped_lock lock(pb_mx);
                if(pb->currentpos < static_cast<unsigned int>(i))
                    pb->update(i);
            }
        }
    }
};


void BasisSelectionRegressor::computeWeightedAveragesWithResidue(const TVec<RealFunc>& functions,  
                                                                 real& wsum,
                                                                 Vec& E_x, Vec& E_xx,
                                                                 real& E_y, real& E_yy,
                                                                 Vec& E_xy) const
{
    const Vec& Y = residue;
    int n_candidates = functions.length();
    E_x.resize(n_candidates);
    E_x.fill(0.);
    E_xx.resize(n_candidates);
    E_xx.fill(0.);
    E_y = 0.;
    E_yy = 0.;
    E_xy.resize(n_candidates);
    E_xy.fill(0.);
    wsum = 0;

    Vec candidate_features;
    real w;
    int l = train_set->length();

    PP<ProgressBar> pb;
    if(report_progress)
        pb = new ProgressBar("Computing residue scores for " + tostring(n_candidates) + " candidate functions", l);

    if(n_threads > 0)
    {
        Vec wsums(n_threads);
        TVec<Vec> E_xs(n_threads);
        TVec<Vec> E_xxs(n_threads);
        Vec E_ys(n_threads);
        Vec E_yys(n_threads);
        TVec<Vec> E_xys(n_threads);
        boost::mutex ts_mx, pb_mx;
        TVec<boost::thread* > threads(n_threads);
        TVec<thread_wawr* > tws(n_threads);

        for(int i= 0; i < n_threads; ++i)
        {
            tws[i]= new thread_wawr(i, n_threads, functions, 
                                    wsums[i], 
                                    E_xs[i], E_xxs[i],
                                    E_ys[i], E_yys[i],
                                    E_xys[i], Y, ts_mx, train_set,
                                    pb_mx, pb, thread_subtrain_length);
            threads[i]= new boost::thread(*tws[i]);
        }
        for(int i= 0; i < n_threads; ++i)
        {
            threads[i]->join();
            wsum+= wsums[i];
            E_y+= E_ys[i];
            E_yy+= E_yys[i];
            for(int j= 0; j < n_candidates; ++j)
            {
                E_x[j]+= E_xs[i][j];
                E_xx[j]+= E_xxs[i][j];
                E_xy[j]+= E_xys[i][j];
            }
            delete threads[i];
            delete tws[i];
        }
    }
    else // single-thread version
    {
        for(int i=0; i<l; i++)
        {
            real y = Y[i];
            train_set->getExample(i, input, targ, w);
            evaluate_functions(functions, input, candidate_features);
            wsum += w;
            real wy = w*y;
            E_y  += wy;
            E_yy  += wy*y;
            for(int j=0; j<n_candidates; j++)
            {
                real x = candidate_features[j];
                real wx = w*x;
                E_x[j] += wx;
                E_xx[j] += wx*x;
                E_xy[j] += wx*y;
            }
            if(pb)
                pb->update(i);
        }
    }

    // Finalize computation
    real inv_wsum = 1.0/wsum;
    E_x  *= inv_wsum;
    E_xx *= inv_wsum;
    E_y  *= inv_wsum;
    E_yy *= inv_wsum;
    E_xy *= inv_wsum;

}


/*
void BasisSelectionRegressor::computeWeightedCorrelationsWithY(const TVec<RealFunc>& functions, const Vec& Y,  
                                                               real& wsum,
                                                               Vec& E_x, Vec& V_x,
                                                               real& E_y, real& V_y,
                                                               Vec& E_xy, Vec& V_xy,
                                                               Vec& covar, Vec& correl, real min_variance) const
{
    int n_candidates = functions.length();
    E_x.resize(n_candidates);
    E_x.fill(0.);
    V_x.resize(n_candidates);
    V_x.fill(0.);
    E_y = 0.;
    V_y = 0.;
    E_xy.resize(n_candidates);
    E_xy.fill(0.);
    V_xy.resize(n_candidates);
    V_xy.fill(0.);
    wsum = 0;

    Vec candidate_features;
    real w;
    int l = train_set->length();
    for(int i=0; i<l; i++)
    {
        real y = Y[i];
        train_set->getExample(i, input, targ, w);
        evaluate_functions(functions, input, candidate_features);
        wsum += w;
        E_y  += w*y;
        V_y  += w*y*y;
        for(int j=0; j<n_candidates; j++)
        {
            real x = candidate_features[j];
            E_x[j] += w*x;
            V_x[j] += w*x*x;
            real xy = x*y;
            E_xy[j] += w*xy;
            V_xy[j] += w*xy*xy;
        }
    }

    // Finalize computation
    real inv_wsum = 1.0/wsum;
    E_y *= inv_wsum;
    V_y  = V_y*inv_wsum - square(E_y);
    if(V_y<min_variance)
        V_y = min_variance;
    covar.resize(n_candidates);
    correl.resize(n_candidates);
    for(int j=0; j<n_candidates; j++)
    {
        real E_x_j = E_x[j]*inv_wsum;
        E_x[j] = E_x_j;
        real V_x_j = V_x[j]*inv_wsum - square(E_x_j);
        if(V_x_j<min_variance)
            V_x_j = min_variance;
        V_x[j] = V_x_j;
        real E_xy_j = E_xy[j]*inv_wsum;
        E_xy[j] = E_xy_j;
        real V_xy_j = V_xy[j]*inv_wsum - square(E_xy_j);
        V_xy[j] = V_xy_j;
        real covar_j = E_xy_j - square(E_x_j);
        real correl_j = covar_j/sqrt(V_x_j*V_y);
        covar[j] = covar_j;
        correl[j] = correl_j;
    }
}


//! Note that the returned statistics (except wsum) are invariant to arbitrary scaling of the weights
//! Note that the variances computed are the sample variances, which are not an unbiased estimator. 
void weighted_XY_statistics(const Vec& X, const Vec& Y, const Vec& W, 
                                  real& wsum,
                                  real& E_x, real& V_x,
                                  real& E_y, real& V_y,
                                  real& E_xy, real& V_xy,
                                  real& covar, real& correl)
{
    E_x = 0;
    V_x = 0;
    E_y = 0;
    V_y = 0;
    E_xy = 0;
    V_xy = 0;
    wsum = 0;

    const real* pX = X.data();
    const real* pY = Y.data();
    const real* pW = W.data();

    int l = X.length();
    while(l--)
    {
        real x = *pX++;
        real y = *pY++;
        real w = *pW++;
        wsum += w;
        E_x  += w*x;
        V_x  += w*x*x;
        E_y  += w*y;
        V_y  += w*y*y;
        real xy = x*y;
        E_xy += w*xy;
        V_xy += w*xy*xy;
    }
    real inv_wsum = 1.0/wsum;
    E_x  *= inv_wsum;
    V_x  = V_x*inv_wsum - E_x*E_x;
    E_y  *= inv_wsum;
    V_y  = V_y*inv_wsum - E_y*E_y;
    E_xy = E_xy*inv_wsum;
    V_xy = V_xy*inv_wsum - E_xy*E_xy;

    covar = E_xy - E_x*E_y;
    correl = covar/sqrt(V_x*V_y);
}
*/

void BasisSelectionRegressor::appendFunctionToSelection(int candidate_index)
{
    RealFunc f = candidate_functions[candidate_index];
    if(precompute_features)
    {
        int l = train_set->length();
        int nf = selected_functions.length();
        features.resize(l,nf+1, max(1,static_cast<int>(0.25*l*nf)),true);  // enlarge width while preserving content
        real weight;
        for(int i=0; i<l; i++)
        {
            train_set->getExample(i,input,targ,weight);
            features(i,nf) = f->evaluate(input);
        }
    }
    selected_functions.append(f);

    if(!consider_interaction_terms)
        candidate_functions.remove(candidate_index);
    else
        buildAllCandidateFunctions();
}

void BasisSelectionRegressor::retrainLearner()
{    
    int l  = train_set->length();
    int nf = selected_functions.length();
    bool weighted = train_set->hasWeights();

    // set dummy training set, so that undelying learner frees reference to previous training set
    /*
    VMat newtrainset = new MemoryVMatrix(1,nf+(weighted?2:1));
    newtrainset->defineSizes(nf,1,weighted?1:0);
    learner->setTrainingSet(newtrainset);
    learner->forget();
    */

    // Deep-copy the underlying learner
    CopiesMap copies;
    learner = template_learner->deepCopy(copies);
    PP<VecStatsCollector> statscol = template_learner->getTrainStatsCollector();
    learner->setTrainStatsCollector(statscol);
    PPath expdir = template_learner->getExperimentDirectory();
    learner->setExperimentDirectory(expdir);

    VMat newtrainset;
    if(precompute_features)
    {
        features.resize(l,nf+(weighted?2:1), max(1,int(0.25*l*nf)), true); // enlarge width while preserving content
        if(weighted)
        {
            for(int i=0; i<l; i++) // append target and weight columns to features matrix
            {
                features(i,nf) = targets[i];
                features(i,nf+1) = weights[i];
            }
        }
        else // no weights
            features.lastColumn() << targets; // append target column to features matrix
    
        newtrainset = new MemoryVMatrix(features);
    }
    else
        newtrainset= new RealFunctionsProcessedVMatrix(train_set, selected_functions, false, true, true);
    newtrainset->defineSizes(nf,1,weighted?1:0);
    learner->setTrainingSet(newtrainset);
    template_learner->setTrainingSet(newtrainset);
    learner->forget();
    learner->train();
    // resize features matrix so it contains only the features
    if(precompute_features)
        features.resize(l,nf);
}


void BasisSelectionRegressor::train()
{
    if(nstages > 0)
    {
        if (!initTrain())
            return;
    } // work around so that nstages can be zero...
    else if (!train_stats)
        train_stats = new VecStatsCollector();

    if(stage==0)
    {
        simple_candidate_functions.resize(0);
        buildAllCandidateFunctions();
    }

    while(stage<nstages)
    {
        if(targets.length()==0)
        {
            initTargetsResidueWeight();
            if(selected_functions.length()>0)
            {
                recomputeFeatures();
                if(stage==0) // only mandatory funcs.
                    retrainLearner();
                if (candidate_functions.length()>0)
                    recomputeResidue();
            }
        }

        if(candidate_functions.length()>0)
        {
            int best_candidate_index = -1;
            real best_score = 0;
            findBestCandidateFunction(best_candidate_index, best_score);
            if(verbosity>=2)
                perr << "\n\n*** Stage " << stage << " *****" << endl
                     << "Best candidate: index=" << best_candidate_index << endl
                     << "  score=" << best_score << endl;
            if(best_candidate_index>=0)
            {
                if(verbosity>=2)
                    perr << "  function info = " << candidate_functions[best_candidate_index]->getInfo() << endl;
                if(verbosity>=3)
                    perr << "  function= " << candidate_functions[best_candidate_index] << endl;
                appendFunctionToSelection(best_candidate_index);

                if(verbosity>=2)
                    perr << "residue_sum_sq before retrain: " << residue_sum_sq << endl;
                retrainLearner();
                recomputeResidue();
                if(verbosity>=2)
                    perr << "residue_sum_sq after retrain: " << residue_sum_sq << endl;
            }
        }
        else
        {
            if(verbosity>=2)
                perr << "\n\n*** Stage " << stage << " : no more candidate functions. *****" << endl;
        }
        ++stage;
    }
}

void BasisSelectionRegressor::initTargetsResidueWeight()
{
    int l = train_set.length();
    residue.resize(l);
    targets.resize(l);
    residue_sum = 0.;
    residue_sum_sq = 0.;
    weights.resize(l);

    real w;
    for(int i=0; i<l; i++)
    {
        train_set->getExample(i, input, targ, w);
        real t = targ[0];
        targets[i] = t;
        residue[i] = t;
        weights[i] = w;
        residue_sum += w*t;
        residue_sum_sq += w*square(t);
    }
}

void BasisSelectionRegressor::recomputeFeatures()
{
    if(!precompute_features)
        return;
    int l = train_set.length();
    int nf = selected_functions.length();
    features.resize(l,nf);
    real weight = 0;
    for(int i=0; i<l; i++)
    {
        train_set->getExample(i, input, targ, weight);
        Vec v = features(i);
        evaluate_functions(selected_functions, input, v);
    }    
}

void BasisSelectionRegressor::recomputeResidue()
{
    int l = train_set.length();
    residue.resize(l);
    residue_sum = 0;
    residue_sum_sq = 0;
    Vec output(outputsize());
    // perr << "recomp_residue: { ";
    for(int i=0; i<l; i++)
    {
        real t = targets[i];
        real w = weights[i];
        if(precompute_features)
            computeOutputFromFeaturevec(features(i),output);
        else
        {
            real wt;
            train_set->getExample(i,input,targ,wt);
            computeOutput(input,output);
        }

        real resid = t-output[0];
        residue[i] = resid;
        // perr << "feature " << i << ": " << features(i) << " t:" << t << " out: " << output[0] << " resid: " << residue[i] << endl;
        residue_sum += resid;
        residue_sum_sq += w*square(resid);
    }
    // perr << "}" << endl;
    // perr << "targets: \n" << targets << endl;
    // perr << "residue: \n" << residue << endl;
}

void BasisSelectionRegressor::computeOutputFromFeaturevec(const Vec& featurevec, Vec& output) const
{
    int nout = outputsize();
    if(nout!=1 && !use_all_basis)
        PLERROR("outputsize should always be 1 for this learner (=%d)", nout);
    output.resize(nout);

    if(learner.isNull())
        output[0] = dot(alphas, featurevec);
    else
        learner->computeOutput(featurevec, output);
}

void BasisSelectionRegressor::computeOutput(const Vec& input, Vec& output) const
{
    evaluate_functions(selected_functions, input, featurevec);
    computeOutputFromFeaturevec(featurevec, output);
}

void BasisSelectionRegressor::printModelFunction(PStream& out) const
{
    for(int k=0; k<selected_functions.length(); k++)
    {
        out << "+ " << alphas[k] << "* " << selected_functions[k];
        out << endl;
    }
}

void BasisSelectionRegressor::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& targ, Vec& costs) const
{
    costs.resize(1);
    costs[0] = square(output[0]-targ[0]);
}

TVec<string> BasisSelectionRegressor::getTestCostNames() const
{
    return TVec<string>(1,string("mse"));
}

void BasisSelectionRegressor::setTrainStatsCollector(PP<VecStatsCollector> statscol)
{ 
    inherited::setTrainStatsCollector(statscol);
    template_learner->setTrainStatsCollector(statscol);
}


TVec<string> BasisSelectionRegressor::getTrainCostNames() const
{
    return template_learner->getTrainCostNames();
}

void BasisSelectionRegressor::setTrainingSet(VMat training_set, bool call_forget)
{
    inherited::setTrainingSet(training_set, call_forget);
    template_learner->setTrainingSet(training_set, call_forget);
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
