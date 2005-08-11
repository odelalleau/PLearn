#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include "FinancialAdvisor.h"

namespace PLearn {

class DERIVEDCLASS: public FinancialAdvisor
{

private:
  
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
    virtual TVec<std::string> getTrainCostNames() const;

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

} // end of namespace PLearn

#endif


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
