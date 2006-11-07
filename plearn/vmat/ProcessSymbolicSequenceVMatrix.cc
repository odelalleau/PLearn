// -*- C++ -*-

// ProcessSymbolicSequenceVMatrix.cc
//
// Copyright (C) 2004 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file ProcessSymbolicSequenceVMatrix.cc */


#include "ProcessSymbolicSequenceVMatrix.h"
#include <plearn/base/tostring.h>

namespace PLearn {
using namespace std;


ProcessSymbolicSequenceVMatrix::ProcessSymbolicSequenceVMatrix(bool call_build_)
    : inherited(call_build_),
      n_left_context(2), n_right_context(2),
      conditions_offset(0), conditions_for_exclusion(1), full_context(true),
      put_only_target_attributes(false), use_last_context(true), 
      exclude_missing_target_tokens(false)
    /* ### Initialise all fields to their default value */
{
    if( call_build_ )
        build_();
}

ProcessSymbolicSequenceVMatrix::ProcessSymbolicSequenceVMatrix(VMat s,
                                                               int l_context,
                                                               int r_context,
                                                               bool call_build_)
    : inherited(s, call_build_),
      conditions_offset(0), conditions_for_exclusion(1), full_context(true),
      use_last_context(true)
    /* ### Initialise all fields to their default value */
{
    n_left_context= l_context;
    n_right_context = r_context;
    if( call_build_ )
        build_();
}

PLEARN_IMPLEMENT_OBJECT(ProcessSymbolicSequenceVMatrix,
"Takes a VMat of a sequence of symbolic elements and constructs context rows.",
"This VMatrix takes a VMat of a sequence of symbolic elements (corresponding\n"
"to a set of symbolic attributes) and constructs context rows.\n"
"An example of a sequence of elements would be a sequence of words, with\n"
"their lemma form and POS tag\n"
"This sequence is encoded using integers, and is represented by the source\n"
"VMatrix such as each row is an element, and each column is a certain type\n"
"of attribute (e.g. lemma, POS tag, etc.).\n"
"The source VMat string mapping (functions getStringVal(...) and\n"
"getValString(...)) contains the integer/string encoding of the symbolic\n"
"data.\n"
"The context rows can be of fixed length, or constrained by delimiter\n"
"symbols.\n"
"Certain rows can be selected/excluded, and certain elements can be excluded\n"
"of a context according to some conditions on its attributes.\n"
"The conditions are expressed as disjunctions of conjunctions. For example:\n"
"\n"
"[ [ 0 : \"person\", 1 : \"NN\" ] , [ 0 : \"kind\", 2 : \"plural\" ] ]\n"
"\n"
"is equivalent in C++ logic form to:\n"
"\n"
"(fields[0]==\"person\" && fields[1]==\"NN\") ||\n"
"    (fields[0]==\"kind\" && fields[2]==\"plural\").\n"
"\n"
"Conditions can be expressed in string or int format. The integer/string\n"
"mapping is used to make the correspondance.\n"
"We call the 'target element' of a context the element around which other\n"
"elements are collected to construct the context.\n"
);

void ProcessSymbolicSequenceVMatrix::getNewRow(int i, const Vec& v) const
{
    if(i<0 || i>=length_)
        PLERROR("In SelectAttributeSequenceVMatrix::getNewRow() :\n"
                " invalid row acces i=%d for VMatrix(%d,%d)\n",
                i,length_,width_);

    current_row_i.resize(n_attributes*(max_context_length));
    int cp,target_position;
    fill_current_row(i,cp,target_position);

    if(current_row_i.length()-cp*n_attributes > 0)
        current_row_i.subVec(cp*n_attributes,current_row_i.length()-cp*n_attributes).fill(MISSING_VALUE);

    // Seperating input and output fields

    for(int t=0; t<current_row_i.length(); t++)
    {
        if(t%n_attributes < source->inputsize())
            v[t/n_attributes * source->inputsize() + t%n_attributes] = current_row_i[t];
        else
            if(put_only_target_attributes)
            {
                if(t/n_attributes == target_position)
                    v[max_context_length*source->inputsize() + t%n_attributes - source->inputsize() ] = current_row_i[t];
            }
            else
                v[max_context_length*source->inputsize() + t/n_attributes * source->targetsize() + t%n_attributes - source->inputsize() ] = current_row_i[t];
    }

    return;
}

void ProcessSymbolicSequenceVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "n_left_context",
                  &ProcessSymbolicSequenceVMatrix::n_left_context,
                  OptionBase::buildoption,
                  "Number of elements at the left of (or before) the target"
                  " element.\n"
                  "(if < 0, all elements to the left are included until a"
                  " delimiter is met)\n");

    declareOption(ol, "n_right_context",
                  &ProcessSymbolicSequenceVMatrix::n_right_context,
                  OptionBase::buildoption,
                  "Number of elements at the right of (or after) the target"
                  " element.\n"
                  "(if < 0, all elements to the right are included until a"
                  " delimiter is met)\n");

    declareOption(ol, "conditions_offset",
                  &ProcessSymbolicSequenceVMatrix::conditions_offset,
                  OptionBase::buildoption,
                  "Offset for the position of the element on which conditions"
                  " are tested\n"
                  "(default = 0)\n");

    declareOption(ol, "conditions_for_exclusion",
                  &ProcessSymbolicSequenceVMatrix::conditions_for_exclusion,
                  OptionBase::buildoption,
                  "Indication that the specified conditions are for the"
                  " exclusion (true)\n"
                  "or inclusion (false) of elements in the VMatrix\n");

    declareOption(ol, "full_context",
                  &ProcessSymbolicSequenceVMatrix::full_context,
                  OptionBase::buildoption,
                  "Indication that ignored elements of context should be"
                  " replaced by the\n"
                  "next nearest valid element\n");

    declareOption(ol, "put_only_target_attributes",
                  &ProcessSymbolicSequenceVMatrix::put_only_target_attributes,
                  OptionBase::buildoption,
                  "Indication that the only target fields of the VMatrix rows"
                  " should be\n"
                  "the (target) attributes of the context's target element\n");

    declareOption(ol, "use_last_context",
                  &ProcessSymbolicSequenceVMatrix::use_last_context,
                  OptionBase::buildoption,
                  "Indication that the last accessed context should be put in"
                  " a buffer.\n");

    declareOption(ol, "exclude_missing_target_tokens",
                  &ProcessSymbolicSequenceVMatrix::exclude_missing_target_tokens,
                  OptionBase::buildoption,
                  "Indication to exclude from the VMatrix the context centered\n"
                  "around a token with missing value target fields.\n");



    declareOption(ol, "conditions",
                  &ProcessSymbolicSequenceVMatrix::conditions,
                  OptionBase::buildoption,
                  "Conditions to be satisfied for the exclusion or inclusion"
                  " (see\n"
                  "conditions_for_exclusion) of elements in the VMatrix\n");

    declareOption(ol, "string_conditions",
                  &ProcessSymbolicSequenceVMatrix::string_conditions,
                  OptionBase::buildoption,
                  "Conditions, in string format, to be satisfied for the"
                  " exclusion or\n"
                  "inclusion (see conditions_for_exclusion) of elements in the"
                  " VMatrix\n");

    declareOption(ol, "delimiters",
                  &ProcessSymbolicSequenceVMatrix::delimiters,
                  OptionBase::buildoption,
                  "Delimiters of context\n");

    declareOption(ol, "string_delimiters",
                  &ProcessSymbolicSequenceVMatrix::string_delimiters,
                  OptionBase::buildoption,
                  "Delimiters, in string format, of context\n");

    declareOption(ol, "ignored_context",
                  &ProcessSymbolicSequenceVMatrix::ignored_context,
                  OptionBase::buildoption,
                  "Elements to be ignored in context\n");

    declareOption(ol, "string_ignored_context",
                  &ProcessSymbolicSequenceVMatrix::string_ignored_context,
                  OptionBase::buildoption,
                  "Elements, in string format, to be ignored in context\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ProcessSymbolicSequenceVMatrix::build_()
{

    if(!source)
        PLERROR("In SelectAttributeSequenceVMatrix::build_() : no source"
                " defined");

    //  defineSizes(source->inputsize(),source->targetsize(),source->weightsize()); pas bon car ecrase declare options
    n_attributes = source->width();
    row.resize(n_attributes);
    element.resize(n_attributes);
    target_element.resize(n_attributes);
    max_context_length = 0;

    fixed_context = n_left_context >=0 && n_right_context >= 0;

    // from string to int format on ...

    // conditions
    from_string_to_int_format(string_conditions, conditions);
    string_conditions.clear();
    string_conditions.resize(0);

    // delimiters
    from_string_to_int_format(string_delimiters, delimiters);
    string_delimiters.clear();
    string_delimiters.resize(0);

    // ignored_context
    from_string_to_int_format(string_ignored_context, ignored_context);
    string_ignored_context.clear();
    string_ignored_context.resize(0);

    // gathering information from source VMat

    indices.clear();
    indices.resize(0);
    int current_context_length = 0;
    bool target_contains_missing = false;
    ProgressBar *pb = new ProgressBar("Gathering information from source VMat of length " + tostring(source->length()), source->length());
    for(int i=0; i<source->length(); i++)
    {
        source->getRow(i,row);

        if(is_true(delimiters,row))
        {
            max_context_length = max_context_length < current_context_length ? current_context_length : max_context_length;
            current_context_length = 0;
        }
        else
        {
            if(!full_context || !is_true(ignored_context,row)) current_context_length++;
        }

        // Testing conditions for inclusion/exclusion

        if(i + conditions_offset >= 0 && i + conditions_offset < source->length())
        {
            source->getRow(i+conditions_offset,row);
            if(exclude_missing_target_tokens)
            {
                target_contains_missing = false;
                int ni = source->targetsize()+source->inputsize();
                for(int j=source->inputsize(); j<ni; j++)
                    if(is_missing(row[j]))
                    {
                        target_contains_missing = true;
                        break;
                    }
            }
            
            if(!exclude_missing_target_tokens || !target_contains_missing)
            {
                if(is_true(conditions,row))
                {
                    if(!conditions_for_exclusion) indices.append(i);
                }
                else
                {
                    if(conditions_for_exclusion) indices.append(i);
                }
            }
        }
        pb->update(i+1);
    }

    max_context_length = max_context_length < current_context_length ? current_context_length : max_context_length;

    if(n_left_context >= 0 && n_right_context >= 0)
        max_context_length = 1 + n_left_context + n_right_context;

    length_ = indices.length();
    width_ = n_attributes*(max_context_length);

    if(inputsize_ < 0) inputsize_ = max_context_length * source->inputsize();
    if(targetsize_ < 0 && put_only_target_attributes) targetsize_ = source->targetsize();
    if(targetsize_ < 0 && !put_only_target_attributes) targetsize_ = max_context_length * source->targetsize();
    if(weightsize_ < 0) weightsize_ = source->weightsize();
    if(weightsize_ > 0) PLERROR("In ProcessSymbolicSequenceVMatrix:build_() : does not support weightsize > 0");

    current_row_i.resize(n_attributes*(max_context_length));
    current_target_pos = -1;
    current_context_pos.clear();
    lower_bound = 1;
    upper_bound = -1;

    if(n_left_context < 0)
    {
        left_context.resize(width_);
        left_positions.resize(max_context_length);
    }
    else
    {
        left_context.resize(n_attributes*n_left_context);
        left_positions.resize(n_left_context);
    }

    if(n_right_context < 0)
    {
        right_context.resize(width_);
        right_positions.resize(max_context_length);
    }
    else
    {
        right_context.resize(n_attributes*n_right_context);
        right_positions.resize(n_right_context);
    }

    if(put_only_target_attributes)
    {
        width_ = source->inputsize()*max_context_length + source->targetsize();
    }

    if(inputsize_+targetsize_ != width_) PLERROR("In ProcessSymbolicSequenceVMatrix:build_() : inputsize_ + targetsize_ != width_");

    // Should we call:
    // setMetaInfoFromSource(); // ?

}

void ProcessSymbolicSequenceVMatrix::build()
{
    inherited::build();
    build_();
}

real ProcessSymbolicSequenceVMatrix::getStringVal(int col, const string & str) const
{
    if(source)
    {
        int src_col;
        if(col < max_context_length * source->inputsize())
            src_col = col%source->inputsize();
        else
            src_col = source->inputsize() + (col-max_context_length * source->inputsize())%source->targetsize();
        return source->getStringVal(src_col,str);
    }

    return MISSING_VALUE;
}

string ProcessSymbolicSequenceVMatrix::getValString(int col, real val) const
{
    if(source)
    {
        int src_col;
        if(col < max_context_length * source->inputsize())
            src_col = col%source->inputsize();
        else
            src_col = source->inputsize() + (col-max_context_length * source->inputsize())%source->targetsize();
        return source->getValString(src_col,val);
    }

    return "";
}

void ProcessSymbolicSequenceVMatrix::getValues(int row, int col, Vec& values) const
{
    if(row < 0 || row >= length_) PLERROR("In ProcessSymbolicSequenceVMatrix::getValues() : invalid row %d, length()=%d", row, length_);
    if(col < 0 || col >= width_) PLERROR("In ProcessSymbolicSequenceVMatrix::getValues() : invalid col %d, width()=%d", col, width_);
    if(source)
    {
        int src_col;
        if(col < max_context_length * source->inputsize())
            src_col = col%source->inputsize();
        else
            src_col = source->inputsize() + (col-max_context_length * source->inputsize())%source->targetsize();
        source->getValues(indices[row],src_col, values);
    }
    else
        values.resize(0);
}

void ProcessSymbolicSequenceVMatrix::getValues(const Vec& input, int col, Vec& values) const
{
    if(col < 0 || col >= width_) PLERROR("In ProcessSymbolicSequenceVMatrix::getValues() : invalid col %d, width()=%d", col, width_);
    if(source)
    {
        int sinputsize = source->inputsize();
        int stargetsize = source->targetsize();
        int src_col,this_col;
        if(col < max_context_length * sinputsize)
            src_col = col%sinputsize;
        else
            src_col = sinputsize + (col-max_context_length * sinputsize)%stargetsize;

        // Fill subinput input part
        subinput.resize(sinputsize);
        if(col>=inputsize_)
        {
            if(put_only_target_attributes)
                this_col = n_left_context;
            else
                this_col = (col-inputsize_)/stargetsize;
        }
        else
            this_col = col/sinputsize;
        subinput << input.subVec(this_col*sinputsize,sinputsize);

        // Fill subinput target part
        subinput.resize(sinputsize+stargetsize);
        if(!put_only_target_attributes || col == inputsize_ || (col<inputsize_ && col/sinputsize == n_left_context))
        {
            if(put_only_target_attributes)
                subinput.subVec(sinputsize,stargetsize) << input.subVec(inputsize_,stargetsize);
            else
                subinput.subVec(sinputsize,stargetsize) << input.subVec(inputsize_+this_col*stargetsize,stargetsize);
                
        }
        else
        {
            subinput.subVec(sinputsize,stargetsize).fill(MISSING_VALUE);
        }
        source->getValues(subinput,src_col,values);
    }
    else values.resize(0);
}

PP<Dictionary> ProcessSymbolicSequenceVMatrix::getDictionary(int col) const
{
    if(col < 0 || col >= width_) PLERROR("In ProcessSymbolicSequenceVMatrix::getDictionary() : invalid col %d, width()=%d", col, width_);
    if(source)
    {
        int src_col;
        if(col < max_context_length * source->inputsize())
            src_col = col%source->inputsize();
        else
            src_col = source->inputsize() + (col-max_context_length * source->inputsize())%source->targetsize();
        return source->getDictionary(src_col);
    }
    return 0;
}

void ProcessSymbolicSequenceVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(conditions, copies);
    deepCopyField(string_conditions, copies);
    deepCopyField(delimiters, copies);
    deepCopyField(string_delimiters, copies);
    deepCopyField(ignored_context, copies);
    deepCopyField(string_ignored_context, copies);
    deepCopyField(current_row_i, copies);
    deepCopyField(indices, copies);
    deepCopyField(left_positions, copies);
    deepCopyField(right_positions, copies);
    deepCopyField(left_context, copies);
    deepCopyField(right_context, copies);
    deepCopyField(subinput, copies);
    deepCopyField(row, copies);
    deepCopyField(element, copies);
    deepCopyField(target_element, copies);
}

void ProcessSymbolicSequenceVMatrix::fill_current_row(int i, int& cp, int& target_position) const
{

    int target = indices[i];

    // If the target element is a delimiter, do nothing: Hugo: may not be a good thing!
    source->getRow(target,target_element);
    /*
      if(is_true(delimiters,target_element))
      {
      v.fill(MISSING_VALUE);
      if(use_last_context)
      {
      current_context_pos.clear();
      current_row_i.fill(MISSING_VALUE);
      current_target_pos = -1;
      lower_bound = 1;
      upper_bound = -1;
      }
      return;
      }
    */

    int left_added_elements = 0;
    int right_added_elements = 0;

    int left_pos = 1;
    int right_pos = 1;
    int p = 0;

    bool to_add = false;

    int this_lower_bound = target;
    int this_upper_bound = target;

    // Left context construction

    while(n_left_context < 0 || left_added_elements < n_left_context)
    {
        to_add = false;


        p = target-left_pos;

        if(p < 0 || p >= source->length())
            break;

        // Verifying if context can be found in last context
        if(use_last_context && lower_bound <= p && upper_bound >= p)
        {
            if(current_context_pos.find(p) != current_context_pos.end())
            {
                int position = current_context_pos[p];
                element = current_row_i.subVec(n_attributes*position,n_attributes);
                if(!is_true(ignored_context,element))
                {
                    to_add = true;
                }
            }
            else
            {
                left_pos++;
                continue;
            }
        }  // If not, fetch element in source VMat
        else
        {
            source->getRow(p,element);
            if(!is_true(ignored_context,element))
                to_add = true;
        }

        if(is_true(delimiters,element)) break;

        if(to_add)
        {
            left_context.subVec(n_attributes*left_added_elements,n_attributes) << element;
            if(use_last_context) left_positions[left_added_elements] = p;
            this_lower_bound = p;
            left_added_elements++;
        }
        else
            if(!full_context)
            {
                left_context.subVec(n_attributes*left_added_elements,n_attributes).fill(MISSING_VALUE);
                if(use_last_context) left_positions[left_added_elements] = p;
                this_lower_bound = p;
                left_added_elements++;
            }

        left_pos++;
    }

    // Right context construction

    while(n_right_context < 0 || right_added_elements < n_right_context)
    {
        to_add = false;

        p = target+right_pos;

        if(p < 0 || p >= source->length())
            break;

        // Verifying if context can be found in last context
        if(use_last_context && lower_bound <= p && upper_bound >= p)
        {
            if(current_context_pos.find(p) != current_context_pos.end())
            {
                int position = current_context_pos[p];
                element = current_row_i.subVec(n_attributes*position,n_attributes);
                if(!is_true(ignored_context,element))
                {
                    to_add = true;
                }
            }
            else
            {
                right_pos++;
                continue;
            }
        }  // If not, fetch element in source VMat
        else
        {
            source->getRow(p,element);
            if(!is_true(ignored_context,element))
                to_add = true;
        }

        if(is_true(delimiters,element)) break;

        if(to_add)
        {
            right_context.subVec(n_attributes*right_added_elements,n_attributes) << element;
            if(use_last_context) right_positions[right_added_elements] = p;
            this_upper_bound = p;
            right_added_elements++;
        }
        else
            if(!full_context)
            {
                right_context.subVec(n_attributes*right_added_elements,n_attributes).fill(MISSING_VALUE);
                if(use_last_context) right_positions[right_added_elements] = p;
                this_upper_bound = p;
                right_added_elements++;
            }

        right_pos++;
    }

    current_context_pos.clear();
    current_target_pos = -1;
    lower_bound = this_lower_bound;
    upper_bound = this_upper_bound;
    //current_row_i.fill(MISSING_VALUES);

    // Constructing complete row

    cp = 0;

    if(fixed_context)  // Adding missing value, for the case where the context is of fixed length
        if(n_left_context-left_added_elements>0)
        {
            current_row_i.subVec(cp*n_attributes,n_attributes*(n_left_context-left_added_elements)).fill(MISSING_VALUE);
            cp = n_left_context-left_added_elements;
        }

    // adding left context

    int temp = left_added_elements;

    while(temp > 0)
    {
        temp--;
        current_row_i.subVec(cp*n_attributes,n_attributes) << left_context.subVec(temp*n_attributes,n_attributes);
        if(use_last_context) current_context_pos[(int)left_positions[temp]] = cp;
        cp++;
    }

    // adding target element
    target_position = cp;
    current_row_i.subVec(cp*n_attributes,n_attributes) << target_element;
    if(use_last_context)
    {
        current_context_pos[target] = cp;
        current_target_pos = cp;
    }
    cp++;

    // adding right context

    int r=0;
    while(r<right_added_elements)
    {
        current_row_i.subVec(cp*n_attributes,n_attributes) << right_context.subVec(r*n_attributes,n_attributes);
        if(use_last_context) current_context_pos[(int)right_positions[r]] = cp;
        r++;
        cp++;
    }

    if(fixed_context && current_row_i.length()-cp*n_attributes > 0)
    {
        current_row_i.subVec(cp*n_attributes,current_row_i.length()-cp*n_attributes).fill(MISSING_VALUE);
        cp = current_row_i.length()/n_attributes;
    }
}


void ProcessSymbolicSequenceVMatrix::getExample(int i, Vec& input, Vec& target, real& weight)
{
    if(i<0 || i>=length_) PLERROR("In SelectAttributeSequenceVMatrix::getExample() : invalid row acces i=%d for VMatrix(%d,%d)",i,length_,width_);

    int target_position,cp;
    fill_current_row(i,cp,target_position);

    // Seperating input and output fields
    input.resize(cp*source->inputsize());
    target.resize(put_only_target_attributes ? source->targetsize() : cp*source->targetsize());
    if(weightsize_ == 0)
        weight = 1;
    else
        PLERROR("In ProcessSymbolicSequenceVMatrix::getExample(): weighsize() > 0 not implemented");
    for(int t=0; t<cp*n_attributes; t++)
    {
        if(t%n_attributes < source->inputsize())
            input[t/n_attributes * source->inputsize() + t%n_attributes] = current_row_i[t];
        else
            if(put_only_target_attributes)
            {
                if(t/n_attributes == target_position)
                    target[t%n_attributes - source->inputsize() ] = current_row_i[t];
            }
            else
                target[t/n_attributes * source->targetsize() + t%n_attributes - source->inputsize() ] = current_row_i[t];
    }

    if(fixed_context)
    {
        // Verify if inputsize and targetsize were redefined
        if(inputsize_ > input.length())
        {
            input.resize(inputsize_);
            int it = 0;
            for(int i=cp*source->inputsize(); i<inputsize_; i++)
                input[i] = target[it++];
            for(int i=0; i<targetsize_; i++)
                target[i] = target[it++];
            target.resize(targetsize_);
        }
        else if(targetsize() > target.length())
        {
            target.resize(targetsize_);
            int it = cp*source->inputsize()-inputsize_;
            for(int i=0; it<targetsize_; i++)
                target[it++] = target[i];
            it = inputsize_;
            for(int i=0; it<input.length(); i++)
                target[i] = input[it++];
            input.resize(inputsize_);
        }
    }

    return;
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
