#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include <plearn_learners/distributions/PDistribution.h>

namespace PLearn {
using namespace std;

class DERIVEDCLASS: public PDistribution
{

private:

  typedef PDistribution inherited;  

protected:

  // *********************
  // * protected options *
  // *********************

  // ### Declare protected option fields (such as learnt parameters) here.
  // ...
    
public:

  // ************************
  // * public build options *
  // ************************

  // ### Declare public option fields (such as build options) here.
  // ...

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  // ### Make sure the implementation in the .cc
  // ### initializes all fields to reasonable default values.
  DERIVEDCLASS();

  // *************************
  // * PDistribution methods *
  // *************************

private: 

  //! This does the actual building. 
  // ### Please implement in .cc.
  void build_();

protected: 

  //! Declare this class' options.
  // ### Please implement in .cc.
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! Simply call inherited::build() then build_().
  virtual void build();

  //! Transform a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  // Declare other standard object methods.
  // ### If your class is not instantiatable (it has pure virtual methods)
  // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS.
  PLEARN_DECLARE_OBJECT(DERIVEDCLASS);

  // *******************************
  // **** PDistribution methods ****
  // *******************************

  //! Return log of probability density log(p(y | x)).
  virtual real log_density(const Vec& x) const;

  //! Return survival function: P(Y>y | x).
  virtual real survival_fn(const Vec& y) const;

  //! Return cdf: P(Y<y | x).
  virtual real cdf(const Vec& y) const;

  //! Return E[Y | x].
  virtual void expectation(Vec& mu) const;

  //! Return Var[Y | x].
  virtual void variance(Mat& cov) const;

  //! Return a pseudo-random sample generated from the distribution.
  virtual void generate(Vec& y) const;

  //! Reset the random number generator used by generate() using the given seed.
  virtual void resetGenerator(long g_seed) const;

  //! Set the value for the input part of a conditional probability.
  virtual void setInput(const Vec& input) const;

  //! This method updates the internal data given a new sorting of the variables
  //! defined by the conditional flags.
  virtual void updateFromConditionalSorting();

  // ### These methods may be overridden for efficiency purpose:
  /*
  //! Return probability density p(y | x)
  virtual real density(const Vec& y) const;
  */

  // **************************
  // **** PLearner methods ****
  // **************************

  // ### Default version of inputsize returns learner->inputsize()
  // ### If this is not appropriate, you should uncomment this and define
  // ### it properly in the .cc
  // virtual int inputsize() const;

  //! (Re-)initializes the PDistribution in its fresh state (that state may depend on the 'seed' option).
  //! And sets 'stage' back to 0 (this is the stage of a fresh learner!).
  // ### You may remove this method if your distribution does not implement it.
  virtual void forget();
    
  //! The role of the train method is to bring the learner up to stage == nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  // ### You may remove this method if your distribution does not implement it.
  virtual void train();

};

// Declare a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(DERIVEDCLASS);
  
} // end of namespace PLearn
