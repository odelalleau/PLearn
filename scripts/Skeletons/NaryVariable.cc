#include "DERIVEDCLASS.h"

namespace PLearn {
using namespace std;

/** DERIVEDCLASS **/

PLEARN_IMPLEMENT_OBJECT(
    DERIVEDCLASS,
    "ONE LINE USER DESCRIPTION",
    "MULTI LINE\nHELP FOR USERS"
    );

DERIVEDCLASS::DERIVEDCLASS()
    /* ### Initialize all fields to their default value */
{
    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

// constructors from input variables.
// NaryVariable constructor (inherited) takes a VarArray as argument.
// You can either construct from a VarArray (if the number of parent Var is not
// fixed, for instance), or construct a VarArray from Var by operator &:
// input1 & input2 & input3. You can also do both, uncomment what you prefer.

// DERIVEDCLASS::DERIVEDCLASS(const VarArray& vararray)
// ### replace with actual parameters
//  : inherited(vararray, this_variable_length, this_variable_width),
//    parameter(default_value),
//    ...
// {
//     // ### You may (or not) want to call build_() to finish building the
//     // ### object
// }

// DERIVEDCLASS::DERIVEDCLASS(Var input1, Var input2,
//                            Var input3, ...)
// ### replace with actual parameters
//  : inherited(input1 & input2 & input3 & ...,
//              this_variable_length, this_variable_width),
//    parameter(default_value),
//    ...
// {
//     // ### You may (or not) want to call build_() to finish building the
//     // ### object
// }

// constructor from input variable and parameters
// DERIVEDCLASS::DERIVEDCLASS(const VarArray& vararray
//                            param_type the_parameter, ...)
// ### replace with actual parameters
//  : inherited(vararray, this_variable_length, this_variable_width),
//    parameter(the_parameter),
//    ...
// {
//     // ### You may (or not) want to call build_() to finish building the
//     // ### object
// }

// constructor from input variable and parameters
// DERIVEDCLASS::DERIVEDCLASS(Var input1, Var input2,
//                            Var input3, ...,
//                            param_type the_parameter, ...)
// ### replace with actual parameters
//  : inherited(input1 & input2 & input3 & ...,
//              this_variable_length, this_variable_width),
//    parameter(the_parameter),
//    ...
// {
//     // ### You may (or not) want to call build_() to finish building the
//     // ### object
// }

void DERIVEDCLASS::recomputeSizes(int& l, int& w) const
{
    // ### usual code to put here is:
    /*
        if (varray.size() > 0) {
            l = ... ; // the computed length of this Var
            w = ... ; // the computed width
        } else
            l = w = 0;
    */
}

// ### computes value from varray values
void DERIVEDCLASS::fprop()
{
    // ### remove this line when implemented
    PLERROR("In DERIVEDCLASS - fprop() must be implemented.");
}

// ### computes varray gradients from gradient
void DERIVEDCLASS::bprop()
{
    // ### remove this line when implemented
    PLERROR("In DERIVEDCLASS - bprop() must be implemented.");
}

// ### You can implement these methods:
// void DERIVEDCLASS::bbprop() {}
// void DERIVEDCLASS::symbolicBprop() {}
// void DERIVEDCLASS::rfprop() {}


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

    // ### If you want to deepCopy a Var field:
    // varDeepCopyField(somevariable, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("DERIVEDCLASS::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
