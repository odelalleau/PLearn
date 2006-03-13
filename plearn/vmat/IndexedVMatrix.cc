// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Pascal Vincent, Olivier Delalleau
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

#include "IndexedVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(IndexedVMatrix, "ONE LINE DESCR",
                        "VMat class that sees its source as a collection of\n"
                        "triplets (row, column, value). Thus it is a N x 3"
                        " matrix,\n"
                        "with N = the number of elements in source.\n");


IndexedVMatrix::IndexedVMatrix(bool call_build_)
    : inherited(call_build_),
      need_fix_mappings(false)
{
    if( call_build_ )
        build_();
}

IndexedVMatrix::IndexedVMatrix(VMat the_source, bool the_fully_check_mappings,
                               bool call_build_)
    : inherited(the_source, call_build_),
      need_fix_mappings(false),
      fully_check_mappings(the_fully_check_mappings)
{
    if( call_build_ )
        build_();
}


void IndexedVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "m", &IndexedVMatrix::source,
                  (OptionBase::learntoption | OptionBase::nosave),
                  "DEPRECATED - use 'source' instead.");

    declareOption(ol, "fully_check_mappings",
                  &IndexedVMatrix::fully_check_mappings,
                  OptionBase::buildoption,
                  "If set to 1, then columns for which there is a"
                  " string <-> real mapping\n"
                  "will be examined, to ensure that no numerical data in a\n"
                  "column conflicts with a mapping in another column.\n");

    inherited::declareOptions(ol);
}

void IndexedVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(fixed_mappings, copies);
}

void IndexedVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void IndexedVMatrix::build_()
{
    width_ = 3;
    length_ = source->length() * source->width();

    ensureMappingsConsistency();

    if( fully_check_mappings )
        fullyCheckMappings();

    if( need_fix_mappings && !fully_check_mappings )
        PLWARNING( "In IndexedVMatrix::build_ - Mappings need to be fixed,\n"
                   "but you did not set 'fully_check_mappings' to true,\n"
                   "this might be dangerous.\n" );

    TVec<string> fieldnames(3);
    fieldnames[0] = "row";
    fieldnames[1] = "column";
    fieldnames[2] = "value";
    declareFieldNames( fieldnames );

    setMetaInfoFromSource();
}

///////////////
// getNewRow //
///////////////
void IndexedVMatrix::getNewRow(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length())
        PLERROR("In IndexedVMatrix::getNewRow OUT OF BOUNDS");
    if(v.length() != width())
        PLERROR("In IndexedVMatrix::getNewRow v.length() must be equal to 3");
#endif

    int w = source->width();
    int i_ = i / w; // the value of the first column at row i
    int j_ = i % w; // the value of the second column at row i
    real val = source->get(i_,j_);

    if( need_fix_mappings && fixed_mappings[j_].size()>0 && !is_missing(val) )
    {
        map<real, real>::iterator it = fixed_mappings[j_].find(val);
        if( it != fixed_mappings[j_].end() )
        {
            // We need to modify this value.
            val = it->second;
        }
    }

    v[0] = i_;
    v[1] = j_;
    v[2] = val;
}

/* implemented in RowBufferedVMatrix
/////////
// get //
/////////
real IndexedVMatrix::get(int i, int j) const {
    int w = source->width();
    int i_ = i / w; // the value of the first column at row i
    int j_ = i % w; // the value of the second column at row i
    switch (j) {
    case 0:
        return i_;
    case 1:
        return j_;
    case 2:
        return source->get(i_, j_);
    default:
        PLERROR("In IndexedVMatrix::get IndexedVMatrix has only 3 columns\n");
        return 0;
    }
}
*/

/////////
// put //
/////////
void IndexedVMatrix::put(int i, int j, real value) {
    if (j != 2) {
        PLERROR("In IndexedVMatrix::put You can only modify the third column\n");
    }
    int w = source->width();
    int i_ = i / w; // the value of the first column at row i
    int j_ = i % w; // the value of the second column at row i
    source->put(i_, j_, value);
}

///////////////////////////////
// ensureMappingsConsistency //
///////////////////////////////
void IndexedVMatrix::ensureMappingsConsistency()
{
    // Make sure the string mappings are consistent.
    // For this, we start from the mapping of the first column, and add
    // missing mappings obtained from the other columns. If another
    // column has a different mapping for the same string, we remember
    // it in order to fix the output when a getxxxx() method is called.
    need_fix_mappings = false;
    setStringMapping(2, source->getStringToRealMapping(0));
    map<string, real>* first_map = &map_sr[2];
    map<string, real> other_map;
    map<string, real>::iterator it, find_map;

    // Find the maximum mapping value for first column.
    real max = -REAL_MAX;
    for( it = first_map->begin() ; it != first_map->end() ; it++ )
        if( it->second > max )
            max = it->second;

    for( int i = 1 ; i < source->width() ; i++ )
    {
        other_map = source->getStringToRealMapping(i);
        for( it = other_map.begin() ; it != other_map.end() ; it++ )
        {
            find_map = first_map->find( it->first );
            if( find_map != first_map->end() )
            {
                // The string mapped in column 'i' is also mapped in our first
                // mapping.
                if( !fast_exact_is_equal(find_map->second, it->second) )
                {
                    // But the same string is not mapped to the same value.
                    // This needs to be fixed.
                    need_fix_mappings = true;
                    fixed_mappings.resize( source->width() );
                    fixed_mappings[i][it->second] = find_map->second;
                }
            }
            else
            {
                // The string mapped in VMat 'i' is not mapped in our
                // current mapping. We need to add this mapping.
                // But we must make sure there is no existing mapping to
                // the same real number.
                real new_map_val = it->second;
                if( getValString(2, it->second) != "" )
                {
                    // There is already a mapping to this real number.
                    need_fix_mappings = true;
                    fixed_mappings.resize( source->width() );
                    // We pick for the number the maximum of the current mapped
                    // numbers, +1.
                    max++;
                    fixed_mappings[i][it->second] = max;
                    new_map_val = max;
                }
                else
                {
                    // There is no mapping to this real number, it is thus ok
                    // to add it.
                    if( new_map_val > max )
                        max = new_map_val;
                }
                addStringMapping(2, it->first, new_map_val);
            }
        }
    }
}

////////////////////////
// fullyCheckMappings //
////////////////////////
void IndexedVMatrix::fullyCheckMappings()
{
    if( map_sr[2].size() == 0 )
        return;

    Vec row( source->width() );
    for( int i = 0 ; i < source->length() ; i++ )
    {
        source->getRow(i, row);
        for( int j = 0 ; j < source->width() ; j++ )
        {
            if( !is_missing(row[j]) )
            {
                // Note that if the value is missing, we should not
                // look for a mapping from this value, because it would
                // find it even if it does not exist (see the STL map
                // documentation to understand why this happens).
                if( source->getValString(j, row[j]) == "" )
                {
                    // It is a numerical value for the source's j-th column
                    if( map_rs[2].find(row[j]) != map_rs[2].end() )
                    {
                        // And, unfortunately, we have used this
                        // numerical value in the column (2) string
                        // mapping. It could be fixed, but this would be
                        // pretty annoying, thus we just raise an error.
                        PLERROR("In IndexedVMatrix::fullyCheckMappings - In"
                                " column %d (%s) of source, the row %d\n"
                                "contains a numerical value (%f) that is used"
                                " in a string mapping (mapped to %s).\n",
                                j, source->fieldName(j).c_str(), i,
                                row[j], map_rs[2][ row[j] ].c_str() );
                    }
                }
            }
        }
    }
}


} // end of namespcae PLearn


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
