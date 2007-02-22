// -*- C++ -*-

// PStream.cc
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2002 Frederic Morin, Xavier Saint-Mleux, Pascal Vincent
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

#include <limits>
#include "PStream.h"
#include "NullPStreamBuf.h"
#include "PrPStreamBuf.h"
#include <plearn/math/pl_math.h>
#include <nspr/prio.h>
#include <ctype.h>
#include <plearn/io/pl_log.h>


// This is probably an ugly hack to get it to work under Visual Studio.
#if defined(WIN32) && !defined(__CYGWIN__)
#define snprintf _snprintf
#endif

namespace PLearn {
using namespace std;

// Default format string for floats and doubles
const char* PStream::format_float_default  = "%.8g";
const char* PStream::format_double_default = "%.18g";

// Initialization for pin, pout, ...

PStream& get_pnull()
{
    static PStream pnull = new NullPStreamBuf();
    return pnull;    
}

PStream pnull = get_pnull();

PStream& get_pin()
{
    static bool initialized = false;
    static PStream pin = new
        PrPStreamBuf(PR_GetSpecialFD(PR_StandardInput),0);
    if(!initialized)
    {
        pin.setMode(PStream::raw_ascii); // raw_ascii default mode
        initialized = true;
    }
    return pin;
}

PStream pin = get_pin();

PStream& get_pout()
{
    static bool initialized = false;
    static PStream pout = new PrPStreamBuf(0,
                                           PR_GetSpecialFD(PR_StandardOutput));
    if(!initialized)
    {
        pout.setMode(PStream::raw_ascii); // raw_ascii default mode
        initialized = true;
    }
    return pout;
}

PStream pout = get_pout();

PStream& get_pio()
{
    static bool initialized = false;
    static PStream pio = new
        PrPStreamBuf(PR_GetSpecialFD(PR_StandardInput),
                     PR_GetSpecialFD(PR_StandardOutput));
    if(!initialized)
    {
        pio.setMode(PStream::raw_ascii); // raw_ascii default mode
        initialized = true;
    }
    return pio;
}

PStream pio = get_pio();

PStream& get_perr()
{
    static bool initialized = false;
    static PStream perr = new PrPStreamBuf(0,
                                           PR_GetSpecialFD(PR_StandardError));
    if(!initialized)
    {
        perr.setMode(PStream::raw_ascii); // raw_ascii default mode
        initialized = true;
        perr.setBufferCapacities(0,0,0);  // perr is unbuffered by default
    }
    return perr;
}

PStream perr = get_perr();

char PStream::tmpbuf[100];

PStream& flush(PStream& out)
{
    out.flush();
    return out;
}

PStream& endl(PStream& out)
{
    out.endl();
    return out;
}

PStream& ws(PStream& in)
{
    switch(in.inmode) {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
    case PStream::raw_binary:
        in.skipBlanks();
        break;
    case PStream::plearn_ascii:
    case PStream::plearn_binary:
        // Also skip comments.
        in.skipBlanksAndComments();
        break;
    default:
        PLERROR("In ws(PStream& in) - in's inmode is not supported");
    }
    return in;
}

string pgetline(PStream& in)
{
    string line;
    in.getline(line);
    // remove any trailing \n and \r
    string::size_type pos = line.length();
    while(pos>=1 && (line[pos - 1]=='\r' || line[pos - 1]=='\n'))
        pos--;
    return line.substr(0,pos);
}

PStream::PStream()
    :inherited(0),
     inmode(plearn_ascii), 
     outmode(plearn_ascii), 
     format_float (format_float_default),
     format_double(format_double_default),
     implicit_storage(true),
     compression_mode(compr_none)
{}

PStream::PStream(streambuftype* sb)
    :inherited(sb),
     inmode(plearn_ascii), 
     outmode(plearn_ascii), 
     format_float (format_float_default),
     format_double(format_double_default),
     implicit_storage(true),
     compression_mode(compr_none)
{}


//! ctor. from an istream (I)
PStream::PStream(istream* pin_, bool own_pin_)
    :inherited(new StdPStreamBuf(pin_,own_pin_)),
     inmode(plearn_ascii), 
     outmode(plearn_ascii),
     format_float (format_float_default),
     format_double(format_double_default),
     implicit_storage(true),
     compression_mode(compr_none)
{}
//! ctor. from an ostream (O)

PStream::PStream(ostream* pout_, bool own_pout_)
    :inherited(new StdPStreamBuf(pout_,own_pout_)),
     inmode(plearn_ascii), 
     outmode(plearn_ascii),
     format_float (format_float_default),
     format_double(format_double_default),
     implicit_storage(true),
     compression_mode(compr_none)
{}

//! ctor. from an iostream (IO)
PStream::PStream(iostream* pios_, bool own_pios_)
    :inherited(new StdPStreamBuf(pios_,own_pios_)),
     inmode(plearn_ascii), 
     outmode(plearn_ascii),
     format_float (format_float_default),
     format_double(format_double_default),
     implicit_storage(true),
     compression_mode(compr_none)
{}

//! ctor. from an istream and an ostream (IO)
PStream::PStream(istream* pin_, ostream* pout_, bool own_pin_, bool own_pout_)
    :inherited(new StdPStreamBuf(pin_,pout_,own_pin_,own_pout_)),
     inmode(plearn_ascii), 
     outmode(plearn_ascii),
     format_float (format_float_default),
     format_double(format_double_default),
     implicit_storage(true),
     compression_mode(compr_none)
{}

//! dtor.
PStream::~PStream()
{ }

PStream::mode_t PStream::switchToPLearnOutMode() 
{ 
    mode_t oldmode = outmode;
    switch(outmode)
    {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
        outmode = PStream::plearn_ascii;
        break;
    case PStream::raw_binary:
        outmode = PStream::plearn_binary;
        break;
    default:
        break;
    }
    return oldmode;
}

//////////////////
// readExpected //
//////////////////

void PStream::readExpected(char expect)
{
    int c = get(); 
    if(c!=expect)
        PLERROR("In readExpected : expected %c, but read %c",expect,c);
}

void PStream::readExpected(char* expect)
{
    for(char c = *expect; c!=0; c=*expect++)
        readExpected(c);    
}

void PStream::readExpected(const string& expect)
{
    int l = int(expect.size());
    for(int i=0; i<l; i++)
        readExpected(expect[i]);
}

  
streamsize PStream::readUntil(char* buf, streamsize n, char stop_char)
{
    streamsize nread = 0;

    while(nread<n)
    {        
        int c = get();
        if(c==EOF)
            break;
        if((char)c == stop_char)
        {
            putback(c);
            break;
        }
        *buf++ = (char)c;
        ++nread;
    }

    return nread;
}

streamsize PStream::readUntil(char* buf, streamsize n, const char* stop_chars)
{
    streamsize nread = 0;

    while(nread<n)
    {        
        int c = get();
        if(c==EOF)
            break;
        if(strchr(stop_chars, c))
        {
            putback(c);
            break;
        }
        *buf++ = (char)c;
        ++nread;
    }

    return nread;
}

int PStream::smartReadUntilNext(const string& stoppingsymbols, string& characters_read, bool ignore_brackets, bool skip_comments)
{
    int c;
    while( (c=get()) != EOF)
    {
        if(stoppingsymbols.find(c)!=string::npos)
            break;
        else if(skip_comments && c=='#')  // skip until end of line
            skipRestOfLine();
        else
        {
            if(characters_read.length() == characters_read.capacity())
                characters_read.reserve(characters_read.length()*2); //don't realloc&copy every time a char is appended...
            characters_read+= static_cast<char>(c);
          
            switch(c)
            {
            case '(':
                smartReadUntilNext(")", characters_read, ignore_brackets, skip_comments);
                characters_read+= ')';          
                break;
            case '[':
                if(!ignore_brackets)
                {
                    smartReadUntilNext("]", characters_read, ignore_brackets, skip_comments);
                    characters_read+= ']';          
                }
                break;
            case '{':
                smartReadUntilNext("}", characters_read, ignore_brackets, skip_comments);
                characters_read+= '}';          
                break;
            case '"':
                smartReadUntilNext("\"", characters_read, ignore_brackets, false);
                characters_read+= '"';          
                break;          
            }
        }
    }
    return c;
}

//! skips all occurences of any of the given characters
void PStream::skipAll(const char* chars_to_skip)
{
    int c = get();
    while(c!=EOF && strchr(chars_to_skip, c))
        c = get();
    if(c!=EOF)
        putback(c);
}

// reads everything until '\n' (also consumes the '\n')
void PStream::skipRestOfLine()
{
    int c = get();
    while(c!='\n' && c!=EOF)
        c=get();
}

void PStream::skipBlanks()
{
    int c = get();
    while(c!=EOF && (c==' ' || c=='\t' || c=='\n' || c=='\r'))
        c = get();
    if(c!=EOF)
        putback(c);
}

void PStream::skipBlanksAndComments()
{
    int c = get();
    while(c!=EOF)
    { 
        if(c=='#')
            skipRestOfLine();     
        else if(c!=' ' && c!='\t' && c!='\n' && c!='\r')
            break;
        c = get();
    }
    if(c!=EOF)
        putback(c);
}

void PStream::skipBlanksAndCommentsAndSeparators()
{
    int c = get();
    while(c!=EOF)
    { 
        if(c=='#')
            skipRestOfLine();     
        else if(c!=' ' && c!='\t' && c!='\n' && c!='\r' && c!=';' && c!=',')
            break;
        c = get();
    }
    if(c!=EOF)
        putback(c);
}

///////////
// count //
///////////
int PStream::count(char c) {
    char ch = get();
    int counter = 0;
    while (ch != EOF) {
        if (ch == c)
            counter++;
        ch = get();
    }
    return counter;
}

void PStream::writeAsciiHexNum(unsigned char x)
{
    int d = x>>4;
    put(d<0x0A ?(d+'0') :(d-0x0A+'A'));
    d = x & 0x0F;
    put(d<0x0A ?(d+'0') :(d-0x0A+'A'));
}


void PStream::writeAsciiNum(char x)
{
    snprintf(tmpbuf, sizeof(tmpbuf), "%d", (int)x);
    write(tmpbuf, streamsize(strlen(tmpbuf)));
}

void PStream::writeAsciiNum(unsigned char x)
{
    snprintf(tmpbuf, sizeof(tmpbuf), "%d", (int)x);
    write(tmpbuf, streamsize(strlen(tmpbuf)));
}

void PStream::writeAsciiNum(signed char x)
{
    snprintf(tmpbuf, sizeof(tmpbuf), "%d", (int)x);
    write(tmpbuf, streamsize(strlen(tmpbuf)));
}

void PStream::writeAsciiNum(short x)
{
    snprintf(tmpbuf, sizeof(tmpbuf), "%hd", x);
    write(tmpbuf, streamsize(strlen(tmpbuf)));
}

void PStream::writeAsciiNum(unsigned short x)
{
    snprintf(tmpbuf, sizeof(tmpbuf), "%hu", x);
    write(tmpbuf, streamsize(strlen(tmpbuf)));
}

void PStream::writeAsciiNum(int x)
{
    snprintf(tmpbuf, sizeof(tmpbuf), "%d", x);
    write(tmpbuf, streamsize(strlen(tmpbuf)));
}

void PStream::writeAsciiNum(unsigned int x)
{
    snprintf(tmpbuf, sizeof(tmpbuf), "%u", x);
    write(tmpbuf, streamsize(strlen(tmpbuf)));
}

void PStream::writeAsciiNum(long x)
{
    snprintf(tmpbuf, sizeof(tmpbuf), "%ld", x);
    write(tmpbuf, streamsize(strlen(tmpbuf)));
}

void PStream::writeAsciiNum(unsigned long x)
{
    snprintf(tmpbuf, sizeof(tmpbuf), "%lu", x);
    write(tmpbuf, streamsize(strlen(tmpbuf)));
}

void PStream::writeAsciiNum(long long x)
{
    long long zero(0);
    if(x>=zero)
        writeAsciiNum((unsigned long long)x);
    else
    {        
        put('-');
        writeAsciiNum((unsigned long long) -x);
    }
}

void PStream::writeAsciiNum(unsigned long long x)
{
    unsigned long long zero(0);
    if(x==zero)
        put('0');
    else
    {
        char buf[30];
        char* p = buf+29;
        int n = 0;
        while(x>zero)
        {
            *p-- = '0'+char(x%10);
            x /= 10;
            ++n;
        }
        ++p;
        write(p,n);
    }
}

void PStream::writeAsciiNum(float x)
{
    if(is_missing(x))
        write("nan");
    else if (isinf(x)) {
        if (x < 0)
            write("-inf");
        else
            write("inf");
    }
    else
    {
        snprintf(tmpbuf, sizeof(tmpbuf), format_float, x);
        write(tmpbuf, streamsize(strlen(tmpbuf)));
    }
}

void PStream::writeAsciiNum(double x)
{
    if(is_missing(x))
        write("nan");
    else if (isinf(x)) {
        if (x < 0)
            write("-inf");
        else
            write("inf");
    }
    else
    {
        snprintf(tmpbuf, sizeof(tmpbuf), format_double, x);
        write(tmpbuf, streamsize(strlen(tmpbuf)));
    }
}

void PStream::readAsciiNum(char &x)
{
    long dx;
    readAsciiNum(dx);
    x = char(dx);
}

void PStream::readAsciiNum(unsigned char &x)
{
    long dx;
    readAsciiNum(dx);
    x = (unsigned char)(dx);
}

void PStream::readAsciiNum(signed char &x)
{
    long dx;
    readAsciiNum(dx);
    x = (signed char)(dx);
}

void PStream::readAsciiNum(short &x)
{
    long dx;
    readAsciiNum(dx);
    x = short(dx);
}

void PStream::readAsciiNum(unsigned short &x)
{
    unsigned long dx;
    readAsciiNum(dx);
    x = (unsigned short)(dx);
}

void PStream::readAsciiNum(int &x)
{
    long dx;
    readAsciiNum(dx);
    x = int(dx);
}

void PStream::readAsciiNum(unsigned int &x)
{
    unsigned long dx;
    readAsciiNum(dx);
    x = (unsigned int)(dx);
}

void PStream::readAsciiNum(long &x)
{
    skipBlanks();
    x = 0;
    char c = get();
    bool negate = false;
    if (c == '-')
    {
        negate = true;
        c = get();
    }
    else if (c == '+')
        c = get();
  
    if(!isdigit(c))
        PLERROR("In readAsciiNum: not a valid ascii number, expected a digit, but read %c (ascii code %d)",c,c);

    do
    {
        x = x*10 + c-'0';
        c = get();
    } while(isdigit(c));

    unget();
    if(negate)
        x = -x;
}

void PStream::readAsciiNum(unsigned long &x)
{
    skipBlanks();
    x = 0;
    char c = get();
    while(isdigit(c))
    {
        x = x*10 + c-'0';
        c = get();
    }
    unget();
}

void PStream::readAsciiNum(long long &x)
{
    skipBlanks();
    x = 0;
    char c = get();
    bool negate = false;
    if (c == '-')
    {
        negate = true;
        c = get();
    }
    else if (c == '+')
        c = get();
  
    if(!isdigit(c))
        PLERROR("In readAsciiNum: not a valid ascii number, expected a digit, but read %c (ascii code %d)",c,c);

    do
    {
        x *= 10;
        x += c-'0';
        c = get();
    } while(isdigit(c));

    unget();
    if(negate)
        x = -x;
}

void PStream::readAsciiNum(unsigned long long &x)
{
    skipBlanks();
    x = 0;
    char c = get();
    while(isdigit(c))
    {
        x *= 10;
        x += c-'0';
        c = get();
    }
    unget();
}

void PStream::readAsciiNum(float &x)
{
    double dx;
    readAsciiNum(dx);
    x = float(dx);
}

void PStream::readAsciiNum(double &x)
{
    static const char* error_msg = "Bug while reading file and expecting a double";
    skipBlanks();
    int l=0;
    bool opposite = false;
  
    char c = get();
    if (c == '-') {
        tmpbuf[l++] = c;
        opposite = true;
        c = get();
    }

    switch(c)
    {
    case '?':
        x = MISSING_VALUE;
        break;
    case 'n':
    case 'N':
        if(toupper(get())=='A' && toupper(get())=='N')
            x = MISSING_VALUE;
        else
            PLERROR(error_msg);
        break;
    case 'i':
    case 'I':
        if (toupper(get())=='N' && toupper(get())=='F')
        {
            x = numeric_limits<double>::infinity();
            if(opposite)
                x = -x;
        }
        else
            PLERROR(error_msg);
        break ; 
    default:
        while(isdigit(c) || c=='-' || c=='+' || c=='.' || c=='e' || c=='E')
        {
            tmpbuf[l++] = c;
            c = get();
        }
        tmpbuf[l] = '\0';
        unget();
        sscanf(tmpbuf,"%lf",&x);
        break;
    }
}

PStream& PStream::operator=(const PStream& pios)
{ 
    if(this != &pios)
    {
        inherited::operator=((const inherited&)pios);
        inmode = pios.inmode;
        outmode = pios.outmode;
        implicit_storage = pios.implicit_storage;
        compression_mode = pios.compression_mode;
    }
    return *this;
}

// Implementation of operator>>'s

PStream& PStream::operator>>(char &x)
{
    switch(inmode)
    {
    case raw_binary:
        get(x);
        break;
    case pretty_ascii:
    case raw_ascii:
        skipBlanks();
        get(x);
        break;
    case plearn_ascii:
    case plearn_binary:
        skipBlanksAndCommentsAndSeparators();
        get(x);
        if (x == '\'')  // ascii case (between single quotes)
        {
            get(x);
            get();
        }
        else if (x == 0x01)  // plearn_binary case
            get(x);
        break;
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}
 
PStream& PStream::operator>>(signed char &x)
{
    char c;
    operator>>(c);
    x = (signed char)c;
    return *this;
}

PStream& PStream::operator>>(unsigned char &x)
{
    switch(inmode)
    {
    case raw_binary:
        read(reinterpret_cast<char *>(&x), sizeof(unsigned char));
        break;
    case raw_ascii:
        skipBlanks();
        read(reinterpret_cast<char *>(&x), sizeof(unsigned char));
        break;
    case pretty_ascii:
    case plearn_ascii:
    case plearn_binary:
    {
        skipBlanksAndCommentsAndSeparators();
        char c = get();
        if (c == '\'')  // ascii
        {
            read(reinterpret_cast<char *>(&x), sizeof(unsigned char));
            get();
        }
        else if (c == 0x02)  // plearn_binary
            read(reinterpret_cast<char *>(&x), sizeof(unsigned char));
        else
            x = (unsigned char)c;
        break;
    }
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator>>(char *x)
{
    if(!x)
        PLERROR("In PStream::operator>>(char*) character array must already be allocated to put the read string in");

    switch(inmode)
    {
    case PStream::raw_ascii:
    case PStream::raw_binary:
    case PStream::pretty_ascii:
    {
        skipBlanksAndComments();
        int i=0; // pos within the string
        int c = get();
        while (c!=EOF && wordseparators().find(c)==string::npos) // as long as we don't meet a wordseparator (or eof)...
        {
            x[i++] = static_cast<char>(c);
            c = get();
        }
        x[i++] = 0;
        unget();
    }
    break;
    case PStream::plearn_ascii:
    case PStream::plearn_binary:
    {
        skipBlanksAndComments();
        int c = peek();
        int i=0; // pos within the string
        if(c=='"') // it's a quoted string "..."
        {
            c = get(); // skip the quote
            c = get(); // get the next character
            while(c!='"' && c!=EOF)
            {
                if(c=='\\') // escaped character
                {
                    c = get();
                    switch (c)
                    {
                    case 'n':
                        x[i++] = '\n';
                        break;
                    case 't':
                        x[i++] = '\t';
                        break;
                    case 'r':
                        x[i++] = '\r';
                        break;
                    case '0':
                        x[i++] = '\0';
                        break;
                    case 'a':
                        x[i++] = '\a';
                        break;
                    case 'b':
                        x[i++] = '\b';
                        break;
                    case 'v':
                        x[i++] = '\v';
                        break;
                    case 'f':
                        x[i++] = '\f';
                        break;

                    default:
                        x[i++] = static_cast<char>(c);
                        break;
                    }
                }
                else
                    x[i++]= static_cast<char>(c);

                c = get();
            }
            if(c==EOF)
                PLERROR("In read(istream&, char*) unterminated quoted string");
            if(!isspace(c))
                putback(c);
        }
        else // it's a single word without quotes
        {
            c= get();
            while(c != EOF && wordseparators().find(c)==string::npos) // as long as we don't meet a wordseparator (or eof)...
            {
                x[i++]= static_cast<char>(c);
                c= get();
            }
            if(!isspace(c))
                unget();
        }
        x[i++] = 0;
    }
    break;
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator>>(string& x)
{
    switch(inmode)
    {
    case PStream::raw_ascii:
    case PStream::raw_binary:
    case PStream::pretty_ascii:
    {
        skipBlanksAndComments();
        int c = get();
        x.clear();
        // As long as we don't meet a (raw) wordseparator (or eof)...
        while (c!=EOF && raw_wordseparators().find(c)==string::npos)
        {
            x += static_cast<char>(c);
            c = get();
        }
        unget();
    }
    break;
    case PStream::plearn_ascii:
    case PStream::plearn_binary:
    { 
        skipBlanksAndComments();
        int c = peek();
        if(c=='"') // it's a quoted string "..."
        {
            x.resize(0);
            c = get(); // skip the quote
            c = get(); // get the next character
            while(c!='"' && c!=EOF)
            {
                if(c=='\\') // escaped character
                {
                    c = get();
                    switch (c)             
                    {
                    case 'n':
                        x += '\n';
                        break;
                    case 't':
                        x += '\t';
                        break;
                    case 'r':
                        x += '\r';
                        break;
                    case '0':
                        x += '\0';
                        break;
                    case 'a':
                        x += '\a';
                        break;
                    case 'b':
                        x += '\b';
                        break;
                    case 'v':
                        x += '\v';
                        break;
                    case 'f':
                        x += '\f';
                        break;

                    default:
                        x += static_cast<char>(c);
                        break;
                    }
                }
                else
                    x += static_cast<char>(c);

                c = get();
            }
            if(c==EOF)
                PLERROR("In read(istream&, string&) unterminated quoted string");
            c = get();
            if(!isspace(c)) // skip following blank if any
                putback(c);
        }
        else // it's a single word without quotes
        {
            x.resize(0);      
            c= get();
            while(c != EOF && wordseparators().find(c)==string::npos) // as long as we don't meet a wordseparator (or eof)...
            {
                x+= static_cast<char>(c);
                c= get();
            }
            if(!isspace(c))
                unget();
        }
    }
    break;
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator>>(int &x)
{
    switch(inmode)
    {
    case raw_ascii:
    case pretty_ascii:
        skipBlanks();
        readAsciiNum(x);
        break;
    case raw_binary:
        read(reinterpret_cast<char *>(&x), sizeof(int));
        break;
    case plearn_ascii:
    case plearn_binary:
    {
        skipBlanksAndCommentsAndSeparators();
        int c = get();
        if(c==0x07 || c==0x08 || c==0x0B || c==0x0C )  // plearn_binary
        {
            read(reinterpret_cast<char*>(&x),sizeof(int));
            if( (c==0x07 && byte_order()==BIG_ENDIAN_ORDER) 
                || (c==0x08 && byte_order()==LITTLE_ENDIAN_ORDER) )
                endianswap(&x);
        }
        else  // plearn_ascii
        {
            unget();
            readAsciiNum(x);
        }
        break;
    }
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator>>(unsigned int &x)
{
    switch(inmode)
    {
    case raw_ascii:
    case pretty_ascii:
        skipBlanks();
        readAsciiNum(x);
        break;
    case raw_binary:
        read(reinterpret_cast<char *>(&x), sizeof(unsigned int));
        break;
    case plearn_ascii:
    case plearn_binary:
    {
        skipBlanksAndCommentsAndSeparators();
        int c = get();
        if(c==0x0B || c==0x0C || c==0x07 || c==0x08)  // plearn_binary unsigned int or int
        {
            read(reinterpret_cast<char*>(&x),sizeof(unsigned int));
            if( (c==0x0B && byte_order()==BIG_ENDIAN_ORDER) 
                || (c==0x0C && byte_order()==LITTLE_ENDIAN_ORDER) )
                endianswap(&x);
        }
        else  // plearn_ascii
        {
            unget();
            readAsciiNum(x);
        }
        break;
    }
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}
  
PStream& PStream::operator>>(long &x)
{
    switch(inmode)
    {
    case raw_ascii:
    case pretty_ascii:
        skipBlanks();
        readAsciiNum(x);
        break;
    case raw_binary:
        read(reinterpret_cast<char *>(&x), sizeof(long));
        break;
    case plearn_ascii:
    case plearn_binary:
    {
        skipBlanksAndCommentsAndSeparators();
        int c = get();
        if(c==0x07 || c==0x08)  // plearn_binary
        {
            read(reinterpret_cast<char*>(&x),sizeof(long));
            if( (c==0x07 && byte_order()==BIG_ENDIAN_ORDER) 
                || (c==0x08 && byte_order()==LITTLE_ENDIAN_ORDER) )
                endianswap(&x);
        }
        else  // plearn_ascii
        {
            unget();
            readAsciiNum(x);
        }
        break;
    }
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}
  
PStream& PStream::operator>>(unsigned long &x)
{
    switch(inmode)
    {
    case raw_ascii:
    case pretty_ascii:
        skipBlanks();
        readAsciiNum(x);
        break;
    case raw_binary:
        read(reinterpret_cast<char *>(&x), sizeof(unsigned long));
        break;
    case plearn_ascii:
    case plearn_binary:
    {
        skipBlanksAndCommentsAndSeparators();
        int c = get();
        if(c==0x0B || c==0x0C)  // plearn_binary
        {
            read(reinterpret_cast<char*>(&x),sizeof(unsigned long));
            if( (c==0x0B && byte_order()==BIG_ENDIAN_ORDER) 
                || (c==0x0C && byte_order()==LITTLE_ENDIAN_ORDER) )
                endianswap(&x);
        }
        else  // plearn_ascii
        {
            unget();
            readAsciiNum(x);
        }
        break;
    }
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}


PStream& PStream::operator>>(long long &x)
{
    switch(inmode)
    {
    case raw_ascii:
    case pretty_ascii:
        skipBlanks();
        readAsciiNum(x);
        break;
    case raw_binary:
        read(reinterpret_cast<char *>(&x), sizeof(long long));
        break;
    case plearn_ascii:
    case plearn_binary:
    {
        skipBlanksAndCommentsAndSeparators();
        int c = get();
        if(c==0x16 || c==0x17)  // plearn_binary
        {
            read(reinterpret_cast<char*>(&x),sizeof(long long));
            if( (c==0x16 && byte_order()==BIG_ENDIAN_ORDER) 
                || (c==0x17 && byte_order()==LITTLE_ENDIAN_ORDER) )
                endianswap(&x);
        }
        else  // plearn_ascii
        {
            unget();
            readAsciiNum(x);
        }
        break;
    }
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}
  
PStream& PStream::operator>>(unsigned long long &x)
{
    switch(inmode)
    {
    case raw_ascii:
    case pretty_ascii:
        skipBlanks();
        readAsciiNum(x);
        break;
    case raw_binary:
        read(reinterpret_cast<char *>(&x), sizeof(unsigned long long));
        break;
    case plearn_ascii:
    case plearn_binary:
    {
        skipBlanksAndCommentsAndSeparators();
        int c = get();
        if(c==0x18 || c==0x19)  // plearn_binary
        {
            read(reinterpret_cast<char*>(&x),sizeof(unsigned long long));
            if( (c==0x18 && byte_order()==BIG_ENDIAN_ORDER) 
                || (c==0x19 && byte_order()==LITTLE_ENDIAN_ORDER) )
                endianswap(&x);
        }
        else  // plearn_ascii
        {
            unget();
            readAsciiNum(x);
        }
        break;
    }
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}

  
PStream& PStream::operator>>(short &x)
{
    switch(inmode)
    {
    case raw_ascii:
    case pretty_ascii:
        skipBlanks();
        readAsciiNum(x);
        break;
    case raw_binary:
        read(reinterpret_cast<char *>(&x), sizeof(short));
        break;
    case plearn_ascii:
    case plearn_binary:
    {
        skipBlanksAndCommentsAndSeparators();
        int c = get();
        if(c==0x03 || c==0x04)  // plearn_binary
        {
            read(reinterpret_cast<char*>(&x),sizeof(short));
            if( (c==0x03 && byte_order()==BIG_ENDIAN_ORDER) 
                || (c==0x04 && byte_order()==LITTLE_ENDIAN_ORDER) )
                endianswap(&x);
        }
        else  // plearn_ascii
        {
            unget();
            readAsciiNum(x);
        }
        break;
    }
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}
  
PStream& PStream::operator>>(unsigned short &x)
{
    switch(inmode)
    {
    case raw_ascii:
    case pretty_ascii:
        skipBlanks();
        readAsciiNum(x);
        break;
    case raw_binary:
        read(reinterpret_cast<char *>(&x), sizeof(unsigned short));
        break;
    case plearn_ascii:
    case plearn_binary:
    {
        skipBlanksAndCommentsAndSeparators();
        int c = get();
        if(c==0x05 || c==0x06)  // plearn_binary
        {
            read(reinterpret_cast<char*>(&x),sizeof(unsigned short));
            if( (c==0x05 && byte_order()==BIG_ENDIAN_ORDER) 
                || (c==0x06 && byte_order()==LITTLE_ENDIAN_ORDER) )
                endianswap(&x);
        }
        else  // plearn_ascii
        {
            unget();
            readAsciiNum(x);
        }
        break;
    }
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator>>(bool &x)
{
    int parsed = -1;

    char c;
    switch(inmode)
    {
    case raw_ascii:
    case pretty_ascii:
    case raw_binary:
    case plearn_ascii:
    case plearn_binary:
        skipBlanksAndCommentsAndSeparators();
        c = get();
        if(c=='1')
            parsed = 1;

        else if(c=='0')
            parsed = 0;

        else if(c=='T')
        {
            char r = get();        
            char u = get();        
            char e = get();
            if ( r == 'r' && u == 'u' && e == 'e' )
                parsed = 1;
        }
      
        else if(c=='F')
        {
            char a = get();        
            char l = get();
            char s = get();        
            char e = get();
            if ( a == 'a' && l == 'l' && s == 's' && e == 'e' )
                parsed = 0;
        }

        if ( parsed == -1 )
            PLERROR("In PStream::operator>>(bool &x) wrong format for bool, must be one "
                    "of characters 0 or 1 or unquoted strings True or False" );
        else
            x = (parsed != 0);
        break;

    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator>>(float &x)
{
    switch(inmode)
    {
    case raw_ascii:
    case pretty_ascii:
        skipBlanks();
        readAsciiNum(x);
        break;
    case raw_binary:
        read(reinterpret_cast<char *>(&x), sizeof(float));
        break;
    case plearn_ascii:
    case plearn_binary:
    {
        skipBlanksAndCommentsAndSeparators();
        int c = get();
        if(c==0x0E || c==0x0F)  // plearn_binary
        {
            read(reinterpret_cast<char*>(&x),sizeof(float));
            if( (c==0x0E && byte_order()==BIG_ENDIAN_ORDER) 
                || (c==0x0F && byte_order()==LITTLE_ENDIAN_ORDER) )
                endianswap(&x);
        }
        else  // plearn_ascii
        {
            unget();
            readAsciiNum(x);
        }
        break;
    }
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator>>(double &x)
{
    switch(inmode)
    {
    case raw_ascii:
    case pretty_ascii:
        skipBlanks();
        readAsciiNum(x);
        break;
    case raw_binary:
        read(reinterpret_cast<char *>(&x), sizeof(double));
        break;
    case plearn_ascii:
    case plearn_binary:
    {
        skipBlanksAndCommentsAndSeparators();
        int c = get();
        if(c==0x10 || c==0x11)  // plearn_binary
        {
            read(reinterpret_cast<char*>(&x),sizeof(double));
            if( (c==0x10 && byte_order()==BIG_ENDIAN_ORDER) 
                || (c==0x11 && byte_order()==LITTLE_ENDIAN_ORDER) )
                endianswap(&x);
        }
        else  // ascii
        {
            unget();
            readAsciiNum(x);
        }
        break;
    }
    default:
        PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
        break;
    }
    return *this;
}


// Implementation of operator<<'s

PStream& PStream::operator<<(char x) 
{ 
    switch(outmode)
    {
    case raw_ascii:
    case raw_binary:
    case pretty_ascii:
        put(x);
        break;
    case plearn_ascii:
        put('\'');
        put(x);
        put('\'');
        put(' ');
        break;
    case plearn_binary:
        put((char)0x01);
        put(x);
        break;
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator<<(signed char x) 
{
    operator<<(char(x));
    return *this;
}

PStream& PStream::operator<<(unsigned char x) 
{ 
    switch(outmode)
    {
    case raw_ascii:
        put(x);
        put(' ');
        break;
    case raw_binary:
        put(x);
        break;
    case pretty_ascii:
    case plearn_ascii:
        put('\'');
        put(x);
        put('\'');
        put(' ');
        break;
    case plearn_binary:
        put((char)0x02);
        put(x);
        break;
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator<<(const char *x)
{
    int l = (int)strlen(x);
    switch(outmode)
    {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
    case PStream::raw_binary:
        write(x);
        break;      

    case PStream::plearn_ascii:
    case PStream::plearn_binary:
    {
        put('"');
        for(int i=0; i<l; i++)
        {
            char c = x[i];
            if(c=='"' || c=='\\') // escape quote and backslash
                put('\\');
            put(c);
        }
        put('"');
        put(' ');
    }
    break;

    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator<<(const void* x)
{
    switch(outmode)
    {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
    case PStream::raw_binary:
    case PStream::plearn_ascii:
    case PStream::plearn_binary:
    {
        char buffer[1000];
        snprintf(buffer, 1000, "%p ", x);
        buffer[999] = '\0';
        write(buffer);
        break;
    }
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}


// TODO What if the string contains a word separator ??
PStream& PStream::operator<<(const string &x)
{
    const char* array = x.c_str();
    operator<<(array);
    return *this;
}

PStream& PStream::operator<<(bool x) 
{ 
    switch(outmode)
    {
    case plearn_ascii:
        if(x)
            put('1');
        else
            put('0');
        put(' ');
        break;
    case raw_ascii:
    case pretty_ascii:
    case raw_binary:
    case plearn_binary:
        if(x)
            put('1');
        else
            put('0');
        break;    
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator<<(int x) 
{ 
    switch(outmode)
    {
    case raw_binary:
        write(reinterpret_cast<char *>(&x), sizeof(int));
        break;
    case raw_ascii:
    case pretty_ascii:
        writeAsciiNum(x);
        break;
    case plearn_ascii:
        writeAsciiNum(x);
        put(' ');
        break;
    case plearn_binary:
#ifdef BIGENDIAN
        put((char)0x08);
#else
        put((char)0x07);
#endif
        write((char*)&x,sizeof(int));
        break;
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator<<(unsigned int x) 
{ 
    switch(outmode)
    {
    case raw_binary:
        write(reinterpret_cast<char *>(&x), sizeof(unsigned int));
        break;
    case raw_ascii:
    case pretty_ascii:
        writeAsciiNum(x);
        break;
    case plearn_ascii:
        writeAsciiNum(x);
        put(' ');
        break;
    case plearn_binary:
#ifdef BIGENDIAN
        put((char)0x0C);
#else
        put((char)0x0B);
#endif
        write((char*)&x,sizeof(unsigned int));
        break;
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator<<(long x) 
{ 
    switch(outmode)
    {
    case raw_binary:
        write(reinterpret_cast<char *>(&x), sizeof(long));
        break;
    case raw_ascii:
    case pretty_ascii:
        writeAsciiNum(x);
        break;
    case plearn_ascii:
        writeAsciiNum(x);
        put(' ');
        break;
    case plearn_binary:
#ifdef BIGENDIAN
        put((char)0x08);
#else
        put((char)0x07);
#endif
        write((char*)&x,sizeof(long));
        break;
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator<<(unsigned long x) 
{ 
    switch(outmode)
    {
    case raw_binary:
        write(reinterpret_cast<char *>(&x), sizeof(unsigned long));
        break;
    case raw_ascii:
    case pretty_ascii:
        writeAsciiNum(x);
        break;
    case plearn_ascii:
        writeAsciiNum(x);
        put(' ');
        break;
    case plearn_binary:
#ifdef BIGENDIAN
        put((char)0x0C);
#else
        put((char)0x0B);
#endif
        write((char*)&x,sizeof(unsigned long));
        break;
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator<<(long long x) 
{ 
    switch(outmode)
    {
    case raw_binary:
        write(reinterpret_cast<char *>(&x), sizeof(long long));
        break;
    case raw_ascii:
    case pretty_ascii:
        writeAsciiNum(x);
        break;
    case plearn_ascii:
        writeAsciiNum(x);
        put(' ');
        break;
    case plearn_binary:
#ifdef BIGENDIAN
        put((char)0x17);
#else
        put((char)0x16);
#endif
        write((char*)&x,sizeof(long long));
        break;
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator<<(unsigned long long x) 
{ 
    switch(outmode)
    {
    case raw_binary:
        write(reinterpret_cast<char *>(&x), sizeof(unsigned long long));
        break;
    case raw_ascii:
    case pretty_ascii:
        writeAsciiNum(x);
        break;
    case plearn_ascii:
        writeAsciiNum(x);
        put(' ');
        break;
    case plearn_binary:
#ifdef BIGENDIAN
        put((char)0x19);
#else
        put((char)0x18);
#endif
        write((char*)&x,sizeof(unsigned long long));
        break;
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}


PStream& PStream::operator<<(short x) 
{ 
    switch(outmode)
    {
    case raw_binary:
        write(reinterpret_cast<char *>(&x), sizeof(short));
        break;
    case raw_ascii:
    case pretty_ascii:
        writeAsciiNum(x);
        break;
    case plearn_ascii:
        writeAsciiNum(x);
        put(' ');
        break;
    case plearn_binary:
#ifdef BIGENDIAN
        put((char)0x04);
#else
        put((char)0x03);
#endif
        write((char*)&x,sizeof(short));
        break;
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator<<(unsigned short x) 
{ 
    switch(outmode)
    {
    case raw_binary:
        write(reinterpret_cast<char *>(&x), sizeof(unsigned short));
        break;
    case raw_ascii:
    case pretty_ascii:
        writeAsciiNum(x);
        break;
    case plearn_ascii:
        writeAsciiNum(x);
        put(' ');
        break;
    case plearn_binary:
#ifdef BIGENDIAN
        put((char)0x06);
#else
        put((char)0x05);
#endif
        write((char*)&x,sizeof(unsigned short));
        break;
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator<<(float x)
{
    switch(outmode)
    {
    case raw_binary:
        write(reinterpret_cast<char *>(&x), sizeof(float));
        break;
    case raw_ascii:
    case pretty_ascii:
        writeAsciiNum(x);
        break;
    case plearn_ascii:
        writeAsciiNum(x);
        put(' ');
        break;
    case plearn_binary:
#ifdef BIGENDIAN
        put((char)0x0F);
#else
        put((char)0x0E);
#endif
        write((char*)&x,sizeof(float));
        break;
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}

PStream& PStream::operator<<(double x)
{
    switch(outmode)
    {
    case raw_binary:
        write(reinterpret_cast<char *>(&x), sizeof(double));
        break;
    case raw_ascii:
    case pretty_ascii:
        writeAsciiNum(x);
        break;
    case plearn_ascii:
        writeAsciiNum(x);
        put(' ');
        break;
    case plearn_binary:
#ifdef BIGENDIAN
        put((char)0x11);
#else
        put((char)0x10);
#endif
        write((char*)&x,sizeof(double));
        break;
    default:
        PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
        break;
    }
    return *this;
}


void binread_(PStream& in, bool* x,                
              unsigned int n, unsigned char typecode)  
{                                                      
    if(typecode!=TypeTraits<bool>::little_endian_typecode()) 
        PLERROR("In binread_ incompatible typecode");      

    while(n--)
    {
        int c = in.get();
        if(c=='0')
            *x = false;
        else if(c=='1')
            *x = true;
        else
            PLERROR("In binread_(PStream& in, bool* x, unsigned int n, unsigned char typecode): "
                    "read invalid value for a boolean: should be '1' or '0', not %c", c);
        ++x;
    }
}

#define IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(BASETYPE)  \
void binread_(PStream& in, BASETYPE* x,                \
              unsigned int n, unsigned char typecode)  \
{                                                      \
  if(typecode==TypeTraits<BASETYPE>::little_endian_typecode()) \
    {                                                  \
      in.read((char*)x, streamsize(n*sizeof(BASETYPE)));       \
      if(byte_order()==BIG_ENDIAN_ORDER)               \
        endianswap(x,n);                               \
    }                                                  \
  else if(typecode==TypeTraits<BASETYPE>::big_endian_typecode()) \
    {                                                  \
      in.read((char*)x, streamsize(n*sizeof(BASETYPE)));       \
      if(byte_order()==LITTLE_ENDIAN_ORDER)            \
        endianswap(x,n);                               \
    }                                                  \
  else                                                 \
    PLERROR("In binread_ incompatible typecode");      \
}


// IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(char);
IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(short)
    IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(unsigned short)
    IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(int)
    IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(unsigned int)
    IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(long)
    IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(unsigned long)

//! The binread_ for float and double are special

void binread_(PStream& in, double* x, unsigned int n, unsigned char typecode)
{ 
    if(typecode==TypeTraits<double>::little_endian_typecode())
    {
        in.read((char*)x, streamsize(n*sizeof(double))); 
#ifdef BIGENDIAN
        endianswap(x,n); 
#endif      
    }
    else if(typecode==TypeTraits<double>::big_endian_typecode())
    {
        in.read((char*)x, streamsize(n*sizeof(double))); 
#ifdef LITTLEENDIAN
        endianswap(x,n); 
#endif
    }
    else if(typecode==TypeTraits<float>::little_endian_typecode())
    {    
        float val;
        while(n--)
        {
            in.read((char*)&val, sizeof(float));
#ifdef BIGENDIAN
            endianswap(&val);
#endif
            *x++ = double(val);
        }
    }
    else if(typecode==TypeTraits<float>::big_endian_typecode())
    {    
        float val;
        while(n--)
        {
            in.read((char*)&val, sizeof(float));
#ifdef LITTLEENDIAN
            endianswap(&val);
#endif
            *x++ = double(val);
        }
    }
    else
        PLERROR("In binread_ incompatible typecode");
}



void binread_(PStream& in, float* x, unsigned int n, unsigned char typecode)
{ 
    if(typecode==TypeTraits<float>::little_endian_typecode())
    {
        in.read((char*)x, streamsize(n*sizeof(float))); 
#ifdef BIGENDIAN
        endianswap(x,n); 
#endif      
    }
    else if(typecode==TypeTraits<float>::big_endian_typecode())
    {
        in.read((char*)x, streamsize(n*sizeof(float))); 
#ifdef LITTLEENDIAN
        endianswap(x,n); 
#endif
    }
    else if(typecode==TypeTraits<double>::little_endian_typecode())
    {    
        double val;
        while(n--)
        {
            in.read((char*)&val, sizeof(double));
#ifdef BIGENDIAN
            endianswap(&val);
#endif
            *x++ = float(val);
        }
    }
    else if(typecode==TypeTraits<double>::big_endian_typecode())
    {    
        double val;
        while(n--)
        {
            in.read((char*)&val, sizeof(double));
#ifdef LITTLEENDIAN
            endianswap(&val);
#endif
            *x++ = float(val);
        }
    }
    else
        PLERROR("In binread_ incompatible typecode");
}

} //end of namespace PLearn


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
