#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include "RowBufferedVMatrix.h"

namespace PLearn <%
using namespace std;

class DERIVEDCLASS: public RowBufferedVMatrix
{
protected:
  // *********************
  // * protected options *
  // *********************

public:

  typedef RowBufferedVMatrix inherited;

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

public:
  //!  This is the only method requiring implementation
  virtual void getRow(int i, Vec v) const;

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Provides a help message describing this class
  virtual string help() const;

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  DECLARE_NAME_AND_DEEPCOPY(DERIVEDCLASS);

};

%> // end of namespace PLearn
#endif
