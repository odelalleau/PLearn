// -*- C++ -*-

// UCISpecification.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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
 * $Id$ 
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file UCISpecification.cc */


#include "UCISpecification.h"

namespace PLearn {
using namespace std;

UCISpecification::UCISpecification() 
    : header_exists(0),
      header_fields(0),
      file_all(""),
      file_test(""),
      file_train(""),
      format("UCI"),
      inputsize(-1),
      targetsize(-1),
      weightsize(-1),
      target_is_first(0)
{
}

PLEARN_IMPLEMENT_OBJECT(UCISpecification,
                        "Describes the specifications of a UCI database.",
                        "This object specifies characteristics of a database from the UCI machine\n"
                        "learning repository, such as the input size, target size, etc...\n"
                        "It is intended to be used in a script put in the same directory as the\n"
                        "database, in order to be loaded by the getDataSet() method.\n"
                        "\n"
                        "NB (Olivier): I am re-doing some stuff in there, it may be a bit messy...\n"
    );

void UCISpecification::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "header_exists", &UCISpecification::header_exists, OptionBase::buildoption,
                  "Specifies if there is a header for the dataset");

    declareOption(ol, "header_fields", &UCISpecification::header_fields, OptionBase::buildoption,
                  "The fields that are present in the header");

    declareOption(ol, "data_all", &UCISpecification::data_all, OptionBase::buildoption,
                  "Filename for the whole dataset.");

    declareOption(ol, "data_train", &UCISpecification::data_train, OptionBase::buildoption,
                  "Filename for the train data.");

    declareOption(ol, "data_test", &UCISpecification::data_test, OptionBase::buildoption,
                  "Filename for the test data.");

    declareOption(ol, "file_all", &UCISpecification::file_all, OptionBase::buildoption,
                  "DEPRECATED: The filename for the whole dataset");

    declareOption(ol, "file_train", &UCISpecification::file_train, OptionBase::buildoption,
                  "DEPRECATED: The filename for the train data.");

    declareOption(ol, "file_test", &UCISpecification::file_test, OptionBase::buildoption,
                  "DEPRECATED: The filename for the test data.");

    declareOption(ol, "format", &UCISpecification::format, OptionBase::buildoption,
                  "The format of the dataset file.");

    declareOption(ol, "inputsize", &UCISpecification::inputsize, OptionBase::buildoption,
                  "Input size of the data");

    declareOption(ol, "targetsize", &UCISpecification::targetsize, OptionBase::buildoption,
                  "Target size of the data");

    declareOption(ol, "weightsize", &UCISpecification::weightsize, OptionBase::buildoption,
                  "Weight size of the data");

    declareOption(ol, "target_is_first", &UCISpecification::target_is_first, OptionBase::buildoption,
                  "Whether the target is the first column (otherwise it is assumed to be the last one).");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void UCISpecification::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

// ### Nothing to add here, simply calls build_
void UCISpecification::build()
{
    inherited::build();
    build_();
}

void UCISpecification::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("UCISpecification::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
