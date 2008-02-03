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
   * $Id$
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearn/plearn/display/GhostScript.h */

#ifndef GHOSTSCRIPT_INC
#define GHOSTSCRIPT_INC

#include <plearn/io/PStream.h>
#include <plearn/math/Mat.h>

namespace PLearn {
using namespace std;

/*!     These are used to pack and unpack r,g,b triplets to and from a single
    real. As usual in postscript, r,g and b are values between 0 and 1
    indicating quantity of light.  
*/
  real rgb2real(real r, real g, real b);
  void real2rgb(real colorval, real& r, real& g, real& b);

class GhostScript
{
 protected:
  FILE* gs_cstream;
  PStream togs; 
  char command_string[1000];
  bool isfile;
  static void writeBitmapHexString1Bit(const Mat& bm, PStream& out, bool lastrowfirst=false);
  static void writeBitmapHexString8Bits(const Mat& bm, PStream& out, bool lastrowfirst=false);
  static void writeBitmapHexString24Bits(const Mat& bm, PStream& out, bool lastrowfirst=false);

// norman: win32 compiler knows ios::char_type! :)
#if __GNUC__ > 2 && !defined(WIN32)
  typedef ios::char_type char_type;
#else
  typedef ios::char_type char_type;
#endif 
// old:
//#if __GNUC__ < 3
//  typedef _IO_wchar_t char_type;
//#else
//  typedef ios::char_type char_type;
//#endif

 public:

  //!  opens an X11 ghostscript window
  GhostScript(int width=200, int height=200);

  //!  creates an eps file with the (x1,y1;x2,y2) bounding box
  GhostScript(const string& filename, real x1, real y1, real x2, real y2);

  ~GhostScript();

  void flush();

  PStream& operator<<(char* str)
    { return togs<<str; }

  void run(const char* filename)
    { togs << "\n(" << filename << ") run " << endl; }

  void copypage() 
    { togs << "\ncopypage" << endl; } 

  void showpage() 
    { togs << "\nshowpage" << endl; } 

  void gsave()
    { togs << "\ngsave\n"; }

  void grestore()
    { togs << "\ngrestore\n"; }

  void translate(real x, real y)
    { togs << "\n" << x << " " << y << " translate\n"; }

  void scale(real x, real y)
    { togs << "\n" << x << " " << y << " scale\n"; }

  void moveto(real x, real y)
    { togs << "\n" << x << " " << y << " moveto\n"; }

  void rotate(real angle)
    { togs << "\n" << angle << " rotate\n"; }

/*!     transforms coordinate system such that the source rectangle will be mapped to
    the destination rectangle (i.e. you can then use the source coordinate system to
    do your drawings, and they will be rendered in the destination rectangle).
*/
  void mapping(real sourcex, real sourcey, real sourcewidth, real sourceheight,
           real destx, real desty, real destwidth, real destheight)
    {
      scale(destwidth/sourcewidth, destheight/sourceheight);
      translate(-sourcex+destx*sourcewidth/destwidth,-sourcey+desty*sourceheight/destheight);
    }

  void show(const char* str)
  { togs << "(" << str << ") show" << endl; }

  void show(const string& str)
  { togs << "(" << str << ") show" << endl; }

  //! halign can be 'l' (left), 'c' (center) or 'r' (right)
  //! valign can be 't' (top), 'm' (middle) or 'b' (bottom) 
  void show(real x, real y, const char* str, char halign='l', char valign='b');

  inline void show(real x, real y, const string& str, char halign='l', char valign='b')
  { show(x,y,str.c_str(), halign, valign); }

  void centerShow(real x, real y, const char* str)
  { show(x,y,str,'c','m'); }

  void centerShow(real x, real y, const string& str)
  { show(x,y,str,'c','m'); }

  //! accepts multiple line text (lines separated by \n) 
  void multilineShow(real x, real y, const string& text, real newlinesize, char halign='l', char valign='b');

  void show(real x, real y, Vec v) 
  { togs << "\n" << x << " " << y << " moveto (" << v << ") show" << endl; }

  void centerShow(real x, real y, Vec v) 
    { togs << "\n" << x << " " << y << " moveto (" << v << ") dup stringwidth exch -2 div exch -2 div rmoveto show" << endl; } 

  void erasepage()
    { togs << "\nerasepage" << endl; }

  void setcolor(real r, real g, real b)
    { togs << "\n/DeviceRGB setcolorspace " << r << " " << g << " " << b << " setcolor\n"; }

  //! takes color in rgb real as returned by rgb2real
  void setcolor(real rgb)
    { 
      real r,g,b;
      real2rgb(rgb,r,g,b);
      setcolor(r,g,b);
    }

  void setcolor(char* colorname);

  void setgray(real g)
    { togs << "\n" << g << " setgray\n"; }
  
  void setlinewidth(real w)
    { togs << "\n" << w << " setlinewidth\n"; }

  //!  sets the dash pattern: ary contains the cyclic on-off distances of the dashes, which begin after an offset_of x units from the current point
  void setdash(Vec ary, real x=0)
  { 
    if(ary.isEmpty())
      togs << "[ 3 3 ] " << x << " setdash\n"; 
    else
      togs << "[ " << ary << " ] " << x << " setdash\n"; 
  }

  void drawLine(real x1, real y1, real x2, real y2)
    { togs << "\n" << x1 << " " << y1 << " moveto " << x2 << " " << y2 << " lineto stroke" << endl; }

  void drawArrow(real x1, real y1, real x2, real y2)
    { 
      drawLine(x1,y1,x2,y2);
      drawCircle(x2,y2,2);
    }

  void drawBox(real x, real y, real width, real height);
  void fillBox(real x, real y, real width, real height);

  void drawCross(real x, real y, real r, bool vertical=false, real ry=-1); // default ry=r

  void drawCircle(real x, real y, real r);
  void fillCircle(real x, real y, real r);

/*!     if painton1 is true, then all matrix cells that are different from 0
    will be painted in the current color. If painton1 is false, then all
    cells that are equal to 0 will be painted
*/
  void displayBlack(const Mat& bm, real x, real y, real w, real h, bool painton1=true);

  //!  values in bm indicate quantity of light between 0.0 (black) and 1.0 (white)
  void displayGray(const Mat& bm, real x, real y, real w, real h);

  //!  values in bm are those obtained using the rgb2real function (see above)
  void displayRGB(const Mat& bm, real x, real y, real w, real h);

  void usefont(const char* fontname="Times-Roman", real pointsize=12.0) 
    { togs << "\n/" << fontname << " findfont " << pointsize << " scalefont setfont" << endl; }
};

} // end of namespace PLearn

#endif










