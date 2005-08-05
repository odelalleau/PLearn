// -*- C++ -*-

// diff.cc
//
// Copyright (C) 2005 Olivier Delalleau 
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

// Authors: Olivier Delalleau

/*! \file diff.cc */


#include "diff.h"
#include <plearn/base/Object.h>

namespace PLearn {
using namespace std;

///////////////////
// addDiffPrefix //
///////////////////
void addDiffPrefix(const string& prefix, vector<string>& diffs, int n) {
  assert( n >= 0 && n * 3 <= int(diffs.size()) );
  vector<string>::size_type i = diffs.size() - 3;
  for (int j = 0; j < n; j++, i -= 3)
    diffs[i] = prefix + diffs[i];
}

//////////
// diff //
//////////
int diff(const string& refer, const string& other, const OptionBase* opt, vector<string>& diffs)
{
  pout << "Calling basic diff with Option< ObjectType, " << opt->optiontype() << " >" << endl;
  return diff(refer, other, opt->optionname(), diffs);
}

//////////
// diff //
//////////
int diff(PP<Object> refer, PP<Object> other, vector<string>& diffs)
{
  int n_diffs =
    diff(refer->classname(), other->classname(), "classname", diffs);
  if (n_diffs > 0)
    return n_diffs; // We cannot compare two objects from different classes.
  OptionList& options = refer->getOptionList();
  for (OptionList::const_iterator it = options.begin(); it != options.end(); it++) {
    // pout << "Comparing " << (*it)->optionname() << endl;
    string option = (*it)->optionname();
    string refer_opt = refer->getOption(option);
    string other_opt = other->getOption(option);
    n_diffs += (*it)->diff(refer_opt, other_opt, diffs);
  }
  return n_diffs;
}

/////////////
// newDiff //
/////////////
int diff(const string& refer, const string& other, const string& name,
         vector<string>& diffs)
{
  if (refer != other) {
    diffs.push_back(name);
    diffs.push_back(refer);
    diffs.push_back(other);
    return 1;
  } else
    return 0;
}


} // end of namespace PLearn
