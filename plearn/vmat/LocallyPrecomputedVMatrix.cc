// -*- C++ -*-

// LocallyPrecomputedVMatrix.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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
   * $Id: LocallyPrecomputedVMatrix.cc,v 1.10 2005/02/18 17:14:33 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file LocallyPrecomputedVMatrix.cc */

#include "LocallyPrecomputedVMatrix.h"
#include <plearn/base/tostring.h>
#include <plearn/io/fileutils.h>      //!< For force_mkdir()
#include <plearn/vmat/FileVMatrix.h>

namespace PLearn {
using namespace std;

///////////////////////////////
// LocallyPrecomputedVMatrix //
///////////////////////////////
LocallyPrecomputedVMatrix::LocallyPrecomputedVMatrix()
: local_dir("/Tmp"),
  max_wait(30),
  remove_when_done(true),
  sequential_access(true),
  verbosity(2)
{
  precomp_type = "pmat";
}

PLEARN_IMPLEMENT_OBJECT(LocallyPrecomputedVMatrix,
    "A VMat that precomputes its source in a local directory.",
    "The 'sequential_access' option can be used to ensure that parallel experiments\n"
    "do not access simultaneously the source VMat, in order to stay nice with the\n"
    "disk usage. This is achieved thanks to a system lock file in the metadatadir of\n"
    "the source VMat. Because it may happen that a lock file remains after an experiment\n"
    "crashed, it will be ignored when it gets older than 'max_wait' minutes.\n"
);

////////////////////
// declareOptions //
////////////////////
void LocallyPrecomputedVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "local_dir", &LocallyPrecomputedVMatrix::local_dir, OptionBase::buildoption,
      "The local directory in which we want to save the precomputed data.");

  declareOption(ol, "remove_when_done", &LocallyPrecomputedVMatrix::remove_when_done, OptionBase::buildoption,
      "Whether we want or not to remove the precomputed data when this object is deleted.");

  declareOption(ol, "sequential_access", &LocallyPrecomputedVMatrix::sequential_access, OptionBase::buildoption,
      "If set to 1, ensures there are no multiple parallel precomputations (see class help).");

  declareOption(ol, "max_wait", &LocallyPrecomputedVMatrix::max_wait, OptionBase::buildoption,
      "Maximum wait time in minutes when 'sequential access' is set to 1 (see class help).");

  declareOption(ol, "verbosity", &LocallyPrecomputedVMatrix::verbosity, OptionBase::buildoption,
      "Controls the amount of displayed information.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);

  redeclareOption(ol, "metadatadir", &LocallyPrecomputedVMatrix::metadatadir, OptionBase::nosave,
      "The metadatadir will be defined by the 'local_dir' option.");

  redeclareOption(ol, "precomp_type", &LocallyPrecomputedVMatrix::precomp_type, OptionBase::nosave,
      "We always use 'pmat' here.");
}

///////////
// build //
///////////
void LocallyPrecomputedVMatrix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void LocallyPrecomputedVMatrix::build_()
{
  if (metadatadir == "") {
    bool made_dir = force_mkdir(local_dir);
    if (!made_dir) {
      PLWARNING(
          "In LocallyPrecomputedVMatrix::build_ - Could not create dir: %s\n"
          "The program may crash if it needs to access an element of this VMatrix.",
          local_dir.c_str());
      return;
    }
    metadatadir = newFilename(local_dir, "locally_precomputed_", true);
    if (sequential_access) {
      // If necessary, wait until we are allowed to start the precomputation.
      if (source->hasMetaDataDir())
        source->lockMetaDataDir(max_wait * 60, verbosity >= 2);
      else
        PLERROR("In LocallyPrecomputedVMatrix::build_ - The source VMatrix must have a metadatadir");
    }
    inherited::build();
    precomp_source->setOption("remove_when_done", tostring(remove_when_done));
    precomp_source->setOption("track_ref", "1");
    precomp_source->build();
    if (sequential_access)
      source->unlockMetaDataDir();
  }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LocallyPrecomputedVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

////////////////////////////////
// ~LocallyPrecomputedVMatrix //
////////////////////////////////
LocallyPrecomputedVMatrix::~LocallyPrecomputedVMatrix()
{
  if (remove_when_done && metadatadir != "") {
    string filename_option = precomp_source->getOption("filename");
    // Remove the extra " characters at the beginning and the end of the string.
    filename_option = filename_option.substr(1, filename_option.size() - 2);
    PPath precomputed_file = PPath(filename_option);
    // First we delete the precomputed source, so that it does not try to save
    // more stuff in the metadatadir after it has been deleted.
    precomp_source = 0;
    // Let's check whether more FileVMatrix are accessing the same precomputed file.
    if (FileVMatrix::countRefs(precomputed_file) == 0) {
      bool removed = force_rmdir(metadatadir);
      if (!removed && verbosity >= 1)
        PLWARNING("In LocallyPrecomputedVMatrix::~LocallyPrecomputedVMatrix - "
                  "The precomputed data (in '%s') could not be removed",
                   metadatadir.absolute().c_str());
    }
  }
}


} // end of namespace PLearn

