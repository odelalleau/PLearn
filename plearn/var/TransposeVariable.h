
#ifndef TransposeVariable_INC
#define TransposeVariable_INC

#include <plearn/var/UnaryVariable.h>

namespace PLearn <%
using namespace std;


class TransposeVariable: public UnaryVariable
{
protected:
  typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  TransposeVariable() : startk() {}

protected:
  int startk;
public:
  TransposeVariable(Variable* v);
  PLEARN_DECLARE_OBJECT(TransposeVariable);
  virtual void recomputeSize(int& l, int& w) const;
  
  
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};


%> // end of namespace PLearn

#endif 
