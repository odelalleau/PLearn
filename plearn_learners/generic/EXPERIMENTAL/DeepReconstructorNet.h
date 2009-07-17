// -*- C++ -*-

// DeepReconstructorNet.h
//
// Copyright (C) 2007 Simon Lemieux, Pascal Vincent
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

// Authors: Simon Lemieux, Pascal Vincent

/*! \file DeepReconstructorNet.h */


#ifndef DeepReconstructorNet_INC
#define DeepReconstructorNet_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/var/Variable.h>
#include <plearn/opt/Optimizer.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/var/SourceVariable.h>

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
class DeepReconstructorNet : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!
    
    TVec< pair<int,int> > unsupervised_nepochs;
    Vec unsupervised_min_improvement_rate;

    pair<int,int> supervised_nepochs;    
    real supervised_min_improvement_rate;

    // layers[0] is the input variable
    // last layer is final output layer
    VarArray layers;

    // reconstruction_costs[k] is the reconstruction cost for layers[k]
    VarArray reconstruction_costs;
    
    // The names to be given to each of the elements of a vector cost 
    TVec<string> reconstruction_costs_names;

    // reconstructed_layers[k] is the reconstruction of layer k from layers[k+1]
    VarArray reconstructed_layers;

    // hidden_for_reconstruction[k] is the hidden representation used to reconstruct reconstructed_layers[k] 
    // i.e. it is the representation at layer k+1 but possibly obtained from a corrupted input (contrary to layers[k+1]).
    VarArray hidden_for_reconstruction;

    // optimizers if we use different ones for each layer
    TVec< PP<Optimizer> > reconstruction_optimizers;
    
    // if we use always the same optimizer
    PP<Optimizer> reconstruction_optimizer;


    Var target;
    //TVec<Var> supervised_costs;
    VarArray supervised_costs;
    Var supervised_costvec; // hconcat(supervised_costs)

    TVec<string> supervised_costs_names;

    Var fullcost;
    
    VarArray parameters;

    int minibatch_size;


    PP<Optimizer> supervised_optimizer;

    PP<Optimizer> fine_tuning_optimizer;


    TVec<int> group_sizes;

protected:
    // protected members (not options)

    TVec<Func> compute_layer;
    Func compute_output;
    Func output_and_target_to_cost;
    TVec<VMat> outmat;
    

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    DeepReconstructorNet();


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


    virtual void initializeParams(bool set_seed=true);

    //! Returns the matValue of the parameter variable with the given name
    Mat getParameterValue(const string& varname);

    //! Returns the nth row of the matValue of the parameter variable with the given name
    Vec getParameterRow(const string& varname, int n);

    //! Returns a list of the names of the parameters (in the same order as in listParameter)
    TVec<string> listParameterNames();

    //! Returns a list of the parameters
    TVec<Mat> listParameter();

    void prepareForFineTuning();
    void fineTuningFor1Epoch();
    // void fineTuningFullOld();

    void trainSupervisedLayer(VMat inputs, VMat targets);

    TVec<Mat> computeRepresentations(Mat input);
    void reconstructInputFromLayer(int layer);
    TVec<Mat> computeReconstructions(Mat input);

    Mat getMatValue(int layer);
    void setMatValue(int layer, Mat values);
    Mat fpropOneLayer(int layer);
    Mat reconstructOneLayer(int layer);
       
    void computeAndSaveLayerActivationStats(VMat dataset, int which_layer, const string& pmatfilepath);

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
    PLEARN_DECLARE_OBJECT(DeepReconstructorNet);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    // ...

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    // (PLEASE IMPLEMENT IN .cc)
    static void declareOptions(OptionList& ol);
    static void declareMethods(RemoteMethodMap& rmm);

    void trainHiddenLayer(int which_input_layer, VMat inputs);
    void buildHiddenLayerOutputs(int which_input_layer, VMat inputs, VMat outputs);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    // (PLEASE IMPLEMENT IN .cc)
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
    int nout;

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(DeepReconstructorNet);

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
