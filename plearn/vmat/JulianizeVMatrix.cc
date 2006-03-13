
// -*- C++ -*-

// JulianizeVMatrix.cc
//
// Copyright (C) 2003  Nicolas Chapados 
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

/*! \file JulianizeVMatrix.cc */
#include "JulianizeVMatrix.h"
#include <plearn/base/PDate.h>
#include <plearn/base/PDateTime.h>

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(JulianizeVMatrix,
                        "Converts dates in a several field format to a single"
                        " number format",
                        "JulianizeVMatrix provides a conversion from a VMat containing dates\n"
                        "in an explicit 3-column (YYYY,MM,DD) or 6-column (YYYY,MM,DD,HH,MM,SS)\n"
                        "format to a Julian day number format (including fractional part to\n"
                        "represent the hour within the day).  The dates can be at any columns,\n"
                        "not only columns 0-2 (or 0-5).  More than a single date can be\n"
                        "converted.\n");


JulianizeVMatrix::JulianizeVMatrix(bool call_build_)
    : inherited(call_build_)
    /* all other compiler-supplied defaults are reasonable */
{
    if( call_build_ )
        build_();
}

JulianizeVMatrix::JulianizeVMatrix(VMat the_source,
                                   DateCode date_code,
                                   int starting_column,
                                   bool call_build_)
    : inherited(the_source,
                the_source->length(),
                newWidth(the_source, date_code),
                call_build_),
      cols_codes(1),
      source_row(the_source.width())
{
    cols_codes[0] = make_pair(starting_column, date_code);
    setVMFields();
}


void JulianizeVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getRow(i, source_row);

    Vec::iterator
        src_beg = source_row.begin(),
        src_it  = source_row.begin(),
        src_end = source_row.end(),
        dst_it  = v.begin();
    vector< pair<int,DateCode> >::const_iterator
        codes_it  = cols_codes.begin(),
        codes_end = cols_codes.end();

    for ( ; codes_it < codes_end ; ++codes_it ) {
        // Copy what comes before the current date
        dst_it = copy(src_it, src_beg + codes_it->first, dst_it);
        src_it = src_beg + codes_it->first;

        // Now convert the date per se
        double converted_date = 0;
        int YYYY,MM,DD,hh,mm,ss;
        YYYY = int(*src_it++);
        MM   = int(*src_it++);
        DD   = int(*src_it++);
        switch(codes_it->second) {
        case Date:
            converted_date = PDate(YYYY,MM,DD).toJulianDay();
            break;
        case DateTime:
            hh = int(*src_it++);
            mm = int(*src_it++);
            ss = int(*src_it++);
            converted_date = PDateTime(YYYY,MM,DD,hh,mm,ss).toJulianDay();
            break;
        }
        *dst_it++ = converted_date;
    }

    // And now copy what comes after the last date
    copy(src_it, src_end, dst_it);
}

void JulianizeVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &JulianizeVMatrix::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void JulianizeVMatrix::build_()
{
    // No options to build at this point
    setMetaInfoFromSource();
}

// ### Nothing to add here, simply calls build_
void JulianizeVMatrix::build()
{
    inherited::build();
    build_();
}

void JulianizeVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(source_row, copies);
    // cols_codes already deep-copied since it is an STL vector
}

void JulianizeVMatrix::setVMFields()
{
    Array<VMField>& orig_fields = source->getFieldInfos();
    int new_field = 0;
    int cur_field = 0, end_field = orig_fields.size();
    vector< pair<int,DateCode> >::iterator it = cols_codes.begin(),
        end = cols_codes.end();

    for ( ; cur_field < end_field ; ++cur_field, ++new_field) {
        // We've got a date field
        if (it != end && it->first == cur_field) {
            switch(it->second) {
            case Date:
                this->declareField(new_field, "Date", VMField::Date);
                break;
            case DateTime:
                this->declareField(new_field, "DateTime", VMField::Date);
                break;
            }
            cur_field += dateCodeWidth(it->second)-1;
            ++it;
        }
        else {
            this->declareField(new_field, orig_fields[cur_field].name,
                               orig_fields[cur_field].fieldtype);
        }
    }
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
