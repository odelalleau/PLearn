#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include "Object.h"

namespace PLearn <%
using namespace std;

class DERIVEDCLASS: public Object
{
protected:
  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...
    
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
  // (PLEASE IMPLEMENT IN .cc)
  void build_();

protected: 
  //! Declares this class' options
  // (PLEASE IMPLEMENT IN .cc)
  static void declareOptions(OptionList& ol);

public:

  typedef Object inherited;
  // Declares other standard object methods
  //  If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
  PLEARN_DECLARE_OBJECT(DERIVEDCLASS);

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  // (PLEASE IMPLEMENT IN .cc)
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(DERIVEDCLASS);
  
%> // end of namespace PLearn

#endif
