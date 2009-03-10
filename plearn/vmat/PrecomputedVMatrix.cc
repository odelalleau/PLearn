// -*- C++ -*-

// PrecomputedVMatrix.cc
//
// Copyright (C) 2003 Pascal Vincent
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
 * $Id$
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file PrecomputedVMatrix.cc */


#include "PrecomputedVMatrix.h"
#include "DiskVMatrix.h"
#include "FileVMatrix.h"
#include "TemporaryDiskVMatrix.h"
#include "TemporaryFileVMatrix.h"
#include <plearn/io/PPath.h>
#include <plearn/io/fileutils.h> //!< For force_rmdir()

namespace PLearn {
using namespace std;


PrecomputedVMatrix::PrecomputedVMatrix():
    precomp_type("dmat"),
    temporary(false)
{}

PLEARN_IMPLEMENT_OBJECT(
    PrecomputedVMatrix,
    "VMatrix that pre-computes on disk the content of a source VMatrix",
    "This sub-class of SourceVMatrix pre-computes the content of a source\n"
    "VMatrix in a 'dmat' (DiskVMatrix) or 'pmat' (FileVMatrix) file. The\n"
    "name of the disk file is obtained from the metadatadir option, followed\n"
    "by 'precomp.dmat' or 'precomp.pmat'.\n"
    "With the 'temporary' option, one may automatically delete the\n"
    "precomputed data once it is not used anymore.\n"
);

///////////////
// getNewRow //
///////////////
void PrecomputedVMatrix::getNewRow(int i, const Vec& v) const
{
    if(precomp_source.isNull())
        PLERROR("Source has not been precomputed. Did you properly set the "
                "MetaDataDir?");
    precomp_source->getRow(i,v);
}

////////////////////
// declareOptions //
////////////////////
void PrecomputedVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "precomp_type", &PrecomputedVMatrix::precomp_type, OptionBase::buildoption,
        "The type of VMatrix to be used for the cached precomputed version\n"
        "of the source. Currently supported are 'dmat' and 'pmat'.");

    declareOption(ol, "temporary", &PrecomputedVMatrix::temporary, OptionBase::buildoption,
        "If set to 1, the created precomputed data will be temporary, i.e.\n"
        "will be deleted when not used anymore.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Hide useless options.
    redeclareOption(ol, "writable", &PrecomputedVMatrix::writable, OptionBase::nosave,
                    "Unused option.");
    redeclareOption(ol, "length", &PrecomputedVMatrix::length_, OptionBase::nosave,
                    "Unused option.");
    redeclareOption(ol, "width", &PrecomputedVMatrix::width_, OptionBase::nosave,
                    "Unused option.");
    redeclareOption(ol, "inputsize", &PrecomputedVMatrix::inputsize_, OptionBase::nosave,
                    "Unused option.");
    redeclareOption(ol, "targetsize", &PrecomputedVMatrix::targetsize_, OptionBase::nosave,
                    "Unused option.");
    redeclareOption(ol, "weightsize", &PrecomputedVMatrix::weightsize_, OptionBase::nosave,
                    "Unused option.");
}

////////////////////
// setMetaDataDir //
////////////////////
void PrecomputedVMatrix::setMetaDataDir(const PPath& the_metadatadir)
{
    inherited::setMetaDataDir(the_metadatadir);
    if ( hasMetaDataDir() ) // don't do anything if the meta-data-dir is not yet set.
    {
        setMetaInfoFromSource();
        usePrecomputed();
    }
}

void PrecomputedVMatrix::usePrecomputed()
{
    PPath mdir = getMetaDataDir();

    if ( precomp_type == "dmat" )
    {
        PPath dmatdir  = mdir / "precomp.dmat";
        bool recompute = true;

        if ( isdir(dmatdir) )
        {
            precomp_source = temporary ? new TemporaryDiskVMatrix(dmatdir,
                                                                  false)
                                       : new DiskVMatrix(dmatdir);
            if(isUpToDate(precomp_source))
                recompute = false;
        }

        if(recompute)
        {
            force_rmdir(dmatdir);
            source->saveDMAT(dmatdir);
            precomp_source = temporary ? new TemporaryDiskVMatrix(dmatdir,
                                                                  false)
                                       : new DiskVMatrix(dmatdir);
        }
        length_ = precomp_source->length();
    }

    else if ( precomp_type == "pmat" )
    {
        PPath pmatfile = mdir / "precomp.pmat";
        bool recompute = true;

        if ( isfile(pmatfile) )
        {
            precomp_source = temporary ? new TemporaryFileVMatrix(pmatfile)
                                       : new FileVMatrix(pmatfile);
            if(isUpToDate(pmatfile))
                recompute = false;
        }

        if(recompute)
        {
            rm(pmatfile);
            source->savePMAT(pmatfile);
            precomp_source = temporary ? new TemporaryFileVMatrix(pmatfile)
                                       : new FileVMatrix(pmatfile);
        }
        length_ = precomp_source->length();
    }

    else
        PLERROR("Invalid precomp_type=%s. Must be one of: dmat, pmat.",
                precomp_type.c_str());
}

void PrecomputedVMatrix::build_()
{
    //We don't always call it as some matrix(FilteredVMatrix) are only
    //completly set if they have a metadatadir.
    if(hasMetaDataDir() ||(source->width()>0 && source->length()>0))
        setMetaInfoFromSource();
}

// ### Nothing to add here, simply calls build_
void PrecomputedVMatrix::build()
{
    inherited::build();
    build_();
}

void PrecomputedVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    deepCopyField(precomp_source, copies);
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
