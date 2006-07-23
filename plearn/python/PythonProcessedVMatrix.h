// -*- C++ -*-

// PythonProcessedVMatrix.h
//
// Copyright (C) 2005 Nicolas Chapados 
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file PythonProcessedVMatrix.h */


#ifndef PythonProcessedVMatrix_INC
#define PythonProcessedVMatrix_INC

// Python includes must come first! (as per Python doc)
#include <plearn/python/PythonCodeSnippet.h>

// C++ stdlib stuff
#include <string>

// PLearn stuff
#include <plearn/vmat/SourceVMatrix.h>


namespace PLearn {

/**
 *  Preprocess a source VMatrix using a Python code snippet.
 *
 *  The Python code snippet (see the option 'code' below) must define the
 *  following functions:
 *
 *   from numarray import *
 *
 *   def getRow(row_no, source_row):
 *       """Return a numarray vector containing the processed row, given
 *       the original (source) row vector at the given row number."""
 *
 *   def getFieldNames(source_field_names):
 *       """Return the new fieldnames (after processing), given the
 *       original fieldnames.  This function also determines the NEW
 *       width of the VMatrix."""
 *
 *   def getSizes(source_inputsize, source_targetsize, source_weightsize):
 *       """This function is optional.  If defined, return a list of 3 integers
 *       containing, respectively, the new inputsize, targetsize and
 *       weightsize."""
 *
 *   def build():
 *       """This function is optional.  If defined, it is called on the
 *       build_() of the underlying C++ object.  Its purpose is to allow
 *       matrix-wide processings to be performed and stored in the
 *       Python snippet internal variables.  Use this, for instance,
 *       to compute matrix-wide means and variances for some normalization
 *       functions."""
 *
 *  The Python code snippet has, in turn, access to the following interface:
 *
 *  - source_field_names (global variable): a Python list of strings containing
 *    the fieldnames in the source matrix.  This is the same as the argument
 *    passed to 'getFieldNames()' when called.
 *
 *  - source_length (global variable): number of rows in the Source matrix.
 *    Right now, this cannot be changed and the processed number of rows is
 *    always the same as the original number of rows.
 *
 *  - source_width (global variable): number of columns in the Source matrix.
 *
 *  - source_inputsize, source_targetsize, source_weightsize (global
 *    variables): the corresponding fields as they were set in the source
 *    matrix.
 *
 *  - getSourceRow(row_no): a function that returns a numarray vector
 *    containing the given row in the source matrix.
 *
 *  In addition, a map (from string to string) named "params" is defined as an
 *  option at the C++ level, and is made available to the Python code as a
 *  global variable called, appropriately, "params".  Note that to change this
 *  map programmatically (after build), should should call the setParams()
 *  member function to ensure that the changes are propagated into Python.
 */
class PythonProcessedVMatrix : public SourceVMatrix
{
    typedef SourceVMatrix inherited;

public:
    //#####  Public Build Options  ############################################

    /**
     *  The Python code snippet used to process the matrix.  The functions
     *  described in the class documentation must be provided.  Note that,
     *  after an initial build(), changing this string calling build() again
     *  DOES NOT result in the recompilation of the code.
     */
    string m_code;

    /**
     *  General-purpose parameters that are injected into the Python code
     *  snippet as a global variable under the name "params".  Can be used for
     *  passing processing arguments to the Python code.
     */
    map<string,string> m_params;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    PythonProcessedVMatrix();

    //! Set new parameters into the Python code snippet.  The option m_params
    //! is updated as well
    void setParams(const map<string,string>& params);
    
    
    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(PythonProcessedVMatrix);

    // Simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Member Functions  ######################################
    
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Fill the vector 'v' with the content of the i-th row.
    //! v is assumed to be the right size.
    virtual void getNewRow(int i, const Vec& v) const;

    //! Function injected into the Python code to return a row in the source
    //! VMatrix
    PythonObjectWrapper getSourceRow(const TVec<PythonObjectWrapper>& args) const;

    //! If not already done, compile the Python snippet and inject the
    //! required stuff into the Python environment
    void compileAndInject();
    
protected:
    //! Actual Python environment
    PP<PythonCodeSnippet> python;

private: 
    //! This does the actual building. 
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PythonProcessedVMatrix);
  
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
