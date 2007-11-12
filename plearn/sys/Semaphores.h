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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

// Object-friendly interface to the POSIX semaphore utilities.
// Here we have defined two types of semaphores: ResourceSemaphore
// and CountEventsSemaphore. In both cases multiple semaphores
// of the same type can be managed by the same object.
//
// A ResourceSemaphore allows to control exclusive access to
// some resources (each controlled independently of the others),
// using the lock/unlock methods. When a process tries to
// take the lock while another has it, it will wait until
// the controlling process releases the lock.
//
// A CountEventsSemaphore allows to count the number of occurences
// of one or more events. Some processes can signal the occurence
// of an event of a given type, without waiting. Other processes
// may wait for the count for an event type to reach a certain
// value (and then this value is reset to 0).
//


/*! \file Semaphores.h */

#ifndef SEMAPHORES_INC
#define SEMAPHORES_INC

#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

namespace PLearn {
using namespace std;


#if (defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)) || defined(_NO_XOPEN4)
/*! union semun is defined by including <sys/sem.h> */
#else
/*! according to X/OPEN we have to define it ourselves */
union semun {
    int val;                    /*!< value for SETVAL */
    struct semid_ds *buf;       /*!< buffer for IPC_STAT, IPC_SET */
    unsigned short int *array;  /*!< array for GETALL, SETALL */
    struct seminfo *__buf;      /*!< buffer for IPC_INFO */
};
#endif

//!  This class is defined in order to distinguish semaphore and shared memory id's
//!  from plain integers when constructing a Semaphore or a SharedMemory object. 
class SemId {
public:
    int id;
    SemId(int i = -1) : id(i) {} //!<  -1 means the id was not allocated
    SemId(SemId& semid) : id(semid.id) {}
};

class ResourceSemaphore {
protected:
    SemId id; //!<  semaphore id provided by the operating system at construction
    bool owner; //!<  true if this process is the owner of the semaphore, i.e.
    //!  the semaphore will be released with this object is deleted
    int n_semaphores; //!<  there can be several semaphores in the same object

public:

    //!  construct given number of semaphores for controlling so many exclusive resources.
    ResourceSemaphore(int nb_semaphores=1);

    //!  access an existing semaphore
    ResourceSemaphore(SemId semid);

/*!     Wait for a resource to be freed and then take control of it.
  This is done by waiting for the resource semaphore's to become 1
  and then decrementing its value to 0 (thus taking the lock).
*/
    void lock(int resource=0);

    //!  Release a lock on a resource.
    //!  This is done by incrementing the resource's semaphore. No waiting.
    void unlock(int resource=0);

    //!  to check without waiting wether a resource is locked.
    bool locked(int resource=0);

    //!  make sure we are back in unlocked mode: clear locks of all resources
    void clearAnyLock();
    //!  make sure we are back in unlocked mode: clear lock for given resource
    void clearAnyLock(int resource);

    //!  release the semaphore upon destruction of object
    ~ResourceSemaphore();
};

class CountEventsSemaphore {
protected:
    SemId id; //!<  semaphore id provided by the operating system at construction
    bool owner; //!<  true if this process is the owner of the semaphore, i.e.
    //!  the semaphore will be released with this object is deleted
    int n_semaphores; //!<  there can be several semaphores in the same object

public:

    //!  construct given number of semaphores for event counters
    CountEventsSemaphore(int nb_semaphores=1);

    //!  access an existing semaphore
    CountEventsSemaphore(SemId semid);

    //!  Signal the occurence of event of given "type", where
    //!  type can range from 0 to n_semaphores-1. No waiting.
    void signal(int type=0);

/*!     Wait for n_occurences of event of given "type" to have occured, 
  where "type" can range from 0 to n_semaphoes-1. When this occurs,
  the event count for this type of event is reset to 0.
*/
    void wait(int n_occurences, int type=0);

    //!  return current value of counter for given "type" of event
    int value(int type=0);

    //!  set value of counter 
    void setValue(int value,int resource=0);

    //!  release the semaphore upon destruction of object
    ~CountEventsSemaphore();
};

template <class T>
class SharedMemory {
protected:
    SemId id; //!<  shared memory id provided by the operating system at construction
    bool owner; //!<  true if this process is the owner of the shared memory, i.e.
    //!  the shared memory will be released with this object is deleted
    int size_; //!<  in number of bytes
    T* segment; //!<  allocated segment
public:
    //!  allocate shared memory 
    SharedMemory(int n_items=1) : owner(true) {
        size_ = n_items*sizeof(T);
        int rv=shmget(IPC_PRIVATE, size_, 0666 | IPC_CREAT);
        if (rv == -1) PLERROR("SharedMemory::SharedMemory, shmget failed:%s",strerror(errno));
        else id.id=rv;
        segment = (T*)shmat(id.id,0,0);
        if (segment == 0) 
            PLERROR("SharedMemory::SharedMemory, shmat failed trying to allocate %d bytes: err=%s",
                    size_,strerror(errno));

    }

    //!  access an existing shared memory area
    SharedMemory(SemId semid) : id(semid), owner(false) {
        struct shmid_ds buf;
        int r = shmctl(id.id,IPC_STAT,&buf);
        if (r == -1)
            PLERROR("SharedMemory:: slave SharedMemory(%d) shmctl returns -1, %s",
                    id.id,strerror(errno));
        size_ = buf.shm_segsz;
        segment = (T*)shmat(id.id,0,0);
    }

    //!  convert to address of beginning of shared memory segment
    T* data() const { return segment; }

    int size() const { return size_ / sizeof(T); }

    //!  release id and memory
    ~SharedMemory() {
        int rv=shmdt((char*)segment);
        if (rv == -1)
            PLERROR("SharedMemory::~SharedMemory (id=%d) shmdt failed, %s", 
                    id.id,strerror(errno));
        if (owner)
        {
            rv=shmctl(id.id,IPC_RMID,0);
            if (rv == -1)
                PLERROR("SharedMemory::~SharedMemory (id=%d) shmctl failed, %s", 
                        id.id,strerror(errno));
            cout << "released shared memory segment ID = " << id.id << endl;
        }
    }
};

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
