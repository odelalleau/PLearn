#ifndef ShellProgressBar_H
#define ShellProgressBar_H
#include <iostream.h>
#include <string>
#include "Popen.h"
#include <time.h>
#include "stringutils.h" 

namespace PLearn <%
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

%> // end of namespace PLearn


#endif // ShellProgressBar_H
