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
   * $Id: FileVMatrix.cc,v 1.3 2003/03/19 23:15:23 jkeable Exp $
   ******************************************************* */

#include "FileVMatrix.h"

namespace PLearn <%
using namespace std;


/** FileVMatrix **/

IMPLEMENT_NAME_AND_DEEPCOPY(FileVMatrix);

FileVMatrix::FileVMatrix()
  :filename_(""), f(0) 
{}

FileVMatrix::FileVMatrix(const string& filename)
  :filename_(abspath(filename))
{
  build_();
}

FileVMatrix::FileVMatrix(const string& filename, int the_length, int the_width)
  :VMatrix(the_length, the_width), filename_(abspath(filename))
{
  f = fopen(filename.c_str(),"w+b");
  if (!f)
    PLERROR("In FileVMatrix constructor, could not open file %s for read/write",filename.c_str());

  setMtime(mtime(filename_));

  char header[DATAFILE_HEADERLENGTH]; 

#ifdef USEFLOAT
  file_is_float = true;
#ifdef LITTLEENDIAN
  file_is_bigendian = false; 
  sprintf(header,"MATRIX %d %d FLOAT LITTLE_ENDIAN", length_, width_);
#endif
#ifdef BIGENDIAN
  file_is_bigendian = true; 
  sprintf(header,"MATRIX %d %d FLOAT BIG_ENDIAN", length_, width_);
#endif
#endif
#ifdef USEDOUBLE
  file_is_float = false;
#ifdef LITTLEENDIAN
  file_is_bigendian = false; 
  sprintf(header,"MATRIX %d %d DOUBLE LITTLE_ENDIAN", length_, width_);
#endif
#ifdef BIGENDIAN
  file_is_bigendian = true; 
  sprintf(header,"MATRIX %d %d DOUBLE BIG_ENDIAN", length_, width_);
#endif
#endif

  // Pad the header with whites and terminate it with '\n'
  for(int pos=strlen(header); pos<DATAFILE_HEADERLENGTH; pos++)
    header[pos] = ' ';
  header[DATAFILE_HEADERLENGTH-1] = '\n';

  // write the header to the file
  fwrite(header,DATAFILE_HEADERLENGTH,1,f);

  if(length_ > 0 && width_ > 0) //ensure we can allocate enough space... if len>0, to ensure
  {             // that the header ends with a '\n'.
    if( fseek(f, DATAFILE_HEADERLENGTH+length_*width_*sizeof(real)-1, SEEK_SET) <0 )
    {
      perror("");
      PLERROR("In FileVMatrix constructor Could not fseek to last byte");
    }
    fputc('\0',f);
  }

  string fieldinfosfname = filename+".fieldnames";
  if(isfile(fieldinfosfname))
    loadFieldInfos(fieldinfosfname);
}

void FileVMatrix::build()
{
  inherited::build();
  build_();
}

void FileVMatrix::build_()
{
  char header[DATAFILE_HEADERLENGTH];
  char matorvec[20];
  char datatype[20];
  char endiantype[20];

  setMtime(mtime(filename_));

  if (writable)
    f = fopen(filename_.c_str(), "rw");
  else
    f = fopen(filename_.c_str(), "r");

  if (!f)
    PLERROR("In FileVMatrix constructor, could not open file %s for reading",filename_.c_str());
  fread(header,DATAFILE_HEADERLENGTH,1,f);
  if(header[DATAFILE_HEADERLENGTH-1]!='\n')
    PLERROR("In FileVMatrix constructor, wrong header for PLearn binary matrix format. Please use checkheader (in PLearn/Scripts) to check the file.(0)");
  sscanf(header,"%s%d%d%s%s",matorvec,&length_,&width_,datatype,endiantype);
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
  map_sr = TVec<map<string,real> >(width_);
  map_rs = TVec<map<real,string> >(width_);

  string fieldinfosfname = filename_+".fieldnames";
  if(isfile(fieldinfosfname))
    loadFieldInfos(fieldinfosfname);
}

void FileVMatrix::declareOptions(OptionList & ol)
{
  declareOption(ol, "filename", &FileVMatrix::filename_, OptionBase::buildoption, "Filename of the matrix");
  inherited::declareOptions(ol);
}

FileVMatrix::~FileVMatrix()
{ 
  if(f)
    fclose(f); 
}

real FileVMatrix::get(int i, int j) const
{
  if(file_is_float)
  {
    fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(float), SEEK_SET);
    return (real) fread_float(f,file_is_bigendian);
  }
  else
  {
    fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(double), SEEK_SET);
    return (real) fread_double(f,file_is_bigendian);
  }
}

void FileVMatrix::getSubRow(int i, int j, Vec v) const
{
  if(file_is_float)
  {
    fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(float), SEEK_SET);
    fread_float(f, v.data(), v.length(), file_is_bigendian);
  }
  else
  {
    fseek(f, DATAFILE_HEADERLENGTH+(i*width_+j)*sizeof(double), SEEK_SET);
    fread_double(f, v.data(), v.length(), file_is_bigendian);
  }  
}

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

  for(int pos=strlen(header); pos<DATAFILE_HEADERLENGTH; pos++)
    header[pos] = ' ';
  header[DATAFILE_HEADERLENGTH-1] = '\n';

  // write the header to the file
  fseek(f,0,SEEK_SET);
  fwrite(header,DATAFILE_HEADERLENGTH,1,f);
}

%> // end of namespcae PLearn
