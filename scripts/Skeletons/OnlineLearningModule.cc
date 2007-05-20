
#include "DERIVEDCLASS.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DERIVEDCLASS,
    "ONE LINE DESCRIPTION",
    "MULTI-LINE \nHELP");

DERIVEDCLASS::DERIVEDCLASS()
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
void DERIVEDCLASS::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( ports_value.length() == nPorts() );
    // check which ports are input (ports_value[i] && !ports_value[i]->isEmpty())
    // which ports are output (ports_value[i] && ports_value[i]->isEmpty())
    // and which ports are ignored (!ports_value[i]).
    // If that combination of (input,output,ignored) is feasible by this class
    // then perform the corresponding computation. Otherwise launch the error below.
    // See the comment in the header file for more information.
    PLERROR("In DERIVEDCLASS::fprop - Not implemented for class "
            "'%s'", classname().c_str());
}

/////////////////
// bpropUpdate //
/////////////////


void DERIVEDCLASS::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                          const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_value.length() == nPorts() && ports_gradient.length() == nPorts());
    // check which ports are input (ports_value[i] && !ports_value[i]->isEmpty())
    // which ports are output (ports_value[i] && ports_value[i]->isEmpty())
    // and which ports are ignored (!ports_value[i]).
    // A similar logic applies to ports_gradients (to know whether gradient
    // is coming into the module of coming from the module through a given ports_gradient[i]).
    // An input port_value should correspond to an outgoing port_gradient,
    // an output port_value could either correspond to an incoming port_gradient
    // (when that gradient is to be propagated inside and to the input ports)
    // or it should be null (no gradient is propagated from that output port).
    // If that combination of (input,output,ignored) is feasible by this class
    // then perform the corresponding computation. Otherwise launch the error below.
    // See the comment in the header file for more information.
    PLERROR("In DERIVEDCLASS::bpropAccUpdate - this configuration of ports not implemented for class "
            "'%s'", classname().c_str());
}

/* THIS METHOD IS OPTIONAL
// This version is similar to bpropAccUpdate but it does not accumulate
// in the input ports gradient 
void DERIVEDCLASS::bpropUpdate(const TVec<Mat*>& ports_value,
                                       const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_value.length() == nPorts() && ports_gradient.length() == nPorts());
    // check which ports are input (ports_value[i] && !ports_value[i]->isEmpty())
    // which ports are output (ports_value[i] && ports_value[i]->isEmpty())
    // and which ports are ignored (!ports_value[i]).
    // A similar logic applies to ports_gradients (to know whether gradient
    // is coming into the module of coming from the module through a given ports_gradient[i]).
    // An input port_value should correspond to an outgoing port_gradient,
    // an output port_value could either correspond to an incoming port_gradient
    // (when that gradient is to be propagated inside and to the input ports)
    // If that combination of (input,output,ignored) is feasible by this class
    // then perform the corresponding computation. Otherwise launch the error below.
    PLERROR("In DERIVEDCLASS::bpropUpdate - this configuration of ports not implemented for class "
            "'%s'", classname().c_str());
}
*/

////////////
// forget //
////////////
void DERIVEDCLASS::forget()
{
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
// the default implementation returns false
bool DERIVEDCLASS::bpropDoesNothing()
{
}
*/

/////////////////////
// setLearningRate //
/////////////////////
/* OPTIONAL
// The default implementation raises a warning and does not do anything.
void DERIVEDCLASS::setLearningRate(real dynamic_learning_rate)
{
}
*/

////////////////////////
// getPortDescription //
////////////////////////
/* OPTIONAL
// The default implementation is probably appropriate
TVec<string> DERIVEDCLASS::getPortDescription(const string& port)
{
} 
*/

//////////////////
// getPortIndex //
//////////////////
/* OPTIONAL
// The default implementation is probably appropriate
int DERIVEDCLASS::getPortIndex(const string& port)
{
}
*/

/////////////////
// getPortName //
/////////////////
/* OPTIONAL
// The default implementation is probably appropriate
string DERIVEDCLASS::getPortName(int i)
{
}
*/

//////////////
// getPorts //
//////////////
const TVec<string>& DERIVEDCLASS::getPorts() {
}

//////////////////
// getPortSizes //
//////////////////
const TMat<int>& DERIVEDCLASS::getPortSizes() {
}

///////////////////
// getPortLength //
///////////////////
/* OPTIONAL
// The default implementation is probably appropriate
int DERIVEDCLASS::getPortLength(const string& port)
{
    PLASSERT( getPortIndex(port) >= 0 );
    return getPortSizes()(getPortIndex(port), 0);
}
*/

//////////////////
// getPortWidth //
//////////////////
/* OPTIONAL
// The default implementation is probably appropriate
int DERIVEDCLASS::getPortWidth(const string& port)
{
}
*/

////////////
// nPorts //
////////////
/* OPTIONAL
// The default implementation is probably appropriate
int DERIVEDCLASS::nPorts()
{
}
*/

}
// end of namespace PLearn


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
