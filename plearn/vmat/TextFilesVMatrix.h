
// -*- C++ -*-

// TextFilesVMatrix.h
//
// Copyright (C) 2003-2004 ApSTAT Technologies Inc.
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

// Author: Pascal Vincent

/*! \file TextFilesVMatrix.h */
#ifndef TextFilesVMatrix_INC
#define TextFilesVMatrix_INC

#include <plearn/vmat/RowBufferedVMatrix.h>

namespace PLearn {
using namespace std;


/*! FORMAT DESCRIPTION

This type of vmatrix is described in a basename.txtmat file
and has an associated metadata/ directory

The metadata directory will contain the following files:
- a txtmat.idx binary index file (which will be automatically rebuilt if any of the raw text files is newer)
- a txtmat.idx.log file reporting problems encountered while building the .idx file

The txtmat.idx file is a binary file structured as follows
- 1 byte indicating endianness: 'L' or 'B'
- 4 byte int for length (number of data rows in the raw text file)
- (unsigned char fileno, int pos) indicating in which raw text file and at what position each row starts

*/

class TextFilesVMatrix: public RowBufferedVMatrix
{
protected:
    // *********************
    // * protected options *
    // *********************

    static char buf[];

    FILE* idxfile;
    TVec<FILE*> txtfiles;

    //! list of startpos : width that associate a column range in the resulting VMat to each initial field of the fieldspecs
    //! This is built by the setColumnNamesAndWidth method

public:

    //! indicates start position and size in result vector for each field.
    //! (as a text field may be transformed into more than one real)
    TVec< pair<int,int> > colrange;

    // ************************
    // * public build options *
    // ************************

    /// The path to the .metadata directory
    /// @deprecated Use metadadir of VMatrix class instead.
    PPath metadatapath; 

    //! A list of paths to raw text files containing the records
    TVec<PPath> txtfilenames;

    //! Delimiter to use to split the fields.  Common delimiters are:
    //! - '\t' : used for SAS files (the default)
    //! - ','  : used for CSV files
    //! - ';'  : used for a variant of CSV files
    string delimiter;

    //! The escate caractere where the delimiter is not considered.
    //! - '"' : used frequently
    string quote_delimiter;

    //! An (optional) list of integers, one for each of the txtfilenames
    //! indicating the number of header lines at the top of the file to be skipped.
    TVec<int> skipheader;

    //! Specification of field names and types
    TVec< pair<string, string> > fieldspec;

    //! If true, all mappings will be automatically computed at build time if
    //! they do not exist yet
    bool auto_build_map;

    //! If true, new strings for fields of type AUTO will automatically
    //! appended to the mapping (in the metadata/mappings/fieldname.map file)
    bool auto_extend_map;

    //! If true, standard vmatrix stringmap will be built from the txtmat
    //! specific stringmap
    bool build_vmatrix_stringmap;

    //! If true, will reorder the fieldspec in the order gived
    //! by the field names taken from txtfilenames
    bool reorder_fieldspec_from_headers;

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    TextFilesVMatrix();
    virtual ~TextFilesVMatrix();

    // ******************
    // * Object methods *
    // ******************

private:
    //! This does the actual building.
    // (Please implement in .cc)
    void build_();
    void setColumnNamesAndWidth();
    void getFileAndPos(int i, unsigned char& fileno, int& pos) const;
    void buildIdx();
    static void readAndCheckOptionName(PStream& in, const string& optionname);


protected:
    //! Declares this class' options
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);

    //! a possible string->real mapping for each field
    //! We keep our own mapping in addition to the standard VMatrix string mapping
    //! because we want to handle the case where different strings map to the same value.
    //! (Standard VMatrix string mapping concept is meant to possibly map several values to the same string,
    //! i.e. that's the other way round).
    mutable TVec< hash_map<string, real> > mapping;

    mutable TVec<FILE*> mapfiles;

    //! If no existing mapping has been loaded, will automatically create all mappings.
    void autoBuildMappings();

    void loadMappings();

    virtual real getPostalEncoding(const string& strval, bool display_warning = true) const;

    //! Return true iff 'ftype' is a valid type that does not need to be skipped.
    virtual bool isValidNonSkipFieldType(const string& ftype) const;

public:

    typedef RowBufferedVMatrix inherited;
    PLEARN_DECLARE_OBJECT(TextFilesVMatrix);

    //! returns the mapped value for field fieldnum corresponding to strval
    //! If there was no such mapping present, and auto_extend_map is true,
    //! then a new mapping is appended for that field (with value -1000-size_of_map).
    //! If not a PLERROR is thrown.
    real getMapping(int fieldnum, const string& strval) const;

    string getMapFilePath(int fieldnum) const
    { return getMetaDataDir()/"mappings"/fieldspec[fieldnum].first+".map"; }

    //! Returns the raw text rows
    string getTextRow(int i) const;

    //! Returns the number of text fields in each row
    //! (this may be different from the width() of the resulting
    //! VMatrix, as some fields may be skipped, and others
    //! may be processed into several columns.
    int nTextFields() const
    { return fieldspec.length(); }

    //! Split the passed string into fields given the delimiter
    TVec<string> splitIntoFields(const string& raw_row) const;

    //! Returns the split raw text rows
    TVec<string> getTextFields(int i) const;

    //! Returns the index in a split text row of the given named text field
    //! (this is the position of that named field in fieldspec )
    int getIndexOfTextField(const string& fieldname) const;

    //! Transform field-k value strval according to its fieldtype into one ore more reals
    //! and write those into dest (which should be of appropriate size: colrange[k].second)
    virtual void transformStringToValue(int k, string strval, Vec dest) const;

    virtual void getNewRow(int i, const Vec& v) const;

    // simply calls inherited::build() then build_()
    virtual void build();

    //! Builds standard VMatrix string mapping from this txtmat's specific mapping
    //! saveAllStringMappings() can then be called to save them
    //! (for later reference from another VMatrix for ex.)
    void buildVMatrixStringMapping();

    //! Will generate a counts/ directory in the metadatadir and record,
    //! for each field with a mapping, how many times a particular mapped string was encountered.
    void generateMapCounts();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

};

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
