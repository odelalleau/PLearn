// -*- C++ -*-

// AutoVMatrixTest.cc
//
// Copyright (C) 2005 Nicolas Chapados
// Copyright (C) 2005 Olivier Delalleau 
// Copyright (C) 2005 Christian Dorion
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

// Authors: Nicolas Chapados, Olivier Delalleau, Christian Dorion

/*! \file AutoVMatrixTest.cc */


#include "AutoVMatrixTest.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    AutoVMatrixTest,
    "Test various VMat conversions.",
    ""
);

//////////////////
// AutoVMatrixTest //
//////////////////
AutoVMatrixTest::AutoVMatrixTest() 
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
void AutoVMatrixTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void AutoVMatrixTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("AutoVMatrixTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void AutoVMatrixTest::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &AutoVMatrixTest::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void AutoVMatrixTest::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

void save_load_compare( const AutoVMatrix& vm,
                        const PPath& prefix,
                        const PPath& base,
                        const string& ext, int dot )
{
    PPath save_to = prefix + PPath(base).replace(dot, base.length(), ext);
    if ( ext == ".amat" )
        vm.saveAMAT( save_to );
    else if ( ext == ".pmat" )
        vm.savePMAT( save_to );
    else if ( ext == ".dmat" )
        vm.saveDMAT( save_to );
    else
        PLERROR("!!!");
  
//!<   AutoVMatrix reloaded( save_to );
//!<   bool success = ( vm.toMat() == reloaded.toMat() ); 
//!<   if ( success )
//!<     MAND_LOG << "Save and load suceeded on " << save_to << endl << endl;
//!<   else
//!<     MAND_LOG << "!!! Save and load FAILED on " << save_to << endl << endl;
}

void unitTest(const PPath& path)
{
    AutoVMatrix vm(path);
    MAND_LOG << vm << endl;

    Mat m(vm);  
    MAND_LOG << m << endl;

    PPath base       = path.basename();
    unsigned int dot = base.rfind('.');
    base[dot]        = '_';
    PPath prefix     = base + "__to__";

    save_load_compare( vm, prefix, base, ".amat", dot );
    save_load_compare( vm, prefix, base, ".pmat", dot );
    save_load_compare( vm, prefix, base, ".dmat", dot );
}

inline void UNIT_TEST(const string& argument)
{
    MAND_LOG << plhead(argument) << endl;
    try {
        unitTest(argument);
        MAND_LOG << endl;
    }
    catch(const PLearnError& e)
    {
        perr << "FATAL ERROR: " << e.message() << endl << endl;
    }
    catch (...)
    {
        perr << "FATAL ERROR: uncaught unknown exception" << endl << endl;
    }
}


/////////////
// perform //
/////////////
void AutoVMatrixTest::perform()
{
    try {
        PL_Log::instance().verbosity(VLEVEL_NORMAL);
        PL_Log::instance().outmode(PStream::plearn_ascii);

        UNIT_TEST("PLEARNDIR:examples/data/test_suite/linear_4x_2y.amat");
        UNIT_TEST("PLEARNDIR:examples/data/test_suite/linear_4x_2y.pmat");
        UNIT_TEST("PLEARNDIR:examples/data/test_suite/eslt_mixture/data_train.amat");
    }
    catch(const PLearnError& e)
    {
        perr << "FATAL ERROR: " << e.message() << endl << endl;
    }
    catch (...)
    {
        perr << "FATAL ERROR: uncaught unknown exception" << endl << endl;
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
