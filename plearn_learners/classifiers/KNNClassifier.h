// -*- C++ -*-

// KNNClassifier.h
//
// Copyright (C) 2004 Nicolas Chapados 
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

// Authors: Nicolas Chapados

/*! \file KNNClassifier.h */


#ifndef KNNClassifier_INC
#define KNNClassifier_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/nearest_neighbors/GenericNearestNeighbors.h>
#include <plearn/ker/Kernel.h>

namespace PLearn {

/**
 * This class provides a simple N-class classifier based upon an enclosed
 * K-nearest-neighbors finder (derived from GenericNearestNeighbors;
 * specified with the 'knn' option).  The target variable (the class), is
 * assumed to be coded an integer variable (the class number, from 0 to
 * C-1, where C is the number of classes); the number of classes is
 * specified with the option 'nclasses'. The structure of the learner
 * output is a vector of probabilities for each class (even if
 * numclasses==2, which is NOT collapsed into a probability of the positive
 * class).
 *
 * The class contains several options to determine the number of neighbors
 * to use (K).  This number always overrides the option 'num_neighbors'
 * that may have been specified in the GenericNearestNeighbors utility
 * object.  Basically, the generic formula for the number of neighbors is
 *
 *     K = max(kmin, kmult*(n^kpow)),
 *
 * where 'kmin', 'kmult', and 'kpow' are options, and 'n' is the number of
 * examples in the training set.
 *
 * The costs output from this class are:
 *
 * - 'class_error', the classification error, i.e.
 *       classerror = max_i output[i] != target
 *
 * - 'neglogprob', the total negative log-probability of target, i.e.
 *       neglogprob = -log(output[target])
 *
 * If the option 'use_knn_costs_as_weights' is true (by default), it is
 * assumed that the costs coming from the 'knn' object are kernel
 * evaluations for each nearest neighbor.  These are used as weights to
 * determine the final class probabilities.  (NOTE: it is important to use
 * a kernel that computes a SIMILARITY MEASURE, and not a DISTANCE MEASURE;
 * the default EpanechnikovKernel has the proper behavior.)  If the option
 * is false, an equal weighting is used (equivalent to square window).
 *
 * The weights originally present in the training set ARE TAKEN INTO
 * ACCOUNT when weighting each observation: they serve to multiply the
 * kernel values to give the effective weight for an observation.
 */
class KNNClassifier: public PLearner
{
    typedef PLearner inherited;

protected:
    //! Internal use: temporary buffer for knn output
    mutable Vec knn_output;

    //! Internal use: temporary buffer for knn costs
    mutable Vec knn_costs;

    //! Internal use: temporary buffer for cumulating class weights
    mutable Vec class_weights;

    //! Internal use: this is used when a multi_k option is provided 
    //! to temporarily store the outputs the classifier would give for 
    //! all values of k given in multi_k.
    //! These outputs are computed by the computeOutput method, for
    //! consumption by the computeCostsFromOutputs method (whuch must be 
    //! called right after). 
    mutable Mat multi_k_output;

    //! Internal use to remember the input used in computeOutput when using multi_k option.
    mutable Vec multi_k_input;
  
public:
    //#####  Public Build Options  ############################################

    //! The K-nearest-neighbors finder to use (default is an
    //! ExhaustiveNearestNeighbors with a GaussianKernel, sigma=1)
    PP<GenericNearestNeighbors> knn;

    //! Number of classes in the problem
    int nclasses;
  
    //! Minimum number of neighbors to use (default=5)
    int kmin;

    //! Multiplicative factor on n^kpow to determine number of neighbors to
    //! use (default=0)
    real kmult;

    //! Power of the number of training examples to determine number of
    //! neighbors (default=0.5)
    real kpow;

    //! Whether to weigh each of the K neighbors by the kernel evaluations,
    //! obtained from the costs coming out of the 'knn' object (default=true)
    bool use_knn_costs_as_weights;

    //! If use_knn_costs_as_weights is false, use this kernel to weight the
    //! observations.  If this object is not specified (default), the
    //! rectangular kernel is used.

    //! Disregard the 'use_knn_costs_as_weights' option, and use this kernel
    //! to weight the observations.  If this object is not specified
    //! (default), and the 'use_knn_costs_as_weights' is false, the
    //! rectangular kernel is used.
    Ker kernel;
    
    //! This can be used if you wish to simultaneously compute the costs for several
    //! values of k, efficiently, while doing neighbors search a single time.
    //! (see corresponding declareOption in .cc for more detailed info).
    TVec<int> multi_k;

public:
    //#####  Object Methods  ##################################################

    //! Default constructor.
    KNNClassifier();

    //! Simply calls inherited::build() then build_().
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods.
    // If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT.
    PLEARN_DECLARE_OBJECT(KNNClassifier);

public:
    //#####  PLearner Methods  ####################################################

    //! Overridden to call knn->setTrainingSet
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    //! Forwarded to knn
    virtual void forget();
    
    //! Forwarded to knn
    virtual void train();

    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output. 
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;
  
    //! Returns the names of the costs computed by computeCostsFromOutpus
    //! (and thus the test method).
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes and 
    //! for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;

private: 
    //! This does the actual building. 
    void build_();

protected: 
    //! Declares this class' options.
    static void declareOptions(OptionList& ol);
};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(KNNClassifier);
  
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
