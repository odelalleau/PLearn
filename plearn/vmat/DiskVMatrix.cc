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

#include "DiskVMatrix.h"
#include <errno.h>
#include <errno.h>
#include <plearn/base/stringutils.h>
#include <plearn/io/pl_io.h>
#include <plearn/io/fileutils.h>

namespace PLearn {
using namespace std;

#if defined(WIN32) && !defined(__CYGWIN__)
#include <io.h>
#define unlink _unlink
#endif

/** DiskVMatrix **/
/* Format description

The directory in variable dirname, which should end in .dmat,
contains a file named 'indexfile' and data files of up to roughly
500Meg each named 0.data, 1.data, ...

The indexfile has a 4 byte header, of which currently only the first byte is checked.
'L' means new little-endian format
'B' means new big-endian format
An ascii code of 16 means old little-endian format

In the new formats, the last 3 bytes of the 4 byte header are ' '

All other raw-binary data (whether in the indexfile or the data files)
is written in the specified endianness (and swapping will be performed if
reading it on a machine with a different endianness).

Following the 4-byte header are two 4-byte ints: length and width of the matrix

Following are 5 bytes for each row of the matrix.
The first of those 5 bytes is an unsigned byte indicating the number (0-255) of
the data file in which to find the row.
The remaining 4 bytes are an unsigned int indicating the start position of the row in
that data file (as passed to fseek).

The row is encoded in the thus indicated data file at the indicated position,
using the new vector compression format (see new_read_compressed)
[ the old compression format pf binread_compressed is supported
for backward compatibility only. ]

*/

PLEARN_IMPLEMENT_OBJECT(
    DiskVMatrix,
    "VMatrix whose data are precomputed on disk (in a .dmat directory).",
    ""
);

DiskVMatrix::DiskVMatrix():
    indexf      (0),
    freshnewfile(false),
    old_format  (false),
    swap_endians(false),
    tolerance   (1e-6)
{
    writable = false;
}

DiskVMatrix::DiskVMatrix(const PPath& the_dirname, bool readwrite):
    inherited   (true),
    indexf      (0),
    freshnewfile(false),
    old_format  (false),
    swap_endians(false),
    dirname     (the_dirname),
    tolerance   (1e-6)
{
    writable = readwrite;
    dirname.removeTrailingSlash(); // For safety.
    build_();
}

DiskVMatrix::DiskVMatrix(const PPath& the_dirname, int the_width,
                         bool write_double_as_float):
    inherited   (0, the_width, true),
    indexf      (0),
    freshnewfile(true),
    old_format  (false),
    swap_endians(false),
    dirname     (the_dirname),
    tolerance   (1e-6)
{
    writable = true;
    dirname.removeTrailingSlash(); // For safety.
    build_();
}

void DiskVMatrix::build()
{
    inherited::build();
    build_();
}

void DiskVMatrix::build_()
{
    closeCurrentFiles(); // All file pointers will be re-created.
    if(writable && width_ > 0 && !isdir(dirname))
        freshnewfile= true;
    if(!freshnewfile)
    {
        if(!isdir(dirname))
            PLERROR("In DiskVMatrix constructor, directory %s could not be found",dirname.c_str());
        setMetaDataDir(dirname + ".metadata");
        updateMtime( dirname/"indexfile" );
        string omode;
        if(writable)
            omode = "r+b";
        else // read-only
            omode = "rb";

        string indexfname = dirname/"indexfile";
        indexf = fopen(indexfname.c_str(), omode.c_str());
        if(!indexf)
            PLERROR("In DiskVMatrix constructor, could not open file %s in specified mode", indexfname.c_str());

        unsigned char header[4];
        fread(header,1,4,indexf);
        if(header[0]=='L' || header[0]=='B')
        { // New format
            old_format = false;
            swap_endians = (header[0]!=byte_order());
        }
        else if(header[0]==16)
        { // Old format
            old_format = true;
            if(byte_order()!='L')
                PLERROR("Old format DiskVMatrix can only be read from a little-endian machine.\n"
                        "Convert it to a new format on a little-endian machine prior to attempt\n"
                        "using it from a big endian machine.\n");
            swap_endians = false;
        }
        else
        {
            PLERROR("Wrong header byte in index file %s: ascii code %d\n"
                    "(should be 'L' or 'B' or '...')\n", indexfname.c_str(), header[0]);
        }

        fread(&length_,sizeof(int),1,indexf);
        fread(&width_,sizeof(int),1,indexf);
        if(swap_endians)
        {
            endianswap(&length_);
            endianswap(&width_);
        }
        int k=0;
        string fname = dirname/tostring(k)+".data";
        while(isfile(fname))
        {
            FILE* f = fopen(fname.c_str(), omode.c_str());
            if(!f)
                PLERROR("In DiskVMatrix constructor, could not open file %s in specified mode", fname.c_str());
            dataf.append(f);
            fname = dirname/(tostring(++k)+".data");
        }
        // Stuff related to RowBufferedVMatrix, for consistency
        current_row_index = -1;
        current_row.resize(width_);
        other_row_index = -1;
        other_row.resize(width_);

        //resize the string mappings
        map_sr = TVec<map<string,real> >(width_);
        map_rs = TVec<map<real,string> >(width_);

        getFieldInfos();
        if (writable)
            fseek(indexf, 0, SEEK_END);
    }
    else
    {
        if(isdir(dirname))
            PLERROR("In DiskVMatrix constructor (with specified width), directory %s already exists",dirname.c_str());
        setMetaDataDir(dirname + ".metadata");
        updateMtime( dirname/"indexfile" );

        if(isfile(dirname)) // patch for running mkstemp (TmpFilenames)
            unlink(dirname.c_str());
        if(!force_mkdir(dirname)) // force directory creation
            PLERROR("In DiskVMatrix constructor (with specified width), could not create directory %s  Error was: %s",dirname.c_str(), strerror(errno));

        string indexfname = dirname/"indexfile";
        indexf = fopen(indexfname.c_str(),"w+b");

        char header[4];
        header[0] = byte_order();
        header[1] = ' ';
        header[2] = ' ';
        header[3] = ' ';
        fwrite(header,1,4,indexf);
        fwrite((char*)&length_,sizeof(int),1,indexf);
        fwrite((char*)&width_,sizeof(int),1,indexf);

        string fname = dirname/"0.data";
        FILE* f = fopen(fname.c_str(), "w+b");
        dataf.append(f);
    }
    freshnewfile=false;
    loadAllStringMappings();//make sure string mappings are loaded after width is set
}

void DiskVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "dirname", &DiskVMatrix::dirname, OptionBase::buildoption, "Directory name of the.dmat");
    declareOption(ol, "tolerance", &DiskVMatrix::tolerance, OptionBase::buildoption, "The absolute error tolerance for storing doubles as floats");
    inherited::declareOptions(ol);
}

///////////////////////
// closeCurrentFiles //
///////////////////////
void DiskVMatrix::closeCurrentFiles()
{
    for (int i = 0; i < dataf.length(); i++)
        if(dataf[i]) {
            fclose(dataf[i]);
            dataf[i] = 0;
        }

    dataf.resize(0);
    if (indexf) {
        fclose(indexf);
        indexf = 0;
    }
}

///////////////
// getNewRow //
///////////////
void DiskVMatrix::getNewRow(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>length())
        PLERROR("In DiskVMatrix::getNewRow, bad row number %d",i);
    if(v.length() != width())
        PLERROR("In DiskVMatrix::getNewRow, length of v (%d) does not match matrix width (%d)",v.length(),width());
#endif

    unsigned char filenum;
    unsigned int position;
    fseek(indexf,3*sizeof(int) + i*(sizeof(unsigned char)+sizeof(unsigned int)), SEEK_SET);
    fread(&filenum,sizeof(unsigned char),1,indexf);
    fread(&position,sizeof(unsigned int),1,indexf);
    if(swap_endians)
        endianswap(&position);
    FILE* f = dataf[int(filenum)];
    PLASSERT( f );
    fseek(f,position,SEEK_SET);
    if(old_format)
        binread_compressed(f,v.data(),v.length());
    else
        new_read_compressed(f, v.data(), v.length(), swap_endians);
}

void DiskVMatrix::putRow(int i, Vec v)
{
    PLERROR("putRow cannot in general be correctly and efficiently implemented for a DiskVMatrix.\n"
            "Use appendRow if you wish to write more rows.");
}

void DiskVMatrix::appendRow(Vec v)
{
    if(!writable)
        PLERROR("In DiskVMatrix::appendRow cannot append row in read only mode, set readwrite parameter to true when calling the constructor");
    if(v.length() != width())
        PLERROR("In DiskVMatrix::appendRow, length of v (%d) does not match matrix width (%d)",v.length(),width());

    int filenum = dataf.size()-1;
    FILE* f = dataf[filenum];
    fseek(f,0,SEEK_END);
    unsigned int position = (unsigned int)ftell(f);
    if(position>500000000L)
    {
        fflush(f);
        filenum++;
        string filename = dirname / (tostring(filenum) + ".data");
        f = fopen(filename.c_str(), "w+b");
        dataf.append(f);
        position = 0;
    }
    if(old_format)
        binwrite_compressed(f,v.data(),v.length());
    else
        new_write_compressed(f, v.data(),v.length(), tolerance, swap_endians);

    fseek(indexf,0,SEEK_END);
    fputc(filenum,indexf);
    fwrite((char*)&position,sizeof(unsigned int),1,indexf);
    length_++;
    fseek(indexf,sizeof(int),SEEK_SET);
    int le = length_;
    if(swap_endians)
        endianswap(&le);
    fwrite(&le,sizeof(int),1,indexf);
}

///////////
// flush //
///////////
void DiskVMatrix::flush()
{
    int filenum = dataf.size()-1;
    FILE* f = dataf[filenum];
    fflush(f);
    fflush(indexf);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void DiskVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(dataf, copies);
    // Set pointers to zero, because we will open again the file (file pointers
    // should not be shared between copied objects).
    indexf = 0;
    dataf.fill(0);
    dataf.resize(0);
    build(); // Open again the dataset.
}


//////////////////
// ~DiskVMatrix //
//////////////////
DiskVMatrix::~DiskVMatrix()
{
    if (hasMetaDataDir())
        saveFieldInfos();
    DiskVMatrix::closeCurrentFiles();
}


#ifdef WIN32
#undef unlink
#endif

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
