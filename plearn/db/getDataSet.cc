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
   * $Id: getDataSet.cc,v 1.44 2005/06/14 20:27:05 chrish42 Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

#include "AutoSDBVMatrix.h"
#include <plearn/vmat/ConcatRowsVMatrix.h>      //!< For vconcat.
#include "databases.h"              //!< For loadClassificationDataset.
#include <plearn/vmat/DiskVMatrix.h>
#include <plearn/vmat/FileVMatrix.h>
#include "getDataSet.h"
#include <plearn/vmat/StrTableVMatrix.h>
#include <plearn/base/StringTable.h>
#include <plearn/vmat/VMat.h>
#include <plearn/vmat/VVMatrix.h>
#include <plearn/io/MatIO.h>
#include <plearn/io/PyPLearnScript.h>
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;


string getDataSetHelp()
{
  return "Dataset specification can be one of: \n"
    " - the path to a matrix file (or directory) .amat .pmat .vmat .dmat or plain ascii \n"
    " - the basename of an .sdb \n"
    " - the object specification of a VMatrix subclass \n"
    " - the specification of a preprogrammed dataset i.e. one of the datasetnames below,\n"
    "   followed by the word 'train' or 'test', optionally followed by the word 'normalize'\n"
    + loadClassificationDatasetHelp()  
    + loadUCIDatasetsHelp();
}

time_t getDataSetDate(const string& datasetstring, const string& alias)
{
  VMat vm;

  // search for an alias in a dataset.aliases file
  map<string,string> aliases = getDatasetAliases(PPath::getcwd());
  if(aliases.find(datasetstring)!=aliases.end())
    return getDataSetDate(aliases[datasetstring]);

  if(isfile(datasetstring+".sdb")) // it's an sdb
    return mtime(datasetstring+".sdb");
  else if(pathexists(datasetstring))
    {
      if(isfile(datasetstring))
        {
          if(extract_extension(datasetstring)==".vmat")
            return VVMatrix::getDateOfVMat(datasetstring);
          else return mtime(datasetstring);
        }
      else // it's a directory
        {
          // is it the directory of a DiskVMatrix?
          if(isfile(datasetstring+slash+"indexfile") && isfile(datasetstring+slash+"0.data"))
            return mtime(datasetstring+slash+"indexfile");
          else
            PLERROR("In getDataSetDate: datasetstring is a directory of unknown format"); 
        }
    }

  // Otherwise, it could be a preprogrammed dataset, or just about anything...
  // we don't really know. So we return 0.
  return 0;
}


/** Extracts the dataset and arguments from the string passed
    to getDataSet() and friends.

    The expected format is filename.ext::ARG1=value1:ARG2=value2 ...
*/
void extractDataSetNameAndArgs(const string& datasetString,
                               string& dataset,
                               vector<string>& args)
{
  args.clear();

  string::size_type pos_of_double_colon = datasetString.find("::");
  if (pos_of_double_colon == string::npos) {
    dataset = datasetString;
    return;
  }

  dataset = datasetString.substr(0, pos_of_double_colon);
  string datasetArgs = datasetString.substr(pos_of_double_colon+2, datasetString.length());

  // Split datasetArgs into a vector<string> on "::".
  string::size_type pos_of_arg_start = 0;
  pos_of_double_colon = datasetArgs.find("::", pos_of_arg_start);
  while (pos_of_double_colon != string::npos) {
    string a = datasetArgs.substr(pos_of_arg_start, pos_of_double_colon-pos_of_arg_start);
    args.push_back(a);

    pos_of_arg_start = pos_of_double_colon + 2;
    pos_of_double_colon = datasetArgs.find("::", pos_of_arg_start);
  }
  if (pos_of_arg_start != pos_of_double_colon) {
    // Append the last argument
    args.push_back(datasetArgs.substr(pos_of_arg_start, datasetArgs.size()-pos_of_arg_start));
  }
}


VMat getDataSet(const char*   datasetstring, const string& alias)
{
  return getDataSet(string(datasetstring), alias);
}

VMat getDataSet(const PPath&  datasetpath,   const string& alias)
{
  return getDataSet(string(datasetpath.absolute()), alias);
}

VMat getDataSet(const string& datasetstring, const string& alias)
{
  // search for an alias in a dataset.aliases file
  map<string,string> aliases = getDatasetAliases(PPath::getcwd());
  if(aliases.find(datasetstring)!=aliases.end())
    return getDataSet(aliases[datasetstring]);

  // it wasn't an alias
  string dataset;
  vector<string> datasetArgs;
  extractDataSetNameAndArgs(datasetstring, dataset, datasetArgs);

  VMat vm;
  if (isfile(dataset + ".sdb")) // it's an sdb (without the .sdb extension...)
    {
      vm = new AutoSDBVMatrix(dataset);
    }
  else if (pathexists(dataset))
    {
      if (isfile(dataset))
        {
          string ext = extract_extension(dataset);
          if (ext == ".pmat")
            vm = new FileVMatrix(dataset);
          else if (ext==".vmat" || ext==".txtmat")
          {
            /* Convert datasetArgs vector of arguments into a map from
               argument name to argument value for
               readFileAndMacroProcess */
            map<string, string> vars;
            for (vector<string>::const_iterator it = datasetArgs.begin();
                 it != datasetArgs.end(); ++it) {
              string arg_name, arg_value;
              split_on_first(*it, "=", arg_name, arg_value);
              vars[arg_name] = arg_value;
            }

            string code = readFileAndMacroProcess(dataset, vars);
            if (removeblanks(code)[0] == '<') // old xml-like format 
              vm = new VVMatrix(dataset);
            else
            {
              vm = dynamic_cast<VMatrix*>(newObject(code));
              if(vm.isNull())
                PLERROR("getDataSet: Object described in %s is not a VMatrix subclass",dataset.c_str());
            } 
          }
          else if (ext==".pymat" || ext==".py")
          {
            if (ext==".py")
              PLWARNING("getDataSet: Note that the Python code in a '.py' file must return a pl.VMatrix");
            PP<PyPLearnScript> pyplearn_script = PyPLearnScript::process(dataset, datasetArgs);
            const string code = pyplearn_script->getScript();
            vm = dynamic_cast<VMatrix*>(newObject(code));
            if (vm.isNull())
                PLERROR("getDataSet: Object described in %s is not a VMatrix subclass",dataset.c_str());
          }
          else if (ext==".amat") {
            // Check if the extension is ".bin.amat".
            if (dataset.find(".bin.", ((unsigned int) dataset.size()) - 9) != string::npos){
              Mat tempMat;
              loadAsciiSingleBinaryDescriptor(dataset,tempMat);
              vm = VMat(tempMat);
            } else {
              vm = loadAsciiAsVMat(dataset);
            }
          }
          else if (ext==".strtable")
            vm = new StrTableVMatrix(StringTable(dataset));
          else if (ext==".sdb")
            vm = new AutoSDBVMatrix(remove_extension(dataset));            
          else if (ext==".mat")
            vm = loadAsciiAsVMat(dataset);
          else 
            PLERROR("Unknown extension for vmatrix: %s", ext.c_str());
          if (!vm->hasMetaDataDir())
            vm->setMetaDataDir(extract_directory(dataset) + extract_filename(dataset) + ".metadata");
        }
      else // it's a directory
      {
          // is it the directory of a DiskVMatrix?
          if (isfile(dataset + slash + "indexfile") && isfile(dataset + slash + "0.data"))
            {
              vm = new DiskVMatrix(dataset);
            }
          else
            PLERROR("In getDataSet: dataset is a directory of unknown format"); 
        }
    }
  else // it's either a preprogrammed dataset, or a VMatrix object
  {
    try // try with a preprogrammed dataset
      {
        vector<string> dsetspec = split(dataset);
        if (dsetspec.size() < 2)
          PLERROR("In getDataSet, expecting a specification of the form '<datasetname> <train|test|all> [normalize]. DatasetString = %s' ",dataset.c_str());
        string datasetname = dsetspec[0];
        bool normalizeinputs = false;
        if (dsetspec.size() >= 3)
          {
            if (dsetspec[2] == "normalize")
              normalizeinputs = true;
            else PLERROR("In getDataSet specification of predefined dataset contains 3 words, expecting 3rd one to be 'normalize', don't understand '%s'",dsetspec[2].c_str());
          }
        
        int inputsize, nclasses;
        VMat trainset, testset, allset;
        loadClassificationDataset(datasetname, inputsize, nclasses, trainset, testset, normalizeinputs, allset);
        if (dsetspec[1] == "train") {
          if (trainset == NULL) {
            PLERROR("In getDataSet, there is no trainset available.");
          }
          vm = trainset;
        }
        else if (dsetspec[1] == "test") {
          if (testset == NULL) {
            PLERROR("In getDataSet, there is no testset available.");
          }
          vm = testset;
        }
        else if (dsetspec[1] == "all") {
          if (allset) {
            vm = allset;
          }
          else {
            vm = vconcat(trainset,testset);    
          }
        }
        else 
          PLERROR("In getDataSet specification of predefined dataset: expecting second word to be 'train' or 'test' or 'all' not %s ...",dsetspec[1].c_str());
        vm->defineSizes(inputsize, 1);
        // Set metadatadir depending on the METADATADIR variable.
#ifdef METADATADIR
        string mdir = METADATADIR;
#else
        string mdir = "/u/lisa/db/metadata/";
#endif
        vm->setMetaDataDir(append_slash(mdir) + dataset);
      }
    catch (const PLearnError& e)  // OK, it wasn't a preprogrammed dataset, let's try with a VMatrix object
      {
        try 
          { 
            vm = dynamic_cast<VMatrix*>(newObject(dataset));
            if (!vm)
              PLERROR("Not a VMatrix object (dynamic cast failed)");
          }
        catch (const PLearnError& e2)
          {
            PLERROR("Error in getDataSet with specification: %s\n"
                    "Specification is neither a valid file or directory \n"
                    "Nor is it a preprogrammed dataset (attempt returned: %s)\n"
                    "Nor could it be resolved to a VMatrix object (attempt returned: %s)\n",
                    dataset.c_str(), e.message().c_str(), e2.message().c_str());
          }
      }
  }
  
  vm->loadAllStringMappings();
  // vm->setAlias(alias); // Aliases are now deprecated.
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


string locateDatasetAliasesDir(const PPath& dir_or_file_path)
{
  if(!pathexists(dir_or_file_path))
    PLERROR("In getDatasetAliases argument '%s' is not an existing directory or file!", dir_or_file_path.absolute().c_str());
  string dirname = extract_directory(dir_or_file_path.absolute());
  string dot = ".";
  while(dirname!=slash && dirname!=dot+slash && !isfile(dirname + "dataset.aliases"))
    dirname = extract_directory(remove_trailing_slash(dirname));

  if(isfile(dirname+"dataset.aliases"))
    return dirname;
  else 
    return "";
}

//! Looks for 'dataset.aliases' file in specified directory and its parent directories;
//! loads it and returns the corresponding map. Returns an empty map if file was not found.
map<string,string> getDatasetAliases(const string& dir_or_file_path)
{
  map<string,string> aliases;
  string dirname = locateDatasetAliasesDir(dir_or_file_path);
  if(!dirname.empty() && isfile(dirname+"dataset.aliases"))
    {
      string fpath = dirname+"dataset.aliases";
      ifstream in(fpath.c_str());
      if(!in)
        PLERROR("Could not open %s for reading", fpath.c_str());
      while(in)
        {
          string alias;
          getline(in,alias,'=');
          alias = removeblanks(alias);
          if(alias.empty())
            break;
          string datasetdef;
          PLearn::read(in,datasetdef);
          aliases[alias] = datasetdef;
          in >> ws;//skipBlanks(in);
          if(in.peek()==';')
            in.get();
        }
    }
  return aliases;
}


} // end of namespace PLearn
