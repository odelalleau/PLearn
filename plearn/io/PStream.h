// -*- C++ -*-

// PStream.h
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio and
//  University of Montreal
// Copyright (C) 2002 Frederic Morin, Xavier Saint-Mleux, Pascal Vincent
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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

//!The size of the temporary buffer to use inside some class function.
//!Putting the buffer as a class variable don't give any speedup.
//!So to remove threads issue, we put them as function variable.
#define PSTREAM_BUF_SIZE 100

#include <map>
#include <set>
#include <sstream>
#include <fstream>
#include <limits>
#include <plearn/base/byte_order.h>
#include <plearn/base/pl_hash_fun.h>
#include <plearn/base/plerror.h>
#include <plearn/base/pl_stdint.h>
#include "PStream_util.h"
#include "PStreamBuf.h"
#include "StdPStreamBuf.h"

namespace PLearn {

using namespace std;


// for readBinaryNumAs
template<bool x> struct chkUnsigned 
{ template<class I> static bool notOk(I& y) { return false; } };
template<> struct chkUnsigned<true> 
{ template<class I> static bool notOk(I& y) { return y<0; } };


/*!
 * PStream:
 *  This class defines a type of stream that should be used for all I/O within
 *  PLearn.
 *  It supports most operations available for standard C++ streams, plus:
 *   - a set of option flags that indicate which types of Object Options should
 *     be read/written (option_flags_{in|out}; has effect only for Object's);
 *   - a set of mode flags that define format used for I/O. e.g.:
 *      "raw_ascii" for  standard C++ stream behaviour,
 *      "plearn_ascii"  for human-readable serialization format,
 *      etc. (inmode and outmode);
 *   - a copies map to allow smart serialization of pointers;
 *   - a markable stream buffer which allows to 'seek back' to a previously set
 *     mark on any type of istream;
 *   - an 'attach' function to attach the stream to a POSIX file descriptor.
 */

class PStream : public PP<PStreamBuf>
{
public:
    //! typedef's for PStream manipulators
    typedef PStream& (*pl_pstream_manip)(PStream&);

private:

    typedef PP<PStreamBuf> inherited;
    typedef PStreamBuf streambuftype;

public:

// norman: check for win32
#if __GNUC__ < 3 && !defined(WIN32)
    //! typedef's for compatibility w/ old libraries
    typedef int streamsize;
    typedef ios ios_base;
#endif

    /**
     *  plearn_ascii and plearn_binary are used on output to determine in which
     *  format to write stuff.  On input however, they are equivalent, as the
     *  right format is automatically detected.
     */
    enum mode_t
    {
        //! PLearn ascii serialization format (can be mixed with plearn_binary)
        plearn_ascii,
        //! PLearn binary serialization format (can be mixed with plearn_ascii)
        plearn_binary,
        //! Raw C++ ascii output without additional separators
        //! (direct output to underlying ostream)
        raw_ascii,
        //! Simply writes the bytes as they are in memory.
        raw_binary,
        //! Ascii pretty print (in particular for Vec and Mat, formatted output
        //! without size info)
        pretty_ascii
    };


    //! Compression mode (mostly used by binary serialization of sequences of
    //! floats or doubles, such as TMat<real>)
    //! (Used on output only; autodetect on read).
    enum compr_mode_t {
        //! No compression.
        compr_none,
        //! In plearn_binary mode, store doubles as float
        compr_double_as_float,
        //! PLearn
        compr_sparse,
        //! Also stores double as float
        compr_lossy_sparse
    };

public:
    //! mode for input formatting
    mode_t inmode;

    //! format flags for input
    // bitset<32> pl_stream_flags_in;

    //! copies map for input
    map<unsigned int, void *> copies_map_in;

    //! mode for output formatting
    mode_t outmode;

    //! format flags for output
    // bitset<32> pl_stream_flags_out;

    //! copies map for output
    map<void *, unsigned int> copies_map_out;

private:
    //! Current format string for floats
    const char* format_float;

    //! Current format string for doubles
    const char* format_double;

    //! Default format string for floats
    static const char* format_float_default;

    //! Default format string for doubles
    static const char* format_double_default;

public:
    //! If true, then Mat and Vec will be serialized with their elements in
    //! place,
    //! If false, they will have an explicit pointer to a storage
    bool implicit_storage;

    //! Determines the way data is compressed, if any.
    compr_mode_t compression_mode;

    //! Should be true if this stream is used to communicate with a remote
    //! PLearn host.  Will serialize options accordingly.
    bool remote_plearn_comm;
    
    //! If true, we should emit windows end of line(\r\n)
    static bool windows_endl;
public:

    PStream();

    //! constructor from a PStreamBuf
    PStream(streambuftype* sb);

    //! ctor. from an istream (I)
    PStream(istream* pin_, bool own_pin_=false);

    //! ctor. from an ostream (O)
    PStream(ostream* pout_, bool own_pout_=false);

    //! ctor. from an iostream (IO)
    PStream(iostream* pios_, bool own_pios_=false);

    //! ctor. from an istream and an ostream (IO)
    PStream(istream* pin_, ostream* pout_, bool own_pin_=false,
            bool own_pout_=false);

    //! Default copy ctor. should be fine now.

    //! Destructor.
    virtual ~PStream();

    inline void setBufferCapacities(streamsize inbuf_capacity,
                                    streamsize outbuf_capacity,
                                    streamsize unget_capacity)
    {
        ptr->setBufferCapacities(inbuf_capacity,
                                 outbuf_capacity,
                                 unget_capacity);
    }

    inline mode_t setInMode(mode_t m)
    {
        mode_t oldmode = inmode;
        inmode = m;
        return oldmode;
    }

    inline mode_t setOutMode(mode_t m)
    {
        mode_t oldmode = outmode;
        outmode = m;
        return oldmode;
    }

    inline void setMode(mode_t m)
    { inmode = m; outmode = m; }

    inline void clearOutMap()
    { copies_map_out.clear(); }

    inline void clearInMap()
    { copies_map_in.clear(); }

    inline void clearInOutMaps()
    {
        clearInMap();
        clearOutMap();
    }

    //! if outmode is raw_ascii or raw_binary, it will be switched to
    //! corresponding plearn_ascii, resp. plearn_binary.
    //! The old mode will be returned, so that you can call setOutMode
    //! to revert to the old mode when finished
    mode_t switchToPLearnOutMode();

    PStream& operator>>(mode_t m)
    {
        inmode = m;
        return *this;
    }

    PStream& operator<<(mode_t m)
    {
        outmode = m;
        return *this;
    }

    //! Parse a string and return the equivalent PStream::mode_t.
    static PStream::mode_t parseModeT(const string& str);

public:
    //op()'s: re-init with different underlying stream(s)

    PStream& operator=(const PStream& pios);

    PStream& operator=(streambuftype* streambuf)
    {
        inherited::operator=(streambuf);
        return *this;
    }

    bool operator==(const PStream& other)
    { return PP<PStreamBuf>::operator==(other); }


    void writeAsciiNum(char x);
    void writeAsciiNum(unsigned char x);
    void writeAsciiNum(signed char x);
    void writeAsciiNum(short x);
    void writeAsciiNum(unsigned short x);
    void writeAsciiNum(int x);
    void writeAsciiNum(unsigned int x);
    void writeAsciiNum(long x);
    void writeAsciiNum(unsigned long x);
    void writeAsciiNum(long long x);
    void writeAsciiNum(unsigned long long x);
    void writeAsciiNum(float x);
    void writeAsciiNum(double x);

    void readAsciiNum(char &x);
    void readAsciiNum(unsigned char &x);
    void readAsciiNum(signed char &x);
    void readAsciiNum(short &x);
    void readAsciiNum(unsigned short &x);
    void readAsciiNum(int &x);
    void readAsciiNum(unsigned int &x);
    void readAsciiNum(long &x);
    void readAsciiNum(unsigned long &x);
    void readAsciiNum(long long &x);
    void readAsciiNum(unsigned long long &x);
    void readAsciiNum(float &x);
    void readAsciiNum(double &x);

    //! Writes the corresponding 2 hex digits (ex: 0A )
    void writeAsciiHexNum(unsigned char x);

    //! Writes base types to PLearn binary format
    //! TODO: forbid this mechanism for arbitrary classes?
    template<class I>
    void writeBinaryNum(I x)
    {
#ifdef BIGENDIAN
        put(TypeTraits<I>::big_endian_typecode());
#else
        put(TypeTraits<I>::little_endian_typecode());
#endif
        write((char*)&x, sizeof(I));
    }

    //! Reads a J element, optionnally swaps its endianness, and returns an I
    template<class I, class J>
    void readBinaryNumAs(J& x, bool inverted_byte_order)
    {
        I y;
        read(reinterpret_cast<char*>(&y), sizeof(I));
        if (inverted_byte_order)
            endianswap(&y);
#ifdef __INTEL_COMPILER
#pragma warning(disable:1682)
// Yes, I know that "implicit conversion of a 64-bit integral type to a smaller
// integral type (potential portability problem)", but the conversion is
// explicit here.
#endif
        x = static_cast<J>(y);

        // Check if there was a conversion problem (sign or overflow)

        if (static_cast<I>(x) != y
            || (!(numeric_limits<J>::is_signed) 
                && chkUnsigned<numeric_limits<I>::is_signed>::notOk(y)))
        {
            std::stringstream error;
            error << "In PStream::readBinaryNumAs, overflow error: " << std::endl
                << "unable to read value \"" << y << "\""
                << "into a " << TypeTraits<I>::name() << std::endl;

            PLERROR( error.str().c_str() );
        }
#ifdef __INTEL_COMPILER
#pragma warning(default:1682)
#endif
    }

    //! Reads base types from PLearn binary format
    template<class I>
    void readBinaryNum(I &x, unsigned char typecode)
    {
        // If typecode corresponds to I's typecode
        if (typecode==TypeTraits<I>::little_endian_typecode())
        {
            read(reinterpret_cast<char*>(&x), sizeof(I));
#ifdef BIGENDIAN
            endianswap(&x);
#endif
        }
        else if (typecode==TypeTraits<I>::big_endian_typecode())
        {
            read(reinterpret_cast<char*>(&x), sizeof(I));
#ifdef LITTLEENDIAN
            endianswap(&x);
#endif
        }
        // Else, we need to try all the different types
        else if (typecode==TypeTraits<int8_t>::little_endian_typecode())
            readBinaryNumAs<int8_t>(x, false);
        else if (typecode==TypeTraits<uint8_t>::little_endian_typecode())
            readBinaryNumAs<uint8_t>(x, false);
        else if (typecode==TypeTraits<int16_t>::little_endian_typecode())
            readBinaryNumAs<int16_t>(x, byte_order()==BIG_ENDIAN_ORDER);
        else if (typecode==TypeTraits<int16_t>::big_endian_typecode())
            readBinaryNumAs<int16_t>(x, byte_order()==LITTLE_ENDIAN_ORDER);
        else if (typecode==TypeTraits<uint16_t>::little_endian_typecode())
            readBinaryNumAs<uint16_t>(x, byte_order()==BIG_ENDIAN_ORDER);
        else if (typecode==TypeTraits<uint16_t>::big_endian_typecode())
            readBinaryNumAs<uint16_t>(x, byte_order()==LITTLE_ENDIAN_ORDER);
        else if (typecode==TypeTraits<int32_t>::little_endian_typecode())
            readBinaryNumAs<int32_t>(x, byte_order()==BIG_ENDIAN_ORDER);
        else if (typecode==TypeTraits<int32_t>::big_endian_typecode())
            readBinaryNumAs<int32_t>(x, byte_order()==LITTLE_ENDIAN_ORDER);
        else if (typecode==TypeTraits<uint32_t>::little_endian_typecode())
            readBinaryNumAs<uint32_t>(x, byte_order()==BIG_ENDIAN_ORDER);
        else if (typecode==TypeTraits<uint32_t>::big_endian_typecode())
            readBinaryNumAs<uint32_t>(x, byte_order()==LITTLE_ENDIAN_ORDER);
        else if (typecode==TypeTraits<int64_t>::little_endian_typecode())
            readBinaryNumAs<int64_t>(x, byte_order()==BIG_ENDIAN_ORDER);
        else if (typecode==TypeTraits<int64_t>::big_endian_typecode())
            readBinaryNumAs<int64_t>(x, byte_order()==LITTLE_ENDIAN_ORDER);
        else if (typecode==TypeTraits<uint64_t>::little_endian_typecode())
            readBinaryNumAs<uint64_t>(x, byte_order()==BIG_ENDIAN_ORDER);
        else if (typecode==TypeTraits<uint64_t>::big_endian_typecode())
            readBinaryNumAs<uint64_t>(x, byte_order()==LITTLE_ENDIAN_ORDER);
        else if (typecode==TypeTraits<float>::little_endian_typecode())
            readBinaryNumAs<float>(x, byte_order()==BIG_ENDIAN_ORDER);
        else if (typecode==TypeTraits<float>::big_endian_typecode())
            readBinaryNumAs<float>(x, byte_order()==LITTLE_ENDIAN_ORDER);
        else if (typecode==TypeTraits<double>::little_endian_typecode())
            readBinaryNumAs<double>(x, byte_order()==BIG_ENDIAN_ORDER);
        else if (typecode==TypeTraits<double>::big_endian_typecode())
            readBinaryNumAs<double>(x, byte_order()==LITTLE_ENDIAN_ORDER);
        else
            PLERROR("In PStream readBinaryNum: Unknown typecode '%c' (%x)",
                    typecode, typecode);
    }

    inline bool eof() const
    { return ptr->eof(); }

    bool good() const
    { return ptr && ptr->good(); }

    /*****
     * DEPRECATED: this implicit conversion is too dangerous!
     *             use .good() instead.
    operator bool() const
    { return good(); }
    */
    /*****
     * operator void*() causes a compile error ON PURPOSE! (ambiguous w/ PP<T>::operator T*())
     * The implicit conversion of a PStream to a bool or pointer is
     * too dangerous to be allowed; please use PStream::good() (or PP<T>::isNotNull()) instead.
     * EXAMPLE of a flawed conversion to bool:
     *  enum Machin { Truc, Bidule };
     *  Machin mon_machin;
     *  pin >> mon_machin; // OOPS!!! converts pin to bool and applies operator>>(int,int), a shift... 
     */
    /** !!! **/ operator void*() const // DO NOT USE IT; FAILS ON PURPOSE!
                { PLERROR("!?! this should have failed at compile time !?!"); return 0; } 

    //! Test whether a PStream is in an invalid state.
    // This operator is required for compilation under Visual C++.
    bool operator !() const
    { return !good(); }

    /**
     *  Getters/Setters for the printf format strings for writeAsciiNum for
     *  float and double.  By default, this is "%.8g" for float, and "%.18g"
     *  for double.  NOTE: these strings must remain accessible for the
     *  lifetime of the PStream.  In particular, they (generally) should not
     *  come from a string c_str().
     */
    const char* getFloatFormat()  const { return format_float;  }
    const char* getDoubleFormat() const { return format_double; }
    void setFloatFormat(const char* f)  { format_float = f;  }
    void setDoubleFormat(const char* f) { format_double = f; }

    /**
     * The folowing methods are 'forwarded' from {i|o}stream.
     */
    inline int get()
    { return ptr->get(); }

    //! The folowing methods are 'forwarded' from {i|o}stream.
    inline PStream& get(char& c)
    {
        c = (char)ptr->get();
        return *this;
    }

    //! Delimitor is read from stream but not appended to string.
    inline PStream& getline(string& line, char delimitor='\n')
    {
        line.clear();
        int c = get();
        while (c != EOF && c != delimitor)
        {
            line += (char)c;
            c = get();
        }
        return *this;
    }

    inline string getline()
    { string s; getline(s); return s; }

    //! The folowing methods are 'forwarded' from {i|o}stream.
    //! It read from the stream without advancing in it.
    //! I.E. Multiple successive peek() will return the same data.
    inline int peek()
    { return ptr->peek(); }

    //! If you put back the result of a call to get(), make sure it is not EOF.
    inline PStream& putback(char c)
    {
        ptr->putback(c);
        return *this;
    }

    //! Put back the last character read by the get() or read() methods.
    //! You can only call this method once (use the unread() method if you want
    //! to put back more than one character).
    inline PStream& unget()
    {
        ptr->unget();
        return *this;
    }

    inline PStream& unread(const char* p, streamsize n)
    {
        ptr->unread(p,n);
        return *this;
    }

    inline PStream& unread(const char* p)
    { return unread(p, streamsize(strlen(p))); }

    inline PStream& unread(const string& s)
    { return unread(s.data(), streamsize(s.length())); }


    inline PStream& read(char* s, streamsize n)
    {
        ptr->read(s,n);
        return *this;
    }

    inline PStream& read(string& s, streamsize n)
    {
        char* buf = new char[n];
        string::size_type nread = ptr->read(buf, n);
        s.assign(buf, nread);
        delete[] buf;
        return *this;
    }

    //! Reads until an EOF is found, and returns the whole content as a string.
    string readAll();

    //! Reads the next character and launches a PLERROR if it is different from
    //! expect.
    void readExpected(char expect);

    //! Reads character one by one, comparing it with the sequence in expect
    //! (until terminating null character in expect).
    //! Throws a PLERROR as soon as the character read differs from the
    //! character expected.
    void readExpected(char* expect);

    //! Reads character one by one, comparing it with the sequence in expect.
    //! Throws a PLERROR as soon as the character read differs from the
    //! character expected.
    void readExpected(const string& expect);

    //! Reads characters into buf until n characters have been read, or
    //! end-of-file has been reached, or the next character in the stream is
    //! the stop_char.
    //! Returns the total number of characters put into buf.
    //! The stopping character met is not extracted from the stream.
    streamsize readUntil(char* buf, streamsize n, char stop_char);

    //! Reads characters into buf until n characters have been read, or
    //! end-of-file has been reached, or the next character in the stream is
    //! one of the stop_chars (null terminated string).
    //! Returns the total number of characters put into buf.
    //! The stopping character met is not extracted from the stream.
    streamsize readUntil(char* buf, streamsize n, const char* stop_chars);

    inline PStream& write(const char* s, streamsize n)
    {
        ptr->write(s,n);
        return *this;
    }

    inline PStream& put(char c)
    {
        ptr->put(c);
        return *this;
    }

    inline PStream& put(unsigned char c)
    {
        write(reinterpret_cast<char *>(&c), sizeof(c));
        return *this;
    }
    inline PStream& put(int x) { return put((char)x); }

    inline PStream& flush()
    {
        ptr->flush();
        return *this;
    }

    inline PStream& endl()
    {
        if(windows_endl)
            put('\r');
        put('\n');
        flush();
        return *this;
    }

    // These are convenient method for writing raw strings
    // (whatever the outmode):
    inline PStream& write(const char* s)
    {
        write(s, streamsize(strlen(s)));
        return *this;
    }

    inline PStream& write(const string& s)
    {
        write(s.data(), streamsize(s.length()));
        return *this;
    }

    // Useful skip functions

    //! reads everything until '\n' (also consumes the '\n')
    void skipRestOfLine();

    //! skips any blanks (space, tab, newline)
    void skipBlanks();

    //! skips any blanks (space, tab, newline) and comments starting with #
    void skipBlanksAndComments();

    //! skips any blanks, # comments, and separators (',' and ';')
    void skipBlanksAndCommentsAndSeparators();

    //! skips all occurences of any of the given characters
    void skipAll(const char* chars_to_skip);

    //! Reads characters from stream, until we meet one of the stopping symbols
    //! at the current "level". i.e. any opening parenthesis, bracket, brace or
    //! quote will open a next level and we'll be back to the current level
    //! only *after* we meet the corresponding closing parenthesis, bracket,
    //! brace or quote.
    //! All characters read, except the stoppingsymbol, will be *appended* to
    //! characters_read.
    //! The stoppingsymbol is read and returned, but not appended to
    //! characters_read.
    //! Comments starting with # until the end of line may be skipped (as if
    //! they were not part of the stream)
    int smartReadUntilNext(const string& stoppingsymbols,
                           string& characters_read,
                           bool ignore_brackets=false,
                           bool skip_comments=true);

    //! Count the number of occurrences of a character in the stream.
    int count(char c);

    // operator>>'s for base types
    PStream& operator>>(float &x);
    PStream& operator>>(double &x);
    PStream& operator>>(string &x);
    // read string in already allocated char[]
    PStream& operator>>(char* x);
    PStream& operator>>(char &x);
    PStream& operator>>(signed char &x);
    PStream& operator>>(unsigned char &x);
    PStream& operator>>(bool &x);
    PStream& operator>>(short &x);
    PStream& operator>>(unsigned short &x);
    PStream& operator>>(int &x);
    PStream& operator>>(unsigned int &x);
    PStream& operator>>(long &x);
    PStream& operator>>(unsigned long &x);
    PStream& operator>>(long long &x);
    PStream& operator>>(unsigned long long &x);
    PStream& operator>>(pl_pstream_manip func)
    { return (*func)(*this); }

    // operator<<'s for base types
    PStream& operator<<(float x);
    PStream& operator<<(double x);

    //! Warning: string output will be formatted according to outmode
    //! (if you want to output a raw string use the write method instead)
    PStream& operator<<(const char *x);

    //! Overload to print out raw pointers in the form 0x????????
    PStream& operator<<(const void *x);

    //! Warning: string output will be formatted according to outmode
    //! (if you want to output a raw string use the write method instead)
    //! (unless you're in raw_ascii or raw_binary mode!)
    PStream& operator<<(const string &x);

    PStream& operator<<(char x);
    PStream& operator<<(signed char x);
    PStream& operator<<(unsigned char x);

    // Note: If you get mysterious mesages of problems with const bool
    // resolutions, then a workaround might be to not declare <<(bool) as a
    // method, but as an inline function
    PStream& operator<<(bool x);
    PStream& operator<<(short x);
    PStream& operator<<(unsigned short x);
    PStream& operator<<(int x);
    PStream& operator<<(unsigned int x);
    PStream& operator<<(long x);
    PStream& operator<<(unsigned long x);
    PStream& operator<<(long long x);
    PStream& operator<<(unsigned long long x);
    PStream& operator<<(pl_pstream_manip func) { return (*func)(*this); }

};

/*! PStream objects to replace the standard cout, cin, ... */
PStream& get_pin();
PStream& get_pout();
PStream& get_pio();
PStream& get_perr();
PStream& get_pnull();


extern PStream pin;
extern PStream pout;
extern PStream pio;
extern PStream perr;
extern PStream pnull;

// Simulation of <<flush <<endl and >>ws ...

extern PStream& flush(PStream& out);
extern PStream& endl(PStream& out);
extern PStream& ws(PStream& out);

// But inject the standard ones as well to keep them usable!!!
using std::flush;
using std::endl;
using std::ws;

//!  returns the next line read from the stream,
//!  after removing any trailing '\\r' and/or '\\n'
string pgetline(PStream& in);


/*****
 * op>> & op<< for generic pointers
 */

template <class T>
inline PStream& operator>>(PStream& in, T*& x)
{

    in.skipBlanksAndCommentsAndSeparators();
    if (in.peek() == '*')
    {
        in.get(); // Eat '*'
        unsigned int id;
        in >> id;
        // don't skip blanks before we need to read something else
        // (read might block).
        //in.skipBlanksAndCommentsAndSeparators();
        if (id==0)
            x = 0;
        else
        {
            in.skipBlanksAndCommentsAndSeparators();
            if (in.peek() == '-')
            {
                in.get(); // Eat '-'
                char cc = in.get();
                if(cc != '>') // Eat '>'
                    PLERROR("In PStream::operator>>(T*&)  Wrong format.  Expecting \"*%d->\" but got \"*%d-%c\".", id, id, cc);
                // don't skip blanks before we need to read something else
                // (read might block).
                //in.skipBlanksAndCommentsAndSeparators();
                if(!x)
                    x = new T();
                in.skipBlanksAndCommentsAndSeparators();
                in >> *x;
                // don't skip blanks before we need to read something else
                // (read might block).
                //in.skipBlanksAndCommentsAndSeparators();
                in.copies_map_in[id]= x;
            }
            else
            {
                // Find it in map and return ptr;
                map<unsigned int, void *>::iterator it =
                    in.copies_map_in.find(id);
                if (it == in.copies_map_in.end())
                    PLERROR("In PStream::operator>>(T*&) object (ptr) to be read with id='%d' "
                            "has not been previously defined", id);
                x = static_cast<T *>(it->second);
            }
        }
    }
    else
    {
        in >> *x;
        // don't skip blanks before we need to read something else
        // (read might block).
        //in.skipBlanksAndCommentsAndSeparators();
    }

    return in;
}


template <class T>
inline PStream& operator<<(PStream& out, T const * const & x)
{
    if(x)
    {
        map<void *, unsigned int>::iterator it =
            out.copies_map_out.find(const_cast<T*&>(x));
        if (it == out.copies_map_out.end())
        {
            int id = (int)out.copies_map_out.size()+1;
            out.put('*');
            out << id;
            out.write("->");
            out.copies_map_out[const_cast<T*&>(x)] = id;
            out << *x;
        }
        else
        {
            out.put('*');
            out << it->second;
            out.put(' ');
        }
    }
    else
        out.write("*0 ");
    return out;
}

template <class T>
inline PStream& operator>>(PStream& in, const T*& x)
{
    PLERROR("operator>>(PStream&, const T*&) should never be used! (object pointed is const)");
    return in;
}

template <class T>
inline PStream& operator>>(PStream& in, PP<T> &o)
{
    T *ptr;
    if (o.isNull())
        ptr = 0;
    else
        ptr = o;
    in >> ptr;
    o = ptr;
    return in;
}

template <class T>
inline PStream& operator<<(PStream& out, const PP<T> &o)
{
    T *ptr = static_cast<T *>(o);
    out << const_cast<const T * &>(ptr);
    return out;
}

template <class T>
inline PStream& operator<<(PStream& out, T*& ptr)
{
    out << const_cast<T const * const &>(ptr);
    return out;
}


// Serialization of pairs in the form:
// first : second

template<class A,class B>
inline PStream& operator<<(PStream& out, const pair<A,B>& x)
{
    // new format (same as for tuple)
    out.put('(');
    out << x.first;
    out.write(", ");
    out << x.second;
    out.put(')');

#if 0
    // old deprecated format
    switch(out.outmode)
    {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
    case PStream::plearn_ascii:
        out << x.first;
        out.write(": ");
        out << x.second;
        out.put(' ');
        break;
    case PStream::plearn_binary:
        out.put((char)0x16);
        out << x.first << x.second;
        break;
    default:
        PLERROR("PStream mode not supported");
    }
#endif

    return out;
}

template <typename S, typename T>
inline PStream& operator>>(PStream& in, pair<S, T> &x)
{
    in.skipBlanksAndCommentsAndSeparators();
    int c = in.peek();
    if(c==0x16) // binary pair
    {
        in.get(); // eat the header byte
        in >> x.first >> x.second;
    }
    else if(c=='(') // it's the parenthesized (first, second) format
    {
        in.get();
        in.skipBlanksAndComments();
        in >> x.first;
        in.skipBlanksAndCommentsAndSeparators();
        in >> x.second;
        in.skipBlanksAndComments();
        in.readExpected(')');
    }
    else // suppose it's ascii, separated by :
    {
        in >> x.first;
        in.skipBlanksAndComments();
        if(in.get()!=':'){
            //we can't use NORMAL_LOG as can't include pl_log.h
            // can't include pl_log.h as it include PStream.h 
            // and PStream.h would include pl_log.h(recursive include)
            //idem for tostring.h
            perr<<"For the following error, the first half of the pair is '"<<x.first<<"'"<<endl;
            PLERROR("In operator>>(PStream& in, pair<S, T> &x) expected ':' to separate the 2 halves of the pair");
        }
        in.skipBlanksAndComments();
        in >> x.second;
    }
    return in;
}


// Serialization of map types

template<class MapT>
void writeMap(PStream& out, const MapT& m)
{
    typename MapT::const_iterator it = m.begin();
    typename MapT::const_iterator itend = m.end();

    // PStream::mode_t curmode = out.switchToPLearnOutMode();

    out.put('{');
    if(!m.empty())
    {
        // write the first item
        out << it->first;
        out.write(": ");
        out << it->second;
        ++it;
        while(it!=itend)
        {
            out.write(", ");
            out << it->first;
            out.write(": ");
            out << it->second;
            ++it;
        }
    }
    out.put('}');

    // out.setOutMode(curmode);
}

template<class MapT>
void readMap(PStream& in, MapT& m)
{
    m.clear();
    in.skipBlanksAndCommentsAndSeparators();
    int c = in.get();
    if(c!='{')
        PLERROR("In readMap(Pstream& in, MapT& m) expected '{' but read %c",c);
    in.skipBlanksAndCommentsAndSeparators();
    c = in.peek(); // do we have a '}' ?
    while(c!='}')
    {
        pair<typename MapT::key_type, typename MapT::mapped_type> val;
        in >> val.first;
        in.skipBlanksAndCommentsAndSeparators();
        c = in.get();
        if(c!=':')
            PLERROR("In readMap(Pstream& in, MapT& m) separator between key and value must be ':', but I read a '%c'",c);
        in.skipBlanksAndCommentsAndSeparators();
        in >> val.second;
        m.insert(val);
        in.skipBlanksAndCommentsAndSeparators();
        c = in.peek(); // do we have a '}' ?
    }
    in.get(); // eat the '}'
}

inline PStream& operator>>(PStream& in, CopiesMap&)
{
    PLERROR("PLearn::CopiesMap cannot be serialized.");
    return in;
}
inline PStream& operator<<(PStream& out, const CopiesMap&)
{
    PLERROR("PLearn::CopiesMap cannot be [un]serialized.");
    return out;
}

template<class Key, class Value, class Compare, class Alloc>
inline PStream& operator<<(PStream& out,
                           const map<Key, Value, Compare, Alloc>& m)
{
    writeMap(out, m);
    return out;
}

template<class Key, class Value, class Compare, class Alloc>
inline PStream& operator>>(PStream& in, map<Key, Value, Compare, Alloc>& m)
{
    readMap(in, m);
    return in;
}

template<class Key, class Value, class Compare, class Alloc>
inline PStream& operator<<(PStream& out,
                           const multimap<Key, Value, Compare, Alloc>& m)
{
    writeMap(out, m);
    return out;
}

template<class Key, class Value, class Compare, class Alloc>
inline PStream& operator>>(PStream& in,
                           multimap<Key, Value, Compare, Alloc>& m)
{
    readMap(in, m);
    return in;
}


template<class Key, class Value, class Compare, class Alloc>
inline PStream& operator<<(PStream& out,
                           const hash_map<Key, Value, Compare, Alloc>& m)
{
    writeMap(out, m);
    return out;
}

template<class Key, class Value, class Compare, class Alloc>
inline PStream& operator>>(PStream& in,
                           hash_map<Key, Value, Compare, Alloc>& m)
{
    readMap(in, m);
    return in;
}

template<class Key, class Value, class Compare, class Alloc>
inline PStream& operator<<(PStream& out,
                           const hash_multimap<Key, Value, Compare, Alloc>& m)
{
    writeMap(out, m);
    return out;
}

template<class Key, class Value, class Compare, class Alloc>
inline PStream& operator>>(PStream& in,
                           hash_multimap<Key, Value, Compare, Alloc>& m)
{
    readMap(in, m);
    return in;
}


/** Serialization of sequences **/
/* These methods are there only to simplify the writing of operator<< and
   operator>> and should not be called by user code directly */

template<class Iterator>
void binwrite_(PStream& out, Iterator it, unsigned int n)
{
    PStream::mode_t outmode = out.outmode; // store previous outmode
    if(outmode!=PStream::raw_binary && outmode!=PStream::plearn_binary)
        out.outmode = PStream::plearn_binary;
    while(n--)
    {
        out << *it;
        ++it;
    }
    out.outmode = outmode; // restore previous outmode
}

inline void binwrite_(PStream& out, const bool* x, unsigned int n)
{
    while(n--)
    {
        if(*x++)
            out.put('1');
        else
            out.put('0');
    }
}

inline void binwrite_(PStream& out, const char* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(char))); }
inline void binwrite_(PStream& out, char* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(char))); }

inline void binwrite_(PStream& out, const signed char* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(signed char))); }
inline void binwrite_(PStream& out, signed char* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(signed char))); }

inline void binwrite_(PStream& out, const unsigned char* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(unsigned char))); }
inline void binwrite_(PStream& out, unsigned char* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(unsigned char))); }

inline void binwrite_(PStream& out, const short* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(short))); }
inline void binwrite_(PStream& out, short* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(short))); }

inline void binwrite_(PStream& out, const unsigned short* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(unsigned short))); }
inline void binwrite_(PStream& out, unsigned short* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(unsigned short))); }

inline void binwrite_(PStream& out, const int* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(int))); }
inline void binwrite_(PStream& out, int* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(int))); }

inline void binwrite_(PStream& out, const unsigned int* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(unsigned int))); }
inline void binwrite_(PStream& out, unsigned int* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(unsigned int))); }

inline void binwrite_(PStream& out, const long* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(long))); }
inline void binwrite_(PStream& out, long* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(long))); }

inline void binwrite_(PStream& out, const unsigned long* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(unsigned long))); }
inline void binwrite_(PStream& out, unsigned long* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(unsigned long))); }

inline void binwrite_(PStream& out, const long long* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(long long))); }
inline void binwrite_(PStream& out, long long* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(long long))); }

inline void binwrite_(PStream& out, const unsigned long long* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(unsigned long long))); }
inline void binwrite_(PStream& out, unsigned long long* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(unsigned long long))); }

inline void binwrite_(PStream& out, const float* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(float))); }
inline void binwrite_(PStream& out, float* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(float))); }

inline void binwrite_(PStream& out, const double* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(double))); }
inline void binwrite_(PStream& out, double* x, unsigned int n)
{ out.write((char*)x, streamsize(n*sizeof(double))); }

// The typecode indicates the type and format of the elements in the stream

template<class Iterator>
void binread_(PStream& in, Iterator it, unsigned int n, unsigned char typecode)
{
    if(typecode!=0xFF)
        PLERROR("In binread_ : bug! A specialised binread_ should have been called for a typecode other than the 'generic' 0xFF");

    while(n--)
    {
        in >> *it;
        ++it;
    }
}


//! Auxiliary function that reads n elements of type I, optionally swaps
//! their endianness, then converts them into J, and puts them in a J array.
template<class I, class J>
void binread_as(PStream& in, J* x, unsigned int n, bool inverted_byte_order)
{
    I y;
    while(n--)
    {
        in.read(reinterpret_cast<char*>(&y), sizeof(I));
        if (inverted_byte_order)
            endianswap(&y);

#ifdef __INTEL_COMPILER
#pragma warning(disable:1682)
// Yes, I know that "implicit conversion of a 64-bit integral type to a smaller
// integral type (potential portability problem)", but the conversion is
// explicit here.
#endif
        *x = static_cast<J>(y);
#ifdef __INTEL_COMPILER
#pragma warning(default:1682)
#endif
        ++x;
    }
}

void binread_(PStream& in, bool* x, unsigned int n, unsigned char typecode);

inline void binread_(PStream& in, char* x,
                     unsigned int n, unsigned char typecode)
{
    // big endian and little endian have the same typecodes
    // so we need to check only one for consistency

    if(typecode!=TypeTraits<char>::little_endian_typecode()
       && typecode!=TypeTraits<unsigned char>::little_endian_typecode())
        PLERROR("In binread_ incompatible typecode");

    in.read((char*)x, n);
}

inline void binread_(PStream& in, signed char* x, unsigned int n,
                     unsigned char typecode)
{ binread_(in, (char *)x, n, typecode); }

inline void binread_(PStream& in, unsigned char* x, unsigned int n,
                     unsigned char typecode)
{ binread_(in, (char *)x, n, typecode); }

void binread_(PStream& in, short* x, unsigned int n, unsigned char typecode);
void binread_(PStream& in, unsigned short* x, unsigned int n,
              unsigned char typecode);
void binread_(PStream& in, int* x, unsigned int n, unsigned char typecode);
void binread_(PStream& in, unsigned int* x, unsigned int n,
              unsigned char typecode);
void binread_(PStream& in, long* x, unsigned int n, unsigned char typecode);
void binread_(PStream& in, unsigned long* x, unsigned int n,
              unsigned char typecode);
void binread_(PStream& in, long long* x, unsigned int n,
              unsigned char typecode);
void binread_(PStream& in, unsigned long long* x, unsigned int n,
              unsigned char typecode);
void binread_(PStream& in, float* x, unsigned int n, unsigned char typecode);
void binread_(PStream& in, double* x, unsigned int n, unsigned char typecode);


template<class SequenceType>
void writeSequence(PStream& out, const SequenceType& seq)
{
    // norman: added explicit cast
    uint32_t n = (uint32_t)seq.size();
    typename SequenceType::const_iterator it = seq.begin();

    switch(out.outmode)
    {
    case PStream::raw_ascii:
        while(n--)
        {
            out << *it;
            out.put(' ');
            ++it;
        }
        break;

    case PStream::pretty_ascii:
        out.write("[ ");
        while(n--)
        {
            out << *it;
            if(n>0)
                out.write(", ");
            ++it;
        }
        out.write(" ] ");
        break;

    case PStream::raw_binary:
        binwrite_(out, it, n);
        break;

    case PStream::plearn_ascii:
        out << n;
        out.write("[ ");
        while(n--)
        {
            out << *it;
            ++it;
        }
        out.write("] ");
        break;

    case PStream::plearn_binary:
    {
        unsigned char typecode;
        if(byte_order()==LITTLE_ENDIAN_ORDER)
        {
            out.put((char)0x12); // 1D little-endian
            typecode = TypeTraits<typename SequenceType::value_type>
                ::little_endian_typecode();
        }
        else
        {
            out.put((char)0x13); // 1D big-endian
            typecode = TypeTraits<typename SequenceType::value_type>
                ::big_endian_typecode();
        }

        // write typecode
        out.put(typecode);

        // write length in raw_binary
        out.write((char*)&n, sizeof(n));

        // write the data
        binwrite_(out, it, n);
    }
    break;

    default:
        PLERROR("In PStream::writeSequence(Iterator& it, int n)  unknown outmode!!!!!!!!!");
        break;
    }
}


//! Reads in a sequence type from a PStream.
/*! For this to work with the current implementation, the SequenceType must
have:
  - typedefs defining (SequenceType::...) value_type, size_type, iterator,
  - a begin() method that returns a proper iterator,
  - a size_type size() method returning the size of the current container,
  - a resize(size_type n) method that allows to change the size of the
  container (which should also work with resize(0)),
  - a push_back(const value_type& x) method that appends the element x at the
  end.
*/
template<class SequenceType>
void readSequence(PStream& in, SequenceType& seq)
{
    switch(in.inmode)
    {
    case PStream::raw_ascii:
    {
        // norman: added explicit cast
        int n = (int)seq.size();
        typename SequenceType::iterator it = seq.begin();
        while(n--)
        {
            in.skipBlanks();
            in >> *it;
            // don't skip blanks before we need to read something else
            // (read might block).
            //in.skipBlanks();
            ++it;
        }
    }
    break;
    case PStream::raw_binary:
    {
        int n = (int)seq.size();
        typename SequenceType::iterator it = seq.begin();
        while(n--)
        {
            in >> *it;
            ++it;
        }
    }
    break;

    case PStream::plearn_ascii:
    case PStream::plearn_binary:
    {
        in.skipBlanksAndComments();
        int c = in.peek();
        if (c == EOF)
            PLERROR("In PStream.h / readSequence - The input stream is empty");
        else if(c=='[') // read until ']'
        {
            in.get(); // skip '['
            seq.resize(0);
            in.skipBlanksAndCommentsAndSeparators();
            while(in.peek()!=']' && in.peek()!=EOF && !in.eof())
            {
                typename SequenceType::value_type x;
                in >> x;
                seq.push_back(x);
                in.skipBlanksAndCommentsAndSeparators();
            }
            if (in.peek()==EOF || in.eof())
                PLERROR("Reading stream, unmatched left bracket [, missing ]");
            in.get(); // skip ']'
        }
        else if(isdigit(c))
        {
            unsigned int n;
            in >> n;
            in.skipBlanksAndComments();
            c = in.get();
            if(c!='[')
                PLERROR("Error in readSequence(SequenceType& seq), expected '[', read '%c'",c);
            // don't skip blanks before we need to read something else
            // (read might block).
            //in.skipBlanksAndCommentsAndSeparators();
            seq.resize((typename SequenceType::size_type) n);
            if (n>0)
            {
                typename SequenceType::iterator it = seq.begin();
                while(n--)
                {
                    in.skipBlanksAndCommentsAndSeparators();
                    in >> *it;
                    // don't skip blanks before we need to read something else
                    // (read might block).
                    //in.skipBlanksAndCommentsAndSeparators();
                    ++it;
                }
            }
            in.skipBlanksAndCommentsAndSeparators();
            c = in.get();
            if(c!=']')
                PLERROR("Error in readSequence(SequenceType& seq), expected ']', read '%c'",c);

        }
        else if(c==0x12 || c==0x13) // it's a generic binary 1D sequence
        {
            in.get(); // eat c
            unsigned char typecode = in.get();
            unsigned int l;
            in.read((char*)&l,sizeof(l));

            bool inverted_byte_order =
                ((c==0x12 && byte_order()==BIG_ENDIAN_ORDER)
                 || (c==0x13 && byte_order()==LITTLE_ENDIAN_ORDER) );

            if(inverted_byte_order)
                endianswap(&l);
            seq.resize((typename SequenceType::size_type) l);
            binread_(in, seq.begin(), l, typecode);
        }
        else
            PLERROR("In readSequence(SequenceType& seq) '%c' not a proper first character in the header of a sequence!",c);
    }
    break;

    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }

}

// Default behavior for write() and read() is
// to call corresponding operator<<() or operator>>()
// on PStream.

template<class T>
inline void write(ostream& out_, const T& o)
{
    PStream out(&out_);
    out << o;
}

template<class T>
inline void read(istream& in_, T& o)
{
    PStream in(&in_);
    in >> o;
}

template<class T>
inline void read(const string& stringval, T& x)
{
    istringstream in_(stringval);
    PStream in(&in_);
    in >> x;
}


// STL containers:

template <class T> inline PStream &
operator>>(PStream &in, vector<T> &v)
{ readSequence(in, v); return in; }

template <class T> inline PStream &
operator<<(PStream &out, const vector<T> &v)
{ writeSequence(out, v); return out; }

// Serialization of map types

template<class SetT>
void writeSet(PStream& out, const SetT& s)
{
    typename SetT::const_iterator it = s.begin();
    typename SetT::const_iterator itend = s.end();

    out.put('[');
    while(it!=itend)
    {
        out << *it;
        ++it;
        if (it != itend)
            out.write(", ");
    }
    out.put(']');
}

template<class SetT>
void readSet(PStream& in, SetT& s)
{
    s.clear();
    in.skipBlanksAndCommentsAndSeparators();
    int c = in.get();
    if(c!='[')
        PLERROR("In readSet(Pstream& in, SetT& s) expected '[' but read %c",c);
    in.skipBlanksAndCommentsAndSeparators();
    c = in.peek(); // do we have a ']' ?
    while(c!=']')
    {
        typename SetT::value_type val;
        in >> val;
        s.insert(val);
        in.skipBlanksAndCommentsAndSeparators();
        c = in.peek(); // do we have a ']' ?
    }
    in.get(); // eat the ']'
}

template <class T> inline PStream &
operator>>(PStream &in, set<T> &v)
{ readSet(in, v); return in; }

template <class T> inline PStream &
operator<<(PStream &out, const set<T> &v)
{ writeSet(out, v); return out; }



// Serialization of priority_queue types

template<class PriorityQueueT>
void writePriorityQueue(PStream& out, const PriorityQueueT& pq)
{
    PriorityQueueT pq2(pq);
    out.put('[');
    while(!pq2.empty())
    {
        typename PriorityQueueT::value_type val;
        val = pq2.top();
        out << val;
        pq2.pop();
        if (!pq2.empty())
            out.write(", ");
    }
    out.put(']');
}

template<class PriorityQueueT>
void readPriorityQueue(PStream& in, PriorityQueueT& pq)
{
    PLCHECK(pq.empty());
    in.skipBlanksAndCommentsAndSeparators();
    int c = in.get();
    if(c!='[')
        PLERROR("In readPriorityQueue(Pstream& in, PriorityQueueT& pq) expected '[' but read %c",c);
    in.skipBlanksAndCommentsAndSeparators();
    c = in.peek(); // do we have a ']' ?
    while(c!=']')
    {
        typename PriorityQueueT::value_type val;
        in >> val;
        pq.push(val);
        in.skipBlanksAndCommentsAndSeparators();
        c = in.peek(); // do we have a ']' ?
    }
    in.get(); // eat the ']'
}

template <class T> inline PStream &
operator>>(PStream &in, priority_queue<T> &v)
{ readPriorityQueue(in, v); return in; }

template <class T> inline PStream &
operator<<(PStream &out, const priority_queue<T> &v)
{ writePriorityQueue(out, v); return out; }

/// @deprecated Use openFile instead.
class PIFStream: public PStream
{
public:
    PIFStream(const string& fname, ios_base::openmode m = ios_base::in)
        :PStream(new ifstream(fname.c_str()),true)
    {
        PLDEPRECATED("PIFStream is deprecated. Use the openFile function instead.");
    }
};

/// @deprecated Use openFile instead.
class POFStream: public PStream
{
public:
    POFStream(const string& fname,
              ios_base::openmode m = ios_base::out | ios_base::trunc)
        :PStream(new ofstream(fname.c_str()),true)
    {
        PLDEPRECATED("POFStream is deprecated. Use the openFile function instead.");
    }
};


/// @deprecated Use openString instead.
class PIStringStream: public PStream
{
public:
    PIStringStream(const string& s)
        :PStream(new istringstream(s), true /* own it */)
    {
        PLDEPRECATED("PIStringStream is deprecated. Use the openString function instead.");
    }
};


} // namespace PLearn

#endif //ndef PStream_INC


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
