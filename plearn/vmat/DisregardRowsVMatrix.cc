// -*- C++ -*-

// DisregardRowsVMatrix.cc
//
// Copyright (C) 2005 Christian Dorion
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

// Authors: Christian Dorion

/*! \file DisregardRowsVMatrix.cc */

#define PL_LOG_MODULE_NAME "DisregardRowsVMatrix"

#include "DisregardRowsVMatrix.h"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DisregardRowsVMatrix,
    "A vmat that disregard rows containing any specified values.\n",
    "Typically, this vmat is used to select the rows of the source vmat which\n"
    "do not contain missing values. However, this behaviour can be changed by\n"
    "setting the 'disregard_missings' flag to false and by providing any\n"
    "real value list through 'disregard_values'.\n"
    "\n"
    "The default behavior of the class is to inspect all columns of the\n"
    "underlying vmat, but one may specify a subset of the source's fieldnames\n"
    "to restrict the inspection." );


//#####  Constructors  ########################################################

DisregardRowsVMatrix::DisregardRowsVMatrix()
    : inherited(),
      _disregard_missings(true),
      _maximum_length(-1)
{ }

DisregardRowsVMatrix::DisregardRowsVMatrix(VMat the_source)
    : inherited(),
      _disregard_missings(true),
      _maximum_length(-1)
{
    PLASSERT( the_source );
    source = the_source;
    build();
}


////////////////////
// declareOptions //
////////////////////
void
DisregardRowsVMatrix::
declareOptions(OptionList& ol)
{
    declareOption(
        ol, "inspected_fieldnames", &DisregardRowsVMatrix::_inspected_fieldnames,
        OptionBase::buildoption,
        "Field names of the source vmat for which a triggering value (see the\n"
        "disregard_values option) cause this vmat to neglect a row.\n"
        "\n"
        "If empty, all source's fieldnames are used.\n"
        "\n"
        "Default: []." );

    declareOption(
        ol, "disregard_missings", &DisregardRowsVMatrix::_disregard_missings,
        OptionBase::buildoption,
        "Should missing values cause a row to be neglected.\n"
        "\n"
        "Default: 1 (True)" );

    declareOption(
        ol, "disregard_values", &DisregardRowsVMatrix::_disregard_values,
        OptionBase::buildoption,
        "If any of these values is encountered in any column designated in\n"
        "inspected_fieldnames, the whole row is disregarded.\n"
        "\n"
        "Default: [ ]" );

    declareOption(
        ol, "maximum_length", &DisregardRowsVMatrix::_maximum_length,
        OptionBase::buildoption,
        "If positive, only the last 'maximum_length' rows kept from the source\n"
        "vmat will be considered, all other rows being disregarded.\n"
        "\n"
        "Default: -1. " );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void
DisregardRowsVMatrix::
build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void
DisregardRowsVMatrix::
build_()
{
    if ( !source )
        return;

    /* Option: inspected_fieldnames */

    // Default: All fields are inspected.
    if ( _inspected_fieldnames.isEmpty() )
    {
        _inspected_columns.resize( width() );
        for ( int c=0; c < width(); c++ )
            _inspected_columns[c] = c;
    }

    // Get the column indices of the fields to inspect.
    else
    {
        _inspected_columns.resize( _inspected_fieldnames.length() );
        for ( int f=0; f < _inspected_fieldnames.length(); f++ )
            _inspected_columns[f] =
                source->fieldIndex( _inspected_fieldnames[f] );
    }

    inferIndices();

    DBG_MODULE_LOG
        << "Out of " << source.length() << " rows, "
        << "kept " << indices.length() << " rows"
        << endl;

    // Calls back the inherited build now that the row indices are known
    inherited::build();
}

void
DisregardRowsVMatrix::
inferIndices( )
{
    if ( !source )
        return;

    indices.resize( 0, source.length() );
    for ( int r=0; r < source.length(); r++ )
    {
        bool disregard_row = false;
        for ( int inspected=0;
              inspected < _inspected_columns.length() && !disregard_row;
              ++inspected )
        {
            int c = _inspected_columns[ inspected ];
            if( (_disregard_missings && is_missing( source(r,c) ))
                || _disregard_values.contains( source(r,c) ) )
                disregard_row = true;
        }
        if ( !disregard_row )
            indices.append( r );
    }

    if ( _maximum_length > 0 && indices.length() > _maximum_length )
        indices =
            indices.subVec( indices.length()-_maximum_length, _maximum_length );
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void
DisregardRowsVMatrix::
makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField( _inspected_columns,    copies );

    deepCopyField( _inspected_fieldnames, copies );
    deepCopyField( _disregard_values,    copies );
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
