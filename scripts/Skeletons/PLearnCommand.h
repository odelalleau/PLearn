#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include <commands/PLearnCommands/PLearnCommand.h>
#include <commands/PLearnCommands/PLearnCommandRegistry.h>

namespace PLearn {

class DERIVEDCLASS: public PLearnCommand
{
public:
  DERIVEDCLASS();                    
  virtual void run(const std::vector<std::string>& args);

protected:
  static PLearnCommandRegistry reg_;
};

  
} // end of namespace PLearn

#endif
