// -*- C++ -*-

// ManifoldParzen2.h
//
// Copyright (C) 2003 Pascal Vincent, Julien Keable
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

/*! \file ManifoldParzen2.h */
#ifndef ManifoldParzen2_INC
#define ManifoldParzen2_INC

#include "GaussMix.h"

namespace PLearn {
using namespace std;

class ManifoldParzen2 : public GaussMix
{

private:

    typedef GaussMix inherited;

    Vec eigenval_copy;
    mutable Vec row;
    mutable Vec t_row;
    mutable VMat reference_set;
    mutable TMat<real> mu_temp;
    mutable TVec<real> temp_eigv;

    int find_nearest_neighbor(VMat data, Vec x) const;

public:

    // *********************
    // * protected options *
    // *********************

    // ### declare protected option fields (such as learnt parameters) here
    // ...

    //!  If you change one of these, you must retrain
    int nneighbors; //!<  how many neighbors should we consider
    int ncomponents; //!<  how many components do we want to remember from the PCA

    bool use_last_eigenval;
    real scale_factor;
    real global_lambda0;
    bool learn_mu;

    // ************************
    // * public build options *
    // ************************

    // ### declare public option fields (such as build options) here
    // ...

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    ManifoldParzen2();

    // ******************
    // * Object methods *
    // ******************

    ManifoldParzen2(int the_nneighbors, int the_ncomponents, bool use_last_eigenvalue=true, real scale_factor=1);

protected:

    //! Declares this class' options
    static void declareOptions(OptionList& ol);

private:
    //! This does the actual building.
    // (Please implement in .cc)
    void build_();

public:
    // simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(ManifoldParzen2);

    // *******************
    // * Learner methods *
    // *******************

    //! trains the model
    // NOTE : the function assumes that the training_set has only input columns ! (width = dimension of feature space)
    virtual void train();

    //! Produce outputs according to what is specified in outputs_def.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Returned value depends on outputs_def.
    virtual int outputsize() const;

    real evaluate(const Vec x1,const Vec x2,real scale=1);

    real evaluate_i_j(int i, int j,real scale=1);
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ManifoldParzen2);

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
