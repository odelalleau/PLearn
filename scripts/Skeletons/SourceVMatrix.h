#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;

class DERIVEDCLASS: public SourceVMatrix
{

private:

  typedef SourceVMatrix inherited;

protected:

  // *********************
  // * protected options *
  // *********************

public:

  // ************************
  // * public build options *
  // ************************

  // ### declare public option fields (such as build options) here
  // ...

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  // ### Make sure the implementation in the .cc
  // ### initializes all fields to reasonable default values.
  DERIVEDCLASS();

  // ******************
  // * Object methods *
  // ******************

private: 

  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 

  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:

  //! This is the only method requiring implementation.
  virtual void getRow(int i, Vec v) const;

  // Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(DERIVEDCLASS);

};

DECLARE_OBJECT_PTR(DERIVEDCLASS);

} // end of namespace PLearn

#endif
