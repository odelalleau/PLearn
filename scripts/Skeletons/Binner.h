#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include <plearn/math/Binner.h>

namespace PLearn {

class DERIVEDCLASS: public Binner
{

private:

  typedef Binner inherited;  

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
  // (Please implement in .cc)
  void build_();

protected: 
  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:
  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(DERIVEDCLASS);

  // ******************
  // * Binner methods *
  // ******************

public:
  //! Returns a binning for a single column vmatrix v 
  virtual RealMapping getBinning(VMat v) const;

};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(DERIVEDCLASS);
  
} // end of namespace PLearn

#endif
