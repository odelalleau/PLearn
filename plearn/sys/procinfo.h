
#ifndef MODULE_PROCINFO
#define MODULE_PROCINFO

namespace PLearn {

//! Return the total memory installed in the system in bytes
int getSystemTotalMemory();

//! Return the total data memory used by the current process in bytes
int getProcessDataMemory();

} // end of namespace PLearn

#endif


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
