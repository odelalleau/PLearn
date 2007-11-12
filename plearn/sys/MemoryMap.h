
/*! \file MemoryMap.h */

#ifndef MODULE_MEMORY_MAP
#define MODULE_MEMORY_MAP

#include <cstdlib>
#include <sys/stat.h>

namespace PLearn {
using namespace std;

#if !(!defined(_MSC_VER) && !defined(_MINGW_))

//////////version windows NT //////////////////!<  
typedef void * tFileHandle;

const int STORAGE_UNUSED_HANDLE = 0;

//////////////////////////////////////////////////////!<  
/*!   
  maps a file into virtual memory
  
  bool readonly: If true, the virtual memory is created in read-only mode
  offset_: offset_from the start of the file
  length: length of window to map starting at offset_ (a length of 0 implies to the end of file)
  
  returns 0 if fails, and you have to check GetLastError() to see what happened.
  
*/
void * memoryMap(const char *filename, tFileHandle & file_handle, bool readonly=true, int offset_=0, int length=0);

//////////////////////////////////////////////////////!<  
/*!   
  unmaps a file from virtual memory.
  
  the parameter: the pointer returned by a call to memoryMap(...)
  
  return true if succeeds, false if it does not. You will have to 
  check the value of GetLastError() to see what went wrong.
*/
bool memoryUnmap(void * pointer, tFileHandle file_handle);

#else 

/////////////////version linux///////////////////////!<  
typedef int tFileHandle;
const int STORAGE_UNUSED_HANDLE = -1;


//!  returns a pointer to the memory-mapped file
//!  or 0 if it fails for some reason.
void * MemoryMap(const char *filename,tFileHandle & handle, bool read_only, off_t & filesize);

void memoryUnmap(void * mapped_pointer, tFileHandle handle, int length);

#endif

} // end of namespace PLearn

#endif //!<  MODULE_MEMORY_MAP


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
