// -*- C++ -*-

// LayerCostModule.h
//
// Copyright (C) 2007 Jerome Louradour
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

// Authors: Jerome Louradour

/*! \file LayerCostModule.h */

#ifndef LayerCostModule_INC
#define LayerCostModule_INC

#include <plearn_learners/online/CostModule.h>

#include <map>

namespace PLearn {

/**
 * Computes a cost function for a (hidden) representation. Backpropagates it.
 */
class LayerCostModule : public CostModule
{
    typedef CostModule inherited;

public:
    //#####  Public Build Options  ############################################

    //! Generic name of the cost function
    string cost_function;

    //! Maximum number of stages we want to propagate the gradient    
    int nstages_max;

    //! Parameter to compute moving means in non stochastic cost functions
    real momentum;

    //! Kind of optimization
    string optimization_strategy;
    
    //! Parameter in pascal's cost function
    real alpha;    

    //! For non stochastic KL divergence cost function
    int histo_size;


    //#####  Public Learnt Options  ###########################################

    //! Histograms of inputs (estimated empiricially on some data)
    //! Computed only when cost_function == 'kl_div' or 'kl_div_simpe'
    Mat inputs_histo;

    //! Statistics on inputs (estimated empiricially on some data)
    //! Computed only when cost_function == 'correlation'
    //! or (for some) 'pascal'
    Vec inputs_expectation;
    Vec inputs_stds;         //! only for 'correlation' cost function

    Mat inputs_cross_quadratic_mean;
    Mat inputs_correlations; //! only for 'correlation' cost function

    //! Variables for (non stochastic) Pascal's/correlation function with momentum
    //! -------------------------------------------------------------
    //! Statistics on outputs (estimated empiricially on the data)    
    Vec inputs_expectation_trainMemory;
    Mat inputs_cross_quadratic_mean_trainMemory;

    //! The function applied to the local cost between 2 inputs
    //! to obtain the global cost (after averaging)
    string penalty_function;

    //! The generic name of the cost function
    string cost_function_completename;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    LayerCostModule();

    //! given the input and target, compute the cost
    virtual void fprop(const Vec& input, real& cost) const;
    virtual void fprop(const Mat& inputs, Mat& costs) const;
    virtual void fprop(const Mat& inputs, const Mat& targets, Mat& costs) const;
    virtual void fprop(const TVec<Mat*>& ports_value);

    //! backpropagate the derivative w.r.t. activation
    virtual void bpropUpdate(const Mat& inputs, const Mat& targets,
                             const Vec& costs, Mat& input_gradients, bool accumulate=false);
    virtual void bpropUpdate(const Mat& inputs, Mat& inputs_grad);
    virtual void bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient);

    //! Some auxiliary function to deal with empirical histograms
    virtual void computeHisto(const Mat& inputs);
    virtual void computeHisto(const Mat& inputs, Mat& histo) const;
    virtual real delta_KLdivTerm(int i, int j, int index_i, real over_dq);
    virtual real KLdivTerm(real pi, real pj) const;
    virtual real computeKLdiv(bool store_in_cache);
    virtual real computeKLdiv(const Mat& histo) const;
    virtual int histo_index(real q) const;
    virtual real dq(real q) const;
    //! Auxiliary functions for kl_div_simple cost function
    virtual void computeSafeHisto(const Mat& inputs);
    virtual void computeSafeHisto(const Mat& inputs, Mat& histo) const;
    virtual real delta_SafeKLdivTerm(int i, int j, int index_i, real over_dq);

    //! Auxiliary function for the pascal's cost function
    virtual void computePascalStatistics(const Mat& inputs);
    virtual void computePascalStatistics(const Mat& inputs,
                                         Vec& expectation, Mat& cross_quadratic_mean) const;
    virtual real func_(real correlation) const;
    virtual real deriv_func_(real correlation) const;

    //! Auxiliary function for the correlation's cost function
    virtual void computeCorrelationStatistics(const Mat& inputs);
    virtual void computeCorrelationStatistics(const Mat& inputs,
                                              Vec& expectation, Mat& cross_quadratic_mean,
                                              Vec& stds, Mat& correlations) const;    
    //! Returns all ports in a RBMModule.
    virtual const TVec<string>& getPorts();

    //! The ports' sizes are given by the corresponding RBM layers.
    virtual const TMat<int>& getPortSizes();

    //! Return the index (as in the list of ports returned by getPorts()) of
    //! a given port.
    //! If 'port' does not exist, -1 is returned.
    virtual int getPortIndex(const string& port);

    //! Overridden to do nothing (in particular, no warning).
    virtual void setLearningRate(real dynamic_learning_rate) {}

    //! Indicates the name of the computed costs
    virtual TVec<string> costNames();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(LayerCostModule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void forget();

protected:

    bool LINEAR_FUNC;
    bool SQUARE_FUNC;
    bool POW4_FUNC;
    bool EXP_FUNC;
    bool LOG_FUNC;

    //! Number of stage the BPropAccUpdate function was called
    int stage;

    //! Boolean determined by the optimization_strategy
    bool bprop_all_terms;
    bool random_index_during_bprop;

    //! Does stochastic gradient (without memory of the past)
    //! makes sense with our cost function?
    bool is_cost_function_stochastic;

    //! Normalizing factor applied to the cost function
    //! to take into acount the number of weights
    real norm_factor;

    real average_deriv;

    //! Variables for (non stochastic) KL Div cost function
    //! ---------------------------------------------------
    //! Range of a histogram's bin ( HISTO_STEP = 1/histo_size )
    real HISTO_STEP;
    //! the weight of a sample within a batch (usually, 1/n_samples)

    mutable real one_count; 
    TVec< TVec< Vec > > cache_differ_count_i;
    TVec< TVec< Vec > > cache_differ_count_j;
    TVec< TVec< Vec > > cache_n_differ;

    //! Map from a port name to its index in the 'ports' vector.
    map<string, int> portname_to_index;

    //! List of port names.
    TVec<string> ports;

    //#####  Protected Member Functions  ######################################

    //! Add a new port to the 'portname_to_index' map and 'ports' vector.
    void addPortName(const string& name);

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(LayerCostModule);

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
