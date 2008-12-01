#include <plearn/base/RemoteDeclareMethod.h>  
#include "procinfo.h"
#include <plearn/base/plerror.h>
#include <plearn/base/stringutils.h>
#include <plearn/base/tostring.h>  

#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <nspr/prenv.h>

#if defined(WIN32) && defined(_MSC_VER)
// unistd.h is not available under Microsoft Visual Studio, and some function
// names are not the same.
#define popen _popen
#define pclose _pclose
#define getpid _getpid
#define pid_t int
#include <process.h>
#else
#include <unistd.h>                          // for getpid
#endif

namespace PLearn {

size_t getSystemTotalMemory()
{
    unsigned int memory_size_uint = 0;
    char units[1000];
    FILE* p=popen("grep MemTotal /proc/meminfo","r");
    fscanf(p,"%*s %u %s", &memory_size_uint, units);
    size_t memory_size = size_t(memory_size_uint);
    if (strcmp(units,"kB")==0)
        memory_size*=1024;
    else 
        PLERROR("getSystemTotalMemory: unknown memory units %s",units);
    pclose(p);
    return memory_size;
}

size_t getProcessDataMemory()
{
    pid_t pid = getpid();
    size_t memory_size=0;
    string file = "/proc/"+tostring(pid)+"/status";
    ifstream ifs(file.c_str());
    while (ifs) {
        string line = pgetline(ifs);
        if (line.substr(0,7) == "VmData:") {
            vector<string> elements = split(line);
            memory_size = size_t(toint(elements[1]));
            if (elements[2] == "kB")
                memory_size *= 1024;
            else
                PLERROR("getProcessDataMemory: unknown memory units '%s'",
                        elements[2].c_str());
            break;
        }
    }
    return memory_size;
}

////////////
// getPid //
////////////
int getPid()
{
#if _POSIX_VERSION >= 200112L
#include <unistd.h>
    return getpid();
#else
    return -999;
#endif
}

/////////////
// getUser //
/////////////
string getUser()
{
    const char* h = PR_GetEnv("USER");
    if (!h)
        h = PR_GetEnv("LOGNAME");
    if (!h)
        return "USERNAME_NOT_FOUND";
    return tostring(h);
}

BEGIN_DECLARE_REMOTE_FUNCTIONS

    declareFunction("getSystemTotalMemory", &getSystemTotalMemory,
                    (BodyDoc("Return the total memory installed in the system in bytes."),
                     RetDoc ("Memory size")));

    declareFunction("getProcessDataMemory", &getProcessDataMemory,
                    (BodyDoc("Return the total data memory used by the current process in bytes."),
                     RetDoc ("Used memory size")));

END_DECLARE_REMOTE_FUNCTIONS

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
