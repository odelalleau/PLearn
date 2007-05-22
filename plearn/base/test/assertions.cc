#include <iostream>
#include <assert.h>
#include <plearn/base/plerror.h>
#include <assert.h> // NB: is there a reason to include assert.h at all?
#include <string>

// PLASSERT uses __FILE__ variable in its message, but we want the test
// to have the same output, regardless of the absolute path of this file
#undef __FILE__
#define __FILE__ "assertions.cc"

using namespace std;

int main()
{
  try {
    PLASSERT( 1 == 1 );
    PLASSERT( 3+8 == 123+46 );
  }
  catch (const PLearn::PLearnError& e)
  {
      string msg = e.message();
#ifdef WIN32
      // This is a hack so that the test passes under Windows: the assert
      // code unfortunatley does not have access to the function name, and thus
      // displays '(null)' instead of the correct name.
      size_t pos = msg.find("(null)");
      if (pos != string::npos)
          msg = msg.replace(pos, 6, "int main()");

#endif
    std::cerr << "FATAL ERROR: " << msg << std::endl;
  }
  return 0;
}

