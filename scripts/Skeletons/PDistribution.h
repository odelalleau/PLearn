#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include "PDistribution.h"

namespace PLearn <%
using namespace std;

class DERIVEDCLASS: public PDistribution
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
  // * PDistribution methods *
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

  // ************************
  // **** Object methods ****
  // ************************

  //! simply calls parentclass::build() then build_() 
  virtual void build();

  //! Provides a help message describing this class
  static string help();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  // Declares other standard object methods
  //  If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
  PLEARN_DECLARE_OBJECT_METHODS(DERIVEDCLASS, "DERIVEDCLASS", PDistribution);


  // **************************
  // **** PDistribution methods ****
  // **************************

  //! return log of probability density log(p(x))
  virtual double log_density(const Vec& x) const;

  //! return survival fn = P(X>x)
  virtual double survival_fn(const Vec& x) const;

  //! return survival fn = P(X<x)
  virtual double cdf(const Vec& x) const;

  //! return E[X] 
  virtual void expectation(Vec& mu) const;

  //! return Var[X]
  virtual void variance(Mat& cov) const;

  //! Resets the random number generator used by generate using the given seed
  virtual void resetGenerator(long g_seed) const;

  //! return a pseudo-random sample generated from the distribution.
  virtual void generate(Vec& x) const;

  
  // **************************
  // **** Learner methods ****
  // **************************

  // Default version of inputsize returns learner->inputsize()
  // If this is not appropriate, you should uncomment this and define
  // it properly in the .cc
  // virtual int inputsize() const;

  //! (Re-)initializes the PDistribution in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
  //! You may remove this method if your distribution does not implement it
  virtual void forget();

    
  //! The role of the train method is to bring the learner up to stage==nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  //! You may remove this method if your distribution does not implement it
  virtual void train();

};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(DERIVEDCLASS);
  
%> // end of namespace PLearn

#endif
