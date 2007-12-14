// -*- C++ -*-

// PLCheckTest.cc
//
// Copyright (C) 2007 Pascal Lamblin
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

/*! \file PLCheckTest.cc */


#include "PLCheckTest.h"
#include <plearn/base/stringutils.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PLCheckTest,
    "Test of the PLCHECK and PLCHECK_MSG macros",
    "PLCHECK and PLCHECK_MSG are analogous to PLASSERT and PLASSERT_MSG,\n"
    "but they're always performed, even in optimized mode.\n"
);

//////////////////
// PLCheckTest //
//////////////////
PLCheckTest::PLCheckTest()
{
}

///////////
// build //
///////////
void PLCheckTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PLCheckTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

////////////////////
// declareOptions //
////////////////////
void PLCheckTest::declareOptions(OptionList& ol)
{
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void PLCheckTest::build_()
{
}

/////////////
// perform //
/////////////
void PLCheckTest::perform()
{
    // ### The test code should go here.
    int one = 1;
    int ein = 1;
    int stein = 12;

// PLCHECK uses __FILE__ variable in its message, but we want the test
// to have the same output, regardless of the absolute path of this file
#undef __FILE__
#define __FILE__ "PLCheckTest.cc"

// Similarly, PLCHECK uses __PRETTY_FUNCTION__, but we want the test to have
// the same output regardless of small variations in the function display.
// In particular, GCC displays the 'virtual' keyword, but not ICC.
    string pl_assert_func = PL_ASSERT_FUNCTION;
    search_replace(pl_assert_func, "virtual ", "");
#undef PL_ASSERT_FUNCTION
#define PL_ASSERT_FUNCTION (pl_assert_func.c_str())
    PLCHECK( one == ein );
    PLCHECK_MSG( ein == stein, "ein != stein" );
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
