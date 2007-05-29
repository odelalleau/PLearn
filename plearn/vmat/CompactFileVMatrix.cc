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
    "distinct semantics). Each field group has a type (i = int, o = onehot) \n"
    "a length and a maximal value (the values range from 0 to that value). \n"
    "Each field in a field group is typically encoded on the minimal amount \n"
    "of bits required to hold it (it is possible to specify more). Values \n"
    "are converted to floats and normalized by max, unless they are \n"
    "onehot-encoded."
);

////////////////////////
// CompactFileVMatrix //
////////////////////////
CompactFileVMatrix::CompactFileVMatrix():
    inherited       (true),
    active_list_    (Vec()),
    in_ram_         (false),
    f               (0),
    info_           (NULL),
    contents        (NULL)
{
    writable=false;
}

CompactFileVMatrix::CompactFileVMatrix(const PPath& filename):
    inherited       (true),
    filename_       (filename.absolute()),
    active_list_    (Vec()),
    in_ram_         (false),
    f               (0),
    info_           (NULL),
    contents        (NULL)
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

    char header[DATAFILE_HEADERLENGTH];

    if (!isfile(filename_))
    {
        PLERROR("In CompactFileVMatrix::build_ - File does not exist (%s)", filename_.c_str());
    }
    else
    {
#ifdef USE_NSPR_FILE
        f = PR_Open(filename_.c_str(), PR_RDONLY, 0666);
#else
        f = fopen(filename_.c_str(), "rb");
#endif

        if (! f)
            PLERROR("CompactFileVMatrix::build: could not open file %s", filename_.c_str());

#ifdef USE_NSPR_FILE
        PR_Read(f,header,DATAFILE_HEADERLENGTH);
#else
        fread(header,DATAFILE_HEADERLENGTH,1,f);
#endif
        if(header[DATAFILE_HEADERLENGTH-1]!='\n')
            PLERROR("In CompactFileVMatrix constructor, wrong header for cmat format.");
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

        // Here we painfully read the header.
        GroupInfoNode** slot = &info_;
        while (1) {
            char type = header[i];
            if (type == '\n' || type == ' ')
                break;
            if (!type || !(type == 'o' || type == 'i'))
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
            *slot = new GroupInfoNode(type, length, max, bits_per_value, NULL);
            if (header[i] == '\n')
                break;
            i++;

            slot = &((*slot)->next);
        }

        // If there is a non-empty list of active groups, activate only those.
        if (active_list_.length()) {
            for (int i=0; i<active_list_.length(); i++) {
                int next_active = (int)(active_list_[i]);
                if (i && active_list_[i-1] >= next_active)
                    PLERROR("In CompactFileVMatrix - Please give the active fields list in ascending order.");
                GroupInfoNode* info;
                for (info = info_; info && next_active; info = info->next)
                    next_active--;
                if (!info)
                    PLERROR("In CompactFileVMatrix - Bad list of active groups: group index out of bounds.");
                info->active = true;
            }
        }
        // else, activate all of them
        else { 
            for (GroupInfoNode* info = info_; info; info = info->next)
                info->active = true;
        }

        int width = 0;
        int file_width = 0; // real width of an entry in the file
        for (GroupInfoNode* info = info_; info; info = info->next) {
            info->compact_length = (info->length * info->bits_per_value + 7) / 8;
            if (info->active) {
                if (info->type == 'o') { // one-hot encoding
                    width += info->length * (info->max + 1);
                }
                else if (info->type == 'i') { // int (if max=1) or float encoding
                    width += info->length;
                }
            }
            file_width += info->compact_length;
        }

        this->width_ = width + 1;
        this->compact_width_ = file_width + 1;

        // If the option in_ram is active, we load everything in RAM beforehand
        if (in_ram_) {
            int sz = length_ * compact_width_;
            contents = (char*)malloc(sz);
            if (!contents && errno == ENOMEM)
                PLERROR("In CompactFileVMatrix - could not allocate %d bytes. Consider putting the in_ram option to False.", sz);
#ifdef USE_NSPR_FILE
            PR_Read(f,contents, sz);
#else
            fread(contents, sz, 1, f);
#endif
        }
    }

    setMtime(mtime(filename_));
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
    declareOption(ol, "in_ram", &CompactFileVMatrix::in_ram_, OptionBase::buildoption, "if set to true, all the file will be loaded in main memory for faster access");

    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void CompactFileVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    PLERROR("In CompactFileVMatrix::makeDeepCopyFromShallowCopy - not implemented.");
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
    GroupInfoNode* temp = NULL;
    GroupInfoNode* info = info_;
    while (info) {
        temp = info->next;
        delete info;
        info = temp;
    }
    if (contents) {
        free(contents);
        contents = NULL;
    }
}


//////////////////
// nFieldGroups //
//////////////////

int CompactFileVMatrix::nGroups() const
{
    int i = 0;
    for (GroupInfoNode* info = info_; info; info = info->next)
        i++;
    return i;
}


//////////////
// getGroup //
//////////////

GroupInfoNode* CompactFileVMatrix::getGroup(int group) const
{
    GroupInfoNode* info;
    if (group < 0) {
        PLERROR("In CompactFileVMatrix::getGroup - group index out of bounds.");
    }
    for (info = info_; info && group; info = info->next)
        group--;
    if (!info) {
        PLERROR("In CompactFileVMatrix::getGroup - group index out of bounds.");
    }
    return info;
}


///////////////////
// groupEncoding //
///////////////////

char CompactFileVMatrix::groupEncoding(int group) const
{
    return getGroup(group)->type;
}



//////////////////
// groupNFields //
//////////////////

int CompactFileVMatrix::groupNFields(int group) const
{
    return getGroup(group)->length;
}



/////////////////
// groupLength //
/////////////////

int CompactFileVMatrix::groupLength(int group) const
{
    GroupInfoNode* info = getGroup(group);
    if (info->type == 'i')
        return info->length;
    else
        return info->length * info->max;
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

    if (in_ram_) {
        buffer = (unsigned char*)(contents + (i*compact_width_));
    }
    else {
        buffer = (unsigned char*)malloc(compact_width_);

#ifdef USE_NSPR_FILE
        moveto(i);
        PR_Read(f, buffer, compact_width_);
#else
        fseek(f, DATAFILE_HEADERLENGTH+(i*compact_width_), SEEK_SET);
        fread(buffer, compact_width_, 1, f);
#endif
    }

    int current_b = -1;
    int current_v = 0;
    int value_b = 0;

    for (GroupInfoNode* info = info_; info; info = info->next) {
        if (!info->active) {
            current_b += info->compact_length;
            continue;
        }
        int nbits = info->bits_per_value;
        int max = info->max;
        int length = info->length;
        char type = info->type;

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
                    v[current_v] = (float)val / max;
                else if (type == 'o' && val <= max)
                    v[current_v + val] = 1;
            }
            else { // the value is whole in the current byte
                int val = (value_b & ((1<<nbits) - 1));
                if (type == 'i')
                    v[current_v] = (float)val / max;
                else if (type == 'o' && val <= max) {
                    v[current_v + val] = 1;
                }
                value_b >>= nbits;
                remainder -= nbits;
            }
            if (type == 'i')
                current_v++;
            else if (type == 'o')
                current_v += max + 1;
        }
    }

    v[v.length() - 1] = buffer[compact_width_ - 1]; // class

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
    offset += DATAFILE_HEADERLENGTH;
    PR_Seek64(f, offset, PR_SEEK_SET);
#else
    fseek(f, DATAFILE_HEADERLENGTH+(i*compact_width_+j), SEEK_SET);
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
