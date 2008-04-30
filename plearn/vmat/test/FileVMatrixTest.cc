// -*- C++ -*-

// FileVMatrixTest.cc
//
// Copyright (C) 2005 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file FileVMatrixTest.cc */


#include "FileVMatrixTest.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/math/pl_math.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/io/fileutils.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    FileVMatrixTest,
    "ONE LINE USER DESCRIPTION",
    "MULTI LINE\nHELP FOR USERS"
);

//////////////////
// FileVMatrixTest //
//////////////////
FileVMatrixTest::FileVMatrixTest()
    /* ### Initialize all fields to their default value */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

///////////
// build //
///////////
void FileVMatrixTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void FileVMatrixTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("FileVMatrixTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void FileVMatrixTest::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &FileVMatrixTest::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void FileVMatrixTest::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

/////////////
// perform //
/////////////
void FileVMatrixTest::perform()
{
    pout << "Testing small FileVMatrix" << endl;

    pout << "TEST m1" << endl;
    FileVMatrix* m1 = new FileVMatrix("fvm1.pmat",10,3);
    Vec v(3);
    v[0] = 1.0;
    v[1] = 2.0;
    v[2] = 3.0;

    for(int i=0; i<m1->length(); i++)
    {
        m1->putRow(i,v);
        v += 1.0;
    }

    m1->appendRow(v);
    m1->flush();
    m1->putRow(3,v);
    m1->appendRow(v);
    v[1] = MISSING_VALUE;
    m1->putRow(2,v);
    delete m1;

    pout << "TEST m2" << endl;
    FileVMatrix* m2 = new FileVMatrix("fvm1.pmat",true);
    v += 10.;
    m2->appendRow(v);
    m2->putRow(4,v);
    delete m2;

    pout << "TEST m3" << endl;
    FileVMatrix* m3 = new FileVMatrix("fvm1.pmat");
    TVec<string> fieldnames(3);
    fieldnames[0]="aaa";
    fieldnames[1]="bbb";
    fieldnames[2]="ccc";

    pout << "TEST m4" << endl;
    FileVMatrix* m4 = new FileVMatrix("fvm4.pmat",0,fieldnames);
    int l = m3->length();
    for(int i=0; i<l; i++)
    {
        m3->getRow(i,v);
        m4->appendRow(v);
    }
    delete m4;

    pout << "TEST m5" << endl;
    FileVMatrix* m5 = new FileVMatrix("fvm4.pmat");
    pout << "Fieldnames: " << m5->fieldNames() << " SAME? " << (m5->fieldNames()==fieldnames) << endl;
    pout << "toMat(): \n" << m5->toMat() << endl;

    pout << "TEST m6" << endl;
    pout << "Testing huge FileVMatrix" << endl;
    v[0] = 100;
    v[1] = 101;
    v[2] = 102;
    l = 200000000;
    FileVMatrix* m6 = new FileVMatrix("fvm6.pmat",l,3);
    m6->putRow(l-5,v+1.0);
    m6->putRow(5,v-1.0);

    m6->getRow(l-5, v);
    pout << l-5 << ": " << v << " (should be 101 102 103)" << endl;
    m6->getRow(5, v);
    pout << 5 << ": " << v << " (should be 99 100 101)" << endl;

    v.fill(999);
    m6->appendRow(v);

    pout << "l=" << l << " length=" << m6->length() << " (should be l+1)" << endl;
    m6->getRow(l-5, v);
    pout << l-5 << ": " << v << " (should be 101 102 103)" << endl;
    m6->getRow(5, v);
    pout << 5 << ": " << v << " (should be 99 100 101)" << endl;
    m6->getRow(l, v);
    pout << l << ": " << v << " (should be 999 999 999)" << endl;

    PRUint64 siz1 = m6->length();
    siz1 *= m6->width()*sizeof(real);
    siz1 += DATAFILE_HEADERLENGTH;
    delete m6;


    PRUint64 siz2 = filesize64("fvm6.pmat");
    pout << "Huge file size: " << siz2 << " sohuld equal " << siz1 << "  ? " << (siz2==siz1) << endl;

    pout << "TEST m7" << endl;
    FileVMatrix* m7 = new FileVMatrix("fvm6.pmat");

    pout << "l=" << l << " length=" << m7->length() << " (should be l+1)" << endl;
    m7->getRow(l-5, v);
    pout << l-5 << ": " << v << " (should be 101 102 103)" << endl;
    m7->getRow(5, v);
    pout << 5 << ": " << v << " (should be 99 100 101)" << endl;
    m7->getRow(l, v);
    pout << l << ": " << v << " (should be 999 999 999)" << endl;
    delete m7;

    // remove it because we don't want to keep such a huge file under revision control
    rm("fvm6.pmat");
    force_rmdir("fvm6.pmat.metadata");
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
