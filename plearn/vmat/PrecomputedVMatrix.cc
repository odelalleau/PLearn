// -*- C++ -*-

// PrecomputedVMatrix.cc
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
   * $Id: PrecomputedVMatrix.cc,v 1.2 2003/09/09 22:50:54 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file PrecomputedVMatrix.cc */


#include "PrecomputedVMatrix.h"
#include "DiskVMatrix.h"
#include "FileVMatrix.h"

namespace PLearn <%
using namespace std;


PrecomputedVMatrix::PrecomputedVMatrix()
  :precomp_type("dmat")
{
}

PLEARN_IMPLEMENT_OBJECT(PrecomputedVMatrix, "ONE LINE DESCRIPTION", "MULTI-LINE \nHELP");

void PrecomputedVMatrix::getRow(int i, Vec v) const
{
  if(precomp_source.isNull())
    PLERROR("Source has not been precomputed. Did you properly set the MetaDataDir?");
  precomp_source->getRow(i,v);
}

void PrecomputedVMatrix::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // ### ex:
  declareOption(ol, "source", &PrecomputedVMatrix::source, OptionBase::buildoption,
                 "The source vmatrix to be precomputed and cached in the metadata dirrectory");
  declareOption(ol, "precomp_type", &PrecomputedVMatrix::precomp_type, OptionBase::buildoption,
                "The type of vmatrix to be used for the cached precomputed version of the source.\n"
                "Currently supported are: dmat, pmat");
  // ...

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void PrecomputedVMatrix::setMetaDataDir(const string& the_metadatadir)
{
  inherited::setMetaDataDir(the_metadatadir);
  build();
}

void PrecomputedVMatrix::build_()
{
  string mdir = getMetaDataDir();
  if(mdir!="") // don't do anything if the meta-data-dir is not yet set.
    {
      if(precomp_type=="dmat")
        {
          string dmatdir = mdir + "precomp.dmat";
          bool recompute = true;
          if(isdir(dmatdir))
            {
              precomp_source = new DiskVMatrix(dmatdir);
              if(precomp_source->getMtime() >= source->getMtime())
                recompute = false;
            }
          if(recompute)
            {
              force_rmdir(dmatdir);
              source->saveDMAT(dmatdir);
              precomp_source = new DiskVMatrix(dmatdir);
            }
        }
      else if(precomp_type=="pmat")
        {
          string pmatfile = mdir + "precomp.pmat";
          bool recompute = true;
          if(isfile(pmatfile))
            {
              precomp_source = new FileVMatrix(pmatfile);
              if(precomp_source->getMtime() >= source->getMtime())
                recompute = false;
            }
          if(recompute)
            {
              rm(pmatfile);
              source->savePMAT(pmatfile);
              precomp_source = new FileVMatrix(pmatfile);
            }
        }
      else
        PLERROR("Invalid precomp_type=%s. Must be one of: dmat, pmat.",precomp_type.c_str());
          
      setMtime(source->getMtime());

      // Set length and width
      length_ = source->length();
      width_ = source->width();

      // Set sizes
      if(inputsize_<0)
        inputsize_ = source->inputsize();
      if(targetsize_<0)
        targetsize_ = source->targetsize();
      if(weightsize_<0)
        weightsize_ = source->weightsize();

      // Set fieldnames
      setFieldInfos(source->getFieldInfos());

      // Set mapping and info files
      for(int j=0; j<width_; j++)
        {
          setSFIFFilename(j, ".smap", source->getSFIFFilename(j,".smap"));
          setSFIFFilename(j, ".notes", source->getSFIFFilename(j,".notes"));
          setSFIFFilename(j, ".binning", source->getSFIFFilename(j,".binning"));
        }
    }
}

// ### Nothing to add here, simply calls build_
void PrecomputedVMatrix::build()
{
  inherited::build();
  build_();
}

void PrecomputedVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  deepCopyField(source, copies);
  deepCopyField(precomp_source, copies);
}

%> // end of namespace PLearn

