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
   * $Id: plstreams.h,v 1.3 2002/08/07 01:49:49 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

// This file contains additional C++ streams definitions
// that are used in the PLearn Library


/*! \file PLearnLibrary/PLearnCore/plstreams.h */

#ifndef pstreams_INC
#define pstreams_INC

#include <fstream>
#include <map>
#include <bitset>
#include <strstream.h>
#include "plerror.h"

namespace PLearn <%
using namespace std;


extern ofstream& nullout(); //!<  a null ostream: writing to it does nothing
extern ifstream& nullin(); //!<  a null instream: reading from it does nothing
extern fstream& nullinout(); //!< a null iostream: reading/writing from/to it does nothing
//!  input streams that can be assigned to.
class iassignstream : public istream 
{
public:
  //!  iassignstream(): istream() {}
  iassignstream(const istream& in=nullin()): istream(in.rdbuf()) {}
  iassignstream(const iassignstream& in=nullin()): istream(in.rdbuf()) {}
  void operator=(const istream& in) { rdbuf(in.rdbuf()); }
  void operator=(const iassignstream& in) { rdbuf(in.rdbuf()); }
};

//!  output streams that can be assigned to.
class oassignstream : public ostream 
{
public:
  //!  oassignstream(): ostream() {}
  oassignstream(const ostream& out=nullout()): ostream(out.rdbuf()) {}
  oassignstream(const oassignstream& out): ostream(out.rdbuf()) {}
  void operator=(const ostream& out) { rdbuf(out.rdbuf()); }
  void operator=(const oassignstream& out) { rdbuf(out.rdbuf()); }
};

//! input/output streams that can be assigned to.
class ioassignstream : public iostream
{
public:
  ioassignstream(const iostream &stream=nullinout()): iostream(stream.rdbuf()) {};
  ioassignstream(const ioassignstream &stream): iostream(stream.rdbuf()) {};
  void operator=(const iostream &stream) { rdbuf(stream.rdbuf()); };
  void operator=(const ioassignstream &stream) { rdbuf(stream.rdbuf()); }; 
};





//!the following typedef should be equivalent to OptionBase::flag_t
typedef unsigned int OBflag_t;
const OBflag_t dft_option_flag= 1 | 1<<1 | 1<<2;//!< buildoption | learntoption | tuningoption


enum pl_flags { plf_plain,
                plf_binary,
                plf_swap_endian,
                plf_shorts_as_doubles,
                plf_doubles_as_shorts
              };

// For use in bitset<32> constructor ONLY!
#define PLF_PLAIN    ( 1 << (plf_plain) )
#define PLF_BINARY   ( 1 << (plf_binary) )

class pl_istream: public iassignstream {
 public:
    pl_istream(const pl_istream &in)
        : iassignstream(in), user_flags_(in.user_flags_), flags(in.flags)
        {};
    pl_istream(const istream &in = nullin())
        : iassignstream(in), user_flags_(dft_option_flag), flags(0)
        {};

    pl_istream &operator() (istream &in)
        { iassignstream::operator=(in); return *this; };

    OBflag_t user_flags_; 
    bitset<32> flags;
    map<unsigned int, void *> map_;
protected:
 private:
}; // class pl_istream


class pl_ostream: public oassignstream {
 public:
    pl_ostream(const pl_ostream &out)
        : oassignstream(out), user_flags_(out.user_flags_), flags(out.flags)
        {};
    pl_ostream(const ostream &out = nullout())
        : oassignstream(out), user_flags_(dft_option_flag), flags(0)
        {};

    pl_ostream &operator() (ostream &out)
        { oassignstream::operator=(out); return *this; };

    OBflag_t user_flags_;
    bitset<32> flags;
    map<void *, unsigned int> map_;
 protected:
 private:
}; // class pl_ostream

class pl_stream: public ioassignstream {
public:
    pl_stream(const pl_stream &in_out)
        : ioassignstream(in_out), ostr(in_out.ostr), istr(in_out.istr)
        {};
    pl_stream(const iostream &in_out = nullinout())
        : ioassignstream(in_out), ostr(in_out), istr(in_out)
        {};

    pl_stream &operator() (iostream &in_out)
        { ioassignstream::operator=(in_out); ostr(in_out); istr(in_out); return *this; };

    pl_ostream ostr;
    pl_istream istr;
protected:
private:
}; // class pl_stream


// Define some useful shortcuts
class pl_stream_raw {};
class pl_stream_clear_flags {};
class pl_stream_user_flags {
public:
    OBflag_t flags;
    pl_stream_user_flags &operator()(OBflag_t flags_)
        { flags = flags_; return *this; };
};
class pl_stream_initiate {};

extern pl_stream_raw raw;
extern pl_stream_clear_flags clear_flags;
extern pl_stream_user_flags user_flags;
extern pl_stream_initiate initiate;

// Default implementations
//

template <class T> inline pl_istream &
operator>>(pl_istream &in, T &x)
{
    return in;
}

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
            map<unsigned int, void *>::iterator it = in.map_.find(id);
            if (it == in.map_.end())
                PLERROR("Type to be read has not been previously defined");
            x = static_cast<T *>(it->second);
        }
    } else
        in >> *x;

    return in;
}

template <class T> inline pl_ostream &
operator<<(pl_ostream &out, const T &x)
{ out << raw << x; return out; };

template <class T> inline pl_ostream &
operator<<(pl_ostream &out, const T * &x)
{
    map<void *, unsigned int>::iterator it = out.map_.find(const_cast<T * &>(x));
    if (it == out.map_.end()) {
        int id = out.map_.size();
        out << raw << '*' << id << "->";
        out.map_[const_cast<T * &>(x)] = id;
        if (x)
            out << *x;
        else
            out << raw << "<null>";
    } else {
        out << raw << '*' << it->second << ' ';
    }
    return out;
}

// Specialized implementations
//

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
{ in.flags.set(flag) = true; return in; };

inline pl_ostream &
operator<<(pl_ostream &out, const pl_flags flag)
{ out.flags.set(flag) = true; return out; };

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

typedef pl_istream & (*pl_istream_manip)(pl_istream &);
typedef pl_ostream & (*pl_ostream_manip)(pl_ostream &);
typedef istream & (*pl_istream_manip_compat)(istream &);
typedef ostream & (*pl_ostream_manip_compat)(ostream &);

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

template <class T> inline pl_stream &
operator>>(pl_stream &in_out, T &o)
{ in_out.istr >> o; return in_out; };

template <class T> inline pl_stream &
operator<<(pl_stream &in_out, const T &o)
{ in_out.ostr << o; return in_out; };


// Default behavior for write() and read() is
// to call corresponding operator<<() or operator>>()
// on pl_{o|i}stream.

template <class T> inline void
write(ostream &out_, T &o, OBflag_t flags = dft_option_flag)
{
    pl_ostream out = out_;
    out << user_flags(flags) << o;
}

template <class T> inline void
read(istream &in_, T &o, OBflag_t flags = dft_option_flag)
{
    pl_istream in = in_;
    in >> user_flags(flags) >> o;
}

template <class T> inline void
read(const string &stringval, T &x)
{
    istrstream in_(stringval.c_str());
    pl_istream in(in_);
    in >> x;
}

%> // end of namespace PLearn

#endif
