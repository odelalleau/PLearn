#include "VMat.h"

#ifndef VVector_INC
#define VVector_INC

namespace PLearn <%

/*! ** VVector ** */
//! A VVector represents an abstract notion of "sample" or "example"
//! which will allow us to generalize VMatrices to handle objects
//! that are not conveniently representable with ordinary vectors.
class VVector : public Object
{
    // We leave the actual representation choice to some
    // underlying virtual matrix:
  public:
    VMat mat;
    int row_index;

    // to keep compatibility with most current code,
    // VVec's can be converted to Vec's
    virtual Vec const toVec();
    virtual void toVec(Vec row_vec);
    int length() const;
    DECLARE_NAME_AND_DEEPCOPY(VVector);
};

// CA CAUSE UN PROBLEME DE COMPILATION: POURQUOI?
//DECLARE_OBJECT_PTR(VVector);
%> // end of namespace PLearn
#endif
