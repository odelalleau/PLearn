
#include "test_TMat_maths.h"

using namespace PLearn;

int main(int argc, char* argv[])
{
  TestTMatMaths<double> TTMM(argv[1]);
  TTMM.runTest();
  TTMM.notYetImplementedTests();
}
