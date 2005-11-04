// -*- C++ -*-

// PTest.cc
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

/*! \file PTest.cc */


#include "PTest.h"
#include <plearn/io/fileutils.h>        //!< For pathexists(..)
#include <plearn/io/load_and_save.h>    //!< For save(..)


namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PTest,
    "Base class for PLearn internal tests.",
    ""
);

///////////
// PTest //
///////////
PTest::PTest():
    save(true)
{}

///////////
// build //
///////////
void PTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("PTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void PTest::declareOptions(OptionList& ol)
{
    // Option 'name' is declared as 'nosave' because most PTest subclasses will
    // either use the default name (= classname()), or set their own name,
    // without needing to have the user set it by himself.
    // However, a subclass may redeclare this option as a 'buildoption' if
    // needed.
    declareOption(ol, "name", &PTest::name, OptionBase::nosave,
        "The name of this test. If left empty, it will be set to classname()\n"
        "at build time.");

    declareOption(ol, "save", &PTest::save, OptionBase::buildoption,
        "If set to 1, this object will be saved to 'save_path'.");

    declareOption(ol, "save_path", &PTest::save_path, OptionBase::buildoption,
        "The file where this test object should be saved (if empty, it will\n"
        "be set to 'name'.psave).");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void PTest::build_()
{
    if (name.empty())
        name = this->classname();
}

/////////
// run //
/////////
void PTest::run()
{
    this->perform();
    if (save) {
        PPath file = save_path.isEmpty() ? PPath(name + ".psave") : save_path;
        if (pathexists(file))
            PLERROR("In PTest::run - File '%s' already exists",
                    file.errorDisplay().c_str());
        PP<PTest> test_to_save = this;
        PLearn::save(file, test_to_save);
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
