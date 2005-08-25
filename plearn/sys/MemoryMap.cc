
#ifdef _MSC_VER
#include <io.h>
#else
//#include <sys/io.h>
#endif


#include "MemoryMap.h"
#include <plearn/base/general.h>
#include <fcntl.h>


#if !defined(_MSC_VER) && !defined(_MINGW_)

// NOT MICROSOFT: LINUX, SOLARIS, etc.
#include <sys/mman.h>

#else 

// OUINNEDOZE

//#define _X86_
#if !defined (_MINGW_) && !defined(WIN32)
#include <varargs.h>
#else
//#include <windef.h>
//#include <winbase.h>
#endif

#endif 
//

#include <plearn/base/general.h>

//#include <sys/stat.h>

namespace PLearn {
using namespace std;

#if !(!defined(_MSC_VER) && !defined(_MINGW_))

///////////////////////////////////////////
static int fileSize(const char *filename)
{
    struct stat t;
    stat(filename,&t);
    return t.st_size;
}

///////////////////////////////////////////
void * memoryMap(const char *filename, tFileHandle & file_handle, bool readonly, int offset_, int length)
{
#if defined(_MINGW_) || defined(WIN32)
    PLERROR("memoryMap() - Not supported for MINGW");
    return 0;
#else
    if ((file_handle = (tFileHandle)CreateFile( filename, 
                                                readonly ? GENERIC_READ: GENERIC_READ | GENERIC_WRITE, 
                                                readonly ? FILE_SHARE_READ : 0,
                                                0,
                                                OPEN_ALWAYS, 
                                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                                                0)))
    {
        int file_size = fileSize(filename);
        HANDLE map_handle = CreateFileMapping( (LPVOID)file_handle,
                                               0,
                                               readonly ? PAGE_READONLY : PAGE_READWRITE,
                                               0, 
                                               file_size,
                                               0);
        if (map_handle)
            return (LPVOID) MapViewOfFile( map_handle,
                                           readonly ? FILE_MAP_READ : FILE_MAP_WRITE,
                                           0, 
                                           offset_, 
                                           length ? length : file_size-offset_);
        else {
            PLERROR("In Storage: Could not open specified memory-mapping file for reading");
            return 0; // to keep the compiler quiet
        }
    } 
    else {
        PLERROR("In Storage: Could not open specified memory-mapping file for reading");
        return 0; // to keep the compiler quiet
    }
#endif
}

///////////////////////////////////////////
bool memoryUnmap(void * p, tFileHandle file_handle)
{   
#if defined(_MINGW_) || defined(WIN32)
    PLERROR("memoryUnmap - Not supported for MINGW");
    return false;
#else
    // first parameter: start address of byte range to flush
    // second parameter: number of bytes in range (0==flush all bytes)
    // flush==(linux)msync(...)
    if ((bool)FlushViewOfFile(p,0))       
    {
        // UnmapViewOfFile==(linux)munmap(...)
        bool b=UnmapViewOfFile(p); // address where mapped view begins
        CloseHandle((LPVOID)file_handle);
        return b;
    }
    else return false;
#endif
}


#else // fin de version windows

//////////////////////////////////////////////////
void * MemoryMap(const char *filename,tFileHandle & handle, bool read_only,  off_t & filesize)
{
    void * addr;
    if (read_only)
    {
        handle = open(filename,O_RDONLY);
        if (handle<0)
            PLERROR("In Storage: Could not open specified memory-mapping file for reading");
        // get the size of the file in bytes
        filesize = lseek(handle,0,SEEK_END);
        addr = mmap(0, filesize, PROT_READ, MAP_SHARED, handle, 0);
    }
    else {
        int handle = open(filename,O_RDWR);
        if (handle<0)
            PLERROR("In Storage: Could not open specified memory-mapping file for read-write");
        filesize = lseek(handle,0,SEEK_END); 

        addr = mmap(0, filesize, PROT_READ|PROT_WRITE, MAP_SHARED, handle, 0);
    }
    return addr;
}

//////////////////////////////////////////////////
//void FreeMemoryMap(void * data, tFileHandle handle, int length)
void memoryUnmap(void * data, tFileHandle handle, int length)
{
    msync((char*)data, length, MS_SYNC);
    munmap((char *)data, length);
    close(handle);
}

#endif  // version linux

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
