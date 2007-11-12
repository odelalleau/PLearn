// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2001 Pascal Vincent
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
 * AUTHORS: Pascal Vincent 
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLMPI.h */

#ifndef PLMPI_INC
#define PLMPI_INC

#if USING_MPI
#include <mpi.h>
#endif

#include <plearn/math/Mat.h>
#include <plearn/base/plerror.h>
// norman: changed to standard calls
#include <iostream>
#include <fstream>
#include <plearn/io/PStream.h>

namespace PLearn {
using namespace std;


/*! Example of code using the PLMPI facility:

In your main function, make sure to call

int main(int argc, char** argv)
{
PLMPI::init(&argc,&argv);
... [ your code here ]
PLMPI::finalize();
}

Inside the #if USING_MPI section, you can use any MPI calls you see fit.

Note the useful global static variables PLMPI::rank that gives you the rank
of your process and PLMPI::size that gives the number of parallel processes
running in MPI_COMM_WORLD. These are initialised in the init code by
calling MPI_Comm_size and MPI_Commm_rank. They are provided so you don't
have to call those functions each time....


File input/output
-----------------

NFS shared files can typically be opened for *reading* from all the nodes
simultaneously without a problem.

For performance reason, it may sometimes be useful to have local copies (on a
/tmp like directory) of heavily accessed files so tat each node can open
its own local copy for reading, resulting in no NFS network traffic and 
no file-server overload.

In general a file cannot be opened for writing (or read&write)
simultaneously by several processes/nodes. Special handling will typically
be necessary. A strategy consists in having only rank#0 responsible for the
file, and all other nodes communicating with rank#0 to access it.

Direct non-read operations on file descriptors or file-like objects through system or 
C calls, are likely to result in an undesired behaviour, when done simultaneously 
on all nodes!!! This  includes calls to the C library's printf and scanf functions. 
Such code will generally require specific handling/rewriting for parallel execution.


cin, cout and cerr
------------------

Currently, the init function redirects the cout and cin of all processes of
rank other than #0 to/from /dev/null but cerr is left as is. Thus sections
not specifically written for parallel execution will have the following
(somewhat reasonable) default behaviour:

- If the same thing is written to cout by several nodes, it will actually
be written only once (only by rank#0). This is to avoid n=size replications
of a regular program's "output".

If for some strange reason you *do* want a non-rank#0 node to write to your
terminal's cout, you can use PLMPI::mycout (which is copied from the initial cout),
but result will probably be ugly and intermixed with the other processes' cout.

- cerr however (which is typically used for debugging, rather than proper
"program output") is kept as is. So a sequential section writing to cerr on
n parallel processes will result in n replications of the message appearing
on your terminal. (which might be useful for debugging, to see what each 
processes is doing, etc...).

- For proper handling of cin, all cin accesses should for now be within 
if(PLMPI::rank==0) blocks (and possibly followed by communicating the entered data to
the other nodes what the need to know).


pofstream
---------

- pofstream is a class that will only open the actual file for writing on
the rank#0 node. On all other nodes, it's directed towards /dev/null. This
can be used to ensure that regular, non-parallel code (including the
sequential sections of a program using some parallel sections), when
launched with mpirun, will not try to open the same file for writing several
times, and not duplicate its "output". The opened file will solely
be the responsibility of node of rank#0. For instance, messages could be sent
from other nodes to node rank#0, where rank#0 would be the only one to
write to the file.

NOTE: While using pofstream may be useful for quickly adapting old code, I rather
recommend using the regular ofstream in conjunction with if(PLMPI::rank==0)
blocks to control what to do.


The PLMPI::synchronization flag
-------------------------------

This flag is important for a particular form of parallelism:

In this paradigm, the data on all nodes (running an identical program) are 
required to be in the same synchronized state before they enter a parallel
computation. In this paradigm, the nodes will typically all execute the
same "sequential" section of the code, on identical data, roughly at the
same time. This state of things will be denoted by "synchronized=true".
Then at some point they may temporarily enter a section where each node
will carry different computation (or a similar computation but on different
parts of the data). This state will be denoted by "synchronized=false". They
typically resynchronize their states before leaving the section, and resume their
synchronized sequential behaviour until they encounter the next parallel section.  

It is very important that sections using this type of paralelism, check if
synchronized==true prior to entering their parallel implementation and fall
back to a strictly sequential impementation otherwise.  They should also
set the flag to false at entry (so that functions they call will stay
sequential) and then set it back to true after resynchronization, prior to
resuming the synchronized "sequential" computations.  

The real meaning of synchronized=true prior to entering 
a parallel code is actually "all the data *this section uses* 
is the same on all nodes, when they reach this point" rather 
than "all the data in the whole program is the same". This is
a subtle difference, but can allow you to set or unset the flag on a
fine grain basis prior to calling potentially parallel functions. 

Here is what typical code should look like under this paradigm: 

int do_something( ... parameters ...)
{
//  ... sequential part

    if(USING_MPI && PLMPI::synchronized && size_of_problem_is_worth_a_parallel_computation)
    { //  Parallel implementation 
#if USING_MPI 
        PLMPI::synchronized = false;

        //  ... each node starts doing sometihng different
        //  ex: switch(PLMPI::rank) ...

        //  Here we may call other functions, that should entirely run in sequential mode (unless you know what you are doing!)

        //  ... we resynchronize the state of each node. (collect their collective answer, etc...) For ex:
        //  MPI_Allgather(...)

        PLMPI::synchronized = true;
#endif 
    }
    else //  default sequential implementation
    {
    }

}

*/



//  Let's define PLMPI_REAL to be either MPI_FLOAT or MPI_DOUBLE

#if USING_MPI
#ifdef USEDOUBLE
#define PLMPI_REAL MPI_DOUBLE
#endif
#ifdef USEFLOAT
#define PLMPI_REAL MPI_FLOAT
#endif
#endif

//!  ** PLMPI is just a "namespace holder" (because we're not actually using namespaces) for a few MPI related variables. All members are static ** 

class PLMPI
{
protected:
    static streambuf* new_cin_buf;

public:
    static bool using_mpi; //!<  true when USING_MPI is defined, false otherwise
    static int size;  //!<  total number of nodes (or processes) running in this MPI_COMM_WORLD  (0 if not using mpi)
    static int rank;  //!<  rank of this node (if not using mpi it's always 0)

/*!     The synchronized flag is used for a particular kind of parallelism and
  is described in more details above, including a sample of how it
  should typically be used. When synchronized is true at a given point
  in the instruciton stream, it roughly means: *** all the data *used by
  the following section* is the same on all nodes when they are at this
  point***".  It's set to true initially. (But will be set to false if
  you launch the PLMPIServ server, which uses a different
  parallelisation paradigm).
*/
    static bool synchronized; //!<  Do ALL the nodes have a synchronized state and are carrying the same sequential instructions? 

/*!     These will correspond to this process's initial streams before we
  tamper with them (init changes cout and cin, redirecting them to/from
  /dev/null for nodes other than the rank#0 node)
*/
//  static oassignstream mycout;
//  static oassignstream mycerr;
//  static iassignstream mycin;
    static PStream mycout;
    static PStream mycerr;
    static PStream mycin;

    //!  The default tag to be used by all send/receive 
    //!  (we typically use this single tag through all of PLearn)
    static int tag; //!<  Defaults to 2909

    static void init(int* argc, char*** argv);
    static void finalize();
  
    // exchange blocks of data among the PLMPI::size CPUs:
    // each CPU has the block starting at position PLMPI::rank*blocksize
    // and they must exchange the blocks so that, upon return,
    // data contains all of them on all the CPUs. A buffer
    // of length blocksize is provided by the caller.
    // The last block may potentially be shorter than blocksize.
    // (only for PLMPI::rank==PLMPI::size-1). This only happens
    // when blocksize*PLMPI::size != n.
    static void exchangeBlocks(double* data,int n,int blocksize,double* buffer=0);  

    // CPU i holds sourceBlock which is going to be
    // copied into the i-th block of columns of the destBlocks
    // matrix in all the CPUs. Note that the last block may be
    // smaller when destBlocks.width() is not a multiple of PLMPI::size.
    static void exchangeColumnBlocks(Mat sourceBlock, Mat destBlocks);

    static void exchangeBlocks(float* data,int n,int blocksize,float* buffer=0);

    
#if USING_MPI  
/*!     Waits for an incoming message from any node (with the default_tag) 
  and returns the rank of the sending node (the source).
  This call is blocking.
*/
    inline static int wait_any();

/*!     Same as wait_any but the call is not blocking.
  If no message is currently waiting to be received from any node, 
  the call returns -1. If there is a waiting message, the call returns
  the rank of the sending node.
*/
    inline static int peek_any();
#endif

};

//!  The stream classes

#if USING_MPI
class pofstream: public ofstream
{
public:
    pofstream() {}
    pofstream(const char *name, ios::openmode mode=ios::out)
    { open(name, mode); }

    void open(const char *name,  ios::openmode mode=ios::out)
    { 
        if(PLMPI::rank==0)
            ofstream::open(name, mode); 
        else
            ios::rdbuf(nullout.rdbuf());
    }

};
#else
typedef ofstream pofstream;
#endif



#if USING_MPI
inline int PLMPI::wait_any()
{
    MPI_Status status;
    MPI_Probe(MPI_ANY_SOURCE, PLMPI::tag, MPI_COMM_WORLD, &status);
    return status.MPI_SOURCE;
}

inline int PLMPI::peek_any()
{
    int ready;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, PLMPI::tag, MPI_COMM_WORLD, &ready, &status);
    if(!ready)
        return -1;
    else
        return status.MPI_SOURCE;
}
#endif

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
