// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: getDataSet.h,v 1.5 2005/02/08 21:34:27 tihocan Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef getDataSet_INC
#define getDataSet_INC

#include <map>
#include <string>
#include <plearn/vmat/VMat.h>

namespace PLearn {
using namespace std;


//! returns a help describing the datasetstring parameter of getDataSet
string getDataSetHelp();

time_t getDataSetDate(const string& datasetstring, const string& alias="");

/*! datasetstring can be one of the following:
  - the name of a preprogrammed dataset (possibly with parameter specification)
  - the path of the basename of an .sdb 
  - the path of a file in one of the recognized matrix data formats
  - the path of a directory containing a dataset 
  - the name of an alias in the dataset.aliases file of the current directory or one of its parents
  alias is a short name that can be used as part of a filename containing results related to the dataset
  ( it's set using the VMat's setAlias method, and code that wishes to use it can acces it by calling getAlias)
 */
VMat getDataSet(const char*   datasetstring, const string& alias="");
VMat getDataSet(const string& datasetstring, const string& alias="");
VMat getDataSet(const PPath&  datasetpath,   const string& alias="");

//! Looks for 'dataset.aliases' file in specified directory and its parent directories;
//! Returns the directory containing dataset.aliases (returned string will be terminated 
//! by a slash) or an empty string if not found.
string locateDatasetAliasesDir(const PPath& dir_or_file_path=".");

//! Looks for 'dataset.aliases' file in specified directory and its parent directories;
//! loads it and returns the corresponding map. Returns an empty map if file was not found.
map<string,string> getDatasetAliases(const string& dir_or_file_path=".");

} // end of namespace PLearn

#endif

