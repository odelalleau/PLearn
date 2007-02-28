
#include "DERIVEDCLASS.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DERIVEDCLASS,
    "ONE LINE DESCRIPTION",
    "MULTI-LINE \nHELP");

DERIVEDCLASS::DERIVEDCLASS() :
/* ### Initialize all fields to their default value here */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

void DERIVEDCLASS::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &DERIVEDCLASS::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DERIVEDCLASS::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.
}

// ### Nothing to add here, simply calls build_
void DERIVEDCLASS::build()
{
    inherited::build();
    build_();
}


void DERIVEDCLASS::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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

///////////
// fprop //
///////////
void DERIVEDCLASS::fprop(const Vec& input, const Vec& target, Vec& cost) const
{
}

/* OPTIONAL
void DERIVEDCLASS::fprop(const Vec& input, const Vec& target, real& cost) const
{
}
*/

/////////////////
// bpropUpdate //
/////////////////
void DERIVEDCLASS::bpropUpdate(const Vec& input, const Vec& target, real cost,
                               Vec& input_gradient, bool accumulate)
{
}

/* THIS METHOD IS OPTIONAL
void DERIVEDCLASS::bpropUpdate(const Vec& input, const Vec& target, real cost)
{
}
*/

//////////////////
// bbpropUpdate //
//////////////////
/* THIS METHOD IS OPTIONAL
void DERIVEDCLASS::bbpropUpdate(const Vec& input, const Vec& target, real cost,
                                Vec& input_gradient, Vec& input_diag_hessian,
                                bool accumulate)
{
}
*/

/* THIS METHOD IS OPTIONAL
void DERIVEDCLASS::bbpropUpdate(const Vec& input, const Vec& target, real cost)
{
}
*/

////////////
// forget //
////////////
void DERIVEDCLASS::forget()
{
}

//////////
// name //
//////////
TVec<string> DERIVEDCLASS::name()
{
    // ### Usually, the name of the class without the trailing "CostModule"
    return TVec<string>(1, "DERIVEDCLASS");
}

//////////////
// finalize //
//////////////
/* THIS METHOD IS OPTIONAL
void DERIVEDCLASS::finalize()
{
}
*/

//////////////////////
// bpropDoesNothing //
//////////////////////
/* THIS METHOD IS OPTIONAL
bool DERIVEDCLASS::bpropDoesNothing()
{
}
*/

/////////////////////
// setLearningRate //
/////////////////////
/* OPTIONAL
void DERIVEDCLASS::setLearningRate(real dynamic_learning_rate)
{
}
*/


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
