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
   * $Id: Storage.h,v 1.4 2002/12/05 21:30:52 plearner Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/Storage.h */

#ifndef STORAGE_INC
#define STORAGE_INC

#include "general.h"
#include "PP.h"
#include "MemoryMap.h"
#include <map>

namespace PLearn <%
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

  //!  data is initially filled with zeros
  inline Storage(int the_length=0)
    :length_(the_length), data(0), 
     dont_delete_data(false), fd(STORAGE_UNUSED_HANDLE)
    {
      int l = length();
#ifdef BOUNDCHECK
      if(l<0)
        PLERROR("new Storage called with a length() <0");
#endif
      if (l>0) 
        {
          try 
          {
            data = new T[l];
            if(!data)
              PLERROR("OUT OF MEMORY (new returned NULL) in constructor of storage, trying to allocate %d elements",length());
            clear_n(data,l); // clear the zone
          }
          catch(...)
          {
            PLERROR("OUT OF MEMORY in constructor of storage, trying to allocate %d elements",length());
          }
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
#if defined(_MINGW_)
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
    Storage<T>* deepCopy(map<const void*, void*>& copies) const
    {
      map<const void*, void*>::iterator it = copies.find(this);
      if(it!=copies.end())  //!<  a copy already exists, so return it
        return (Storage<T>*) it->second;
      
      //!  Otherwise call the copy constructor to obtain a copy
      Storage<T>* deep_copy = new Storage<T>(*this);
      //!  Put the copy in the map
      if (usage() > 1)
        copies[this] = deep_copy;
      //!  return the completed deep_copy
      return deep_copy;
    }

  inline void push_back(const T& x)
  {
    int n = size();
    resize(n+1);
    data[n] = x;
  }

    void deepRead(istream& in, DeepReadMap& old2new)
    {
      readHeader(in, "Storage");
      int len;
      PLearn::deepRead(in, old2new, len);
      PLearn::deepRead(in, old2new, dont_delete_data);
      resize(len);
      for (int i=0; i<length(); i++) PLearn::deepRead(in, old2new, data[i]);
      readFooter(in, "Storage");

      fd = STORAGE_UNUSED_HANDLE;
    }

    void deepWrite(ostream& out, DeepWriteSet& already_saved) const
    {
      writeHeader(out, "Storage");
      PLearn::deepWrite(out, already_saved, length());
      PLearn::deepWrite(out, already_saved, dont_delete_data);
      for (int i=0; i<length(); i++) PLearn::deepWrite(out, already_saved, data[i]);
      writeFooter(out, "Storage");
    }

};

template <class T>
inline void deepWrite(ostream& out, DeepWriteSet& already_saved, const Storage<T>& s)
{ s.deepWrite(out, already_saved); }

template <class T>
inline void deepRead(istream& in, DeepReadMap& old2new, Storage<T>& s)
{ s.deepRead(in, old2new); }

template <class T>
inline void deepWrite(ostream& out, DeepWriteSet& already_saved, const PP< Storage<T> >& ptr)
{
  write(out, (unsigned long)(Storage<T>*)ptr);
  if (ptr)
  {
    if (already_saved.find((void*)ptr) == already_saved.end())  //!<  not found
    {
      already_saved.insert((void*)ptr);
      PLearn::deepWrite(out, already_saved, *ptr);
    }
  }
}
 
template <class T>
inline void deepRead(istream& in, DeepReadMap& old2new, PP< Storage<T> >& ptr)
{
  unsigned long old_ptr;
  read(in, old_ptr);
  if (old_ptr)
  {
    if (old2new.find(old_ptr) != old2new.end())
      ptr = (Storage<T>*)old2new[old_ptr];
    else
    {
      ptr = new Storage<T>;
      PLearn::deepRead(in, old2new, *ptr);
      old2new[old_ptr] = ptr;
    }
  }
  else
    ptr = 0;
}

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



%> // end of namespace PLearn

#endif
