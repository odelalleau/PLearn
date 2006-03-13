#include "DERIVEDCLASS.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    DERIVEDCLASS,
    "ONE LINE DESCR",
    "NO HELP");

DERIVEDCLASS::DERIVEDCLASS()
    : // ...
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

void DERIVEDCLASS::build()
{
    inherited::build();
    build_();
}

void DERIVEDCLASS::build_()
{
    forget();
}

void DERIVEDCLASS::forget()
{
    inherited::forget();
}

void DERIVEDCLASS::declareOptions(OptionList& ol)
{
    inherited::declareOptions(ol);
}

void DERIVEDCLASS::train()
{
}

void DERIVEDCLASS::test(VMat testset, PP<VecStatsCollector> test_stats,
                        VMat testoutputs=0, VMat testcosts=0) const
{
}

void DERIVEDCLASS::computeOutputAndCosts(const Vec& input,
                                         const Vec& target, Vec& output, Vec& costs) const
{ PLERROR("The method computeOutputAndCosts is not defined for this DERIVEDCLASS"); }

void DERIVEDCLASS::computeCostsOnly(const Vec& input, const Vec& target,
                                    Vec& costs) const
{ PLERROR("The method computeCostsOnly is not defined for this DERIVEDCLASS"); }

void DERIVEDCLASS::computeOutput(const Vec& input, Vec& output) const
{ PLERROR("The method computeOutput is not defined for this DERIVEDCLASS"); }

void DERIVEDCLASS::computeCostsFromOutputs(const Vec& input,
                                           const Vec& output, const Vec& target, Vec& costs) const
{ PLERROR("The method computeCostsFromOutputs is not defined for this DERIVEDCLASS"); }

void DERIVEDCLASS::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("DERIVEDCLASS::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

TVec<string> DERIVEDCLASS::getTrainCostNames() const
{
    return ...;
}

TVec<string> DERIVEDCLASS::getTestCostNames() const
{ return DERIVEDCLASS::getTrainCostNames(); }

} // end of namespace PLearn


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
