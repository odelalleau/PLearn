#include "DERIVEDCLASS.h"

namespace PLearn <%
using namespace std;

DERIVEDCLASS::DERIVEDCLASS() 
{}

PLEARN_IMPLEMENT_OBJECT(DERIVEDCLASS, "ONE LINE DESCRIPTION", "MULTI LINE\nHELP");

Vec DERIVEDCLASS::scale(const Vec& portfolio)
{
#error Must be overloaded
}

void DERIVEDCLASS::fprop()
{
#error Must be overloaded
}

void DERIVEDCLASS::bprop()
{
#error Must be overloaded
}

void DERIVEDCLASS::declareOptions(OptionList& ol)
{
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void DERIVEDCLASS::build_()
{
}

// ### Nothing to add here, simply calls build_
void DERIVEDCLASS::build()
{
  inherited::build();
  build_();
}

void DERIVEDCLASS::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  //PP<Trader> trader;
  deepCopyField(fprop_Rt, copies);
  deepCopyField(fprop_weights, copies);
}

%> // end of namespace PLearn
