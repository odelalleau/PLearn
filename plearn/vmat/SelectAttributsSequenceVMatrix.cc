// -*- C++ -*-

// SelectAttributsSequenceVMatrix.cc
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

/*! \file SelectAttributsSequenceVMatrix.cc */


#include "SelectAttributsSequenceVMatrix.h"
#include <plearn/base/tostring.h>

namespace PLearn {
using namespace std;


SelectAttributsSequenceVMatrix::SelectAttributsSequenceVMatrix()
    :inherited(), n_left_context(2), n_right_context(2), conditions_offset(0), conditions_for_exclusion(1), full_context(true),
     put_only_target_output(false), use_last_context(true)
    /* ### Initialise all fields to their default value */
{
}

SelectAttributsSequenceVMatrix::SelectAttributsSequenceVMatrix(VMat s, int l_context,int r_context)
    :inherited(),conditions_offset(0), conditions_for_exclusion(1), full_context(true),
     use_last_context(true)
    /* ### Initialise all fields to their default value */
{
    source = s;
    n_left_context= l_context;
    n_right_context = r_context;
    build();
}
  
PLEARN_IMPLEMENT_OBJECT(SelectAttributsSequenceVMatrix,
                        "This VMatrix takes a VMat of a sequence of elements (symbol and attributs) and constructs context rows.",
                        "The elements' symbol and attributs have to be coded using integers, and the source VMat string mapping\n"
                        "is also used with the functions getStringVal(...) and getValString(...).\n"
                        "The context rows can be of fixed length, or constrained by delimiter symbols.\n"
                        "Certain rows can be selected/excluded, and certain elements can be excluded of\n"
                        "a context according to some conditions on the symbol or its attributs.\n"
                        "The conditions are expressed as disjunctions of conjunctions. For example: \n\n"
                        "[ [ 0 : \"person\", 1 : \"NN\" ] , [ 0 : \"kind\", 2 : \"plural\" ] ] \n\n"
                        "is equivalent in C++ logic form to : \n\n"
                        "(fields[0]==\"person\" && fields[1]==\"NN\") || (fields[0]==\"kind\" && fields[2]==\"plural\").\n\n"
                        "Conditions can be expressed in string or int format. The string mapping is used to make the correspondance.\n"
    );
  
void SelectAttributsSequenceVMatrix::getNewRow(int i, const Vec& v) const
{
    if(i<0 || i>=length_) PLERROR("In SelectAttributSequenceVMatrix::getNewRow() : invalid row acces i=%d for VMatrix(%d,%d)",i,length_,width_);
  
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
                element = current_row_i.subVec(n_attributs*position,n_attributs);
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
            left_context.subVec(n_attributs*left_added_elements,n_attributs) << element;
            if(use_last_context) left_positions[left_added_elements] = p;
            this_lower_bound = p;
            left_added_elements++;
        }
        else
            if(!full_context)
            {
                left_context.subVec(n_attributs*left_added_elements,n_attributs).fill(MISSING_VALUE);
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
                element = current_row_i.subVec(n_attributs*position,n_attributs);
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
            right_context.subVec(n_attributs*right_added_elements,n_attributs) << element;
            if(use_last_context) right_positions[right_added_elements] = p;
            this_upper_bound = p;
            right_added_elements++;
        }
        else
            if(!full_context)
            {
                right_context.subVec(n_attributs*right_added_elements,n_attributs).fill(MISSING_VALUE);
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

    int cp = 0;

    if(fixed_context)  // Adding missing value, for the case where the context is of fixed length
        if(n_left_context-left_added_elements>0)
        {
            current_row_i.subVec(cp*n_attributs,n_attributs*(n_left_context-left_added_elements)).fill(MISSING_VALUE);
            cp = n_left_context-left_added_elements;
        }

    // adding left context

    int temp = left_added_elements;

    while(temp > 0)
    {
        temp--;
        current_row_i.subVec(cp*n_attributs,n_attributs) << left_context.subVec(temp*n_attributs,n_attributs);
        if(use_last_context) current_context_pos[(int)left_positions[temp]] = cp;
        cp++;
    }

    // adding target element
    int target_position = cp;
    current_row_i.subVec(cp*n_attributs,n_attributs) << target_element;
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
        current_row_i.subVec(cp*n_attributs,n_attributs) << right_context.subVec(r*n_attributs,n_attributs);
        if(use_last_context) current_context_pos[(int)right_positions[r]] = cp;
        r++;
        cp++;
    }

    if(current_row_i.length()-cp*n_attributs > 0)
        current_row_i.subVec(cp*n_attributs,current_row_i.length()-cp*n_attributs).fill(MISSING_VALUE);

    // Seperating input and output fields

    for(int t=0; t<current_row_i.length(); t++)
    {
        if(t%n_attributs < source->inputsize())
            v[t/n_attributs * source->inputsize() + t%n_attributs] = current_row_i[t];
        else
            if(put_only_target_output)
            {
                if(t/n_attributs == target_position)
                    v[inputsize_ + t%n_attributs - source->inputsize() ] = current_row_i[t];
            }
            else
                v[inputsize_ + t/n_attributs * source->targetsize() + t%n_attributs - source->inputsize() ] = current_row_i[t];
    }

    return;
}

void SelectAttributsSequenceVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "n_left_context", &SelectAttributsSequenceVMatrix::n_left_context, OptionBase::buildoption,
                  "Number of elements at the left of the target element (if < 0, all elements to the left are included until a delimiter is met)");
    declareOption(ol, "n_right_context", &SelectAttributsSequenceVMatrix::n_right_context, OptionBase::buildoption,
                  "Number of elements at the right of the target element (if < 0, all elements to the right are included until a delimiter is met)");
    declareOption(ol, "conditions_offset", &SelectAttributsSequenceVMatrix::conditions_offset, OptionBase::buildoption,
                  "Offset for the position of the element on which conditions are tested (default = 0)");
    declareOption(ol, "conditions_for_exclusion", &SelectAttributsSequenceVMatrix::conditions_for_exclusion, OptionBase::buildoption,
                  "Indication that the specified conditions are for the exclusion (true) or inclusion (false) of elements in the VMatrix");
    declareOption(ol, "full_context", &SelectAttributsSequenceVMatrix::full_context, OptionBase::buildoption,
                  "Indication that ignored elements of context should be replaced by the next nearest valid element");
    declareOption(ol, "put_only_target_output", &SelectAttributsSequenceVMatrix::put_only_target_output, OptionBase::buildoption,
                  "Indication that the only output fields of a VMatrix row should be the output fields of the target element");
    declareOption(ol, "use_last_context", &SelectAttributsSequenceVMatrix::use_last_context, OptionBase::buildoption,
                  "Indication that the last accessed context should be put in a buffer.");
    declareOption(ol, "conditions", &SelectAttributsSequenceVMatrix::conditions, OptionBase::buildoption,
                  "Conditions to be satisfied for the exclusion or inclusion (see conditions_for_exclusion) of elements in the VMatrix");
    declareOption(ol, "string_conditions", &SelectAttributsSequenceVMatrix::string_conditions, OptionBase::buildoption,
                  "Conditions, in string format, to be satisfied for the exclusion or inclusion (see conditions_for_exclusion) of elements in the VMatrix");
    declareOption(ol, "delimiters", &SelectAttributsSequenceVMatrix::delimiters, OptionBase::buildoption,
                  "Delimiters of context");
    declareOption(ol, "string_delimiters", &SelectAttributsSequenceVMatrix::string_delimiters, OptionBase::buildoption,
                  "Delimiters, in string format, of context");
    declareOption(ol, "ignored_context", &SelectAttributsSequenceVMatrix::ignored_context, OptionBase::buildoption,
                  "Elements to be ignored in context");
    declareOption(ol, "string_ignored_context", &SelectAttributsSequenceVMatrix::string_ignored_context, OptionBase::buildoption,
                  "Elements, in string format, to be ignored in context");
    declareOption(ol, "source", &SelectAttributsSequenceVMatrix::source, OptionBase::buildoption,
                  "Source VMat, from which contexts are extracted");
  

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void SelectAttributsSequenceVMatrix::build_()
{
  
    if(!source) PLERROR("In SelectAttributSequenceVMatrix::build_() : no source defined");
  
    //  defineSizes(source->inputsize(),source->targetsize(),source->weightsize()); pas bon car ecrase declare options
    n_attributs = source->width();
    row.resize(n_attributs);
    element.resize(n_attributs);
    target_element.resize(n_attributs);

    fixed_context = n_left_context >=0 && n_right_context >= 0;

    // from string to int format on ...

    // conditions
    from_string_to_int_format(string_conditions, conditions);
    string_conditions.clear();

    // delimiters
    from_string_to_int_format(string_delimiters, delimiters);
    string_delimiters.clear();

    // ignored_context
    from_string_to_int_format(string_ignored_context, ignored_context);
    string_ignored_context.clear();

    // gathering information from source VMat

    indices.clear();
    int current_context_length = 0;
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
            if(is_true(conditions,row))
            {
                if(!conditions_for_exclusion) indices.append(i);
            }
            else
            {
                if(conditions_for_exclusion) indices.append(i);
            }
        }
        pb->update(i+1);
    }

    max_context_length = max_context_length < current_context_length ? current_context_length : max_context_length;

    if(n_left_context >= 0 && n_right_context >= 0)
        max_context_length = 1 + n_left_context + n_right_context;

    length_ = indices.length();
    width_ = n_attributs*(max_context_length);

    inputsize_ = max_context_length * source->inputsize();
    targetsize_ = max_context_length * source->targetsize();
    weightsize_ = source->weightsize();

    if(inputsize_+targetsize_ != width_) PLERROR("In SelectAttributsSequenceVMatrix:build_() : inputsize_ + targetsize_ != width_");

    current_row_i.resize(width_);
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
        left_context.resize(n_attributs*n_left_context);
        left_positions.resize(n_left_context);
    }
   
    if(n_right_context < 0) 
    {
        right_context.resize(width_);
        right_positions.resize(max_context_length);
    }
    else 
    {
        right_context.resize(n_attributs*n_right_context);
        right_positions.resize(n_right_context);
    }

    if(put_only_target_output)
    {
        targetsize_ = source->targetsize();
        width_ = inputsize_ + targetsize();
    }

}

void SelectAttributsSequenceVMatrix::build()
{
    inherited::build();
    build_();
}

real SelectAttributsSequenceVMatrix::getStringVal(int col, const string & str) const
{
    if(source)
    {
        int src_col;
        if(col < inputsize_)
            src_col = col%source->inputsize();
        else
            src_col = source->inputsize() + (col-inputsize_)%source->targetsize();
        return source->getStringVal(src_col,str);
    }
      
    return MISSING_VALUE;
}

string SelectAttributsSequenceVMatrix::getValString(int col, real val) const
{
    if(source)
    {
        int src_col;
        if(col < inputsize_)
            src_col = col%source->inputsize();
        else
            src_col = source->inputsize() + (col-inputsize_)%source->targetsize();
        return source->getValString(src_col,val);
    }
      
    return "";
}

Vec SelectAttributsSequenceVMatrix::getValues(int row, int col) const
{
    if(row < 0 || row >= length_) PLERROR("In SelectAttributsSequenceVMatrix::getValues() : invalid row %d, length()=%d", row, length_);
    if(col < 0 || col >= length_) PLERROR("In SelectAttributsSequenceVMatrix::getValues() : invalid col %d, width()=%d", col, width_);
    if(source)
    {
        int src_col;
        if(col < inputsize_)
            src_col = col%source->inputsize();
        else
            src_col = source->inputsize() + (col-inputsize_)%source->targetsize();
        return source->getValues(indices[row],src_col);
    }
    return Vec(0);
}

Vec SelectAttributsSequenceVMatrix::getValues(const Vec& input, int col) const
{
    if(col < 0 || col >= length_) PLERROR("In SelectAttributsSequenceVMatrix::getValues() : invalid col %d, width()=%d", col, width_);
    if(source)
    {
        int src_col;
        if(col < inputsize_)
            src_col = col%source->inputsize();
        else
            src_col = source->inputsize() + (col-inputsize_)%source->targetsize();
        return source->getValues(input,src_col);
    }
    return Vec(0);
}

int SelectAttributsSequenceVMatrix::getDictionarySize(int row, int col) const
{
    if(row < 0 || row >= length_) PLERROR("In SelectAttributsSequenceVMatrix::getDictionarySize() : invalid row %d, length()=%d", row, length_);
    if(col < 0 || col >= length_) PLERROR("In SelectAttributsSequenceVMatrix::getDictionarySize() : invalid col %d, width()=%d", col, width_);
    if(source)
    {
        int src_col;
        if(col < inputsize_)
            src_col = col%source->inputsize();
        else
            src_col = source->inputsize() + (col-inputsize_)%source->targetsize();
        return source->getDictionarySize(indices[row],src_col);
    }
    return -1;
}

void SelectAttributsSequenceVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(conditions, copies);
    deepCopyField(delimiters, copies);
    deepCopyField(ignored_context, copies);
    deepCopyField(source, copies);
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
