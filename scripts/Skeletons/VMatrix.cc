#include "DERIVEDCLASS.h"

namespace PLearn {
using namespace std;

//////////////////
// DERIVEDCLASS //
//////////////////
DERIVEDCLASS::DERIVEDCLASS() 
/* ### Initialize all fields to their default value here */
  {
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
  }

PLEARN_IMPLEMENT_OBJECT(DERIVEDCLASS,
    "ONE LINE DESCR",
    "NO HELP"
);

////////////////////
// declareOptions //
////////////////////
void DERIVEDCLASS::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // ### ex:
  // declareOption(ol, "myoption", &DERIVEDCLASS::myoption, OptionBase::buildoption,
  //               "Help text describing this option");
  // ...

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void DERIVEDCLASS::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void DERIVEDCLASS::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
}

/////////
// get //
/////////
real DERIVEDCLASS::get(int i, int j) const
{
  // get element at i-th row, j-th column
  // ...
}

///////////////
// getSubRow //
///////////////
void DERIVEDCLASS::getSubRow(int i, int j, Vec v) const
{
  // get part or all of the i-th, starting at the j-th column,
  // with v.length() elements; these elements are put in v.
  // ...
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void DERIVEDCLASS::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("DERIVEDCLASS::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn
