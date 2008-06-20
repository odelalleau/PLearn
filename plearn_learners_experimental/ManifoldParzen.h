// -*- C++ -*-

// ManifoldParzen.h
//
// Copyright (C) 2007 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file ManifoldParzen.h */


#ifndef ManifoldParzen_INC
#define ManifoldParzen_INC

#include <plearn/vmat/ClassSubsetVMatrix.h>
#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

/**
 * Manifold Parzen Windows classifier and distribution.
 */
class ManifoldParzen : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! Number of nearest neighbors to use to learn
    //! the manifold structure.
    int nneighbors;

    //! Dimensionality of the manifold
    int ncomponents;

    //! Additive minimum value for the variance in all directions.
    real global_lambda0;

    //! Indication that the meam of the gaussians should also be learned
    bool learn_mu;

    //! Number of classes
    int n_classes;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    ManifoldParzen();

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

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;

    /**
     *  Declares the training set.  Then calls build() and forget() if
     *  necessary.  Also sets this learner's inputsize_ targetsize_ weightsize_
     *  from those of the training_set.  Note: You shouldn't have to override
     *  this in subclasses, except in maybe to forward the call to an
     *  underlying learner.
     */
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(ManifoldParzen);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Not Options  #####################################################

    //! Variables for density of a Gaussian
    mutable Vec mu;

    //! Variables for the SVD and gradient computation
    mutable Mat Ut, U, V;
    mutable Vec diff_neighbor_input, uk, sm_svd, S, diff;

    // Saved components of manifold parzen windows
    //! Eigenvectors
    mutable TVec<Mat> eigenvectors;
    //! Eigenvalues
    mutable Mat eigenvalues;
    //! Sigma noises
    mutable Vec sigma_noises;
    //! Mus
    mutable Mat mus;

    //! Datasets for each class
    TVec< PP<ClassSubsetVMatrix> > class_datasets;

    //! Proportions of examples from the other classes (columns), for each
    //! class (rows)
    //Vec class_proportions;

    //! Nearest neighbors for each training example
    TMat<int> nearest_neighbors_indices;

    //! Nearest neighbor votes for test example
    mutable Vec test_votes;

protected:
    //#####  Protected Member Functions  ######################################

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
DECLARE_OBJECT_PTR(ManifoldParzen);

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
