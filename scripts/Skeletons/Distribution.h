#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include "Distribution.h"

namespace PLearn <%
using namespace std;

class DERIVEDCLASS: public Distribution
{
protected:
  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...
    
public:

  typedef Distribution inherited;

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

  //! Provides a help message describing this class
  static string help();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  DECLARE_NAME_AND_DEEPCOPY(DERIVEDCLASS);

  // *******************
  // * Learner methods *
  // *******************

  //! trains the model
  virtual void train(VMat training_set); 

  //! computes the ouptu of a trained model
  virtual void use(const Vec& input, Vec& output);

  // ************************
  // * Distribution methods *
  // ************************

  //! return log of probability density log(p(x))
  virtual double log_density(const Vec& x) const;

  //! return probability density p(x)
  //! [ default version returns exp(log_density(x)) ]
  //virtual double density(const Vec& x) const;
  
  //! return survival fn = P(X>x)
  virtual double survival_fn(const Vec& x) const;

  //! return survival fn = P(X<x)
  virtual double cdf(const Vec& x) const;

  //! return E[X] 
  virtual double expectation() const;

  //! return Var[X]
  virtual double variance() const;
   
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(DERIVEDCLASS);
  
%> // end of namespace PLearn

#endif
