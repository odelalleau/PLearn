// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2000 Yoshua Bengio and University of Montreal
//

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org


/* *******************************************************      
   * $Id: Semaphores.cc,v 1.2 2004/02/20 21:11:49 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include <cstring>
#include <cerrno>
#include "general.h"
#include "Semaphores.h"

namespace PLearn {
using namespace std;

ResourceSemaphore::ResourceSemaphore(int nb_semaphores)
  : owner(true), n_semaphores(nb_semaphores)
{
  // allocate a semaphore 
  int rv=semget(IPC_PRIVATE, n_semaphores, 0666 | IPC_CREAT);
  if (rv == -1) 
    PLERROR("ResourceSemaphore::ResourceSemaphore(%d) semget returns -1, %s",
          nb_semaphores,strerror(errno));
  else id.id=rv;
  cout << "allocated semaphore " << id.id << endl;
  clearAnyLock(); // sets all the semaphore values to 1
}

ResourceSemaphore::ResourceSemaphore(SemId semid) : id(semid), owner(false)
{
  struct semid_ds buf;
  semun u;
  u.buf = &buf;
  int r=semctl(id.id,0,IPC_STAT,u);
  if (r == -1)
    PLERROR("ResourceSemaphore:: slave ResourceSemaphore(%d) semctl returns -1, %s",
          id.id,strerror(errno));
  n_semaphores = u.buf->sem_nsems;
}

void ResourceSemaphore::lock(int resource)
{
  struct sembuf op;
  op.sem_num = resource;
  op.sem_op = -1;  // wait for value = 1 and then decrement to zero
  op.sem_flg = 0;
  int rv=semop(id.id,&op,1);
  if (rv == -1)
    PLERROR("ResourceSemaphore::lock(%d) semop failed, %s",
          resource,strerror(errno));
}

void ResourceSemaphore::unlock(int resource)
{
  if (!locked(resource))
    PLERROR("ResourceSemaphore::unlock(%d), trying to unlock an unlocked resource",
          resource);
  struct sembuf op;
  op.sem_num = resource;
  op.sem_op = 1; // increment value back to 1
  op.sem_flg = 0;
  int rv=semop(id.id,&op,1);
  if (rv == -1)
    PLERROR("ResourceSemaphore::unlock(%d) semop failed, %s",
          resource,strerror(errno));
}

bool ResourceSemaphore::locked(int resource)
{
  semun v; v.val=0;
  int value=semctl(id.id,resource,GETVAL,v);
  return (value==0);
}

void ResourceSemaphore::clearAnyLock()
{
  for (int i=0;i<n_semaphores;i++)
    clearAnyLock(i);
}

void ResourceSemaphore::clearAnyLock(int resource)
{
  // set the semaphore values to 1 (meaning they are in "unlocked" state")
  semun v; v.val=1;
  int rv=semctl(id.id,resource,SETVAL,v);
  if (rv == -1)
    PLERROR("ResourceSemaphore::clearAnyLock(%d) semctl returns -1, %s",
          resource,strerror(errno));
}

ResourceSemaphore::~ResourceSemaphore()
{
  if (owner)
  {
    semun v; v.val=0;
    int rv=semctl(id.id,0,IPC_RMID,v);
    if (rv == -1)
      PLERROR("ResourceSemaphore::~ResourceSemaphore semctl failed, %s",
            strerror(errno));
    cout << "released semaphore " << id.id << endl;
  }
}

CountEventsSemaphore::CountEventsSemaphore(int nb_semaphores)
  : owner(true), n_semaphores(nb_semaphores)
{
  // allocate a semaphore 
  int rv=semget(IPC_PRIVATE, n_semaphores, 0666 | IPC_CREAT);
  if (rv == -1) 
    PLERROR("CountEventsSemaphore::CountEventsSemaphore(%d) semget returns -1, %s",
          nb_semaphores,strerror(errno));
  else id.id=rv;

  cout << "allocated semaphore " << id.id << endl;

  // set the semaphore values to 0 (meaning that initially the counters are at 0)
  semun v; v.val=0;
  for (int i=0;i<n_semaphores;i++)
    {
      rv=semctl(id.id,i,SETVAL,v);
      if (rv == -1)
        PLERROR("CountEventsSemaphore::CountEventsSemaphore(%d) semctl returns -1, %s",
              nb_semaphores,strerror(errno));
    }
}

CountEventsSemaphore::CountEventsSemaphore(SemId semid)
  : id(semid), owner(false)
{
  struct semid_ds buf;
  semun u;
  u.buf = &buf;
  int r=semctl(id.id,0,IPC_STAT,u);
  if (r == -1)
    PLERROR("CountEventsSemaphore:: slave CountEventsSemaphore(%d) semctl returns -1, %s",
          id.id,strerror(errno));
  n_semaphores = u.buf->sem_nsems;
}

void CountEventsSemaphore::signal(int type)
{
  struct sembuf op;
  op.sem_num = type;
  op.sem_op = 1; // increment value 
  op.sem_flg = 0;
  int rv=semop(id.id,&op,1);
  if (rv == -1)
    PLERROR("CountEventsSemaphore::signal(%d) semop failed, %s",
          type,strerror(errno));
}

int CountEventsSemaphore::value(int type)
{
  semun v; v.val=0;
  return semctl(id.id,type,GETVAL,v);
}

void CountEventsSemaphore::wait(int n_occurences, int type)
{
  struct sembuf op;
  op.sem_num = type;
  op.sem_op = -n_occurences; // wait until n_occurences is reached
  op.sem_flg = 0;
  int rv=0;
  do  // this loop is to deal with possible interrupts which
    rv=semop(id.id,&op,1); // will force return of semop before
  while (rv==-1 && errno==EINTR);  // the count is reached
  if (rv == -1) 
    PLERROR("CountEventsSemaphore::wait(%d,%d) semop failed, %s",
          n_occurences,type,strerror(errno));
}

void CountEventsSemaphore::setValue(int value,int resource)
{
  semun v; v.val=value;
  int rv=semctl(id.id,resource,SETVAL,v);
  if (rv == -1)
    PLERROR("ResourceSemaphore::setValue(%d,%d) semctl returns -1, %s",
          value,resource,strerror(errno));
}

CountEventsSemaphore::~CountEventsSemaphore()
{
  if (owner)
  {
    semun v; v.val=0;
    int rv=semctl(id.id,0,IPC_RMID,v);
    if (rv == -1)
      PLERROR("CountEventsSemaphore::~CountEventsSemaphore semctl failed, %s",
            strerror(errno));
    cout << "released semaphore " << id.id << endl;
  }
}

} // end of namespace PLearn
