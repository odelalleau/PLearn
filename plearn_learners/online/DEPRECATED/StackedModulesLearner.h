// -*- C++ -*-

// StackedModulesLearner.h
//
// Copyright (C) 2006 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file PLearn/plearn_learners/online/DEPRECATED/StackedModulesLearner.h */


#ifndef StackedModulesLearner_INC
#define StackedModulesLearner_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

class OnlineLearningModule;
class PRandom;

/**
 * Trains a stack of OnlineLearningModule, which are layers.
 * The OnlineLearningModule's are disposed like superposed layers:
 * outputs of module i are the inputs of module (i+1), the last layer is
 * the output layer.
 * Another TVec of modules contains the cost modules. The first one is used
 * during the training phase as the cost to minimize, the other ones are only
 * measured.
 *
 * @todo Finish this class...
 * @deprecated Use ../ModulesLearner instead
 *
 */
class StackedModulesLearner : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! Layers of the learner
    TVec< PP<OnlineLearningModule> > modules;

    //! Names of the cost functions to apply on output. First one will be used
    //! as the cost function to optimize. For the moment, supported value are:
    //!   - "mse" (default)
    //!   - "NLL"
    TVec<string> cost_funcs;

    //! Estimation of the second-order terms. One of:
    //!   - "none": using only first-order derivative for update,
    //!   - "diag": estimating the diagonal of the hessian,
    //!   - "simpler_diag": positive estimation of the diagonal
    string hessian_estimation;


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    StackedModulesLearner();


    //#####  PLearner Member Functions  #######################################

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    // (PLEASE IMPLEMENT IN .cc)
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    // (PLEASE IMPLEMENT IN .cc)
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training costs
    //! measured on-line in the process.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void train();

    //! Computes the output from the input.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    // (PLEASE IMPLEMENT IN .cc)
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    // (PLEASE IMPLEMENT IN .cc)
    virtual TVec<std::string> getTrainCostNames() const;


    // *** SUBCLASS WRITING: ***
    // While in general not necessary, in case of particular needs
    // (efficiency concerns for ex) you may also want to overload
    // some of the following methods:
    // virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
    //                                    Vec& output, Vec& costs) const;
    // virtual void computeCostsOnly(const Vec& input, const Vec& target,
    //                               Vec& costs) const;
    // virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
    //                   VMat testoutputs=0, VMat testcosts=0) const;
    // virtual int nTestCosts() const;
    // virtual int nTrainCosts() const;
    // virtual void resetInternalState();
    // virtual bool isStatefulLearner() const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(StackedModulesLearner);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here

    //! Number of module layers
    int nmodules;

    //! Modules that will compute the costs
    TVec< PP<OnlineLearningModule> > cost_modules;

    //! Number of cost modules
    int ncosts;

    //####  Not Options  ######################################################

    //! stores the input and output values of the functions
    TVec<Vec> values;

    //! stores the gradients
    TVec<Vec> gradients;

    //! stores the diagonal of Hessians
    TVec<Vec> diag_hessians;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    // (PLEASE IMPLEMENT IN .cc)
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    // (PLEASE IMPLEMENT IN .cc)
    void build_();

    void buildOptions();
    void buildCostFunctions();
    void buildLayers();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(StackedModulesLearner);

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
