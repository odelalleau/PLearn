#include "DERIVEDCLASS.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DERIVEDCLASS,
    "ONE LINE DESCRIPTION",
    "MULTI-LINE \nHELP"
    );

//////////////////
// DERIVEDCLASS //
//////////////////
DERIVEDCLASS::DERIVEDCLASS()
/* ### Initialize all fields to their default value here */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

////////////////////
// declareOptions //
////////////////////
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

///////////
// build //
///////////
void DERIVEDCLASS::build()
{
    // ### Nothing to add here, simply calls build_
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
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.

    // Since a SourceKernel takes its 'is_symmetric' option from the underlying
    // kernel, you should modify it here if you want it to be different.
}

///////////////////////
// computeGramMatrix //
///////////////////////
void DERIVEDCLASS::computeGramMatrix(Mat K) const {
    // Default = uses the Kernel implementation.
    Kernel::computeGramMatrix(K);
}

//////////////
// evaluate //
//////////////
real DERIVEDCLASS::evaluate(const Vec& x1, const Vec& x2) const {
    // ### Evaluate the kernel on a pair of points.
    // TODO: Implement.
    // Ex: return source_kernel->evaluate(x1,x2);
}

//////////////////
// evaluate_i_j //
//////////////////
real DERIVEDCLASS::evaluate_i_j(int i, int j) const {
    // Default = uses the Kernel implementation.
    // Alternative = return source_kernel->evaluate_i_j(i,j);
    return Kernel::evaluate_i_j(i,j);
}

//////////////////
// evaluate_i_x //
//////////////////
real DERIVEDCLASS::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const {
    // Default = uses the Kernel implementation.
    // Alternative = return source_kernel->evaluate_i_x(i,x,squared_norm_of_x);
    return Kernel::evaluate_i_x(i, x, squared_norm_of_x);
}

////////////////////////
// evaluate_i_x_again //
////////////////////////
real DERIVEDCLASS::evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x, bool first_time) const {
    // Default = uses the Kernel implementation.
    // Alternative = return source_kernel->evaluate_i_x_again(i,x,squared_norm_of_x,first_time);
    return Kernel::evaluate_i_x_again(i, x, squared_norm_of_x, first_time);
}

//////////////////
// evaluate_x_i //
//////////////////
real DERIVEDCLASS::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const {
    // Default = uses the Kernel implementation.
    // Alternative = return source_kernel->evaluate_x_i(x,i,squared_norm_of_x);
    return Kernel::evaluate_x_i(x, i, squared_norm_of_x);
}

////////////////////////
// evaluate_x_i_again //
////////////////////////
real DERIVEDCLASS::evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x, bool first_time) const {
    // Default = uses the Kernel implementation.
    // Alternative = return source_kernel->evaluate_x_i_again(x,i,squared_norm_of_x,first_time);
    return Kernel::evaluate_x_i_again(x, i, squared_norm_of_x, first_time);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
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

/* ### This method may often need to be overridden in subclasses.
   ### If it is not, then the call will be forwarded to 'source_kernel'.
   ### If you override it, be careful that it may be called BEFORE the build_()
   ### method has been called, if the 'specify_dataset' option is used.
////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void DERIVEDCLASS::setDataForKernelMatrix(VMat the_data) {
// Do some preprocessing here, for instance.
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
