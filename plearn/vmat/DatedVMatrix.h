// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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


/*! \file DatedVMatrix.h */

#ifndef DatedVMatrix_INC
#define DatedVMatrix_INC

#include "VMat.h"

namespace PLearn {
using namespace std;


/*!   A distribution to which some dates (or more generally time stamps) have
  been associated to each data row. Special methods then allow
  to take sub-distributions for particular intervals of dates.
  This is an abstract class: subclasses specify what time units
  are available and the semantics of date intervals.
*/
class DatedVMatrix: public VMatrix
{
    typedef VMatrix inherited;

public:
    // ******************
    // *  Constructors  *
    // ******************
    DatedVMatrix() {}; //!<  default constructor (for automatic deserialization)

    DatedVMatrix(int width, int length) : inherited(width,length) {}

/*!     this one calls one of subDistrRelative{Years,Months,Days} according
  to wether units=="years", "months", or "days" (or if the first letter
  matches, irrespective of upper/lower case distinctions)
*/
    virtual VMat subDistrRelativeDates(int first, int n, const string& units) = 0;

/*!     this one calls one of subDistrRelative{Years,Months,Days} according
  to wether units=="years", "months", or "days" (or if the first letter
  matches, irrespective of upper/lower case distinctions)
*/
    virtual VMat subDistrAbsoluteUnits(int year, int month, int day, int n_units,
                                       const string& units) = 0;

    //!  return "size" in the given units (e.g. interval in years, months, etc...)
    virtual int lengthInDates(const string& units) = 0;

    //!  return row position of example whose relative date is the first with
    //!  the given (relative) value, in the given time units
    virtual int positionOfRelativeDate(int first, const string& units) = 0;

    //!  return the number of real fields required to specify a date
    virtual int nDateFields() = 0;

    //!  copy the date fields for the relative positions starting at the
    //!  given row position for the given number of rows, into the given matrix
    virtual void copyDatesOfRows(int from_row, int n_rows, Mat& dates) = 0;

    // added by Julien Keable :
    // returns vector of row at indice 'row'
    // and associated date trough year, month and day
    virtual Vec copyRowDataAndDate(int row, int &year, int &month, int &day)=0;
    virtual void copyDateOfRow(int row, int &year, int &month, int &day)=0;

    PLEARN_DECLARE_ABSTRACT_OBJECT(DatedVMatrix);

    static void declareOptions(OptionList &ol);
    virtual void build();
private:
    void build_();
};

DECLARE_OBJECT_PTR(DatedVMatrix);

} // end of namespace PLearn
#endif


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
