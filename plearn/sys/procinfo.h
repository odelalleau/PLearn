
#ifndef MODULE_PROCINFO
#define MODULE_PROCINFO

namespace PLearn {

//! Return the total memory installed in the system in bytes
int getSystemTotalMemory();

//! Return the total data memory used by the current process in bytes
int getProcessDataMemory();

} // end of namespace PLearn

#endif
