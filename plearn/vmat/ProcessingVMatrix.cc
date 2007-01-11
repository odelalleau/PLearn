// -*- C++ -*-

// ProcessingVMatrix.cc
//
// Copyright (C) 2003 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file ProcessingVMatrix.cc */


#include "ProcessingVMatrix.h"

namespace PLearn {
using namespace std;


ProcessingVMatrix::ProcessingVMatrix()
    :inherited()
{
}

PLEARN_IMPLEMENT_OBJECT(
    ProcessingVMatrix,
    "VMatrix whose rows are processed using a VPL script",
    "This VMatrix class processes each for of a source VMatrix using VPL scripts.\n"
    "The result of this VMatrix can be defined in one of two ways. The first way is to\n"
    "use a single program (in option 'prg') to process the whole row\n"
    "and set the inputsize, targetsize, weightsize and extrasize variables to defines\n"
    "which part of the output of the VPL program is treated as input, target, etc.\n"
    "of the VMatrix \n"
    "\n"
    "The second way is to have separate programs for each of the input, target, etc.\n"
    "parts of the VMatrix (using options 'input_prg', 'target_prg', 'weight_prg' and\n"
    "extra_prg), and then the respective inputsize, etc. will be inferred automatically\n"
    "from the length of the output of each respective VPL program.\n"
    "\n"
    "Note that in both cases, an empty program will produce a vector of length 0 as output\n"
    "If you want for example the output target vector of this VMatrix to be a copy of its\n"
    "input target vector, this must be done explicitly using a short VPL program.\n"
    "\n"
    "See class VMatLanguage for a description of the VPL syntax."
    );

void ProcessingVMatrix::getNewRow(int i, const Vec& v) const
{
    if(!prg.empty()) // we'll use prg rather than the *_prg
        program.run(i,v);
    else // we'll use the *_prg
    {
        if(!input_prg.empty())
            input_prg_.run(i, v.subVec(0, inputsize_));
        if(!target_prg.empty())
            target_prg_.run(i, v.subVec(inputsize_, targetsize_));
        if(!weight_prg.empty())
            weight_prg_.run(i, v.subVec(inputsize_+targetsize_, weightsize_));
        if(!extra_prg.empty())
            extra_prg_.run(i, v.subVec(inputsize_+targetsize_+weightsize_, extrasize_));
    }
}

void ProcessingVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "prg", &ProcessingVMatrix::prg, OptionBase::buildoption,
                  "The VPL code to be applied to each row of the vmat\n"
                  "You can either specify prg and inputsie, targetsize, weightsize, extrasize\n"
                  "OR ALTERNATIVELY leave prg empty, and specify one or several of \n"
                  "input_prg, target_prg, weight_prg, extra_prg. In this latter case,\n"
                  "the sizes will be set from the corresponding program's number of output fieldnames \n");

    declareOption(ol, "input_prg", &ProcessingVMatrix::input_prg, OptionBase::buildoption,
                  "This option is considered only if prg is the empty string\n"
                  "Program string in VPL language to be applied to each raw input \n"
                  "to generate the new preprocessed input.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n");

    declareOption(ol, "target_prg", &ProcessingVMatrix::target_prg, OptionBase::buildoption,
                  "This option is considered only if prg is the empty string\n"
                  "Program string in VPL language to be applied to a dataset row\n"
                  "to generate a proper target for the underlying learner.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n");
  
    declareOption(ol, "weight_prg", &ProcessingVMatrix::weight_prg, OptionBase::buildoption,
                  "This option is considered only if prg is the empty string\n"
                  "Program string in VPL language to be applied to a dataset row\n"
                  "to generate a proper weight for the underlying learner.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n");

    declareOption(ol, "extra_prg", &ProcessingVMatrix::extra_prg, OptionBase::buildoption,
                  "This option is considered only if prg is the empty string\n"
                  "Program string in VPL language to be applied to a dataset row\n"
                  "to generate proper extra fields for the underlying learner.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ProcessingVMatrix::build_()
{
    // Do not do anything until we get the source VMat.
    if (!source)
        return;

    length_ = source->length();

    TVec<string> fieldnames;
    if(!prg.empty()) // ignore the *_prg
    {
        program.setSource(source);
        program.compileString(prg,fieldnames); 
        int nfields = fieldnames.size();
        width_ = nfields;

        declareFieldNames(fieldnames);
        setMetaInfoFromSource();

        if(inputsize_<0)
            inputsize_ = nfields;
        if(targetsize_<0)
            targetsize_ = 0;            
        if(weightsize_<0)
            weightsize_ = 0;
        if(extrasize_<0)
            extrasize_ = 0;            
    }
    else // use the *_prg
    {
        
        TVec<string> addfieldnames;
        if(!input_prg.empty())
        {
            input_prg_.setSource(source);
            input_prg_.compileString(input_prg, addfieldnames);
            inputsize_ = addfieldnames.length();
            fieldnames.append(addfieldnames);
        }
        else
            inputsize_ = 0;

        if(!target_prg.empty())
        {
            target_prg_.setSource(source);
            target_prg_.compileString(target_prg, addfieldnames);
            targetsize_ = addfieldnames.length();
            fieldnames.append(addfieldnames);
        }
        else
            targetsize_ = 0;

        if(!weight_prg.empty())
        {
            weight_prg_.setSource(source);
            weight_prg_.compileString(weight_prg, addfieldnames);
            weightsize_ = addfieldnames.length();
            fieldnames.append(addfieldnames);
        }
        else
            weightsize_ = 0;

        if(!extra_prg.empty())
        {
            extra_prg_.setSource(source);
            extra_prg_.compileString(extra_prg, addfieldnames);
            extrasize_ = addfieldnames.length();
            fieldnames.append(addfieldnames);
        }
        else
            extrasize_ = 0;
        
        width_ = inputsize_+targetsize_+weightsize_+extrasize_;
        declareFieldNames(fieldnames);
        setMetaInfoFromSource();
    }

    sourcevec.resize(source->width());
}

void ProcessingVMatrix::build()
{
    inherited::build();
    build_();
}

void ProcessingVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(sourcevec, copies);
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
