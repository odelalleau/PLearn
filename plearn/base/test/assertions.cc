#include <iostream>
#include <assert.h>
#include <plearn/base/plerror.h>

#undef __FILE__
#define __FILE__ "assertions.cc"

int main()
{
  try {
    assert( 1 == 1 );
    assert( 3+8 == 123+46 );
  }
  catch (const PLearn::PLearnError& e)
  {
    std::cerr << "FATAL ERROR: " << e.message() << std::endl;
  }
  return 0;
}

