#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include "DERIVEDCLASS.h"

namespace PLearn <%
using namespace std;

class DERIVEDCLASS: public PLearnCommand
{
public:
  DERIVEDCLASS():
    PLearnCommand(">>>> INSERT COMMAND NAME HERE",

                  ">>>> INSERT A SHORT ONE LINE DESCRIPTION HERE",

                  ">>>> INSERT SYNTAX AND \n"
                  "FULL DETAILED HELP HERE \n"
                  ) 
  {}
                    
  virtual void run(const vector<string>& args);

protected:
  static PLearnCommandRegistry reg_;
};

  
%> // end of namespace PLearn

#endif
