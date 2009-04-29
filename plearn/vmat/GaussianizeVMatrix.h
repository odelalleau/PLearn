// -*- C++ -*-

// GaussianizeVMatrix.h
//
// Copyright (C) 2006 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file GaussianizeVMatrix.h */


#ifndef GaussianizeVMatrix_INC
#define GaussianizeVMatrix_INC

#include <plearn/vmat/SourceVMatrix.h>

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
class GaussianizeVMatrix : public SourceVMatrix
{
    typedef SourceVMatrix inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

    bool gaussianize_input;
    bool gaussianize_target;
    bool gaussianize_weight;
    bool gaussianize_extra;
    bool gaussianize_binary;
    real threshold_ratio;
    bool save_and_reuse_stats;
    VMat train_source;
    string stats_file_to_use;
    TVec<int> fields_to_gaussianize;
    PPath save_fields_gaussianized;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    GaussianizeVMatrix();


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(GaussianizeVMatrix);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


    //! return the approximate value of value before being gaussianized.
    real unGauss(real value, int col)const;
    void unGauss(Vec& values, Vec& ret, int col)const;

protected:

    //! List of features that need to be Gaussianized.
    TVec<int> features_to_gaussianize;

    TVec<int> fields_gaussianized;

    //! Scaling factor to map the rank to [0,1].
    Vec scaling_factor;

    //! The j-th element is the list of values appearing in the variable
    //! features_to_gaussianize[j].
    TVec<Vec> values;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Declare the methods that are remote-callable
    static void declareMethods(RemoteMethodMap& rmm);

    //! Fill the vector 'v' with the content of the i-th row.
    //! v is assumed to be the right size.
    //! ### This function must be overridden in your class
    virtual void getNewRow(int i, const Vec& v) const;

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();
    virtual void setMetaDataDir(const PPath& the_metadatadir);
    real remote_unGauss(real input, int col)const;
    Vec remote_unGauss_vec(Vec inputs, int col) const;
    void append_col_to_gaussianize(int col, StatsCollector stat);
private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(GaussianizeVMatrix);

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
