#include <plearn/io/pl_log.h>
using namespace PLearn;

int main()
{
  PL_Log::instance().verbosity(5);
  PL_LOG(1) << plsep << "Ceci est une chaine idiote au niveau 1" << endl;
  PL_LOG(10) << plsep << "Ceci est une chaine idiote au niveau 10" << endl;
  PL_LOG(5) << plsep << "Ceci est une chaine idiote au niveau 5" << endl;
  PL_LOG(5) << plhead("Titre cool") << endl;
  return 0;
}
