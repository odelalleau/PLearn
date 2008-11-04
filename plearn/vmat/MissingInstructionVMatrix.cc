// -*- C++ -*-

// MissingInstructionVMatrix.cc
//
// Copyright (C) 2007 Frederic Bastien
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

// Authors: Frederic Bastien

/*! \file MissingInstructionVMatrix.cc */


#include "MissingInstructionVMatrix.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    MissingInstructionVMatrix,
    "Transform some value as missing following received instruction",
    "NO HELP"
    );

MissingInstructionVMatrix::MissingInstructionVMatrix():
    default_instruction(""),
    missing_instruction_error(true),
    missing_field_error(true)
/* ### Initialize all fields to their default value */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

void MissingInstructionVMatrix::getNewRow(int i, const Vec& v) const
{
    // ...
    source->getRow(i,tmp2);
    for (int col = 0,merge_col = 0; col < source->width(); col++)
    {
        if (ins[col] == "skip") continue;
        else if (ins[col] == "as_is")
        {
            v[merge_col] = tmp2[col];
        }
        else if (ins[col] == "zero_is_missing")
        {
            if (tmp2[col] == 0.0) v[merge_col] = MISSING_VALUE;
            else v[merge_col] = tmp2[col];
        }
        else if (ins[col] == "2436935_is_missing")
        {
            if (tmp2[col] == 2436935.0) v[merge_col] = MISSING_VALUE;
            else v[merge_col] = tmp2[col];
        }
        else if (ins[col] == "present")
        {
            if (is_missing(tmp2[col])) v[merge_col] = 0.0;
            else v[merge_col] = 1.0;
        }
        merge_col +=1;
    }
}

void MissingInstructionVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &MissingInstructionVMatrix::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
    declareOption(ol, "missing_instructions", &MissingInstructionVMatrix::missing_instructions,
                  OptionBase::buildoption,
                  "The variable missing regeneration instructions in the form of pairs field : instruction.\n"
                  "Supported instructions are skip, as_is, zero_is_missing, 2436935_is_missing(01JAN1960 in julian day), present.\n"
                  "If the instruction fieldname end with '*' we will extend it to all the source matrix fieldname that match the regex.\n"
                  "No other regex are supported");
    declareOption(ol, "default_instruction",
                  &MissingInstructionVMatrix::default_instruction,
                  OptionBase::buildoption,
                  "If some field in the source matrix have no instruction," 
                  " we will use this instruction. We will warn about field"
                  " with empty instruction then will stop.");
   declareOption(ol, "missing_instruction_error",
                  &MissingInstructionVMatrix::missing_instruction_error,
                  OptionBase::buildoption,
                 "If true will generate an error if some field have field without instruction" );
   declareOption(ol, "missing_field_error",
                  &MissingInstructionVMatrix::missing_field_error ,
                  OptionBase::buildoption,
                 "If true will generate an error if some instruction reference missing field" );
}

void MissingInstructionVMatrix::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.
    updateMtime(source);
    length_ = source->length();
    ins.resize(source->width());
    tmp2.resize(source->width());
    TVec<string> source_names = source->fieldNames();

    //set default instruction
    ins.fill(default_instruction);

    int skip_instruction_input = 0;
    int skip_instruction_target = 0;
    int skip_instruction_weight = 0;
    if(default_instruction!="skip")
        width_=source->width();
    else{
        width_=missing_instructions.size();
        PLWARNING("MissingInstructionVMatrix::build_() - "
                  "we suppose that the default instruction apply only to input fields");
        skip_instruction_input = source->width() - missing_instructions.size();
    }
    int missing_field = 0;
    for (int ins_col = 0; ins_col < missing_instructions.size(); ins_col++)
    {
        int source_col = 0;
        pair<string, string> mis_ins=missing_instructions[ins_col];
        for (source_col = 0; source_col < source->width(); source_col++)
        {
            if (mis_ins.first == source_names[source_col]) break;
        }
        if (source_col >= source->width() && 
            mis_ins.first[mis_ins.first.size()-1]=='*') {
            string sub_name=mis_ins.first.substr(0,mis_ins.first.size()-1);
            for (source_col = 0; source_col < source->width(); source_col++)
            {
                if (string_begins_with(source_names[source_col],sub_name)){
                    missing_instructions.append(
                        make_pair(source_names[source_col],mis_ins.second));
                } 
            }
            continue;
        } else if (source_col >= source->width()) 
        {
            if(missing_instructions[ins_col].second!="skip"){
                //if the instruction is skip, we don't care that it is missing in the source!
                PLWARNING("In MissingInstructionVMatrix::build_() - missing_instructions '%d': no field with this name: '%s'."
                          " It have '%s' as spec"
                          ,ins_col,(missing_instructions[ins_col].first).c_str(),
                          (missing_instructions[ins_col].second).c_str());
                missing_field++;
            }
            continue;
        }
        if (missing_instructions[ins_col].second == "skip")
            ins[source_col] = "skip";
        else if (missing_instructions[ins_col].second == "as_is")
            ins[source_col] = "as_is";
        else if (missing_instructions[ins_col].second == "zero_is_missing")
            ins[source_col] = "zero_is_missing";
        else if (missing_instructions[ins_col].second == "2436935_is_missing")
            ins[source_col] = "2436935_is_missing";
        else if (missing_instructions[ins_col].second == "present")
            ins[source_col] = "present";
        else if (missing_instructions[ins_col].second.empty())
            PLWARNING("In MergeDond2Files::build_() - merge instruction empty for field '%s', we keep the previous instruction who could be the default_instruction",(missing_instructions[source_col].first).c_str());
        else PLERROR("In MergeDond2Files::build_() - unsupported merge instruction: '%s'", 
                     (missing_instructions[ins_col].second).c_str());
        if (ins[source_col] == "skip"){
            if(source_col<source->inputsize())
                skip_instruction_input++;
            else if(source_col<(source->inputsize()+source->targetsize()))
                skip_instruction_target++;
            else skip_instruction_weight++;
        }
    }
    setMetaInfoFromSource();
    inputsize_ = source->inputsize() - skip_instruction_input;
    targetsize_ = source->targetsize() - skip_instruction_target;
    weightsize_ = source->weightsize() - skip_instruction_weight;
    width_ = inputsize_ + targetsize_ + weightsize_;
    int missing_instruction = 0;
    for (int col = 0; col < source->width(); col++)
    {
        if(ins[col] == "")
        {
            PLWARNING("In MissingInstructionVMatrix::build_ - their is no instruction for the field '%s'",
                    source_names[col].c_str());
            missing_instruction++;
        }   
    }
    if(missing_instruction && missing_instruction_error)
        PLERROR("In MissingInstructionVMatrix::build_ - Their have been %d field in the source matrix that have no instruction",missing_instruction);
    if(missing_field && missing_field_error)
        PLERROR("In MissingInstructionVMatrix::build_ - Their have been %d instruction that have no correcponding field in the source matrix",missing_field);

    // Copy the appropriate VMFields
    fieldinfos.resize(width());
    if (source->getFieldInfos().size() > 0) {
        for (int source_col=0,merge_col=0; source_col<source->width(); ++source_col) {
            if(ins[source_col]!="skip"){
                fieldinfos[merge_col] = source->getFieldInfos(source_col);
                merge_col++;
            }
        }
    }

}

// ### Nothing to add here, simply calls build_
void MissingInstructionVMatrix::build()
{
    inherited::build();
    build_();
}

void MissingInstructionVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(missing_instructions, copies);
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
