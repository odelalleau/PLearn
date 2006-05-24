// -*- C++ -*-

// TupleTest.cc
//
// Copyright (C) 2006 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file TupleTest.cc */


#include "TupleTest.h"
#include <plearn/base/tuple.h>
#include <plearn/io/openString.h>
#include <plearn/base/tostring.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    TupleTest,
    "Simple test testing tuple i/o with PStreams",
    ""
);

//////////////////
// TupleTest //
//////////////////
TupleTest::TupleTest()
    /* ### Initialize all fields to their default value */
{}

///////////
// build //
///////////
void TupleTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void TupleTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("TupleTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void TupleTest::declareOptions(OptionList& ol)
{
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void TupleTest::build_()
{}

/////////////
// perform //
/////////////
void TupleTest::perform()
{
    string srepr;
    PStream fromsrepr;

    // first test pair
    
    pout << "\n***** Simple pair test *****" << endl;
    
    typedef pair<string, pair<int, string> > p1_t;
    p1_t p1("mot1",make_pair(3,string("mot2")));
    pout << "Type: " << TypeTraits<p1_t>::name() << endl;
    pout << "Value: " << p1 << endl;

    srepr = tostring(p1,PStream::plearn_ascii);
    fromsrepr = openString(srepr, PStream::plearn_ascii);
    p1_t p1_fromascii;
    fromsrepr >> p1_fromascii;
    pout << "Value reread from plearn_ascii serialization: " << p1_fromascii << endl;
    pout << "Same? " << (p1_fromascii==p1) << endl;

    srepr = tostring(p1,PStream::plearn_binary);
    fromsrepr = openString(srepr, PStream::plearn_binary);
    p1_t p1_frombinary;
    fromsrepr >> p1_frombinary;
    pout << "Value reread from plearn_binary serialization: " << p1_frombinary << endl;
    pout << "Same? " << (p1_frombinary==p1) << endl;

    // test a simple tuple

    pout << "\n***** Simple tuple test *****" << endl;

    typedef tuple<int, double, string, char> t1_t;
    t1_t t1(3,4.5,string("essai"),'X');
    pout << "Type: " << TypeTraits<t1_t>::name() << endl;
    pout << "Value: " << t1 << endl;

    srepr = tostring(t1,PStream::plearn_ascii);
    fromsrepr = openString(srepr, PStream::plearn_ascii);
    t1_t t1_fromascii;
    fromsrepr >> t1_fromascii;
    pout << "Value reread from plearn_ascii serialization: " << t1_fromascii << endl;
    pout << "Same? " << (t1_fromascii==t1) << endl;

    srepr = tostring(t1,PStream::plearn_binary);
    fromsrepr = openString(srepr, PStream::plearn_binary);
    t1_t t1_frombinary;
    fromsrepr >> t1_frombinary;
    pout << "Value reread from plearn_binary serialization: " << t1_frombinary << endl;
    pout << "Same? " << (t1_frombinary==t1) << endl;

    // test a complex tuple

    pout << "\n***** Complex tuple test *****" << endl;

    typedef tuple<int, 
        float, 
        tuple<string, pair<double, int>, map<string, float> >,
        char> t2_t;
    map<string, float> m;
    m["key1"] = 1.0;
    m["key2"] = 2.0;
    m["key3"] = 3.0;
    t2_t t2(36, 
            3.25,
            make_tuple(string("mot1"), make_pair(4.5,3), m),
            'Z');
    pout << "Type: " << TypeTraits<t2_t>::name() << endl;
    pout << "Value: " << t2 << endl;

    srepr = tostring(t2,PStream::plearn_ascii);
    fromsrepr = openString(srepr, PStream::plearn_ascii);
    t2_t t2_fromascii;
    fromsrepr >> t2_fromascii;
    pout << "Value reread from plearn_ascii serialization: " << t2_fromascii << endl;
    pout << "Same? " << (t2_fromascii==t2) << endl;

    srepr = tostring(t2,PStream::plearn_binary);
    fromsrepr = openString(srepr, PStream::plearn_binary);
    t2_t t2_frombinary;
    fromsrepr >> t2_frombinary;
    pout << "Value reread from plearn_binary serialization: " << t2_frombinary << endl;
    pout << "Same? " << (t2_frombinary==t2) << endl;

    pout << "\n**** ALL DONE *****" << endl;
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
