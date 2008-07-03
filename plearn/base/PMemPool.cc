// -*- C++ -*-

// PMemPool.cc
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
 * $Id$ 
 ******************************************************* */

// Authors: Nicolas Chapados

/*! \file PMemPool.cc */

// From C++ stdlib
#include <assert.h>

// From PLearn
#include "plerror.h"
#include "PMemPool.h"

namespace PLearn {
using namespace std;


//#####  PMemArena  ###########################################################

PMemArena::PMemArena(size_t object_size_, size_t max_num_objects)
    : storage(0), end_of_storage(0), object_size(object_size_), 
      allocated_objects(0), watermark(0), free_list(0)
{
    if (object_size < sizeof(void*))
        PLERROR("PMemArena::PMemArena: object_size must be at least %ld; passed size is %ld",
                long(sizeof(void*)), long(object_size));
    size_t mem_size = object_size * max_num_objects;
    size_t num_align = mem_size / sizeof(Aligner);
    if (mem_size % sizeof(Aligner) != 0)
        num_align++;
    storage = new Aligner[num_align];
    if (storage) {
        watermark = static_cast<char*>(storage) + object_size * max_num_objects;
        end_of_storage = watermark;
    }
}

PMemArena::~PMemArena()
{
    delete[] static_cast<Aligner*>(storage);
}

void PMemArena::deallocate(void* p)
{
    // If the freed object is exactly at the watermark location, add it back
    // to watermark
    if (p == watermark)
        watermark += object_size;
    else {
        // Otherwise, add p to free_list
        PLASSERT( belongsToArena(p) );
        void** new_free_head = static_cast<void**>(p);
        *new_free_head = free_list;
        free_list = new_free_head;
    }
    allocated_objects--;
}


//#####  PMemPool  ############################################################

PMemPool::PMemPool(size_t object_size_, size_t initial_size_,
                   float growth_factor_, bool use_fast_deallocator)
    : last_arena(0),
      object_size(object_size_),
      initial_arena_size(initial_size_),
      cur_arena_size(initial_size_),
      arena_growth_factor(growth_factor_),
      fast_deallocate(use_fast_deallocator),
      free_list(0)
{ }

void* PMemPool::allocate()
{
    // First try to allocate from last-used arena
    if (last_arena) {
        if (void* new_mem = last_arena->allocate())
            return new_mem;
    }

    // Otherwise, try to allocate from free list
    if (free_list) {
        void** to_return = static_cast<void**>(free_list);
        free_list = *to_return;
        return to_return;
    }

    // Otherwise, try to allocate from an existing arena
    if (void* new_mem = allocateFromArenas())
        return new_mem;

    // Otherwise, create a new arena and allocate from it
    last_arena = newArena();
    if (last_arena)
        if (void* new_mem = last_arena->allocate())
            return new_mem;

    // Last resort, throw bad_alloc...
    throw std::bad_alloc();
}

void PMemPool::deallocate(void* p)
{
    // Fast deallocation :: append to free list
    if (fast_deallocate) {
        void** new_free_head = static_cast<void**>(p);
        *new_free_head = free_list;
        free_list = new_free_head;
    }
    // Traditional deallocator
    else {
        if (last_arena && last_arena->belongsToArena(p))
            last_arena->deallocate(p);
        else {
            // Find arena from map
            map<void*,PMemArena*>::iterator arena_it = stormap.upper_bound(p);
            PLASSERT( arena_it != stormap.begin() );
            --arena_it;
            PMemArena* arena = arena_it->second;
            PLASSERT( arena && arena->belongsToArena(p));
            arena->deallocate(p);
            last_arena = arena;
        }
  
        // Check to see if arena should be eliminated
        if (last_arena && last_arena->empty()) {
            map<void*,PMemArena*>::iterator arena_it = stormap.upper_bound(p);
            PLASSERT( arena_it != stormap.begin() );
            --arena_it;
            PLASSERT( last_arena == arena_it->second );
            stormap.erase(arena_it);
            arenas.remove(last_arena);
            last_arena = 0;
        }
    }
}

void* PMemPool::allocateFromArenas()
{
    for (list< PP<PMemArena> >::iterator it = arenas.begin(), end = arenas.end()
             ; it != end ; ++it) {
        if (void* newmem = (*it)->allocate()) {
            last_arena = *it;
            return newmem;
        }
    }
    return 0;
}

PMemArena* PMemPool::newArena()
{
    PP<PMemArena> new_arena = new PMemArena(object_size, cur_arena_size);
    cur_arena_size = size_t(cur_arena_size * arena_growth_factor);
    stormap[new_arena->storage] = (PMemArena*)new_arena;
    arenas.push_back(new_arena);
    return new_arena;
}
  
void PMemPool::purge_memory()
{
    arenas.clear();
    stormap.clear();
    last_arena = 0;
    free_list  = 0;
    cur_arena_size = initial_arena_size;
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
