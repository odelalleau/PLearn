// -*- C++ -*-

// test_AutoVMatrix.cc
//
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
   * $Id: test_AutoVMatrix.cc,v 1.1 2005/01/23 22:11:29 dorionc Exp $ 
   ******************************************************* */

// Authors: Christian Dorion

/*! \file test_AutoVMatrix.cc */

#include <plearn/io/pl_log.h>
#include <plearn/vmat/AutoVMatrix.h>
#include <plearn/math/TMat_maths.h>

using namespace PLearn;

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
    cerr << "FATAL ERROR: " << e.message() << endl << endl;
  }
  catch (...)
  {
    cerr << "FATAL ERROR: uncaught unknown exception" << endl << endl;
  }
}

int main()
{
  try {
    PL_Log::instance().verbosity(VLEVEL_NORMAL);
    
    UNIT_TEST("data.amat");
    UNIT_TEST("data.pmat");
    UNIT_TEST("PLEARNDIR:test_suite/data/eslt_mixture/data_train.amat");
  }
  catch(const PLearnError& e)
  {
    cerr << "FATAL ERROR: " << e.message() << endl << endl;
  }
  catch (...)
  {
    cerr << "FATAL ERROR: uncaught unknown exception" << endl << endl;
  }
  
  return 0;
}
