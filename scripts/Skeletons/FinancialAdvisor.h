#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include "FinancialAdvisor.h"

namespace PLearn <%
using namespace std;

class DERIVEDCLASS: public FinancialAdvisor
{
public:
  typedef FinancialAdvisor inherited;

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

  // *****************************
  // * public method to overload *
  // *****************************
  
  // This function must be overloaded!
  virtual void train();

  // This function must be overloaded!  
  virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
                    VMat testoutputs=0, VMat testcosts=0) const;

  // This function must be overloaded!  
  virtual TVec<string> getTrainCostNames() const;

  // This function must be overloaded!
  virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                     Vec& output, Vec& costs) const;
  
private:
  //! This does the actual building
  void build_();
  
protected:
  //! Declare this class' options
  static void declareOptions(OptionList& ol);

public:  
  //! Constructor
  DERIVEDCLASS();
    
  virtual void computeCostsOnly(const Vec& input, const Vec& target,
                                Vec& costs) const;
  
  virtual void computeOutput(const Vec& input, Vec& output) const;
  
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                       const Vec& target, Vec& costs) const;
  
  //! simply calls inherited::build() then build_()
  virtual void build();

  virtual void forget();
    
  //!  Does the necessary operations to transform a shallow copy (this)
  //!  into a deep copy by deep-copying all the members that need to be.
  PLEARN_DECLARE_OBJECT(DERIVEDCLASS);
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(DERIVEDCLASS);

%> // end of namespace PLearn

#endif
