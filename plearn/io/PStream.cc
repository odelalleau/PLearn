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

#include <iomanip>
#include "PStream.h"
#include "stringutils.h"
#include "fileutils.h" 

namespace PLearn <%
using namespace std;

  //! default ctor: the stream is unusable ...
PStream::PStream()
    :pin(0), pout(0), own_pin(false), own_pout(false), 
     option_flags_in(dft_option_flag), inmode(plearn_ascii), 
     option_flags_out(dft_option_flag), outmode(plearn_ascii), 
     original_bufin(0), original_bufout(0), 
     implicit_storage(true), compression_mode(compr_none)
  {}
  //! ctor. from an istream (I)
PStream::PStream(istream* pin_)
    :pin(pin_), pout(0), own_pin(false), own_pout(false), 
     option_flags_in(dft_option_flag), inmode(plearn_ascii), 
     option_flags_out(dft_option_flag), outmode(plearn_ascii),
     original_bufin(pin_->rdbuf()), original_bufout(0), 
     implicit_storage(true), compression_mode(compr_none)
  { initInBuf(); }
  //! ctor. from an ostream (O)

PStream::PStream(ostream* pout_)
    :pin(0), pout(pout_), own_pin(false), own_pout(false), 
     option_flags_in(dft_option_flag), inmode(plearn_ascii), 
     option_flags_out(dft_option_flag), outmode(plearn_ascii),
     original_bufin(0), original_bufout(pout_->rdbuf()), 
     implicit_storage(true), compression_mode(compr_none)
  {}

  //! ctor. from an iostream (IO)
PStream::PStream(iostream* pios_)
    :pin(pios_), pout(pios_), own_pin(false), own_pout(false), 
     option_flags_in(dft_option_flag), inmode(plearn_ascii), 
     option_flags_out(dft_option_flag), outmode(plearn_ascii),
     original_bufin(pios_->rdbuf()), original_bufout(pios_->rdbuf()),
     implicit_storage(true), compression_mode(compr_none)
  { initInBuf(); }

  //! ctor. from an istream and an ostream (IO)
  PStream::PStream(istream* pin_, ostream* pout_)
    :pin(pin_), pout(pout_), own_pin(false), own_pout(false), 
     option_flags_in(dft_option_flag), inmode(plearn_ascii), 
     option_flags_out(dft_option_flag), outmode(plearn_ascii),
     original_bufin(pin_->rdbuf()), original_bufout(pout_->rdbuf())
  { initInBuf(); }

//! copy ctor.
  PStream::PStream(const PStream& pios)
    :the_inbuf(pios.the_inbuf), the_fdbuf(pios.the_fdbuf),
     pin(pios.pin), pout(pios.pout), own_pin(false), own_pout(false), 
     option_flags_in(pios.option_flags_in), inmode(pios.inmode), 
     option_flags_out(pios.option_flags_out), outmode(pios.outmode), 
     original_bufin(pios.original_bufin), original_bufout(pios.original_bufout)
  {}

//! dtor.
PStream::~PStream()
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
  
void PStream::initInBuf()
  {
    if(pin)
      {
        the_inbuf= dynamic_cast<pl_streambuf*>(pin->rdbuf());
        if(the_inbuf.isNull())
          the_inbuf= new pl_streambuf(*pin->rdbuf());
        pin->rdbuf(the_inbuf);
      }
  }

// reads everything until '\n' (also consumes the '\n')
void PStream::skipRestOfLine()
{
  int c=get();
  while(c!='\n' && c!=EOF)
    c=get();
}

void PStream::skipBlanks()
{
  int c = get();
  while(c!=EOF && (c==' ' || c=='\t' || c=='\n' || c=='\r'))
    c = get();
  if(c!=EOF)
    unget();
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
    unget();
}

void PStream::skipBlanksAndCommentsAndSeparators()
{
  int c = get();
  while(c!=EOF)
    { 
      if(c=='#')
        skipRestOfLine();     
      else if(c!=' ' && c!='\t' && c!='\n' && c!='\r' && c!=';' && c!=',' && c!=':')
        break;
      c = get();
    }
  if(c!=EOF)
    unget();
}

PStream& PStream::operator()(istream* pin_)
{ 
  if(pin && original_bufin) pin->rdbuf(original_bufin);
  if(pin && original_bufout) pin->rdbuf(original_bufout);
  if(own_pin) delete pin;
  if(own_pout) delete pout;
  pin= pin_;
  pout= 0;
  own_pin= false;
  own_pout= false;
  option_flags_in= dft_option_flag;
  inmode = plearn_ascii; 
  option_flags_out= dft_option_flag;
  outmode = plearn_ascii; 
  the_fdbuf= 0;
  original_bufin= pin->rdbuf();
  original_bufout= 0;
  initInBuf(); 
  return *this;
}

PStream& PStream::operator()(ostream* pout_)
{ 
  if(pin && original_bufin) pin->rdbuf(original_bufin);
  if(pin && original_bufout) pin->rdbuf(original_bufout);
  if(own_pin) delete pin;
  if(own_pout) delete pout;
  pin= 0;
  pout= pout_;
  own_pin= false;
  own_pout= false;
  option_flags_in= dft_option_flag;
  inmode = plearn_ascii; 
  option_flags_out= dft_option_flag;
  outmode = plearn_ascii; 
  the_fdbuf= 0;
  the_inbuf= 0;
  original_bufin= 0;
  original_bufout= pout->rdbuf();
  return *this;
}

PStream& PStream::operator()(iostream* pios_)
{ 
  if(pin && original_bufin) pin->rdbuf(original_bufin);
  if(pin && original_bufout) pin->rdbuf(original_bufout);
  if(own_pin) delete pin;
  if(own_pout) delete pout;
  pin= pios_;
  pout= pios_;
  own_pin= false;
  own_pout= false;
  option_flags_in= dft_option_flag;
  inmode = plearn_ascii; 
  option_flags_out= dft_option_flag;
  outmode = plearn_ascii; 
  the_fdbuf= 0;
  original_bufin= pin->rdbuf();
  original_bufout= pout->rdbuf();
  initInBuf(); 
  return *this;
}

PStream& PStream::operator()(istream* pin_, ostream* pout_)
{ 
  if(pin && original_bufin) pin->rdbuf(original_bufin);
  if(pin && original_bufout) pin->rdbuf(original_bufout);
  if(own_pin) delete pin;
  if(own_pout) delete pout;
  pin= pin_;
  pout= pout_;
  own_pin= false;
  own_pout= false;
  option_flags_in= dft_option_flag;
  inmode = plearn_ascii; 
  option_flags_out= dft_option_flag;
  outmode = plearn_ascii; 
  the_fdbuf= 0;
  original_bufin= pin->rdbuf();
  original_bufout= pout->rdbuf();
  initInBuf(); 
  return *this;
}

PStream& PStream::operator()(const PStream& pios)
{ 
  if(this != &pios)
    {
      if(pin && original_bufin) pin->rdbuf(original_bufin);
      if(pout && original_bufout) pout->rdbuf(original_bufout);
      if(own_pin) delete pin;
      if(own_pout) delete pout;
      pin= pios.pin;
      pout= pios.pout;
      own_pin= false;
      own_pout= false;
      option_flags_in= pios.option_flags_in;
      inmode= pios.inmode;
      option_flags_out= pios.option_flags_out;
      outmode= pios.outmode;
      the_inbuf= pios.the_inbuf;
      the_fdbuf= pios.the_fdbuf;
      original_bufin= pios.original_bufin;
      original_bufout= pios.original_bufout;
    }
  return *this;
}




PStream& PStream::operator>>(char &x)
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
 
PStream& PStream::operator>>(signed char &x)
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

PStream& PStream::operator>>(unsigned char &x)
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

PStream& PStream::operator>>(int &x)
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

PStream& PStream::operator>>(unsigned int &x)
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
  
PStream& PStream::operator>>(long int &x)
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
  
PStream& PStream::operator>>(unsigned long &x)
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
  
PStream& PStream::operator>>(short int &x)
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
  
PStream& PStream::operator>>(unsigned short &x)
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

// Implementation of operator<<'s

PStream& PStream::operator<<(char x) 
{ 
  switch(outmode)
    {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
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

PStream& PStream::operator<<(signed char x) 
{ 
  switch(outmode)
    {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
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

PStream& PStream::operator<<(unsigned char x) 
{ 
  switch(outmode)
    {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
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

PStream& PStream::operator<<(int x) 
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
    case PStream::pretty_ascii:
    case PStream::plearn_binary:
      rawout() << x << ' ';
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
    case PStream::raw_ascii:
      rawout() << x;
      break;
    case PStream::raw_binary:
      write(reinterpret_cast<char *>(&x), sizeof(x));
      break;
    case PStream::plearn_ascii:
    case PStream::pretty_ascii:
    case PStream::plearn_binary:
      rawout() << x << ' ';
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
    case PStream::raw_ascii:
      rawout() << x;
      break;
    case PStream::raw_binary:
      write(reinterpret_cast<char *>(&x), sizeof(x));
      break;
    case PStream::plearn_ascii:
    case PStream::pretty_ascii:
    case PStream::plearn_binary:
      rawout() << x << ' ';
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
    case PStream::raw_ascii:
      rawout() << x;
      break;
    case PStream::raw_binary:
      write(reinterpret_cast<char *>(&x), sizeof(x));
      break;
    case PStream::plearn_ascii:
    case PStream::pretty_ascii:
    case PStream::plearn_binary:
      rawout() << x << ' ';
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
    case PStream::raw_ascii:
      rawout() << x;
      break;
    case PStream::raw_binary:
      write(reinterpret_cast<char *>(&x), sizeof(x));
      break;
    case PStream::plearn_ascii:
    case PStream::pretty_ascii:
    case PStream::plearn_binary:
      rawout() << x << ' ';
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
    case PStream::raw_ascii:
      rawout() << x;
      break;
    case PStream::raw_binary:
      write(reinterpret_cast<char *>(&x), sizeof(x));
      break;
    case PStream::plearn_ascii:
    case PStream::pretty_ascii:
    case PStream::plearn_binary:
      rawout() << x << ' ';
      break;
    default:
      PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
      break;
    }
  return *this;
}

PStream& PStream::operator<<(bool x) 
{ 
  switch(outmode)
    {
    case raw_ascii:
    case raw_binary:
      rawout() << (x?'1':'0');
      break;
    case plearn_ascii:
    case pretty_ascii:
    case plearn_binary:
      rawout() << (x?'1':'0') << ' ';
      break;
    default:
      PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
      break;
    }
  return *this;
}



//! attach: "attach" the PStream to a POSIX file descriptor.
void PStream::attach(int fd)
{
  the_fdbuf= new pl_fdstreambuf(fd, pl_dftbuflen);
  the_inbuf= new pl_streambuf(*the_fdbuf);
  if(pin)
    pin->rdbuf(the_inbuf);
  else
    {
      own_pin= true;
      pin= new istream(the_inbuf);
    }
  if(pout) 
    pout->rdbuf(the_fdbuf);
  else
    {
      own_pout= true;
      pout= new ostream(the_fdbuf);
    }
}

/***************************
 * op>>'s
 ***************************/

PStream& PStream::operator>>(bool& x)
{
  if(inmode==PStream::raw_ascii)// std c++ istream format
    rawin() >> x;
  else //human-readable serialization format
    { 
      rawin() >> ws;
      int c = get();
      char tmp[10];
      switch(c)
        {
        case '0':
          x= false;
          break;
        case '1':
          x = true;
          break;
        case 't':
          c = peek();
          if(!isalpha(c))
            {
              unget();
              x = true;
              break;
            }
          rawin().get(tmp,4);
          if(strcmp(tmp,"rue")!=0 || isalpha(peek()))
            PLERROR("In PStream::operator>>(bool&), wrong format, not a bool");
          x = true;
          break;
        case 'f':
          c = peek();
          if(!isalpha(c))
            {
              unget();
              x = false;
              break;
            }
          rawin().get(tmp,5);
          if(strcmp(tmp,"alse")!=0 || isalpha(peek()))
            PLERROR("In PStream::operator>>(bool&), wrong format, not a bool");
          x = false;
          break;
        default:
          PLERROR("In PStream::operator>>(bool&), wrong format, not a bool, first character read was %c",c);
        }
      if(!isspace(get())) 
        unget();
    }
  
  return *this;
}

PStream& PStream::operator>>(float &x)
{
  switch(inmode)
    {
    case PStream::raw_ascii:
      rawin() >> x;
      break;
    case PStream::raw_binary:
      read(reinterpret_cast<char *>(&x), sizeof(x));
      break;
    case PStream::plearn_ascii:
    case PStream::plearn_binary:
      {
        skipBlanksAndComments();
        int c = get();
        if(c=='n' || c=='N')
          {
            if(get()=='a' && get()=='n')
              x = MISSING_VALUE;
            else
              PLERROR("Bug while reading file and expecting a float");
          }
        else
          {
            unget();
            rawin() >> x; 
          }
        if(!isspace(get()))
          unget();
      }
      break;
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
    case PStream::raw_ascii:
      rawin() >> x;
      break;
    case PStream::raw_binary:
      read(reinterpret_cast<char *>(&x), sizeof(x));
      break;
    case PStream::plearn_ascii:
    case PStream::plearn_binary:
      { 
        skipBlanksAndComments();
        int c = peek();
        switch(c)
          {
          case '?':
            get();
            x = MISSING_VALUE;
            break;
          case 'n':
          case 'N':
            get();
            if(get()=='a' && get()=='n')
              x = MISSING_VALUE;
            else
              PLERROR("Bug while reading file and expecting a double");
            break;
          default:
            rawin() >> x; 
          }
      }
      break;
    default:
      PLERROR("In PStream::operator>>  unknown inmode!!!!!!!!!");
      break;
    }
  
  return *this;
}


PStream& PStream::operator>>(char *x)
{
  switch(inmode)
    {
    case PStream::raw_ascii:
    case PStream::raw_binary:
      rawin() >> x;
      break;
    case PStream::plearn_ascii:
    case PStream::plearn_binary:
      {
        if(!x)
          PLERROR("In PStream::operator>>(char*) character array must already be allocated to put the read string in");
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
                  c = get();
                x[i++]= static_cast<char>(c);
                c = get();
              }
            if(c==EOF)
              PLERROR("In read(istream&, string&) unterminated quoted string");
            if(!isspace(c))
              unget();
          }
        else // it's a single word without quotes
          {
            c= get();
            while(c != EOF && wordseparators.find(c)==string::npos) // as long as we don't meet a wordseparator (or eof)...
              {
                x[i++]= static_cast<char>(c);
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
  
PStream& PStream::operator>>(string &x)
{
  switch(inmode)
    {
    case PStream::raw_ascii:
    case PStream::raw_binary:
      rawin() >> x;
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
                  c = get();
                x+= static_cast<char>(c);
                c = get();
              }
            if(c==EOF)
              PLERROR("In read(istream&, string&) unterminated quoted string");
            if(!isspace(get())) // skip following blank if any
              unget();
          }
        /* NO LONGER SUPPORTED
        else if(isdigit(c)) // format is the old <nchars> <stringval>
          {
            int l; 
            rawin() >> l; 
            get(); // skip space
            x.resize(l); 
            for(int i=0; i<l; i++)
              x[i] = get();
            if(!isspace(get())) // skip following blank
              unget();
          }
        */
        else // it's a single word without quotes
          {
            x.resize(0);      
            c= get();
            while(c != EOF && wordseparators.find(c)==string::npos) // as long as we don't meet a wordseparator (or eof)...
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

/***************************
 * op<<'s
 ***************************/


PStream& PStream::operator<<(float x)
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
    case PStream::pretty_ascii:
    case PStream::plearn_binary:
      { 
        if(is_missing(x))
          rawout() << "nan ";
        else
          rawout() << setprecision(7) << x << ' '; 
      }
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
    case PStream::raw_ascii:
      rawout() << x;
      break;
    case PStream::raw_binary:
      write(reinterpret_cast<char *>(&x), sizeof(x));
      break;
    case PStream::plearn_ascii:
    case PStream::pretty_ascii:
    case PStream::plearn_binary:
      { 
        if(is_missing(x))
          rawout() << "nan ";
        else
          rawout() << setprecision(17) << x << ' '; 
      }
      break;
    default:
      PLERROR("In PStream::operator<<  unknown outmode!!!!!!!!!");
      break;
    }
  return *this;
}

PStream& PStream::operator<<(const char *x)
{
  switch(outmode)
    {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
    case PStream::raw_binary:
      rawout() << x;
      break;
    case PStream::plearn_ascii:
    case PStream::plearn_binary:
      {
        put('"');
        int l = strlen(x);
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

PStream& PStream::operator<<(const string &x)
{
  switch(outmode)
    {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
    case PStream::raw_binary:
      rawout() << x;
      break;
    case PStream::plearn_ascii:
    case PStream::plearn_binary:
      { 
        put('"');
        int l = x.length();
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
      in.read((char*)x, n*sizeof(BASETYPE));           \
      if(byte_order()==BIG_ENDIAN_ORDER)               \
        endianswap(x,n);                               \
    }                                                  \
  else if(typecode==TypeTraits<BASETYPE>::big_endian_typecode()) \
    {                                                  \
      in.read((char*)x, n*sizeof(BASETYPE));           \
      if(byte_order()==LITTLE_ENDIAN_ORDER)            \
        endianswap(x,n);                               \
    }                                                  \
  else                                                 \
    PLERROR("In binread_ incompatible typecode");      \
}


// IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(char);
IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(short);
IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(unsigned short);
IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(int);
IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(unsigned int);
IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(long);
IMPLEMENT_TYPICAL_BASETYPE_BINREAD_(unsigned long);

//! The binread_ for float and double are special

void binread_(PStream& in, double* x, unsigned int n, unsigned char typecode)
{ 
  if(typecode==TypeTraits<double>::little_endian_typecode())
    {
      in.read((char*)x, n*sizeof(double)); 
#ifdef BIGENDIAN
      endianswap(x,n); 
#endif      
    }
  else if(typecode==TypeTraits<double>::big_endian_typecode())
    {
      in.read((char*)x, n*sizeof(double)); 
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
      in.read((char*)x, n*sizeof(float)); 
#ifdef BIGENDIAN
      endianswap(x,n); 
#endif      
    }
  else if(typecode==TypeTraits<float>::big_endian_typecode())
    {
      in.read((char*)x, n*sizeof(float)); 
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



%> //end of namespace PLearn
