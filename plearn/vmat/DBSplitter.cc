// -*- C++ -*-

// DBSplitter.cc
//
// Copyright (C) 2004 Marius Muja
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

// Authors: Marius Muja

/*! \file DBSplitter.cc */


#include "DBSplitter.h"
#include <plearn/db/getDataSet.h>

namespace PLearn {
using namespace std;

DBSplitter::DBSplitter()
    :Splitter(),
     databases(0)
{
}

PLEARN_IMPLEMENT_OBJECT(DBSplitter,
                        "A Splitter that contains several databases.",
                        "The databases to be used can be specified with the 'databases' option. ");

void DBSplitter::declareOptions(OptionList& ol)
{

    declareOption(ol, "databases", &DBSplitter::databases, OptionBase::buildoption,
                  "Vector with the specifications of the databases to be used.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DBSplitter::build_()
{
}

// ### Nothing to add here, simply calls build_
void DBSplitter::build()
{
    inherited::build();
    build_();
}

void DBSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    Splitter::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("DBSplitter::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

int DBSplitter::nsplits() const
{
    return databases.size();
}

int DBSplitter::nSetsPerSplit() const
{
    return 1;
}

TVec<VMat> DBSplitter::getSplit(int k)
{
    TVec<VMat> result;
    result.append(PLearn::getDataSet(databases[k]));

    return result;
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
