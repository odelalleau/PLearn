// -*- C++ -*-

// Copyright (C) 2004 Université de Montréal
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
 * $Id: FieldConvertCommand.h,v 1.19 2004/07/21 16:30:49 chrish42 Exp $
 ******************************************************* */

#ifndef FieldConvertCommand_INC
#define FieldConvertCommand_INC

#include "PLearnCommand.h"
#include "PLearnCommandRegistry.h"
#include <plearn/math/pl_math.h>    //!< For 'real'.

namespace PLearn {
using namespace std;

class FieldConvertCommand: public PLearnCommand
{
public:

  //! Default empty constructor.
  FieldConvertCommand();

  //! The available types for a field.
  enum FieldType {
    binary,
    constant,
    continuous,
    discrete_corr,
    discrete_uncorr,
    skip,
    unknown //< Default value before the decision on the field type is made.
  };
  
  virtual void run(const vector<string>& args);

protected:

  // Convert a string into our enum type.
  FieldType stringToFieldType(string s);

  static PLearnCommandRegistry reg_;

  real DISCRETE_TOLERANCE;
  real UNIQUE_NMISSING_FRACTION_TO_ASSUME_CONTINUOUS;
  real PVALUE_THRESHOLD;
  real FRAC_MISSING_TO_SKIP;
  real FRAC_ENOUGH;
  string source_fn, desti_fn,force_fn,report_fn;
  string precompute;
  int target;
  FieldType type;
};

  
} // end of namespace PLearn

#endif

