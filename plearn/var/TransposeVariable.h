
#ifndef TransposeVariable_INC
#define TransposeVariable_INC

#include <plearn/var/UnaryVariable.h>

namespace PLearn {
using namespace std;


class TransposeVariable: public UnaryVariable
{
private:
    typedef UnaryVariable inherited;

public:

    //!  Default constructor for persistence
    TransposeVariable() : startk(0) {}

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

DECLARE_OBJECT_PTR(TransposeVariable);

inline Var transpose(Var v)
{ return new TransposeVariable(v); }

} // end of namespace PLearn

#endif 

/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
