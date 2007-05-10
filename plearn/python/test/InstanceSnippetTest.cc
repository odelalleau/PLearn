// -*- C++ -*-

// InstanceSnippetTest.cc
//
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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

// Authors: Xavier Saint-Mleux

/*! \file InstanceSnippetTest.cc */


#include "InstanceSnippetTest.h"
#include <plearn_learners/testers/PTester.h>
#include <plearn/vmat/TrainTestSplitter.h>
#include <plearn/sys/procinfo.h>
#include <boost/regex.hpp>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    InstanceSnippetTest,
    "ONE LINE USER DESCRIPTION",
    "MULTI LINE\nHELP FOR USERS"
);

//////////////////
// InstanceSnippetTest //
//////////////////
InstanceSnippetTest::InstanceSnippetTest()
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
void InstanceSnippetTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void InstanceSnippetTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("InstanceSnippetTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void InstanceSnippetTest::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &InstanceSnippetTest::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void InstanceSnippetTest::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.
}

/////////////
// perform //
/////////////
void InstanceSnippetTest::perform()
{
    TestSnippet zz1;
    zz1.build();

    int n= zz1.invoke("getN");

    zz1.invoke("test0");
    
    PP<PTester> t= new PTester();//on heap, please
    t->final_commands= TVec<string>(2, "allo");
    t->splitter= new TrainTestSplitter();
    t->learner= zz1.ll;
    zz1.invoke("chkObj", t);

    zz1.invoke("testRec", n);
    zz1.invoke("testRec2", (Object*)zz1.ll);

    try
    {
        zz1.invoke("testRecCrash", 3);
    }
    catch(const PythonException& e)
    {
        string exception_msg = e.message();
        // Remove memory addresses from the exception message, as they may
        // differ from one computer to another, causing the test to fail.
        boost::regex memory_adr("0x[abcdef0123456789]{6,}",
                                boost::regex::perl|boost::regex::icase);
        string msg_without_sys_dependent_data =
            regex_replace(exception_msg, memory_adr,
                    "0x[memory_address]");
        boost::regex python_ver("Python 2\\.[0-9]\\.[0-9]",
                                boost::regex::perl|boost::regex::icase);
        msg_without_sys_dependent_data =
            regex_replace(msg_without_sys_dependent_data, python_ver,
                    "Python 2.X.Y");
        boost::regex plearn_path("(/[a-zA-Z0-9_-]+)+/python_modules/",
                                boost::regex::perl);
        msg_without_sys_dependent_data =
            regex_replace(msg_without_sys_dependent_data, plearn_path,
                    "PLEARN_PYTHON_PATH:");
        pout << "[INTENDED ERROR] Caught Python Exception: '" 
             << msg_without_sys_dependent_data
             << "'" << endl;
    }

    PythonCodeSnippet zz2("\nimport gc\n"
                          "gc.collect()\n"
                          "print 'nrefs 1010', len(gc.get_referrers(1010))\n");
    zz2.build();

    TVec<PyObject*> refs= zz1.invoke("getRecTestReferrers");

    pout << "refs0: " << (refs[0] == zz1.m_compiled_code.getPyObject()) << endl;
    PyObject* dic= PyObject_GetAttrString(zz1.m_instance.getPyObject(), "__dict__");
    pout << "refs1: " << (refs[1] ==  dic) << endl;
    Py_DECREF(dic);


#ifndef PYTEST_ACTIVE
    for(int i= 0; i < 1000; ++i)
    {
        zz1.invoke("testRec", n);
        zz1.ll->nstages= n;
        zz1.invoke("testRec2", (Object*)zz1.ll);
        pout << "at iter " << i << "\t: " << getProcessDataMemory() << endl;
    }
#endif //ndef PYTEST_ACTIVE



    zz1.invoke("fini");

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
