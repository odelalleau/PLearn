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
   * $Id: Storage.h,v 1.16 2004/09/14 16:04:35 chrish42 Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/Storage.h */

#ifndef STORAGE_INC
#define STORAGE_INC

#include <map>
#include "general.h"
#include <plearn/sys/MemoryMap.h>
#include "PP.h"
#include <plearn/io/PStream.h>
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

  inline Storage(int the_length, T* dataptr)
    :length_(the_length), data(dataptr), 
     dont_delete_data(true), fd(STORAGE_UNUSED_HANDLE)
    {
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
  Storage(int the_length=0)
    :length_(the_length), data(0), 
     dont_delete_data(false), fd((tFileHandle)STORAGE_UNUSED_HANDLE)
    {
      int l = length();
#ifdef BOUNDCHECK
      if(l<0)
        PLERROR("new Storage called with a length() <0");
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
  inline void resize(int newlength)
    {
#ifdef BOUNDCHECK
      if(newlength<0)
        PLERROR("Storage::resize called with a length() <0");
#endif
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
