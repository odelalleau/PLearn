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
 * $Id: ShellProgressBar.cc,v 1.2 2004/04/05 13:19:57 tihocan Exp $
******************************************************* */


#include "ShellProgressBar.h"
#include "stringutils.h" 
#include <fstream>
#include "Popen.h" //!< For execute.

namespace PLearn {
using namespace std;

ShellProgressBar::ShellProgressBar(int min, int max, string caption, int width) 
  : min(min), max(max), caption(caption), width(width)
{
  init();
}

void ShellProgressBar::init()
{
  blockwidth = (real)(max - min) / width;
  pos = 0;
  max_reached = false;
}

void ShellProgressBar::draw(bool simple_mode)
{
  if (simple_mode)
  {
    cout << caption << " " << getTime() << " ";
    cout.flush();
  } else
  {
    cout << caption << " " << getTime() << " |";
    for (int i = 0;i < width; i++) cout << " ";
    cout <<"|";
    string move_cursor_left = "\033[" + tostring(width + 1) + "D";
    cout << move_cursor_left;
    cout.flush();
  }
}

bool ShellProgressBar::update(int value)
{
  if (value < min || max_reached)
  {
    return false;
  } else if (value >= max)
  {
    for (int i = pos; i < width; i++)
    {
      cout << "=";
      cout.flush();
    }
    max_reached = true;
    return true;
  }

  int inc = (int)((value - min) / blockwidth);

  int i;
  for (i = pos; i < inc; i++)
  {
    cout << "=";
    cout.flush();
  }
  pos = i;

  return true;
}

void ShellProgressBar::reset()
{
  max_reached = false;
  pos = 0;
}

void ShellProgressBar::setCaption(string the_caption)
{
  caption = the_caption;
}

void ShellProgressBar::setMin(int the_min)
{
  min = the_min;
}

void ShellProgressBar::setMax(int the_max)
{
  max = the_max;
}

void ShellProgressBar::done()
{
  if (!max_reached)
    update(max);
  cout << "\033[2C" << getTime() << endl;
}

int ShellProgressBar::getAsciiFileLineCount(string file)
{
  // not terribly efficient, I fear, but I don't 
  // have the time for a better solution (the one
  // with Popen crashes with a MPI program...)
  ifstream in(file.c_str());
  int n_lines = 0;
  while (in)
  {
    pgetline(in);
    n_lines++;
  }
  in.close();
  return n_lines;
}

int ShellProgressBar::getWcAsciiFileLineCount(string file)
{
  string wc = "wc -l " + file;
  vector<string> answer = execute(wc);
  return toint(answer[0]);
}

string ShellProgressBar::getTime()
{
  time_t tt;
  time(&tt);
  string time_str(ctime(&tt));
  vector<string> tokens = split(time_str);
  return "[" + tokens[3] + "]";
}

} // end of namespace PLearn


