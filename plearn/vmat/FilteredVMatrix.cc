// -*- C++ -*-

// FilteredVMatrix.cc
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
   * $Id: FilteredVMatrix.cc,v 1.17 2005/02/24 16:34:02 lheureup Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file FilteredVMatrix.cc */

#include <plearn/base/ProgressBar.h>
#include "FilteredVMatrix.h"

namespace PLearn {
using namespace std;


FilteredVMatrix::FilteredVMatrix()
  : inherited(),
    build_complete(false),
    report_progress(true)
{
}

FilteredVMatrix::FilteredVMatrix( VMat the_source, const string& program_string,
                                  const PPath& the_metadatadir, bool the_report_progress)
  : SourceVMatrix(the_source),
    report_progress(the_report_progress),
    prg(program_string)
{
  metadatadir = the_metadatadir;
  build_();
}

PLEARN_IMPLEMENT_OBJECT(FilteredVMatrix, "A filtered view of its source vmatrix", 
    "The filter is an exression in VPL language.\n"
    "The filtered indexes are saved in the metadata directory, that NEEDS to\n"
    "be provided.\n" );



void FilteredVMatrix::openIndex()
{
  string idxfname = getMetaDataDir() / "filtered.idx";
  if(!force_mkdir(getMetaDataDir()))
     PLERROR("In FilteredVMatrix::openIndex could not create directory %s",getMetaDataDir().absolute().c_str());

  if(file_exists(idxfname) && mtime(idxfname)>=getMtime())
    indexes.open(idxfname);
  else  // let's (re)create the index
    {
      rm(idxfname);       // force remove it
      int l = source.length();
      Vec result(1);
      indexes.open(idxfname,true);
      ProgressBar* pb = 0;
      if (report_progress)
        pb = new ProgressBar("Filtering source vmat", l);
      for(int i=0; i<l; i++)
        {
          if (report_progress)
            pb->update(i);
          program.run(i,result);
          if(result[0]!=0)
            indexes.append(i);
        }
      if (pb)
        delete pb;
      indexes.close();
      indexes.open(idxfname);
    }

  length_ = indexes.length();
}

void FilteredVMatrix::setMetaDataDir(const PPath& the_metadatadir)
{
  inherited::setMetaDataDir(the_metadatadir);
  if (build_complete) {
    // Only call openIndex() if the build has been completed,
    // otherwise the filtering program won't be ready yet.
    openIndex();
  }
}

void FilteredVMatrix::getNewRow(int i, const Vec& v) const
{
  if (indexes.length() == -1)
    PLERROR("In FilteredVMatrix::getNewRow - The filtered indexes are not set, make sure you provided a metadatadir");
  source->getRow(indexes[i],v);
}

void FilteredVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "prg", &FilteredVMatrix::prg, OptionBase::buildoption,
                "The VPL code that should produce a single scalar, indicating whether \n"
                "we should keep the line (if the produced scalar is non zero) or throw it away (if it's zero)");

  declareOption(ol, "report_progress", &FilteredVMatrix::report_progress, OptionBase::buildoption,
      "Whether to report the filtering progress or not.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void FilteredVMatrix::build_()
{
  vector<string> fieldnames;
  program.setSource(source);
  // TODO: What happens when ptr is null
  program.compileString(prg,fieldnames); 
  if(metadatadir != "") {
    openIndex();
  }
  setMetaInfoFromSource();
  build_complete = true;
}

// ### Nothing to add here, simply calls build_
void FilteredVMatrix::build()
{
  build_complete = false;
  inherited::build();
  build_();
}

void FilteredVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

} // end of namespace PLearn

