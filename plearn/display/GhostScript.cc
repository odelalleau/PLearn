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


 

/* *******************************************************      
   * $Id: GhostScript.cc,v 1.2 2002/09/17 01:27:33 zouave Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */


#include "GhostScript.h"

namespace PLearn <%
using namespace std;

  real rgb2real(real r, real g, real b)
  { 
    int r100 = int(r*99.99);
    int g100 = int(g*99.99);
    int b100 = int(b*99.99);
    return real(r100*10000+g100*100+b100);
  }

  void real2rgb(real colorval, real& r, real& g, real& b)
  {
    int col = int(colorval);
    int r100 = col/10000;
    col = col%10000;
    int g100 = col/100;
    col = col%100;
    int b100 = col;
    r = real(r100)/100.0;
    g = real(g100)/100.0;
    b = real(b100)/100.0;
  }

  GhostScript::GhostScript(int width, int height)
  {
    static char command_string[1000];
    // sprintf(command_string,"gs -sDEVICE=x11 -g%d%c%d - > /dev/null",width,'x',height);
    sprintf(command_string,"gs -sDEVICE=x11 -g%d%c%d > /dev/null",width,'x',height);
    gs_cstream = popen(command_string,"w");
    togs.attach(fileno(gs_cstream));
    togs.outmode=PStream::raw_ascii;
  }

  GhostScript::GhostScript(const string& filename, real x1, real y1, real x2, real y2)
    // :gs_cstream(0), togs(filename.c_str())
  {
    gs_cstream= fopen(filename.c_str(), "w");
    togs.attach(fileno(gs_cstream));
    togs.outmode=PStream::raw_ascii;
    togs << "%!PS-Adobe-2.0 EPSF-2.0" << endl;
    togs << "%%BoundingBox: " << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
  }

  GhostScript::~GhostScript()
  { 
    if(gs_cstream)
      {
        togs << "\nquit" << endl; 
        fclose(gs_cstream);
      }
    else
      {
        // togs << "showpage" << endl;
      }
  }

  void GhostScript::flush() 
  { 
    togs.flush(); 
    if(gs_cstream) 
      copypage();
  }

  void GhostScript::writeBitmapHexString1Bit(const Mat& bm, PStream& out, bool lastrowfirst)
  {
    GhostScript::ioflag_type oldflags = out.flags_out();
    out.unsetf_out(ios::dec);
    out.setf_out(ios::hex);
    GhostScript::char_type oldfill = out.fill_out('0');
    unsigned char bitmask = 128;
    unsigned char value = 0;
    for(int i=0; i<bm.length(); i++)
      {
        const real* bm_i = lastrowfirst ?bm.rowdata(bm.length()-1-i) :bm.rowdata(i);
        for(int j=0; j<bm.width(); j++)
          {
            if(bm_i[j]>0)
              value |= bitmask;
            bitmask = bitmask>>1;
            if(bitmask==0)
              {
                out.width_out(2);
                out << (unsigned int)value << " ";
                bitmask = 128;
                value = 0;
              }
          }
        if(bitmask!=128)
          {
            out.width_out(2);
            out << (unsigned int)value;
            bitmask = 128;
            value = 0;
          }
      }
    out.flags_out(oldflags);
    out.fill_out(oldfill);
  }

  void GhostScript::writeBitmapHexString8Bits(const Mat& bm, PStream& out, bool lastrowfirst)
  {
    GhostScript::ioflag_type oldflags = out.flags_out();
    out.unsetf_out(ios::dec);
    out.setf_out(ios::hex);
    char_type oldfill = out.fill_out('0');
    for(int i=0; i<bm.length(); i++)
      {
        const real* bm_i = lastrowfirst ?bm.rowdata(bm.length()-1-i) :bm.rowdata(i);
        for( int j=0; j<bm.width(); j++)
          {
            out.width_out(2);
            unsigned int value = (unsigned int)(bm_i[j]*255.99);
            out << value;
          }
        out << "\n";
      }
    out.flags_out(oldflags);
    out.fill_out(oldfill);
  }
  
  void GhostScript::writeBitmapHexString24Bits(const Mat& bm, PStream& out, bool lastrowfirst)
  {
    GhostScript::ioflag_type oldflags = out.flags_out();
    out.unsetf_out(ios::dec);
    out.setf_out(ios::hex);
    char_type oldfill = out.fill_out('0');

    for(int i=0; i<bm.length(); i++)
      {
        const real* bm_i = lastrowfirst ?bm.rowdata(bm.length()-1-i) :bm.rowdata(i);
        for( int j=0; j<bm.width(); j++)
          {
            real r,g,b; // values between 0 and 255
            real2rgb(bm_i[j],r,g,b);
            out.width_out(2);
            out << (unsigned int)(r*255.99);
            out.width_out(2);
            out << (unsigned int)(g*255.99);
            out.width_out(2);
            out << (unsigned int)(b*255.99);
          }
        out << "\n";
      }

    out.flags_out(oldflags);
    out.fill_out(oldfill);
  }
  
  void GhostScript::displayBlack(const Mat& bm, real x, real y, real w, real h, bool painton1)
  {
    togs << "\ngsave\n" 
         << "/pstr " << bm.width() << " string def\n"
         << x << " " << y << " translate\n"
         << w << " " << h << " scale\n" 
         << bm.width() << " " << bm.length() << " " << (painton1 ?"true" :"false") << " "
         << "[" << bm.width() << " 0 0 " << bm.length() << " 0 0 ]\n";
    // * Version using huge string
    togs << "<";
    writeBitmapHexString1Bit(bm,togs,true);
    togs << ">\nimage\ngrestore" << endl;
    // * Version using procedure
    // togs << "{currentfile pstr readhexstring pop} imagemask\n";
    // writeBitmapHexString1Bit(bm,togs,true);
    // togs << "\ngrestore\n";
  }

  void GhostScript::displayGray(const Mat& bm, real x, real y, real w, real h)
  {
    togs << "\ngsave\n" 
         << "/pstr " << bm.width() << " string def\n"
         << x << " " << y << " translate\n"
         << w << " " << h << " scale\n" 
         << bm.width() << " " << bm.length() << " 8 "
         << "[" << bm.width() << " 0 0 " << bm.length() << " 0 0 ]\n";
    // * Version using huge string
    togs << "<";
    writeBitmapHexString8Bits(bm,togs,true);
    togs << ">\nimage\ngrestore" << endl;
    // * Version using procedure
    // togs << "{currentfile pstr readhexstring pop} image\n";
    // writeBitmapHexString8Bits(bm,togs,true);
    // togs << "\ngrestore\n";
  }

  void GhostScript::displayRGB(const Mat& bm, real x, real y, real w, real h)
  {
    togs << "\ngsave\n" 
         << "/pstr " << 3*bm.width() << " string def\n"
         << x << " " << y << " translate\n"
         << w << " " << h << " scale\n" 
         << bm.width() << " " << bm.length() << " 8 "
         << "[" << bm.width() << " 0 0 " << bm.length() << " 0 0 ]\n";
    // * Version using huge string
    // togs << "<";
    // writeBitmapHexString24Bits(bm,togs);
    // togs << ">\nfalse 3 image\ngrestore" << endl;
    // * Version using procedure
    togs << "{currentfile pstr readhexstring pop} false 3 colorimage\n";
    writeBitmapHexString24Bits(bm,togs,true);
    togs << "\ngrestore\n";
  }

  void GhostScript::drawBox(real x, real y, real width, real height)
  { 
    togs << "\n" << x << " " << y << " moveto "
         << x+width << " " << y << " lineto "
         << x+width << " " << y+height << " lineto "
         << x << " " << y+height << " lineto "
         << x << " " << y << " lineto "
         << "stroke" << endl;
  }

  void GhostScript::fillBox(real x, real y, real width, real height)
  { 
    togs << "\n" << x << " " << y << " moveto "
         << x+width << " " << y << " lineto "
         << x+width << " " << y+height << " lineto "
         << x << " " << y+height << " lineto "
         << x << " " << y << " lineto "
         << "fill" << endl;
  }

  void GhostScript::drawCircle(real x, real y, real r)
  {
    moveto(x+r, y);
    togs << x << ' ' << y << ' ' << r << " 0 360 arc stroke" << endl; 
  }

  void GhostScript::drawCross(real x, real y, real r, bool vertical)
  {
    if(vertical)
      {
        drawLine(x-r,y,x+r,y);
        drawLine(x,y-r,x,y+r);
      }
    else // diagonal
      {
#if defined(_MINGW_)
        r /= sqrt(2.0);
#else
        r /= M_SQRT2;
#endif
        drawLine(x-r,y-r,x+r,y+r);
        drawLine(x-r,y+r,x+r,y-r);
      }
  }

  void GhostScript::fillCircle(real x, real y, real r)
  {
    moveto(x+r, y);
    togs << x << ' ' << y << ' ' << r << " 0 360 arc fill" << endl; 
  }




%> // end of namespace PLearn
