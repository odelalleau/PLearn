// -*- C++ -*-

// LIBSVMSparseVMatrix.h
//
// Copyright (C) 2008 Hugo Larochelle
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

/*! \file LIBSVMSparseVMatrix.h */


#ifndef LIBSVMSparseVMatrix_INC
#define LIBSVMSparseVMatrix_INC

#include <plearn/vmat/RowBufferedVMatrix.h>
#include <plearn/db/getDataSet.h>
#include <plearn/vmat/VMat.h>

namespace PLearn {

/**
 * VMatrix containing data from a libsvm format file
 */
class LIBSVMSparseVMatrix : public RowBufferedVMatrix
{
    typedef RowBufferedVMatrix inherited;
    static VMatrixExtensionRegistrar* extension_registrar;

public:
    //#####  Public Build Options  ############################################

    //! Strings associated to the different classes
    TVec<string> class_strings;

    //! File name of libsvm data
    PPath libsvm_file;

    //! Indication that a coarse (i.e. fixed length, filled with 0's) representation
    //! of the data in the .libsvm file should be used.
    bool use_coarse_representation;

    //! Input data
    TVec< Vec > libsvm_input_data;

    //! Index of non-zero inputs
    TVec< Vec > libsvm_extra_data;

    //! Target data
    Vec libsvm_target_data;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    LIBSVMSparseVMatrix();
    LIBSVMSparseVMatrix(PPath filename, bool use_coarse_representation);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(LIBSVMSparseVMatrix);

    // Simply calls inherited::build() then build_()
    virtual void build();

    static VMat instantiateFromPPath(const PPath& filename)
    {
        //By default use normal representation
        return VMat(new LIBSVMSparseVMatrix(filename, true));
    }

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void getExample(int i, Vec& input, Vec& target, real& weight);

    void getExamples(int i_start, int length, Mat& inputs, Mat& targets,
                     Vec& weights, Mat* extra = NULL,
                     bool allow_circular = false);

    virtual void getExtra(int i, Vec& extra);

protected:
    //#####  Protected Options  ###############################################

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Fill the vector 'v' with the content of the i-th row.
    //! 'v' is assumed to be the right size.
    virtual void getNewRow(int i, const Vec& v) const;

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(LIBSVMSparseVMatrix);

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
