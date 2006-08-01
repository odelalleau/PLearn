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
 * $Id$
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file SourceVMatrix.cc */


#include "SourceVMatrix.h"
#include <plearn/io/fileutils.h>        //!< For 'isfile(..)'.
#include <plearn/base/stringutils.h>

namespace PLearn {
using namespace std;


SourceVMatrix::SourceVMatrix(bool call_build_)
    : inherited(call_build_)
{
    if( call_build_ )
        build_();
}

SourceVMatrix::SourceVMatrix(VMat the_source, bool call_build_)
    : inherited(call_build_),
      source(the_source)
{
    if( call_build_ )
        build_();
}

SourceVMatrix::SourceVMatrix(VMat the_source, int the_length, int the_width,
                             bool call_build_)
    : inherited(the_length, the_width, call_build_),
      source(the_source)
{
    if( call_build_ )
        build_();
}

PLEARN_IMPLEMENT_OBJECT(SourceVMatrix,
                        "Super-class for VMatrices that point to another one (the source vmatrix)",
                        ""
    );

void SourceVMatrix::setMetaDataDir(const PPath& the_metadatadir)
{
    inherited::setMetaDataDir(the_metadatadir);

    if (!source)
        return;

    if(!source->hasMetaDataDir())
        source->setMetaDataDir(the_metadatadir/"Source");

    // Set mapping and info files from source if not set
    if(isdir(getSFIFDirectory()) && hasFieldInfos())
    {
        for(int j=0; j<width_; j++)
        {
            string fnam = fieldName(j);
            if(!isfile(getSFIFFilename(fnam,".smap")) && isfile(source->getSFIFFilename(fnam,".smap")))
                setSFIFFilename(fnam, ".smap", source->getSFIFFilename(fnam,".smap"));

            if(!isfile(getSFIFFilename(fnam,".notes")) && isfile(source->getSFIFFilename(fnam,".notes")))
                setSFIFFilename(fnam, ".notes", source->getSFIFFilename(fnam,".notes"));

            if(!isfile(getSFIFFilename(fnam,".binning")) && isfile(source->getSFIFFilename(fnam,".binning")))
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

///////////////////////////
// setMetaInfoFromSource //
///////////////////////////
void SourceVMatrix::setMetaInfoFromSource()
{
    setMetaInfoFrom(source);
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

PP<Dictionary> SourceVMatrix::getDictionary(int col) const
{
    return source->getDictionary(col);
}

Vec SourceVMatrix::getValues(int row, int col) const
{
    return source->getValues(row,col);
}

Vec SourceVMatrix::getValues(const Vec& input, int col) const
{
    return source->getValues(input,col);
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
