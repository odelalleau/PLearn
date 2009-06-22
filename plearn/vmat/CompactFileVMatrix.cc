// -*- C++ -*-

// CompactFileVMatrix.h
//
// Copyright (C) 2007 Olivier Breuleux
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


#include "CompactFileVMatrix.h"
#include <plearn/io/fileutils.h>
#include <plearn/io/pl_NSPR_io.h>

#include <iostream>
#include <errno.h>

namespace PLearn {
using namespace std;


/** CompactFileVMatrix **/

PLEARN_IMPLEMENT_OBJECT(
    CompactFileVMatrix,
    "Compact VMatrix where fields are bitfields.",
    "VMatrix whose data is stored on disk in a compacted '.cmat' file. \n"
    "The file contains a certain amount of field groups (with supposed \n"
    "distinct semantics). Each field group has an encoding type, \n"
    "a length and a maximal value (the values range from 0 to that value). \n"
    "Each field in a field group is typically encoded on the minimal amount \n"
    "of bits required to hold it (it is possible to specify more). Values \n"
    "are converted to floats and normalized by max, unless they are \n"
    "onehot-encoded. \n \n"
    "Encoding types: 'i' = float between 0 and 1, 'o' = onehot, 'u' = unsigned integer"
);

////////////////////////
// CompactFileVMatrix //
////////////////////////
CompactFileVMatrix::CompactFileVMatrix():
    inherited       (true),
    in_ram_         (false),
    f               (0)
{
    writable=false;
}

CompactFileVMatrix::CompactFileVMatrix(const PPath& filename):
    inherited       (true),
    filename_       (filename.absolute()),
    in_ram_         (false),
    f               (0)
{
    build_();
}


///////////
// build //
///////////
void CompactFileVMatrix::build()
{
    inherited::build();
    build_();
}



////////////
// build_ //
////////////
void CompactFileVMatrix::build_()
{
    // cleanup (close file, free structures, etc.)
    cleanup();

    // If no file is given, we can stop here.
    if (filename_.isEmpty())
        return;

    char header[COMPACTFILEVMATRIX_HEADERLENGTH_MAX + 1];
    header[COMPACTFILEVMATRIX_HEADERLENGTH_MAX] = '\n';

    openCurrentFile();


#ifdef USE_NSPR_FILE
    PR_Read(f,header,COMPACTFILEVMATRIX_HEADERLENGTH_MAX);
#else
    fread(header,COMPACTFILEVMATRIX_HEADERLENGTH_MAX,1,f);
#endif

    int file_length;

    file_length = atoi(header);

    if (!file_length) {
        PLERROR("In CompactFileVMatrix::build_ - ill-formed header for the cmat file (0)");
    }
    this->length_ = file_length;

    int i = 0;
    while (header[i] >= '0' && header[i] <= '9')
        i++;
    i++;

    info = TVec<GroupInfo>();

    // Here we painfully read the header.
    while (1) {
        char type = header[i];
        if (type == '\n' || type == ' ')
            break;
        if (!type || !(type == 'o' || type == 'i' || type == 'u'))
            PLERROR("In CompactFileVMatrix::build_ - ill-formed header for the cmat file (bad type: %c)", type);
        i++;

        if (header[i] != ':')
            PLERROR("In CompactFileVMatrix::build_ - ill-formed header for the cmat file (2)");
        i++;

        int length = atoi(&(header[i]));
        if (!length)
            PLERROR("In CompactFileVMatrix::build_ - ill-formed header for the cmat file (3)");
        while (header[i] >= '0' && header[i] <= '9')
            i++;

        if (header[i] != ':')
            PLERROR("In CompactFileVMatrix::build_ - ill-formed header for the cmat file (4)");
        i++;

        int max = atoi(&(header[i]));
        if (!max)
            PLERROR("In CompactFileVMatrix::build_ - ill-formed header for the cmat file (5)");
        while (header[i] >= '0' && header[i] <= '9')
            i++;

        if (header[i] != ':')
            PLERROR("In CompactFileVMatrix::build_ - ill-formed header for the cmat file (6)");
        i++;

        int bits_per_value = atoi(&(header[i]));
        if (!bits_per_value)
            PLERROR("In CompactFileVMatrix::build_ - ill-formed header for the cmat file (7)");
        while (header[i] >= '0' && header[i] <= '9')
            i++;

        if (bits_per_value > 8)
            PLERROR("In CompactFileVMatrix::build_ - each field must span at most 8 bits");

        if (header[i] != ' ' && header[i] != '\n')
            PLERROR("In CompactFileVMatrix::build_ - ill-formed header for the cmat file");

        info.append(GroupInfo(type, length, max, bits_per_value));

        if (header[i] == '\n') {
            i++;
            break;
        }
        i++;
    }

    header_length = i;


    // If there is a non-empty list of active groups, activate only those.
    if (active_list_.length()) {
        for (int i=0; i<active_list_.length(); i++) {
            int next_active = (int)(active_list_[i]);
            if (i && active_list_[i-1] >= next_active)
                PLERROR("In CompactFileVMatrix - Please give the active fields list in ascending order.");
            if (i < 0 || i >= info.length())
                PLERROR("In CompactFileVMatrix - Bad list of active groups: group index out of bounds.");
            info[next_active].active = true;
        }
    }
    // else, activate all of them
    else {
        for(int i=0; i<info.length(); i++)
            info[i].active = true;
    }

    int width = 0;
    int file_width = 0; // real width of an entry in the file
    for (int i=0; i<info.length(); i++) {
        info[i].compact_length = (info[i].length * info[i].bits_per_value + 7) / 8;
        if (info[i].active) {
            if (info[i].type == 'o') { // one-hot encoding
                width += info[i].length * (info[i].max + 1);
            }
            else if (info[i].type == 'i' || info[i].type == 'u') { // int (if max=1) or float encoding
                width += info[i].length;
            }
        }
        file_width += info[i].compact_length;
    }

    this->width_ = width; // + 1;
    this->compact_width_ = file_width; // + 1;

    // If the option in_ram is specified, we allocate a buffer
    if (in_ram_) {
        int sz = length_ * compact_width_;
        cache = string(sz, '\0');
        cache_index = TVec<unsigned char>((length_ + 7) / 8);
    }

    updateMtime(filename_);
}



/////////////////////
// openCurrentFile //
/////////////////////
void CompactFileVMatrix::openCurrentFile()
{
    if (f) {
        PLERROR("In CompactFileVMatrix::openCurrentFile - File already open (%s)", filename_.c_str());
    }
    else if (!isfile(filename_)) {
        PLERROR("In CompactFileVMatrix::openCurrentFile - File does not exist (%s)", filename_.c_str());
    }
    else {
#ifdef USE_NSPR_FILE
        f = PR_Open(filename_.c_str(), PR_RDONLY, 0666);
#else
        f = fopen(filename_.c_str(), "rb");
#endif
        if (! f)
            PLERROR("CompactFileVMatrix::openCurrentFile: could not open file %s", filename_.c_str());
    }
}



//////////////////////
// closeCurrentFile //
//////////////////////
void CompactFileVMatrix::closeCurrentFile()
{
    if (f)
    {
#ifdef USE_NSPR_FILE
        PR_Close(f);
#else
        fclose(f);
#endif
        f = 0;
    }
}

////////////////////
// declareOptions //
////////////////////
void CompactFileVMatrix::declareOptions(OptionList & ol)
{
    declareOption(ol, "filename", &CompactFileVMatrix::filename_, OptionBase::buildoption, "Filename of the matrix");
    declareOption(ol, "active_list", &CompactFileVMatrix::active_list_, OptionBase::buildoption, "comma-separated list of active field groups");
    declareOption(ol, "cache", &CompactFileVMatrix::in_ram_, OptionBase::buildoption, "if set to true, all rows read from the file will be cached");

    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void CompactFileVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    TVec<GroupInfo> temp = info;
    info = TVec<GroupInfo>();
    for (int i=0; i<temp.length(); i++) {
        info.append(temp[i]);
    }

    deepCopyField(cache_index, copies);

    // We don't want to share the file descriptor
    f = 0;
    openCurrentFile();
}

/////////////////////////
// ~CompactFileVMatrix //
/////////////////////////
CompactFileVMatrix::~CompactFileVMatrix()
{
    CompactFileVMatrix::cleanup();
}

//////////////////
// nFieldGroups //
//////////////////

void CompactFileVMatrix::cleanup() {
    CompactFileVMatrix::closeCurrentFile();
    cache = string();
    cache_index = TVec<unsigned char>();
}


//////////////////
// nFieldGroups //
//////////////////

int CompactFileVMatrix::nGroups() const
{
    return info.length();
}


//////////////
// getGroup //
//////////////

GroupInfo& CompactFileVMatrix::getGroup(int group) const
{
    if (group < 0 || group >= info.length()) {
        PLERROR("In CompactFileVMatrix::getGroup - group index out of bounds.");
    }
    return info[group];
}


///////////////////
// groupEncoding //
///////////////////

char CompactFileVMatrix::groupEncoding(int group) const
{
    return getGroup(group).type;
}



//////////////////
// groupNFields //
//////////////////

int CompactFileVMatrix::groupNFields(int group) const
{
    return getGroup(group).length;
}



//////////////////
// groupNValues //
//////////////////

int CompactFileVMatrix::groupNValues(int group) const
{
    return getGroup(group).max;
}



/////////////////
// groupLength //
/////////////////

int CompactFileVMatrix::groupLength(int group) const
{
    GroupInfo& info = getGroup(group);
    if (info.type == 'i')
        return info.length;
    else
        return info.length * info.max;
}



///////////////
// getNewRow //
///////////////
void CompactFileVMatrix::getNewRow(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(v.length()!=width_)
        PLERROR("In CompactFileVMatrix::getNewRow - length of v (%d) differs from matrix width (%d)",v.length(),width_);
#endif

    unsigned char* buffer;
    bool m_flag=false; // to check if we allocated the buffer using malloc

    if (in_ram_ && (cache_index[i/8] & (1 << (i%8)))) {
        buffer = (unsigned char*)(cache.data() + (i*compact_width_));
    }
    else {
        buffer = (unsigned char*)malloc(compact_width_);
        m_flag = true;

#ifdef USE_NSPR_FILE
        moveto(i);
        PR_Read(f, buffer, compact_width_);
#else
        fseek(f, COMPACTFILEVMATRIX_HEADERLENGTH_MAX+(i*compact_width_), SEEK_SET);
        fread(buffer, compact_width_, 1, f);
#endif
        if (in_ram_) {
            cache_index[i/8] |= 1 << (i%8);
            for (int j=0; j<compact_width_; j++)
                cache[(i*compact_width_) + j] = buffer[j];
        }
    }

    int current_b = -1;
    int current_v = 0;
    int value_b = 0;

    for (int g=0; g<info.length(); g++) {
        if (!info[g].active) {
            current_b += info[g].compact_length;
            continue;
        }
        int nbits = info[g].bits_per_value;
        int max = info[g].max;
        int length = info[g].length;
        char type = info[g].type;

        if (type == 'o') {
            v.subVec(current_v, length * (max + 1)).clear();
        }

        int remainder = 0;
        for (int i = 0; i < length; i++) {
            if (!remainder) { // we're done with the current byte (this block is entered in the first iteration)
                remainder = 8;
                current_b++;
                PLASSERT(current_b < compact_width_); // just in case
                value_b = buffer[current_b];
            }
            if (remainder < nbits) { // the value overlaps two bytes
                int val = value_b;
                int overflow = nbits - remainder;
                value_b = buffer[++current_b];
                val |= (value_b & (1<<overflow)-1) << remainder;
                value_b >>= overflow;
                remainder = 8 - nbits + remainder;
                if (type == 'i')
                    v[current_v] = (real)val / max;
                else if (type == 'u')
                    v[current_v] = val;
                else if (type == 'o' && val <= max)
                    v[current_v + val] = 1;
            }
            else { // the value is whole in the current byte
                int val = (value_b & ((1<<nbits) - 1));
                if (type == 'i')
                    v[current_v] = (real)val / max;
                else if (type == 'u')
                    v[current_v] = val;
                else if (type == 'o' && val <= max) {
                    v[current_v + val] = 1;
                }
                value_b >>= nbits;
                remainder -= nbits;
            }
            if (type == 'i' || type == 'u')
                current_v++;
            else if (type == 'o')
                current_v += max + 1;
        }
    }

   if (m_flag)
        free(buffer);
}

///////////////
// putSubRow //
///////////////
void CompactFileVMatrix::putSubRow(int i, int j, Vec v)
{
    PLERROR("In CompactFileVMatrix::putSubRow - not implemented.");
}

void CompactFileVMatrix::moveto(int i, int j) const
{
#ifdef USE_NSPR_FILE
    PRInt64 offset = i;
    offset *= compact_width_;
    offset += j;
    offset += header_length;
    PR_Seek64(f, offset, PR_SEEK_SET);
#else
    fseek(f, header_length+(i*compact_width_+j), SEEK_SET);
#endif
}

/////////
// put //
/////////
void CompactFileVMatrix::put(int i, int j, real value)
{
    PLERROR("In CompactFileVMatrix::put - not implemented.");
}

///////////////
// appendRow //
///////////////
void CompactFileVMatrix::appendRow(Vec v)
{
    PLERROR("In CompactFileVMatrix::put - not implemented.");
}

///////////
// flush //
///////////
void CompactFileVMatrix::flush()
{
    if(f)
    {
#ifdef USE_NSPR_FILE
    PR_Sync(f);
#else
    fflush(f);
#endif
    }
}

//////////////////
// updateHeader //
//////////////////
void CompactFileVMatrix::updateHeader() {
    PLERROR("In CompactFileVMatrix::put - not implemented.");
}

VMatrixExtensionRegistrar* CompactFileVMatrix::extension_registrar =
    new VMatrixExtensionRegistrar(
        "cmat",
        &CompactFileVMatrix::instantiateFromPPath,
        "A compact file matrix");

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
