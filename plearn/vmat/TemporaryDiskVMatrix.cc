// -*- C++ -*-

// TemporaryDiskVMatrix.cc
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

/*! \file TemporaryDiskVMatrix.cc */


#include "TemporaryDiskVMatrix.h"
#include <plearn/io/fileutils.h>    //!< For 'force_rmdir'.

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    TemporaryDiskVMatrix,
    "DiskVMatrix to store data temporarily.",
    "When a TemporaryDiskVMatrix object is deleted, it automatically removes\n"
    "its data directory, as well as its metadata directory.\n"
    );

//////////////////////////
// TemporaryDiskVMatrix //
//////////////////////////
TemporaryDiskVMatrix::TemporaryDiskVMatrix()
{}

TemporaryDiskVMatrix::TemporaryDiskVMatrix(const PPath& filename,
                                           bool writable):
    inherited(filename, writable)
{
    build_();
}

////////////////////
// declareOptions //
////////////////////
void TemporaryDiskVMatrix::declareOptions(OptionList& ol)
{
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void TemporaryDiskVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void TemporaryDiskVMatrix::build_()
{
    last_files_opened.resize(0);
    last_dirname = dirname;
    addReferenceToFile(dirname);
    if (indexf)
        last_files_opened.append(dirname / "indexfile");
    updateMtime(dirname/"indexfile");
    for (int i = 0; i < dataf.length(); i++)
        if (dataf[i])
            last_files_opened.append(dirname / (tostring(i) + ".data"));
    for (int i = 0; i < last_files_opened.length(); i++)
        addReferenceToFile(last_files_opened[i]);
}

///////////////////////
// closeCurrentFiles //
///////////////////////
void TemporaryDiskVMatrix::closeCurrentFiles()
{
    inherited::closeCurrentFiles();
    for (int i = 0; i < last_files_opened.length(); i++)
        removeReferenceToFile(last_files_opened[i]);
    removeReferenceToFile(last_dirname);
    last_files_opened.resize(0);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void TemporaryDiskVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(last_files_opened, copies);
    // Clearing 'last_files_opened' is important: no file has been opened yet
    // by this VMat, as file pointers need to be recreated (this will be done
    // in inherited::makeDeepCopyFromShallowCopy(..)).
    // Similarly, 'last_dirname' needs to be cleared since it has not actually
    // been opened yet by this VMat.
    last_files_opened.resize(0);
    last_dirname = "";
    inherited::makeDeepCopyFromShallowCopy(copies);
}

///////////////////////////
// ~TemporaryDiskVMatrix //
///////////////////////////
TemporaryDiskVMatrix::~TemporaryDiskVMatrix()
{
    TVec<PPath> backup_files_opened = last_files_opened.copy();
    TemporaryDiskVMatrix::closeCurrentFiles();
    bool can_delete_whole_dir = true;
    bool one_file_deleted = false;
    for (int i = 0; i < backup_files_opened.length(); i++) {
        if (noReferenceToFile(backup_files_opened[i])) {
            rm(backup_files_opened[i]);
            one_file_deleted = true;
        } else
            can_delete_whole_dir = false;
    }
    if (one_file_deleted && !can_delete_whole_dir)
        PLERROR("In TemporaryDiskVMatrix::~TemporaryDiskVMatrix - Some data "
                "file has been deleted, but not all of them: something must "
                "be wrong");
    if (can_delete_whole_dir && !noReferenceToFile(dirname))
        PLERROR("In TemporaryDiskVMatrix::~TemporaryDiskVMatrix - Was able to "
                "delete all files in '%s', but it looks like someone is still "
                "using this directory: something must be wrong",
                dirname.absolute().c_str());
    if (!can_delete_whole_dir && noReferenceToFile(dirname))
        PLERROR("In TemporaryDiskVMatrix::~TemporaryDiskVMatrix - Directory "
                "'%s' is said to have noone referencing it, however it looks "
                "like some files in the directory are still being used: this "
                "should not happen",
                dirname.absolute().c_str());
    if (can_delete_whole_dir) {
        force_rmdir(last_dirname);
        if (hasMetaDataDir()) {
            force_rmdir(getMetaDataDir());
            // Clear the metadatadir so that the parent class does not try to
            // do anything with it.
            metadatadir = "";
        }
    }
}

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
