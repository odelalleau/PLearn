#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include <apstatlib/finance/timeseries/SharedCalendarTimeSeries.h>

namespace PLearn {
using namespace std;

class DERIVEDCLASS: public SharedCalendarTimeSeries
{
private:
  typedef SharedCalendarTimeSeries inherited;

  //! This does the actual building.. 
  void build_();

protected:
  // SUBCLASS WRITING: THIS IS TYPICAL WHENEVER THERE IS ONLY ONE
  // INPUT TO THE TIMESERIES. PLEASE CHANGE IF IT IS NOT YOUR CASE.
  //! Internal use. Pointer to the series this series lags.
  PTimeSeries series_;

  //! Declares this class' options.
  // ### (PLEASE IMPLEMENT IN .cc)
  static void declareOptions(OptionList& ol);

  /*!
    This method is responsible for the update of any internal data and
    for calling setNow(now) on the children of the TimeSeries, if any.
    
    The returned range corresponds to the temporal range that was changed
    (if anything was changed) and may be used by the parent to intelligently
    update its own values.
  */
  virtual CTimeRange update(JTime now) const;

  /*!
    Puts in "result" the content of the value at time "time".
    Method setNow(JTime) should be called before calling this
    function.
  */
  virtual void getValue(CTime time, Vec& result) const;


public:

  //! Default constructor.
  DERIVEDCLASS(const PTimeSeries series = 0);

  //! Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(DERIVEDCLASS);

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
};

//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(DERIVEDCLASS);
  
} // end of namespace PLearn

#endif
