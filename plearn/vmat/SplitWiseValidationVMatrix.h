// -*- C++ -*-

// SplitWiseValidationVMatrix.h
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

/*! \file SplitWiseValidationVMatrix.h */


#ifndef SplitWiseValidationVMatrix_INC
#define SplitWiseValidationVMatrix_INC

#include <plearn/vmat/RowBufferedVMatrix.h>

namespace PLearn {

/**
 * VMatrix that takes several experiment split_stats.pmat to 
 * extract the split statistics and perform validation.
 * It looks at the validation error, selects the best experiment 
 * and only report the associated statistics, for each split.
 */
class SplitWiseValidationVMatrix : public RowBufferedVMatrix
{
    typedef RowBufferedVMatrix inherited;

public:
    //#####  Public Build Options  ############################################

    //! PPath of the experiment split_stats.pmat
    TVec<PPath> split_stats_ppaths;

    //! Index of the validation error statistic column
    int validation_error_column;
    
    //! Number of splits to consider, starting from the first one
    int nsplits;

    //! Indication that the average of the columns should be added
    bool add_column_average;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    SplitWiseValidationVMatrix();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(SplitWiseValidationVMatrix);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    //! Contains all information about this VMatrix
    Mat data;

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
DECLARE_OBJECT_PTR(SplitWiseValidationVMatrix);

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
