// -*- C++ -*-

// MultiInstanceVMatrix.cc KNN VMatrix!
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
   * $Id: MultiInstanceVMatrix.cc,v 1.1 2004/02/24 20:44:27 nova77 Exp $ 
   ******************************************************* */

// Authors: Norman Casagrande

/*! \file MultiInstanceVMatrix.cc */

#include <map>
#include <algorithm>
#include <iterator>
#include <sstream>
#include "MultiInstanceVMatrix.h"

namespace PLearn {
using namespace std;

MultiInstanceVMatrix::MultiInstanceVMatrix()
  :inherited(), specification_("")
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
  inherited::getRow(i, v);
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

  ifstream inFile(specification_.c_str());
  if(!inFile)
    PLERROR("In MultiInstanceVMatrix could not open file %s for reading", specification_.c_str());

  string lastName = "";
  string newName;
  string aLine;
  string inp_element;
  int configNum;
  int i;
  real* mat_i;

  int nRows = count(istreambuf_iterator<char>(inFile),
                    istreambuf_iterator<char>(), '\n');

  data.resize(nRows, inputsize_ + targetsize_);
  stringstream ss (stringstream::in | stringstream::out);

  inFile.seekg(0);

  for (int lineNum = 0; !inFile.eof() && lineNum < nRows; ++lineNum)
  {
    getline(inFile, aLine, '\n');
    ss << aLine;

    // first column: name of the compound
    ss >> newName;
    if (newName != lastName)
    {
      lastName = newName;
      names_.push_back( make_pair(newName, lineNum) );
    }

    // next column: the number of the compound
    ss >> configNum;
    configs_.push_back(configNum);
    
    mat_i = data[lineNum];
    for(i = 0; i < inputsize_ + targetsize_; i++)
    {
      ss >> inp_element;
      mat_i[i] = strtod(inp_element.c_str(), 0);
    }

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

