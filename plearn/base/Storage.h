// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * AUTHORS: Pascal Vincent & Yoshua Bengio
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file Storage.h */

#ifndef STORAGE_INC
#define STORAGE_INC

#include <map>
#include "general.h"
#include <plearn/sys/MemoryMap.h>
#include "PP.h"
#include <plearn/io/PStream.h>
#include <limits>

//! A define used to debug Storage::resize.
//! Compile with this symbol defined to enable it.
#ifdef DEBUG_PLEARN_STORAGE_RESIZE
#  include <plearn/sys/procinfo.h>
#endif
//#include "Object.h"

namespace PLearn {
using namespace std;

    
template <class T>
class Storage: public PPointable
{
public: // A few STL-like typedefs
    typedef T value_type;
    typedef int size_type;
    typedef T* iterator;
    typedef const T* const_iterator;

public:
    int length_;
    T* data;
    bool dont_delete_data; //!<  if true, the destructor won't delete[] data, because it will assume it is somebody else's responsibility
    tFileHandle fd; //!<  The descriptor for the memory-mapped file (-1 if there is no memory mapping)

    inline Storage(const Storage& other)
        :length_(other.length()), dont_delete_data(false), fd((tFileHandle)STORAGE_UNUSED_HANDLE)
    {
        try 
        {
            data = new T[length()];
            if(!data)
                PLERROR("OUT OF MEMORY (new returned NULL) in copy constructor of storage, trying to allocate %d elements",length());
            //memcpy(data,other.data,length()*sizeof(T));
            copy(other.data, other.data+length(), data);
        }
        catch(...)
        {
            PLERROR("OUT OF MEMORY in copy constructor of storage, trying to allocate %d elements",length());
        }
    }

    inline Storage(long the_length, T* dataptr)
        :length_(int(the_length)), data(dataptr), 
         dont_delete_data(true), fd(STORAGE_UNUSED_HANDLE)
    {
        //we do the check outside a BOUNDCHECK as we normaly do our test with
        //small dataset. Also, this is not a performance bottleneck and is a
        //fraction of the time of the malloc.
        if(the_length>std::numeric_limits<int>::max())
            PLERROR("In Storage(%ld) - we ask to create a bigger Storage than "
                    "is possible (limited to 2e31, int)",the_length);

    }

    inline int length() const
    { return length_; }

    inline int size() const
    { return length_; }

    inline iterator begin() const
    { return data; }

    inline iterator end() const
    { return data+length_; }

    // If you're wondering why I did this stupid (and useless) function, ask GCC 3.0 that does not accept
    // to allocate a template into the constructor!!!! (and GCC 3.0 is the standard version on SGI)
    void mem_alloc(int len)
    {
        data = new T[len];
        if(!data)
            PLERROR("OUT OF MEMORY (new returned NULL) in constructor of storage, trying to allocate %d elements",length());
        clear_n(data,len); // clear the zone
    }
	

    //!  data is initially filled with zeros
    Storage(long the_length=0)
        :length_((int)the_length), data(0), 
         dont_delete_data(false), fd((tFileHandle)STORAGE_UNUSED_HANDLE)
    {
        //we do the check outside the BOUNDCHECK as we normaly do our test with
        //small dataset. Also, this is not a performance bottleneck and is a
        //fraction of the time of the malloc.
        if(the_length>std::numeric_limits<int>::max())
            PLERROR("In Storage(%ld) - we ask to create a bigger Storage than "
                    "is possible (limited to 2e31, int)",the_length);

        int l = length();
#ifdef BOUNDCHECK
        if(l<0)
            PLERROR("new Storage called with a length() < 0: length = %d", l);
#endif
        if (l>0) 
        {
            mem_alloc(l);
        }
    }

/*!     Constructor for memory-mapped file 
  The file is supposed to exist and have the correct size
  length() of the storage will be set to the size of the file divided by sizeof(T)
*/
    inline Storage(const char* filename, bool readonly)
    {
        void* addr;
        off_t filesize;
        if(readonly)
        {
#if !(!defined(_MSC_VER) && !defined(_MINGW_))
            PLERROR("Not implemented for MinGW");
//!           addr = (T*)memoryMap(filename, fd);
#else
            addr = (T*)MemoryMap(filename, fd, true, filesize);
#endif
        }
        else //!<  read-write
        {
#if !(!defined(_MSC_VER) && !defined(_MINGW_))
            PLERROR("Not implemtned for MinGW");
//!           addr = (T*)memoryMap(filename, fd, false);
#else
            addr = (T*)MemoryMap(filename, fd, false, filesize);
#endif
        }

        if(addr==0)
        {
            perror("Error when calling mmap: ");
            PLERROR("In Storage: Memory-mapping failed");
        }

        data = (T*) addr;
        length_ = filesize/sizeof(T);
        dont_delete_data = false;
    }

    inline void pointTo(int the_length, T* dataptr)
    {
        if (data && !dont_delete_data) 
        {
            if (fd!=STORAGE_UNUSED_HANDLE)//(fd>=0) //!<  we are using a memory-mapped file
            { 
#if !(!defined(_MSC_VER) && !defined(_MINGW_))
//!               memoryUnmap((void *)data,fd); //!<  windows doesnt need length()
                PLERROR("Not implemented for MinGW");
#else
                memoryUnmap((void *)data,fd,length()*sizeof(T));
#endif
            }
            else
                delete[] data;
        }
        length_=the_length;
        data=dataptr;
        fd=STORAGE_UNUSED_HANDLE;
        dont_delete_data=true; //!<  allocated elsewhere
    }

    inline ~Storage()
    {
        if (data && !dont_delete_data) 
        {
            if (fd!=STORAGE_UNUSED_HANDLE)//(fd>=0) //!<  we are using a memory-mapped file
            { 
#if !(!defined(_MSC_VER) && !defined(_MINGW_))
                PLERROR("Not implemented for MinGW");
//!               memoryUnmap((void *)data,fd); //!<  windows doesnt need length()
#else
                memoryUnmap((void *)data,fd,length()*sizeof(T));
#endif
            }
            else
                delete[] data;
        }
    }
      
/*!     Grow or shrink data memory
  If newlength==length() this call does nothing
  If newlength<=0 it outputs an PLERROR(i.e. cannot shrink memory to 0)
  Otherwise this call ALWAYS:
  -> allocates a new block of exactly the given size
  -> copies all the possible the data of the old block to the new one
  -> fills the remaining of the new block (if any) with 0.0
  -> frees the old block
  It is the job of the CALLER (Mat and Vec) to have an appropriate
  policy to minimize the number of calls to Storage::resize 
*/
    inline void resize(long lnewlength)
    {
        //we do the check outside a BOUNDCHECK as we normaly do our test with
        //small dataset. Also, this is not a performance bottleneck and is a
        //fraction of the time of the malloc.
        if(lnewlength>std::numeric_limits<int>::max() || lnewlength<std::numeric_limits<int>::min())
            PLERROR("In Storage(%ld) - we ask to create a bigger/smaller"
                    " Storage than is possible with an int",
                    lnewlength);
#ifdef BOUNDCHECK
        if(lnewlength<0)
            PLERROR("Storage::resize(%ld) called with a length() <0",
                    lnewlength);
#endif

        int newlength=(int)lnewlength;

        if (newlength==length())
            return;
#if defined(_MINGW_) || defined(WIN32)
        else if(fd>0 || dont_delete_data) //!<  we are using a memory-mapped file
#else
            else if(fd>=0 || dont_delete_data) //!<  we are using a memory-mapped file
#endif
                PLERROR("In Storage::resize cannot change size of memory-mapped data or of data allocated elsewhere");
        else if (newlength==0)
        {
            if (data) delete[] data;
            data = 0;
            length_ = 0;
        }
        else if (newlength > length()) //!<  growing
        {
#ifdef DEBUG_PLEARN_STORAGE_RESIZE
            int mem_before = getProcessDataMemory();
            int length_before = length();
#endif
            try 
            {
                T* newdata = new T[newlength];
                if(!newdata)
                    PLERROR("OUT OF MEMORY (new returned NULL) in Storage::resize, trying to allocate %d elements",newlength);
                if(data)
                {
                    // memcpy(newdata,data,length()*sizeof(T));
                    copy(data,data+length(),newdata);
                    delete[] data;
                }
                // memset(&newdata[length()],0,(newlength-length())*sizeof(T));
                clear_n(newdata+length(),newlength-length());
                length_ = newlength;
                data = newdata;
            }
            catch(...)
            {
                PLERROR("OUT OF MEMORY in Storage::resize, trying to allocate %d elements",newlength);
            }
#ifdef DEBUG_PLEARN_STORAGE_RESIZE
            int mem_after = getProcessDataMemory();
            if (mem_after - mem_before > 256*1024)
                cerr << "Storage::resize: for storage at "
                     << hex << this << dec
                     << " fromsize=" << length_before << " tosize=" << newlength
                     << " : memusage " << (mem_before/1024) << " kB  ==>  "
                     << (mem_after/1024) << " kB" << endl;
            if (mem_after - mem_before > 10000*1024)
                PLWARNING("Storage::resize: memory usage increased by more than 10000 kB");
#endif
        }
        else //!<  newlength<length() (shrinking)
        {
            try 
            { 
                T* newdata = new T[newlength];
                if(!newdata)
                    PLERROR("OUT OF MEMORY (new returned NULL) in copy constructor of storage, trying to allocate %d elements",length());

                if(data)
                {
                    //memcpy(newdata,data,newlength*sizeof(T));
                    copy(data,data+newlength,newdata);
                    delete[] data;
                }
                length_ = newlength;
                data = newdata;          
            }
            catch(...)
            {
                PLERROR("OUT OF MEMORY in copy constructor of storage, trying to allocate %d elements",length());
            }
        }
    }


    inline void resizeMat(int new_length, int new_width, int extrarows, int extracols, int new_offset, int old_mod, int old_length, int old_width, int old_offset)
    {
        long ls = new_length*new_width;
        long lextrabytes = (new_length+extrarows)*(new_width+extracols) - ls;
        long lnewsize = new_offset+ls+lextrabytes;

        //we do the check outside a BOUNDCHECK as we normaly do our test with
        //small dataset. Also, this is not a performance bottleneck and is a
        //fraction of the time of the malloc.
        if(lnewsize>std::numeric_limits<int>::max())
            PLERROR("In Storage.resizeMat - we ask to create a bigger Storage "
                    " %ld then is possible (limited to 2e31, int)",lnewsize);
        int newsize=(int)lnewsize;
        int new_mod = new_width+extracols;

#ifdef BOUNDCHECK
        if(newsize<0)
            PLERROR("Storage::resize called with a length() <0");
#endif
        if (newsize==length())
            return;
#if defined(_MINGW_) || defined(WIN32)
        else if(fd>0 || dont_delete_data) //!<  we are using a memory-mapped file
#else
            else if(fd>=0 || dont_delete_data) //!<  we are using a memory-mapped file
#endif
                PLERROR("In Storage::resize cannot change size of memory-mapped data or of data allocated elsewhere");
        else if (newsize==0)
        {
            if (data) delete[] data;
            data = 0;
            length_ = 0;
        }
        else
        {
#ifdef DEBUG_PLEARN_STORAGE_RESIZE
            int mem_before = getProcessDataMemory();
            int length_before = length();
#endif
            try 
            {
                T* newdata = new T[newsize];
                if(!newdata)
                    PLERROR("OUT OF MEMORY (new returned NULL) in Storage::resizeMat, trying to allocate %d elements",newsize);
                if(data)
                {
                    // perform a 'structured' copy that keeps all the old values
                    T* oldp = data+old_offset;
                    T* newp = newdata+new_offset;
                    int w = min(old_width,new_width);
                    int l = min(old_length,new_length);
                    if (new_offset!=0)
                    { 
                        if (new_offset!=old_offset)
                            PLERROR("Storage::resizeMat: when new_offset!=0 it should equal old_offset");
                        copy(data,data+new_offset,newdata);
                    }
                    for (int row=0;row<l;row++, oldp+=old_mod, newp+=new_mod)
                    {
                        copy(oldp,oldp+w,newp);
                        if(new_width>old_width)
                            clear_n(newp+old_width,new_width+extracols-old_width);
                    }
                    if (new_length>old_length)
                        clear_n(newp,(new_length+extrarows-old_length)*new_mod);

                    delete[] data;
                }
                length_ = newsize;
                data = newdata;
            }
            catch(...)
            {
                PLERROR("OUT OF MEMORY in Storage::resize, trying to allocate %d elements",newsize);
            }
#ifdef DEBUG_PLEARN_STORAGE_RESIZE
            int mem_after = getProcessDataMemory();
            if (mem_after - mem_before > 256*1024)
                cerr << "Storage::resize: for storage at "
                     << hex << this << dec
                     << " fromsize=" << length_before << " tosize=" << newsize
                     << " : memusage " << (mem_before/1024) << " kB  ==>  "
                     << (mem_after/1024) << " kB" << endl;
            if (mem_after - mem_before > 10000*1024)
                PLWARNING("Storage::resize: memory usage increased by more than 10000 kB");
#endif
        }
    }

    //!  Deep copying
    Storage<T>* deepCopy(CopiesMap& copies) const
    {
        CopiesMap::iterator it = copies.find(this);
        if(it!=copies.end())  //!<  a copy already exists, so return it
            return (Storage<T>*) it->second;
      
        //!  Otherwise call the copy constructor to obtain a copy
        Storage<T>* deep_copy = new Storage<T>(*this);
        for (int i = 0; i < size(); i++) {
            deepCopyField(deep_copy->data[i], copies);
        }
        //!  Put the copy in the map
        //if (usage() > 1)
        copies[this] = deep_copy;
        //!  return the completed deep_copy
        return deep_copy;
    }

    T& operator[](int idx) const {return data[idx];}

    inline void push_back(const T& x)
    {
        int n = size();
        resize(n+1);
        data[n] = x;
    }

};


template<class T>
PStream& operator<<(PStream& out, const Storage<T>& seq)
{
    writeSequence(out, seq);
    return out;
}

template<class T>
PStream& operator>>(PStream& in, Storage<T>& seq)
{
    readSequence(in, seq);
    return in;
}



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
