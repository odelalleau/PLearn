/* PLearnLibrary/TestSuite/test_Storage.cc */


#include "test_Storage.h"
#include <iostream.h>

using namespace PLearn;

int main(int argc, char* argv)
{
  cout << "Creating TestStorage..." << endl;
  TestStorage<Integer> ts( /*currentTime()*/100 );
  
  cout << "Lunching TestStorage..." << endl;
  
  DO_TEST("Ctors", ts.testCtor());
  DO_TEST("Memory management", ts.testManagement());
  //  DO_TEST("Serialization", ts.testSerialization());
  
  cout << "Quitting TestStorage..." << endl;
  
  return 0;
}
