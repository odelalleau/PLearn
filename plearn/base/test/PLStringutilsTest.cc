// -*- C++ -*-

// PLStringutilsTest.cc
//
// Copyright (C) 2007 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file PLStringutilsTest.cc */


#include "PLStringutilsTest.h"
#include <plearn/base/stringutils.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PLStringutilsTest,
    "Test functions declared in stringutils.h",
    ""
);

///////////////////////
// PLStringutilsTest //
///////////////////////
PLStringutilsTest::PLStringutilsTest()
{}

///////////
// build //
///////////
void PLStringutilsTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PLStringutilsTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("PLStringutilsTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void PLStringutilsTest::declareOptions(OptionList& ol)
{
    // ### ex:
    // declareOption(ol, "myoption", &PLStringutilsTest::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void PLStringutilsTest::build_()
{
}

/////////////
// perform //
/////////////
void PLStringutilsTest::perform()
{
    // Currently only test the 'begins_with' and 'ends_with' functions.
    // More tests may (should!) be added in the future.
    string s1 = "Hello";
    string s2 = "Hello World";
    string s3 = "Hell no!";
    string s4 = "Say Hello";
    string s5 = "Say Hello!";
    string s6 = "Say Jello";
    string s7 = "Hello\n \n\nWorld\n";
    string s8 = "\nHello\nWorld";
    pout << string_begins_with(s2, s1) << endl;
    pout << string_begins_with(s3, s1) << endl;
    pout << string_ends_with(s4, s1) << endl;
    pout << string_ends_with(s5, s1) << endl;
    pout << string_ends_with(s6, s1) << endl;
    pout << addprepostfix("pre-", s7, "-post") << endl;
    pout << addprepostfix("pre-", s8, "-post") << endl;
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
