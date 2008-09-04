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

/* IntStream.h

This class represents a read-only stream of integers (int).

It can be constructed from a set of files (using the 
FilesIntStream subclass).

*/


/*! \file PLearn/plearn/io/IntStream.h */

#ifndef MODULE_INTSTREAM
#define MODULE_INTSTREAM

//#include <cstdio>
//#include "VMat.h" //!<  to make a sub-class of VMat with an IntStream inside
#include <plearn/vmat/RowBufferedVMatrix.h>

namespace PLearn {
using namespace std;

class IntStream : public Object {

protected:
    long pos;

public:
    IntStream(int p=0) : pos(p) {}

    //!  move to given position
    virtual void seek(long position) { if (position!=0) PLERROR("IntStream::position should be 0"); }

    //!  return currently available int from the stream (at currente position)
    //!  and THEN increment position to NEXT item
    virtual int next() { return 0; }

    //!  return next available int from the stream
    virtual int current() { return 0; }

    //!  total length of the stream
    virtual long size() { return 0; }

    //!  current position (= size if we are past the last read element)
    virtual int position() { return pos; }

    //!  remaining int's (INCLUDING the current one), = 0 if none left to read
    virtual long remaining() { return size()-position(); }

/*!     Re-open all the file pointers and seek(position()).
  This is useful when a forked child process wants access
  to the same stream without interfering with parent
*/
    virtual void reopen() {}

    virtual ~IntStream() {}
};

// JS-HACK
#define USE_JS_HACK 0
#if USE_JS_HACK
#define MAX_VOC_SIZE 3
#endif
// a VMat which is implemented with an IntStream 
class IntStreamVMatrix : public RowBufferedVMatrix {
protected:
    PP<IntStream> stream; //!< where the data actually is
    mutable int position; //!< position in the stream corresponding to the current window

    /*! current_row is a window on the stream, with
      current_row[width()-1] being the element at position 'position' of the stream.
      Note also that the first width()-1 elements of current row when at position 0
      or "end of sequence" are filled with a special value, dummy_input: */
    int dummy_input;
    /*! When the end_of_sequence_symbol is encountered, the symbols that follow
      are "isolated" from the symbols that precede, i.e. when this symbol moves
      from element width()-1 to width()-1, the elements that precede are
      replaced by 'dummy_input'. */
    int end_of_sequence_symbol;
public:
    IntStreamVMatrix() {}
    IntStreamVMatrix(IntStream& s, int window_size, int dummy_input, int eos);
    virtual void getRow(int i, Vec v) const;
};

class FilesIntStream : public IntStream {

protected:
    int n_files; //!<  number of files
    const char* *file_names; //!<  names of each of the files;
    FILE** fp; //!<  pointers to each of the files;
    int current_file; //!<  between 0 and n_files-1
    int next_pos_in_current_file; //!<  position within current file
    int* sizes;
    int total_size; //!<  sum_i sizes[i]
    int current_value; //!<  just read at current position
    //!  disallow a copy
    FilesIntStream(FilesIntStream& x) { PLERROR("FilesIntStream can't be copied"); }

    //!  read from current current_file at next_pos_in_current_file into current_value
    void read_current();

public:
    FilesIntStream(int nfiles, const char* files[]);

    //!  move to given position
    virtual void seek(long position);

    //!  return next available int from the stream and increment position
    virtual int next();

    //!  return next available int from the stream
    virtual int current();

    //!  total length of the stream
    virtual long size();

/*!     re-open all the file pointers and seek(position())
  this is useful when a forked child process wants access
  to the same stream without interfering with parent
*/
    virtual void reopen();

    virtual ~FilesIntStream();
};

class InMemoryIntStream : public IntStream {
protected:
    int* data;
    int length;

public:
    //!  copy stream to memory
    InMemoryIntStream(IntStream& stream);

    virtual void seek(long position) 
    { 
        pos = position; 
#ifdef BOUNDCHECK
        if (pos<0 || pos>=length) PLERROR("InMemoryIntStream::seek(%d) out of range (0,%d)",
                                          position,length-1);
#endif
    }
    virtual int next() { 
        int v=data[pos];
        pos++;
        if (pos>=length) pos=0;
        return v;
    }
    virtual int current() { return data[pos]; }
    virtual long size() { return length; }

    virtual ~InMemoryIntStream() { delete[] data; }
};

//*!< *****************************************************
/*!   Convert \<word_sequences\> filename into a FilesIntStream stream.
  This file must contain one line per \<word_sequence\> filename,
  and each of these filenames must represent binary integers files
  that can be associated to an IntStream.
*/
FilesIntStream* word_sequences2files_int_stream(const char* word_sequences_file);

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
