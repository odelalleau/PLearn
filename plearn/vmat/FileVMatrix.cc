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

#include "FileVMatrix.h"
#include <plearn/io/fileutils.h>
#include <plearn/io/pl_NSPR_io.h>

namespace PLearn {
using namespace std;


/** FileVMatrix **/

PLEARN_IMPLEMENT_OBJECT(
    FileVMatrix,
    "VMatrix whose data is stored on disk in an uncompressed '.pmat' file.",
    ""
);

/////////////////
// FileVMatrix //
/////////////////
FileVMatrix::FileVMatrix():
    filename_       (""),
    f               (0),
    build_new_file  (false)
{
    writable=true;
    remove_when_done = track_ref = -1;
}

FileVMatrix::FileVMatrix(const PPath& filename, bool writable_):
    inherited       (true),
    filename_       (filename.absolute()),
    f               (0),
    build_new_file  (!isfile(filename))
{
    remove_when_done = track_ref = -1;
    writable = writable_;
    build_();
}

FileVMatrix::FileVMatrix(const PPath& filename, int the_length, int the_width,
                         bool force_float, bool call_build_):
    inherited       (the_length, the_width, true),
    filename_       (filename.absolute()),
    f               (0),
    force_float     (force_float),
    build_new_file  (true)
{
    remove_when_done = track_ref = -1;
    writable = true;
    if(call_build_)
        build_();
}

FileVMatrix::FileVMatrix(const PPath& filename, int the_length,
                         const TVec<string>& fieldnames):
    inherited       (the_length, fieldnames.length(), true),
    filename_       (filename.absolute()),
    f               (0),
    build_new_file  (true)
{
    remove_when_done = track_ref = -1;
    writable = true;
    build_();
    declareFieldNames(fieldnames);
    saveFieldInfos();
}

static int strlen(char* s) {
    int n=0;
    while (s[n]!=0)
        n++;
    return n;
}

///////////
// build //
///////////
void FileVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void FileVMatrix::build_()
{
    // Check for deprecated options.
    if (remove_when_done != -1) {
        PLDEPRECATED("In FileVMatrix::build_ - The option 'remove_when_done' "
                     "is now deprecated, please use the "
                     "TemporaryFileVMatrix class instead: this data file "
                     "may thus not be properly deleted");
        PLASSERT( remove_when_done == 0 || remove_when_done == 1 );
    }
    if (track_ref != -1) {
        PLDEPRECATED("In FileVMatrix::build_ - The option 'track_ref' "
                     "is now deprecated, please use the "
                     "TemporaryFileVMatrix class instead: this data file "
                     "may thus not be properly deleted");
        PLASSERT( track_ref == 0 || track_ref == 1 );
    }
    // Since we are going to re-create it, we can close the current f.
    closeCurrentFile();

    // If no file is given, we can stop here.
    if (filename_.isEmpty())
        return;

    char header[DATAFILE_HEADERLENGTH];
    char matorvec[20];
    char datatype[20];
    char endiantype[20];

    if (build_new_file || !isfile(filename_))
        force_mkdir_for_file(filename_);

    if (build_new_file || !isfile(filename_))
    {
        if (!writable)
            PLERROR("In FileVMatrix::build_ - You asked to create a new file (%s), but 'writable' is set to 0 !", filename_.c_str());

        openfile(filename_,"w+b", 
                 PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE, 0666);
        if (!f)
            PLERROR("In FileVMatrix constructor, could not open file %s",filename_.c_str());

#ifdef USEFLOAT
        file_is_float = true;
#endif
#ifdef USEDOUBLE
        file_is_float = false;
#endif
#ifdef LITTLEENDIAN
        file_is_bigendian = false;
#endif
#ifdef BIGENDIAN
        file_is_bigendian = true;
#endif
        if(force_float)
            file_is_float = true;

        updateHeader();

        if(length_ > 0 && width_ > 0)
        {
            // Ensure we can allocate enough space... if len>0
#ifdef USE_NSPR_FILE
            moveto(length_-1,width_-1);
            char c = '\0';
            PR_Write(f, &c, 1);
            PR_Sync(f);
#else
            if( fseek(f, DATAFILE_HEADERLENGTH+length_*width_*sizeof(real)-1, SEEK_SET) <0 )
            {
                perror("");
                PLERROR("In FileVMatrix::build_ - Could not fseek to last byte");
            }
            fputc('\0',f);
#endif
        }

        // Get rid of any pre-existing .metadata :: this ensures that old
        // fieldsizes do not pollute a newly-created FileVMatrix
        force_rmdir(filename_ + ".metadata");
    }
    else
    {
        if (writable)
            openfile(filename_, "r+b", PR_RDWR | PR_CREATE_FILE, 0666);
        else
            openfile(filename_, "rb", PR_RDONLY, 0666);

        if (! f)
            PLERROR("FileVMatrix::build: could not open file %s", filename_.c_str());

#ifdef USE_NSPR_FILE
        PR_Read(f,header,DATAFILE_HEADERLENGTH);
#else
        fread(header,DATAFILE_HEADERLENGTH,1,f);
#endif
        if(header[DATAFILE_HEADERLENGTH-1]!='\n')
            PLERROR("In FileVMatrix constructor, wrong header for PLearn binary matrix format. Please use checkheader (in PLearn/Scripts) to check the file.(0)");
        int file_length, file_width;
        bool need_update_header = false;
        sscanf(header, "%s%d%d%s%s", matorvec, &file_length, &file_width, datatype, endiantype);
        if (file_length == -1 && this->length_ >= 0 && writable) {
            // The length set in the file is not valid, but we have specified a length.
            // This can happen if build() has been called once before the sizes have
            // been specified. In this case we must modify the file's length.
            need_update_header = true;
        } else if (file_length >= 0 && this->length_ >= 0 && file_length != this->length_) {
            PLWARNING("In FileVMatrix::build_ - The length option of the VMatrix and the "
                      "loaded file differ; using the loaded file length (%d instead of %d)",
                      file_length, this->length_);
            this->length_ = file_length;
        } else {
            this->length_ = file_length;
        }

        if (file_width == -1 && this->width_ >= 0 && writable) {
            // Same as above, but for the width.
            need_update_header = true;
        } else if (file_width >= 0 && this->width_ >= 0 && file_width != this->width_) {
            PLWARNING("In FileVMatrix::build_ - The width option of the VMatrix and the "
                      "loaded file differ; using the loaded file width (%d instead of %d)",
                      file_width, this->width_);
            this->width_ = file_width;
        } else {
            this->width_ = file_width;
        }

        if (need_update_header) {
            updateHeader();
        }

        if (strcmp(matorvec,"MATRIX")!=0)
            PLERROR("In FileVMatrix constructor, wrong header for PLearn binary matrix format. Please use checkheader (in PLearn/Scripts) to check the file.(1)");

        if (strcmp(endiantype,"LITTLE_ENDIAN")==0)
            file_is_bigendian = false;
        else if (strcmp(endiantype,"BIG_ENDIAN")==0)
            file_is_bigendian = true;
        else
            PLERROR("In FileVMatrix constructor, wrong header for PLearn binary matrix format. Please use checkheader (in PLearn/Scripts) to check the file.(2)");

        if (strcmp(datatype,"FLOAT")==0)
            file_is_float = true;
        else if (strcmp(datatype,"DOUBLE")==0)
            file_is_float = false;
        else
            PLERROR("In FileVMatrix constructor, wrong header for PLearn binary matrix format. Please use checkheader (in PLearn/Scripts) to check the file.(3)");

        //resize the string mappings
        if (width_ >= 0) {
            map_sr = TVec<map<string,real> >(width_);
            map_rs = TVec<map<real,string> >(width_);
        }
#ifdef USE_NSPR_FILE
        //check if the file have all data
        PRFileInfo64 info;
        if(PR_FAILURE==PR_GetFileInfo64(filename_.absolute().c_str(), &info))
            PLERROR("In FileVMatrix::build_() - can't get file info for file '%s'",
                    filename_.c_str());
        PRInt64 elemsize = file_is_float ? sizeof(float) : sizeof(double);
        PRInt64 expectedsize=DATAFILE_HEADERLENGTH+length_*width_*elemsize;
        if(info.size!=expectedsize)
            PLWARNING("In FileVMatrix::build_() - The file '%s' have a size"
                      " of %ld, expected %ld",
                      filename_.c_str(), (long int)info.size, (long int)expectedsize);
#endif
        
    }

    setMetaDataDir(filename_ + ".metadata");
    setMtime(mtime(filename_));

    if (width_ >= 0)
        getFieldInfos();

    loadFieldInfos();
}


//////////////////////
// closeCurrentFile //
//////////////////////
void FileVMatrix::closeCurrentFile()
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
void FileVMatrix::declareOptions(OptionList & ol)
{
    declareOption(ol, "filename", &FileVMatrix::filename_, OptionBase::buildoption, "Filename of the matrix");

    declareOption(ol, "remove_when_done", &FileVMatrix::remove_when_done,
            OptionBase::learntoption,
            "Deprecated option! (use TemporaryFileVMatrix instead).");

    declareOption(ol, "track_ref", &FileVMatrix::track_ref,
            OptionBase::learntoption,
            "Deprecated option! (use TemporaryFileVMatrix instead).");

    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void FileVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // it is unclear whether f should be shared or not...
    // if we allow multi-threading, we should probably not share it
    // so for now we will not share it. But a cleaner behavoiur would probably be
    // to share it in multiple threads but make sure that getRow, putRow, etc... operations
    // are atomic (no context switch to another thread).

    f = 0;   // Because we will open again the file (f should not be shared).
    // however reopening the file twice in write mode is certainly a VERY bad idea.
    // thus we switch to read-mode 
    build_new_file = false;
    writable = false;

    build(); // To open the file.
}

//////////////////
// ~FileVMatrix //
//////////////////
FileVMatrix::~FileVMatrix()
{
    if (hasMetaDataDir() && isWritable())
        saveFieldInfos();
    FileVMatrix::closeCurrentFile();
}


///////////////
// getNewRow //
///////////////
void FileVMatrix::getNewRow(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(v.length()!=width_)
        PLERROR("In FileVMatrix::getNewRow length of v (%d) differs from matrix width (%d)",v.length(),width_);
#endif

    moveto(i);
#ifdef USE_NSPR_FILE
    if(file_is_float)
        PR_Read_float(f, v.data(), v.length(), file_is_bigendian);
    else
        PR_Read_double(f, v.data(), v.length(), file_is_bigendian);
#else
    if(file_is_float)
        fread_float(f, v.data(), v.length(), file_is_bigendian);
    else
        fread_double(f, v.data(), v.length(), file_is_bigendian);
#endif
}

///////////////
// putSubRow //
///////////////
void FileVMatrix::putSubRow(int i, int j, Vec v)
{
    moveto(i,j);
#ifdef USE_NSPR_FILE
    if(file_is_float)
        PR_Write_float(f, v.data(), v.length(), file_is_bigendian);
    else
        PR_Write_double(f, v.data(), v.length(), file_is_bigendian);
#else
    if(file_is_float)
        fwrite_float(f, v.data(), v.length(), file_is_bigendian);
    else
        fwrite_double(f, v.data(), v.length(), file_is_bigendian);
#endif

    invalidateBuffer();
}

void FileVMatrix::moveto(int i, int j) const
{
#ifdef USE_NSPR_FILE
    PRInt64 offset = i;
    int elemsize = file_is_float ? sizeof(float) : sizeof(double);
    offset *= width_;
    offset += j;
    offset *= elemsize;
    offset += DATAFILE_HEADERLENGTH;
    PR_Seek64(f, offset, PR_SEEK_SET);
#else
    if(file_is_float)
        fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(float), SEEK_SET);
    else
        fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(double), SEEK_SET);
#endif
}

/////////
// put //
/////////
void FileVMatrix::put(int i, int j, real value)
{
    moveto(i,j);
#ifdef USE_NSPR_FILE
    if(file_is_float)
        PR_Write_float(f,float(value),file_is_bigendian);
    else
        PR_Write_double(f,double(value),file_is_bigendian);
#else
    if(file_is_float)
        fwrite_float(f,float(value),file_is_bigendian);
    else
        fwrite_double(f,double(value),file_is_bigendian);
#endif
    invalidateBuffer();
}

///////////////
// appendRow //
///////////////
void FileVMatrix::appendRow(Vec v)
{
    moveto(length_,0);
#ifdef USE_NSPR_FILE
    if(file_is_float)
        PR_Write_float(f, v.data(), v.length(), file_is_bigendian);
    else
        PR_Write_double(f, v.data(), v.length(), file_is_bigendian);
#else
    if(file_is_float)
        fwrite_float(f, v.data(), v.length(), file_is_bigendian);
    else
        fwrite_double(f, v.data(), v.length(), file_is_bigendian);
#endif

    length_++;
    updateHeader();
}

///////////
// flush //
///////////
void FileVMatrix::flush()
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
void FileVMatrix::updateHeader() {
    char header[DATAFILE_HEADERLENGTH];
    string real = "DOUBLE";
    if(file_is_float)
        real = "FLOAT";

#ifdef LITTLEENDIAN
    sprintf(header,"MATRIX %d %d %s LITTLE_ENDIAN", length_, width_, real.c_str());
#endif
#ifdef BIGENDIAN
    sprintf(header,"MATRIX %d %d %s BIG_ENDIAN", length_, width_, real.c_str());
#endif


    int pos = strlen(header);
    for(; pos<DATAFILE_HEADERLENGTH; pos++)
    {
        header[pos] = ' ';
    }
    header[DATAFILE_HEADERLENGTH-1] = '\n';

#ifdef USE_NSPR_FILE
    PR_Seek64(f, 0, PR_SEEK_SET);
    PR_Write(f, header, DATAFILE_HEADERLENGTH);
#else
    fseek(f,0,SEEK_SET);
    fwrite(header,1,DATAFILE_HEADERLENGTH,f);
#endif
}

//////////////////
// updateHeader //
//////////////////
void FileVMatrix::openfile(const PPath& path, const char *mode,
                           PRIntn flags, PRIntn mode2) {
#ifdef USE_NSPR_FILE
        f = PR_Open(path.absolute().c_str(), flags, mode2);
#else
        f = fopen(path.absolute().c_str(), mode);
#endif

}


int64_t FileVMatrix::getSizeOnDisk(){
    return DATAFILE_HEADERLENGTH + width_*length_*(file_is_float ? 4 : 8);
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
