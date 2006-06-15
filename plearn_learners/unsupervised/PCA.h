// -*- C++ -*-

// PCA.h
//
// Copyright (C) 2003  Pascal Vincent 
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

/*! \file PCA.h */
#ifndef PCA_INC
#define PCA_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {
using namespace std;

class PCA: public PLearner
{
private:
    typedef PLearner inherited;

protected:

    // Cache for incremental_algo()
    VecStatsCollector _incremental_stats;  
  
    /*! Incremental algo:
      The first time values are fed to _incremental_stats, we must remember
      the first observation in order not to remove observation that never
      contributed to the covariance matrix.

      Initialized to -1;
    */
    int _oldest_observation;

  
    // The following methods correspond to the algo option choices
    void classical_algo   ( );
    void incremental_algo ( );
    void em_algo          ( );
    void em_orth_algo     ( );
  
public:
  
    // ************************
    // * public build options *
    // ************************

    /*!
      The algorithm used to perform the Principal Component Analysis:
      - 'classical'   : compute the eigenvectors of the covariance matrix
        
      - 'incremental' : Uses the classical algorithm but computes the
      covariance matrix in an incremental manner. When
      'incremental' is used, a new training set is
      assumed to be a superset of the old training set,
      i.e. begining with the rows of the old training
      set but ending with some new rows. 
        
      - 'em'          : EM algorithm from "EM algorithms for PCA and
      SPCA" by S. Roweis
        
      - 'em_orth'     : a variant of 'em', where orthogonal components
      are directly computed          
    */
    string algo;

    /*!
      Incremental algorithm option: This option specifies a window over
      which the PCA should be done. That is, if the length of the training
      set is greater than 'horizon', the observations that will effectively
      contribute to the covariance matrix will only be the last 'horizon'
      ones. All negative values being interpreted as 'keep all observations'.

      Default: -1 (all observations are kept)
    */
    int _horizon;
  
    int ncomponents;  //! The number of principal components to keep (that's also the outputsize)
    real sigmasq;     //! This gets added to the diagonal of the covariance matrix prior to eigen-decomposition
    bool normalize;   //! If true, we divide by sqrt(eigenval) after projecting on the eigenvec.
    bool normalize_warning;

    //! If true, if a missing value is encountered on an input variable
    //! for a computeOutput, it is replaced by the estimated mu for that
    //! variable before projecting on the principal components
    bool impute_missing;
  
    // Saved options
    Vec mu; //! The (weighted) mean of the samples 
    Vec eigenvals;  //! The ncomponents eigenvalues corresponding to the principal directions kept
    Mat eigenvecs;  //! A ncomponents x inputsize matrix containing the principal eigenvectors

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    PCA();

    // ******************
    // * PLearner methods *
    // ******************

private: 

    //! This does the actual building. 
    void build_();

protected: 

    //! Declares this class' options
    static void declareOptions(OptionList& ol);

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! Set nstages to the training_set length under the 'incremental' algo.
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);
  
    //! Simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods
    //  If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
    PLEARN_DECLARE_OBJECT(PCA);


    // **************************
    // **** PLearner methods ****
    // **************************

    //! returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options)
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    virtual void forget();

    
    //! The role of the train method is to bring the learner up to stage==nstages,
    //! updating the train_stats collector with training costs measured on-line in the process.
    virtual void train();


    //! Computes the output from the input
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Reconstructs an input from a (possibly partial) output (i.e. the first few princial components kept).
    void reconstruct(const Vec& output, Vec& input) const;
  
    //! Computes the costs from already computed output.
    //! The only computed cost is the squared_reconstruction_error
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;
                                

    //! Returns [ "squared_reconstruction_error" ]
    virtual TVec<string> getTestCostNames() const;


    //! No trian costs are computed for this learner
    virtual TVec<string> getTrainCostNames() const;


    // *** SUBCLASS WRITING: ***
    // While in general not necessary, in case of particular needs 
    // (efficiency concerns for ex) you may also want to overload
    // some of the following methods:
    // virtual void computeOutputAndCosts(const Vec& input, const Vec& target, Vec& output, Vec& costs) const;
    // virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;
    // virtual void test(VMat testset, PP<VecStatsCollector> test_stats, VMat testoutputs=0, VMat testcosts=0) const;
    // virtual int nTestCosts() const;
    // virtual int nTrainCosts() const;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PCA);
  
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
