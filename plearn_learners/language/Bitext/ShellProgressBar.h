// -*- C++ -*-

//
// Copyright (C) 2004 Universite de Montreal
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
 * $Id: ShellProgressBar.h,v 1.4 2004/07/21 16:30:56 chrish42 Exp $
******************************************************* */


#ifndef ShellProgressBar_H
#define ShellProgressBar_H
#include <string>
#include <plearn/math/pl_math.h> //!< For real.

namespace PLearn {
using namespace std;

//
// This class implements a simple and convenient ASCII
// text progress bar to be displayed in the console.
//

// - Position the Cursor:
//   \033[<L>;<C>H
//      Or
//      \033[<L>;<C>f
//   puts the cursor at line L and column C.
//           - Move the cursor up N lines:
//   \033[<N>A
//        - Move the cursor down N lines:
//   \033[<N>B
//        - Move the cursor forward N columns:
//   \033[<N>C
//        - Move the cursor backward N columns:
//   \033[<N>D
//        - Clear the screen, move to (0,0):
//   \033[2J
//        - Erase to end of line:
//   \033[K
//        - Save cursor position:
//   \033[s
//        - Restore cursor position:
//   \033[u

class ShellProgressBar
{

public:

  ShellProgressBar() : min(0), max(0), caption(""), width(10) {}
  ShellProgressBar(int min, int max, string caption = "", int width = 10);

  void init();
  void draw(bool simple_mode = false); // must be called before any update;
                                       // simple mode can be used when outputting in file,
                                       // or when using 'gdb', for instance 
                                       // (it simply won't perform the positional trick on the cursor)
  bool update(int value);
  void reset();
  void set(int min, int max, string caption = "", int w = 10) { setMin(min); setMax(max); setCaption(caption); width = w; }
  void setCaption(string caption);
  void setMin(int min);
  void setMax(int max);
  void done();
  static string getTime();
  static int getAsciiFileLineCount(string file); // stupid version!
  static int getWcAsciiFileLineCount(string file); // this is just for convenience : it is a bit risky!

private:

  int min;
  int max;
  real blockwidth;
  int pos;
  string caption;
  int width;
  bool max_reached;

};

} // end of namespace PLearn

#endif

