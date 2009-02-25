// -*- C++ -*-

// DichotomizeVMatrix.cc
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

/*! \file DichotomizeVMatrix.cc */


#include "DichotomizeVMatrix.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    DichotomizeVMatrix,
    "Dichotomize variables with discrete values",
    "Instructions are provided with the discrete_variable_instructions option.\n"
    "Can map range of value to one value for each variable\n"
    "Variables with no specification will be kept as_is\n"
    );

DichotomizeVMatrix::DichotomizeVMatrix():
    verbose(3),
    missing_field_error(true)
/* ### Initialize all fields to their default value */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

void DichotomizeVMatrix::getNewRow(int i, const Vec& v) const
{
    Vec source_row(source->width());
    source->getRow(i,source_row);
    
    for (int source_col = 0,output_col = 0; source_col < source->width(); source_col++)
        {
            if (instruction_index[source_col] < 0)
            {
                v[output_col] = source_row[source_col];
                output_col += 1;
            }
            else
            {
               TVec<pair<real, real> > instruction_ptr = discrete_variable_instructions[instruction_index[source_col]].second;
               if (instruction_ptr.size() == 0) continue;
               bool missing = is_missing(source_row[source_col]);
               bool found_range = false;
               if(missing)
                   found_range=true;
               for (int ins_col = 0; ins_col < instruction_ptr.size(); ins_col++)
               {
                   if (missing)
                       v[output_col] = MISSING_VALUE;
                   else if (source_row[source_col] < instruction_ptr[ins_col].first 
                            || source_row[source_col] > instruction_ptr[ins_col].second) 
                       v[output_col] = 0.0;
                   else 
                   {
                       v[output_col] = 1.0;
                       found_range=true;
                   }
                   output_col += 1;
               }
               if(found_range==false && verbose>2)
               {
                   PLWARNING("DichotomizeVMatrix::getNewRow() - "
                             "row %d, fields %s, have a value (%f) that are outside all dichotomize range",
                             i,source->fieldName(source_col).c_str(),source_row[source_col]);
               }
            }    
        }

}

void DichotomizeVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "discrete_variable_instructions", &DichotomizeVMatrix::discrete_variable_instructions,
                  OptionBase::buildoption,
                  "The instructions to dichotomize the variables in the form of field_name : TVec<pair>.\n"
                  "The pairs are values from : to, each creating a 0, 1 variable.\n"
                  "Variables with no specification will be kept as_is.\n");

    declareOption(ol, "instruction_index", &DichotomizeVMatrix::instruction_index,
                  OptionBase::learntoption,
                  "An array that point each columns of the source matrix to its instruction.");

    declareOption(ol, "missing_field_error", &DichotomizeVMatrix::missing_field_error,
                  OptionBase::buildoption,
                  "If true we will generate an error is a field is"
                  " in the instruction but not in the source."
                  " Otherwise will generate a warning.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DichotomizeVMatrix::build_()
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

    TVec<string> source_names = source->fieldNames();

    instruction_index.resize(source->width());
    instruction_index.fill(-1);

    //validate the instruction and order them as in the source matrix
    for (int ins_col = 0; ins_col < discrete_variable_instructions.size();
         ins_col++)
    {
        int source_col;
        for (source_col = 0; source_col < source->width(); source_col++)
        {
            if (discrete_variable_instructions[ins_col].first == source_names[source_col]) break;
        }
        if (source_col >= source->width()){
            if(missing_field_error)
                PLERROR("In DichotomizeVMatrix::build_() -  "
                        "no field with this name in the source data set: %s",
                        (discrete_variable_instructions[ins_col].first).c_str());
            else
                PLWARNING("In DichotomizeVMatrix::build_() -  "
                        "no field with this name in the source data set: %s",
                        (discrete_variable_instructions[ins_col].first).c_str());
        }
        else instruction_index[source_col] = ins_col;
    }

    // initialize inputsize_,targetsize_,weightsize_, width_,length()
    length_=source->length();
    int sisize=source->inputsize();
    int stsize=source->targetsize();
    int swsize=source->weightsize();
    int isize=0;
    int tsize=0;
    int wsize=0;
    int esize=0;
    for (int source_col = 0; source_col < sisize; source_col++)
    {
        if (instruction_index[source_col] < 0) isize++;
        else
        {
            TVec<pair<real, real> > instruction_ptr = 
                discrete_variable_instructions[instruction_index[source_col]].second;
            isize += instruction_ptr.size();
        }
    }
    for (int source_col = sisize; 
         source_col < sisize + stsize; source_col++)
    {
        if (instruction_index[source_col] < 0) tsize++;
        else
        {
            TVec<pair<real, real> > instruction_ptr = 
                discrete_variable_instructions[instruction_index[source_col]].second;
            tsize += instruction_ptr.size();
        }
    }
    for (int source_col = sisize + stsize;
         source_col < sisize + stsize + swsize ; source_col++)
    {
        if (instruction_index[source_col] < 0) wsize++;
        else
        {
            TVec<pair<real, real> > instruction_ptr = 
                discrete_variable_instructions[instruction_index[source_col]].second;
            wsize += instruction_ptr.size();
        }
    }
    for (int source_col = sisize + stsize + swsize;
         source_col < sisize + stsize + swsize + source->extrasize() ; source_col++)
    {
        if (instruction_index[source_col] < 0) esize++;
        else
        {
            TVec<pair<real, real> > instruction_ptr = 
                discrete_variable_instructions[instruction_index[source_col]].second;
            esize += instruction_ptr.size();
        }
    }
    defineSizes(isize, tsize, wsize, esize);
    width_ = isize + tsize + wsize + esize;

    //get the fieldnames
    TVec<string> fnames(width());
    int field_col = 0;
    for (int source_col = 0; source_col < source->width(); source_col++)
    {
        if (instruction_index[source_col] < 0)
        {
            fnames[field_col] = source_names[source_col];
            field_col += 1;
        }
        else
        {
           TVec<pair<real, real> > instruction_ptr =
               discrete_variable_instructions[instruction_index[source_col]].second;
           if (instruction_ptr.size() == 0) 
           {
               PLWARNING("In DichotomizeVMatrix::build_() -"
                         "instruction for field %s have no range!",
                         discrete_variable_instructions[instruction_index[source_col]].first.c_str());
               continue;
           }
           for (int ins_col = 0; ins_col < instruction_ptr.size(); ins_col++)
           {
               fnames[field_col] = source_names[source_col] + "_"
                                        + tostring(instruction_ptr[ins_col].first) + "_" 
                                        + tostring(instruction_ptr[ins_col].second);
               field_col += 1;
           }
        }    
    }
    declareFieldNames(fnames);
}

// ### Nothing to add here, simply calls build_
void DichotomizeVMatrix::build()
{
    inherited::build();
    build_();
}

void DichotomizeVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(discrete_variable_instructions, copies);
    deepCopyField(instruction_index, copies);

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
