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
 * $Id: FieldConvertCommand.h,v 1.14 2004/03/11 19:11:28 tihocan Exp $
 ******************************************************* */

#ifndef FieldConvertCommand_INC
#define FieldConvertCommand_INC

#include "PLearnCommand.h"
#include "PLearnCommandRegistry.h"
#include "pl_math.h"    //!< For 'real'.

namespace PLearn {
using namespace std;

class FieldConvertCommand: public PLearnCommand
{
public:
  FieldConvertCommand():
    PLearnCommand("FieldConvert",

                  "Reads a dataset and generates a .vmat file based on the data, but optimized for training.\n",

                  "The nature of each field of the original dataset is automatically detected, and determines the approriate treatment.\n"\
                  "The possible field types with the corresponding treatment can be one of :\n"\
                  "continuous      - quantitative data (data is real): the field is replaced by the normalized data (minus means, divided by stddev)\n"\
                  "discrete_uncorr - discrete integers (qualitative data, e.g : postal codes, categories) not corr. with target: the field is replaced by a group of fields in a one-hot fashion.\n"\
                  "discrete_corr   - discrete integers, correlated with target : both the normalised and the onehot versions of the field are used in the new dataset\n"\
                  "constant        - constant data : the field is skipped (it is not present in the new dataset)\n"\
                  "skip            - unrelevant data : the field is skipped (it is not present in the new dataset)\n"\
                  "\n"\
                  "When there are ambiguities, messages are displayed for the problematic field(s) and they are skipped. The user must use a 'force' file,\n"\
                  "to explicitely force the types of the ambiguous field(s). The file is made of lines of the following 2 possible formats:\n"\
                  "FIELDNAME=type\n"\
                  "fieldNumberA-fieldNumberB=type   [e.g : 200-204=constant, to force a range]\n"\
                  "\n"\
                  "Note that all types but skip, if the field contains missing values, an additionnal 'missing-bit' field is added and is '1' only for missing values.\n"\
                  "The difference between types constant and skip is only cosmetic: constant means the field is constant, while skip means either there are too many missing values or it has been forced to skip.\n"\
                  "A report file is generated and contains the information about the processing for each field.\n"\
                  "Target index of source needs to be specified (ie. to perform corelation test). It can be any field of the "\
                  "source dataset, but will be the last field of the new dataset.*** We assume target is never missing *** \n\n"\
                  "usage : FieldConvert *source=[source dataset] *destination=[new dataset with vmat extension] *target=[field index of target]\n"\
                  "force=[force file] report=[report file] fraction=[if number of unique values is > than 'fraction' * NonMISSING -> the field is continuous. Default=.3] "\
                  "max_pvalue=[maximum pvalue to assume correlation with target, default=0.025] frac_missing_to_skip=[if MISSING >= 'frac_missing_to_skip * number of samples then this field is skipped, default=1.0] "\
                  "frac_enough=[if a field is discrete, only values represented by at least frac_enough * nSamples elements will be considered, default=0.005] "\
                  "precompute=[\"none\" | \"pmat\" | ... : possibly add a <PRECOMPUTE> tag in the destination, default=\"none\"]\n"\
                  "\nfields with asterix * are not optional\n"
                  ) 
  {}
                    
  //! The available types for a field.
  enum FieldType {
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

