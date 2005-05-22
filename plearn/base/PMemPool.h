// -*- C++ -*-

// PMemPool.h
//
// Copyright (C) 2005 Nicolas Chapados 
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
   * $Id: PMemPool.h,v 1.2 2005/05/22 04:10:56 chapados Exp $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file PMemPool.h */


#ifndef PMemPool_INC
#define PMemPool_INC

// From C++ stdlib
#include <new>
#include <list>
#include <map>

// From PLearn
#include "PP.h"

namespace PLearn {

// Forward-declaration
class PMemPool;

//#####  PMemArena  ###########################################################

/**
 * A PMemArena is a fixed-size contiguous block of memory for allocating
 * objects of the SAME SIZE.  Allocation and deallocation operations are
 * extremely fast, with the following provision: the arena cannot be
 * resized (see PMemPool for a class that encapsulates several arenas).
 * Internally, memory is either allocated using the "high-watermark" or the
 * free list.  The high-watermark is a pointer that starts at the end of
 * the arena and always decreases, until it reaches the beginning of the
 * block; allocations are always extremely fast, since they correspond to
 * pointer decrements.  As objects are DEALLOCATED, they are added to the
 * free list, from where they can thence be re-allocated once the watermark
 * reaches the bottom.
 */
class PMemArena : public PPointable
{
  friend class PMemPool;                     //!< access to "storage"
  
protected:
  void* storage;                             //!< ptr to physical storage
  void* end_of_storage;                      //!< "one"-past end-of-storage
  size_t object_size;                        //!< size of each allocated object
  size_t allocated_objects;                  //!< number of allocated objects
  char* watermark;                           //!< location of high-watermark
  void* free_list;                           //!< next free object

  //! Utility union to ensure alignment across platforms
  union Aligner {
    char a;
    int b;
    long c;
    double d;
    void* x;
  };

public:
  //! The constructor needs the size of each object (minimum size = sizeof(void*)),
  //! and how many objects the arena should contain
  PMemArena(size_t object_size, size_t max_num_objects);

  //! Free the allocated memory.  Note that since a PMemArena deals with
  //! RAW STORAGE, no destructors are ever called before freeing memory
  ~PMemArena();
  
  //! Allocate one object from the arena.  Return NULL if cannot allocate anything.
  inline void* allocate();

  //! Free one object from the arena
  void deallocate(void* p);

  //! Return true if the pointer corresponds to an object that might have
  //! been allocated from the arena
  bool belongsToArena(void* p) const
    { return p >= storage && p < end_of_storage; }

  //! Return true if no objects remain allocated within the arena
  bool empty() const { return allocated_objects == 0; }

private:
  //! No copy or assignment
  PMemArena(const PMemArena&);
  void operator=(const PMemArena&);
};


//#####  PMemPool  ############################################################

/**
 * A PMemPool is a collection of arenas for allocating an arbitrary number
 * of objects of a fixed size.  In addition to the fixed object size, the
 * user may pass the initial arena size to create and by how much
 * subsequent arenas should grow.
 *
 * The pool offers two methods for object deallocation, the "standard
 * deallocator" and the "fast" one.  The standard deallocator must search
 * for the applicable arena for releasing the memory, which can be rather
 * slow.  On the other hand, if an arena turns out to be empty after
 * releasing the memory, its storage is releaseed, and memory given back to
 * the system.  The fast deallocator simply adds the released block to a
 * pool-local free list (distinct from the Arena free list), and allocate
 * may then take memory from there.  It is, by construction, very fast (and
 * the default behavior), at the cost of code obscurity and a less
 * efficient storage release policy.
 */
class PMemPool : public PPointable
{
protected:
  std::list< PP<PMemArena> > arenas; //!< list of allocated arenas
  PMemArena* last_arena;             //!< arena we last allocated from
  size_t object_size;                //!< size of each allocated object
  size_t initial_arena_size;         //!< initially, now many objects per arena
  size_t cur_arena_size;             //!< currently, how many objects per arena
  float arena_growth_factor;         //!< by how much to grow cur_arena_size
  map<void*, PMemArena*> stormap;    //!< quick access to arena storage addresses
  bool fast_deallocate;              //!< whether to use fast deallocator
  void* free_list;

public:
  //! Constructor specifies the size of each object to be allocated from
  //! the pool, how big (in number of objects) should be the first
  //! allocated arena, and how should subsequent arenas grow in size
  PMemPool(size_t object_size, size_t initial_arena_size,
           float growth_factor = 1.5,
           bool use_fast_deallocator = true);

  // Default destructor is OK

  //! Allocate one object from the pool. Throws bad_alloc if cannot allocate
  void* allocate();

  //! Free an object from the pool; this is not extremely fast, as the
  //! arenas must be searched for who contains the address.  If an arena
  //! becomes completely free, it is destroyed and its memory released.
  void deallocate(void* p);

  //! Return true if no objects remain allocated within the pool
  bool empty() const { return arenas.size() == 0; }

  //! Deallocate all memory from the pool
  void purge_memory();
  
protected:
  //! Search the list of arenas for one that contains some storage;
  //! if found, return it.  last_arena is modified.
  void* allocateFromArenas();
  
  //! Construct a new arena and return it; it is added to arenas and stormap
  PMemArena* newArena();
};


//#####  PObjectPool  #########################################################

/**
 * A PObjectPool is a thin wrapper around PMemPool that provides
 * typed pointers on the allocated memory.  Note that NO CONSTRUCTORS OR
 * DESTRUCTORS are ever called on the memory; this remains the
 * responsibility of the caller.
 */
template <class T>
class PObjectPool : public PMemPool
{
  typedef PMemPool inherited;
  
public:
  PObjectPool(size_t initial_arena_size, float growth_factor = 1.5,
              bool use_fast_deallocator = true)
    : inherited(sizeof(T), initial_arena_size, growth_factor,
                use_fast_deallocator)
    { }

  //! Allocate raw memory for one object. The object is not constructed.
  T* allocate() { return static_cast<T*>(inherited::allocate()); }

  //! Deallocate raw memory from the pool. The object must have been
  //! destroyed prior to calling this function.
  void deallocate(T* p) { inherited::deallocate(p); }
};



//#####  PSystemPool  #########################################################

/**
 * This
 */

//#####  Some Implementations  ################################################

inline void* PMemArena::allocate()
{
  // See if we can allocate from watermark
  if (watermark > storage) {
    watermark -= object_size;
    allocated_objects++;
    return watermark;
  }
  // Otherwise check if we can allocate from free list
  else if (free_list) {
    void** to_return = static_cast<void**>(free_list);
    free_list = *to_return;
    allocated_objects++;
    return to_return;
  }
  // Otherwise, cannot allocate at all: return NULL
  else
    return 0;
}

} // end of namespace PLearn


#endif
