// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2002 Frederic Morin
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
   * $Id: plstreams.h,v 1.5 2002/09/17 01:27:33 zouave Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

// This file contains additional C++ streams definitions
// that are used in the PLearn Library


/*! \file PLearnLibrary/PLearnCore/plstreams.h */

#ifndef plstreams_INC
#define plstreams_INC
/*
#include <fstream>
#include <map>
#include <bitset>
#include <strstream.h>
#include "plerror.h"
#include "pl_nullstreambuf.h"
#include "pl_streambuf.h"
#include "pl_fdstream.h"
#include "PStream_util.h"
*/
namespace PLearn <%
using namespace std;


//class PStream;
//fwd decl
//class pl_istream;
//class pl_ostream
//class pl_stream;

/*
typedef PStream pl_istream;
typedef PStream pl_ostream;
*/
//#define pl_istream PStream
//#define pl_ostream PStream

// NOTE: For portability, we should define {i|o|io} streams which does nothing
//       instead of opening (a probably non-existing file) named /dev/null
//extern ostream nullout; //!<  a null ostream: writing to it does nothing
//extern istream nullin; //!<  a null instream: reading from it does nothing
//extern iostream nullinout; //!< a null iostream: reading/writing from/to it does nothing

//!  input streams that can be assigned to.
/*
class iassignstream : public istream 
{
private:
  PP<pl_streambuf> the_buf;
public:
  //!  iassignstream(): istream(0), the_buf(0) {}
  iassignstream(const istream& in=nullin)
    : istream(0), the_buf(dynamic_cast<pl_streambuf*>(in.rdbuf()))
  {
    if(the_buf.isNull() && in.rdbuf() != 0)
      the_buf= new pl_streambuf(*in.rdbuf());
    rdbuf(the_buf);
    clear(in.rdstate());
  }
  iassignstream(const iassignstream& in): istream(in.rdbuf()), the_buf(in.the_buf) {}
  void operator=(const istream& in) 
  {
    the_buf= dynamic_cast<pl_streambuf*>(in.rdbuf());
    if(the_buf.isNull() && in.rdbuf() != 0)
      the_buf= new pl_streambuf(*in.rdbuf());
    rdbuf(the_buf);
    clear(in.rdstate());
  }
  void operator=(const iassignstream& in) 
  { 
    rdbuf(in.rdbuf()); 
    the_buf= in.the_buf; 
    clear(in.rdstate());
  }
  
  pl_streambuf* pl_rdbuf()
  { return the_buf; }

};
*/
//!  output streams that can be assigned to.
/*
class oassignstream : public ostream 
{
public:
  //!  oassignstream(): ostream() {}
  oassignstream(const ostream& out=nullout): ostream(out.rdbuf()) {}
  oassignstream(const oassignstream& out): ostream(out.rdbuf()) {}
  void operator=(const ostream& out) 
  { 
    rdbuf(out.rdbuf()); 
    clear(out.rdstate());
  }
  void operator=(const oassignstream& out) 
  { 
    rdbuf(out.rdbuf()); 
    clear(out.rdstate());
  }
};
*/

//! input/output streams that can be assigned to.
/*
class ioassignstream : public iostream
{
private:
  PP<pl_streambuf> the_buf;
  PP<pl_fdstreambuf> the_fdbuf;
public:
  ioassignstream(const iostream &stream=nullinout)
    : iostream(0), the_buf(dynamic_cast<pl_streambuf*>(stream.rdbuf())), the_fdbuf(0)
  {
    if(the_buf.isNull() && stream.rdbuf() != 0)
      the_buf= new pl_streambuf(*stream.rdbuf());
    rdbuf(the_buf);
    clear(stream.rdstate());
  }
  ioassignstream(const ioassignstream &stream): iostream(stream.rdbuf()), the_buf(stream.the_buf), the_fdbuf(stream.the_fdbuf) {}
  void operator=(const iostream &stream)
  {
    the_buf= dynamic_cast<pl_streambuf*>(stream.rdbuf());
    if(the_buf.isNull() && stream.rdbuf() != 0)
      the_buf= new pl_streambuf(*stream.rdbuf());
    the_fdbuf= dynamic_cast<pl_fdstreambuf*>(&the_buf->original_buf);
    rdbuf(the_buf);
    clear(stream.rdstate());
  }
  void operator=(const ioassignstream &stream) 
  { 
    rdbuf(stream.rdbuf());
    the_buf= stream.the_buf;
    the_fdbuf= stream.the_fdbuf;
    clear(stream.rdstate());
  }

  //! for compatibility with old non-standard stl fstream
  void attach(int fd);

  pl_streambuf* pl_rdbuf()
  { return the_buf; }

};

*/


/*
//!the following typedef should be equivalent to OptionBase::flag_t
typedef unsigned int OBflag_t;
const OBflag_t dft_option_flag= 1 | 1<<1 | 1<<2;//!< buildoption | learntoption | tuningoption


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
*/

//typedef pl_istream & (*pl_istream_manip)(pl_istream &);
//typedef pl_ostream & (*pl_ostream_manip)(pl_ostream &);

/*
class pl_istream: public iassignstream {
 public:
    pl_istream(const pl_istream &in)
        : iassignstream(in), user_flags_(in.user_flags_), pl_stream_flags(in.pl_stream_flags)
        {};
    pl_istream(const istream &in = nullin)
        : iassignstream(in), user_flags_(dft_option_flag), pl_stream_flags(0)
        {}

    pl_istream &operator() (istream &in)
        { iassignstream::operator=(in); return *this; };

    OBflag_t user_flags_; 
    bitset<32> pl_stream_flags;
    map<unsigned int, void *> map_;
protected:
 private:
}; // class pl_istream


class pl_ostream: public oassignstream {
 public:
    pl_ostream(const pl_ostream &out)
        : oassignstream(out), user_flags_(out.user_flags_), pl_stream_flags(out.pl_stream_flags)
        {};
    pl_ostream(const ostream &out = nullout)
        : oassignstream(out), user_flags_(dft_option_flag), pl_stream_flags(0)
        {};

    pl_ostream &operator() (ostream &out)
        { oassignstream::operator=(out); return *this; };

    OBflag_t user_flags_;
    bitset<32> pl_stream_flags;
    map<void *, unsigned int> map_;
 protected:
 private:
}; // class pl_ostream
*/

/*
class pl_stream: public ioassignstream {
public:
    pl_stream(const pl_stream &in_out)
        : ioassignstream(in_out), ostr(in_out.ostr), istr(in_out.istr)
        {
	}
    pl_stream(const iostream &in_out = nullinout)
        : ioassignstream(in_out), ostr(), istr()
        {
	  ostr(*this);
	  istr(*this);
	}

    pl_stream &operator() (iostream &in_out)
        { ioassignstream::operator=(in_out); ostr(in_out); istr(in_out); return *this; };


    operator pl_istream&() { return istr; }
    operator pl_ostream&() { return ostr; }
    inline void clear(iostate st= ios::goodbit)
    {
      iostream::clear(st);
      istr.clear(st);
      ostr.clear(st);
    }

    void attach(int fd);

    pl_ostream ostr;
    pl_istream istr;
protected:
private:
}; // class pl_stream
*/









/*
class pl_stream: public ioassignstream {
public:
    pl_stream(const pl_stream &in_out)
      : ioassignstream(in_out), user_flags_in_(in_out.user_flags_in_), pl_stream_flags_in(in_out.pl_stream_flags_in), 
      user_flags_out_(in_out.user_flags_out_), pl_stream_flags_out(in_out.pl_stream_flags_out)
        {
	}
    pl_stream(const iostream &in_out = nullinout)
        : ioassignstream(in_out), user_flags_in_(dft_option_flag), pl_stream_flags_in(0), user_flags_out_(dft_option_flag), pl_stream_flags_out(0)
        {
	}

    pl_stream &operator() (iostream &in_out)
        { ioassignstream::operator=(in_out); return *this; };


// Implementation of read operators
//

  pl_stream& operator>>(bool &x)
  {
    rawin() >> x;
    if(!inmode==PStream::raw_ascii) rawin() >> ws;
    return *this;
  }
  
  pl_stream& operator>>(float &x)
  {
    rawin() >> x;
    if(!inmode==PStream::raw_ascii) rawin() >> ws;
    return *this;
  }

  pl_stream& operator>>(double &x)
  {
    rawin() >> x;
    if(!inmode==PStream::raw_ascii) rawin() >> ws;
    return *this;
  }
  
pl_stream& operator>>(char * &x)
  {
    rawin() >> x;
    if(!inmode==PStream::raw_ascii) rawin() >> ws;
    return *this;
  }
  
pl_stream& operator>>(string &x)
  {
    rawin() >> x;
    if(!inmode==PStream::raw_ascii) rawin() >> ws;
    return *this;
  }
  

// Implementation of write operators
// 

  pl_stream& operator<<(float x)
  {
    PLERROR("Not yet implemented");
    return *this;
  }

  pl_stream& operator<<(double x)
  {
    rawout() << x; 
    if(!outmode==PStream::raw_ascii) rawout() << ' ';
    return *this;
  }
  
  pl_stream& operator<<(const char *x)
  {
    rawout() << x;
    if(!outmode==PStream::raw_ascii) rawout() << ' ';
    return *this;
  }

  pl_stream& operator<<(const string &x)
  {
    rawout() << x;
    if(!outmode==PStream::raw_ascii) rawout() << ' ';
    return *this;
  }
  
  
  istream &
  operator>>(pl_stream_raw raw_)
  { return static_cast<istream &>(*this); };
  
  istream& rawin() { return static_cast<istream &>(*this); }
  ostream& rawout() { return static_cast<ostream &>(*this); }
  iostream& rawio() { return static_cast<iostream &>(*this); }
  
  ostream &
  operator<<(const pl_stream_raw raw_)
  { return static_cast<ostream &>(*this); };
  
  pl_stream &
  operator>>(pl_stream_clear_flags &flags_)
  { user_flags_in_ = 0; return *this; };
  
  pl_stream &
  operator<<(const pl_stream_clear_flags &flags_)
  { user_flags_out_ = 0; return *this; };
  
  pl_stream &
  operator>>(pl_flags flag)
  { pl_stream_flags_in.set(flag) = true; return *this; };
  
  pl_stream &
  operator<<(const pl_flags flag)
  { pl_stream_flags_out.set(flag) = true; return *this; };
  
  pl_stream &
  operator>>(pl_stream_user_flags &flags_)
  { user_flags_in_ = flags_.flags; return *this; };
  
  pl_stream &
  operator<<(const pl_stream_user_flags &flags_)
  { user_flags_out_ = flags_.flags; return *this; };
  
  pl_stream &
  operator>>(pl_stream_initiate &initiate_)
  { map_.clear(); return *this; };
  
  pl_stream &
  operator<<(const pl_stream_initiate &initiate_)
  { map_.clear(); return *this; };
  
  typedef pl_stream & (*pl_stream_manip)(pl_stream &);
  
  pl_stream &
  operator>>(pl_stream_manip func)
  { return (*func)(*this); };
  
  pl_stream &
  operator<<(pl_stream_manip func)
  { return (*func)(*this); };
  
  pl_stream &
  operator>>(pl_istream_manip_compat func)
  { (*func)(*this); return *this; }
  
  pl_stream &
  operator<<(pl_ostream_manip_compat func)
  { 
    (*func)(*this); 
    return *this; 
  };


  
  void setf_in(pl_flags flag) { pl_stream_flags_in.set(flag) = true; }
  void setf_out(const pl_flags flag) { pl_stream_flags_out.set(flag) = true; }
  
  
    OBflag_t user_flags_in_; 
    bitset<32> pl_stream_flags_in;
    OBflag_t user_flags_out_; 
    bitset<32> pl_stream_flags_out;
    map<unsigned int, void *> map_;
protected:
private:
}; // class pl_stream


*/



//typedef PStream pl_stream;




// Default implementations
//
/*
template <class T> inline pl_istream &
operator>>(pl_istream &in, T &x)
{
  in.rawin() >> x;
  return in;
}
*/
/*
template <class T> inline pl_istream &
operator>>(pl_istream &in, T * &x)
{
    if (in.peek() == '*') {
        in.get(); // Eat '*'
        unsigned int id;
        in >> id;
        if (in.peek() == '-') {
            in.get(); // Eat '-'
            in.get(); // Eat '>'
            in >> ws;
            if (in.peek() == '<') { // Not for the heartfainted
                static char *null_string = "<null>";
                for (int i = 0; i < 6; ++i)
                    if (in.get() != null_string[i])
                        PLERROR("Bad input");
                x = 0;
            } else {
                if (!x)
                    x = new T();
                in >> *x;
            }
            in.map_[id] = x;
        } else {
            // Find it in map and return ptr;
            map<unsigned int, void *>::iterator it = in.copies_map_in.find(id);
            if (it == in.copies_map_in.end())
                PLERROR("Type to be read has not been previously defined");
            x = static_cast<T *>(it->second);
        }
    } else
        in >> *x;

    return in;
}
*/
/*
template <class T> inline pl_ostream &
operator<<(pl_ostream &out, const T &x)
{ out.rawout() << x; return out; };
*/
/*
template <class T> inline pl_ostream &
operator<<(pl_ostream &out, const T * &x)
{
    map<void *, unsigned int>::iterator it = out.copies_map_out.find(const_cast<T * &>(x));
    if (it == out.copies_map_out.end()) {
        int id = out.copies_map_out.size();
        out << raw << '*' << id << "->";
        out.copies_map_out[const_cast<T * &>(x)] = id;
        if (x)
            out << *x;
        else
            out.rawout() << "<null>";
    } else {
        out.rawout() << '*' << it->second << ' ';
    }
    return out;
}
*/
// Specialized implementations
//
/*
inline istream &
operator>>(pl_istream &in, pl_stream_raw raw_)
{ return static_cast<istream &>(in); };

inline ostream &
operator<<(pl_ostream &out, const pl_stream_raw raw_)
{ return static_cast<ostream &>(out); };

inline pl_istream &
operator>>(pl_istream &in, pl_stream_clear_flags &flags_)
{ in.user_flags_ = 0; return in; };

inline pl_ostream &
operator<<(pl_ostream &out, const pl_stream_clear_flags &flags_)
{ out.user_flags_ = 0; return out; };

inline pl_istream &
operator>>(pl_istream &in, pl_flags flag)
{ in.pl_stream_flags.set(flag) = true; return in; };

inline pl_ostream &
operator<<(pl_ostream &out, const pl_flags flag)
{ out.pl_stream_flags.set(flag) = true; return out; };

inline pl_istream &
operator>>(pl_istream &in, pl_stream_user_flags &flags_)
{ in.user_flags_ = flags_.flags; return in; };

inline pl_ostream &
operator<<(pl_ostream &out, const pl_stream_user_flags &flags_)
{ out.user_flags_ = flags_.flags; return out; };

inline pl_istream &
operator>>(pl_istream &in, pl_stream_initiate &initiate_)
{ in.map_.clear(); return in; };

inline pl_ostream &
operator<<(pl_ostream &out, const pl_stream_initiate &initiate_)
{ out.map_.clear(); return out; };

inline pl_istream &
operator>>(pl_istream &in, pl_istream_manip func)
{ return (*func)(in); };

inline pl_ostream &
operator<<(pl_ostream &out, pl_ostream_manip func)
{ return (*func)(out); };

inline pl_istream &
operator>>(pl_istream &in, pl_istream_manip_compat func)
{ (*func)(in); return in; }

inline pl_ostream &
operator<<(pl_ostream &out, pl_ostream_manip_compat func)
{ (*func)(out); return out; };

*/


/***********************************************************************************************************************************************************/

// Specialized implementations
//
/*
inline istream &
operator>>(pl_stream &in, pl_stream_raw raw_)
{ return static_cast<istream &>(in); };

inline ostream &
operator<<(pl_stream &out, const pl_stream_raw raw_)
{ return static_cast<ostream &>(out); };

inline pl_stream &
operator>>(pl_stream &in, pl_stream_clear_flags &flags_)
{ in.user_flags_in_ = 0; return in; };

inline pl_stream &
operator<<(pl_stream &out, const pl_stream_clear_flags &flags_)
{ out.user_flags_out_ = 0; return out; };

inline pl_stream &
operator>>(pl_stream &in, pl_flags flag)
{ in.pl_stream_flags_in.set(flag) = true; return in; };

inline pl_stream &
operator<<(pl_stream &out, const pl_flags flag)
{ out.pl_stream_flags_out.set(flag) = true; return out; };

inline pl_stream &
operator>>(pl_stream &in, pl_stream_user_flags &flags_)
{ in.user_flags_in_ = flags_.flags; return in; };

inline pl_stream &
operator<<(pl_stream &out, const pl_stream_user_flags &flags_)
{ out.user_flags_out_ = flags_.flags; return out; };

inline pl_stream &
operator>>(pl_stream &in, pl_stream_initiate &initiate_)
{ in.map_.clear(); return in; };

inline pl_stream &
operator<<(pl_stream &out, const pl_stream_initiate &initiate_)
{ out.map_.clear(); return out; };

typedef pl_stream & (*pl_stream_manip)(pl_stream &);

inline pl_stream &
operator>>(pl_stream &in, pl_stream_manip func)
{ return (*func)(in); };

inline pl_stream &
operator<<(pl_stream &out, pl_stream_manip func)
{ return (*func)(out); };

inline pl_stream &
operator>>(pl_stream &in, pl_istream_manip_compat func)
{ (*func)(in); return in; }

inline pl_stream &
operator<<(pl_stream &out, pl_istream_manip_compat func)
{ (*func)(out); return out; };
*/
/******************************************************************************************************************************/

/*
template <class T> inline pl_stream &
operator>>(pl_stream &in_out, T &o)
{ 
  in_out.istr >> o; 
  in_out.clear(in_out.istr.rdstate());  
  return in_out; 
};

template <class T> inline pl_stream &
operator<<(pl_stream &in_out, const T &o)
{ 
  in_out.ostr << o; 
  in_out.clear(in_out.ostr.rdstate()); 
  return in_out; 
};
*/

%> // end of namespace PLearn

//#include "PStream.h"

#endif //ndef plstreams_INC
