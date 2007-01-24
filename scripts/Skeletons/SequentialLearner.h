#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include <plearn_learners/sequential/SequentialLearner.h>

namespace PLearn {

class DERIVEDCLASS: public SequentialLearner
{

private:

    typedef SequentialLearner inherited;

private:
    //! This does the actual building
    void build_();

protected:
    //! Declare this class' options
    static void declareOptions(OptionList& ol);

public:

    //! Constructor
    DERIVEDCLASS();

    //! simply calls inherited::build() then build_()
    virtual void build();

    //! *** SUBCLASS WRITING: ***
    virtual void train();

    //! *** SUBCLASS WRITING: ***
    virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
                      VMat testoutputs=0, VMat testcosts=0) const;

    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                       Vec& output, Vec& costs) const;

    virtual void computeCostsOnly(const Vec& input, const Vec& target,
                                  Vec& costs) const;

    virtual void computeOutput(const Vec& input, Vec& output) const;

    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    virtual void forget();

    virtual TVec<std::string> getTrainCostNames() const;
    virtual TVec<std::string> getTestCostNames() const;

    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(DERIVEDCLASS);

    //!  Does the necessary operations to transform a shallow copy (this)
    //!  into a deep copy by deep-copying all the members that need to be.
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
