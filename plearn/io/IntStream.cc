// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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

/* IntStream.cc */

#include "IntStream.h"
//#include <sys/stat.h>

#ifndef _MSC_VER
#include <unistd.h> 
#else 
#include <cstdlib>
#endif

//#include <cerrno>

// I am commenting this out because the compiler complained of previous declarations fronm stdlib.h throwing different exceptions!!! (Pascal)
// #include <malloc.h>

namespace PLearn {
using namespace std;

  IntStreamVMatrix::IntStreamVMatrix(IntStream& s, int window_size, int dummyinput, int eos)
    : RowBufferedVMatrix(s.size(),window_size), stream(&s), position(-1), 
    dummy_input(dummyinput), end_of_sequence_symbol(eos)
  {
    current_row.fill(dummy_input);
  }

  void IntStreamVMatrix::getRow(int i, Vec v) const
  {
    if (i==position+1) // most frequent case
    {
      int j=1;
      if (current_row[width()-1]==end_of_sequence_symbol)
      {
        for (;j<width()-1;j++) current_row[j-1]=dummy_input;
        current_row[j++ - 1]=end_of_sequence_symbol;
      }
      else
        for (;j<width();j++)
          current_row[j-1]=current_row[j];
#if USE_JS_HACK
      {
      real next=stream->next();
      if (next>=MAX_VOC_SIZE) next = MAX_VOC_SIZE-1;
      current_row[j-1] = next;
      }
#else
      current_row[j-1]=stream->next();
#endif
    }
    else if (i!=position) 
    {
      if (i>=width())
      {
        stream->seek(i-width()+1);
        for (int j=0;j<width();j++)
#if USE_JS_HACK
      {
      real next=stream->next();
      if (next>=MAX_VOC_SIZE) next = MAX_VOC_SIZE-1;
      current_row[j] = next;
      }
#else
      current_row[j]=stream->next();
#endif
      } else
      {
        if (i<0) PLERROR("IntStreamVMat::getRow at row %d < 0!",i);
        stream->seek(0);
        int j=0;
        for (;j<width()-i-1;j++)
          current_row[j]=dummy_input;
        for (;j<width();j++)
#if USE_JS_HACK
      {
      real next=stream->next();
      if (next>=MAX_VOC_SIZE) next = MAX_VOC_SIZE-1;
      current_row[j] = next;
      }
#else
      current_row[j]=stream->next();
#endif
      }
    }
    position=i;
    v << current_row;
  }

FilesIntStream::FilesIntStream(int nfiles, const char* files[])
  : IntStream(-1), n_files(nfiles), file_names(files), current_file(0),
    next_pos_in_current_file(0)
{
  fp=(FILE**)malloc(n_files*sizeof(FILE*));
  sizes=(int*)calloc(n_files,sizeof(int));
  total_size=0;
  for (int i=0;i<n_files;i++) {
    fp[i]=fopen(file_names[i],"r");
    if (!fp[i]) 
      PLERROR("FilesIntStream::FilesIntStream, can't open file %s\n",file_names[i]);
    if (fseek(fp[i],0,SEEK_END))
      PLERROR("In FileIntStream constructor: fseek(%s,0,SEEK_END) failed\n",file_names[i]);
    total_size+=(sizes[i] = (ftell(fp[i])/sizeof(int)));
    fseek(fp[i],0,SEEK_SET);
  }
  read_current();
}

void FilesIntStream::reopen()
{
  // re-open all the file pointers
  for (int i=0;i<n_files;i++) {
    fp[i]=fopen(file_names[i],"r");
    if (!fp[i]) 
      PLERROR("FilesIntStream::reopen, can't open file %s\n",file_names[i]);
    fseek(fp[i],0,SEEK_SET);
  }
  // return to same position as previously
  seek(pos);
}

// read from current current_file at next_pos_in_current_file into current_value,
// and increment next_pos_in_current_file
void FilesIntStream::read_current()
{
  if (n_files<1) PLERROR("FilesIntStream::read_current(): no file opened");
  if (pos==total_size) {
    seek(0);
    return;
  }
  if (next_pos_in_current_file==sizes[current_file]) {
    next_pos_in_current_file = 0;
    current_file++;
    if (current_file==n_files)
      { seek(0); return; }
  }
  if (fread(&current_value,sizeof(int),1,fp[current_file])!=1) {
    int posit=ftell(fp[current_file]);

    // norman: added check. Can be done better
#ifdef WIN32
    fprintf(stderr,"process could not read 1 int from %s at position %d, ftell=%d\nerrno=%d,%s",
      file_names[current_file],next_pos_in_current_file+1,
      posit,errno,strerror(errno));
#else
    int pid=getpid();
    fprintf(stderr,"process %d could not read 1 int from %s at position %d, ftell=%d\nerrno=%d,%s",
      pid,file_names[current_file],next_pos_in_current_file+1,
      posit,errno,strerror(errno));
#endif

    exit(1);
  }
#ifdef BIGENDIAN
    reverse_int(&current_value,1);
#endif
  next_pos_in_current_file++;
  pos++;
}

// move to given position
void FilesIntStream::seek(long position)
{
  if (position<0 || position>=total_size) {
    fprintf(stderr,"FilesIntStream::seek(%ld), argument must be in [0,%d)\n",
      position,total_size);
    exit(1);
  }
  pos=0;
  int i;
  for (i=0;i<n_files-1 && position>=pos+sizes[i];i++) pos+=sizes[i];
  next_pos_in_current_file=position-pos;
  for (int j=0;j<n_files;j++) {
    int p = (i==j)?next_pos_in_current_file*sizeof(int):0;
    if (fseek(fp[j],p,SEEK_SET)) 
      PLERROR("In FileIntStream::seek fseek(%s,%d,SEEK_SET) failed\n",file_names[j],next_pos_in_current_file);
  }
  current_file=i;
  // pos will be incremented in read_current()
  pos=position-1;
  read_current();
}

// return next available int from the stream and increment position
int FilesIntStream::next()
{
  int c=current_value;
  read_current();
  return c;
}

// return next available int from the stream
int FilesIntStream::current()
{
  return current_value;
}

// total length of the stream
long FilesIntStream::size()
{
  return total_size;
}

FilesIntStream::~FilesIntStream()
{
  for (int i=0;i<n_files;i++)
    fclose(fp[i]);
  free(fp);
  free(sizes);
}

// ******************************************************
// convert <word_sequences> filename into a FilesIntStream stream
FilesIntStream* word_sequences2files_int_stream(const char* word_sequences_file)
{
  FILE* word_sequences_fp=fopen(word_sequences_file,"r");
  if (!word_sequences_fp)
    PLERROR("word_sequences2files_int_stream: can't open file %s",word_sequences_file);
  typedef const char* cstring;
  const char** word_sequences = new cstring[1000];
  int n_word_sequences=0;
  char buffer[1000];
  while (!feof(word_sequences_fp)) {
    if (!fgets(buffer,1000,word_sequences_fp)) break;
    int l=(int)strlen(buffer);
    if (buffer[l-1]=='\n') buffer[l-1]='\0';
    word_sequences[n_word_sequences]=tostring(buffer).c_str();
    n_word_sequences++;
  }
  fclose(word_sequences_fp);
  return new FilesIntStream(n_word_sequences,word_sequences);
}

InMemoryIntStream::InMemoryIntStream(IntStream& stream)
{
  length = stream.size();
  data = new int[length];
  for (int i=0;i<length;i++)
    data[i] = stream.next();
}

} // end of namespace PLearn

