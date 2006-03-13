#include "DERIVEDCLASS.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DERIVEDCLASS,
    "ONE LINE DESCRIPTION",
    "MULTI LINE\nHELP");

DERIVEDCLASS::DERIVEDCLASS()
{}

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

void DERIVEDCLASS::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    //PP<Trader> trader;
    deepCopyField(fprop_Rt, copies);
    deepCopyField(fprop_weights, copies);
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
