#include "DERIVEDCLASS.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_OBJECT_METHODS(DERIVEDCLASS, "DERIVEDCLASS", SequentialLearner);

DERIVEDCLASS::DERIVEDCLASS()
  : // ...
{
  // ...

  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

void DERIVEDCLASS::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  parentclass::makeDeepCopyFromShallowCopy(copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("DERIVEDCLASS::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
} 

void DERIVEDCLASS::build_()
{
}

void DERIVEDCLASS::build()
{
  parentclass::build();
  build_();
}

void DERIVEDCLASS::declareOptions(OptionList& ol)
{
  parentclass::declareOptions(ol);
}

void DERIVEDCLASS::forget()
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

%> // end of namespace PLearn

