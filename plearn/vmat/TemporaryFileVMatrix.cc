// -*- C++ -*-

// TemporaryFileVMatrix.cc
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

/*! \file TemporaryFileVMatrix.cc */


#include "TemporaryFileVMatrix.h"
#include <plearn/io/fileutils.h>    //!< For the system of references to files.

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    TemporaryFileVMatrix,
    "FileVMatrix whose data file is deleted with the object.",
    "The '.pmat' file is actually only deleted when it is believed that no\n"
    "other FileVMatrix is using it. This is necessary for instance when\n"
    "a deep-copy of a FileVMatrix is performed (e.g. for cross-validation).\n"
    "Note that the metadatadir (file.pmat.metadata) will also be deleted at\n"
    "the same time."
);

//////////////////////////
// TemporaryFileVMatrix //
//////////////////////////
TemporaryFileVMatrix::TemporaryFileVMatrix()
{}

TemporaryFileVMatrix::TemporaryFileVMatrix(const PPath& filename,
                                           bool writable):
    inherited(filename, writable)
{
    build_();
}

TemporaryFileVMatrix::TemporaryFileVMatrix(const PPath& filename,
                                           int the_length, int the_width):
    inherited(filename, the_length, the_width)
{
    build_();
}

////////////////////
// declareOptions //
////////////////////
void TemporaryFileVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &TemporaryFileVMatrix::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void TemporaryFileVMatrix::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void TemporaryFileVMatrix::build_()
{
    if (f) {
        // File has been opened successfully.
        addReferenceToFile(filename_);
        last_filename = filename_;
    } else
        last_filename = "";
    updateMtime(filename_);
}

//////////////////////
// closeCurrentFile //
//////////////////////
void TemporaryFileVMatrix::closeCurrentFile()
{
    if (f)
        removeReferenceToFile(last_filename);
    inherited::closeCurrentFile();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void TemporaryFileVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    // Clearing 'last_filename' is important: since at this point it is a
    // shallow copy, the file pointed by 'last_filename' has not actually
    // been opened by this object, thus there is no need to decrease its
    // number of references when build() is called in the parent
    // makeDeepCopyFromShallowCopy(..) method.
    last_filename = "";
    inherited::makeDeepCopyFromShallowCopy(copies);
}

///////////////////////////
// ~TemporaryFileVMatrix //
///////////////////////////
TemporaryFileVMatrix::~TemporaryFileVMatrix()
{
    TemporaryFileVMatrix::closeCurrentFile();
    if (noReferenceToFile(filename_)) {
        rm(filename_);
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
