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
   * $Id: MultiInstanceVMatrix.cc,v 1.2 2004/02/25 05:20:34 nova77 Exp $ 
   ******************************************************* */

// Authors: Norman Casagrande

/*! \file MultiInstanceVMatrix.cc */

#include <map>
#include <algorithm>
#include <iterator>
#include "MultiInstanceVMatrix.h"
#include "SumOverBagsVariable.h"
#include "stringutils.h"
#include "fileutils.h"

namespace PLearn {
using namespace std;

MultiInstanceVMatrix::MultiInstanceVMatrix()
  :inherited(), data_(Mat())
{
  // ### You may or may not want to call build_() to finish building the object
  //build_();
}

//MultiInstanceVMatrix::MultiInstanceVMatrix(const string& filename) 
//  :inherited(), specification_(abspath(filename))
//{
//  //build();
//}


PLEARN_IMPLEMENT_OBJECT(MultiInstanceVMatrix, "Matrix designed for multi instance data", "NO HELP (yet)");

void MultiInstanceVMatrix::getRow(int i, Vec v) const
{
  // for the moment just call the super method
  //inherited::getRow(i, v);
}

void MultiInstanceVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "specification", &MultiInstanceVMatrix::specification_, OptionBase::buildoption,
    "This is any string understood by getDataSet. Typically a file or directory path");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void MultiInstanceVMatrix::build_()
{
  //this->setMetaDataDir(filename_ + ".metadata"); 

  // To be used in the end.. it is about 5 secs slower in debug
  //int nRows = countNonBlankLinesOfFile(specification_);

  ifstream inFile(specification_.c_str());
  if(!inFile)
    PLERROR("In MultiInstanceVMatrix could not open file %s for reading", specification_.c_str());

  string lastName = "";
  string newName;
  string aLine;
  string inp_element;
  int configNum, bagType;
  int i, nComp = 0;
  int lastColumn = inputsize_ + targetsize_;
  real* mat_i = NULL;

  int nRows = count(istreambuf_iterator<char>(inFile),
                    istreambuf_iterator<char>(), '\n');

  inFile.seekg(0);

  // Check the number of columns
  getline(inFile, aLine, '\n');
  int nFields = (int)split(aLine).size();
  if ( (nFields-1) != inputsize_ + targetsize_)
  {
    PLERROR("Either inputsize or targetsize are different from the specified file!\n"
            " Got %d+%d (inputsize+targetsize) = %d, and found %d!", 
            inputsize_, targetsize_, inputsize_+targetsize_, nFields - 1);
  }

  data_.resize(nRows, inputsize_ + targetsize_);

  inFile.seekg(0);

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

    // next column: the number of the compound
    inFile >> configNum;

    configs_.push_back(configNum);
    
    mat_i = data_[lineNum];
    for(i = 0; i < inputsize_ + targetsize_ - 1; i++)
    {
      inFile >> inp_element;
      mat_i[i] = strtod(inp_element.c_str(), 0);
    }
    mat_i[lastColumn] = bagType;

  }
  
  cout << "Passed!" << endl;
  this->setMtime(mtime(specification_));
  inFile.close();
}

// ### Nothing to add here, simply calls build_
void MultiInstanceVMatrix::build()
{
  inherited::build();
  build_();
}

void MultiInstanceVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // TODO: Copy also the other features

  // ### Remove this line when you have fully implemented this method.
  PLERROR("MultiInstanceVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn

