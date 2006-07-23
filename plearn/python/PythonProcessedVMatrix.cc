// -*- C++ -*-

// PythonProcessedVMatrix.cc
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

/*! \file PythonProcessedVMatrix.cc */


#include "PythonProcessedVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PythonProcessedVMatrix,
    "Preprocess a source VMatrix using a Python code snippet.",
    "The Python code snippet (see the option 'code' below) must define the\n"
    "following functions:\n"
    "\n"
    " from numarray import *\n"
    "\n"
    " def getRow(row_no, source_row):\n"
    "     \"\"\"Return a numarray vector containing the processed row, given\n"
    "     the original (source) row vector at the given row number.\"\"\"\n"
    "\n"
    " def getFieldNames(source_field_names):\n"
    "     \"\"\"Return the new fieldnames (after processing), given the\n"
    "     original fieldnames.  This function also determines the NEW\n"
    "     width of the VMatrix.\"\"\"\n"
    "\n"
    " def getSizes(source_inputsize, source_targetsize, source_weightsize):\n"
    "     \"\"\"This function is optional.  If defined, return a list of 3 integers\n"
    "     containing, respectively, the new inputsize, targetsize and\n"
    "     weightsize.\"\"\"\n"
    "\n"
    " def build():\n"
    "     \"\"\"This function is optional.  If defined, it is called on the\n"
    "     build_() of the underlying C++ object.  Its purpose is to allow\n"
    "     matrix-wide processings to be performed and stored in the\n"
    "     Python snippet internal variables.  Use this, for instance,\n"
    "     to compute matrix-wide means and variances for some normalization\n"
    "     functions.\"\"\"\n"
    "\n"
    "The Python code snippet has, in turn, access to the following interface:\n"
    "\n"
    "- source_field_names (global variable): a Python list of strings containing\n"
    "  the fieldnames in the source matrix.  This is the same as the argument\n"
    "  passed to 'getFieldNames()' when called.\n"
    "\n"
    "- source_length (global variable): number of rows in the Source matrix.\n"
    "  Right now, this cannot be changed and the processed number of rows is\n"
    "  always the same as the original number of rows.\n"
    "\n"
    "- source_width (global variable): number of columns in the Source matrix.\n"
    "\n"
    "- source_inputsize, source_targetsize, source_weightsize (global\n"
    "  variables): the corresponding fields as they were set in the source\n"
    "  matrix.\n"
    "\n"
    "- getSourceRow(row_no): a function that returns a numarray vector\n"
    "  containing the given row in the source matrix.\n"
    "\n"
    "In addition, a map (from string to string) named \"params\" is defined as an\n"
    "option at the C++ level, and is made available to the Python code as a\n"
    "global variable called, appropriately, \"params\".  Note that to change this\n"
    "map programmatically (after build), should should call the setParams()\n"
    "member function to ensure that the changes are propagated into Python.\n"
    );


//#####  Constructor  #########################################################

PythonProcessedVMatrix::PythonProcessedVMatrix()
    : inherited(),
      m_code(""),
      python()
{ }


//#####  declareOption  #######################################################

void PythonProcessedVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "code", &PythonProcessedVMatrix::m_code,
                  OptionBase::buildoption,
                  "The Python code snippet used to process the matrix.  The functions\n"
                  "described in the class documentation must be provided.  Note that,\n"
                  "after an initial build(), changing this string calling build() again\n"
                  "DOES NOT result in the recompilation of the code.\n");

    declareOption(ol, "params", &PythonProcessedVMatrix::m_params,
                  OptionBase::buildoption,
                  "General-purpose parameters that are injected into the Python code\n"
                  "snippet as a global variable under the name \"params\".  Can be used for\n"
                  "passing processing arguments to the Python code.\n");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}


//#####  build  ###############################################################

void PythonProcessedVMatrix::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

void PythonProcessedVMatrix::build_()
{
    // First step, compile the Python code
    compileAndInject();
    assert( python );
    assert( source );
    
    // Next ensure that we have some consistency
    if (! python->isInvokable("getRow"))
        PLERROR("PythonProcessedVMatrix: the Python code snippet must define the function "
                "'getRow'");
    if (! python->isInvokable("getFieldNames"))
        PLERROR("PythonProcessedVMatrix: the Python code snippet must define the function "
                "'getFieldNames'");

    // Set the global variables into the Python environment
    python->setGlobalObject("source_field_names", source->fieldNames());
    python->setGlobalObject("source_length",      source->length());
    python->setGlobalObject("source_width",       source->width());
    python->setGlobalObject("source_inputsize",   source->inputsize());
    python->setGlobalObject("source_targetsize",  source->targetsize());
    python->setGlobalObject("source_weightsize",  source->weightsize());

    // Set the parameters if any are defined
    if (! m_params.empty())
        setParams(m_params);
    
    // Get the new fieldnames (and the new width by the same token)
    TVec<string> fieldnames =
        python->invoke("getFieldNames", source->fieldNames()).as< TVec<string> >();
    length_ = source->length();
    width_  = fieldnames.size();
    declareFieldNames(fieldnames);
    
    // Obtain and set the new sizes if defined
    if (python->isInvokable("getSizes")) {
        TVec<int> sizes = python->invoke("getSizes",
                                       source->inputsize(),
                                       source->targetsize(),
                                       source->weightsize()).as< TVec<int> >();
        if (sizes.size() != 3)
            PLERROR("PythonProcessedVMatrix: the python function 'getSizes' must return\n"
                    "a vector of integers of size 3 (exactly), containing respectively\n"
                    "the new inputsize, targetsize and weightsize");
        inputsize_  = sizes[0];
        targetsize_ = sizes[1];
        weightsize_ = sizes[2];
    }
    
    // Finish building from source informations...
    setMetaInfoFromSource();

    // Finally, call the Python builder if one exists
    if (python->isInvokable("build"))
        python->invoke("build");
}


//#####  setParams  ###########################################################

void PythonProcessedVMatrix::setParams(const map<string,string>& params)
{
    m_params = params;
    python->setGlobalObject("params", params);
}


//#####  getNewRow  ###########################################################

void PythonProcessedVMatrix::getNewRow(int i, const Vec& v) const
{
    // Get the correct row from the source matrix
    sourcerow.resize(source->width());
    source->getRow(i, sourcerow);

    // Process this row using Python
    Vec processed = python->invoke("getRow", i, sourcerow).as<Vec>();

    // Ensure it has the correct size and copy into v
    if (processed.size() != v.size())
        PLERROR("PythonProcessedVMatrix: the Python function 'getRow' returned a vector "
                "of the wrong length; at row %d, obtained length=%d, expected length=%d",
                i, processed.size(), v.size());

    v << processed;
}


//#####  getSourceRow  ########################################################

PythonObjectWrapper
PythonProcessedVMatrix::getSourceRow(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 1)
        PLERROR("PythonProcessedVMatrix::getSourceRow: expected 1 argument; got %d",
                args.size());
    int i = args[0].as<int>();
    if (i < 0 || i >= source->length())
        PLERROR("PythonProcessedVMatrix::getSourceRow: desired row number %d is "
                "outside the range [0,%d).", i, source->length());

    sourcerow.resize(source->width());
    source->getRow(i, sourcerow);
    return sourcerow;
}


//#####  compileAndInject  ####################################################

void PythonProcessedVMatrix::compileAndInject()
{
    if (! python) {
        python = new PythonCodeSnippet(m_code);
        assert( python );
        python->build();
        python->inject("getSourceRow", this,
                       &PythonProcessedVMatrix::getSourceRow);
    }
}


//#####  makeDeepCopyFromShallowCopy  #########################################

void PythonProcessedVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // Need to recompile the Python code
    python = 0;
    compileAndInject();
}

} // end of namespace PLearn


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
