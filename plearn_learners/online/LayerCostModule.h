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

#include <map>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn/vmat/VMat.h>

namespace PLearn {

/**
 * Computes a cost function for a (hidden) representation, given two "expectation" vectors. Backpropagates it.
 */
class LayerCostModule : public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

    string cost_function;
    
    int histo_size;
    
    real alpha;
    
    real momentum;
    
public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    LayerCostModule();


    //! given the input and target, compute the cost
    virtual void fprop(const Vec& expectation, real& cost) const;
    virtual void fprop(const Mat& expectations, Mat& costs);
    //! Overridden.
    virtual void fprop(const TVec<Mat*>& ports_value);
    
    //! backpropagate the derivative w.r.t. activation
    virtual void bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient);

    //! Some auxiliary function to deal with empirical histograms
    virtual void computeHisto(const Mat& expectations);
    virtual void computeSafeHisto(const Mat& expectations);
    virtual real delta_KLdivTerm(int i, int j, int index_i, real over_dq, real one_count);
    virtual real delta_KLdivTerm_2(int i, int j, int index_i, real over_dq, real one_count);
    virtual real delta_SafeKLdivTerm(int i, int j, int index_i, real over_dq, real one_count);
    virtual real KLdivTerm(real pi, real pj);
    virtual int histo_index(real q);
    virtual real dq(real q);

    //! Auxiliary function for the pascal's cost function
    virtual void computePascalStatistics(const Mat& expectations, bool duringTraining);

    //! Overridden to do nothing (in particular, no warning).
    virtual void setLearningRate(real dynamic_learning_rate) {}

    //! Returns all ports in a RBMModule.
    virtual const TVec<string>& getPorts();

    //! The ports' sizes are given by the corresponding RBM layers.
    virtual const TMat<int>& getPortSizes();

    //! Return the index (as in the list of ports returned by getPorts()) of
    //! a given port.
    //! If 'port' does not exist, -1 is returned.
    virtual int getPortIndex(const string& port);

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

    //! Does stochastic gradient makes sense with our cost function?
    bool is_cost_function_stochastic;

    //! Histograms of expectations (estimated empiricially on the data)
    Mat expectations_histo;

    //! Some features of the histogram of expectations
    real LINHISTO_STEP;
    real LOGHISTO_BASE;
    real LOGHISTO_MIN;

    //! Statistics on matrix of expectations (estimated empiricially on the data)
    Vec expectations_expectation;
    Mat expectations_cross_quadratic_mean;
    Vec expectations_expectation_trainMemory;
    Mat expectations_cross_quadratic_mean_trainMemory;
    Vec expectations_expectation_testMemory;
    Mat expectations_cross_quadratic_mean_testMemory;
    int ntest;

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
