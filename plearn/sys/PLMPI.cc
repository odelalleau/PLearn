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
 * This file is part of the PLearn library.
 ******************************************************* */

#include "PLMPI.h"
#include <plearn/base/plerror.h>
#include <plearn/io/FdPStreamBuf.h>
#include <plearn/io/PStream.h>
#include <plearn/math/Mat.h>

#include "string.h"
#include <stdio.h>
namespace PLearn {
using namespace std;


streambuf* PLMPI::new_cin_buf = 0;

#if USING_MPI
bool PLMPI::using_mpi = true;
#else
bool PLMPI::using_mpi = false;
#endif

bool PLMPI::synchronized = true;

int PLMPI::size = 0;
int PLMPI::rank = 0;

PStream PLMPI::mycout;
PStream PLMPI::mycerr;
PStream PLMPI::mycin;

int PLMPI::tag = 2909;

void PLMPI::init(int* argc, char*** argv)
{
#ifndef WIN32    
    mycin = new FdPStreamBuf(0, -1);
    mycout = new FdPStreamBuf(-1, 1);
    mycerr = new FdPStreamBuf(-1, 2, false, false);
    
#if USING_MPI
    MPI_Init( argc, argv );
    MPI_Comm_size( MPI_COMM_WORLD, &size) ;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    if(rank!=0)
    {
        cout.rdbuf(nullout.rdbuf());
        cin.rdbuf(nullin.rdbuf());
    }
#endif

#endif // WIN32
}

void PLMPI::finalize()
{
#if USING_MPI
    MPI_Finalize();
#endif 
}


void PLMPI::exchangeBlocks(double* data, int n, int blocksize, double* buffer)
{
//#if USING_MPI
    int blockstart = PLMPI::rank*blocksize;
    //fprintf(stderr,"blockstart = %d\n",blockstart);
    int theblocksize = blocksize;
    int diff=n-blockstart;
    if (blocksize>diff) theblocksize=diff;
    if (theblocksize<=0)
        PLERROR("PLMPI::exchangeBlocks with block of size %d (b=%d,n=%d,r=%d,N=%d)",
                theblocksize,blocksize,n,PLMPI::rank,PLMPI::size);
    //fprintf(stderr,"theblocksize = %d\n",theblocksize);
    if (blocksize*PLMPI::size==n && buffer)
    {
        memcpy(buffer,&data[blockstart],theblocksize*sizeof(double));
#if USING_MPI
        MPI_Allgather(buffer,blocksize,MPI_DOUBLE,
                      data,blocksize,MPI_DOUBLE,MPI_COMM_WORLD);
#endif
    }
    else
    {
        for (int i=0;i<PLMPI::size;i++)
        {
            int bstart = i*blocksize;
            diff = n-bstart;
            int bsize = blocksize;
            if (bsize>diff) bsize=diff;
            //if (i==PLMPI::rank)
            //{
            //fprintf(stderr,"start broadcast of %d, bstart=%d, size=%d\n",i,bstart,bsize);
            //for (int j=0;j<bsize;j++)
            //cerr << data[bstart+j] << endl;
            //}
#if USING_MPI
            MPI_Bcast(&data[bstart],bsize,MPI_DOUBLE,i,MPI_COMM_WORLD);
#endif
            //printf("done broadcast of %d",i);
        }
    }
//#endif
}

void PLMPI::exchangeBlocks(float* data, int n, int blocksize, float* buffer)
{
//#if USING_MPI
    int blockstart = PLMPI::rank*blocksize;
    //fprintf(stderr,"blockstart = %d\n",blockstart);
    int theblocksize = blocksize;
    int diff=n-blockstart;
    if (blocksize>diff) theblocksize=diff;
    if (theblocksize<=0)
        PLERROR("PLMPI::exchangeBlocks with block of size %d (b=%d,n=%d,r=%d,N=%d)",
                theblocksize,blocksize,n,PLMPI::rank,PLMPI::size);
    //fprintf(stderr,"theblocksize = %d\n",theblocksize);
    if (blocksize*PLMPI::size==n && buffer)
    {
        memcpy(buffer,&data[blockstart],theblocksize*sizeof(float));
#if USING_MPI
        MPI_Allgather(buffer,blocksize,MPI_FLOAT,
                      data,blocksize,MPI_FLOAT,MPI_COMM_WORLD);
#endif
    }
    else
    {
        for (int i=0;i<PLMPI::size;i++)
        {
            int bstart = i*blocksize;
            diff = n-bstart;
            int bsize = blocksize;
            if (bsize>diff) bsize=diff;
            //if (i==PLMPI::rank)
            //{
            //fprintf(stderr,"start broadcast of %d, bstart=%d, size=%d\n",i,bstart,bsize);
            //for (int j=0;j<bsize;j++)
            //cerr << data[bstart+j] << endl;
            //}
#if USING_MPI
            MPI_Bcast(&data[bstart],bsize,MPI_FLOAT,i,MPI_COMM_WORLD);
#endif
            //printf("done broadcast of %d",i);
        }
    }
//#endif
}

void PLMPI::exchangeColumnBlocks(Mat sourceBlock, Mat destBlocks)
{
#if USING_MPI
    int width = sourceBlock.width();
    int length = sourceBlock.length();
  
    int total_width = destBlocks.width();
  
    if (total_width%PLMPI::size == 0) // dest width is a multiple of size
    {
        for (int i=0; i<length; i++)
        {
            MPI_Allgather(sourceBlock[i],width,PLMPI_REAL,
                          destBlocks[i],width,PLMPI_REAL,MPI_COMM_WORLD);
        }
    }
    else // last block has different width
    {
        int size_minus_one = PLMPI::size-1;
        int* counts = (int*)malloc(sizeof(int)*PLMPI::size);
        int* displs = (int*)malloc(sizeof(int)*PLMPI::size);
        int* ptr_counts = counts;
        int* ptr_displs = displs;
        int norm_width = total_width / PLMPI::size + 1; // width of all blocks except last
        int last_width = total_width - norm_width*size_minus_one; // width of last block
        if (last_width<=0)
            PLERROR("In PLMPI::exchangeColumnsBlocks: unproper choice of processes (%d) for matrix width (%d) leads "
                    "to width = %d for first mats and width = %d for last mat.",PLMPI::size,total_width,norm_width,last_width);
        // sanity check
        if (PLMPI::rank==size_minus_one)
        {
            if (width!=last_width)
                PLERROR("In PLMPI::exchangeColumnsBlocks: width of last block is %d, should be %d.",width,last_width);
        }
        else
            if (width!=norm_width)
                PLERROR("In PLMPI::exchangeColumnsBlocks: width of block %d is %d, should be %d.",PLMPI::rank,width,last_width);
        for (int i=0;i<size_minus_one;i++)
        {
            *ptr_counts++ = norm_width;
            *ptr_displs++ = norm_width*i;
        }
        // last block
        *ptr_counts = last_width;
        *ptr_displs = norm_width*size_minus_one;
        for (int i=0; i<length; i++)
        {
            MPI_Allgatherv(sourceBlock[i],width,PLMPI_REAL,
                           destBlocks[i],counts,displs,PLMPI_REAL,MPI_COMM_WORLD);
        }
        free(counts);
        free(displs);
    }
#else
    PLWARNING("PLMPI::exchangeColumnsBlocks: in order to use this function, you should recompile with the flag USING_MPI set to 1.");
#endif
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
