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
   * $Id: FileVMatrix.cc,v 1.29 2005/02/18 17:13:39 tihocan Exp $
   ******************************************************* */

#include "FileVMatrix.h"
#include <plearn/io/fileutils.h>

namespace PLearn {
using namespace std;


/** FileVMatrix **/

PLEARN_IMPLEMENT_OBJECT(FileVMatrix,
    "Saves and reads '.pmat' files.",
    "The 'track_ref' option can be used to count how many FileVMatrix are\n"
    "accessing a given file. This can be useful to remove the '.pmat' file\n"
    "only when nobody else accesses it."
);

/////////////////
// FileVMatrix //
/////////////////
FileVMatrix::FileVMatrix()
: filename_(""), f(0), build_new_file(false),
  old_filename(""),
  remove_when_done(false),
  track_ref(false)

{
  writable=true;
}

FileVMatrix::FileVMatrix(const PPath& filename, bool writable_)
: filename_(filename.absolute()), f(0), build_new_file(!isfile(filename)),
  old_filename(""),
  remove_when_done(false),
  track_ref(false)
{
  writable = writable_;
  build_();
}

static int strlen(char* s) {
  int n=0;
  while (s[n]!=0) 
    n++;
  return n;
}

FileVMatrix::FileVMatrix(const PPath& filename, int the_length, int the_width)
: inherited(the_length, the_width), filename_(filename.absolute()), f(0),
  build_new_file(true),
  old_filename(""),
  remove_when_done(false),
  track_ref(false)
{
  writable = true;
  build_();
}

FileVMatrix::FileVMatrix(const PPath& filename, int the_length, const TVec<string>& fieldnames)
: inherited(the_length, fieldnames.length()), filename_(filename.absolute()), f(0),
  build_new_file(true),
  old_filename(""),
  remove_when_done(false),
  track_ref(false)
{
  writable = true;
  build_();
  declareFieldNames(fieldnames);
  saveFieldInfos();
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
  
  // Code below is commented because the filename may have been changed,
  // in which case f should be modified.
  // TODO note that since it's a FILE*, there will probably be some memory leak.
//  if (f) return; // file already built
  // Since we are going to re-create it, we can close the current f.
  if (f) {
    fclose(f);
  }
  if (track_ref && old_filename != "") {
    count_refs[old_filename.absolute()]--;
  }

  char header[DATAFILE_HEADERLENGTH];
  char matorvec[20];
  char datatype[20];
  char endiantype[20];

  if (build_new_file || !isfile(filename_))
    force_mkdir_for_file(filename_);

  setMetaDataDir(filename_ + ".metadata"); 
  setMtime(mtime(filename_));

  if (build_new_file || !isfile(filename_))
  {
    if (!writable) {
      PLERROR("In FileVMatrix::build_ - You asked to create a new file, but 'writable' is set to 0 !");
    }
    f = fopen(filename_.c_str(),"w+b");
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

    updateHeader();
    
    if(length_ > 0 && width_ > 0) //ensure we can allocate enough space... if len>0, to ensure
    {             // that the header ends with a '\n'.
      if( fseek(f, DATAFILE_HEADERLENGTH+length_*width_*sizeof(real)-1, SEEK_SET) <0 )
      {
        perror("");
        PLERROR("In FileVMatrix::build_ - Could not fseek to last byte");
      }
      fputc('\0',f);
    }
  }
  else
  {
    if (writable)
      f = fopen(filename_.c_str(), "r+b");
    else
      f = fopen(filename_.c_str(), "rb");

    if (! f)
      PLERROR("FileVMatrix::build: could not open file %s", filename_.c_str());
    
    fread(header,DATAFILE_HEADERLENGTH,1,f);
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
      PLERROR("In FileVMatrix::build_ - Lengths of the VMatrix and of the file loaded differ");
    } else {
      this->length_ = file_length;
    }

    if (file_width == -1 && this->width_ >= 0 && writable) {
      // Same as above, but for the width.
      need_update_header = true;
    } else if (file_width >= 0 && this->width_ >= 0 && file_width != this->width_) {
      PLERROR("In FileVMatrix::build_ - Widths of the VMatrix and of the file loaded differ");
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
  }

  if (width_ >= 0) {
    getFieldInfos();
  }

  if (track_ref) {
    string abs = filename_.absolute();
    if (count_refs.find(abs) != count_refs.end())
      count_refs[abs]++;
    else
      count_refs[abs] = 1;
    old_filename = filename_;
  } else {
    old_filename = "";
  }
}

////////////////
// count_refs //
////////////////
map<string, int> FileVMatrix::count_refs;

///////////////
// countRefs //
///////////////
int FileVMatrix::countRefs(const PPath& filename) {
  string abs = filename.absolute();
  if (count_refs.find(abs) == count_refs.end())
    return 0;
  return count_refs[abs];
}

////////////////////
// declareOptions //
////////////////////
void FileVMatrix::declareOptions(OptionList & ol)
{
  declareOption(ol, "filename", &FileVMatrix::filename_, OptionBase::buildoption, "Filename of the matrix");

  declareOption(ol, "remove_when_done", &FileVMatrix::remove_when_done, OptionBase::buildoption,
      "If set to 1, the file 'filename' will be deleted when this object is deleted\n"
      "(and we think no other FileVMatrix is accessing it.");

  declareOption(ol, "track_ref", &FileVMatrix::track_ref, OptionBase::buildoption,
      "If set to 1, will be counted in the FileVMatrix that access 'filename', which may prevent conflicts.");

  inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void FileVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields
  // ### that you wish to be deepCopied rather than
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // TODO Copy correctly the field FILE* f.
//  deepCopyField(f, copies);

  // Not an error because we may want to do some deep-copying sometimes.
//  PLWARNING("FileVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");

  f = 0;   // Because we will open again the file (f should not be shared).
  old_filename = ""; // Because build() has not yet been called for this FileVMatrix.
  build(); // To open the file.
}

//////////////////
// ~FileVMatrix //
//////////////////
FileVMatrix::~FileVMatrix()
{ 
  saveFieldInfos();
  if(f) {
    fclose(f); 
    // TODO Shouldn't we also delete f ?
  }
  if (track_ref)
    count_refs[filename_]--;

  if (remove_when_done && filename_ != "")
    if (!track_ref || count_refs[filename_] == 0) {
      rm(filename_);
    }
}

///////////////
// getNewRow //
///////////////
void FileVMatrix::getNewRow(int i, const Vec& v) const
{
  if(file_is_float)
  {
    fseek(f, DATAFILE_HEADERLENGTH+(i*width_)*sizeof(float), SEEK_SET);
    fread_float(f, v.data(), v.length(), file_is_bigendian);
  }
  else
  {
    fseek(f, DATAFILE_HEADERLENGTH+(i*width_)*sizeof(double), SEEK_SET);
    fread_double(f, v.data(), v.length(), file_is_bigendian);
  }  
}

///////////////
// putSubRow //
///////////////
void FileVMatrix::putSubRow(int i, int j, Vec v)
{
  if(file_is_float)
  {
    fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(float), SEEK_SET);
    fwrite_float(f, v.data(), v.length(), file_is_bigendian);
  }
  else
  {
    fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(double), SEEK_SET);
    fwrite_double(f, v.data(), v.length(), file_is_bigendian);
  }  
}

/////////
// put //
/////////
void FileVMatrix::put(int i, int j, real value)
{
  if(file_is_float)
  {
    fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(float), SEEK_SET);
    fwrite_float(f,float(value),file_is_bigendian);
  }
  else
  {
    fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(double), SEEK_SET);
    fwrite_double(f,double(value),file_is_bigendian);
  }
}

///////////////
// appendRow //
///////////////
void FileVMatrix::appendRow(Vec v)
{
  if(file_is_float)
  {
    fseek(f,DATAFILE_HEADERLENGTH+length_*width_*sizeof(float), SEEK_SET);
    fwrite_float(f, v.data(), v.length(), file_is_bigendian);
  }
  else
  {
    fseek(f,DATAFILE_HEADERLENGTH+length_*width_*sizeof(double), SEEK_SET);
    fwrite_double(f, v.data(), v.length(), file_is_bigendian);
  }
  length_++;

  updateHeader();
}

///////////
// flush //
///////////
void FileVMatrix::flush()
{
  fflush(f);
}

//////////////////
// updateHeader //
//////////////////
void FileVMatrix::updateHeader() {
  char header[DATAFILE_HEADERLENGTH]; 
#ifdef USEFLOAT
#ifdef LITTLEENDIAN
  sprintf(header,"MATRIX %d %d FLOAT LITTLE_ENDIAN", length_, width_);
#endif
#ifdef BIGENDIAN
  sprintf(header,"MATRIX %d %d FLOAT BIG_ENDIAN", length_, width_);
#endif
#endif
#ifdef USEDOUBLE
#ifdef LITTLEENDIAN
  sprintf(header,"MATRIX %d %d DOUBLE LITTLE_ENDIAN", length_, width_);
#endif
#ifdef BIGENDIAN
  sprintf(header,"MATRIX %d %d DOUBLE BIG_ENDIAN", length_, width_);
#endif
#endif
  int pos = strlen(header);
  for(; pos<DATAFILE_HEADERLENGTH; pos++)
    {
      header[pos] = ' ';
    }
  header[DATAFILE_HEADERLENGTH-1] = '\n';
  fseek(f,0,SEEK_SET);
  fwrite(header,1,DATAFILE_HEADERLENGTH,f);
}

} // end of namespcae PLearn
