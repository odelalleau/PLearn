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



#ifndef PStream_INC
#define PStream_INC

#include <iosfwd>
#include <map>
#include <bitset>
#include <strstream>
#include "PP.h"
#include "pl_streambuf.h"
#include "pl_fdstream.h"
#include "PStream_util.h"
#include "plerror.h"

namespace PLearn <%
using namespace std;

/*!
 * PStream:
 *  This class defines a type of stream that should be used for all I/O within PLearn.
 *  It supports most operations available for standard c++ streams, plus:
 *   - a set of option flags that indicate which types of Object Options should be
 *     read/written (option_flags_{in|out}; has effect only for Object's);
 *   - a set of format flags that define format used for I/O. e.g.: "raw" for
 *     standard stream behaviour, "binary" for binary format, none for human-readable
 *     serialization format, etc. (pl_stream_flags_{in|out});
 *   - a copies map to alow smart serialization of pointers;
 *   - a markable stream buffer which allows to 'seek back' to a previously set mark
 *     on any type of istream;
 *   - an 'attach' function to attach the stream to a POSIX file descriptor;
 */

class PStream : public PPointable
{
 public:
  //! typedef's for PStream manipulators
  typedef PStream& (*pl_pstream_manip)(PStream&);
  typedef istream& (*pl_istream_manip_compat)(istream&);
  typedef ostream& (*pl_ostream_manip_compat)(ostream&);
  
#if __GNUC__ < 3
  //! typedef's for compatibility w/ old libraries
  typedef int streamsize;
  typedef ios ios_base;
#endif

  enum mode_t 
    {
      plearn_ascii,    //<! PLearn ascii serialization format (can be mixed with plearn_binary)
      plearn_binary,   //<! PLearn binary serialization format (can be mixed with plearn_ascii)
      raw_ascii,       //<! Raw C++ ascii output without additional separators (direct output to underlying ostream)
      raw_binary,      //<! Simply writes the bytes as they are in memory.
      pretty_ascii     //<! Ascii pretty print (in particular for Vec and Mat, formatted output without size info)
    };
  
  //! plearn_ascii and plearn_binary are used on output to determine in which format to write stuff.
  //! On input however, they are equivalent, as the right format is automatically detected.

  //! Compression mode (mostly used by binary serialization of sequences of floats or doubles, such as TMat<real>)
  //! (Used on output only; autodetect on read).
  enum compr_mode_t { 
    compr_none,            //<! No compression.
    compr_double_as_float, //<! In plearn_binary mode, store doubles as float
    compr_sparse,          //<! PLearn 
    compr_lossy_sparse     //<! Also stores double as float 
  };

  
protected:
  PP<pl_streambuf> the_inbuf;   //<! markable input buffer
  PP<pl_fdstreambuf> the_fdbuf; //<! buffer on a POSIX file descriptor
  
  istream* pin;  //<! underlying input stream
  ostream* pout; //<! underlying output stream
  bool own_pin, own_pout; //<! true if {pin|pout} was created internally
public:  
  OBflag_t option_flags_in;   //<! option flags for input
  mode_t inmode;              //<! mode for input formatting
  // bitset<32> pl_stream_flags_in;  //<! format flags for input
  map<unsigned int, void *> copies_map_in; //<! copies map for input
  OBflag_t option_flags_out; //<! option flags for output
  mode_t outmode;            //<! mode for output formatting
  // bitset<32> pl_stream_flags_out; //<! format flags for output
  map<void *, unsigned int> copies_map_out; //<! copies map for output

protected:
  //! ptrs. to the original buffers;  used to 'reset' the underlying
  //! streams to a valid state when the PStream is destroyed.
  streambuf* original_bufin, * original_bufout;
  
public:
  //! If true, then Mat and Vec will be serialized with their elements in place,
  //! If false, they will have an explicit pointer to a storage
  bool implicit_storage;
  //! Determines the way data is compressed, if any.
  compr_mode_t compression_mode;

public:  
  //! default ctor: the stream is unusable ...
  PStream()
    :pin(0), pout(0), own_pin(false), own_pout(false), 
     option_flags_in(dft_option_flag), inmode(plearn_ascii), 
     option_flags_out(dft_option_flag), outmode(plearn_ascii), 
     original_bufin(0), original_bufout(0), 
     implicit_storage(false), compression_mode(compr_none)
  {}
  //! ctor. from an istream (I)
  PStream(istream* pin_)
    :pin(pin_), pout(0), own_pin(false), own_pout(false), 
     option_flags_in(dft_option_flag), inmode(plearn_ascii), 
     option_flags_out(dft_option_flag), outmode(plearn_ascii),
     original_bufin(pin_->rdbuf()), original_bufout(0), 
    implicit_storage(false), compression_mode(compr_none)
  { initInBuf(); }
  //! ctor. from an ostream (O)
  PStream(ostream* pout_)
    :pin(0), pout(pout_), own_pin(false), own_pout(false), 
     option_flags_in(dft_option_flag), inmode(plearn_ascii), 
     option_flags_out(dft_option_flag), outmode(plearn_ascii),
     original_bufin(0), original_bufout(pout_->rdbuf()), 
     implicit_storage(false), compression_mode(compr_none)
  {}
  //! ctor. from an iostream (IO)
  PStream(iostream* pios_)
    :pin(pios_), pout(pios_), own_pin(false), own_pout(false), 
     option_flags_in(dft_option_flag), inmode(plearn_ascii), 
     option_flags_out(dft_option_flag), outmode(plearn_ascii),
     original_bufin(pios_->rdbuf()), original_bufout(pios_->rdbuf()),
  implicit_storage(false), compression_mode(compr_none)
  { initInBuf(); }
  //! ctor. from an istream and an ostream (IO)
  PStream(istream* pin_, ostream* pout_)
    :pin(pin_), pout(pout_), own_pin(false), own_pout(false), 
     option_flags_in(dft_option_flag), inmode(plearn_ascii), 
     option_flags_out(dft_option_flag), outmode(plearn_ascii),
     original_bufin(pin_->rdbuf()), original_bufout(pout_->rdbuf())
  { initInBuf(); }
  //! copy ctor.
  PStream(const PStream& pios)
    :the_inbuf(pios.the_inbuf), the_fdbuf(pios.the_fdbuf),
     pin(pios.pin), pout(pios.pout), own_pin(false), own_pout(false), 
     option_flags_in(pios.option_flags_in), inmode(pios.inmode), 
     option_flags_out(pios.option_flags_out), outmode(pios.outmode), 
     original_bufin(pios.original_bufin), original_bufout(pios.original_bufout)
  {}
  //! dtor.
  virtual ~PStream()
  {
    // am I the only PStream using this buffer?
    if(the_inbuf && 1 == the_inbuf->usage())
      {// reset underlying streams's buffers before destroying the_inbuf
	if(pin) pin->rdbuf(original_bufin);
	if(pout) pout->rdbuf(original_bufout);
      }
    if(own_pin && pin) delete pin; // delete pin if we created it
    if(own_pout && pout) delete pout; // delete pout if we created it
  }
  
protected:
  //! initInBuf: called by ctors. to ensure that pin's buffer is markable
  inline void initInBuf()
  {
    if(pin)
      {
	the_inbuf= dynamic_cast<pl_streambuf*>(pin->rdbuf());
	if(the_inbuf.isNull())
	  the_inbuf= new pl_streambuf(*pin->rdbuf());
	pin->rdbuf(the_inbuf);
      }
  }

public:
  //op()'s: re-init with different underlying stream(s)
  PStream& operator()(istream* pin_);
  PStream& operator()(ostream* pout_);
  PStream& operator()(iostream* pios_);
  PStream& operator()(istream* pin_, ostream* pout_);
  PStream& operator()(const PStream& pios);

  inline PStream& operator=(istream* pin_) { return operator()(pin_); }
  inline PStream& operator=(ostream* pout_)  { return operator()(pout_); }
  inline PStream& operator=(iostream* pios_)  { return operator()(pios_); }
  inline PStream& operator=(const PStream& pios)  { return operator()(pios); }


  //! op bool: true if the stream is in a valid state (e.g. "while(stm) stm >> x;")
  inline operator bool() { return (!pin || *pin) && (!pout || *pout) && (pin || pout); }

  inline istream& rawin() { return *pin; }   //<! access to underlying istream
  inline ostream& rawout() { return *pout; } //<! access to underlying ostream
  

  /******
   * The folowing methods are 'forwarded' from {i|o}stream.
   */
  inline int get() { return pin->get(); }
  inline PStream& get(char& c) { pin->get(c); return *this; }
  inline int peek() { return pin->peek(); }
  inline PStream& putback(char c) { pin->putback(c); return *this; }
  inline PStream& unget() { pin->unget(); return *this; }
  inline PStream& read(char* s, streamsize n) { pin->read(s,n); return *this; }
  inline PStream& put(char c) { pout->put(c); return *this; }
  inline PStream& write(const char* s, streamsize n) { pout->write(s,n); return *this; }
  inline PStream& flush() { pout->flush(); return *this; }
  /******/

  //! returns the markable input buffer
  inline pl_streambuf* pl_rdbuf() { return the_inbuf; }

  /******
   * The folowing methods are 'forwarded' from ios;  Two versions of each method
   * are provided so that input and output behaviour may be different.
   */
  inline ios_base::fmtflags flags_in() const { return pin->flags(); }
  inline ios_base::fmtflags flags_out() const { return pout->flags(); }
  inline ios_base::fmtflags flags_in(ios_base::fmtflags ff) { return pin->flags(ff); }
  inline ios_base::fmtflags flags_out(ios_base::fmtflags ff) { return pout->flags(ff); }
  /*NOTE: setf_{in|out} also exist in 'PLearn::pl_flags' version... see below */
  inline ios_base::fmtflags setf_in(ios_base::fmtflags ff) { return pin->setf(ff); }
  inline ios_base::fmtflags setf_out(ios_base::fmtflags ff) { return pout->setf(ff); }
  inline ios_base::fmtflags setf_in(ios_base::fmtflags ff, ios_base::fmtflags mask)
    { return pin->setf(ff,mask); }
  inline ios_base::fmtflags setf_out(ios_base::fmtflags ff, ios_base::fmtflags mask)
    { return pout->setf(ff,mask); }
  inline void unsetf_in(ios_base::fmtflags mask) { pin->unsetf(mask); }
  inline void unsetf_out(ios_base::fmtflags mask) { pout->unsetf(mask); }
  inline char fill_in() const { return pin->fill(); }
  inline char fill_out() const { return pout->fill(); }
  inline char fill_in(char c) { return pin->fill(c); }
  inline char fill_out(char c) { return pout->fill(c); }
  inline streamsize width_in() const { return pin->width(); }
  inline streamsize width_out() const { return pout->width(); }
  inline streamsize width_in(streamsize w) { return pin->width(w); }
  inline streamsize width_out(streamsize w) { return pout->width(w); }
  /******/

  //! attach this stream to a POSIX file descriptor.
  void attach(int fd);

  // decl./implementation of operator>>'s
  PStream& operator>>(bool &x);
  PStream& operator>>(float &x);
  PStream& operator>>(double &x);
  PStream& operator>>(char * &x);
  PStream& operator>>(string &x);
 
  inline PStream& operator>>(char &x)
  {
    switch(inmode)
      {
      case PStream::raw_ascii:
	rawin() >> x;
	break;
      case PStream::raw_binary:
	get(x);
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawin() >> x >> ws;
	break;
      default:
	PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
	break;
      }
    return *this;
  }
 
  inline PStream& operator>>(signed char &x)
  {
    switch(inmode)
      {
      case PStream::raw_ascii:
	rawin() >> x;
	break;
      case PStream::raw_binary:
	get(reinterpret_cast<char &>(x));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawin() >> x >> ws;
	break;
      default:
	PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
	break;
      }
    return *this;
  }

  inline PStream& operator>>(unsigned char &x)
  {
    switch(inmode)
      {
      case PStream::raw_ascii:
	rawin() >> x;
	break;
      case PStream::raw_binary:
	get(reinterpret_cast<char &>(x));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawin() >> x >> ws;
	break;
      default:
	PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
	break;
      }
    return *this;
  }

  inline PStream& operator>>(int &x)
  {
    switch(inmode)
      {
      case PStream::raw_ascii:
	rawin() >> x;
	break;
      case PStream::raw_binary:
	read(reinterpret_cast<char *>(&x), sizeof(int));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawin() >> x >> ws;
	break;
      default:
	PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
	break;
      }
    return *this;
  }

  inline PStream& operator>>(unsigned int &x)
  {
    switch(inmode)
      {
      case PStream::raw_ascii:
	rawin() >> x;
	break;
      case PStream::raw_binary:
	read(reinterpret_cast<char *>(&x), sizeof(unsigned int));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawin() >> x >> ws;
	break;
      default:
	PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
	break;
      }
    return *this;
  }
  
  inline PStream& operator>>(long int &x)
  {
    switch(inmode)
      {
      case PStream::raw_ascii:
	rawin() >> x;
	break;
      case PStream::raw_binary:
	read(reinterpret_cast<char *>(&x), sizeof(long int));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawin() >> x >> ws;
	break;
      default:
	PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
	break;
      }
    return *this;
  }
  
  inline PStream& operator>>(unsigned long &x)
  {
    switch(inmode)
      {
      case PStream::raw_ascii:
	rawin() >> x;
	break;
      case PStream::raw_binary:
	read(reinterpret_cast<char *>(&x), sizeof(unsigned long));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawin() >> x >> ws;
	break;
      default:
	PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
	break;
      }
    return *this;
  }
  
  inline PStream& operator>>(short int &x)
  {
    switch(inmode)
      {
      case PStream::raw_ascii:
	rawin() >> x;
	break;
      case PStream::raw_binary:
	read(reinterpret_cast<char *>(&x), sizeof(short int));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawin() >> x >> ws;
	break;
      default:
	PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
	break;
      }
    return *this;
  }
  
  inline PStream& operator>>(unsigned short &x)
  {
    switch(inmode)
      {
      case PStream::raw_ascii:
	rawin() >> x;
	break;
      case PStream::raw_binary:
	read(reinterpret_cast<char *>(&x), sizeof(unsigned short));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawin() >> x >> ws;
	break;
      default:
	PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
	break;
      }
    return *this;
  }
  
  template <typename S, typename T> 
  inline PStream& operator>>(pair<S, T> &x) { return *this >> x.first >> x.second; }
  
  template <typename S, typename T> 
  inline PStream& operator>>(map<S, T> &x)
  {
    int l;
    pair<S, T> p;
    *this >> l;
    x.clear();
    for (int i = 0; i < l; ++i) 
      {
	*this >> p;
	x.insert(p);
      }
    return *this;
  }


  // Implementation of operator<<'s
  PStream& operator<<(float x);
  PStream& operator<<(double x);
  PStream& operator<<(const char *x);
  PStream& operator<<(const string &x);

  inline PStream& operator<<(char x) 
  { 
    switch(outmode)
      {
      case PStream::raw_ascii:
	rawout() << x;
	break;
      case PStream::raw_binary:
	write(reinterpret_cast<char *>(&x), sizeof(x));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawout() << x << ' ';
	break;
      default:
	PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
	break;
      }
    return *this;
  }

  inline PStream& operator<<(signed char x) 
  { 
    switch(outmode)
      {
      case PStream::raw_ascii:
	rawout() << x;
	break;
      case PStream::raw_binary:
	write(reinterpret_cast<char *>(&x), sizeof(x));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawout() << x << ' ';
	break;
      default:
	PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
	break;
      }
    return *this;
  }

  inline PStream& operator<<(unsigned char x) 
  { 
    switch(outmode)
      {
      case PStream::raw_ascii:
	rawout() << x;
	break;
      case PStream::raw_binary:
	write(reinterpret_cast<char *>(&x), sizeof(x));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawout() << x << ' ';
	break;
      default:
	PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
	break;
      }
    return *this;
  }

  inline PStream& operator<<(int x) 
  { 
    switch(outmode)
      {
      case PStream::raw_ascii:
	rawout() << x;
	break;
      case PStream::raw_binary:
	write(reinterpret_cast<char *>(&x), sizeof(x));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawout() << x << ' ';
	break;
      default:
	PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
	break;
      }
    return *this;
  }

  inline PStream& operator<<(unsigned int x) 
  { 
    switch(outmode)
      {
      case PStream::raw_ascii:
	rawout() << x;
	break;
      case PStream::raw_binary:
	write(reinterpret_cast<char *>(&x), sizeof(x));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawout() << x << ' ';
	break;
      default:
	PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
	break;
      }
    return *this;
  }

  inline PStream& operator<<(long int x) 
  { 
    switch(outmode)
      {
      case PStream::raw_ascii:
	rawout() << x;
	break;
      case PStream::raw_binary:
	write(reinterpret_cast<char *>(&x), sizeof(x));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawout() << x << ' ';
	break;
      default:
	PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
	break;
      }
    return *this;
  }

  inline PStream& operator<<(unsigned long x) 
  { 
    switch(outmode)
      {
      case PStream::raw_ascii:
	rawout() << x;
	break;
      case PStream::raw_binary:
	write(reinterpret_cast<char *>(&x), sizeof(x));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawout() << x << ' ';
	break;
      default:
	PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
	break;
      }
    return *this;
  }

  inline PStream& operator<<(short int x) 
  { 
    switch(outmode)
      {
      case PStream::raw_ascii:
	rawout() << x;
	break;
      case PStream::raw_binary:
	write(reinterpret_cast<char *>(&x), sizeof(x));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawout() << x << ' ';
	break;
      default:
	PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
	break;
      }
    return *this;
  }

  inline PStream& operator<<(unsigned short x) 
  { 
    switch(outmode)
      {
      case PStream::raw_ascii:
	rawout() << x;
	break;
      case PStream::raw_binary:
	write(reinterpret_cast<char *>(&x), sizeof(x));
	break;
      case PStream::plearn_ascii:
      case PStream::plearn_binary:
	rawout() << x << ' ';
	break;
      default:
	PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
	break;
      }
    return *this;
  }

  inline PStream& operator<<(bool x) 
  { 
    switch(outmode)
      {
      case raw_ascii:
      case raw_binary:
	rawout() << (x?'1':'0');
	break;
      case plearn_ascii:
      case plearn_binary:
	rawout() << (x?'1':'0') << ' ';
	break;
      default:
	PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
	break;
      }
    return *this;
  }

  template<class A,class B>
  inline PStream& operator<<(const pair<A,B>& x) 
  { return *this << x.first << x.second; }

  template<class K, class V>
  inline PStream& operator<<(const map<K,V>& x) 
  {
    *this << x.size();
    for(typename map<K,V>::const_iterator i= x.begin(); i != x.end(); ++i) 
      *this << *i;
    return *this;
  }

  /*****
   * operator<<'s and operator>>'s to set flags, etc.
   */
  inline istream& operator>>(pl_stream_raw& raw_) { return rawin(); }
  inline ostream& operator<<(const pl_stream_raw& raw_) { return rawout(); }
  inline PStream& operator>>(pl_stream_clear_flags& flags_) { option_flags_in= 0; return *this; }
  inline PStream& operator<<(const pl_stream_clear_flags& flags_) { option_flags_out= 0; return *this; }
  inline PStream& operator>>(pl_stream_option_flags& flags_) { option_flags_in= flags_.flags; return *this; }
  inline PStream& operator<<(pl_stream_option_flags& flags_) { option_flags_out= flags_.flags; return *this; }
  inline PStream& operator>>(pl_stream_initiate& initiate_) { copies_map_in.clear(); return *this; }
  inline PStream& operator<<(const pl_stream_initiate& initiate_) { copies_map_out.clear(); return *this; }
  inline PStream& operator>>(pl_pstream_manip func) { return (*func)(*this); }
  inline PStream& operator<<(const pl_pstream_manip func) { return (*func)(*this); }
  inline PStream& operator>>(pl_istream_manip_compat func) { (*func)(*pin); return *this; }
  inline PStream& operator<<(const pl_ostream_manip_compat func) { (*func)(*pout); return *this; }
 

  /*****
   * op>> & op<< for generic pointers
   */

  template <class T> 
  inline PStream& operator>>(T*& x)
  {
    *this >> ws;
    if (peek() == '*') 
      {
	get(); // Eat '*'
	unsigned int id;
	*this >> id;
	if (id==0)
	  x = 0;
	else if (peek() == '-') 
	  {
	    get(); // Eat '-'
	    char cc = get();
	    if(cc != '>') // Eat '>'
	      PLERROR("In PStream::operator>>(T*&)  Wrong format.  Expecting \"*%d->\" but got \"*%d-%c\".", id, id, cc);
	    *this >> ws;
	    if(!x)
	      x= new T();
	    *this >> *x >> ws;
	    copies_map_in[id]= x;
	  } 
	else 
	  {
	    // Find it in map and return ptr;
	    map<unsigned int, void *>::iterator it = copies_map_in.find(id);
	    if (it == copies_map_in.end())
	      PLERROR("In PStream::operator>>(T*&) object (ptr) to be read has not been previously defined");
	    x= static_cast<T *>(it->second);
	  }
      } 
    else
      *this >> *x >> ws;
    
    return *this;
  }
  
  template <class T> 
  inline PStream& operator<<(const T*& x)
  {
    if(x)
      {
	map<void *, unsigned int>::iterator it = copies_map_out.find(const_cast<T*&>(x));
	if (it == copies_map_out.end()) 
	  {
	    int id = copies_map_out.size()+1;
	    rawout() << '*' << id << "->";
	    copies_map_out[const_cast<T*&>(x)] = id;
	    *this << *x;
	  }
	else 
	  rawout() << '*' << it->second << ' ';
      }
    else
      rawout() << "*0 ";
    return *this;
  }



  template <class T> 
  inline PStream& operator>>(PP<T> &o)
  {
    T *ptr;
    if (o.isNull())
      ptr = 0;
    else
      ptr = o;
    *this >> ptr;
    o = ptr;
    return *this;
  }

  template <class T> 
  inline PStream& operator<<(const PP<T> &o)
  {
    T *ptr = static_cast<T *>(o);
    *this << const_cast<const T * &>(ptr);
    return *this;
  }

  
};




// Default behavior for write() and read() is
// to call corresponding operator<<() or operator>>()
// on pl_{o|i}stream.

template<class T> 
inline void write(ostream& out_, const T& o, OBflag_t the_flags= dft_option_flag)
{
    PStream out(&out_);
    out << option_flags(the_flags) << o;
}

template<class T> 
inline void read(istream& in_, T& o, OBflag_t the_flags= dft_option_flag)
{
    PStream in(&in_);
    in >> option_flags(the_flags) >> o;
}

template<class T> 
inline void read(const string& stringval, T& x)
{
    istrstream in_(stringval.c_str());
    PStream in(&in_);
    in >> x;
}


  
%> // namespace PLearn

#endif //ndef PStream_INC
