// -*- C++ -*-

// PyPlearnDriver.h
//
// Copyright (C) 2004 ApSTAT Technologies Inc. 
// All rights reserved.
//
// This program may not be modified, translated, 
// copied or distributed in whole or in part or 
// as part of a derivative work, in any form, 
// whether as source code or binary code or any 
// other form of translation, except if authorized 
// by a prior agreement signed with ApSTAT Technologies Inc.

/* *******************************************************      
   * $Id: PyPlearnDriver.h,v 1.1 2004/12/07 22:38:46 chapados Exp $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file PyPlearnDriver.h */

#ifndef PyPlearnDriver_INC
#define PyPlearnDriver_INC

// From C++
#include <string>
#include <vector>

namespace PLearn {

/**
 * Given a filename, call an external process with the given name (default
 * = "pyplearn_driver.py") to preprocess it and return the preprocessed
 * version.  Arguments to the subprocess can be passed.  The first element,
 * args[0], is supposed to be some program name, and is never touched.  Of
 * the following arguments, passing '--help' passes it unmodified to the
 * subprogram and is assumed to print a help string.  If '--dump' is
 * passed, the Python-preprocessed script is printed to standard output.
 * If either '--dump' or '--help' is passed, nothing is returned from this
 * function. 
 */
std::string process_pyplearn_script(
  const std::string& filename,
  const std::vector<std::string>& args = std::vector<std::string>(),
  const std::string& drivername = "pyplearn_driver.py");

} // end of namespace PLearn

#endif
