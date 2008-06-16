// -*- C++ -*-

// PStreamBufTest.cc
//
// Copyright (C) 2005 Nicolas Chapados
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

// Authors: Nicolas Chapados, Olivier Delalleau

/*! \file PStreamBufTest.cc */


#include "PStreamBufTest.h"
#include <iostream>
#include <string>
#include <plearn/io/openString.h>
#include <plearn/io/StringPStreamBuf.h>


namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PStreamBufTest,
    "Test the different PStreamBuf implementations.",
    ""
);

//////////////////
// PStreamBufTest //
//////////////////
PStreamBufTest::PStreamBufTest() 
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
void PStreamBufTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PStreamBufTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("PStreamBufTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void PStreamBufTest::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &PStreamBufTest::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void PStreamBufTest::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

/// Complains if the actual and expected string parameters are not equal.
void test(const char* test_name, const std::string actual, const std::string expected)
{
    if (actual != expected)
        std::cout << test_name << ": expected '" << expected
                  << "', got '" << actual << "'" << std::endl;
}


/// Returns a string with the character, or "EOF" for the end of file value.
static std::string char_string(int c)
{
    char string_template[] = { '\'', '!', '\'', '\0' };
    if (c == EOF)
        return "EOF";
    else {
        string_template[1] = c;
        return std::string(string_template);
    }
}

/// Complains if the actual and expected char parameters are not equal.
static void test(const char* test_name, const int actual, const int expected)
{
    if (actual != expected)
        std::cout << test_name << ": expected " << char_string(expected)
                  << ", got " << char_string(actual) << "" << std::endl;
}

void test_read() {
    const std::string str = "abcdefghijklmnopqrstuvwxyz";
    PLearn::PStream s = PLearn::openString(str, PLearn::PStream::raw_ascii);

    const unsigned int inbuf_size = 6;
    const unsigned int outbuf_size = 5;
    const unsigned int ungetbuf_size = 4;
    s.setBufferCapacities(inbuf_size, outbuf_size, ungetbuf_size);

    test("Read a single char", s.get(), 'a');

    std::string temp;
  
    s.read(temp, 2);
    test("Read a couple of chars", temp, "bc");

    s.read(temp, inbuf_size - 2);
    test("Read enough to force a buffer refill", temp, "defg");

    s.read(temp, inbuf_size);
    test("Read a whole inbuf_size", temp, "hijklm");

    s.read(temp, inbuf_size + 2);
    test("Read a whole inbuf_size, and then some", temp, "nopqrstu");

    test("Peek", s.peek(), 'v');

    //test("Conversion to bool, not on EOF", bool(s), 1);//DEPRECATED
    test("stream.good(), not on EOF", s.good(), 1);
  
    s.putback('U');
    test("Putback of a single char", s.get(), 'U');

    s.unread("RSTu");
    s.read(temp, 5);
    test("Unread of 4 chars", temp, "RSTuv");

    s.read(temp, 4);
    test("Read rest of string", temp, "wxyz");

    test("EOF", s.get(), EOF);

    s.read(temp, 10);
    test("EOF, reading into a string", temp, "");

    //test("Conversion to bool on EOF", bool(s), 0);//DEPRECATED
    test("stream.good() on EOF", s.good(), 0);
  
    test("EOF, second time", s.get(), EOF);

    s.unread("1234");
    s.read(temp, 8);
    test("Unread after EOF", temp, "1234");

    // Test bug fixed in r8087 when reading a quoted string whose last
    // character is also the last quote.
    string quoted_str("\"123\"");
    s = PLearn::openString(quoted_str, PLearn::PStream::plearn_ascii);
    s >> temp;
    test("EOF at end of quoted string", s.peek(), EOF);
}


void test_write()
{
    std::string str;
    PLearn::PStream s = PLearn::openString(str, PLearn::PStream::raw_ascii, "a");

    const unsigned int inbuf_size = 6;
    const unsigned int outbuf_size = 5;
    const unsigned int ungetbuf_size = 4;
    s.setBufferCapacities(inbuf_size, outbuf_size, ungetbuf_size);

    s.put('a');
    test("Write a single char", str, "");

    s.write("bcd");
    test("Write a couple of chars", str, "");

    s.write("ef");
    test("Write to overflow the outbuf", str, "abcde");

    s.flush();
    test("Flush", str, "abcdef");

    s.write("ghijk");
    test("Write a outbuf size chunk", str, "abcdef");

    s.flush();
    test("Flush of full buffer", str, "abcdefghijk");

    s.write("lmnopqrs");
    test("Write more than outbuf size", str, "abcdefghijklmnop");

    s.write("tuvwxyzABCDE");
    test("Write more than two outbuf size", str, "abcdefghijklmnopqrstuvwxyzABCDE");

    s.flush();
    test("Flush on empty buffer", str, "abcdefghijklmnopqrstuvwxyzABCDE");

    s.write("FGHIJK");
    test("Write outbuf size plus one", str, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJ");

    s.flush();
    test("Another flush", str, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK");

    s.write("LMNOP");
    test("Fill up the buffer...", str, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK");

    s.put('Q');
    test("... and put a single char", str, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOP");
}

void test_write_unbuffered()
{
    std::string str;
    PLearn::PStream s = PLearn::openString(str, PLearn::PStream::raw_ascii, "w");

    const unsigned int inbuf_size = 6;
    const unsigned int outbuf_size = 0;
    const unsigned int ungetbuf_size = 4;
    s.setBufferCapacities(inbuf_size, outbuf_size, ungetbuf_size);

    s.write("abc");
    test("Write a short string (unbuffered)", str, "abc");

    s.write("defghijklm");
    test("Write a longer string (unbuffered)", str, "abcdefghijklm");

    s.flush();
    test("Flush (unbuffered)", str, "abcdefghijklm");  
}

void test_negchar()
{
    std::string str;
    PLearn::PStream s = PLearn::openString(str, PLearn::PStream::raw_ascii, "w");

    const unsigned int inbuf_size = 6;
    const unsigned int outbuf_size = 0;
    const unsigned int ungetbuf_size = 4;
    s.setBufferCapacities(inbuf_size, outbuf_size, ungetbuf_size);

    s.write("\xff");
    test("Write a single 'negative' character", str, "\xff");
}

/////////////
// perform //
/////////////
void PStreamBufTest::perform()
{
    using std::cerr;
    using std::endl;

    try {
        test_read();
        test_write();
        test_write_unbuffered();
        test_negchar();
    }
    catch (const PLearn::PLearnError& e) {
        cerr << "PLearnError: " << e.message() << endl;
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
