#include "DERIVEDCLASS.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_ABSTRACT_OBJECT(DERIVEDCLASS, "ONE LINE DESCR", "NO HELP");

DERIVEDCLASS::DERIVEDCLASS():
{}

void DERIVEDCLASS::build()
{
  inherited::build();
  build_();
}

void DERIVEDCLASS::build_()
{      
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
#error This function must be overloaded!  
}

void DERIVEDCLASS::test(VMat testset, PP<VecStatsCollector> test_stats,
                        VMat testoutputs=0, VMat testcosts=0) const
{
#error This function must be overloaded!  
}
 
TVec<string> DERIVEDCLASS::getTrainCostNames() const
{
#error This function must be overloaded!  
}

void DERIVEDCLASS::computeOutputAndCosts(const Vec& input,
                                         const Vec& target, Vec& output, Vec& costs) const
{ 
#error This function must be overloaded!  
}

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

%> // end of namespace PLearn

