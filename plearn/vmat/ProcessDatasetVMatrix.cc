// -*- C++ -*-

// ProcessDatasetVMatrix.cc
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

/*! \file ProcessDatasetVMatrix.cc */


#include "ProcessDatasetVMatrix.h"
#include <plearn/io/fileutils.h>                  //!< For isfile().
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn/vmat/ShiftAndRescaleVMatrix.h>
#include <plearn/vmat/SubVMatrix.h>

namespace PLearn {
using namespace std;

///////////////////////////
// ProcessDatasetVMatrix //
///////////////////////////
ProcessDatasetVMatrix::ProcessDatasetVMatrix():
    max_mbytes_disk     (1000),
    max_mbytes_memory   (30),
    duplicate           ("none"),
    input_normalization ("none"),
    precompute          ("auto"),
    target_normalization("none")
{}

PLEARN_IMPLEMENT_OBJECT(ProcessDatasetVMatrix,
                        "Perform some standard preprocessing over a dataset.",
                        ""
    );

////////////////////
// declareOptions //
////////////////////
void ProcessDatasetVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "source", &ProcessDatasetVMatrix::source, OptionBase::buildoption,
                  "The underlying VMatrix.");

    declareOption(ol, "input_normalization", &ProcessDatasetVMatrix::input_normalization, OptionBase::buildoption,
                  "Kind of normalization performed on the input features:\n"
                  " - 'none'      : no normalization\n"
                  " - 'standard'  : rescaled to have mean = 0 and standard deviation = 1\n");

    declareOption(ol, "target_normalization", &ProcessDatasetVMatrix::target_normalization, OptionBase::buildoption,
                  "Kind of normalization performed on the target features (see 'input_normalization')");

    declareOption(ol, "duplicate", &ProcessDatasetVMatrix::duplicate, OptionBase::buildoption,
                  "What to do with duplicated / conflicting samples:\n"
                  " - 'all'                      : nothing (all samples are kept)\n"
                  " - 'no_same_input'            : samples must have a unique input  (first one is kept)\n"
                  " - 'no_same_input_and_target' : only exact duplicates are removed (first one is kept)\n"
                  "Note: duplicated samples will only be removed after normalization is performed.");

    declareOption(ol, "precompute", &ProcessDatasetVMatrix::precompute, OptionBase::buildoption,
                  "How to precompute the dataset:\n"
                  " - 'none'   : it is not precomputed\n"
                  " - 'memory' : it is precomputed in memory\n"
                  " - 'disk'   : it is precomputed in the underlying VMat metadatadir\n"
                  " - 'auto'   : it is precomputed in memory if it takes less than 'max_mbytes' Mb,\n"
                  "              it is precomputed in the underlying VMat metadatadir if it takes\n"
                  "              less than 'max_mbytes_disk' Mb, otherwise it is not precomputed.\n");

    declareOption(ol, "max_mbytes_memory", &ProcessDatasetVMatrix::max_mbytes_memory, OptionBase::buildoption,
                  "Maximum number of megabytes allowed in memory when 'precompute' is set to 'auto'");

    declareOption(ol, "max_mbytes_disk", &ProcessDatasetVMatrix::max_mbytes_disk, OptionBase::buildoption,
                  "Maximum number of megabytes allowed on disk when 'precompute' is set to 'auto'");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Hide unused options.

    redeclareOption(ol, "vm", &ProcessDatasetVMatrix::vm, OptionBase::nosave,
                    "Defined at build time.");

}

///////////
// build //
///////////
void ProcessDatasetVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ProcessDatasetVMatrix::build_()
{
    if (!source) {
        // Empty VMatrix.
        vm = 0;
        inherited::build();
        return;
    }
    if (source->inputsize() < 0 || source->targetsize() < 0 || source->weightsize() < 0)
        PLERROR("In ProcessDatasetVMatrix::build_ - The source VMat's sizes must be defined");
    updateMtime(source);
    vm = source;
    PPath filename_base = string("processed_dataset")
        + "-input_normalization="  + input_normalization
        + "-target_normalization=" + target_normalization
        + "-duplicate="            + duplicate;

    bool target_normalization_is_performed = false;
    if (input_normalization == "none") {
    } else if (input_normalization == "standard") {
        int n_inputs = vm->inputsize();
        if (target_normalization == "standard") {
            target_normalization_is_performed = true;
            n_inputs += vm->targetsize();
        }
        vm = new ShiftAndRescaleVMatrix(vm, n_inputs);
    } else
        PLERROR("In ProcessDatasetVMatrix::build_ - Unknown value for the "
                "'input_normalization' option: %s", input_normalization.c_str());

    if (!target_normalization_is_performed) {
        if (target_normalization == "none") {
        } else if (target_normalization == "standard") {
            VMat noninput_part = new SubVMatrix(vm, 0, vm->inputsize(), vm->length(),
                                                vm->targetsize() + vm->weightsize());
            VMat input_part = new SubVMatrix(vm, 0, 0, vm->length(), vm->inputsize());
            noninput_part = new ShiftAndRescaleVMatrix(noninput_part, vm->targetsize());
            VMat result = new ConcatColumnsVMatrix(input_part, noninput_part);
            result->defineSizes(vm->inputsize(), vm->targetsize(), vm->weightsize());
            vm = result;
        }
    }

    if (duplicate == "all") {
    } else if (duplicate == "no_same_input") {
        if (!source->hasMetaDataDir())
            PLERROR("In ProcessDatasetVMatrix::build_ - The source VMatrix needs to have "
                    "a metadata directory in order to compute duplicated samples");
        PPath meta = source->getMetaDataDir();
        // TODO Continue here!
    }

    int n = source->length();
    int w = source->width();
    string precomp = precompute;
    if (precompute == "auto") {
        // Need to find out whether to precompute in memory or on disk.
        real memory_used = n / real(1024) * w / real(1024) * sizeof(real);
        // pout << "Memory used: " << memory_used << " Mbs" << endl;
        if (memory_used <= max_mbytes_memory)
            precomp = "memory";
        else if (memory_used <= max_mbytes_disk)
            precomp = "disk";
        else
            precomp = "none";
    }

    if (precomp == "none") {
    } else if (precomp == "memory") {
        vm = new MemoryVMatrix(vm);
    } else if (precomp == "disk") {
        if (!source->hasMetaDataDir())
            PLERROR("In ProcessDatasetVMatrix::build_ - The source VMatrix needs to have "
                    "a metadata directory in order to precompute on disk");
        PPath metadata = source->getMetaDataDir();
        PPath filename = metadata / (filename_base + ".pmat");
        bool need_recompute = true;
        VMat old_vm;
        if (isfile(filename)) {
            old_vm = new FileVMatrix(filename);
            if (old_vm->length() == n && old_vm->width() == w)
                need_recompute = false;
        }
        if (need_recompute) {
            vm->savePMAT(filename);
            vm = new FileVMatrix(filename);
        } else
            vm = old_vm;
    } else
        PLERROR("In ProcessDatasetVMatrix::build_ - Unknown value for the "
                "'precompute' option: '%s'", precompute.c_str());

    inherited::build();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ProcessDatasetVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ProcessDatasetVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
