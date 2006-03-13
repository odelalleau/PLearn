#include "DERIVEDCLASS.h"

namespace PLearn {
using namespace std;

//! This allows to register the 'DERIVEDCLASS' command in the command registry
PLearnCommandRegistry DERIVEDCLASS::reg_(new DERIVEDCLASS);

DERIVEDCLASS::DERIVEDCLASS()
    : PLearnCommand(
        ">>>> INSERT COMMAND NAME HERE",
        ">>>> INSERT A SHORT ONE LINE DESCRIPTION HERE",
        ">>>> INSERT SYNTAX AND \n"
        "FULL DETAILED HELP HERE \n"
        )
{}

//! The actual implementation of the 'DERIVEDCLASS' command
void DERIVEDCLASS::run(const vector<string>& args)
{
    // *** PLEASE COMPLETE HERE ****
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
