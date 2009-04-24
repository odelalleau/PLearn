// -*- C++ -*-

// BasisSelectionRegressor.h
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

/*! \file BasisSelectionRegressor.h */


#ifndef BasisSelectionRegressor_INC
#define BasisSelectionRegressor_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/math/RealFunction.h>
#include <plearn/ker/Kernel.h>

namespace PLearn {

/**
 * The first sentence should be a BRIEF DESCRIPTION of what the class does.
 * Place the rest of the class programmer documentation here.  Doxygen supports
 * Javadoc-style comments.  See http://www.doxygen.org/manual.html
 *
 * @todo Write class to-do's here if there are any.
 *
 * @deprecated Write deprecated stuff here if there is any.  Indicate what else
 * should be used instead.
 */

class BasisSelectionRegressor : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################
    bool consider_constant_function;
    TVec<RealFunc> explicit_functions;
    TVec<RealFunc> explicit_interaction_functions;
    TVec<string> explicit_interaction_variables;
    TVec<RealFunc> mandatory_functions;
    bool consider_raw_inputs;
    bool consider_normalized_inputs;
    bool consider_input_range_indicators;
    bool fixed_min_range;
    real indicator_desired_prob;
    real indicator_min_prob;
    TVec<Ker> kernels;
    mutable Mat kernel_centers;
    int n_kernel_centers_to_pick;
    bool consider_interaction_terms;
    int max_interaction_terms;
    int consider_n_best_for_interaction;
    int interaction_max_order;
    bool consider_sorted_encodings;
    int max_n_vals_for_sorted_encodings;
    bool normalize_features;
    PP<PLearner> learner;
    bool precompute_features;
    int n_threads;
    int thread_subtrain_length;
    bool use_all_basis;

    //#####  Public Learnt Options  ############################################
    TVec<RealFunc> selected_functions;
    Vec alphas;
    mutable Mat scores;


    struct thread_wawr;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    BasisSelectionRegressor();

    void printModelFunction(PStream& out) const;

    //#####  PLearner Member Functions  #######################################

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training costs
    //! measured on-line in the process.
    virtual void train();

    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    virtual TVec<std::string> getTestCostNames() const;


    //! simply forwards stats coll. to sub-learner
    virtual void setTrainStatsCollector(PP<VecStatsCollector> statscol);

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;

    //! Forwards the call to sub-learner
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(BasisSelectionRegressor);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void setExperimentDirectory(const PPath& the_expdir);

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

    void appendCandidateFunctionsOfSingleField(int fieldnum, TVec<RealFunc>& functions) const;
    void appendKernelFunctions(TVec<RealFunc>& functions) const;
    void appendConstantFunction(TVec<RealFunc>& functions) const;

    //! Fills the simple_candidate_functions
    void buildSimpleCandidateFunctions();

    //! Builds candidate_functions.
    //! If consider_interactions is false, candidate_functions is the same as simple_candidate_functions
    //! If consider_interactions is true,  candidate_functions will in addidion include all products 
    //! between simple_candidate_functions and selected_functions 
    void buildAllCandidateFunctions();

    //! Builds top best candidate functions (from simple_candidate_functions only)
    TVec<RealFunc> buildTopCandidateFunctions();

    //! Returns the index of the best candidate function (most colinear with the residue)
    void findBestCandidateFunction(int& best_candidate_index, real& best_score) const;

    //! Adds an interaction term to all_functions as RealFunctionProduct(f1, f2)
    void addInteractionFunction(RealFunc& f1, RealFunc& f2, TVec<RealFunc>& all_functions);

    //! Computes the order of the given function (number of single function embedded)
    void computeOrder(RealFunc& func, int& order);

    void computeWeightedAveragesWithResidue(const TVec<RealFunc>& functions,   
                                            real& wsum,
                                            Vec& E_x, Vec& E_xx,
                                            real& E_y, real& E_yy,
                                            Vec& E_xy) const;

    /*
    void computeWeightedCorrelationsWithY(const TVec<RealFunc>& functions, const Vec& Y,  
                                          real& wsum,
                                          Vec& E_x, Vec& V_x,
                                          real& E_y, real& V_y,
                                          Vec& E_xy, Vec& V_xy,
                                          Vec& covar, Vec& correl,
                                          real min_variance = 1e-6) const;
    */
    void appendFunctionToSelection(int candidate_index);
    void retrainLearner();
    void initTargetsResidueWeight();
    void recomputeFeatures();
    void recomputeResidue();
    void computeOutputFromFeaturevec(const Vec& featurevec, Vec& output) const;

protected:
    //#####  Protected Data Members  ##########################################

    // Template learner.  Each train step is done with a copy of this one.
    PP<PLearner> template_learner;

private:
    //#####  Private Data Members  ############################################

    TVec<RealFunc> simple_candidate_functions;
    TVec<RealFunc> candidate_functions;
    Mat features;
    Vec residue;
    Vec targets;
    Vec weights;
    double residue_sum;
    double residue_sum_sq;

    mutable Vec input;
    mutable Vec targ;
    mutable Vec featurevec;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(BasisSelectionRegressor);

} // end of namespace PLearn

#endif


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
