
// -*- C++ -*-

// JulianizeVMatrix.h
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

/*! \file JulianizeVMatrix.h */
#ifndef JulianizeVMatrix_INC
#define JulianizeVMatrix_INC

#include <utility>
#include <vector>
#include <algorithm>

#include "SourceVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;

/*! JulianizeVMatrix
 *
 * The purpose of a JulianizeVMatrix is to convert some columns of a
 * source VMatrix into a JulianDayNumber (represented as a double).
 * This VMatrix can convert any triplet of (YYYY,MM,DD) or sextuplet
 * (YYY,MM,DD,HH,MM,SS) into a single double, whose integer part is the JDN
 * and the fractional part codes the HH:MM:SS as a day fraction.  Columns
 * anywhere in the VMat can thereby be sepcified.
 *
 * A class invariant here is that the member variable cols_codes (which
 * stores pairs of the starting columns for dates to convert along with the
 * date code for carrying out the conversion) is kept SORTED in order of
 * increasing column.
 */
class JulianizeVMatrix : public SourceVMatrix
{
    typedef SourceVMatrix inherited;

public:
    //! This specifies the how the dates are coded in the source VMatrix;
    //! for now only two formats are allowed.
    enum DateCode {
        Date = 0,                                //!< (YYYY, MM, DD)
        DateTime = 1                             //!< (YYYY, MM, DD, HH, MM, SS)
    };

    //! Return the number of columns taken by each date code
    static int dateCodeWidth(DateCode dc) {
        switch(dc) {
        case Date: return 3;
        case DateTime: return 6;
        }
        PLERROR("JulianizeVMatrix::dateCodeWidth: unknown date code");
        return 0;
    }

protected:
// DEPRECATED - use "source"    VMat underlying_;         //!< underlying vmat
    vector< pair<int,DateCode> > cols_codes;  //!< all columns/date codes
    mutable Vec source_row;                   //!< buffer for source row

public:


    // ************************
    // * public build options *
    // ************************

    // ### declare public option fields (such as build options) here
    // ...

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor, the implementation in the .cc
    //! initializes all fields to reasonable default values.
    JulianizeVMatrix(bool call_build_ = false);

    //! Simple constructor: takes as input only the date code and the starting
    //! column for a single date.  Starting columns are zero-based.
    JulianizeVMatrix(VMat the_source,
                     DateCode date_code = Date,
                     int starting_column = 0,
                     bool call_build_ = false);

    // ******************
    // * Object methods *
    // ******************

private:
    //! This does the actual building.
    void build_();

protected:
    //! Declares this class' options.  No options are currently supported
    static void declareOptions(OptionList& ol);

    //!  Implement the base class abstract member function
    virtual void getNewRow(int i, const Vec& v) const;

public:

    // simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(JulianizeVMatrix);

protected:
    // Return the new number of columns
    static int newWidth(VMat the_source, DateCode dc) {
        return the_source->width() - dateCodeWidth(dc) + 1;
    }

    // Set the VMFields in this VMatrix from the source field names.  All
    // "date" fields are named "Date", and "date-time" are named "DateTime".
    void setVMFields();
};

DECLARE_OBJECT_PTR(JulianizeVMatrix);

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
