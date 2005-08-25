#include <sys/types.h>
#include <unistd.h>                          // for getpid
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "procinfo.h"
#include <plearn/base/plerror.h>
#include <plearn/base/stringutils.h>

namespace PLearn {

int getSystemTotalMemory()
{
    int memory_size=0;
    char units[1000];
    FILE* p=popen("grep MemTotal /proc/meminfo","r");
    fscanf(p,"%*s %d %s",&memory_size,units);
    if (strcmp(units,"kB")==0)
        memory_size*=1024;
    else 
        PLERROR("getSystemTotalMemory: unknown memory units %s",units);
    pclose(p);
    return memory_size;
}

int getProcessDataMemory()
{
    pid_t pid = getpid();
    int memory_size=-1;
    string file = "/proc/"+tostring(pid)+"/status";
    ifstream ifs(file.c_str());
    while (ifs) {
        string line = pgetline(ifs);
        if (line.substr(0,7) == "VmData:") {
            vector<string> elements = split(line);
            memory_size = toint(elements[1]);
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
