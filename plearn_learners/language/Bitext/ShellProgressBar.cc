#include "ShellProgressBar.h"

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


