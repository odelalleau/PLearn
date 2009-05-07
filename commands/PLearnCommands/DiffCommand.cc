// -*- C++ -*-

// DiffCommand.cc
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

/*! \file DiffCommand.cc */


#include "DiffCommand.h"
#include <plearn/base/Object.h>
#include <plearn/base/PLearnDiff.h>
#include <plearn/io/PyPLearnScript.h>
#include <plearn/math/TVec_decl.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'DiffCommand' command in the command registry
PLearnCommandRegistry DiffCommand::reg_(new DiffCommand);

DiffCommand::DiffCommand():
    PLearnCommand("diff",
        "Compare PLearn objects",
        prgname() + " diff <reference.psave> <other_1.psave> ..."
                    "<other_n.psave> "
                    "[ <tolerance> [ <relative_tolerance> ] ]\n"
        "The files with the objects' specifications are given in argument,\n"
        "the first one being the reference object.\n"
        "If 'tolerance' is specified, it is taken as the absolute tolerance\n"
        "(when two numbers are less than 1), and as the relative tolerance\n"
        "(when one of two numbers is more than 1) unless "
        "'relative_tolerance'\n"
        "is also specified.\n"
    )
{}

//! The actual implementation of the 'DiffCommand' command 
void DiffCommand::run(const vector<string>& args)
{
    const char* error_msg = "In DiffCommand::run - You need to provide at "
                            "least two file names";
    if (args.size() < 2) PLERROR(error_msg);
    // Parse arguments.
    // First check whether some tolerance is given.
    real absolute_tolerance = ABSOLUTE_TOLERANCE;
    real relative_tolerance = RELATIVE_TOLERANCE;
    real tol;
    string tol_str = args[args.size() - 1];
    int to_ignore = 0;  // Number of arguments to ignore at end of 'args'.
    if (pl_isnumber(tol_str, &tol)) {
        relative_tolerance = tol;
        tol_str = args[args.size() - 2];
        to_ignore++;
        if (pl_isnumber(tol_str, &tol)) {
            to_ignore++;
            absolute_tolerance = tol;
        } else
            absolute_tolerance = relative_tolerance;
    }
    // Then read the object specifications paths.
    TVec<PPath> obj_spec;
    int n = int(args.size()) - to_ignore;
    if (n < 2) PLERROR(error_msg);
    for (vector<string>::size_type i = 0; i<vector<string>::size_type(n); i++)
        obj_spec.append(args[i]);
    // Load objects.
    TVec< PP<Object> > obj;
    for (int i = 0; i < n; i++) {
        PP<Object> new_object = smartLoadObject(obj_spec[i]);

        if (!new_object)
            PLERROR("In DiffCommand::run - Unable to serialize file %s as an Object",
                    obj_spec[i].absolute().c_str());
        obj.append(new_object);
    }
    // Compare objects.
    PStream& out = pout;
    PP<Object> refer = obj[0];
    PP<PLearnDiff> diffs = new PLearnDiff();
    diffs->absolute_tolerance = absolute_tolerance;
    diffs->relative_tolerance = relative_tolerance;
    for (int i = 1; i < n; i++) {
        PP<Object> other = obj[i];
        diffs->forget();
        int n_diffs = diff(refer, other, diffs);
        if (n_diffs > 0) {
            out << "Reference (" << obj_spec[0] << ") and object " << i << " ("
                << obj_spec[i] << ") differ:" << endl;
            diffs->printDiffs(out);
        }
        /*
          if (!diffs.empty()) {
          for (vector<string>::size_type j = 0; j < diffs.size(); j += 3)
          out << "  " << diffs[j] << " = " << diffs[j+1] << " != " << diffs[j+2] << endl;
          }
        */
    }
}

/*
/////////////
// newDiff //
/////////////
void DiffCommand::newDiff(vector<string>& diffs, const string& name,
const string& val1,  const string& val2)
{
static TVec<string> new_diff;
new_diff.resize(3);
new_diff[0] = name;
new_diff[1] = val1;
new_diff[2] = val2;
diffs.appendRow(new_diff);
}
*/

/*
//////////
// diff //
//////////
int DiffCommand::diff(PP<Object> refer, PP<Object> other,
vector<string>& diffs)
{
int n_diffs = 0;
OptionBase::newDiff("classname", refer->classname(),
other->classname(), diffs, n_diffs);
if (n_diffs > 0)
return n_diffs; // We cannot compare two objects from different classes.
OptionList& options = refer->getOptionList();
for (OptionList::const_iterator it = options.begin(); it != options.end(); it++) {
// pout << "Comparing " << (*it)->optionname() << endl;
n_diffs += (*it)->isDiff(refer->getOption((*it)->optionname()),
other->getOption((*it)->optionname()),
diffs);
}
return n_diffs;
}
*/

/*
  bool DiffCommand::diff(PP<Object> refer, PP<Object> other, PP<OptionBase> opt,
  vector<string>& diffs)
  {
  string optionname = opt->optionname();
  string optiontype = opt->optiontype();
  string refer_opt = refer->getOption(optionname);
  string other_opt = other->getOption(optionname);
  // pout << "Comparing " << opt->optionname() << ":" << endl;
  // pout << "Refer = " << refer_opt << endl;
  // pout << "Other = " << other_opt << endl;
  return diff(refer_opt, other_opt, optionname, optiontype, diffs);
  // pout << "Total: " << diffs.length() << " differences" << endl;
  }

  bool DiffCommand::diff(const string& refer, const string& other, const string& name,
  const string& type, vector<string>& diffs)
  {
  bool is_diff;
  if (type == "double") {
  double val_refer = todouble(refer);
  double val_other = todouble(other);
  is_diff = !is_equal(val_refer, val_other);
  } else if (type == "float") {
  float val_refer = tofloat(refer);
  float val_other = tofloat(other);
  is_diff = !is_equal(val_refer, val_other);
  } else
  // Default: just compare the strings.
  is_diff = (refer != other);
  if (is_diff)
  newDiff(diffs, name, refer, other);
  return is_diff;
  }
*/

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
