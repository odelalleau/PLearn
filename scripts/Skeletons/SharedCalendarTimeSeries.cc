#include "DERIVEDCLASS.h"

namespace PLearn {
using namespace std;


// SUBCLASS WRITING: THIS IS TYPICAL WHENEVER THERE IS ONLY ONE
// INPUT TO THE TIMESERIES. PLEASE CHANGE IF IT IS NOT YOUR CASE.
DERIVEDCLASS::DERIVEDCLASS(const PTimeSeries series) 
  : inherited(), series_(series)
{
  inputs_.resize(1);
  inputs_[0] = series;
}

PLEARN_IMPLEMENT_OBJECT(DERIVEDCLASS,
    "ONE LINE DESCRIPTION",
    "NO HELP"
);

void DERIVEDCLASS::declareOptions(OptionList& ol)
{
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

CTimeRange DERIVEDCLASS::update(JTime now) const
{
  series_->setNow(now);
  return inherited::update(now); // default
}

void DERIVEDCLASS::getValue(CTime time, Vec& result) const
{
  // SUBCLASS WRITING: THIS IS TYPICAL WHENEVER THERE IS ONLY ONE
  // INPUT TO THE TIMESERIES. PLEASE CHANGE IF IT IS NOT YOUR CASE.
  series_->getValue(time, result);
  PLERROR("Please implement!");
}

void DERIVEDCLASS::build_()
{
  // SUBCLASS WRITING: THIS IS TYPICAL WHENEVER THERE IS ONLY ONE
  // INPUT TO THE TIMESERIES. PLEASE CHANGE IF IT IS NOT YOUR CASE.
  // set inputs and outputs
  inputs_.resize(1);
  series_ = inputs_[0];

  if (series_)
  {
    // ...
    inherited::build(); // call parent's build() with new dimensions
  }
}

void DERIVEDCLASS::build()
{
  inherited::build();
  build_();
}

void DERIVEDCLASS::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  PLERROR("Please implement.");
  inherited::makeDeepCopyFromShallowCopy(copies);
}


} // end of namespace PLearn
