// -*- C++ -*-

// Copyright (c) 2002 by Julien Keable

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

#ifndef STRTABLE_INC
#define STRTABLE_INC

#include <string>
#include <vector>
#include <map>
#include <list>
#include <plearn/math/TMat.h>

namespace PLearn {
using namespace std;


using namespace std;


class StringTable
{
  TMat<string> data;
  vector<string> fieldnames; // fieldnames
  map<string, int> rev_fn;   // maps fieldnames to column numbers

public:
  /* construct a string table from a '.strtable' file
     File format is:

     #size: length width [optionnal]
     #: fieldname1 fieldname2 ... [optionnal]
     f1;f2;f3
     f1;f2;f3
     ...
     
     e.g:
     #:date bench machine compiler compilerOpt custCompilation custExecution benchOpt userTime realTime result
     2002/08/20 15:43:47;nearest;P3;pymake;-opt;;;-nrepeat 1 -ndata 1000 -nfeat 50;1.11;1.11;3.08696
     2002/08/20 15:43:47;nearest;P3;pymake;-opt;;;-nrepeat 1 -ndata 1000 -nfeat 100;2.13;2.13;8.36876
     ...
  */


  StringTable(const string & filename);
  StringTable();
  inline int length() const {return data.length();}
  inline int width() const {return data.width();}
  inline string& operator()(int i,int j){return data(i,j);}
  inline TVec<string> operator()(int i)const {return data(i);}
  // next 2 functions are to construct a string table on the fly
  // (don't bother with them if you're only loading .strtable files)
  void appendRow(const list<pair<string,string> >& row);
  void declareFields(const list<pair<string,string> > &row);
  friend ostream& operator<<(ostream& out,const StringTable& st);
  string getFieldName(int i)const {return fieldnames[i];}
};

} // end of namespace PLearn


#endif
