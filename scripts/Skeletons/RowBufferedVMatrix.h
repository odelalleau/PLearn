#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include "RowBufferedVMatrix.h"

namespace PLearn {
using namespace std;

class DERIVEDCLASS: public RowBufferedVMatrix
{

private:

  typedef RowBufferedVMatrix inherited;

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

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
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

  //! Fill the vector 'v' with the content of the i-th row.
  //! v is assumed to be the right size.
  virtual void getNewRow(int i, const Vec& v) const;

public:

  // Simply call inherited::build() then build_().
  virtual void build();

  //! Transform a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declare name and deepCopy methods.
  PLEARN_DECLARE_OBJECT(DERIVEDCLASS);

};

DECLARE_OBJECT_PTR(DERIVEDCLASS);

} // end of namespace PLearn
#endif
