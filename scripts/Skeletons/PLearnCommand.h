#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include <commands/PLearnCommands/PLearnCommand.h>
#include <commands/PLearnCommands/PLearnCommandRegistry.h>

namespace PLearn {
using namespace std;

class DERIVEDCLASS: public PLearnCommand
{
public:
  DERIVEDCLASS();                    
  virtual void run(const vector<string>& args);

protected:
  static PLearnCommandRegistry reg_;
};

  
} // end of namespace PLearn

#endif
