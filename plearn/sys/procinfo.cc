
#include <cstdio>
#include <cstdlib>
#include "procinfo.h"
#include "plerror.h"

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

} // end of namespace PLearn
