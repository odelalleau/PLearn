// -*- C++ -*-

// TestDependencyCommand.cc
// 
// Copyright (C) 2003 Pascal Vincent
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
   * $Id: TestDependencyCommand.cc,v 1.3 2004/09/27 20:19:16 plearner Exp $
   ******************************************************* */

/*! \file TestDependencyCommand.cc */
#include "TestDependencyCommand.h"
#include <plearn/db/getDataSet.h>
#include <plearn/math/stats_utils.h>
#include <plearn/vmat/VMat_basic_stats.h>
#include <plearn/sys/procinfo.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'TestDependencyCommand' command in the command registry
PLearnCommandRegistry TestDependencyCommand::reg_(new TestDependencyCommand);

//! The actual implementation of the 'TestDependencyCommand' command 
void TestDependencyCommand::run(const vector<string>& args)
{
  if(args.size()<3 || args.size()>3)
    PLERROR("test-dependencies expects 3 arguments, check the help");

  VMat data = getDataSet(args[0]);
  string x_spec = args[1];
  string y_spec = args[2];
  int x_col=0, y_col=0;
  if (x_spec[0]!='@')
    x_col = toint(x_spec);
  else {
    string x_name = x_spec.substr(1,x_spec.length()-1);
    x_col = data->fieldIndex(x_name);
    if (x_col<0) PLERROR("could not find field named %s in %s",x_name.c_str(),args[0].c_str());
  }
  if (y_spec[0]!='@')
    y_col = toint(y_spec);
  else {
    string y_name = y_spec.substr(1,y_spec.length()-1);
    y_col = data->fieldIndex(y_name);
    if (y_col<0) PLERROR("could not find field named %s in %s",y_name.c_str(),args[0].c_str());
  }

  // extract the two columns
  TVec<int> columns(2);
  columns[0]=x_col;
  columns[1]=y_col;
  Mat xy_mat = data.columns(columns).toMat();
  VMat x = VMat(xy_mat.column(0));
  VMat y = VMat(xy_mat.column(1));

  Mat spearman_pvalue(1,1);
  Mat spearman_r(1,1);
  testSpearmanRankCorrelation(x,y,spearman_r,spearman_pvalue);
  Mat linear_pvalue(1,1);
  Mat linear_r(1,1);
  correlations(x,y,linear_r,linear_pvalue);

  cout << "test-dependency between " << data->fieldName(x_col) << " (column " << x_col << ") and "
       <<  data->fieldName(y_col) << " (column " << y_col << "):" << endl;
  cout << "rank correlation = " << spearman_r(0,0) << " {p-value = " << spearman_pvalue(0,0) << "}" << endl;
  cout << "linear correlation = " << linear_r(0,0) << " {p-value = " << linear_pvalue(0,0) << "}" << endl;
}

} // end of namespace PLearn

