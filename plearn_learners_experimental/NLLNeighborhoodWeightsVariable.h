// -*- C++ -*-

// NLLNeighborhoodWeightsVariable.h
//
// Copyright (C) 2006 Hugo Larochelle
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

/*! \file NLLNeighborhoodWeightsVariable.h */


#ifndef NLLNeighborhoodWeightsVariable_INC
#define NLLNeighborhoodWeightsVariable_INC

#include <plearn/var/BinaryVariable.h>

namespace PLearn {
using namespace std;

/*! * NLLNeighborhoodWeightsVariable * */

/**
   WeightsVariable updated online, based on negative log-likelihood of the neighbors
 */
class NLLNeighborhoodWeightsVariable : public BinaryVariable
{
    typedef BinaryVariable inherited;

public:
    //#####  Public Build Options  ############################################

    //! Total number of points to be weighted
    int n;
    //! Exponential decay
    real alpha;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor, usually does nothing
    NLLNeighborhoodWeightsVariable();

    //! Constructor initializing from two input variables
    NLLNeighborhoodWeightsVariable(Variable* neighbor_nlls, Variable* neighbor_indexes, int n, real alpha);

    //#####  PLearn::Variable methods #########################################
    // (PLEASE IMPLEMENT IN .cc)
    virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();

    // ### These ones are not always implemented
    // virtual void bbprop();
    // virtual void symbolicBprop();
    // virtual void rfprop();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(NLLNeighborhoodWeightsVariable);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    //! Online log-sum of weights 
    TVec<real> online_weights_log_sum;
    //! log(alpha)
    real log_alpha;
    real log_1_minus_alpha;

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

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(NLLNeighborhoodWeightsVariable);

inline Var nll_neighborhood_weights(Var neighbor_nlls, Var neighbor_indexes, int n, real alpha)
 { 
     return new NLLNeighborhoodWeightsVariable(neighbor_nlls, neighbor_indexes, n, alpha); 
 }

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
