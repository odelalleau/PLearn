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
   * $Id: getDataSet.cc,v 1.20 2004/06/18 16:39:19 tihocan Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

//#include "MatIO.h"
#include "AutoSDBVMatrix.h"
//#include "stringutils.h"
//#include "fileutils.h"
#include "ConcatRowsVMatrix.h"      //!< For vconcat.
#include "databases.h"              //!< For loadClassificationDataset.
#include "DiskVMatrix.h"
#include "FileVMatrix.h"
#include "getDataSet.h"
//#include "ConcatColumnsVMatrix.h"
//#include "SubVMatrix.h"
#include "StrTableVMatrix.h"
#include "StringTable.h"
#include "VMat.h"
#include "VVMatrix.h"

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
    + loadClassificationDatasetHelp() + "\n";
}

time_t getDataSetDate(const string& datasetstring, const string& alias)
{
  VMat vm;

  // search for an alias in a dataset.aliases file
  map<string,string> aliases = getDatasetAliases(getcwd());
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

VMat getDataSet(const string& datasetstring, const string& alias)
{
  // search for an alias in a dataset.aliases file
  map<string,string> aliases = getDatasetAliases(getcwd());
  if(aliases.find(datasetstring)!=aliases.end())
    return getDataSet(aliases[datasetstring]);

  // it wasn't an alias
  VMat vm;
  if(isfile(datasetstring+".sdb")) // it's an sdb (without the .sdb extension...)
    {
      vm = new AutoSDBVMatrix(datasetstring);
    }
  else if(pathexists(datasetstring))
    {
      if(isfile(datasetstring))
        {
          string ext = extract_extension(datasetstring);
          if(ext==".pmat")
            vm = new FileVMatrix(datasetstring);
          else if(ext==".vmat" || ext==".txtmat")
          {
            string code = readFileAndMacroProcess(datasetstring);
            if(removeblanks(code)[0]=='<') // old xml-like format 
              vm = new VVMatrix(datasetstring);
            else
            {
              vm = dynamic_cast<VMatrix*>(newObject(code));
              if(vm.isNull())
                PLERROR("Object described in %s is not a VMatrix subclass",datasetstring.c_str());
            } 
          }
          else if(ext==".amat") {
            // Check if the extension is ".bin.amat".
            if (datasetstring.find(".bin.", ((unsigned int) datasetstring.size()) - 9) != string::npos){
              Mat tempMat;
              loadAsciiSingleBinaryDescriptor(datasetstring,tempMat);
              vm = VMat(tempMat);
            } else {
              vm = loadAsciiAsVMat(datasetstring);
            }
          }
          else if(ext==".strtable")
            vm = new StrTableVMatrix(StringTable(datasetstring));
          else if(ext==".sdb")
            vm = new AutoSDBVMatrix(remove_extension(datasetstring));            
          else if(ext==".mat")
            vm = loadAsciiAsVMat(datasetstring);
          else 
            PLERROR("Unknown extension for vmatrix: %s", ext.c_str());
          vm->setMetaDataDir(extract_directory(datasetstring) + extract_filename(datasetstring) + ".metadata");
        }
      else // it's a directory
      {
          // is it the directory of a DiskVMatrix?
          if(isfile(datasetstring+slash+"indexfile") && isfile(datasetstring+slash+"0.data"))
            {
              vm = new DiskVMatrix(datasetstring);
            }
          else
            PLERROR("In getDataSet: datasetstring is a directory of unknown format"); 
        }
    }
  else // it's either a preprogrammed dataset, or a VMatrix object
  {
    try // try with a preprogrammed dataset
      {
        vector<string> dsetspec = split(datasetstring);
        if(dsetspec.size()<2)
          PLERROR("In getDataSet, expecting a specification of the form '<datasetname> <train|test|all> [normalize]. DatasetString = %s' ",datasetstring.c_str());
        string datasetname = dsetspec[0];
        bool normalizeinputs = false;
        if(dsetspec.size()>=3)
          {
            if(dsetspec[2]=="normalize")
              normalizeinputs = true;
            else PLERROR("In getDataSet specification of predefined dataset contains 3 words, expecting 3rd one to be 'normalize', don't understand '%s'",dsetspec[2].c_str());
          }
        
        int inputsize, nclasses;
        VMat trainset, testset;
        loadClassificationDataset(datasetname, inputsize, nclasses, trainset, testset, normalizeinputs);
        if(dsetspec[1]=="train")
          vm = trainset;
        else if(dsetspec[1]=="test")
          vm = testset;
        else if(dsetspec[1]=="all")
          vm = vconcat(trainset,testset);    
        else 
          PLERROR("In getDataSet specification of predefined dataset: expecting second word to be 'train' or 'test' or 'all' not %s ...",dsetspec[1].c_str());
        vm->defineSizes(inputsize, 1);
        vm->setMetaDataDir("/u/lisa/db/metadata/" + datasetstring);
      }
    catch(const PLearnError& e)  // OK, it wasn't a preprogrammed dataset, let's try with a VMatrix object
      {
        try 
          { 
            vm = dynamic_cast<VMatrix*>(newObject(datasetstring));
            if(!vm)
              PLERROR("Not a VMatrix object (dynamic cast failed)");
          }
        catch(const PLearnError& e2)
          {
            PLERROR("Error in getDataSet with specification: %s\n"
                    "Specification is neither a valid file or directory \n"
                    "Nor is it a preprogrammed dataset (attempt returned: %s)\n"
                    "Nor could it be resolved to a VMatrix object (attempt returned: %s)\n",
                    datasetstring.c_str(), e.message().c_str(), e2.message().c_str());
          }
      }
  }
  
  vm->loadAllStringMappings(); // let's comment this until bug fixed by Julien
  vm->setAlias(alias);
  vm->unduplicateFieldNames();
  return vm;
}


string locateDatasetAliasesDir(const string& dir_or_file_path)
{
  if(!pathexists(dir_or_file_path))
    PLERROR("In getDatasetAliases argument '%s' is not an existing directory or file!", dir_or_file_path.c_str());
  string dirname = extract_directory(abspath(dir_or_file_path));
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
