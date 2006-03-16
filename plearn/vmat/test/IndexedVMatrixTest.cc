// -*- C++ -*-

// IndexedVMatrixTest.cc
//
// Copyright (C) 2006 Pascal Lamblin 
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

// Authors: Pascal Lamblin

/*! \file IndexedVMatrixTest.cc */


#include "IndexedVMatrixTest.h"
#include <plearn/vmat/AutoVMatrix.h>
#include <plearn/vmat/IndexedVMatrix.h>
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    IndexedVMatrixTest,
    "Tests IndexedVMatrixTest behaviour, especially with NaN and strings",
    ""
);

////////////////////////
// IndexedVMatrixTest //
////////////////////////
IndexedVMatrixTest::IndexedVMatrixTest() 
{
}

///////////
// build //
///////////
void IndexedVMatrixTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void IndexedVMatrixTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

////////////////////
// declareOptions //
////////////////////
void IndexedVMatrixTest::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &IndexedVMatrixTest::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    declareOption(ol, "source_matrix", &IndexedVMatrixTest::source_matrix,
                  OptionBase::buildoption,
                  "Source matrix of the IndexedVMatrix being tested");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void IndexedVMatrixTest::build_()
{
}

/////////////
// perform //
/////////////
void IndexedVMatrixTest::perform()
{
    // ### The test code should go here.
    if( source_matrix == "" )
        PLERROR( "In RowBufferedVMatrixTest::perform - You must provide"
                 " 'source_matrix' option" );

    VMat source_vmat = new AutoVMatrix( source_matrix );
    source_vmat->build();

    VMat indexed_vmat = new IndexedVMatrix( source_vmat, true, true );

    int n = indexed_vmat->length();

    MAND_LOG << "source_matrix = " << source_matrix.canonical() << endl;
    MAND_LOG << "Column 2 (value) of indexed_vmat as strings:" << endl;
    for( int i=0 ; i<n ; i++ )
    {
        MAND_LOG << indexed_vmat->getString(i,2) << endl;
    }
    MAND_LOG << endl;

    MAND_LOG << "Underlying (real) values:" << endl;
    for( int i=0 ; i<n ; i++ )
    {
        MAND_LOG << indexed_vmat(i,2) << endl;
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
