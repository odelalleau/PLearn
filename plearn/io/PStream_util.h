// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2002 Frederic Morin, Xavier Saint-Mleux <saintmlx@iro.umontreal.ca>
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


#ifndef PStream_util_INC
#define PStream_util_INC

#include <string>
#include "pl_nullstreambuf.h"
#include <iosfwd>

namespace PLearn <%
using namespace std;


//!  *** LITTLE-ENDIAN / BIG-ENDIAN HELL... ***

  // This is initially set to false.  This flag is useful for reading data
  // that has been written on an architecture with a different endianness
  // than the current architecture. If set to true, then the binread
  // operations will call endianswap after reading their data.  The
  // principle is that binwrite always writes the data as it has it in
  // memory (i.e. with the endianness of the architecture that writes it).
  // save/load will typically store/use endianness information in the file
  // header (typically 'ABCD' for big endian 'DCBA' for little endian), so
  // that load can possibly set swap_endians_on_read before calling read
  extern bool swap_endians_on_read; 

  //! swaps endians for n 2-byte elements (such as short)
  void endianswap2(void* ptr, int n);
  //! swaps endians for n 4-byte elements (such as int or float)
  void endianswap4(void* ptr, int n);
  //! swaps endians for n 8-byte elements (such as double)
  void endianswap8(void* ptr, int n);

  // Versions for short, int, float and double
  inline void endianswap(short* ptr, int n=1) { endianswap2(ptr,n); }
  inline void endianswap(unsigned short* ptr, int n=1) { endianswap2(ptr,n); }
  inline void endianswap(int* ptr, int n=1) { endianswap4(ptr,n); }
  inline void endianswap(unsigned int* ptr, int n=1) { endianswap4(ptr,n); }
  inline void endianswap(float* ptr, int n=1) { endianswap4(ptr,n); }
  inline void endianswap(double* ptr, int n=1) { endianswap8(ptr,n); }



//! List of characters considered to mark a separation between "words"; 
//! This is a fairly restricted list, meaning that many things can be part of a "word"
//! in this sense (for ex: "this-is_a+single@wor'd"), this is to insure a smooth transition
//! for the new setOption, which calls readOptionVal ... which may call read(istream&, string&)...
static const string wordseparators(" \t\n\r)]};,|#"); 





//!the following typedef should be equivalent to OptionBase::flag_t
typedef unsigned int OBflag_t;
const OBflag_t dft_option_flag= 1 | 1<<1 | 1<<2;//!< buildoption | learntoption | tuningoption

/*
//format fags available for PStreams
enum pl_flags { plf_plain,
                plf_binary,
                plf_swap_endian,
                plf_shorts_as_doubles,
                plf_doubles_as_shorts,
		plf_raw
             };


// For use in bitset<32> constructor ONLY!
#define PLF_PLAIN    ( 1 << (plf_plain) )
#define PLF_BINARY   ( 1 << (plf_binary) )
*/

// Define some useful shortcuts
class pl_stream_raw {};
class pl_stream_clear_flags {};
class pl_stream_option_flags {
public:
    OBflag_t flags;
    pl_stream_option_flags &operator()(OBflag_t flags_)
        { flags = flags_; return *this; };
};
class pl_stream_initiate {};


extern pl_stream_raw raw;
extern pl_stream_clear_flags clear_flags;
extern pl_stream_option_flags option_flags;
extern pl_stream_initiate initiate;


extern ostream nullout; //!<  a null ostream: writing to it does nothing
extern istream nullin; //!<  a null instream: reading from it does nothing
extern iostream nullinout; //!< a null iostream: reading/writing from/to it does nothing


%>

#endif //ndef PStream_util_INC
