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
   * $Id$
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

#include "getDataSet.h"
#include <plearn/io/MatIO.h>              //!< For loadAsciiSingleBinaryDescriptor().

#include <plearn/base/stringutils.h>      //!< For split_on_first().
#include <plearn/io/fileutils.h>          //!< For isfile().
#include <plearn/io/pl_log.h>
#include <plearn/io/PyPLearnScript.h>
#include <plearn/vmat/DiskVMatrix.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/vmat/VMat.h>
#include <plearn/vmat/VVMatrix.h>

namespace PLearn {
using namespace std;

////////////////////
// getDataSetDate //
////////////////////
time_t getDataSetDate(const PPath& dataset_path)
{
  return getDataSet(dataset_path)->getMtime();
}


///////////////////////////////
// extractDataSetNameAndArgs //
///////////////////////////////
void extractDataSetNameAndArgs(const PPath&         dataset_full,
                               PPath&               dataset_base,
                               vector<string>&      args_vec,
                               map<string, string>& args_map)
{
  args_vec.clear();
  args_map.clear();
  // First try with url-like parameters.
  dataset_full.parseUrlParameters(dataset_base, args_map);
  if (!args_map.empty()) {
    map<string, string>::const_iterator it = args_map.begin();
    for (; it != args_map.end(); it++)
      args_vec.push_back(it->first + "=" + it->second);
    return;
  }
  // No url-like parameters were found, trying the second format (see .h).
  string dataset_abs = dataset_full.absolute();
  string::size_type pos_of_double_colon = dataset_abs.find("::");
  if (pos_of_double_colon == string::npos) {
    // No parameters at all.
    dataset_base = dataset_full;
    return;
  }

  dataset_base = dataset_abs.substr(0, pos_of_double_colon);
  string dataset_args = dataset_abs.substr(pos_of_double_colon+2, dataset_abs.length());

  // TODO Use the PLearn::split() method instead.
  // Split dataset_args into a vector<string> on "::".
  string::size_type pos_of_arg_start = 0;
  pos_of_double_colon = dataset_args.find("::", pos_of_arg_start);
  while (pos_of_double_colon != string::npos) {
    string a = dataset_args.substr(pos_of_arg_start, pos_of_double_colon-pos_of_arg_start);
    args_vec.push_back(a);
    pos_of_arg_start = pos_of_double_colon + 2;
    pos_of_double_colon = dataset_args.find("::", pos_of_arg_start);
  }
  if (pos_of_arg_start != pos_of_double_colon)
    // Append the last argument
    args_vec.push_back(dataset_args.substr(pos_of_arg_start, dataset_args.size()-pos_of_arg_start));
  string name, value;
  vector<string>::const_iterator it = args_vec.begin();
  for (; it != args_vec.end(); it++) {
    PLearn::split_on_first(*it, "=", name, value);
    args_map[name] = value;
  }
}

////////////////
// getDataSet //
////////////////
VMat getDataSet(const PPath& dataset_path)
{
  VMat vm;
  // Parse the base file name and the potential parameters.
  PPath dataset;
  vector<string> args_vec;
  map<string, string> args_map;
  extractDataSetNameAndArgs(dataset_path, dataset, args_vec, args_map);

  // Supported formats: .amat .pmat .vmat .txtmat .pymat (file)
  //                    .dmat                            (directory)
  string ext = dataset.extension();
  if (isfile(dataset)) {
    if (ext == "amat") {
      // Check if the extension is ".bin.amat".
      if (dataset.find(".bin.", ((unsigned int) dataset.size()) - 9) != string::npos) {
        // TODO Ask PJ if he still uses this hack.
        PLDEPRECATED("In getDataSet - Do we really need .bin.amat files ?");
        Mat tempMat;
        loadAsciiSingleBinaryDescriptor(dataset,tempMat);
        vm = VMat(tempMat);
      } else
        vm = loadAsciiAsVMat(dataset);
    } else if (ext == "pmat") {
      vm = new FileVMatrix(dataset);
    } else if (ext == "vmat" || ext == "txtmat") {
      const string code = readFileAndMacroProcess(dataset, args_map);
      if (removeblanks(code)[0] == '<') {
        // Old XML-like format.
        PLDEPRECATED("In getDataSet - File %s is using the old XML-like VMat format, " 
                     "you should switch to a PLearn script (ideally a .pymat file).",
                     dataset.absolute().c_str());
        vm = new VVMatrix(dataset);
      } else {
        vm = dynamic_cast<VMatrix*>(newObject(code));
        if (vm.isNull())
          PLERROR("In getDataSet - Object described in %s is not a VMatrix subclass",
                  dataset.absolute().c_str());
      }
    } else if (ext == "pymat" || ext == "py") {
      if (ext == ".py")
        PLWARNING("In getDataSet - Note that the Python code in a '.py' file must return a pl.VMatrix");
      PP<PyPLearnScript> pyplearn_script = PyPLearnScript::process(dataset, args_vec);
      const string code = pyplearn_script->getScript();
      vm = dynamic_cast<VMatrix*>(newObject(code));
      if (vm.isNull())
        PLERROR("In getDataSet - Object described in %s is not a VMatrix subclass",
                dataset.absolute().c_str());
     }
    else 
      PLERROR("In getDataSet - Unknown extension for VMat file: %s", ext.c_str());
    // Set default metadata directory if not already set.
    if (!vm->hasMetaDataDir())
      vm->setMetaDataDir(dataset.dirname() / (dataset.basename() + ".metadata"));
  } else if (isdir(dataset)) {
    if (ext == "dmat")
      vm = new DiskVMatrix(dataset);
    else
      PLERROR("In getDataSet - Unknown extension for VMat directory: %s", ext.c_str());
  }
  
  vm->loadAllStringMappings();
  vm->unduplicateFieldNames();

  if (vm->inputsize() < 0 && vm->targetsize() < 0 && vm->weightsize() < 0) {
    DBG_LOG << "In getDataSet - The loaded VMat has no inputsize, targetsize "
            << "or weightsize specified, setting them to (" << vm->width() << ",0,0)"
            << endl;
    vm->defineSizes(vm->width(), 0, 0);
  }

  // Ensure sizes do not conflict with width.
  if (vm->inputsize() >= 0 && vm->targetsize() >= 0 && vm->weightsize() >= 0 &&
      vm->width() >= 0 &&  vm->width() < vm->inputsize() + vm->targetsize() + vm->weightsize())
    PLERROR("In getDataSet - The matrix width (%d) should not be smaller than inputsize (%d) "
            "+ targetsize (%d) + weightsize (%d)",
            vm->width(), vm->inputsize(), vm->targetsize(), vm->weightsize());

  return vm;
}

////////////////////
// getDataSetHelp //
////////////////////
string getDataSetHelp() {
  return "Dataset specification must be either:\n"
         "- a file with extension:      .amat .pmat .vmat .txtmat .pymat\n"
         "- a directory with extension: .dmat\n"
         "Optionally, arguments for scripts can be given with the following syntax:\n"
         "  path/file.ext?arg1=val1&arg2=val2&arg3=val3\n";
}

} // end of namespace PLearn
