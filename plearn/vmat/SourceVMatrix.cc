// -*- C++ -*-

// SourceVMatrix.cc
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
   * $Id: SourceVMatrix.cc,v 1.12 2004/09/14 16:04:39 chrish42 Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file SourceVMatrix.cc */


#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;


SourceVMatrix::SourceVMatrix()
  :inherited()
  /* ### Initialise all fields to their default value */
{
  // ...

  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

PLEARN_IMPLEMENT_OBJECT(SourceVMatrix,
    "Super-class for VMatrices that point to another one (the source vmatrix)",
    ""
);

void SourceVMatrix::setMetaDataDir(const string& the_metadatadir)
{
  inherited::setMetaDataDir(the_metadatadir);
  if(!source->hasMetaDataDir())
    source->setMetaDataDir(the_metadatadir+slash+"Source"+slash);

  // Set mapping and info files from source if not set
  if(hasFieldInfos())
    {
      for(int j=0; j<width_; j++)
        {
          string fnam = fieldName(j);
          if(!file_exists(getSFIFFilename(fnam,".smap")) && file_exists(source->getSFIFFilename(fnam,".smap")))            
            setSFIFFilename(fnam, ".smap", source->getSFIFFilename(fnam,".smap"));
          if(!file_exists(getSFIFFilename(fnam,".notes")) && file_exists(source->getSFIFFilename(fnam,".notes")))            
            setSFIFFilename(fnam, ".notes", source->getSFIFFilename(fnam,".notes"));
          if(!file_exists(getSFIFFilename(fnam,".binning")) && file_exists(source->getSFIFFilename(fnam,".binning")))            
            setSFIFFilename(fnam, ".binning", source->getSFIFFilename(fnam,".binning"));
        }
    }
}

void SourceVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "source", &SourceVMatrix::source, OptionBase::buildoption,
                "The source VMatrix");

  /*
  declareOption(ol, "dependencies", &SourceVMatrix::dependencies, OptionBase::buildoption,
                "a list of paths to files that this VMat depends on. \n"
                "This vmat's mtime will initially be set to the latest of \n"
                "the last modification times of its dependencies. \n"
                "The mtime of its source will also be taken into account \n"
                "generally later, when setMetaInfoFromSource gets called \n");
  */

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void SourceVMatrix::build_()
{
  /*
  time_t mt = getMtime();
  for(int k=0; k<dependencies.size(); k++)
    mt = max(mt, mtime(dependencies[k]));
  setMtime(mt);
  */
}

void SourceVMatrix::setMetaInfoFromSource()
{
  setMtime(max(getMtime(),source->getMtime()));

  // copy length and width from source if not set
  if(length_<0)
    length_ = source->length();
  if(width_<0)
    width_ = source->width();

  // copy sizes from source if not set
  if(inputsize_<0)
    inputsize_ = source->inputsize();
  if(targetsize_<0)
    targetsize_ = source->targetsize();
  if(weightsize_<0)
    weightsize_ = source->weightsize();

  // copy fieldnames from source if not set and they look good
  if(!hasFieldInfos() && (width() == source->width()) && source->hasFieldInfos() )
    setFieldInfos(source->getFieldInfos());

  // Copy String/Real mappings if not set.
  if (map_rs.length() == 0) {
    map_rs.resize(width_);
    for (int j = 0; j < width_; j++) {
      if (j < source->width()) {
        map_rs[j] = source->getRealToStringMapping(j);
      } else {
        // Empty map.
        map_rs[j] = map<real,string>();
      }
    }
  }
  if (map_sr.length() == 0) {
    map_sr.resize(width_);
    for (int j = 0; j < width_; j++) {
      if (j < source->width()) {
        map_sr[j] = source->getStringToRealMapping(j);
      } else {
        // Empty map.
        map_sr[j] = map<string,real>();
      }
    }
  }
}

// ### Nothing to add here, simply calls build_
void SourceVMatrix::build()
{
  inherited::build();
  build_();
}

void SourceVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(sourcerow, copies);
  deepCopyField(source, copies);
}

///////////////
// getNewRow //
///////////////
void SourceVMatrix::getNewRow(int i, const Vec& v) const {
  PLERROR("In SourceVMatrix::getNewRow - getNewRow not implemented for this subclass of SourceVMatrix");
}

} // end of namespace PLearn

