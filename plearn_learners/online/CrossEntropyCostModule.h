// -*- C++ -*-

// CrossEntropyCostModule.h
//
// Copyright (C) 2007 Pascal Lamblin, Dumitru Erhan and Hugo Larochelle
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

// Authors: Pascal Lamblin, Dumitru Erhan and Hugo Larochelle

/*! \file CrossEntropyCostModule.h */


#ifndef CrossEntropyCostModule_INC
#define CrossEntropyCostModule_INC

#include <plearn_learners/online/CostModule.h>

namespace PLearn {

/**
 * Computes the cross-entropy, given two activation vectors. Backpropagates it.
 */
class CrossEntropyCostModule : public CostModule
{
    typedef CostModule inherited;

public:
    //#####  Public Build Options  ############################################


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    CrossEntropyCostModule();

    // Your other public member functions go here

    //! given the input and target, compute the cost
    virtual void fprop(const Vec& input, const Vec& target, real& cost) const;
    virtual void fprop(const Vec& input, const Vec& target, Vec& cost) const;
    virtual void fprop(const Mat& input, const Mat& target, Mat& cost) const;

    //! Standard backpropagation
    virtual void bpropUpdate(const Vec& input, const Vec& target, real cost,
                             Vec& input_gradient, bool accumulate=false);

    //! Minibatch backpropagation
    virtual void bpropUpdate(const Mat& inputs, const Mat& targets,
                             const Vec& costs, Mat& input_gradients,
                             bool accumulate=false);

    //! New version of backpropagation
    virtual void bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient);

    //! Overridden to do nothing (in particular, no warning).
    virtual void setLearningRate(real dynamic_learning_rate) {}

    //! Indicates the name of the computed costs
    virtual TVec<string> costNames();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(CrossEntropyCostModule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


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
DECLARE_OBJECT_PTR(CrossEntropyCostModule);

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
