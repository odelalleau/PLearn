// -*- C++ -*-

// MultiInstanceVMatrix.cc
//
// Copyright (C) 2004 Norman Casagrande
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

// Authors: Norman Casagrande

/*! \file MultiInstanceVMatrix.cc */

#include "MultiInstanceVMatrix.h"
#include <map>
#include <algorithm>
#include <iterator>
#include <plearn/var/SumOverBagsVariable.h>
#include <plearn/base/stringutils.h>
#include <plearn/io/fileutils.h>
#include <plearn/io/openFile.h>

namespace PLearn {
using namespace std;

MultiInstanceVMatrix::MultiInstanceVMatrix()
    :inherited(), data_(Mat()), source_targetsize(1),
     header_lines_to_skip(0)
{
    // ### You may or may not want to call build_() to finish building the object
    //build_();
}

//MultiInstanceVMatrix::MultiInstanceVMatrix(const string& filename)
//  :inherited(), filename_(abspath(filename))
//{
//  //build();
//}


PLEARN_IMPLEMENT_OBJECT(MultiInstanceVMatrix, "Virtual Matrix for a multi instance dataset",
                        "In a multi-instance dataset examples come in 'bags' with only one target label\n"
                        "for each bag. This class is built upon a source text file that describes such\n"
                        "a dataset (see the help on the 'filename' option for format details).\n"
                        "The resulting VMatrix shows the following structure in its rows, with\n"
                        "all the rows of a bag being consecutive. Each row represents an instance and has:\n"
                        "  - the input features for the instance\n"
                        "  - the bag's source_targetsize target values (repeated over bag instances)\n"
                        "  - a bag signal integer that identifies the beginning and end of the bag:\n"
                        "     1 means the first instance of the bag\n"
                        "     2 means the last instance of the bag\n"
                        "     3 is for a bag with a single row (= 1+2)\n"
                        "     0 is for intermediate instances.\n"
                        "The targetsize of the VMatrix is automatically set to source_targetsize+1\n"
                        "since the bag_signal is included (appended) in the target vector\n"
    );

void MultiInstanceVMatrix::getNewRow(int i, const Vec& v) const
{
    v << data_(i);
}

void MultiInstanceVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "source_targetsize", &MultiInstanceVMatrix::source_targetsize, OptionBase::buildoption,
                  "The source targetsize");

    declareOption(ol, "filename", &MultiInstanceVMatrix::filename_, OptionBase::buildoption,
                  "This is the name of the ascii 'mimat' format filename. It is a supervised learning dataset\n"
                  "in which each input object can come in several instances (e.g. conformations) and the target is given to the\n"
                  "whole bag of these instances, not to individual instances. The expected format is the following:\n"
                  "Each row contains:\n"
                  "  - the object name (a string without white space)\n"
                  "  - the instance number (a non-negative integer)\n"
                  "  - the inputsize features for that instance (numeric, white-separated)\n"
                  "  - the source_targetsize target values for the bag (repeated on each row).\n"
                  "If the inputsize option is not specified it is inferred from the text file.\n"
        );

    declareOption(ol, "header_lines_to_skip", &MultiInstanceVMatrix::header_lines_to_skip, OptionBase::buildoption,
                  "The number of lines to skip at the beginning of the file (they may be garbage, or \n"
                  "a header for a TextFilesVMatrix for instance).");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void MultiInstanceVMatrix::build_()
{

    updateMtime(filename_);
    //this->setMetaDataDir(filename_ + ".metadata");

    // To be used in the end.. it is about 5 secs slower in debug
    //int nRows = countNonBlankLinesOfFile(filename_);
    
    PStream inFile = openFile(filename_, PStream::raw_ascii, "r");
//    PStream inFile = openFile(filename_, PStream::raw_ascii, "r");

    // inFile.seekg(0); TODO See if it still works without this.
    skipBlanksAndComments(inFile);

    string lastName = "";
    string newName,configNum;
    string aLine;
    string inp_element;
    int bagType;
    int nComp = 0;
	
    int i;

    real* mat_i = NULL;

    // one more column for the bag signal
    targetsize_ = source_targetsize + 1;

    // Check the number of columns
    for (i = 0; i < header_lines_to_skip; i++) {
        inFile.getline(aLine, '\n');
    }
    inFile.getline(aLine, '\n');
    vector<string> entries = split(aLine);
    int nFields = (int)entries.size();
    if (inputsize_>=0)
    {
        if ( (nFields-2) != inputsize_ + source_targetsize) // 2 for the object name and the instance number
        {
            PLERROR("Either inputsize or source_targetsize are inconsistent with the specified file!\n"
                    " Got %d+%d (inputsize+source_targetsize) = %d, and found %d! If unsure about inputsize, don't specify it or set to -1.",
                    inputsize_, source_targetsize, inputsize_+source_targetsize, nFields - 2);
        }
    } else inputsize_ = nFields-2-source_targetsize;

    int lastColumn = inputsize_ + source_targetsize;

    inFile = openFile(filename_, PStream::raw_ascii, "r");
    // inFile.seekg(0); // TODO See if it still works without this.
    skipBlanksAndComments(inFile);
    for (i = 0; i < header_lines_to_skip; i++) {
        inFile.getline(aLine, '\n');
    }
    skipBlanksAndComments(inFile);

    int nRows = inFile.count('\n');
    /* TODO Make sure it works like the old code:
       int nRows = count(istreambuf_iterator<char>(inFile),
       istreambuf_iterator<char>(), '\n');
    */

    inFile = openFile(filename_, PStream::raw_ascii, "r");
    // inFile.seekg(0); // TODO See if it still works without this.
    skipBlanksAndComments(inFile);
    for (i = 0; i < header_lines_to_skip; i++) {
        inFile.getline(aLine, '\n');
    }
    skipBlanksAndComments(inFile);

    data_.resize(nRows, inputsize_ + targetsize_);

    width_ = inputsize_ + targetsize_;
    length_ = nRows;

    for (int lineNum = 0; !inFile.eof() && lineNum < nRows; ++lineNum)
    {
        // first column: name of the compound
        inFile >> newName;
        if (newName != lastName)
        {
            lastName = newName;
            names_.push_back( make_pair(newName, lineNum) );
            bagType = SumOverBagsVariable::TARGET_COLUMN_FIRST;
            if (mat_i != NULL)
            {
                if (nComp > 1)
                    mat_i[lastColumn] = SumOverBagsVariable::TARGET_COLUMN_LAST;
                else
                    mat_i[lastColumn] = SumOverBagsVariable::TARGET_COLUMN_SINGLE;
            }
            nComp = 0;
        }
        else
        {
            bagType = SumOverBagsVariable::TARGET_COLUMN_INTERMEDIATE;
        }
        nComp++;

        // get next column: the number of the compound
        inFile >> configNum;

        configs_.push_back(configNum);

        // get the actual data columns + the target
        mat_i = data_[lineNum];
        for(int k = 0; k < inputsize_ + source_targetsize; k++)
        {
            inFile >> inp_element;
            mat_i[k] = pl_strtod(inp_element.c_str(), 0);
        }

        // close the last bag if necessary
        if (lineNum+1==nRows)
        {
            if (nComp > 1)
                mat_i[lastColumn] = SumOverBagsVariable::TARGET_COLUMN_LAST;
            else
                mat_i[lastColumn] = SumOverBagsVariable::TARGET_COLUMN_SINGLE;
        }
        else
            mat_i[lastColumn] = bagType;
    }


    //ofstream test("g:/test.txt");
    //test << data_;
    //test.close();

    this->updateMtime(filename_);
}

// ### Nothing to add here, simply calls build_
void MultiInstanceVMatrix::build()
{
    inherited::build();
    build_();
}

void MultiInstanceVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:

    deepCopyField(data_, copies);

    // TODO: Copy also the other features

    // ### Remove this line when you have fully implemented this method.
    PLERROR("MultiInstanceVMatrix::makeDeepCopyFromShallowCopy not fully implemented yet!");
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
